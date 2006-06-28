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

#include "aiCache.h"
#include <sstream>

using std::stringstream;

string CacheManager::dbFileName(const Hand& withCommunity, const Hand& onlyCommunity)
{
    TriviaDeck o;
    o.OmitCards(withCommunity);
    o.DiffHand(onlyCommunity);
    o.sortSuits();

    return o.NamePockets();

}

void CacheManager::Query(WinStats& q, const Hand& withCommunity, const Hand& onlyCommunity, int8 n)
{
    myStatBuilder.UndealAll();
    myStatBuilder.OmitCards(withCommunity);
}

void CacheManager::Query(CallStats& q, const Hand& withCommunity, const Hand& onlyCommunity, int8 n)
{
    myStatBuilder.UndealAll();
    myStatBuilder.OmitCards(withCommunity);
}


string TriviaDeck::NamePockets() const
{
    stringstream ss;
    string temp;

    uint32 x[2] = {OrderedDeck::dealtHand[firstSuit], OrderedDeck::dealtHand[nextSuit[firstSuit]]};
    int8 val;

    for(val=14;val>=2;--val)
    {
            if ((x[0] & HoldemConstants::CARD_ACEHIGH) != 0)
            {
                ss << HoldemUtil::VALKEY[val];
                x[0] ^= HoldemConstants::CARD_ACEHIGH; ///Eliminate the card we used, so as to quickly check for more cards
                break;
            }
            x[0] <<= 1;
            x[1] <<= 1;
    }

    if( x[0] == 0 )
    {///This suit is empty aside from the card we found. The hand is offsuit.
        for(;val>=2;--val)
        {
                if ((x[1] & HoldemConstants::CARD_ACEHIGH) != 0)
                {
                    ss << HoldemUtil::VALKEY[val];
                    ss << "x";
                    return ss.str();
                }
                x[1] <<= 1;
        }
    }else
    {///This suit has more cards. The hand is suited.
        while(val>=2)
        {
            --val;
            x[0] <<= 1;
            if ((x[0] & HoldemConstants::CARD_ACEHIGH) != 0)
            {
                ss << HoldemUtil::VALKEY[val];
                ss << "S";
                return ss.str();
            }

        }
    }
    ss << "?";
    return ss.str();
}

const void TriviaDeck::DiffHand(const Hand& h)
{
    OrderedDeck::dealtHand[0] ^= OrderedDeck::dealtHand[0] & h.SeeCards(0);
	OrderedDeck::dealtHand[1] ^= OrderedDeck::dealtHand[1] & h.SeeCards(1);
	OrderedDeck::dealtHand[2] ^= OrderedDeck::dealtHand[2] & h.SeeCards(2);
	OrderedDeck::dealtHand[3] ^= OrderedDeck::dealtHand[3] & h.SeeCards(3);

}

