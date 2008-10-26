/***************************************************************************
 *   Copyright (C) 2008 by Joseph Huang                                    *
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
#include <string.h>


/* Application flow
 *
 * BeginInitialState()
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



void HoldemArena::RequestCards(SerializeRandomDeck * myDealer, uint8 numCards, CommunityPlus & intoCards, const char * request_str)
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
        #ifdef DEBUGSAVEGAME
        if( !bLoadGame )
        {
            std::ofstream saveFile(DEBUGSAVEGAME,std::ios::app);
            intoCards.HandPlus::DisplayHand(saveFile);
            saveFile.close();
        }
        #endif
    }

}

DeckLocation HoldemArena::RequestCard(SerializeRandomDeck * myDealer)
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
        #ifdef DEBUGSAVEGAME
        if( !bLoadGame )
        {
            std::ofstream saveFile(DEBUGSAVEGAME,std::ios::app);
            HoldemUtil::PrintCard( saveFile, intoCard.GetIndex() );
            saveFile.close();
        }
        #endif
    }

    return intoCard;

}



void HoldemArena::DealAllHands(SerializeRandomDeck * tableDealer)
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

                withP.myStrat->StoreDealtHand(dealHandP);

                dealHandP.HandPlus::DisplayHand(holecardsData);
                holecardsData << withP.GetIdent().c_str() << endl;
            }
        }
        else
        {
            withP.lastBetSize = INVALID;
            withP.myBetSize = INVALID;
        }



    }while(curDealer != curIndex);
}



//If tableDealer is null, you may specify dealt cards using the console.
void HoldemArena::BeginNewHands(SerializeRandomDeck * tableDealer)
{
    roundPlayers = livePlayers;

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
        #if defined(GRAPHMONEY)
        " #"<< handnum <<
        #else
        "==" <<
        #endif //GRAPHMONEY, with #else
        "========================" << endl;

    }





    if( tableDealer )
    {
        SerializeRandomDeck & dealer = *tableDealer;



        #ifdef DEBUGSAVEGAME
        if( !bLoadGame )
        #endif
        {
            //Shuffle the deck here
            dealer.UndealAll();
            if(randRem != 0)
            {
                #ifdef DEBUGSAVEGAME
                std::ofstream shuffleData( DEBUGSAVEGAME, std::ios::app );
                dealer.LoggedShuffle(shuffleData, randRem);
                shuffleData << endl;
                shuffleData.close();



                    #if defined(DEBUGSAVEGAME_ALL) && defined(GRAPHMONEY)
                char handnumtxt[23+12] = "./" DEBUGSAVEGAME_ALL "/" DEBUGSAVEGAME "-";

                FileNumberString( handnum , handnumtxt + strlen(handnumtxt) );
                handnumtxt[23+12-1] = '\0'; //just to be safe

                shuffleData.open( handnumtxt , std::ios::app );
                dealer.LogDeckState( shuffleData );
                shuffleData << endl;
                shuffleData.close();
                    #endif



                #else
                dealer.ShuffleDeck( randRem );
                #endif


        #ifdef DEBUGASSERT
            }
            else
            {
                std::cerr << "YOU DIDN'T SHUFFLE" << endl;
                gamelog << "YOU DIDN'T SHUFFLE" << endl;
                exit(1);
        #endif
            }
        }


        ///Note: If tableDealer is defined,
        ///ExternalQueryCard won't manage bLoadGame for us.
        ///We need to clear it here.
        if( bLoadGame )
        {
            bLoadGame = false;
        }

    }

    #ifdef DEBUGHOLECARDS
        holecardsData <<
                #if defined(GRAPHMONEY)
                "############ Hand " << handnum << " " <<
                #endif
        "############" << endl;

    #endif


        randRem = 1;





}




bool HoldemArena::BeginInitialState()
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

#ifdef DEBUGSAVEGAME
    if( !bLoadGame )
#endif
    {
        curIndex = 0;
        curDealer = 0;

            #ifdef GRAPHMONEY

                handnum = 1;
                scoreboard.open(GRAPHMONEY);
                scoreboard << "#Hand";
                for(int8 i=0;i<nextNewPlayer;++i)
                {
                    scoreboard << "," << (p[i])->GetIdent();
                }
                scoreboard << endl;
                scoreboard << "0";
                for(int8 i=0;i<nextNewPlayer;++i)
                {
                    scoreboard << "," << (p[i])->GetMoney();
                }
                scoreboard << endl;
            #endif

        #ifdef REPRODUCIBLE
            randRem = 1;
        #endif

        #ifdef DEBUGHOLECARDS
        holecardsData.open( DEBUGHOLECARDS );
        #endif
            /*
#ifdef DEBUGSAVEGAME
             std::ofstream killfile(DEBUGSAVEGAME,std::ios::out | std::ios::trunc);
             killfile.close();
#endif
             */
#ifdef DEBUGSAVEGAME
            saveState();
#endif
    }

#if defined(GRAPHMONEY) && defined(DEBUGSAVEGAME)
    else
    {
        scoreboard.open(GRAPHMONEY , std::ios::app);
        #ifdef DEBUGHOLECARDS
        holecardsData.open( DEBUGHOLECARDS, std::ios::app );
        #endif

    return false;
    }
#endif

    return true;

}


Player * HoldemArena::FinalizeReportWinner()
{

#ifdef GRAPHMONEY
    scoreboard.close();
#endif



    for(int8 i=0;i<nextNewPlayer;++i)
    {
        Player *withP = (p[i]);
        if( withP->myMoney > 0 ) return withP;
    }
    return 0;
}


int8 HoldemArena::PlayRound_BeginHand()
{
    gamelog << "BEGIN" << endl;



    if( blinds->HandPlayed(0) )
    {
        if( bVerbose )
        {
            gamelog << "Blinds increased to " << blinds->mySmallBlind << "/" << blinds->myBigBlind << endl;
        }
    }

    playersInHand = livePlayers;
    roundPlayers = livePlayers;
    bettingRoundsRemaining = 4;
    playersAllIn = 0;



    --bettingRoundsRemaining;

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


    roundPlayers = livePlayers;
    --bettingRoundsRemaining;


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


    roundPlayers = livePlayers;
    --bettingRoundsRemaining;

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


    roundPlayers = livePlayers;
    --bettingRoundsRemaining;

    return PlayRound(community,5);
}




