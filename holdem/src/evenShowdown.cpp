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


/*
 *  evenShowdown.cpp
 *  holdem
 *
 *  Created by Joseph Huang on 2006.12.05.
 *  Copyright 2006 __MyCompanyName__. All rights reserved.
 *
 */

#include "evenShowdown.h"

void AllInShowdown::initS(const int8 cardsInCommunity, const int8 numAllIn)
{
	moreCards = 5-cardsInCommunity;


	oppUndo = new CommunityPlus[moreCards];

    int8 cardsAvail = 52-cardsInCommunity-numAllIn;

    int32 oppHands = cardsAvail*(cardsAvail-1)/2;
    myTotalChances = static_cast<float64>(oppHands);
	statCount = oppHands;

    myWins = new StatResult[oppHands];

    myChancesEach = HoldemUtil::nchoosep<float64>(cardsAvail - 2,5-cardsInCommunity);

}
