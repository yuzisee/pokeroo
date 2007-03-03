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

#ifndef HOLDEM_ThresholdStrat
#define HOLDEM_ThresholdStrat

#ifndef NO_LOG_FILES
#define LOGTHRESHOLD
#endif


#include "stratSearch.h"
#include "ai.h"

class ThresholdStrategy : virtual public SearchStrategy
{
	protected:
        #ifdef LOGTHRESHOLD
        ofstream logFile;
        #endif
		float64 aiThreshold;
	public:

		ThresholdStrategy(float64 thresh=.5) : SearchStrategy(), aiThreshold(thresh) {}
		virtual ~ThresholdStrategy();

		virtual void SeeCommunity(const Hand&, const int8);
		virtual float64 MakeBet();
		virtual void SeeOppHand(const int8, const Hand&){};
        virtual void SeeAction(const HoldemAction&) {};
}
;

class MultiThresholdStrategy : virtual public ThresholdStrategy
{
    protected:
        int8 redundancy;
        int8 bCall;
	public:

		MultiThresholdStrategy(int8 tight = 0, int8 call = 0, float64 thresh=.5) : ThresholdStrategy(thresh), redundancy(tight), bCall(call){}
		//virtual ~MultiThresholdStrategy();

		virtual void SeeCommunity(const Hand&, const int8);
		virtual float64 MakeBet();
		virtual void FinishHand(){}
		//virtual void SeeOppHand(const int8, const Hand&);
        //virtual void SeeAction(const HoldemAction&);
}
;

#endif
