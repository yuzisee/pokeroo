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
    float64 abRepeated;
	float64 repeated;
	ShowdownRep result;

	PocketHand() : result(-1){}

	PocketHand(const CommunityPlus* h, const uint8 carda, const uint8 cardb, const float64 groupRepeated, const float64 occ)
	: a(carda), b(cardb), abRepeated(groupRepeated), repeated(occ), result(h,-1)
	{}
	PocketHand(const PocketHand& p)
	: a(p.a), b(p.b), abRepeated(p.abRepeated),repeated(p.repeated), result(p.result)
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
    const PocketHand& operator= (const PocketHand& o)
    {
        a = o.a;
        b = o.b;
        abRepeated = o.abRepeated;
        repeated = o.repeated;
        result = o.result;
        return *this;
    }

//void PopulateByIndex();
}
;

class CommunityCallStats : public virtual CallStats
{
private:
	void initCC(const int8);
    bool bSortedHands;
protected:

	int8 indexHistory[2];
	PocketHand* myHands;
	int32 showdownIndex;
	int32 showdownCount;
	float64 showdownMax;
    float64 groupRepeated;

    virtual void fillMyWins(StatResult ** table);

    const virtual int8 realCardsAvailable(const int8 cardsInCommunity) const;
    virtual void showProgressUpdate() const;
    virtual void setCurrentGroupOcc(const float64 occ);
    virtual void mynoAddCard(const DeckLocation& cardinfo, const int16 undoIndex);
    virtual void myAddCard(const DeckLocation& cardinfo, const int16 undoIndex){}
	virtual void myEval(){}
	virtual void myRevert(const int16 undoIndex){}
public:

    CommunityCallStats(const CommunityPlus& hP, const CommunityPlus& onlycommunity,
		int8 cardsInCommunity) : PlayStats(hP,onlycommunity),CallStats(hP,onlycommunity,cardsInCommunity)
	{
	    initCC(cardsInCommunity);
	}
    CommunityCallStats(const CommunityCallStats& covered, const CommunityPlus& withCommunity, const CommunityPlus& onlycommunity)
        : PlayStats(withCommunity,onlycommunity)
        ,CallStats(withCommunity,onlycommunity,static_cast<int8>(7)-covered.moreCards)
    {
        initCC(static_cast<int8>(7)-covered.moreCards);
        showdownIndex = covered.showdownIndex;
        showdownCount = covered.showdownCount;
        showdownMax = covered.showdownMax;
        for( int32 i=0;i<showdownCount;++i )
        {
            myHands[i].a = covered.myHands[i].a;
            myHands[i].b = covered.myHands[i].b;
            myHands[i].abRepeated = covered.myHands[i].abRepeated;
            myHands[i].repeated = covered.myHands[i].repeated;
            myHands[i].result.strength = covered.myHands[i].result.strength;
            myHands[i].result.valueset = covered.myHands[i].result.valueset;
            myHands[i].result.revtiebreak = covered.myHands[i].result.revtiebreak;
        }
        statCount = covered.statCount;
        bSortedHands = covered.bSortedHands;
    }

    virtual ~CommunityCallStats();

	virtual void Analyze();

	virtual void Compare(const float64 occ);
}
;

class TriviaDeck : public OrderedDeck
{
    private:
        const static uint32 largestCard(uint32 suitcards);
    public:
    void DiffHand(const Hand&);

}
;


#endif

