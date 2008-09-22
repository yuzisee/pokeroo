/***************************************************************************
 *   Copyright (C) 2008 by Joseph Huang                                    *
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

#include "stratFear.h"

float64 OpponentFoldWait::ActOrReact(float64 callb, float64 lastbet, float64 limit) const //Returns 0 for full act, returns 1 for full react
{
//One must consider the possibilities:
//1. [ACT] The player betting has been folding all this time, and has hit his/her hand ALREADY
//2. [ACT] The opponents have not bet yet, and would be the reactors of this hand.
//3. [REACT] The pot is large from previous rounds, opponents can't fold easily

	const float64 nPlayers = 1+tableinfo->handsToBeat();
	const float64 mPlayers = 1 - (1 / nPlayers); //We need to scale actOrReact based on how many players are at the table...?

	const float64 mylimit = tableinfo->maxRaiseAmount();
	float64 tablepot = tableinfo->table->GetPotSize();
	const float64 maxToShowdownPot = mylimit * (tableinfo->handsIn()-1) + tableinfo->alreadyBet();
	if( tablepot > maxToShowdownPot ) tablepot = maxToShowdownPot;

    //const float64 avgControl = (stagnantPot() + table->GetUnbetBlindsTotal()) / table->GetNumberInHand();
    //const float64 raiseOverBet = (callb + avgControl);// < lastbet) ? 0 : (callb + avgControl - lastbet) ;
    const float64 raiseOverOthers = (tablepot - callb) / tableinfo->handsIn();
    const float64 raiseOver = (raiseOverOthers);// + raiseOverBet)/2;
    const float64 actOrReact = (raiseOver > limit) ? mPlayers : (raiseOver / limit);
    return actOrReact / mPlayers;
}

float64 OpponentFoldWait::FearStartingBet(ExactCallBluffD & oppFoldEst, float64 oppFoldStartingPct, float64 maxScaler)
{
    ScalarPWinFunction searchFoldPcts(oppFoldEst, tableinfo->chipDenom()/2.0, oppFoldStartingPct);
    return searchFoldPcts.FindZero(0.0,maxScaler);
}




float64 ScalarPWinFunction::f( const float64 betSize )
{
    return deterredCall.pWin(betSize) - oppFoldStartingPct;

}

float64 ScalarPWinFunction::fd( const float64 betSize, const float64 y )
{
    return deterredCall.pWinD(betSize);
}

