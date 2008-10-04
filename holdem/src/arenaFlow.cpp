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

bool HoldemArena::BeginInitialState()
{
	if( p.empty() ) return false;

#ifdef DEBUGSAVEGAME
    if( !bLoadGame )
#endif
    {
        curIndex = 0;
        curDealer = 0;

        if( !bExternalDealer )
        {
            dealer.ShuffleDeck(static_cast<float64>(livePlayers));
        }


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




void HoldemArena::PlayGame()
{

    gamelog << "BEGIN" << endl;

    CommunityPlus flop;
    DeckLocation turn;


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

	community.SetEmpty();

	--bettingRoundsRemaining;
	if( PlayRound(0) == -1 ) return;


    if( bExternalDealer )
    {
        std::cerr << "Please enter the flop (no whitespace): " << endl;
        std::cin.sync();
        std::cin.clear();
        community.AddToHand(ExternalQueryCard(std::cin));
        community.AddToHand(ExternalQueryCard(std::cin));
        community.AddToHand(ExternalQueryCard(std::cin));
        std::cin.sync();
        std::cin.clear();
        #ifdef DEBUGSAVEGAME
        if( !bLoadGame )
        {
            std::ofstream saveFile(DEBUGSAVEGAME,std::ios::app);
            community.HandPlus::DisplayHand(saveFile);
            saveFile.close();
        }
        #endif
    }
    else
    {
        if (!dealer.DealCard(community))  gamelog << "OUT OF CARDS ERROR" << endl;
        if (!dealer.DealCard(community))  gamelog << "OUT OF CARDS ERROR" << endl;
        if (!dealer.DealCard(community))  gamelog << "OUT OF CARDS ERROR" << endl;
    }


    if( bSpectate )
    {
        gamelog << endl;
        gamelog << "Flop:\t" << flush;
        community.HandPlus::DisplayHand(gamelog);

        flop.SetUnique(community);
        gamelog << "   " << flush;

    }


	roundPlayers = livePlayers;
	--bettingRoundsRemaining;
	if( PlayRound(3) == -1 ) return;


    if( bSpectate )
    {
        gamelog << endl;
        gamelog << "Turn:\t" << flush;
    }

    if( bExternalDealer )
    {
        std::cerr << "Please enter the turn (no whitespace): " << endl;
        std::cin.sync();
        std::cin.clear();
        turn = ExternalQueryCard(std::cin);
        community.AddToHand(turn);
        std::cin.sync();
        std::cin.clear();
        #ifdef DEBUGSAVEGAME
        if( !bLoadGame )
        {
            std::ofstream saveFile(DEBUGSAVEGAME,std::ios::app);
            HoldemUtil::PrintCard( saveFile, turn.GetIndex() );
            saveFile.close();
        }
        #endif
    }
    else
    {
        if (!dealer.DealCard(community))  gamelog << "OUT OF CARDS ERROR" << endl;

        turn = dealer.dealt;
    }


	if( bSpectate )
	{
	    flop.HandPlus::DisplayHand(gamelog);
        HoldemUtil::PrintCard(gamelog, turn.Suit,turn.Value);
        gamelog << "   " << flush;
	}


	roundPlayers = livePlayers;
	--bettingRoundsRemaining;
	if( PlayRound(4) == -1 ) return;



    if( bSpectate )
    {
        gamelog << endl;
        gamelog << "River:\t" << flush;
    }
    DeckLocation river;

    if( bExternalDealer )
    {
        std::cerr << "Please enter the river (no whitespace): " << endl;
        std::cin.sync();
        std::cin.clear();
        river = ExternalQueryCard(std::cin);
        community.AddToHand(river);
        std::cin.sync();
        std::cin.clear();
        #ifdef DEBUGSAVEGAME
        if( !bLoadGame )
        {
            std::ofstream saveFile(DEBUGSAVEGAME,std::ios::app);
            HoldemUtil::PrintCard( saveFile, river.GetIndex() );
            saveFile.close();
        }
        #endif
    }
    else
    {
        dealer.DealCard(community);
        river = dealer.dealt;
    }

    if( bSpectate )
    {
        flop.HandPlus::DisplayHand(gamelog);
        HoldemUtil::PrintCard(gamelog, turn.Suit,turn.Value);
        gamelog << " " << flush;

        HoldemUtil::PrintCard(gamelog, river.Suit,river.Value);
         gamelog << "  " << flush;
    }


	roundPlayers = livePlayers;
	--bettingRoundsRemaining;
	int8 playerToReveal = PlayRound(5);

	if( playerToReveal == -1 ) return;
	roundPlayers = livePlayers;

	PlayShowdown(playerToReveal);
}


