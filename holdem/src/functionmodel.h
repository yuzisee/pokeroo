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
#include <float.h>



#define DEFAULT_EPS_STEP 0.001

class HoldemFunctionModel : public virtual ScalarFunctionModel
{
    protected:
    ExpectedCallD * const estat;
    public:

    HoldemFunctionModel(float64 step,ExpectedCallD *c) : ScalarFunctionModel(step),estat(c) {};

    virtual float64 FindBestBet();
    virtual float64 FindFoldBet(const float64);

    virtual float64 GetFoldGain(CallCumulationD* const e, float64 * const foldWaitLength_out);


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



class GainModel : public virtual HoldemFunctionModel
{

	protected:
	ExactCallD & espec;
	StatResult shape;
	float64 f_battle;
	uint8 e_battle;
	float64 p_cl;
	float64 p_cw;


        virtual float64 g(float64);
        virtual float64 gd(float64, const float64);

	public:

        static inline float64 cleanpow(float64 b, float64 x)
        {
            if( b < DBL_EPSILON ) return 0;
            //if( b > 1 ) return 1;
            return pow(b,x);
        }



        const StatResult & ViewShape() { return shape; }

	static StatResult ComposeBreakdown(const float64 pct, const float64 wl);
	GainModel(const StatResult s,ExactCallD & c)
		: ScalarFunctionModel(c.tableinfo->chipDenom()),HoldemFunctionModel(c.tableinfo->chipDenom(),c.tableinfo),espec(c),shape(s)
		{

		    const int8 totalEnemy = c.tableinfo->handsToBeat(); //To beat

		    f_battle = c.callingPlayers(); //Floating point version of totalEnemy, but adjustable by playerStrategy based on expectations

		    e_battle = c.tableinfo->handsIn()-1; //Who can you split with?

		    if( quantum == 0 ) quantum = 1;

			if( shape.splits == 1 || (shape.loss + shape.wins == 0) )
			{
				p_cl = 0;
				p_cw = 0;
				shape.wins = 1; //You need wins to split, and shape is only used to split so this okay
			}else
			{

			///Use f_battle instead of e_battle, convert to equivelant totalEnemy
				p_cl =  1 - cleanpow(1 - shape.loss,f_battle);
				p_cw = cleanpow(shape.wins,f_battle);

                #ifdef DEBUG_TRACE_SEARCH
                std::cout << "\t\t\t(1 -  " << shape.loss  << ")^" << f_battle << "   =   p_cl "  << p_cl << std::endl;
                std::cout << "\t\t\t" << shape.wins  << "^fbattle   =   p_cw "  << p_cw << std::endl;
                #endif

				const float64 newTotal = p_cl + p_cw;

				shape.wins = cleanpow(p_cw,1.0/totalEnemy);
				shape.loss = 1 - cleanpow(1 - p_cl,1.0/totalEnemy);
			///Normalize, total possibility must add up to 1
                const float64 hundredTotal = shape.wins + shape.loss + shape.splits;
                shape = shape * (1.0/hundredTotal);
                shape.genPCT();
            ///Normalize, total possibilities must add up to 1 (certain splits are impossible)
                float64 splitTotal = 0;
                for( int8 i=1;i<=e_battle;++i )
                {//Split with i
                    splitTotal += HoldemUtil::nchoosep<float64>(e_battle,i)*pow(shape.wins,e_battle-i)*pow(shape.splits,i);
                }

				p_cl *= (1-splitTotal)/newTotal;
				p_cw *= (1-splitTotal)/newTotal;

                #ifdef DEBUG_TRACE_SEARCH
                std::cout << "\t\t\t\t" << p_cl << " after totalEnemy is  p_cl "  << std::endl;
                std::cout << "\t\t\t\t" << p_cw << " after totalEnemy is  p_cw "  << std::endl;
                #endif

			}
		}

///When the two-StatResult constructor is used, the .repeated properties represent weights
	GainModel(const StatResult s, const StatResult opportunity,ExactCallD & c)
		: ScalarFunctionModel(c.tableinfo->chipDenom()),HoldemFunctionModel(c.tableinfo->chipDenom(),c.tableinfo),espec(c),shape(s)
		{

		    const int8 totalEnemy = c.tableinfo->handsToBeat();

		    f_battle = c.callingPlayers();

		    e_battle = c.tableinfo->handsIn()-1;

		    if( quantum == 0 ) quantum = 1;

			if( shape.splits == 1 || (shape.loss + shape.wins == 0) )
			{
				p_cl = 0;
				p_cw = 0;
				shape.wins = 1; //You need wins to split, and shape is only used to split so this okay
			}else
			{

                shape.splits = s.splits * s.repeated + opportunity.splits * (1 - s.repeated);

			///Use f_battle instead of e_battle, convert to equivelant totalEnemy
				p_cl =  1 - cleanpow(1 - shape.loss,f_battle);
				p_cw = cleanpow(shape.wins,f_battle);

                const float64 p_cl_draw = 1 - cleanpow(1 - opportunity.loss,e_battle);
                const float64 p_cw_draw = cleanpow(opportunity.wins,e_battle);

                if( p_cw_draw > p_cw && p_cl_draw < p_cl )
                {
                    p_cl = p_cl_draw;
                    p_cw = p_cw_draw;
                    shape = opportunity;
                }else
                {
                    p_cl = p_cl * s.repeated + p_cl_draw * (1 - s.repeated);
                    p_cw = p_cw * s.repeated + p_cw_draw * (1 - s.repeated);
                }

				const float64 newTotal = p_cl + p_cw;

				shape.wins = cleanpow(p_cw,1.0/totalEnemy);
				shape.loss = 1 - cleanpow(1 - p_cl,1.0/totalEnemy);
			///Normalize, total possibility must add up to 1
                const float64 hundredTotal = shape.wins + shape.loss;
                shape.wins *= (1-shape.splits)/hundredTotal;
                shape.loss *= (1-shape.splits)/hundredTotal;
                shape.genPCT();
            ///Normalize, total possibilities must add up to 1 (certain splits are impossible)
                float64 splitTotal = 0;
                for( int8 i=1;i<=e_battle;++i )
                {//Split with i
                    splitTotal += HoldemUtil::nchoosep<float64>(e_battle,i)*pow(shape.wins,e_battle-i)*pow(shape.splits,i);
                }

				p_cl *= (1-splitTotal)/newTotal;
				p_cw *= (1-splitTotal)/newTotal;

			}
		}


    virtual ~GainModel();

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

class GainModelNoRisk : public virtual GainModel
{
    protected:
        virtual float64 g(float64);
        virtual float64 gd(float64,const float64);
    public:
	GainModelNoRisk(const StatResult s,ExactCallD & c) : ScalarFunctionModel(c.tableinfo->chipDenom()),HoldemFunctionModel(c.tableinfo->chipDenom(),c.tableinfo),GainModel(s,c){}
	GainModelNoRisk(const StatResult s,const StatResult sk,ExactCallD & c) : ScalarFunctionModel(c.tableinfo->chipDenom()),HoldemFunctionModel(c.tableinfo->chipDenom(),c.tableinfo),GainModel(s,sk,c){}
	virtual ~GainModelNoRisk();

	virtual float64 f(const float64);
    virtual float64 fd(const float64, const float64);
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



