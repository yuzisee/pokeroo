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


//This file is used as a multi-line macro expansion

last_x = betSize;

///Establish [PushGain] values

	float64 potFoldWin = ea->PushGain();
	const float64 potFoldWinD = 0;
    float64 oppFoldChance = ea->pWin(betSize);
    float64 oppFoldChanceD = ea->pWinD(betSize);
#ifdef DEBUGASSERT
	if( potFoldWin < 0 || oppFoldChance <= 0 ){
		potFoldWin =  1;
		oppFoldChance = 0;
		oppFoldChanceD = 0;
	}
#endif

///Establish [Raised] values

#ifdef RAISED_PWIN
    float64 raiseAmount;
    float64 minRaiseDirect = ea->minRaiseTo();
    float64 minRaiseBy = minRaiseDirect - ea->callBet();
    float64 minRaiseBet = betSize - ea->callBet();
    if( minRaiseBet < minRaiseDirect )
    {
        //Two minraises above bet to call
        raiseAmount = minRaiseDirect + minRaiseBy;
    }else{
        raiseAmount = betSize + minRaiseBet;
    }

    if( raiseAmount > ea->maxBet() )
    {
        raiseAmount = ea->maxBet();
    }

    const float64 oppRaisedChance = ea->pWin(raiseAmount);
    const float64 oppRaisedChanceD = ea->pWinD(raiseAmount);
    const float64 potRaisedWin = g(raiseAmount);
    const float64 potRaisedWinD = gd(raiseAmount,potRaisedWin);
#else
	float64 raiseAmount = 0;
    const float64 oppRaisedChance = 0;
    const float64 oppRaisedChanceD = 0;
    const float64 potRaisedWin = 1;
    const float64 potRaisedWinD = 0;
#endif

///Establish [Play] values
	const float64 playChance = 1 - oppFoldChance - oppRaisedChance;
	const float64 playChanceD = - oppFoldChanceD - oppRaisedChanceD;
    const float64 potNormalWin = g(betSize);
    const float64 potNormalWinD = gd(betSize,potNormalWin);



