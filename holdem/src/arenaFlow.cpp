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

