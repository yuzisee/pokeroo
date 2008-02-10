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
#include "callPredictionFunctions.h"

//#define DEBUG_EXFDEXF
#define ASSUMEFOLDS
#define ANTI_PRESSURE_FOLDGAIN
#define CONSISTENT_AGG
//#define PURE_BLUFF


#define BLIND_ADJUSTED_FOLD
#undef SACRIFICE_COMMITTED
//#define SAME_WILL_LOSE_BLIND
//#define GEOM_COMBO_FOLDPCT





class ExpectedCallD /*: public virtual ExpectedCall*/
{
protected:
    const int8 playerID;
    const HoldemArena* table;
    CallCumulationD* e;

    const float64 potCommitted;

#ifdef ANTI_PRESSURE_FOLDGAIN
    FoldGainModel FG;
    const float64 handRarity;
    const float64 meanW;
#endif

    #if defined(ASSUMEFOLDS)
    float64 eFold;

    #endif

public:
    ExpectedCallD(const int8 id, const HoldemArena* base
#ifdef ANTI_PRESSURE_FOLDGAIN
            , const float64 rankPCT, const float64 meanPCT
#endif
                    , CallCumulationD* data, const float64 commit = 0)
    : playerID(id), table(base), e(data), potCommitted(0)
    #ifdef ANTI_PRESSURE_FOLDGAIN
    ,FG(base->GetChipDenom()/2),handRarity(1-rankPCT), meanW(meanPCT)
    #endif
    #if defined(ASSUMEFOLDS)
    ,eFold(base->GetNumberAtTable()-1)
    #endif
    {}

    virtual ~ExpectedCallD();

    virtual float64 forfeitChips() const;
    virtual float64 foldGain();
    virtual float64 foldGain(const float64 extra, const float64 facedBet);
#ifdef ANTI_PRESSURE_FOLDGAIN
    virtual float64 foldWaitLength();
#endif
    virtual float64 oppBet() const;
    virtual float64 alreadyBet() const;
    virtual float64 callBet() const;
    virtual float64 minCallFraction(const float64 betSize);
    virtual float64 chipDenom() const;
    virtual float64 allChips() const;
    virtual float64 maxBet() const;
    virtual int8 handsDealt() const;
    virtual int8 handsIn() const;
    virtual float64 prevpotChips() const;
    virtual float64 stagnantPot() const;
    virtual float64 betFraction(const float64 betSize) const;
    virtual float64 handBetBase() const; //The B (bankroll) in calculations
	virtual float64 minRaiseTo() const;
	virtual bool inBlinds() const;


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
class AutoScalingExpectedCallD : public virtual ExpectedCallD
{
    protected:
        virtual void query(float64 x);
        const float64 saturate_min, saturate_max, saturate_upto;
        float64 last_x;
        float64 y;

        const ExpectedCallD* left;
        const ExpectedCallD* right;
    public:

ExpectedCallD(const int8 id, const HoldemArena* base
#ifdef ANTI_PRESSURE_FOLDGAIN
            , const float64 rankPCT
#endif
                    , const CallCumulationD* data, const float64 commit = 0)

        AutoScalingExpectedCallD(const ExpectedCallD* e_left,const ExpectedCallD* e_right,const float64 minX, const float64 maxX, const float64 upto)
        : ExpectedCallD(
                            (e_left->playerID == e_right->playerID) ? e_left->playerID : -1
                            , (e_left->table == e_right->table) ? e_left->table : 0
#ifdef ANTI_PRESSURE_FOLDGAIN
                            ,sqrt(e_left->handRarity * e_right->handRarity) //Geometric mean here?
#endif
                            ,(e_left->e != e_right->e
                            ,0
                        )
        , saturate_min(minX), saturate_max(maxX), saturate_upto(upto), left(e_left), right(e_right)
        {
            if( (e_left->playerID != e_right->playerID)
                || (e_left->table != e_right->table)
                || (e_left->handRarity != e_right->handRarity)
                || e_left->
            query(0);
        }
}
;
*/
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


class ZeroCallD : public virtual ExpectedCallD
{
public:
    ZeroCallD(const int8 id, const HoldemArena* base
#ifdef ANTI_PRESSURE_FOLDGAIN
            , const float64 rankPCT, const float64 meanPCT
#endif
                    , CallCumulationD* data)
    : ExpectedCallD(id,base
#ifdef ANTI_PRESSURE_FOLDGAIN
            ,rankPCT, meanPCT
#endif
                    ,data)
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

