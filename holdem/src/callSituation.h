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

#define DEBUG_EXFDEXF
#define ASSUMEFOLDS


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

    #ifdef ASSUMEFOLDS
    float64 eFold;
/*
    float64 wPct;
    float64 scaleToMean;
    float64 offset;
*/
    #endif

public:
    ExpectedCallD(const int8 id, const HoldemArena* base, const CallCumulationD* data)
    : playerID(id), table(base), e(data)
    #ifdef ASSUMEFOLDS
    ,eFold(base->GetNumberAtTable()-1)
    #endif
    {}

    virtual ~ExpectedCallD();

    virtual float64 forfeitChips() const;
    virtual float64 foldGain() const;
    virtual float64 oppBet() const;
    virtual float64 alreadyBet() const;
    virtual float64 callBet() const;
    virtual float64 chipDenom() const;
    virtual float64 allChips() const;
    virtual float64 maxBet() const;
    virtual int8 handsDealt() const;
    virtual float64 prevpotChips() const;
    virtual float64 betFraction(const float64 betSize) const;
    virtual float64 exf(const float64 betSize) = 0;
    virtual float64 dexf(const float64 betSize) = 0;

    #ifdef ASSUMEFOLDS
    virtual float64 callingPlayers() const;
    virtual void callingPlayers(float64 n);
    #endif


        #ifdef DEBUG_EXFDEXF
            void breakdown(float64 dist, std::ostream& target)
            {



                target << "vodd,exf.pct,dexf" << std::endl;
                for( float64 i=0;i<=1;i+=dist)
                {

                    target << i << "," << e->pctWillCall(i) << "," << e->pctWillCallD(i) << std::endl;

                }

            }
        #endif

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
    static const float64 UNITIALIZED_QUERY;
    float64 queryinput;
    float64 totalexf;
    float64 totaldexf;

    float64 impliedFactor;

    void query(const float64 betSize);
public:
    ExactCallD(const int8 id, const HoldemArena* base, const CallCumulationD* data)
    : ExpectedCallD(id,base,data), impliedFactor(1)
    {
        queryinput = UNITIALIZED_QUERY;
    }

    virtual float64 exf(const float64 betSize);
    virtual float64 dexf(const float64 betSize);

    virtual void SetImpliedFactor(const float64 bonus);
}
;

class ZeroCallD : public virtual ExpectedCallD
{
public:
    ZeroCallD(const int8 id, const HoldemArena* base, const CallCumulationD* data)
    : ExpectedCallD(id,base,data)
    {}

    virtual float64 exf(const float64 betSize);
    virtual float64 dexf(const float64 betSize);
};


#ifdef DEBUGBETMODEL

class DebugArena : public virtual HoldemArena
{
private:
    void updatePot();
protected:
    float64 deadPot;
public:

    DebugArena(BlindStructure* b,std::ostream& target, bool illustrate) : HoldemArena(b,target,illustrate,false), deadPot(0)
    {}

    void InitGame();
    const float64 PeekCallBet();
    void SetBlindPot(float64 amount);
    void SetDeadPot(float64 amount);
    void SetBet(int8 playerNum, float64 amount);

    void GiveCards(int8 playerNum, CommunityPlus h);
    void SetCommunity(const CommunityPlus h, const int8 cardsInComunity);

    void AssignHandNum( uint32 n );
}
;
#endif


#endif

