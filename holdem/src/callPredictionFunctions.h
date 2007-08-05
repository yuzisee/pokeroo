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


#ifndef HOLDEM_OpponentFunctions
#define HOLDEM_OpponentFunctions


#include "functionbase.h"
#include "callSituation.h"



#define RAREST_HAND_CHANCE 221.0


//For calculating geometric betting expectation
class ExactCallFunctionModel : public virtual ScalarFunctionModel
{
    protected:

        const CallCumulationD* e;

    public:

        float64 Bankroll;

        float64 n;

        float64 pot;

        float64 alreadyBet;
        float64 bet;

        float64 avgBlind;

        bool bOppCheck;

        ExactCallFunctionModel(float64 step, const CallCumulationD* e) : ScalarFunctionModel(step), e(e){};

        virtual ~ExactCallFunctionModel();

        virtual float64 f(const float64);
        virtual float64 fd(const float64, const float64);


    #ifdef DEBUG_CALLPRED_FUNCTION
        void breakdown(float64 points, std::ostream& target, float64 start=0, float64 end=1)
        {
            target.precision(17);

            float64 dist;
            if( points > 0 ) dist = (end-start)/points;


            target << "w,p(w),dp/dw,avgBlind/fNRank" << std::endl;
            if( points > 0 && dist > 0 )
            {
                for( float64 i=start;i<=end;i+=dist)
                {

                    const float64 y = f(i);
					const float64 frank = e->pctWillCall(1 - i);
					const float64 fNRank = (frank >= 1) ? 1.0/RAREST_HAND_CHANCE : (1 - frank);



                    target << i << "," << y << "," << fd(i,y) << "," << y - avgBlind/fNRank << std::endl;

                }
            }else
            {
				const float64 frank = e->pctWillCall(1 - end);
				const float64 fNRank = (frank >= 1) ? 1.0/RAREST_HAND_CHANCE : (1 - frank);


                target << end << "," << f(end) << "," << fd(end,f(end)) << "," << f(end) - avgBlind/fNRank << std::endl;
            }

            target.precision(6);


        }
    #endif

}
;


#endif

