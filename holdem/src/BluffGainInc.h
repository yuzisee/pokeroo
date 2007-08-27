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
    const float64 invisiblePercent = quantum / ea->allChips();

///Establish [PushGain] values

	float64 potFoldWin = ea->PushGain();
	const float64 potFoldWinD = 0;
    float64 oppFoldChance = ea->pWin(betSize);
    float64 oppFoldChanceD = ea->pWinD(betSize);

#ifdef DEBUGASSERT
	if( potFoldWin < 0 || oppFoldChance < invisiblePercent ){
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
    if(betSize < ea->maxBet()) ++arraySize; //Now it's the size of the array (unless you're pushing all-in already)


    //Create arrays
    float64 * raiseAmount_A = new float64[arraySize];

    float64 * oppRaisedChance_A = new float64[arraySize];
    float64 * oppRaisedChanceD_A = new float64[arraySize];


    float64 * potRaisedWin_A = new float64[arraySize];
    float64 * potRaisedWinD_A = new float64[arraySize];


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

        potRaisedWin_A[i] = g_raised(betSize,raiseAmount_A[i]);
        potRaisedWinD_A[i] = gd_raised(betSize,raiseAmount_A[i],potRaisedWin_A[i]);

        const float64 oppRaisedFoldGain = e->foldGain(betSize - ea->alreadyBet(),raiseAmount_A[i]); //You would fold the additional (betSize - ea->alreadyBet() )

        if( potRaisedWin_A[i] < oppRaisedFoldGain )
        {
            potRaisedWin_A[i] = oppRaisedFoldGain;
            potRaisedWinD_A[i] = 0;
        }

    if( oppRaisedChance_A[i] < invisiblePercent )
#endif
	{raiseAmount_A[i] = 0;
    oppRaisedChance_A[i] = 0;
    oppRaisedChanceD_A[i] = 0;
    potRaisedWin_A[i] = 1;
    potRaisedWinD_A[i] = 0;}

    }



///Establish [Play] values
	float64 playChance = 1 - oppFoldChance;
	float64 playChanceD = - oppFoldChanceD;
	for( int32 i=0;i<arraySize;++i )
	{
        playChance -= oppRaisedChance_A[i];
        playChanceD -= oppRaisedChanceD_A[i];
	}

    float64 potNormalWin = g_raised(betSize,betSize);
    float64 potNormalWinD = gd_raised(betSize,betSize,potNormalWin);

    if( playChance <= 0 ) //roundoff, but {playChance == 0} is push-fold for the opponent
    {
        //Correct other odds
        const float64 totalChance = 1 - playChance;
        for( int32 i=arraySize-1;i>=0; --i)
        {
            potRaisedWin_A[i] /= totalChance;
            potRaisedWinD_A[i] /= totalChance;
        }
        oppFoldChance /= totalChance;
        oppFoldChanceD /= totalChance;

        //Remove call odds
        playChance = 0;
        playChanceD = 0;
        potNormalWin = 1;
        potNormalWinD = 0;
    }

