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




#ifndef HOLDEM_Community
#define HOLDEM_Community

#include "holdemutil.h"
#include <iostream>

using std::cout;
using std::endl;



class CommunityPlus : public virtual HandPlus //greater strength wins, then greater valueset
{
private:
    int8 flushCount[4]; //number of cards in that suit

	uint8 threeOfAKind;
	uint8 bestPair;
	uint8 nextbestPair;

	const void preEvalStrength();
    const void cleanLastTwo();
    const void PrintInterpretHand() const;
public:


	const void evaluateStrength();
    char strength;

	const virtual void AppendUnique(const Hand&);
	const virtual void AppendUnique(const HandPlus&);
	const virtual void AppendUnique(const CommunityPlus&);


    virtual void AddToHand(const DeckLocation& deck)
    {	AddToHand(deck.Suit,deck.Rank,deck.Value);	}
    virtual void RemoveFromHand(const DeckLocation& deck)
    {	RemoveFromHand(deck.Suit,deck.Rank,deck.Value);	}

	const virtual void AddToHand(const int8,const uint8,const uint32);
	const virtual void RemoveFromHand(const int8,const uint8,const uint32);

	const virtual void SetUnique(const Hand&);
	const virtual void SetUnique(const HandPlus&);
	const virtual void SetUnique(const CommunityPlus&);

    const virtual void Empty();

    CommunityPlus();

	const int8 CardsInSuit(const int8) const;
	const virtual void DisplayHand() const;
    const virtual void DisplayHandBig() const;
}
;


#endif
