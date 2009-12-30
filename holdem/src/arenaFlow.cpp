/***************************************************************************
 *   Copyright (C) 2009 by Joseph Huang                                    *
 *                                                                         *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU Library General Public License as       *
 *   published by the Free Software Foundation; either version 2 of the    *
 *   License, or (at your option) any later version.                       *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU Library General Public     *
 *   License along with this program; if not, write to the                 *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/


#include "arena.h"


/* Application flow
 *
 * BeginInitialStateLOGIC
 * loop{
 *     DealHands();
 *     repeat
 *     {
 *         ~ShowCardsToBots(Cards, Flop, Turn, River) for each Strat
 *         BeginBettingRound();
 *         loop{
 *             ~GetBetFromBot()
 *             MakeBet();
 *         }
 *     }
 *     RefreshPlayers();
 * }
 * FinalizeReportWinner();
 *
 */




DeckLocation HoldemArena::ExternalQueryCard(std::istream& s)
{
    DeckLocation userCard;
	/*
    #ifdef DEBUGSAVEGAME
    bool bNextCardSaved;
    bNextCardSaved = bLoadGame && (!loadFile.eof());
    bNextCardSaved = bNextCardSaved && (loadFile.rdbuf()->in_avail() > 0);
    bNextCardSaved = bNextCardSaved && (loadFile.is_open());


    while( bNextCardSaved )
    {
        int16 upcomingChar = loadFile.peek();
        if( upcomingChar == '\n' || upcomingChar == '\r'  || upcomingChar == ' ' )
        {
            loadFile.ignore(1);
            bNextCardSaved = bLoadGame && (loadFile.is_open()) && (!loadFile.eof()) && (loadFile.rdbuf()->in_avail() > 0);
        }else
        {
            bNextCardSaved = true;
            break;
        }
    }
    if( bNextCardSaved )
    {
        userCard.SetByIndex( HoldemUtil::ReadCard( loadFile ) );
    }else
    #endif
    {
        bLoadGame = false;
	*/
        userCard.SetByIndex( HoldemUtil::ReadCard( s ) );
    //}
    return userCard;
}


void HoldemArena::RequestCards(SerializeRandomDeck * myDealer, uint8 numCards, CommunityPlus & intoCards, const char * request_str)
//, std::ofstream *saveCards)
{
    if( myDealer )
    {

        for(uint8 n=0;n<numCards;++n)                    {
            if (!(  myDealer->DealCard(intoCards)  ))        {
                std::cerr << "OUT OF CARDS ERROR" << endl; exit(1);        }}

    }
    else
    {
        std::cerr << request_str << endl;
        std::cin.sync();
        std::cin.clear();

        intoCards.SetEmpty();
        for(uint8 n=0;n<numCards;++n) intoCards.AddToHand(ExternalQueryCard(std::cin));


        std::cin.sync();
        std::cin.clear();
        /*
        if( saveCards )
        {
            intoCards.HandPlus::DisplayHand(*saveCards);
            saveCards->flush();
        }
        */
    }

}

DeckLocation HoldemArena::RequestCard(SerializeRandomDeck * myDealer)
//, std::ofstream * saveCards)
{
    DeckLocation intoCard;

    if( myDealer )
    {
        Hand newCard;
        if (!(  myDealer->DealCard(newCard)  ))
        {
            std::cerr << "OUT OF CARDS ERROR" << endl;
            exit(1);
        }

        intoCard = myDealer->dealt;
    }
    else
    {
        std::cerr << "Please enter the next community card (no whitespace): " << endl;
        std::cin.sync();
        std::cin.clear();
        intoCard = ExternalQueryCard(std::cin);
        std::cin.sync();
        std::cin.clear();
        /*
        if( saveCards )
        {
            HoldemUtil::PrintCard( *saveCards, intoCard.GetIndex() );
            saveCards->flush();
        }
        */
    }

    return intoCard;

}

bool HoldemArena::ShowHoleCards(const Player & withP, const CommunityPlus & dealHandP)
{
	if( withP.myMoney > 0 )
	{
		if( withP.IsBot() )
		{
			withP.myStrat->StoreDealtHand(dealHandP);
			return true;
		}
	}

	return false;
}

void HoldemArena::DealAllHands(SerializeRandomDeck * tableDealer, std::ofstream & holecardsData)
{


    do
    {
        incrIndex();

        Player& withP = *(p[curIndex]);

        if(withP.myMoney > 0)
        {

            if( withP.IsBot() )
            {
                CommunityPlus dealHandP;

                if( !tableDealer ) std::cerr << withP.GetIdent().c_str() << std::flush;
                RequestCards(tableDealer,2,dealHandP,", enter your cards (no whitespace): ");

				#ifdef DEBUGASSERT
				if(!
				#endif
				ShowHoleCards(withP,dealHandP)
				#ifdef DEBUGASSERT
				){
					std::cerr << "Dealing card to player who doesn't request them?" << endl;
					exit(1);
				}
				#endif
				;;

                dealHandP.HandPlus::DisplayHand(holecardsData);
                holecardsData << withP.GetIdent().c_str() << endl;
            }
        }

    }while(curDealer != curIndex);
}



//If tableDealer is null, you may specify dealt cards using the console.
void HoldemArena::BeginNewHands(const BlindValues & roundBlindValues, const bool & bNewBlindValues, playernumber_t newDealer)
{
	if( IsAlive(newDealer) ) curDealer = newDealer;

    startRoundPlayers = livePlayers;

    myPot        = 0;
    prevRoundPot = 0;
    prevRoundFoldedPot = 0;
    myFoldedPot  = 0;

    curIndex = curDealer;

    if( bVerbose && bSpectate )
    {
        gamelog << endl << "Next Dealer is " << p[curDealer]->GetIdent() << endl;
    }


    if( bVerbose )
    {
        gamelog << "================================================================" << endl;
        gamelog << "============================New Hand" <<
        " #"<< handnum <<
        "========================" << endl;

    }

    myBlinds = roundBlindValues;

    if( bNewBlindValues )
    {
        if( bVerbose )
        {
            gamelog << "Blinds increased to " << myBlinds.GetSmallBlind() << "/" << myBlinds.GetBigBlind() << endl;
        }
    }


	do
    {
        incrIndex();

        Player& withP = *(p[curIndex]);

        if(withP.myMoney <= 0)
        {
            withP.lastBetSize = INVALID;
            withP.myBetSize = INVALID;
        }



    }while(curDealer != curIndex);


}

void HoldemArena::AssertInitialState()
{
#ifdef DEBUGASSERT

    bool bPlayersWithMoney = false;
    for(playernumber_t t=0;t<SEATS_AT_TABLE;++t)
    {
        if( p[t] != 0 )
        {
            if( p[t]->myMoney > 0 )
            {
                bPlayersWithMoney = true;
                break;
            }
        }
    }

    if( livePlayers == 0 || nextNewPlayer == 0 || !bPlayersWithMoney)
    {
        std::cerr << "Add players before playing";
        exit(1);
    }

#endif


}

void HoldemArena::ResetDRseed()
{
    randRem = 1;
}

float64 HoldemArena::GetDRseed()
{
    return randRem;
}



void HoldemArena::BeginInitialState(handnum_t game_id)
{


        curIndex = 0;
        curDealer = 0;

        handnum = game_id;


}


Player * HoldemArena::FinalizeReportWinner()
{



    for(int8 i=0;i<nextNewPlayer;++i)
    {
        Player *withP = (p[i]);
        if( withP->myMoney > 0 ) return withP;
    }
    return 0;
}


void HoldemArena::PrepBettingRound(bool bFirstBettingRound, uint8 otherBettingRounds)
{
	if( bFirstBettingRound ){
		playersInHand = livePlayers; //Must be called before first initRoundPlayers();
		playersAllIn = 0;
	}

	bettingRoundsRemaining = otherBettingRounds;
	initRoundPlayers(); //Must be called after playersInHand initialized
}


int8 HoldemArena::PlayRound_BeginHand()
{
    gamelog << "BEGIN" << endl;


    //Preflop
	PrepBettingRound(true,3);  //flop, turn, river remaining


    return PlayRound(CommunityPlus::EMPTY_COMPLUS,0);

}


int8 HoldemArena::PlayRound_Flop(const CommunityPlus & flop)
{


    if( bSpectate )
    {
        gamelog << endl;
        gamelog << "Flop:\t" << flush;
        flop.HandPlus::DisplayHand(gamelog);
        gamelog << "   " << flush;

    }

	PrepBettingRound(false,2); //turn, river remaining

    return PlayRound(flop,3);
}

int8 HoldemArena::PlayRound_Turn(const CommunityPlus & flop, const DeckLocation & turn)
{
    CommunityPlus community;
    community.SetUnique(flop);
    community.AddToHand(turn);

    if( bSpectate )
    {
        gamelog << endl;
        gamelog << "Turn:\t" << flush;

        flop.HandPlus::DisplayHand(gamelog);
        HoldemUtil::PrintCard(gamelog, turn.Suit,turn.Value);
        gamelog << "   " << flush;
    }

    PrepBettingRound(false,1); //river remaining


    return PlayRound(community,4);

}

int8 HoldemArena::PlayRound_River(const CommunityPlus & flop, const DeckLocation & turn, const DeckLocation & river)
{
    CommunityPlus community;
    community.SetUnique(flop);
    community.AddToHand(turn);
    community.AddToHand(river);

    if( bSpectate )
    {
        gamelog << endl;
        gamelog << "River:\t" << flush;

        flop.HandPlus::DisplayHand(gamelog);
        HoldemUtil::PrintCard(gamelog, turn.Suit,turn.Value);
        gamelog << " " << flush;

        HoldemUtil::PrintCard(gamelog, river.Suit,river.Value);
        gamelog << "  " << flush;
    }


    PrepBettingRound(false,0); //no other betting rounds

    return PlayRound(community,5);
}




