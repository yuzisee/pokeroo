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

#ifndef HOLDEM_ScalarFunctions
#define HOLDEM_ScalarFunctions


#define DEBUG_GAIN




#define DEFAULT_EPS_STEP 0.001

#include "inferentials.h"
#include "arenaSituation.h"

class ScalarFunctionModel
{
    protected:
        float64 trisectionStep(float64,float64,float64,float64,float64,float64) const;
        float64 searchStep(float64,float64,float64,float64,float64,float64) const;
        float64 quadraticStep(float64,float64,float64,float64,float64,float64) const;
        float64 newtonStep(float64,float64) const;
        float64 bisectionStep(float64,float64) const;
		virtual float64 FindTurningPoint(float64,float64,float64,float64,float64,float64,float64) const;
    public:
    float64 quantum;
    ScalarFunctionModel(float64 step) : quantum(step){};
    virtual float64 f(const float64) const = 0;
    virtual float64 fd(const float64, const float64) const = 0;
	virtual float64 FindMax(float64,float64) const;
	virtual float64 FindMin(float64,float64) const;
	virtual float64 FindZero(float64,float64) const;
    virtual ~ScalarFunctionModel();


}
;
/*
class DummyFunctionModel : public virtual ScalarFunctionModel
{
	public:
	DummyFunctionModel(float64 step) : ScalarFunctionModel(step){};
    virtual float64 f(const float64) const;
    virtual float64 fd(const float64, const float64) const;
}
;
*/
class GainModel : public virtual ScalarFunctionModel
{
	protected:
	StatResult shape;
	ExpectedCallD *e;
	uint8 e_battle;
	public:
	static StatResult ComposeBreakdown(const float64 pct, const float64 wl);
	GainModel(const StatResult s,ExpectedCallD *c)
		: ScalarFunctionModel(c->chipDenom()),shape(s),e(c),e_battle(c->handsDealt()-1)
		{
//		    const float64 t = c->chipDenom();
		    if( quantum == 0 ) quantum = 1;
		}

    virtual ~GainModel();

    virtual float64 FindBestBet() const;

	virtual float64 f(const float64) const;
    virtual float64 fd(const float64, const float64) const;

    #ifdef DEBUG_GAIN
        void breakdown(float64 points, std::ostream& target, float64 start=0, float64 end=1)
        {

            float64 dist = (end-start)/points;

            target << "x,gain,dgain,vodd,exf,dexf" << std::endl;
            for( float64 i=start;i<end;i+=dist)
            {
                float64 vodd;
                if( i == 0 )
                {
                    vodd = 0;
                }else
                {
                    vodd = i/(2*i+e->deadpotFraction());
                }
                float64 y = f(i);
                //float64 exf = e->pctWillCall(vodd);
                //float64 dexf = e->pctWillCallD(vodd) * f_pot / (2*i+f_pot) /(2*i+f_pot);
                float64 exf = e->exf(i);
                float64 dexf = e->dexf(i);

                target << i << "," << f(i) << "," << fd(i,y) << "," << vodd << "," << exf << "," << dexf << std::endl;

            }


        }

        void breakdownC(float64 points, std::ostream& target, float64 start=0, float64 end=1)
        {
            float64 dist = (end-start)/points;

            target << "i,exf,dexf" << std::endl;
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

#endif



