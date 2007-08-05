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

#include "callPredictionFunctions.h"



class ExactCallD : public virtual ExpectedCallD
{
    private:
        float64 totalexf;
        float64 totaldexf;

        void GenerateRaiseChances(float64 raisebet, float64 raisedFrom, float64 noraiseRank, float64 noraiseRankD, const Player * withP, float64 sig, float64 liveOpp, float64 & out, float64 & outD);
    protected:
        static const float64 UNITIALIZED_QUERY;
        float64 queryinput;

        float64 nearest;
        float64 impliedFactor;

        ExactCallFunctionModel geomFunction;

        int8 noRaiseArraySize;
        float64 *noRaiseChance_A;
        float64 *noRaiseChanceD_A;


        float64 facedOdds_Geom(float64 bankroll, float64 pot, float64 alreadyBet, float64 bet, float64 n, bool bCheckPossible);
        float64 facedOddsND_Geom(float64 bankroll, float64 pot, float64 alreadyBet, float64 bet, float64 dpot, float64 w, float64 n, bool bCheckPossible);
		float64 facedOdds_Algb_step(float64 bankroll, float64 pot, float64 alreadyBet, float64 bet, bool bRank, float64 wGuess);
		float64 facedOdds_Algb(float64 bankroll, float64 pot, float64 alreadyBet, float64 bet,float64 sig, bool bRank);
        float64 facedOddsND_Algb(float64 bankroll, float64 pot, float64 alreadyBet, float64 bet, float64 dpot, float64 w, float64 n, bool bRank);


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

            float64 ActOrReact(float64 callb, float64 lastbet,float64 limit);
            float64 RiskPrice();
}
;

class ExactCallBluffD : public virtual ExactCallD
{
    private:
        //topTwoOfThree returns the average of the top two values {a,b,c} through the 7th parameter.
        //The average of the corresponding values of {a_d, b_d, c_d} are returned by the function.
        float64 topTwoOfThree(float64 a, float64 b, float64 c, float64 a_d, float64 b_d, float64 c_d, float64 & r) const;
        float64 bottomTwoOfThree(float64 a, float64 b, float64 c, float64 a_d, float64 b_d, float64 c_d, float64 & r) const;
        float64 topThreeOfFour(float64 a, float64 b, float64 c, float64 d, float64 a_d, float64 b_d, float64 c_d, float64 d_d, float64 & r) const;
        float64 bottomThreeOfFour(float64 a, float64 b, float64 c, float64 d, float64 a_d, float64 b_d, float64 c_d, float64 d_d, float64 & r) const;
    protected:
        const CallCumulationD* ea;
        float64 allFoldChance;
        float64 allFoldChanceD;

        float64 queryinputbluff;

        void query(const float64 betSize);
    public:
        float64 insuranceDeterrent;

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
                                    ,data,commit), ea(foldData), insuranceDeterrent(0)
                            {
                                queryinputbluff = UNITIALIZED_QUERY;
                            }


                            float64 PushGain();

                            virtual float64 pWin(const float64 betSize);
                            virtual float64 pWinD(const float64 betSize);




}
;


