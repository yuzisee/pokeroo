/***************************************************************************
 *   Copyright (C) 2006 by Joseph Huang                                    *
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

#ifndef HOLDEM_TournamentStrat
#define HOLDEM_TournamentStrat

#include "stratSearch.h"
#include "ai.h"

#define LOGTOURN

class TournamentStrategy : virtual public PlayerStrategy
{
    protected:
        DistrShape detailPCT;
        int8 bGamble;
        StatResult statmean;
        StatResult statworse;
        //CallCumulationD foldcumu;
        CallCumulationD callcumu;
        #ifdef LOGTOURN
        ofstream logFile;
        #endif
	public:

		TournamentStrategy(int8 riskymode = 0) : PlayerStrategy(), detailPCT(0), bGamble(riskymode) {}
		virtual ~TournamentStrategy();

		virtual void SeeCommunity(const Hand&, const int8);
		virtual float64 MakeBet();
		virtual void SeeOppHand(const int8, const Hand&){};
        virtual void SeeAction(const HoldemAction&) {};
}
;


#endif


