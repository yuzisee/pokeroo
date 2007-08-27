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


#ifndef HOLDEM_OpponentFunctions
#define HOLDEM_OpponentFunctions

#undef OLD_PREDICTION_ALGORITHM

#include "functionbase.h"
#include "inferentials.h"




#define RAREST_HAND_CHANCE 221.0

class FoldWaitLengthModel : public virtual ScalarFunctionModel
{
    private:
    float64 cacheRarity;
    protected:
    const float64 dRemainingBet_dn();
    const float64 grossSacrifice(const float64 n);
    const float64 lookup(const float64 x) const;
    const float64 dlookup(const float64 x) const;
    public:



    CallCumulationD (* meanConv); //Set to zero if using RANK
    float64 w;
    float64 amountSacrifice;
    float64 bankroll;
    float64 opponents;
    float64 betSize;

    FoldWaitLengthModel() : ScalarFunctionModel(1.0/3.0), meanConv(0), w(0), amountSacrifice(0), bankroll(0), opponents(1){};
    FoldWaitLengthModel(const FoldWaitLengthModel & o) : ScalarFunctionModel(1.0/3.0), w(o.w), amountSacrifice(o.amountSacrifice), bankroll(o.bankroll), opponents(o.opponents), betSize(o.betSize){};

    const FoldWaitLengthModel & operator= ( const FoldWaitLengthModel & o );
    const bool operator== ( const FoldWaitLengthModel & o ) const;

    virtual ~FoldWaitLengthModel();

    virtual float64 f(const float64);
    virtual float64 fd(const float64, const float64);
    virtual float64 d_dbetSize( const float64 n );
    virtual float64 d_dw( const float64 n );
    virtual float64 d_dC( const float64 n );

    virtual float64 FindBestLength();
    const float64 rarity();

}
;



class FoldGainModel : public virtual ScalarFunctionModel
{
    protected:
    FoldWaitLengthModel lastWaitLength;
    float64 lastBetSize;
    float64 last_dw_dbet;
    float64 lastf;
    float64 lastfd;
    float64 lastFA, lastFB,lastFC;

    void query(const float64 betSize);

    public:
    float64 n;
    float64 dw_dbet;



    FoldWaitLengthModel waitLength;

    FoldGainModel(float64 quantum) : ScalarFunctionModel(quantum), last_dw_dbet(0){};

    virtual ~FoldGainModel();

    virtual float64 f(const float64 betSize);
    virtual float64 fd(const float64 betSize, const float64 gain);
    virtual float64 F_a(const float64 betSize);
    virtual float64 F_b(const float64 betSize);
    virtual float64 dF_dAmountSacrifice(const float64 betSize);


}
;

class FacedOddsCallGeom : public virtual ScalarFunctionModel
{
    protected:
    float64 lastW;
    float64 lastF;
    float64 lastFD;
    void query( const float64 w );
    public:
    float64 B;
    float64 pot;
    float64 alreadyBet;
    float64 outsidebet;
    float64 opponents;


    FoldGainModel FG;
    FacedOddsCallGeom(float64 quantum) : ScalarFunctionModel(0.5/RAREST_HAND_CHANCE), lastW(-1), FG(quantum/2) {}
    virtual float64 f(const float64 w);
    virtual float64 fd(const float64 w, const float64 U);
}
;


class FacedOddsAlgb : public virtual ScalarFunctionModel
{
    protected:
    float64 lastW;
    float64 lastF;
    float64 lastFD;
    void query( const float64 w );
    public:
    float64 pot;
    float64 alreadyBet;
    float64 betSize;


    FoldGainModel FG;
    FacedOddsAlgb(float64 quantum) : ScalarFunctionModel(0.5/RAREST_HAND_CHANCE), lastW(-1), FG(quantum/2) {}
    virtual float64 f(const float64 w);
    virtual float64 fd(const float64 w, const float64 U);
}
;


class FacedOddsRaiseGeom : public virtual ScalarFunctionModel
{
    protected:
    float64 lastW;
    float64 lastF;
    float64 lastFD;
    void query( const float64 w );
    public:
    float64 pot;
    float64 raiseTo;
    float64 fold_bet;
    float64 riskLoss;
    bool bCheckPossible;

    FoldGainModel FG;
    FacedOddsRaiseGeom(float64 quantum) : ScalarFunctionModel(0.5/RAREST_HAND_CHANCE), lastW(-1), FG(quantum/2) {}
    virtual float64 f(const float64 w);
    virtual float64 fd(const float64 w, const float64 U);
}
;

#endif

