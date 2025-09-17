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

#include "stratFear.h"

// TODO(from yuzisee): handsToBeat() here??
// ?? TODO??? --> This is the "established" hand strength requirement of anyone willing to claim they will win this hand.


float64 OpponentFoldWait::ActOrReact(float64 callb, float64 lastbet, float64 limit) const //Returns 0 for full act, returns 1 for full react
{
//One must consider the possibilities:
//1. [  ACT a.k.a. 0.0] The player betting has been folding all this time, and has hit his/her hand ALREADY
//2. [  ACT a.k.a. 0.0] The opponents have not bet yet, and would be the reactors of this hand.
//3. [REACT a.k.a. 1.0] The pot is large from previous rounds, opponents can't fold easily

	const float64 nPlayers = 1+tableinfo->handsToShowdownAgainst();

    //The share of the pot that others are entitled to on average
	const float64 othersShare = 1 - (1 / nPlayers);
    //We need to scale actOrReact based on how many players are at the table...?

	const float64 myAllInEffective = tableinfo->maxRaiseAmount(); // At this price either I'm all-in or everyone else (in the hand) is.
	float64 tablepot = tableinfo->table->GetPotSize();
	const float64 maxToShowdownPot = myAllInEffective * (tableinfo->handsIn()-1) + tableinfo->alreadyBet();
	if( tablepot > maxToShowdownPot ) tablepot = maxToShowdownPot;

    //const float64 avgControl = (stagnantPot() + table->GetUnbetBlindsTotal()) / table->GetNumberInHand();
    //const float64 raiseOverBet = (callb + avgControl);// < lastbet) ? 0 : (callb + avgControl - lastbet) ;
    const float64 raiseOverOthers = (tablepot - callb) / tableinfo->handsIn();
    const float64 raiseOver = (raiseOverOthers);// + raiseOverBet)/2;

    // If raiseOver is slightly less than limit, actOrReact should not be greater than 1.0
    const float64 uncappedActOrReact = (raiseOver / (limit * othersShare));
    const float64 actOrReact = (uncappedActOrReact > 1.0) ? 1.0 : uncappedActOrReact;
    return actOrReact;
    //const float64 actOrReact = (raiseOver > limit) ? mPlayers : (raiseOver / limit);
    //return actOrReact / mPlayers;
}

float64 OpponentFoldWait::FearStartingBet(ExactCallBluffD & oppFoldEst, float64 maxScaler, const ExpectedCallD & table_info)
{
    ScalarPWinFunction searchFoldPcts(oppFoldEst, table_info.chipDenom()/2.0, oppFoldStartingPct(table_info));
    return searchFoldPcts.FindZero(0.0,maxScaler, true);
    // Solve for the bet that brings Pr{opponentFold} to oppFoldStartingPct
}

float64 ScalarPWinFunction::f( const float64 betSize )
{
    return deterredCall.pWin(betSize) - oppFoldStartingPct;
}

float64 ScalarPWinFunction::fd( const float64 betSize, const float64 y )
{
    return deterredCall.pWinD(betSize);
}
