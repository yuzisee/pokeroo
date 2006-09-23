/***************************************************************************
 *   Copyright (C) 2005 by Joseph Huang                                    *
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

#ifndef HOLDEM_PositionalStrat
#define HOLDEM_PositionalStrat

#include "stratSearch.h"
#include "ai.h"

#define LOGPOSITION
//#define DEBUGSPECIFIC


///TODO: Modularize this class so as to try other combinations
class PositionalStrategy : virtual public PlayerStrategy
{
	protected:
        int8 bGamble; /* 0,1,2,3 */
        int8 roundNumber;
        DistrShape detailPCT;
        StatResult statmean;
        StatResult statworse;
        CallCumulationD callcumu;
        #ifdef LOGPOSITION
        ofstream logFile;
        #endif
	public:

		PositionalStrategy(int8 riskymode) : PlayerStrategy(), bGamble(riskymode), detailPCT(0) {}
		virtual ~PositionalStrategy();

		virtual void SeeCommunity(const Hand&, const int8);
		virtual float64 MakeBet();
		virtual void SeeOppHand(const int8, const Hand&){};
        virtual void SeeAction(const HoldemAction&) {};
}
;

#endif
