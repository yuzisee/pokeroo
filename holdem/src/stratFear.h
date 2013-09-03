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

#define NUM_STARTING_HANDS 169

class OpponentFoldWait
{
    protected:
        ExpectedCallD * const tableinfo;

    public:

    OpponentFoldWait(ExpectedCallD * const tbase) : tableinfo(tbase)
    {}

    float64 ActOrReact(float64 callb, float64 lastbet,float64 tablelimit) const;

    // find the bet, between 0.0 and maxScaler, where someone might fold at all.
    // this allows us to understand which bets aren't a fear threat.
    // Alternatively, consider that any bet where someone has a choice of folding, he/she will start to wait for better hands.
    // It may also be just fine to use minRaise instead. But let's see.
    float64 FearStartingBet(ExactCallBluffD & oppFoldEst, float64 maxScaler);

    // If we hit this percentage, it's possible to get all-fold for one of the 169 hands for each remaining player.
    // This is the smallest relevant non-zero win percentage we care about.
    float64 oppFoldStartingPct(ExactCallD & oppFoldEst) {
        return pow(1.0 / NUM_STARTING_HANDS, oppFoldEst.tableinfo->handsToShowdown());
    }
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
