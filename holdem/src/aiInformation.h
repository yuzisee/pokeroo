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


#ifndef HOLDEM_GAUSSAI
#define HOLDEM_GAUSSAI

#include "ai.h"
//#include "aiCombo.h"
#include "engine.h"


class PocketHand
{
public:
	uint8 a;
	uint8 b;
	float64 repeated;
	ShowdownRep result;

	PocketHand() : result(-1){}

	PocketHand(const CommunityPlus* h, const uint8 carda, const uint8 cardb, const float64 occ)
	: a(carda), b(cardb), repeated(occ), result(h,-1)
	{}
	PocketHand(const PocketHand& p)
	: a(p.a), b(p.b), repeated(p.repeated), result(p.result)
	{}

    bool operator> (const PocketHand& x) const
	{
		return result > x.result;
	}
	bool operator< (const PocketHand& x) const
	{
		return result < x.result;
	}
	bool operator== (const PocketHand& x) const
	{
	    return result == x.result;
	}

	void PopulateByIndex();
}
;

class CommunityCallStats : public virtual CallStats
{
private:
	const void initCC(const int8);
protected:

	int8 indexHistory[2];
	PocketHand* myHands;
	int32 showdownIndex;
	int32 showdownCount;
	float64 showdownMax;
    float64 groupRepeated;

    const virtual void fillMyWins(StatResult ** table);

    const virtual int8 realCardsAvailable(const int8 cardsInCommunity) const;
    const virtual void showProgressUpdate() const;
    const virtual void setCurrentGroupOcc(const float64 occ);
    const virtual void mynoAddCard(const DeckLocation& cardinfo, const int16 undoIndex);
    const virtual void myAddCard(const DeckLocation& cardinfo, const int16 undoIndex){}
	const virtual void myEval(){}
	const virtual void myRevert(const int16 undoIndex){}
public:

    CommunityCallStats(const CommunityPlus& hP, const CommunityPlus& onlycommunity,
		int8 cardsInCommunity) : PlayStats(hP,onlycommunity),CallStats(hP,onlycommunity,cardsInCommunity)
	{
	    initCC(cardsInCommunity);
	}
    virtual ~CommunityCallStats();

	const virtual void Analyze();

	const virtual void Compare(const float64 occ);
}
;

class TriviaDeck : public OrderedDeck
{
    private:
        const static uint32 largestCard(uint32 suitcards);
    public:
    const void DiffHand(const Hand&);

}
;


#endif

