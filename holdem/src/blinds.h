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


#ifndef HOLDEM_Blinds
#define HOLDEM_Blinds

#include "portability.h"
#include "debug_flags.h"

class BlindStructure
{
	public:
        float64 myBigBlind;
        float64 mySmallBlind;

            BlindStructure(float64 small, float64 big)
    : myBigBlind(big), mySmallBlind(small) {};
            virtual ~BlindStructure();
            virtual bool PlayerEliminated(){return false;};
            virtual bool HandPlayed(float64 timepassed=0){return false;}; //support time-based blinds
            virtual const float64 BigBlind();
            virtual const float64 SmallBlind();
            #if defined(GRAPHMONEY)
            virtual void Reload(const float64 small,const float64 big,const uint32 handnum);
            #else
            virtual void Reload(const float64 small,const float64 big);
            #endif
}
;


class StackPlayerBlinds : virtual public BlindStructure
{
	protected:
		const float64 totalChips;
		const float64 blindRatio;
		int8 playerNum;
	public:
		StackPlayerBlinds(float64 stack, int8 players, float64 ratio)
	: BlindStructure(stack*ratio/2.0, stack*ratio), totalChips(stack*players), blindRatio(ratio), playerNum(players) {}

		virtual bool PlayerEliminated();

}
;

class GeomPlayerBlinds : virtual public BlindStructure
{
	protected:
		float64 bigRatio;
		float64 smallRatio;
	public:
		GeomPlayerBlinds(float64 small, float64 big, float64 smallIncr, float64 bigIncr)
	: BlindStructure(small, big), bigRatio(bigIncr), smallRatio(smallIncr) {}

		virtual bool PlayerEliminated();

}
;

class AlgbHandBlinds : virtual public BlindStructure
{
	protected:

		float64 bigPlus, smallPlus;
		int16 freq,since;
	public:
		AlgbHandBlinds(float64 small, float64 big, float64 smallIncr, float64 bigIncr, int16 afterHands)
	: BlindStructure(small, big), bigPlus(bigIncr), smallPlus(smallIncr), freq(afterHands),since(0) {}

		virtual bool HandPlayed(float64 timepassed=0); //support time-based blinds

}
;

class SitAndGoBlinds : virtual public BlindStructure
{
    private:
    static float64 fibIncr(float64 a, float64 b);

    protected:
    float64 hist[3];
    int16 handPeriod;
    int16 handCount;

    public:
    #if defined(GRAPHMONEY)
    virtual void Reload(const float64 small,const float64 big,const uint32 handnum);
    #else
    virtual void Reload(const float64 small,const float64 big);
    #endif

    SitAndGoBlinds(float64 small, float64 big, const int16 afterHands)
	: BlindStructure(small, big), handPeriod(afterHands), handCount(afterHands)
    {
        Reload(small,big
        #if defined(GRAPHMONEY)
        ,afterHands
        #endif
        );
    }

    virtual bool HandPlayed(float64 timepassed=0);
}
;

#endif
