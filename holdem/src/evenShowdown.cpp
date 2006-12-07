/*
 *  evenShowdown.cpp
 *  holdem
 *
 *  Created by Joseph Huang on 2006.12.05.
 *  Copyright 2006 __MyCompanyName__. All rights reserved.
 *
 */

#include "evenShowdown.h"

void AllInShowdown::initS(const int8 cardsInCommunity, const int8 playersAllIn)
{
	moreCards = 5-cardsInCommunity;


	oppUndo = new CommunityPlus[moreCards];

    int8 cardsAvail = 52-cardsInCommunity-playersAllIn;

    int32 oppHands = cardsAvail*(cardsAvail-1)/2;
    myTotalChances = static_cast<float64>(oppHands);
	statCount = oppHands;

    myWins = new StatResult[oppHands];

    myChancesEach = HoldemUtil::nchoosep<float64>(cardsAvail - 2,5-cardsInCommunity);

}
