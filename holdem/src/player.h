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

#ifndef HOLDEM_PlayerBase
#define HOLDEM_PlayerBase

#include "holdem2.h"
#include "arena.h"
#include "player.h"
#include <string>

using std::string;

class HoldemAction;
class HoldemArena;
class Player;

class PlayerStrategy
{
	friend class HoldemArena;
	private:

		Hand* myHand;
		Player* me;
		HoldemArena* game;

	public:

		const Hand& ViewHand() const { return *myHand; }
		const Player& ViewPlayer() const {return *me;}
		const HoldemArena& ViewTable() const {return *game; }

		PlayerStrategy() : me(0), game(0){}
		virtual ~PlayerStrategy(){};

		virtual void SeeCommunity(const Hand&, const int8) = 0;
				/*
		double myBetsIn; //(fraction)
		double p; //dead money in pot (fraction)
		double BetToCall; //(fraction)
				*/
		virtual float64 MakeBet() = 0;
			// 0 for check/fold
			// -1 will definately fold
			// this is the bet SINCE ROUND

		virtual void SeeOppHand(const int8, const Hand&) = 0;

		virtual void SeeAction(const HoldemAction&) = 0;
}
;

class Player
{
	friend class HoldemArena;
	private:
		static const int16 NAMECHARS = 40;
		PlayerStrategy* myStrat;
		string myName;

		float64 allIn;
		float64 myMoney;
		float64 handBetTotal; //Sum of bets made during COMPLETED betting rounds
		float64 myBetSize; //Within a betting round, the current bet on the table
		float64 lastBetSize; //Within a betting round, the bet before myBetSize

		Player( double money, const std::string name, PlayerStrategy* strat, float64 init_play)
		: myStrat(strat),  allIn(init_play), myMoney(money),
		 handBetTotal(0), myBetSize(0), lastBetSize(init_play)
		{
			myName = name;
			myHand.Empty();
		}

	protected:

		Hand myHand;
		const Hand& GetHand() const { return myHand; }

	public:

		const std::string & GetIdent() const
		{	return myName;	}

		float64 GetMoney() const
		{	return myMoney;	}

		float64 GetContribution() const
		{	return handBetTotal;	}

		//BetSize is since ROUND start, active players only
		float64 GetBetSize() const
		{	return myBetSize;	}

		float64 GetLastBet() const
		{	return lastBetSize;	}

}
;



/*
For KellyStrategy
		int getCallsInHand();
			//That is, say 2 people have called, one has folded, and one
			//has half the call in the pot. Then you return 2.5

			//Utilize getBetSize vs. callSize

		double Winnable(); //potSize - betsizes (try to take into account folded money and money from past rounds?
*/




/*const double Player::PredictedGain(double w, double s, int e, double p, double f)
{
    double WSgain=1;
    for(int m = 0;m<e;++m)
{
        double eCm = 1; // e CHOOSE m
        for(int i=0;i<e;++i)
{}

        double fProfit = f*static_cast<double>(e-m);
        WSgain *= pow(
                      1 + (p + fProfit) / static_cast<double>(m+1),
                      eCm*pow(w,e-m)*pow(s,m)
                  );
}

    return WSgain*pow(1 - f,1-pow(w+s,e));
}

const double Player::CubicEasyRoot(double a3, double a2, double a1, double a0)
{
    if(a3 == 0)
{
        if(a2 == 0)
{
            return a0/a1*-1;
}
        return (-a1 + sqrt(a1*a1-4*a2*a0))/2/a2;
}
    double a[3];
    a[0] = a0/a3;
    a[1] = a1/a3;
    a[2] = a2/a3;
    double Q = (3*a[1]-a[2]*a[2])/9;
    double R = (9*a[1]*a[2]-27*a[0]-2*a[2]*a[2]*a[2])/54;
    double x = pow(R + sqrt(R*R+Q*Q*Q),1.0/3.0);
    if (R == 0)
{//Since x^3 + 3Qx = 2R
        x = 0;
        //or
        //x = Math.sqrt(3*Q)

        //but we should probably not run into
        //this case using our utility function
}
    else
{
        x = x - Q/x;
}
    return x - a[2]/3;
}

double Player::MakeBet(double odds[],unsigned long oddsSize)
//d, or playersToCall, can be fractional? (if someone is in $8 to call a
// $12 bet. He's ToCall .33 and Called .66)
{
    //double betToCall=0;
    double w, s=0;

    w = odds[0];

    double e=0, d=0;
    for(int i=0;i<playerCount;++i)
{
        if ( 'f' != oppState[i] )
{
            ++e;
}
}

    double effSplit = pow(w+s, e) - pow(w, e);
    double a = pow(w,e) ;
    double b = pow(s,e);
    double c = pow((w+s),e);
    double u,x,y,z,kellyBet;
    double k = 0.9379*e - 0.8758 + effSplit*s * (-1.0418*e + 2.0836);

    u = d*k*(b-a-1);
    x = (k*(1+3*a-b)-(1+2*a-c)*(p+e+1))*d+e*k*(1-b);
    y = (a-c+1)*e*e+((b-a-1)*k+(1+2*d+p)*a+(p+1)*(1-c))*e+(p+1)*(1-a-b)*k+2*a*d*(p-k+1);
    z = (1-c)*p*p+(e+2-2*c+k*b-c*e+k*a-k-e*a)*p+(e*a+a-1+b)*k-(e+1)*(e*a+c-1);

    kellyBet = CubicEasyRoot(u,x,y,z);

    return kellyBet;
}*/
/*old theory:
double estTotalCall = improveFreq*playersToCall + playersCalled; //by players
solve {estCallMoney = estToCall*RoundBetSize
{RoundBetSize = Kelly(odds*/



#endif

