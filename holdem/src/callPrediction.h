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

//#define DEBUG_CALLPRED_FUNCTION

#include "callSituation.h"
#include "functionbase.h"


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

class ExactCallD : public virtual ExpectedCallD
{
    private:
        float64 totalexf;
        float64 totaldexf;
    protected:
        static const float64 UNITIALIZED_QUERY;
        float64 queryinput;


        float64 impliedFactor;

        ExactCallFunctionModel geomFunction;

        int8 noRaiseArraySize;
        float64 *noRaiseChance_A;
        float64 *noRaiseChanceD_A;


        float64 facedOdds_Geom(float64 bankroll, float64 pot, float64 alreadyBet, float64 bet, float64 n, bool bCheckPossible);
        float64 facedOddsND_Geom(float64 bankroll, float64 pot, float64 alreadyBet, float64 bet, float64 dpot, float64 w, float64 n, bool bCheckPossible);
		float64 facedOdds_Algb_step(float64 bankroll, float64 pot, float64 alreadyBet, float64 bet, float64 wGuess = 0.75);
		float64 facedOdds_Algb(float64 bankroll, float64 pot, float64 alreadyBet, float64 bet);
        float64 facedOddsND_Algb(float64 bankroll, float64 pot, float64 alreadyBet, float64 bet, float64 dpot, float64 w, float64 n);



        void query(const float64 betSize);
    public:
        ExactCallD(const int8 id, const HoldemArena* base
#ifdef ANTI_PRESSURE_FOLDGAIN
                , const float64 rankPCT
#endif
                        , const CallCumulationD* data, const float64 commit = 0)
    : ExpectedCallD(id,base
#ifdef ANTI_PRESSURE_FOLDGAIN
            ,rankPCT
#endif
                    ,data,commit), impliedFactor(1), geomFunction(0.5/RAREST_HAND_CHANCE,data),noRaiseArraySize(0),noRaiseChance_A(0),noRaiseChanceD_A(0)
            {
                queryinput = UNITIALIZED_QUERY;
            }

            ~ExactCallD();

            virtual float64 exf(const float64 betSize);
            virtual float64 dexf(const float64 betSize);

			virtual float64 RaiseAmount(const float64 betSize, int32 step);
			virtual float64 pRaise(const float64 betSize, const int32 step );
			virtual float64 pRaiseD(const float64 betSize, const int32 step );

            virtual void SetImpliedFactor(const float64 bonus);
}
;

class ExactCallBluffD : public virtual ExactCallD
{
    private:
        //topTwoOfThree returns the average of the top two values {a,b,c} through the 7th parameter.
        //The average of the corresponding values of {a_d, b_d, c_d} are returned by the function.
        float64 topTwoOfThree(float64 a, float64 b, float64 c, float64 a_d, float64 b_d, float64 c_d, float64 & r) const;
    protected:
        const CallCumulationD* ea;
        float64 allFoldChance;
        float64 allFoldChanceD;

        float64 queryinputbluff;

        void query(const float64 betSize);
    public:
        float64 insuranceDeterrent;
        float64 minimaxAdjustment;

        ExactCallBluffD(const int8 id, const HoldemArena* base
#ifdef ANTI_PRESSURE_FOLDGAIN
                , const float64 rankPCT
#endif
                        , const CallCumulationD* data, const CallCumulationD* foldData, const float64 commit = 0)
    : ExpectedCallD(id,base
#ifdef ANTI_PRESSURE_FOLDGAIN
            ,rankPCT
#endif
                    ,data,commit),ExactCallD(id,base
#ifdef ANTI_PRESSURE_FOLDGAIN
                            ,rankPCT
#endif
                                    ,data,commit), ea(foldData), insuranceDeterrent(0), minimaxAdjustment(0)
                            {
                                queryinputbluff = UNITIALIZED_QUERY;
                            }


                            float64 PushGain();

                            virtual float64 pWin(const float64 betSize);
                            virtual float64 pWinD(const float64 betSize);


}
;


