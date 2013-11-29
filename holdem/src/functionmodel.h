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

#ifndef HOLDEM_GainModels
#define HOLDEM_GainModels

#include "functionbase.h"
#include "inferentials.h"
#include "callPrediction.h"
#include <math.h>



#define DEFAULT_EPS_STEP 0.001

class HoldemFunctionModel : public virtual ScalarFunctionModel
{
    protected:
    ExpectedCallD * const estat;
    public:

    HoldemFunctionModel(float64 step,ExpectedCallD *c) : ScalarFunctionModel(step),estat(c) {};

    // What would you best bet be, if you were to bet? (This assumes you aren't going to check or fold -- we handle those cases outside of this function)
    virtual float64 FindBestBet();
    virtual float64 FindFoldBet(const float64);

    #ifdef DEBUG_GAIN
        void breakdown(float64 points, std::ostream& target, float64 start=0, float64 end=1)
        {
            target.precision(17);

            float64 dist;
            if( points > 0 ) dist = (end-start)/points;


            target << "x,gain,dgain,wch%,dwch%" << std::endl;
            if( points > 0 && dist > 0 )
            {
                for( float64 i=start;i<=end;i+=dist)
                {

                    float64 y = f(i);
					float64 dy = fd(i,y);
					float64 wch = e->exf(i);
					float64 dwchp =  e->dexf(i);

                    target << i << "," << y << "," << dy << "," << e->betFraction(wch)  << "," <<  dwchp << std::endl;

                }
            }else
            {
                target << end << "," << f(end) << "," << fd(end,f(end)) << "," << e->betFraction(e->exf(end))  << "," <<  e->dexf(end) << std::endl;
            }

            target.precision(6);


        }
    #endif

}
;

// Evaluate gain at showdown of any price.
// This does NOT evaluate relative to foldGain
class GainModel : public virtual HoldemFunctionModel {
public:


    virtual ~GainModel() {}

protected:

    GainModel(float64 step,ExpectedCallD *c)
    :
    ScalarFunctionModel(step)
    ,
    HoldemFunctionModel(step, c)
    {

        if( quantum == 0 ) quantum = 1;

    }

    virtual float64 g(float64) = 0;
    virtual float64 gd(float64, const float64) = 0;

}
;

class ICombinedStatResults {
public:
    virtual ~ICombinedStatResults() {}

    virtual playernumber_t splitOpponents() const = 0;
    const virtual StatResult & ViewShape(float64 betSize) = 0; // per-player outcome: wins and splits are used to calculate split possibilities
    virtual float64 getLoseProb(float64 betSize) = 0;
	virtual float64 getWinProb(float64 betSize) = 0;

    virtual float64 get_d_LoseProb_dbetSize(float64 betSize) = 0;
    virtual float64 get_d_WinProb_dbetSize(float64 betSize) = 0;
}
;

struct NetStatResult {
    StatResult fShape;
    float64 fLoseProb;
	float64 fOutrightWinProb;
}
;

/**
 * // TODO: Do we ever use rank here in the non heads-up case?
 * Determine your chance to win by counting how many times your opponent can wait before calling you, assuming they know what you have.
 *
 * If you use this exclusively as your winPct, then the bot should bet small enough that it can be called by the distribution of hands it expects.
 */
class CombinedStatResultsPessimistic : public virtual ICombinedStatResults {
public:
    CombinedStatResultsPessimistic(OpponentHandOpportunity & opponentHandOpportunity, CoreProbabilities & core);

    virtual ~CombinedStatResultsPessimistic() {}

    virtual playernumber_t splitOpponents() const { return fSplitOpponents; }

    // per-player outcome: wins and splits are used to calculate split possibilities across NumPlayersInHand()
    const virtual StatResult & ViewShape(float64 betSize) { query(betSize); return fNet.fShape; }

    virtual float64 getLoseProb(float64 betSize) { query(betSize); return fNet.fLoseProb; }
	virtual float64 getWinProb(float64 betSize) { query(betSize); return fNet.fOutrightWinProb; }

    virtual float64 get_d_LoseProb_dbetSize(float64 betSize) { query(betSize); return f_d_LoseProb_dbetSize; }
    virtual float64 get_d_WinProb_dbetSize(float64 betSize) { query(betSize); return f_d_WinProb_dbetSize; }

    virtual float64 getHandsToBeat(float64 betSize) { query(betSize); return fHandsToBeat; }
    
private:
    void query(float64 betSize);
    
    // query inputs
    float64 fLastBetSize;

    // query outputs
    struct NetStatResult fNet;
    float64 f_d_LoseProb_dbetSize;
    float64 f_d_WinProb_dbetSize;

    float64 fHandsToBeat;

    // Count the number of possible opponents, including hypothetical "fold and come back stronger" hands.
    // This accounts for the fact that if you make an overbet in a particular situation, opponents will find only to return to this same situation in the future but with a better hand.
    OpponentHandOpportunity & fOpposingHands; // All OPPONENT odds against ME.
    CallCumulationD * const fFoldCumu; // OPPOSING hands (as faring against me)

    const playernumber_t fSplitOpponents;
}
;


// Use rank for multi-handed, mean for heads-up
class PureStatResultGeom : public virtual ICombinedStatResults {
private:
    const playernumber_t fDifficultyOpponents;
    const playernumber_t fShowdownOpponents;

    const struct NetStatResult fNet;

public:
    PureStatResultGeom(const StatResult mean, const StatResult rank, CallCumulationD &foldcumu, const ExpectedCallD &tableinfo);
    virtual ~PureStatResultGeom() {}

    playernumber_t splitOpponents() const override final { return fShowdownOpponents; }
    
    const StatResult & ViewShape(float64 betSize) override final { return fNet.fShape; } // per-player outcome: wins and splits are used to calculate split possibilities
    float64 getLoseProb(float64 betSize) override final { return fNet.fLoseProb; }
	float64 getWinProb(float64 betSize) override final { return fNet.fOutrightWinProb; }

    float64 get_d_LoseProb_dbetSize(float64 betSize) override final { return 0.0; }
    float64 get_d_WinProb_dbetSize(float64 betSize) override final { return 0.0; }
}
;

/**
 * Convert a particular StatResult into the odds of winning at the table.
 * Supported features:
 *  + This class will automatically determine the number of hands at the table you would have to beat in order to win, via the ExactCallD's tableInfo
 *  + Pass in two different StatResult objects and we'll use the geometric mean of them instead.
 *
 * DEPRECATED( Use PureStatResultGeom instead. It's simpler, more robust (e.g. fewer knobs), and should be closer to optimal. )
 */
class CombinedStatResultsGeom : public virtual ICombinedStatResults {
private:
    void combineStatResults(const StatResult s_acted, const StatResult s_nonacted, bool bConvertToNet);

    // Adjust p_cl and p_cw slightly according to split probabilities.
    // Prior to calling forceRenormalize, you just want the relative weight of p_cl and p_cw to be accurate.
    void forceRenormalize();

    
	StatResult shape;
	float64 p_cl;
	float64 p_cw;


    //Who can you split with?
	const uint8 e_battle;

    

public:
    //Floating point version of totalEnemy (which is handsToBeat), but adjustable by playerStrategy based on expectations
	const float64 f_battle;


    
    CombinedStatResultsGeom(const StatResult s_acted, const StatResult s_nonacted, bool bConvertToNet, ExactCallD & c)
    : f_battle(c.tableinfo->handStrengthOfRound())
    , e_battle(c.tableinfo->handsIn()-1)
    {
        combineStatResults(s_acted,s_nonacted, bConvertToNet);
    }

    virtual playernumber_t splitOpponents() const { return e_battle; }
    const virtual StatResult & ViewShape(float64 betSize) { return shape; }
    const StatResult & ViewShape() { return shape; }

    /**
     * ComposeBreakdown()
     *
     *  Discussion:
     *    Helper function for constructing a StatResult object.
     *
     *  Return Value:
     *    A StatResult with .wins .splits .loss & .pct populated based on a wl ratio and a raw expectation (percentage)
     *
     *  Parameters:
     *    pct:
     *      Raw expected value (outcome) as a percentage.
     *      Examples:
     *        If you have .wins=50%, .splits=0%, and .loss=50%, you would have pct=0.5
     *        If you have .wins=10%, .splits=40%, and .loss=10%, you would have pct=0.5 as well.
     *      Thus, we also need the wl parameter to disambiguate.
     *    wl:
     *      Ratio "wins / (wins + loss)", or 0.0 if all-split
     */
	static StatResult ComposeBreakdown(const float64 pct, const float64 wl);


    /*
     * Raise s to s^f_battle, where s is the weighted geomean of b1 (weight x1) and b2 (weight x2)
     */
    static float64 cleangeomeanpow(float64 b1, float64 x1, float64 b2, float64 x2, float64 f_battle);

    float64 getLoseProb(float64 betSize) override final {
        return p_cl;
    }

    float64 getWinProb(float64 betSize) override final {
        return p_cw;
    }

    float64 get_d_LoseProb_dbetSize(float64 betSize) override final {
        return 0.0;
    }

    float64 get_d_WinProb_dbetSize(float64 betSize) override final {
        return 0.0;
    }
}
;

class GainModelGeom : public virtual GainModel
{

protected:

	IExf & espec;
    
    ICombinedStatResults & fOutcome; // predict the outcome of a showdown



    virtual float64 g(float64) override final;
    virtual float64 gd(float64, const float64) override final;
    
	public:

    static float64 h(float64 betFraction, float64 betSize, float64 exf, float64 f_pot, ICombinedStatResults & fOutcome);
    static float64 hdx(float64 betFraction, float64 betSize, float64 exf, float64 dexf, float64 f_pot, float64 dx, ICombinedStatResults & fOutcome, float64 h);

    /**
     *  Parameters:
     *    convertToNet:
     *      Set this to true if the StatResult objects provided are the odds to beat one person.
     *      If this is false, we will assume the StatResult objects provided are the odds of winning the table.
     */
    GainModelGeom(ICombinedStatResults & outcome, ExactCallD & c)
		:
    ScalarFunctionModel(c.tableinfo->chipDenom())
    ,
    HoldemFunctionModel(c.tableinfo->chipDenom(), c.tableinfo)
    ,
    GainModel(c.tableinfo->chipDenom(),c.tableinfo)
        , espec(c)
        , fOutcome(outcome)
    {}


    virtual ~GainModelGeom();

	virtual float64 f(const float64);
    virtual float64 fd(const float64, const float64);

    #ifdef DEBUG_GAIN
        void breakdown(float64 points, std::ostream& target, float64 start=0, float64 end=1)
        {

            float64 dist;
            if( points > 0 ) dist = (end-start)/points;

            target << "x,gain,dgain,exf.chips,dexf,win,lose" << std::endl;
            if( points > 0 && dist > 0 )
            {
                for( float64 i=start;i<=end;i+=dist)
                {
                    float64 y = f(i);
                    //float64 exf = e->pctWillCall(vodd);
                    //float64 dexf = e->pctWillCallD(vodd) * f_pot / (2*i+f_pot) /(2*i+f_pot);
                    float64 exf = e->exf(i);
                    float64 dexf = e->dexf(i);

                    target << i << "," << y << "," << fd(i,y) << "," << exf << "," << dexf << "," << p_cw << "," << p_cl << std::endl;


                }
            }else
            {
                target << end << "," << f(end);
                target << "," << fd(end,f(end));
                target << "," << e->exf(end);
                target << "," << e->dexf(end);
                target << "," << p_cw << "," << p_cl <<std::endl;
            }

        }

        void breakdownC(float64 points, std::ostream& target, float64 start=0, float64 end=1)
        {
            float64 dist = (end-start)/points;

            target << "i,exf.chips,dexf" << std::endl;
            for( float64 i=start;i<end;i+=dist)
            {
                //float64 exf = e->pctWillCall(vodd);
                //float64 dexf = e->pctWillCallD(vodd);
                float64 exf = e->exf(i);
                float64 dexf = e->dexf(i);
                target << i << "," << exf << "," << dexf << std::endl;

            }


        }
       /* void breakdownE(float64 points, std::ostream& target)
        {
            e->breakdown(points,target);

        }*/
    #endif
}
;

/**
 * Algebraic mean of two StatResult objects.
 */
class GainModelNoRisk : public virtual GainModel
{
    protected:
    IExf & espec;
    ICombinedStatResults & fOutcome; // predict the outcome of a showdown
    
        virtual float64 g(float64);
        virtual float64 gd(float64,const float64);
    public:


    static float64 h(float64 betFraction, float64 betSize, float64 exf, float64 f_pot, ICombinedStatResults & fOutcome);
    static float64 hdx(float64 betFraction, float64 betSize, float64 exf, float64 dexf, ICombinedStatResults & fOutcome, float64 h);


    /**
     *  Parameters:
     *    convertToNet:
     *      Set this to true if the StatResult objects provided are the odds to beat one person.
     *      If this is false, we will assume the StatResult objects provided are the odds of winning the table.
     */
	GainModelNoRisk(ICombinedStatResults & outcome, ExactCallD & c)
    : ScalarFunctionModel(c.tableinfo->chipDenom()),HoldemFunctionModel(c.tableinfo->chipDenom(),c.tableinfo)
    ,
    GainModel(c.tableinfo->chipDenom(),c.tableinfo)
    ,espec(c)
    ,fOutcome(outcome){}
	virtual ~GainModelNoRisk();

	virtual float64 f(const float64) override final;
    virtual float64 fd(const float64, const float64) override final;
}
;


class SlidingPairFunction : public virtual HoldemFunctionModel
{//NO ASSIGNMENT OPERATOR
    protected:
        virtual void query(float64 x);
        const float64 slider;
        float64 last_x;
        float64 y;
        float64 dy;
        ScalarFunctionModel *left;
        ScalarFunctionModel *right;
    public:
	SlidingPairFunction(ScalarFunctionModel *f_left, ScalarFunctionModel *f_right, const float64 scale,ExpectedCallD *c)
	: ScalarFunctionModel(c->chipDenom()),HoldemFunctionModel(f_left->quantum*(1-scale)+f_right->quantum*scale, c), slider(scale), left(f_left), right(f_right){
	    query(0);
	    }
	virtual ~SlidingPairFunction(){}

	virtual float64 f(const float64);
    virtual float64 fd(const float64, const float64);

}
;




#endif // HOLDEM_GainModels



