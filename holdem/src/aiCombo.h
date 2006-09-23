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


#ifndef HOLDEM_AICombo
#define HOLDEM_AICombo

#include "ai.h"
#include "engine.h"



class HandHash
{
public:
	static const uint32 hashCardPattern(const Hand& h, uint32 tableSize);
	static const uint32 hashValueset(const HandPlus& h, uint32 tableSize);
	static const uint32 hashValues(const Hand& h, uint32 tableSize);
	static const uint32 hashSuitCounts(const CommunityPlus& h, uint32 tableSize);
}
;

class PocketHash
{
public:
	static const uint32 hashRank(const PocketHand& h, const uint32 tableSize);
	static const bool sameDealtHand(const PocketHand& a, const PocketHand& b);
}
;



class PocketsMap
{
private:
	//static const uint32 PRIMESIZE_2d;
	static const uint32 PRIMESIZE_1d;
	virtual void cleanup();

protected:
///TODO: on new, store BOTH [31][52] and [52][31]
///TODO: on hit, store INTO BOTH ^^^^^^^^^^^^^^^^
	PocketHand **table;
	uint32 tableSize;
	uint32 itrIndex;

	StatResult *(callWinsPtr[52][52]);

public:

	const virtual void add(const struct PocketHand& p) ;
	//const virtual void add(const Hand&, const uint8, const uint8) ;
/*
#ifdef DEBUGASSERT
	double
#else
	void
#endif
virtual BuildTable(const Hand& h);

	//void Hash2D(const CommunityPlus&, uint32 &, uint32 &) const;
	PocketsMap() : itrIndex(0)
	{
		for(int8 ia=0;ia<52;++ia)
		{for(int8 ib=0;ib<52;++ib){
			callWinsPtr[ia][ib] = 0;
		}}

		tableSize=PRIMESIZE_1d;
		table = new PocketHand *[tableSize];
		for(uint32 i=0;i<tableSize;++i)
		{
			table[i] = 0;
		}
	}
*/
	virtual ~PocketsMap();

	virtual StatResult & fetchStat(const uint8, const uint8);
	virtual void BeginIteration();
	virtual StatResult * IterateNext();

}
;

#endif
