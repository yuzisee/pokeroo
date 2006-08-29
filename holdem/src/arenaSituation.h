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

#ifndef HOLDEM_ArenaSituations
#define HOLDEM_ArenaSituations

#include "arena.h"
/*
class ExpectedCall
{
    protected:

    const HoldemArena* table;
    const int8 playerID;
    public:
    virtual float64 exf(float64 betSize);
}
;
*/
class ExpectedCallD /*: public virtual ExpectedCall*/
{
protected:
    const int8 playerID;
    const HoldemArena* table;
    const CallCumulationD* e;
public:
    ExpectedCallD(const int8 id, const HoldemArena* base, const CallCumulationD* data)
    : playerID(id), table(base), e(data)
    {}

    virtual ~ExpectedCallD();

    virtual float64 deadpotFraction() const;
    virtual float64 betFraction(const float64 betSize) const;
    virtual float64 exf(float64 betSize) = 0;
    virtual float64 dexf(float64 betSize) = 0;
}
;

/*
class EstimateCallD : public virtual ExpectedCallD
{
public:

    EstimateCallD(const int8 id, const HoldemArena* base, const CallCumulationD* data)
    : ExpectedCallD(id,base,data)
    {};

    virtual float64 exf(float64 betSize);
    virtual float64 dexf(float64 betSize);
}
;
*/

class ExactCallD : public virtual ExpectedCallD
{
protected:
    static const float64 UNITIALIZED_QUERY = -1;
    float64 queryinput;
    float64 totalexf;
    float64 totaldexf;

    void query(float64 betSize);
public:
    ExactCallD(const int8 id, const HoldemArena* base, const CallCumulationD* data)
    : ExpectedCallD(id,base,data)
    {
        queryinput = UNITIALIZED_QUERY;
    };

    virtual float64 exf(float64 betSize);
    virtual float64 dexf(float64 betSize);
}
;

/*
class DummyArena : public virtual HoldemArena
{
public:

    DummyArena(BlindStructure* b, bool illustrate) : HoldemArena(b,illustrate)
    {}

    virtual const Player* ViewPlayer(int8) const;
}
;
*/


#endif

