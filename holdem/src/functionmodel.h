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

class ScalarFunctionModel
{
    protected:
        float64 trisectionStep(float64,float64,float64,float64,float64,float64);
        float64 searchStep(float64,float64,float64,float64,float64,float64);
        float64 quadraticStep(float64,float64,float64,float64,float64,float64);
        float64 newtonStep(float64,float64);
        float64 bisectionStep(float64,float64);
		virtual float64 FindTurningPoint(float64,float64,float64,float64,float64,float64,float64);
    public:
    float64 quantum;
    ScalarFunctionModel(float64 step) : quantum(step){};
    virtual float64 f(const float64) const = 0;
    virtual float64 fd(const float64, const float64) const = 0;
	virtual float64 FindMax(float64,float64);
	virtual float64 FindMin(float64,float64);
	virtual float64 FindZero(float64,float64);
    virtual ~ScalarFunctionModel();


}
;

class DummyFunctionModel : public virtual ScalarFunctionModel
{
	public:
	DummyFunctionModel(float64 step) : ScalarFunctionModel(step){};
    virtual float64 f(const float64) const;
    virtual float64 fd(const float64, const float64) const;
}
;

class GainModel : public virtual ScalarFunctionModel
{
	protected:
	StatResult shape;
	CallCumulationD *e;
	float64 f_pot;
	float64 e_called; //REMEMBER e_call is the number of OPPONENTS, not the number of players.
	float64 e_tocall; //REMEMBER e_call is the number of OPPONENTS, not the number of players.
	uint8 e_battle;
	public:
	static StatResult ComposeBreakdown(const float64 pct, const float64 wl);
	GainModel(const StatResult s,CallCumulationD *c, float64 pot, float64 called, float64 tocall, uint8 oppTable, const float64 step)
		: ScalarFunctionModel(step),shape(s),e(c), f_pot(pot),e_called(called),e_tocall(tocall),e_battle(oppTable)
		{};
	virtual float64 f(const float64) const;
    virtual float64 fd(const float64, const float64) const;


    #ifdef DEBUG_GAIN
        void breakdown(float points, std::ostream& target, float start=0, float end=1)
        {

            float dist = (end-start)/points;

            target << "x,gain,dgain,vodd,exf,dexf" << std::endl;
            for( float i=start;i<end;i+=dist)
            {
                float vodd = i/(2*i+f_pot);
                float y = f(i);
                float exf = e->pctWillCall(vodd);
                float64 dexf = e->pctWillCallD(vodd) * f_pot / (2*i+f_pot) /(2*i+f_pot);
                target << i << "," << f(i) << "," << fd(i,y) << "," << vodd << "," << exf << "," << dexf << std::endl;

            }


        }
        void breakdownC(float points, std::ostream& target, float start=0, float end=1)
        {
            float dist = (end-start)/points;

            target << "i,exf,dexf" << std::endl;
            for( float vodd=start;vodd<end;vodd+=dist)
            {
                float exf = e->pctWillCall(vodd);
                float64 dexf = e->pctWillCallD(vodd);
                target << vodd << "," << exf << "," << dexf << std::endl;

            }


        }
       /* void breakdownE(float points, std::ostream& target)
        {
            e->breakdown(points,target);

        }*/
    #endif
}
;

#endif



