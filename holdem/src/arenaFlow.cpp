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


void HoldemArena::PlayGame(SerializeRandomDeck * tableDealer)
{

	if( PlayRound_BeginHand() == -1 ) return;

	CommunityPlus myFlop;
	RequestCards(tableDealer,3,myFlop);
    if( PlayRound_Flop(myFlop) == -1 ) return;


	DeckLocation myTurn = RequestCard(tableDealer);
    if( PlayRound_Turn(myFlop,myTurn) == -1 ) return;

	DeckLocation myRiver = RequestCard(tableDealer);
    int8 playerToReveal = PlayRound_River(myFlop,myTurn,myRiver);
    if( playerToReveal == -1 ) return;


	CommunityPlus finalCommunity;
	finalCommunity.SetUnique(myFlop);
	finalCommunity.AddToHand(myTurn);
	finalCommunity.AddToHand(myRiver);

	roundPlayers = livePlayers;
	PlayShowdown(finalCommunity,playerToReveal);
}




bool HoldemArena::BeginInitialState()
{
	if( p.empty() ) return false;

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


void HoldemArena::RequestCards(SerializeRandomDeck * myDealer, uint8 numCards, CommunityPlus & intoCards)
{
	if( myDealer )
    {

		for(uint8 n=0;n<numCards;++n)					{
			if (!(  myDealer->DealCard(intoCards)  ))		{
				std::cerr << "OUT OF CARDS ERROR" << endl; exit(1);		}}

	}
	else
	{
        std::cerr << "Please enter the flop (no whitespace): " << endl;
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



