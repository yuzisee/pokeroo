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

    //Count needed array size
    int32 arraySize = 0;
    while( ea->RaiseAmount(betSize,arraySize) < ea->maxBet() )
    {
        ++arraySize;
    }
    //This array loops until noRaiseArraySize is the index of the element with RaiseAmount(noRaiseArraySize) == maxBet()
    ++arraySize; //Now it's the size of the array


    //Create arrays
    float64 * raiseAmount_A = new float64[arraySize];

    float64 * oppRaisedChance_A = new float64[arraySize];
    float64 * oppRaisedChanceD_A = new float64[arraySize];


    float64 * potRaisedWin_A = new float64[arraySize];
    float64 * potRaisedWinD_A = new float64[arraySize];

	float64 * oppRaisedFoldGain_A = new float64[arraySize];


    float64 lastuptoRaisedChance = 0;
    float64 lastuptoRaisedChanceD = 0;
    float64 newRaisedChance = 0;
    float64 newRaisedChanceD = 0;
    for( int32 i=arraySize-1;i>=0; --i)
    {
        raiseAmount_A[i] = ea->RaiseAmount(betSize,i);
#ifdef RAISED_PWIN

        newRaisedChance = ea->pRaise(betSize,i);
        newRaisedChanceD = ea->pRaiseD(betSize,i);
        oppRaisedChance_A[i] = newRaisedChance - lastuptoRaisedChance;
        oppRaisedChanceD_A[i] = newRaisedChanceD - lastuptoRaisedChanceD;
        lastuptoRaisedChance = newRaisedChance;
        lastuptoRaisedChanceD = newRaisedChanceD;

        potRaisedWin_A[i] = g(raiseAmount_A[i]);
        potRaisedWinD_A[i] = gd(raiseAmount_A[i],potRaisedWin_A[i]);

        oppRaisedFoldGain_A[i] = e->foldGain() - e->betFraction(betSize - ea->alreadyBet() ); //You would fold the additional (betSize - ea->alreadyBet() )
        if( potRaisedWin_A[i] < oppRaisedFoldGain_A[i] )
        {
            potRaisedWin_A[i] = oppRaisedFoldGain_A[i];
            potRaisedWinD_A[i] = 0;
        }
		
#else
	raiseAmount_A[i] = 0;
    oppRaisedChance_A[i] = 0;
    oppRaisedChanceD_A[i] = 0;
    potRaisedWin_A[i] = 1;
    potRaisedWinD_A[i] = 0;
#endif
    }

///Establish [Play] values
	float64 playChance = 1 - oppFoldChance;
	float64 playChanceD = - oppFoldChanceD;
	for( int32 i=0;i<arraySize;++i )
	{
        playChance -= oppRaisedChance_A[i];
        playChanceD -= oppRaisedChanceD_A[i];
	}

    const float64 potNormalWin = g(betSize);
    const float64 potNormalWinD = gd(betSize,potNormalWin);



