/***************************************************************************
 *   Copyright (C) 2005 by Joseph Huang                                    *
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

#ifndef HOLDEM_RandomDeck
#define HOLDEM_RandomDeck

#include "engine.h"


class GameDeck : virtual public OrderedDeck
{

public:
        
    virtual void ShuffleDeck()=0;
    virtual void ShuffleDeck(float64)=0;
    
    virtual float64 DealCard(Hand&)=0;
    
}
;

class RandomDeck : virtual public GameDeck
{

	protected:
   		static const uint8 DECKSIZE = 52;
        bool bDeckEmpty;
        bool bAutoShuffle;
		uint8 deckOrder[DECKSIZE];
		int8 lastDealtPos;
		int8 firstDealtPos;
	public:

		virtual void ShuffleDeck();
		virtual void ShuffleDeck(float64);

		virtual float64 DealCard(Hand&);

		RandomDeck(bool autoshuffle = false) : OrderedDeck(), bDeckEmpty(false), bAutoShuffle(autoshuffle), lastDealtPos(-1)
		{
			firstDealtPos = DECKSIZE-1;
			for(uint8 i=0;i<DECKSIZE;++i)
			{
				deckOrder[i] = i;
			}
			ShuffleDeck();
		}

}
;

class LiveDeck : virtual public GameDeck
{
    protected:
    vector<DeckLocation> nextDeal;
    public:
    virtual void ShuffleDeck(){}
    virtual void ShuffleDeck(float64){}
    
    virtual float64 DealCard(Hand&)=0;
    
    void SetTopCard(const DeckLocation &d);
    
    LiveDeck() {}
}
;


#endif


