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

#include "callPrediction.h"

#ifndef HOLDEM_FearManagement
#define HOLDEM_FearManagement



class OpponentFoldWait
{
    protected:
        ExpectedCallD * const tableinfo;

    public:

    OpponentFoldWait(ExpectedCallD * const tbase) : tableinfo(tbase)
    {}

    float64 ActOrReact(float64 callb, float64 lastbet,float64 tablelimit) const;

    float64 FearStartingBet(ExactCallBluffD & oppFoldEst, float64 oppFoldStartingPct, float64 maxScaler);
    //float64 FearMaxPercentage(ExactCallBluffD & oppFoldEst, float64 maxScaler);

}
;

class ScalarPWinFunction : public virtual ScalarFunctionModel
{

    protected:
        ExactCallBluffD & deterredCall;
        float64 oppFoldStartingPct;

    public:


    ScalarPWinFunction(ExactCallBluffD & oppFoldEst, float64 myQuantum, float64 offset) : ScalarFunctionModel(myQuantum), deterredCall(oppFoldEst), oppFoldStartingPct(offset) {}
    virtual float64 f(const float64 bet);
    virtual float64 fd(const float64 bet, const float64 pw);


}
;


#endif // HOLDEM_FearManagement
