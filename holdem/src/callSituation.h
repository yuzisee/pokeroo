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

#ifndef HOLDEM_ArenaSituations
#define HOLDEM_ArenaSituations

#include "arena.h"
#include "callPredictionFunctions.h"

//#define DEBUG_EXFDEXF

#define ANTI_PRESSURE_FOLDGAIN
#define CONSISTENT_AGG
//#define PURE_BLUFF


#define BLIND_ADJUSTED_FOLD
#define SACRIFICE_COMMITTED
//#define SAME_WILL_LOSE_BLIND
//#define GEOM_COMBO_FOLDPCT





class ExpectedCallD
{//NO ASSIGNMENT OPERATOR
protected:
    const float64 potCommitted;


public:
    const int8 playerID;

#ifdef ANTI_PRESSURE_FOLDGAIN
    const float64 handRarity;
    const float64 meanW;
#endif

    const HoldemArena* const table;
    ExpectedCallD(const int8 id, const HoldemArena* base
#ifdef ANTI_PRESSURE_FOLDGAIN
            , const float64 rankPCT, const float64 meanPCT
#endif
            )
    : potCommitted(0)

    , playerID(id)
    #ifdef ANTI_PRESSURE_FOLDGAIN
    ,handRarity(1-rankPCT), meanW(meanPCT)
    #endif
    , table(base)
    {}

    virtual ~ExpectedCallD();

    virtual float64 foldGain(CallCumulationD* const e);
    virtual float64 foldGain(CallCumulationD* const e, float64 * const foldWaitLength_out);
    virtual float64 foldGain(CallCumulationD* const e, const float64 extra, const float64 facedBet);
    virtual float64 foldGain(CallCumulationD* const e, const float64 extra, const float64 facedBet, float64 * const foldWaitLength_out);
    
    /**
     * The "opportunity cost of folding" formula can also be applied from the perspective of the raiser,
        rather than the folder, in a sort of "opportunity cost of raising" heuristic.
     * The reverse application of the formula is implemented in a function called ``RiskLoss``.
     * This ``RiskLoss`` heuristic reports a loss (negative value) if your bet is large enough for the average opponent to profit (opportunistically) by folding and waiting for a better hand.
     * Since making a small bet does not allow an average opponent to profit via his/her opportunity cost of folding, your ``RiskLoss`` remains zero as long as your bet is suffciently small compared to an average opponent's opportunity.
     * The value returned by the RiskLoss function is used as a deterrent for raising too high.
     */
    virtual float64 RiskLoss(float64 alreadyBet, float64 bankroll, float64 opponents, float64 raiseTo, CallCumulationD * useMean, float64 * out_dPot = 0) const;
    virtual float64 PushGain();

    virtual uint8 OppRaiseOpportunities(int8 oppID) const;

    virtual float64 oppBet() const;
    virtual float64 alreadyBet() const;
    virtual float64 callBet() const;
    virtual float64 minCallFraction(const float64 betSize);
    virtual float64 chipDenom() const;
    virtual float64 allChips() const;
    virtual float64 maxBet() const;
	virtual float64 maxBetAtTable() const;
	virtual float64 maxRaiseAmount() const;
    
    virtual playernumber_t handStrengthOfRound() const; // Same units as handsToBeat() -- it's the number of opponents, not the number in the hand
    virtual playernumber_t handsToBeat() const;
    
    virtual playernumber_t handsDealt() const;
    virtual playernumber_t handsIn() const;
    virtual float64 prevpotChips() const;
    virtual float64 stagnantPot() const;
    virtual float64 betFraction(const float64 betSize) const;
    virtual float64 handBetBase() const; //The B (bankroll) in calculations
	virtual float64 minRaiseTo() const;
	virtual bool inBlinds() const;


        #ifdef DEBUG_EXFDEXF
            void breakdown(float64 dist, std::ostream& target)
            {



                target << "vodd,exf.pct,dexf" << std::endl;
                for( float64 i=0;i<=1;i+=dist)
                {

                    target << i << "," << e->pctWillCall(i) << "," << e->pctWillCallD(i) << std::endl;

                }

            }
        #endif

}
;





#endif // HOLDEM_ArenaSituations

