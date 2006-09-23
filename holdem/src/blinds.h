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

class BlindStructure
{
	protected:
		float64 myBigBlind;
		float64 mySmallBlind;
	public:
            BlindStructure(float64 small, float64 big)
    : myBigBlind(big), mySmallBlind(small) {};
            virtual ~BlindStructure();
            virtual void PlayerEliminated(){};
            virtual void HandPlayed(float64 timepassed=0){}; //support time-based blinds
            virtual const float64 BigBlind();
            virtual const float64 SmallBlind();

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

		virtual void PlayerEliminated();

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

		virtual void HandPlayed(float64 timepassed=0); //support time-based blinds

}
;

#endif
