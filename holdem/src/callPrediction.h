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

#ifndef HOLDEM_CallPredict
#define HOLDEM_CallPredict




//#define DEBUG_CALLPRED_FUNCTION
#undef DEBUG_TRACE_PWIN
#undef DEBUG_TRACE_DEXF

#include "callPredictionFunctions.h"
#include "callSituation.h"

#if defined(DEBUG_TRACE_PWIN) || defined(DEBUG_TRACE_DEXF)
#define DEBUG_TRACE_EXACTCALL
#else
#undef DEBUG_TRACE_EXACTCALL
#endif

#ifdef DEBUG_TRACE_EXACTCALL
#include <iostream>
#endif

struct ChipPositionState
{
    ChipPositionState(float64 stack, float64 tablepot, float64 sofar, float64 commit)
       : bankroll(stack), pot(tablepot), alreadyBet(sofar), alreadyContributed(commit)
    {}

    float64 bankroll;
    float64 pot;
    float64 alreadyBet;
    float64 alreadyContributed;

}
;


///==============================
///   Pr{call} and Pr{raiseby}
///==============================
class ExactCallD
{//NO ASSIGNMENT OPERATOR
    private:

        float64 totalexf;
        float64 totaldexf;





    protected:
        static const float64 UNITIALIZED_QUERY;
        float64 queryinput;
		int32 querycallSteps;

        float64 nearest;
        float64 impliedFactor;



        int32 noRaiseArraySize;
        float64 *noRaiseChance_A;
        float64 *noRaiseChanceD_A;


        float64 facedOdds_call_Geom(const ChipPositionState & cps, float64 humanbet, float64 n,  CallCumulationD * useMean);
        float64 dfacedOdds_dbetSize_Geom(const ChipPositionState & cps, float64 humanbet, float64 dpot, float64 w, float64 n,  CallCumulationD * useMean);


        float64 facedOdds_raise_Geom(const ChipPositionState & cps, float64 startingPoint, float64 incrbet_forraise, float64 fold_bet, float64 n, bool bCheckPossible, bool bMyWouldCall, CallCumulationD * useMean);
        float64 dfacedOdds_dpot_GeomDEXF(const ChipPositionState & cps, float64 incrbet_forraise, float64 fold_bet, float64 w, float64 opponents, float64 dexf, bool bCheckPossible, bool bMyWouldCall, CallCumulationD * useMean);

        float64 facedOdds_Algb(const ChipPositionState & cps, float64 bet,float64 opponents,  CallCumulationD * useMean);
        float64 facedOddsND_Algb(const ChipPositionState & cps, float64 bet, float64 dpot, float64 w, float64 n);


        void query(const float64 betSize, const int32 callSteps);
    public:

    // By default, startingPoint == 0.0
    // When using this function for the purposes of nextNoRaise_A, you'll want to start at the previous value to avoid rounding errors.
    static float64 facedOdds_raise_Geom_forTest(float64 startingPoint, float64 denom, float64 raiseto, float64 riskLoss, float64 avgBlind, const ChipPositionState & cps, float64 fold_bet, float64 opponents, bool bCheckPossible, bool bMyWouldCall, CallCumulationD * useMean);


        CallCumulationD * const ed;
        ExpectedCallD * const tableinfo;
#ifdef DEBUG_TRACE_EXACTCALL
		std::ostream * traceOut;
#endif

        ExactCallD(ExpectedCallD * const tbase, CallCumulationD* data)
        :
        impliedFactor(1)
        , noRaiseArraySize(0),noRaiseChance_A(0),noRaiseChanceD_A(0)
        , ed(data),tableinfo(tbase)
#ifdef DEBUG_TRACE_EXACTCALL
					,traceOut(0)
#endif
            {
                queryinput = UNITIALIZED_QUERY;
				querycallSteps = -1;
            }

            virtual ~ExactCallD();


            virtual float64 exf(const float64 betSize);
            virtual float64 dexf(const float64 betSize);

            // pRaise() is the probability of being raised by RaiseAmount().
            // betSize is the bet you're considering, step is an iterator (because RasieAmount is a gradient but still discrete, so we only have hard pRaise() for each particular RaiseAmount().
            // If you want you could interpolate in between, but we typically just average the outcomes, since we're taking an expectation over all raise amounts that we might face.
            // callSteps is an index that indicates: "all iterator values (of step) starting from this one and higher, are raises that I would fold against)
            // In other worst, callSteps it the smallest RaiseAmount where we know we would just fold to it.
            virtual float64 RaiseAmount(const float64 betSize, int32 step);
			virtual float64 pRaise(const float64 betSize, const int32 step, const int32 callSteps  );
			virtual float64 pRaiseD(const float64 betSize, const int32 step, const int32 callSteps );

            virtual void SetImpliedFactor(const float64 bonus);



            virtual float64 FoldGain(){ return tableinfo->foldGain(ed);}
            virtual float64 FoldGain(float64 extra, float64 facedbet){ return tableinfo->foldGain(ed,extra,facedbet);}


}
;

///======================
///   Pr{opponentFold}
///======================
class ExactCallBluffD : public virtual ExactCallD
{//NO ASSIGNMENT OPERATOR
    private:
        //topTwoOfThree returns the average of the top two values {a,b,c} through the 7th parameter.
        //The average of the corresponding values of {a_d, b_d, c_d} are returned by the function.
        float64 topTwoOfThree(float64 a, float64 b, float64 c, float64 a_d, float64 b_d, float64 c_d, float64 & r) const;
        float64 bottomTwoOfThree(float64 a, float64 b, float64 c, float64 a_d, float64 b_d, float64 c_d, float64 & r) const;
        float64 topThreeOfFour(float64 a, float64 b, float64 c, float64 d, float64 a_d, float64 b_d, float64 c_d, float64 d_d, float64 & r) const;
        float64 bottomThreeOfFour(float64 a, float64 b, float64 c, float64 d, float64 a_d, float64 b_d, float64 c_d, float64 d_d, float64 & r) const;
    protected:

        float64 allFoldChance;
        float64 allFoldChanceD;

        float64 queryinputbluff;

        void query(const float64 betSize);
        CallCumulationD * const ef;
    public:
        float64 insuranceDeterrent;

        ExactCallBluffD( ExpectedCallD * const tbase

                        , CallCumulationD* data, CallCumulationD* foldData)
    : ExactCallD(tbase,data), ef(foldData), insuranceDeterrent(0)
                            {
                                queryinputbluff = UNITIALIZED_QUERY;
                            }



                            /** 
                             *  pWin()
                             *
                             *   Parameters:
                             *     betSize:
                             *       The size of bet you would make
                             *
                             *   Return value:
                             *     The probability that everyone folds to your bet.
                             */
                            virtual float64 pWin(const float64 betSize);

                            virtual float64 pWinD(const float64 betSize);

            float64 RiskPrice() const;

            ~ExactCallBluffD();


}
;


#endif
