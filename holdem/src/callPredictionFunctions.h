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


#ifndef HOLDEM_OpponentFunctions
#define HOLDEM_OpponentFunctions


#include "functionbase.h"
#include "inferentials.h"
#include <math.h>

#ifndef log1p
inline float64 log1p(float64 x)
{
    return log(1+x);
}
#endif
/*
#ifndef log1p
#define log1p( _X_ ) log( (_X_) + 1 )
#endif
*/


#ifndef round
#include <cmath>
inline float64 round(float64 a)
{
    return floor(a+0.5);
}

#endif


// The probability of being dealt, e.g. two aces: 1 in 221
#define RAREST_HAND_CHANCE 221.0

class FoldWaitLengthModel : public virtual ScalarFunctionModel
{
    private:
    float64 cacheRarity;
    float64 lastdBetSizeN;
    float64 lastRawPCT;
    protected:
    const float64 dRemainingBet_dn();
    const float64 grossSacrifice(const float64 n);

    /**
     *  Parameters:
     *    rank:
     *      Your rank
     *  Returns:
     *    Either mean or rank (depending on mode) of the hand with the given rank
     */
    const float64 lookup(const float64 rank) const;
    const float64 dlookup(const float64 rank, const float64 mean) const;

    float64 cached_d_dbetSize;
    bool bSearching;
    
    public:


    float64 get_cached_d_dbetSize() const { return cached_d_dbetSize; }

    // Describe the hand they would be folding
    float64 w;                    // Set to RANK if meanConv is null. Set to MEAN_winpct if using *meanConv
    CallCumulationD (* meanConv); // Set to null if using RANK for payout simulation

    // Describe the situation
    float64 amountSacrificeVoluntary;
    float64 amountSacrificeForced;
    float64 bankroll;
    float64 opponents;
    float64 betSize;

    // NOTE: quantum is 1/3rd of a hand. We don't need more precision than that when evaluating f(n).
    FoldWaitLengthModel() : ScalarFunctionModel(1.0/3.0),
        cacheRarity(std::nan("")), lastdBetSizeN(std::nan("")), lastRawPCT(std::nan("")), bSearching(false),
    w(std::nan("")), meanConv(0), amountSacrificeVoluntary(0), amountSacrificeForced(0), bankroll(0), opponents(1), betSize(std::nan(""))
    {}

    // NOTE: Although this is the copy constructor, it doesn't copy caches. This lets you clone a configuration and re-evaluate it.
    FoldWaitLengthModel(const FoldWaitLengthModel & o) : ScalarFunctionModel(1.0/3.0),
        cacheRarity(std::nan("")), lastdBetSizeN(std::nan("")), lastRawPCT(std::nan("")), bSearching(false),
        w(o.w), meanConv(o.meanConv), amountSacrificeVoluntary(o.amountSacrificeVoluntary), amountSacrificeForced(o.amountSacrificeForced), bankroll(o.bankroll), opponents(o.opponents), betSize(o.betSize){};

    const FoldWaitLengthModel & operator= ( const FoldWaitLengthModel & o );
    const bool operator== ( const FoldWaitLengthModel & o ) const;

    virtual ~FoldWaitLengthModel();

    /*! @brief Here, n, is the number of HANDS, not the number of folds. */
    virtual float64 f(const float64);
    virtual float64 fd(const float64, const float64);
    virtual float64 d_dbetSize( const float64 n );
    virtual float64 d_dw( const float64 n );
    virtual float64 d_dC( const float64 n );

    virtual float64 FindBestLength();
    const float64 rarity();

}
;


// NOTE:
// You never use FoldGainModel or FoldWaitLengthModel with handcumu.
// If it's an opponent against you that knows your hand, they use foldcumu.
// If it's you, you use callcumu.
// If it's an opponent that doesn't know your hand, you use callcumu.
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

    FoldGainModel(float64 myQuantum) : ScalarFunctionModel(myQuantum)
            , lastWaitLength(), lastBetSize(-1), last_dw_dbet(0) //Cache variables
            , dw_dbet(0) //Input variables
            {};

    virtual ~FoldGainModel();

    virtual float64 f(const float64 betSize);
    virtual float64 fd(const float64 betSize, const float64 gain);
    virtual float64 F_a(const float64 betSize);
    virtual float64 F_b(const float64 betSize);
    virtual float64 dF_dAmountSacrifice(const float64 betSize);


}
;

//How much call can you pick up to your bet?
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
//    float64 alreadyBet;
    float64 outsidebet;
    float64 opponents;


    FoldGainModel FG;
    FacedOddsCallGeom(float64 myQuantum) : ScalarFunctionModel(0.5/RAREST_HAND_CHANCE), lastW(-1), FG(myQuantum/2) {}
    virtual float64 f(const float64 w);
    virtual float64 fd(const float64 w, const float64 U);
}
;

//Will everybody fold consecutively to your bet?
class FacedOddsAlgb : public virtual ScalarFunctionModel
{
    protected:
    float64 lastW;
    float64 lastF;
    float64 lastFD;
    void query( const float64 w );
    public:
    float64 pot;
    //float64 alreadyBet;
    float64 betSize;


    FoldGainModel FG;
    FacedOddsAlgb(float64 myQuantum) : ScalarFunctionModel(0.5/RAREST_HAND_CHANCE), lastW(-1), FG(myQuantum/2) {}
    virtual float64 f(const float64 w);
    virtual float64 fd(const float64 w, const float64 U);
}
;

//How much/likely would they raise or reraise?
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
	float64 callIncrLoss;
	float64 callIncrBase;
    bool bCheckPossible;

    FoldGainModel FG;
    FacedOddsRaiseGeom(float64 myQuantum) : ScalarFunctionModel(0.5/RAREST_HAND_CHANCE), lastW(-1), FG(myQuantum/2) {}
    virtual float64 f(const float64 w);
    virtual float64 fd(const float64 w, const float64 U);
}
;

#endif

