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

#include "callPrediction.h"
#include <algorithm>


//The idea behind eaFold, meanFold, rankFold, eaRkFold is that opponents have different reasons for folding


///In the end, we had to stick with Algb for foldgain. (At low blinds, oppfold is guaranteed in Geom)


#define NO_AUTO_RAISE



//#define ANTI_CHECK_PLAY

ExactCallD::~ExactCallD()
{
    if( noRaiseChance_A != 0 )
    {
        delete [] noRaiseChance_A;
    }
    if( noRaiseChanceD_A != 0 )
    {
        delete [] noRaiseChanceD_A;
    }
}

ExactCallBluffD::~ExactCallBluffD()
{
}

void ExactCallD::SetImpliedFactor(const float64 bonus)
{
    impliedFactor = bonus;
}


/*
float64 ExactCallBluffD::topTwoOfThree(float64 a, float64 b, float64 c, float64 a_d, float64 b_d, float64 c_d, float64 & r) const
{
    if( b < a )
    {//a cannot be smallest
        if( b < c )
        {//b is smallest
            r = (a + c)/2;
            return (a_d + c_d)/2;
        }else //c < b < a
        {//c is smallest
            r = (a + b)/2;
            return (a_d + b_d)/2;
        }
    }
    else
    {//Since b > a, b cannot be smallest
        if( a < c )
        {//a is smallest
            r = (b + c)/2;
            return (b_d + c_d)/2;
        }else
        {//c is smallest
            r = (a + b)/2;
            return (a_d + b_d)/2;
        }
    }
}
*/
///This function is used to maximize chance to fold on a player-by-player basis
float64 ExactCallBluffD::bottomThreeOfFour(float64 a, float64 b, float64 c, float64 d, float64 a_d, float64 b_d, float64 c_d, float64 d_d, float64 & r) const
{
    float64 x[4] = {a,b,c,d};

    std::sort(x,x+4); //Ascending, so drop bottom
    r = ( x[0] + x[1] + x[2] ) / 3;

    if( x[3] == a )
    {
        return (b_d + c_d + d_d)/3;
    }else if( x[3] == b )
    {
        return (a_d + c_d + d_d)/3;
    }else if( x[3] == c )
    {
        return (a_d + b_d + d_d)/3;
    }else
    {
        return (a_d + b_d + c_d)/3;
    }
}

/*
float64 ExactCallBluffD::topThreeOfFour(float64 a, float64 b, float64 c, float64 d, float64 a_d, float64 b_d, float64 c_d, float64 d_d, float64 & r) const
{
    float64 x[4] = {a,b,c,d};

    std::sort(x,x+4); //Ascending, so drop first
    r = ( x[3] + x[2] + x[1] ) / 3;

    if( x[0] == a )
    {
        return (b_d + c_d + d_d)/3;
    }else if( x[0] == b )
    {
        return (a_d + c_d + d_d)/3;
    }else if( x[0] == c )
    {
        return (a_d + b_d + d_d)/3;
    }else
    {
        return (a_d + b_d + c_d)/3;
    }
}


///This function is used to maximize chance to CALL on a player-by-player basis
float64 ExactCallBluffD::bottomTwoOfThree(float64 a, float64 b, float64 c, float64 a_d, float64 b_d, float64 c_d, float64 & r) const
{
    if( b < a )
    {//b cannot be greatest
        if( a < c )
        {//c is greatest
            r = (a + b)/2;
            return (a_d + b_d)/2;
        }else //a > c, a > b
        {//a is greatest
            r = (c + b)/2;
            return (c_d + b_d)/2;
        }
    }
    else
    {//Since b >= a, a cannot be greatest
        if( b < c )
        {//c is greatest
            r = (b + a)/2;
            return (b_d + a_d)/2;
        }else
        {//b is greatest
            r = (a + c)/2;
            return (a_d + c_d)/2;
        }
    }
}
*/

// useMean is a table metric here (see RiskLoss), so always use callcumu
float64 ExactCallD::facedOdds_raise_Geom(const ChipPositionState & cps, float64 startingPoint, float64 incrRaise, float64 fold_bet, float64 opponents, bool bCheckPossible, bool bMyWouldCall, CallCumulationD * useMean) const
{

    float64 raiseto = cps.alreadyBet + incrRaise; //Assume this is controlled to be less than or equal to bankroll
    const playernumber_t N = tableinfo->handsDealt();
    const float64 avgBlind = tableinfo->table->GetBlindValues().OpportunityPerHand(N);


    return facedOdds_raise_Geom_forTest( startingPoint
                                        ,tableinfo->table->GetChipDenom()
                                        ,raiseto
                                        ,tableinfo->RiskLoss(cps.alreadyBet, cps.bankroll, opponents, raiseto, useMean, 0)
                                        ,avgBlind
                                        ,cps
                                        ,fold_bet
                                        ,opponents
                                        ,bCheckPossible
                                        ,bMyWouldCall
                                        ,useMean
                                        );
}

float64 ExactCallD::facedOdds_raise_Geom_forTest(float64 startingPoint, float64 denom, float64 raiseto, float64 riskLoss, float64 avgBlind, const ChipPositionState & cps, float64 fold_bet, float64 opponents, bool bCheckPossible, bool bMyWouldCall, CallCumulationD * useMean)
{
    if( raiseto >= cps.bankroll )
    {
        const float64 w_r = 1-1.0/RAREST_HAND_CHANCE; // because an opponent would still _always_ raise with the best hand.
        if (startingPoint < w_r) {
            return w_r;
        } else {
            return startingPoint;
        }
    }

    if( fold_bet > cps.bankroll )
    {
        fold_bet = cps.bankroll;
    }

    FacedOddsRaiseGeom a(denom);

	a.pot = cps.pot + (bMyWouldCall ? (raiseto-fold_bet) : 0);
    a.raiseTo = raiseto;
    a.fold_bet = fold_bet;
    a.bCheckPossible = bCheckPossible;
    a.riskLoss = (bCheckPossible) ? 0 : riskLoss;

	if( bMyWouldCall )
	{
		a.callIncrLoss = 1 - fold_bet/cps.bankroll;
		a.callIncrBase = (cps.bankroll + cps.pot)/(cps.bankroll - fold_bet); // = 1 + (pot - fold_bet) / (bankroll - fold_bet);
	}else
	{
		a.callIncrLoss = 0;
		a.callIncrBase = 0;
	}



    // We don't need to set w, because a.FindZero searches over w
    a.FG.waitLength.load(cps, avgBlind);
    a.FG.waitLength.opponents = opponents;
    a.FG.waitLength.meanConv = useMean;

    //a.FG.dw_dbet = 0; //We don't need this, unless we want the derivative of FG.f; Since we don't need any extrema or zeros of FG, we can set this to anything


/*
    const float64 fold_utility = bCheckPossible ? 0 : FG.f(fold_bet);

    if( fold_utility + raiseto < 0 )
    {//Folding is so pointless, you may as well play
        return 0;
    }
*/
    return a.FindZero(startingPoint,1.0, false);


}


//Here, dbetsize/dpot = 0
float64 ExactCallD::dfacedOdds_dpot_GeomDEXF(const ChipPositionState & cps, float64 incrRaise, float64 fold_bet, float64 w, float64 opponents, float64 dexfy,  bool bCheckPossible, bool bMyWouldCall, CallCumulationD * useMean) const
{
    if( w <= 0 ) return 0;
    const float64 raiseto = cps.alreadyBet + incrRaise;
	if( raiseto >= cps.bankroll - tableinfo->chipDenom()/2 ) return 0;


    if( fold_bet > cps.bankroll )
    {
        fold_bet = cps.bankroll;
    }

	const float64 retBet = bMyWouldCall ? (raiseto-fold_bet) : 0;

    const int8 N = tableinfo->handsDealt();
    const float64 avgBlind = tableinfo->table->GetBlindValues().OpportunityPerHand(N);

    //The pot can't be zero, so base_minus_1 can't be 0, so base can't be 1, so log(base) can't be zero
    const float64 base_minus_1 = (cps.pot+raiseto+retBet)/(cps.bankroll-raiseto);//base = (B+pot)/(B-betSize); = 1 + (pot+betSize)/(B-betSize);

    const float64 wN_1 = pow(w,opponents-1);
    float64 fw = wN_1 * w;
    float64 dfw = opponents * wN_1;

    const float64 A = dfw * log1p( base_minus_1 );
    const float64 C = fw/(cps.bankroll+cps.pot+retBet) ;
    const float64 h_times_remaining = pow( (cps.bankroll+cps.pot+retBet)/(cps.bankroll-raiseto), fw ) * (cps.bankroll - raiseto);



    if(bCheckPossible)
    {
        return (h_times_remaining*C*dexfy) / (h_times_remaining*A);
    }else
    {
    //USE FG for riskLoss
        float64 dRiskLoss_pot = std::numeric_limits<float64>::signaling_NaN();
        tableinfo->RiskLoss(cps.alreadyBet, cps.bankroll, opponents, raiseto, useMean, &dRiskLoss_pot);
        #ifdef DEBUGASSERT
          if (std::isnan(dRiskLoss_pot)) {
            std::cerr << "dRiskLoss_pot failed to initialize, please unit test tableinfo->RiskLoss("
              << cps.alreadyBet << " , " << cps.bankroll << " , " << opponents << " , " << raiseto << " , … , &dRiskLoss_pot)" << std::endl;
            exit(1);
          }
        #endif

        FoldGainModel myFG(tableinfo->chipDenom());

    //USE myFG for F_a and F_b
        myFG.waitLength.meanConv = useMean;
        myFG.waitLength.setW(w);
        myFG.waitLength.load(cps, avgBlind);
        myFG.waitLength.opponents = opponents;
        //myFG.dw_dbet = 0; //Again, we don't need this


        return (h_times_remaining*C*dexfy - myFG.F_b(fold_bet) + dRiskLoss_pot*dexfy) / (myFG.F_a(fold_bet) - h_times_remaining*A);
    }


}



float64 ExactCallD::facedOdds_call_Geom(const ChipPositionState & cps, float64 humanbet, float64 opponents, CallCumulationD * useMean) const
{

    if( humanbet >= cps.bankroll )
    {
        return 1 - 1.0/RAREST_HAND_CHANCE;
    }


    FacedOddsCallGeom a(tableinfo->chipDenom());
    a.B = cps.bankroll;
    a.pot = cps.pot;
//    a.alreadyBet = cps.alreadyBet;
    a.outsidebet = humanbet;
    a.opponents = opponents;

    const int8 N = tableinfo->handsDealt();
    const float64 avgBlind = tableinfo->table->GetBlindValues().OpportunityPerHand(N);
    a.FG.waitLength.load(cps, avgBlind);
    a.FG.waitLength.opponents = opponents;
    a.FG.waitLength.meanConv = useMean;
    //a.FG.dw_dbet = 0; //We don't need this, unless we want the derivative of FG.f; Since we don't need any extrema or zeros of FG, we can set this to anything

    return a.FindZero(0,1, false);
}

float64 ExactCallD::dfacedOdds_dbetSize_Geom(const ChipPositionState & cps, float64 humanbet, float64 dpot_dhumanbet, float64 w, float64 opponents, CallCumulationD * useMean) const
{
    if( w <= 0 ) return 0;
	if( humanbet >= cps.bankroll ) return 0;


    const int8 N = tableinfo->handsDealt();
    const float64 avgBlind = tableinfo->table->GetBlindValues().OpportunityPerHand(N);
    const float64 base_minus_1 = (cps.pot+humanbet)/(cps.bankroll-humanbet);//base = (B+pot)/(B-betSize); = 1 + (pot+betSize)/(B-betSize);

    FoldGainModel FG(tableinfo->chipDenom());
    FG.waitLength.setW(w);
    FG.waitLength.load(cps, avgBlind);
    FG.waitLength.opponents = opponents;
    FG.waitLength.meanConv = useMean;
    //FG.dw_dbet = 0; //Again, we don't need this

    const float64 wN_1 = pow(w,opponents-1);
    float64 fw = wN_1 * w;
    float64 dfw = opponents * wN_1;

    const float64 h = pow( (cps.bankroll+cps.pot)/(cps.bankroll-humanbet), fw );
    const float64 A = dfw * log1p( base_minus_1 );
    const float64 C = (  dpot_dhumanbet/(cps.bankroll+cps.pot) + 1/(cps.bankroll-humanbet)  ) * fw;

    return
    (  (cps.bankroll*C-C*humanbet-1)*h - FG.F_b(humanbet)  )
     /
    (  FG.F_a(humanbet) - (cps.bankroll-humanbet)*h*A  )
    ;
}

// useMean must be from the perspective of ChipPositionState, if any.
// Thus, it can't be handcumu but can be foldcumu or callcumu depending on how much information the opponent has.
float64 ExactCallBluffD::facedOdds_Algb(const ChipPositionState & cps, float64 betSize, float64 opponents, CallCumulationD * useMean)
{
    FacedOddsAlgb a(tableinfo->chipDenom());
    a.pot = cps.pot;
    //a.alreadyBet = cps.alreadyBet; //just for the books?
    a.betSize = betSize;

    const int8 N = tableinfo->handsDealt();
    const float64 avgBlind = tableinfo->table->GetBlindValues().OpportunityPerHand(N);
    a.FG.waitLength.load(cps, avgBlind);
    a.FG.waitLength.opponents = opponents;
    a.FG.waitLength.meanConv = useMean;
    //a.FG.dw_dbet = 0; //We don't need this...

    #if defined(DEBUG_TRACE_PWIN) && defined(DEBUG_TRACE_SEARCH)
		a.bTraceEnable = traceOut != 0 && useMean == 0;
    #endif

    return a.FindZero(0,1, false);
}
float64 ExactCallBluffD::facedOddsND_Algb(const ChipPositionState & cps, float64 incrbet, float64 dpot, float64 w, float64 opponents)
{
    if( w <= 0 ) return 0;

    const float64 wN_1 = pow(w,opponents-1);

    float64 fw = wN_1 * w;
    float64 dfw = opponents * wN_1;

    float64 bet = cps.alreadyBet + incrbet;
    if( bet > cps.bankroll )
    {
        bet = cps.bankroll;
    }


    const int8 N = tableinfo->handsDealt();
    const float64 avgBlind = tableinfo->table->GetBlindValues().OpportunityPerHand(N);
    FoldGainModel FG(tableinfo->chipDenom());
    FG.waitLength.load(cps, avgBlind);
    FG.waitLength.opponents = opponents;
    FG.waitLength.setW(w);
//derivative of algebraic uses FG_a and FG_b which are independent of useMean


    return (
                ( (1+FG.F_b(bet)) - ((dpot+1) * fw ) )
                        /
                ( (cps.pot+bet) * dfw - FG.F_a(bet) )
            );
}


float64 ExactCallD::RaiseAmount(const float64 betSize, int32 step) const
{

    float64 minRaiseDirect = tableinfo->minRaiseTo();
    float64 minRaiseBy = minRaiseDirect - tableinfo->callBet();
    float64 minRaiseBet = betSize - tableinfo->callBet();

    float64 raiseAmount =
      ( minRaiseBet < minRaiseDirect ) ?
      (
        betSize + minRaiseBy
      )
      :
      (
        betSize + minRaiseBet
      )
    ;

	if( step > 0 )
	{
		raiseAmount = raiseAmount + (raiseAmount - tableinfo->callBet());

		if( raiseAmount < betSize + minRaiseDirect )
		{
			//Two minraises above bet to call
			raiseAmount = betSize + minRaiseDirect;
		}
		--step;
		while(step > 0)
		{
			//You would be raising by {raiseAmount - callBet()};
			raiseAmount = raiseAmount + (raiseAmount - tableinfo->callBet());
			--step;
		}
	}

    if( raiseAmount > tableinfo->maxRaiseAmount() )
    {
        raiseAmount = tableinfo->maxRaiseAmount();
    }

	return raiseAmount;
}

struct FacedOdds {
  float64 pess;
  float64 mean;
  float64 rank;

  bool constexpr is_all_zero() const noexcept {
    return ((pess == 0.0) && (mean == 0.0) && (rank == 0.0));
  }

  bool constexpr assert_not_nan() const {
    if (!std::isnan(pess) && !std::isnan(mean) && !std::isnan(rank)) {
      return true;
    }
    #ifdef DEBUGASSERT
    else {
      std::cerr << "FacedOdds data is corrupt: pess=" << pess << " , mean=" << mean << " , rank = " << rank << std::endl;
      std::cerr << "prev_w_r.mean and prev_w_r.rank only become NaN after thisRaise passes oppBankRoll." << std::endl;
      exit(1);
    }
    #endif // DEBUGASSERT
	return false;
  }

  static FacedOdds constexpr all_nan() {
    FacedOdds invalid_val = {
      std::numeric_limits<float64>::signaling_NaN(),
      std::numeric_limits<float64>::signaling_NaN(),
      std::numeric_limits<float64>::signaling_NaN()
    };

    return invalid_val;
  }

  // TODO(from yuzisee): Raises are now Algb instead of Geom?
  void init_facedOdds_raise(ExactCallD & pr_call_pr_raiseby, const ChipPositionState & oppCPS, const FacedOdds &prev_w_r, const float64 oppRaiseMake, const float64 betSize, const bool bOppCouldCheck, const bool bMyWouldCall) {
    const float64 opponents = pr_call_pr_raiseby.tableinfo->handsToShowdownAgainst();

    this->pess = pr_call_pr_raiseby.facedOdds_raise_Geom(oppCPS,prev_w_r.pess, oppRaiseMake, betSize, opponents,bOppCouldCheck,bMyWouldCall,(&pr_call_pr_raiseby.fCore.foldcumu));
    #ifdef ANTI_CHECK_PLAY
    if( bOppCouldCheck )
    {
        this->mean = 1;
        this->rank = 1;
    } else
    #endif
    {
      this->mean = pr_call_pr_raiseby.facedOdds_raise_Geom(oppCPS,prev_w_r.mean, oppRaiseMake, betSize, opponents,bOppCouldCheck,bMyWouldCall,(&pr_call_pr_raiseby.fCore.callcumu));
      this->rank = pr_call_pr_raiseby.facedOdds_raise_Geom(oppCPS,prev_w_r.rank, oppRaiseMake, betSize, opponents,bOppCouldCheck,bMyWouldCall,0);
    }
  }

};

/*
    ::query generates the values noRaiseChance_A and totalexf along with their corresponding derivatives.
    ::query is called by exf() and pRaise() (along with their corresponding derivatives)
    ::query provides a mechanism for caching the result for multiple calls during the same configuration

    Note: callSteps represents firstFoldToRaise and is used in bMyWouldCall
*/
void ExactCallD::query(const float64 betSize, const int32 callSteps)
{
///Check for cache hit
	if( queryinput == betSize && querycallSteps == callSteps )
    {
		return;
	}

///Update cache
	this->queryinput = betSize;
	this->querycallSteps = callSteps;


    this->nearest = (betSize <= tableinfo->callBet() + tableinfo->chipDenom()/2) ? betSize : 0; //nearest can probably be ALWAYS callBet() to start!

    const float64 myexf = betSize;
    const float64 mydexf = 1;

    float64 overexf = 0;
    float64 overdexf = 0;

    this->totalexf = tableinfo->table->GetPotSize() - tableinfo->alreadyBet()  +  myexf; //alreadyBet is GetBetSize
    this->totaldexf = mydexf;

    #ifdef DEBUG_TRACE_DEXF
            if( traceOut != 0 ) *traceOut << "Begin Query with mydexf " << mydexf << endl;
    #endif

    this->noRaiseArraySize = 0;
    while( RaiseAmount(betSize,this->noRaiseArraySize) < tableinfo->maxRaiseAmount() )
    {
        ++noRaiseArraySize;
    }
    //This array loops until noRaiseArraySize is the index of the element with RaiseAmount(noRaiseArraySize) == maxBet()
    ++noRaiseArraySize; //Now it's the size of the array

    const size_t noRaiseArraySize_now = noRaiseArraySize;

    if( noRaiseChance_A != 0 ) delete [] noRaiseChance_A;
    if( noRaiseChanceD_A != 0 ) delete [] noRaiseChanceD_A;

    this->noRaiseChance_A = new float64[noRaiseArraySize_now];
    this->noRaiseChanceD_A = new float64[noRaiseArraySize_now];

    for( size_t i=0; i<noRaiseArraySize_now; ++i)
    {
        noRaiseChance_A[i] = 1;
        noRaiseChanceD_A[i] = 0;
    }


    ValueAndSlope * nextNoRaise_A = new ValueAndSlope[noRaiseArraySize_now];

    // Loop through each player to:
    //  + accumulate chance of NOT being raised by that player
    // as well as the
    //  + aggregate chance of being folded against. (conjunctive)
    int8 pIndex = tableinfo->playerID;
    tableinfo->table->incrIndex(pIndex);
    while( pIndex != tableinfo->playerID )
    {
      #ifdef DEBUG_TRACE_DEXF
		  if( traceOut != 0 )
		  {
		  	*traceOut << endl << "totaldexf is " << totaldexf << " overdexf is " << overdexf << endl;
		  	*traceOut << "\tPlayer " << (int)pIndex;
		  }
		  #endif

        this->accumulateOneOpponentPossibleRaises(pIndex, nextNoRaise_A, noRaiseArraySize_now, betSize, callSteps, &overexf, &overdexf);

        tableinfo->table->incrIndex(pIndex);
    }


    delete [] nextNoRaise_A;

    #ifdef DEBUG_TRACE_DEXF
    if( traceOut != 0 )  *traceOut << endl << "Final is " << totaldexf;
    #endif

    this->totalexf = totalexf - myexf - overexf;
    this->totaldexf = totaldexf - mydexf - overdexf;

    #ifdef DEBUG_TRACE_DEXF
    if( traceOut != 0 )  *traceOut << " adjusted to " << totaldexf << " by mydexf=" << mydexf << " and overdexf=" << overdexf << endl;
    #endif

    if( totalexf < 0 ) this->totalexf = 0; //Due to rounding error in overexf?
    if( totaldexf < 0 ) this->totaldexf = 0; //Due to rounding error in overexf?

    for( size_t i=0;i<noRaiseArraySize_now;++i)
    {
        this->noRaiseChanceD_A[i] *= noRaiseChance_A[i];
    }
}

void ExactCallD::accumulateOneOpponentPossibleRaises(const int8 pIndex, ValueAndSlope * const nextNoRaise_A, const size_t noRaiseArraySize_now, float64 betSize, const int32 callSteps, float64 * const overexf_out, float64 * const overdexf_out) {
  const float64 prevPot = tableinfo->table->GetPrevPotSize();
  const float64 opponents = tableinfo->handsToShowdownAgainst(); // The number of "opponents" that people will think they have (as expressed through their predicted showdown hand strength)

  ///Initialize player-specific callgain
  float64 nextexf = 0;
  float64 nextdexf = 0;

  ///Initialize player-specific (no)raisechange
  int8 oppRaiseChances = 0;
  int8 oppRaiseChancesPessimistic = 0;
  for(size_t i=0;i<noRaiseArraySize_now;++i)
  {
            nextNoRaise_A[i].v = 1; //Won't raise (by default)
            nextNoRaise_A[i].D_v = 0;
  }

  ///Initialize player bet state
  const Player * withP = tableinfo->table->ViewPlayer(pIndex);
  const float64 oppBetAlready = withP->GetBetSize();
  const float64 oppPastCommit = withP->GetVoluntaryContribution();
  const float64 oppBankRoll = withP->GetMoney(); //To predict how much the bet will be

  if( oppBankRoll - betSize < tableinfo->chipDenom()/4  ) // oppBankRoll <= betSize
  {
      oppRaiseChances = 0; //Not enough to raise more
      oppRaiseChancesPessimistic = 0;
  }
  else
  {
      if( betSize > tableinfo->callBet() && betSize < tableinfo->maxRaiseAmount() - tableinfo->chipDenom()/4  ) //this bet would be a raise, so anyone is allowed to reraise me
      {//(provided they have enough chips)
          oppRaiseChances = 1 + tableinfo->table->FutureRounds(); //this round, or any future round
          oppRaiseChancesPessimistic = 1;
      }else
      {
          oppRaiseChances = tableinfo->OppRaiseOpportunities(pIndex); //the opponent's only chance to raise is once in each future round
          oppRaiseChancesPessimistic = oppRaiseChances - tableinfo->table->FutureRounds();
      }
  }

  if( tableinfo->table->CanStillBet(pIndex) ) // Make sure the player is still fit to bet ( in this or any future round )
  {

      if( betSize - oppBankRoll < tableinfo->chipDenom()/4  ) // betSize <= oppBankRoll
      {	//Can still call, at least

          ChipPositionState oppCPS(oppBankRoll,totalexf,oppBetAlready,oppPastCommit, prevPot);

			///=========================
			///   1. Estimate noRaise
			///=========================
      // This populates nextNoRaise_A of this player, for all raise amounts. Step 3 will aggregate it them into noRaiseChance_A which combines all players.
          if( oppRaiseChances > 0 )
          { //The player can raise you if he hasn't called yet, OR you're (hypothetically) raising

              //if( callBet() > 0 && oppBetAlready == callBet() ) bInBlinds = false;

              float64 prevRaise = 0.0;
              FacedOdds prev_w_r = {0.0, 0.0, 0.0};
              ///Check for each raise percentage
              for( size_t i_step=0;i_step<noRaiseArraySize_now;++i_step)
              {
                  const int32 i = i_step;
                  const bool bMyWouldCall = i < callSteps;
                  const float64 thisRaise = RaiseAmount(betSize,i);
                  const float64 oppRaiseMake = thisRaise - oppBetAlready;
                  if( oppRaiseMake <= 0 ) {
                      nextNoRaise_A[i_step].v = 0.0; // well then we're guaranteed to hit this amount
                      nextNoRaise_A[i_step].D_v = 0.0;
                      prevRaise = 0;
#ifdef DEBUGASSERT
                      if (!prev_w_r.is_all_zero()) {
                          std::cerr << "How did prev_w_r_mean get set while we're still in oppRaiseMake <= 0.0?" << std::endl;
                          exit(1);
                      }
#endif // DEBUGASSERT
                  } else
                  {
                      if(thisRaise <= oppBankRoll)
                      {

                          const bool bOppCouldCheck = (betSize == 0) || /*(betSize == callBet())*/(oppBetAlready == betSize);//If oppBetAlready == betSize AND table->CanRaise(pIndex, playerID), the player must be in the blind. Otherwise,  table->CanRaise(pIndex, playerID) wouldn't hold
                                                                                                                                 //The other possibility is that your only chance to raise is in later rounds. This is the main force of bWouldCheck.

                         if (prev_w_r.assert_not_nan()) {
                          FacedOdds w_r_facedodds; // TODO(from joseph): Rename to noraiseRank or, rename noraiseRankD to w_r_facedodds_D
                          w_r_facedodds.init_facedOdds_raise(*this, oppCPS, prev_w_r, oppRaiseMake, betSize, bOppCouldCheck, bMyWouldCall);

                          const float64 noraiseRankD = dfacedOdds_dpot_GeomDEXF( oppCPS,oppRaiseMake,tableinfo->callBet(), w_r_facedodds.rank, opponents, totaldexf, bOppCouldCheck, bMyWouldCall ,0);

                                const ValueAndSlope noraisePess = {
                                  1.0 - fCore.foldcumu.Pr_haveWinPCT_strictlyBetterThan(w_r_facedodds.pess - EPS_WIN_PCT)  // 1 - ed()->Pr_haveWinPCT_orbetter(w_r_pess)
                                  ,
                                  fCore.foldcumu.Pr_haveWorsePCT_continuous(w_r_facedodds.pess - EPS_WIN_PCT).second * dfacedOdds_dpot_GeomDEXF( oppCPS,oppRaiseMake,tableinfo->callBet(),w_r_facedodds.pess, opponents,totaldexf,bOppCouldCheck, bMyWouldCall, (&fCore.foldcumu))
                                };

                                const ValueAndSlope noraiseMean = {
                                  1.0 - fCore.callcumu.Pr_haveWinPCT_strictlyBetterThan(w_r_facedodds.mean - EPS_WIN_PCT) // 1 - ed()->Pr_haveWinPCT_orbetter(w_r_mean)
                                  ,
                                  fCore.callcumu.Pr_haveWorsePCT_continuous(w_r_facedodds.mean - EPS_WIN_PCT).second * dfacedOdds_dpot_GeomDEXF( oppCPS,oppRaiseMake,tableinfo->callBet(),w_r_facedodds.mean, opponents,totaldexf,bOppCouldCheck, bMyWouldCall, (&fCore.callcumu))
                                };

                          //nextNoRaise_A[i_step].v = w_r_facedodds.rank;
                          //nextNoRaise_A[i_step].D_v = noraiseRankD;

                          // But the opponent may or may not know your hand!
                          // Unforunately, knowing your hand is weak doesn't always make more opponents want to raise.
                          // However, we can guide the choice between callcumu and foldcumu, in this case, adversarially:
                          //   If you know you'd fold (weak hand), let the opponent raise the worse amount (larger)
                          //   If you know you'd call (good hand), let the opponent raise the worse amount (smaller)

                                const ValueAndSlope noraise = bMyWouldCall ?
                                        ValueAndSlope::lesserOfTwo(noraisePess, noraiseMean) : // I would call. I want them to raise. (Adversarial is smaller)
                                        ValueAndSlope::greaterOfTwo(noraisePess, noraiseMean) // I won't call. I want them not to raise. (Adversarial is larger)
                                    ;

                          nextNoRaise_A[i_step].v = (noraise.v + w_r_facedodds.rank)/2;
                          nextNoRaise_A[i_step].D_v = (noraise.D_v + noraiseRankD)/2;

                          // nextNoRaise should be monotonically increasing. That is, the probability of being raised all-in is lower than the probabilty of being raised at least minRaise.
                          if (i_step>0) {
                              //if (nextNoRaise_A[i_step].v < nextNoRaise_A[i_step-1].v) {
                                  // The returned total cumulative probability distributions won't be allowed to drop.
                                  // However, this can happen in many cases.
                                  // For example, say you have a Q3o
                                  // A bunch of the _better_ hands have a slightly better chance to win against most hands, but although they fare better against random hands they fare just the same against your Q3o.
                                  // When this happens it means: if the opponent knew your hand, fewer of them would want to raise -- even if those that do would beat you by more or those that don't have better odds against random hands.
                              //}
#ifdef DEBUGASSERT
                              if (!(nextNoRaise_A[i_step-1].v <= nextNoRaise_A[i_step].v)) {
                                  std::cerr << "Invalid nextNoRaise_A for player " << tableinfo->table->ViewPlayer(pIndex)->GetIdent() << " raising to " << thisRaise << std::endl;
                                  // If you get here, look at prev_w_r_mean, prev_w_r_rank, etc. to help debug.
                                  // They are populated just below.
                                  // Also, check callSteps!
                                  for( size_t k=0;k<=i_step;++k) {
                                      std::cerr << "nextNoRaise_A[" << (int)k << "]=" << nextNoRaise_A[k].v << std::endl;
                                  }
                                  exit(1);
                              }
#endif //DEBUGASSERT

                          }

                          prevRaise = thisRaise;

                          prev_w_r = w_r_facedodds;
                         } // endif prev_w_r.assert_not_nan()

                      }else
                      { // raising this amount would put player[pIndex] all-in.
                          const float64 oppAllInMake = oppBankRoll - oppBetAlready;

                          if (prevRaise > 0 && oppBankRoll > prevRaise && oppAllInMake > 0)
                          { //This is the precise bet of the all-in raise. Higher raiseAmounts won't be raised by this player, no matter what hand he/she has regardless of their chance to win the showdown.

                              float64 w_r = 1-1.0/RAREST_HAND_CHANCE;

                              //float64 noraiseMean = e->weakestOpponent(); //What are his odds against the weakest opponent

                              //if(noraiseMean < w_r){ noraiseMean = 0; }

                              nextNoRaise_A[i_step].v = w_r;//(noraiseMean+w_r)/2;

                              // ... but ensure that nextNoRaise remains monotonic
                              if (i_step > 0) {
                                  const size_t i_cascade = i_step-1;
                                  #ifdef DEBUGASSERT
                                    if (noRaiseArraySize_now <= i_cascade ) {
                                      std::cerr << "We need this assertion to solve a 'core.UndefinedBinaryOperatorResult' compiler (clang++ static analyzer) warning, but it's already impossible because the for-loop above only goes up to: i=" << static_cast<int>(i) << " < noRaiseArraySize=" << static_cast<int>(noRaiseArraySize) << std::endl;
                                      exit(1);
                                    } else
                                  #endif
                                  if (nextNoRaise_A[i_step].v < nextNoRaise_A[i_cascade].v) {
                                      nextNoRaise_A[i_step].v = nextNoRaise_A[i_cascade].v;
                                      // i.e.
                                      // nextNoRaise_A[i].v = nextNoRaise_A[i-1].v;
                                  }
                              }

                              nextNoRaise_A[i_step].D_v = 0;
                          }

                          prevRaise = 0;
                          prev_w_r = FacedOdds::all_nan();

                      } // end of block: if (thisRaise <= oppBankRoll), else ...

                  } // end of block: if (oppRaiseMake <= 1), else ...
              }//end of for loop
          }

			///=====================
			///   2. Estimate exf
			///=====================

				///Check for most likely call amount
          const float64 oppBetMake = betSize - oppBetAlready;
				//To understand the above, consider that totalexf includes already made bets

          #ifdef DEBUG_TRACE_DEXF
          if( traceOut != 0 )  *traceOut << " to bet " << oppBetMake << "more";
          #endif

          if( oppBetMake <= std::numeric_limits<float64>::epsilon() )
          { //Definitely call
              nextexf = 0;
              nextdexf = 1;

              #ifdef DEBUG_TRACE_DEXF
              if( traceOut != 0 )  *traceOut << " ALREADY CALLED" << endl ;
              #endif
          }else
          {

              // Since this is Pr{call}, we're using table stats, i.e. ed()
              // Nothing special going on here.
              // TODO(from yuzisee): if you feel that predicted calls are too loose, we can switch to ef() which is more adversarial.f
              // We don't use RANK here. RANK might overestimate the amount of calls from strong hands.
              const float64 w = facedOdds_call_Geom(oppCPS,betSize, opponents, ed());
              nextexf = ed()->Pr_haveWinPCT_strictlyBetterThan(w - EPS_WIN_PCT);


              nextdexf = nextexf + oppBetMake * (- ed()->Pr_haveWorsePCT_continuous(w - EPS_WIN_PCT).second)
                                  * dfacedOdds_dbetSize_Geom(oppCPS,betSize,totaldexf,w, opponents, ed());

              nextexf *= oppBetMake;
              if( oppBetAlready + nextexf > nearest )
              {
                  this->nearest = oppBetAlready + nextexf;
              }

              #ifdef DEBUG_TRACE_DEXF
              //if( traceOut != 0 )  *traceOut << " nextdexf=" << nextdexf << endl;
              #endif

          }
				//End of else, blocked executed UNLESS oppBetMake <= 0

      }else
      {///Opponent would be all-in to call this bet
          const float64 oldpot = tableinfo->table->GetPrevPotSize();
          const float64 effroundpot = (totalexf - oldpot) * oppBankRoll / betSize;
          const float64 oppBetMake = oppBankRoll - oppBetAlready;

          ChipPositionState oppmaxCPS(oppBankRoll,oldpot + effroundpot,oppBetAlready,oppPastCommit, prevPot);

          nextexf = ed()->Pr_haveWinPCT_strictlyBetterThan( facedOdds_call_Geom(oppmaxCPS,oppBankRoll, opponents,ed()) - EPS_WIN_PCT );

				nextexf *= oppBetMake ;

          if( oppBetAlready + nextexf + (betSize - oppBankRoll) > nearest )
          {
              this->nearest = oppBetAlready + nextexf + (betSize - oppBankRoll);
          }

          nextdexf = 0;

              #ifdef DEBUG_TRACE_DEXF
              if( traceOut != 0 )  *traceOut << " Is ALL IN" << endl;
              #endif

				//Obviously the opponent won't raise...  ie. NoRaise = 100%
				// (nextNoRaise , nextNoRaiseD ) is already (1,0)
      }

			///===================================
			///   3. Tally/Aggregate/Accumulate
			///===================================


      //lastexf = nextexf;
      this->totalexf += nextexf;

              #ifdef DEBUG_TRACE_DEXF
              //if( traceOut != 0 )  *traceOut << "totaldexf was " << totaldexf;
              #endif

      //lastdexf = nextdexf;
      this->totaldexf += nextdexf;


              #ifdef DEBUG_TRACE_DEXF
              //if( traceOut != 0 )  *traceOut << " is " << totaldexf << ",  last added " << nextdexf << endl;
              //if( traceOut != 0 )  *traceOut << " is " << totaldexf << endl;
              //if( traceOut != 0 )  *traceOut << " last added " << nextdexf << endl;
              //if( traceOut != 0 )  *traceOut << endl;
              #endif


      const float64 oppInPot = oppBetAlready + nextexf;
      if( oppInPot - betSize > std::numeric_limits<float64>::epsilon() )
      {
          *overexf_out += oppInPot - betSize;
          *overdexf_out += nextdexf;
      }

  } // end if CanStillBet(pIndex)

  for( size_t i_step=0;i_step<noRaiseArraySize_now;++i_step)
  {
      const int32 i = i_step;
      //Always be pessimistic about the opponent's raises.
      //If being raised against is preferable, then expect an aware opponent not to raise into you in later rounds -- since they'd be giving you money.
      //If being raised against is undesirable, expect an aware opponent to raise you early and often -- since you are giving them push-opportunity
      const bool bMyWouldCall = (i < callSteps);

      const int8 oppRaiseChancesAware = bMyWouldCall ? oppRaiseChancesPessimistic : oppRaiseChances;
      // Increasing oppRaiseChancesAware decreases noRaiseChance, which increases the chance we expect to be raised at a certain price.


      //At this point, each nextNoRaise is 100% unless otherwise adjusted.
      const float64 noRaiseChance_adjust = (nextNoRaise_A[i_step].v < 0) ? 0 : pow(nextNoRaise_A[i].v,oppRaiseChancesAware);


#ifdef DEBUGASSERT
      if (oppRaiseChancesAware < 0) {
          std::cerr << "Invalid oppRaiseChancesAware " << (int)oppRaiseChancesAware << " for player " << tableinfo->table->ViewPlayer(pIndex)->GetIdent() << std::endl;
          exit(1);
      }

      if (i_step>0
           && (i != callSteps) // at the callSteps boundary, sometimes the raise probability spikes (and thus the noRaise probability drops) before continuing to converge toward "very unlikely to raise all-in" and thus noRaise --> 1.0 again.
          ) {
          if (!(
                noRaiseChance_A[i_step-1]
                <=
                noRaiseChance_A[i_step] * noRaiseChance_adjust
                )) {
              std::cerr << "Invalid noRaiseChance_A for player " << tableinfo->table->ViewPlayer(pIndex)->GetIdent() << std::endl;
              for( size_t k=0;k<i_step;++k) {
                  std::cerr << "noRaiseChance_A[" << (int)k << "]=" << noRaiseChance_A[k] << std::endl;
              }
              std::cerr << "vs. noRaiseChance_A[" << (int)i << "] is " << noRaiseChance_A[i_step] << " --> " << (noRaiseChance_A[i_step] * noRaiseChance_adjust) << std::endl;
              exit(1);
          }
      }
#endif //DEBUGASSERT

      noRaiseChance_A[i_step] *= noRaiseChance_adjust;
      if( std::fabs(noRaiseChance_A[i_step]) <= std::numeric_limits<float64>::epsilon() ) //and nextNoRaiseD == 0
      {
          noRaiseChanceD_A[i_step] = 0;
      }else
      {
          noRaiseChanceD_A[i_step] += nextNoRaise_A[i_step].D_v/nextNoRaise_A[i_step].v  *   oppRaiseChancesAware; //Logarithmic differentiation
      }
  }
} // end accumulateOneOpponentPossibleRaises

void ExactCallBluffD::query(const float64 betSize)
{
    ExactCallD::query(betSize, querycallSteps);

    if( queryinputbluff == betSize ) { return; }

	queryinputbluff = betSize;


	///This is one of the iterators over the upcoming while loop.
	///It represents the count of players an opponent would be folding to.
    float64 countMayFold = tableinfo->table->NumberInHandInclAllIn() - 1 ;


    const float64 myexf = betSize;
    const float64 mydexf = 1;

#ifdef DEBUG_TRACE_PWIN
		if( traceOut != 0 ) *traceOut << "Begin Query" << endl;
#endif

    float64 nextFold = -1;
    float64 nextFoldPartial = 0;
    allFoldChance = 1;
    allFoldChanceD = 0;
    const float64 origPot = tableinfo->table->GetPotSize() - tableinfo->alreadyBet()  +  myexf;
    const float64 origPotD = mydexf;
    const float64 prevPot = tableinfo->table->GetPrevPotSize();


    if( betSize < tableinfo->minRaiseTo() - tableinfo->chipDenom()/4 || tableinfo->callBet() >= tableinfo->table->GetMaxShowdown(tableinfo->maxBet()) )
    { //Bet is a call, no chance of oppFold
        allFoldChance = 0;
        allFoldChanceD = 0;
        nextFold = 0;
        // nextFoldPartial already initialized to 0 above
        // TODO(from joseph_huang): Early return?

		#ifdef DEBUG_TRACE_PWIN
		    if( traceOut != 0 ) *traceOut << "N/A. Call doesn't pWin" << endl;
		#endif
    }

    int8 pIndex = tableinfo->playerID;
    tableinfo->table->incrIndex(pIndex);
    while( pIndex != tableinfo->playerID && allFoldChance > 0)
    {
		#ifdef DEBUG_TRACE_PWIN
		if( traceOut != 0 )
		{
			*traceOut << "allFoldChance is " << allFoldChance << ",  last multiplied by " << nextFold << endl;
			*traceOut << "\tPlayer " << (int)pIndex;
		}
		#endif

        const float64 oppBetAlready = tableinfo->table->ViewPlayer(pIndex)->GetBetSize();
        const float64 oppPastCommit = tableinfo->table->ViewPlayer(pIndex)->GetVoluntaryContribution();

        const float64 nLinear =/* 1/countToBeat ;*/ (countMayFold + insuranceDeterrent);

        if( tableinfo->table->CanStillBet(pIndex) )
        {///Predict how much the bet will be
            const float64 oppBankRoll = tableinfo->table->ViewPlayer(pIndex)->GetMoney();

		#ifdef DEBUG_TRACE_PWIN
		    if( traceOut != 0 ) *traceOut << " can still bet" << endl;
		#endif

            if( betSize < oppBankRoll )
            {

		#ifdef DEBUG_TRACE_PWIN
		    if( traceOut != 0 ) *traceOut << "\t\tCan raise" << endl;
		#endif

                ChipPositionState opporigCPS(oppBankRoll,origPot,oppBetAlready,oppPastCommit,prevPot);
                const float64 oppBetMake = betSize - oppBetAlready;
                //To understand the above, consider that totalexf includes already made bets

                // TODO(from yuzisee): Since this is Pr{opponentFold}, do we use Algb still? See updated PureGainStrategy.
                float64 w_rank = facedOdds_Algb(opporigCPS,oppBetMake,nLinear,0);
                float64 w_mean = facedOdds_Algb(opporigCPS,oppBetMake,nLinear,ed());
                float64 w_pess = facedOdds_Algb(opporigCPS,oppBetMake,nLinear,ef());
                if( nLinear <= 0 )
                {
                    w_mean = 1.0;
                    w_pess = 1.0;
                    w_rank = 0;
                }
                const float64 dw_dbetSize_pess = facedOddsND_Algb( opporigCPS,oppBetMake,origPotD,w_pess, nLinear); // for eaFold
                const float64 dw_dbetSize_mean = facedOddsND_Algb( opporigCPS,oppBetMake,origPotD,w_mean, nLinear);
                const float64 dw_dbetSize_rank = facedOddsND_Algb( opporigCPS,oppBetMake,origPotD,w_rank, nLinear);


                    //float64 oppCommitted = stagnantPot() - table->ViewPlayer(pIndex)->GetContribution();
                    //oppCommitted = oppCommitted / (oppCommitted + oppBankRoll);
                    //ea-> is if they know your hand
                    std::pair<float64,float64> eaFold = fCore.foldcumu.Pr_haveWorsePCT_continuous(w_pess); // (1 - ef()->Pr_haveWinPCT_orbetter_continuous( w_mean ));// *(1 - oppCommitted);
                    //e-> is if they don't know your hand
                    std::pair<float64,float64> meanFold = ed()->Pr_haveWorsePCT_continuous(w_mean); //1 - ed()->Pr_haveWinPCT_orbetter( w_mean );
                    //w is if they don't know your hand
                    const float64 rankFold = w_rank;
                    //handRarity is based on if they know your hand
                    const float64 eaRkFold = 1-tableinfo->handRarity;

					#ifdef DEBUG_TRACE_PWIN
						if( traceOut != 0 )
						{
						    *traceOut << "\t\tWillFold (eaFold,meanFold,rankFold,eaRkFold) = (" << eaFold << "," << meanFold << "," << rankFold << "," << eaRkFold << ")" << endl;
						    *traceOut << "\t\t\tusing w_rank = " << w_rank << endl;
						}

					#endif

//(nLinear <= 0 && wn > 1) ? 0 :


                    const float64 rankFoldPartial = dw_dbetSize_rank;
                    meanFold.second *= dw_dbetSize_mean;
                    eaFold.second *= dw_dbetSize_pess;
                    const float64 eaRkFoldPartial = 0;

                    ///topTwoOfThree is on a player-by-player basis
                    nextFoldPartial = bottomThreeOfFour(eaFold.first,meanFold.first,rankFold,eaRkFold,eaFold.second,meanFold.second,rankFoldPartial,eaRkFoldPartial,nextFold);
                    //nextFold = (eaFold+meanFold+rankFold+eaRkFold)/4;
                    //nextFoldPartial=(eaFoldPartial+meanFoldPartial+rankFoldPartial+eaRkFoldPartial)/4;

                    //nextFold = sqrt((eaFold*eaFold+rankFold*rankFold)/2);
                    //nextFoldPartial = (eaFold*eaFoldPartial+rankFold*rankFoldPartial)*sqrt(2)/nextFold ;
                    //nextFold = (meanFold+rankFold+eaFold)/3;
                    //nextFoldPartial = (meanFoldPartial+rankFoldPartial+eaFoldPartial)/3;

					#ifdef DEBUG_TRACE_PWIN
						if( traceOut != 0 ) *traceOut << "\t\tPicked = (" << nextFold << ")" << endl;
					#endif


            }else
            {///Opponent would be all-in to call this bet

				#ifdef DEBUG_TRACE_PWIN
					if( traceOut != 0 ) *traceOut << "\t\tShort stacked" << endl;
				#endif

                const float64 oldpot = tableinfo->table->GetPrevPotSize();
                const float64 effroundpot = (origPot - oldpot) * oppBankRoll / betSize;
                const float64 oppBetMake = oppBankRoll - oppBetAlready;

                if( oppBankRoll < tableinfo->minRaiseTo() - tableinfo->chipDenom()/4 )
                { //Guaranteed call
                    allFoldChance = 0;
                    allFoldChanceD = 0;
                    nextFold = 0;
                }else
                {
                    ChipPositionState opporigmaxCPS(oppBankRoll,oldpot + effroundpot,oppBetAlready,oppPastCommit,prevPot);

                    float64 w_mean = facedOdds_Algb(opporigmaxCPS,oppBetMake, nLinear,ed());
                    float64 w_rank = facedOdds_Algb(opporigmaxCPS,oppBetMake, nLinear,0);

                    if( nLinear <= 0 )
                    {
                        w_mean = 1;
                        w_rank = 1;
                    }

                    //float64 oppCommitted = table->ViewPlayer(pIndex)->GetContribution();
                    //oppCommitted = oppCommitted / (oppCommitted + oppBankRoll);
                    std::pair<float64, float64> eaFold = fCore.foldcumu.Pr_haveWorsePCT_continuous(w_mean); //(1 - ef()->Pr_haveWorsePCT_continuous( w_mean ));//*(1 - oppCommitted);
                    std::pair<float64, float64> meanFold = ed()->Pr_haveWorsePCT_continuous(w_mean); //1 - ed()->Pr_haveWinPCT_orbetter( w_mean );
                    const float64 rankFold = w_rank;
                    const float64 eaRkFold = 1-tableinfo->handRarity;

                    ///topTwoOfThree is on a player-by-player basis
                    bottomThreeOfFour(eaFold.first,meanFold.first,rankFold,eaRkFold,0,0,0,0,nextFold);
//                    nextFold = (eaFold+meanFold+rankFold+eaRkFold)/4;


                    //nextFold = (meanFold+rankFold+eaFold)/3;
                    //nextFold = sqrt((eaFold*eaFold+rankFold*rankFold)/2);

                    if( nextFold > 1 ) nextFold = 1;

                }
                nextFoldPartial = 0;
            }


            countMayFold -= 1;
//            countToBeat  += 1;

            if(
               (allFoldChance == 0 && allFoldChanceD == 0) || (nextFold == 0 && nextFoldPartial == 0)
              )
            {
                allFoldChance = 0;
                allFoldChanceD = 0;
            }else
            {
                allFoldChance *= nextFold;
                allFoldChanceD += nextFoldPartial / nextFold;
            }


        }else
        {//Player can't bet anymore

            #ifdef DEBUG_TRACE_PWIN
		        if( traceOut != 0 ) *traceOut << " ignored" << endl;
		    #endif

            if( oppBetAlready > 0 )
            {//Must have gone all-in just now, or at least this round
                #ifdef DEBUG_TRACE_PWIN
		            if( traceOut != 0 ) *traceOut << "N/A! All-in detected." << endl;
		        #endif

                allFoldChance = 0;
                allFoldChanceD = 0;
            }
        }


        tableinfo->table->incrIndex(pIndex);
    }

    allFoldChanceD *= allFoldChance;

}

float64 ExpectedCallD::PushGain()
{
    const float64 baseFraction = betFraction(table->GetPotSize() - alreadyBet());

#ifdef BLIND_ADJUSTED_FOLD
    const float64 rawWinFreq = (1.0 / table->NumberAtTable()) ; //It's raw, so it's defined by NumberAtTable
    const float64 blindsPow = rawWinFreq*(1.0 - 1.0 / table->NumberAtTable());

    const float64 bigBlindFraction = betFraction( table->GetBlindValues().GetBigBlind() );
    const float64 smallBlindFraction = betFraction( table->GetBlindValues().GetSmallBlind() );
#ifdef CONSISTENT_AGG
    const float64 handFreq = 1+handRarity;//1/(1-handRarity);
    if( handRarity >= 1 ) //all hands are better than this one
    {
           //1326 is the number of hands possible
//		#ifdef PURE_BLUFF
            return 1;

//		#else
//			return (1 + baseFraction + (bigBlindFraction+smallBlindFraction)*blindsPow*RAREST_HAND_CHANCE);
//		#endif
    }
	#ifdef PURE_BLUFF
        const float64 //totalFG;
        //if( blindsPow*handFreq > 1 )
        //{
            totalFG = (1 + (baseFraction+(bigBlindFraction+smallBlindFraction)*blindsPow)*handFreq);
        //}else
        //{
        //    totalFG = (1 + baseFraction+(bigBlindFraction+smallBlindFraction*blindsPow)*handFreq);
        //}
	#else
		const float64 totalFG = (1 + baseFraction + ((bigBlindFraction+smallBlindFraction)*blindsPow)*handFreq);
	#endif

#else
    const float64 blindsGain = (1 + baseFraction + bigBlindFraction)*(1 + baseFraction + smallBlindFraction);
        //const float64 blindPerHandGain = ( ViewTable().GetBigBlind()+ViewTable().GetSmallBlind() ) / myMoney / ViewTable().GetNumberAtTable();
    const float64 totalFG = pow(1+baseFraction,1-2*blindsPow)*pow(blindsGain,blindsPow);

#endif



    return totalFG;
#else
    return 1 + baseFraction;
#endif
}

float64 ExactCallBluffD::pWin(const float64 betSize)
{

    query(betSize);

    ///Try pow(,impliedFactor) maybe
    return allFoldChance;//*impliedFactor;
}

float64 ExactCallBluffD::pWinD(const float64 betSize)
{
    query(betSize);

    return allFoldChanceD;//*impliedFactor;
}


float64 ExactCallD::pRaise(const float64 betSize, const int32 step, const int32 callSteps)
{
    query(betSize,callSteps);

	if( RaiseAmount( betSize, step ) >= tableinfo->maxBet() - tableinfo->chipDenom()/2 )
    { return 0; } //You don't care about raises if you are all-in
	else if( step >= noRaiseArraySize )
    { return std::numeric_limits<float64>::signaling_NaN(); }

    return 1.0-noRaiseChance_A[step];
}

float64 ExactCallD::pRaiseD(const float64 betSize, const int32 step, const int32 callSteps)
{
    query(betSize,callSteps);

    if( step < noRaiseArraySize ) return -noRaiseChanceD_A[step];
    else return 0;
}




float64 ExactCallD::exf(const float64 betSize)
{
    query(betSize,querycallSteps);

    return totalexf*impliedFactor + (betSize - nearest);
}

float64 ExactCallD::dexf(const float64 betSize)
{

    query(betSize,querycallSteps);


    return totaldexf*impliedFactor;
}


// RiskPrice is only used as a heuristic and only for the bot itself during stratPosition so it's fine to use either ef() here (if pessimistic) or ed() here (if no information has been revealed yet)
float64 ExactCallBluffD::RiskPrice(const ExpectedCallD &tableinfo, CallCumulationD * foldcumu_caching)
{//At what price is it always profitable to fold if one has the average winning hand?
	const int8 Ne_int = tableinfo.table->NumberStartedRound().inclAllIn() - 1; // This is the number of players you'd have to beat, regardless of who is betting.
    const float64 Ne = static_cast<float64>(Ne_int);

    const float64 estSacrifice = (tableinfo.table->GetPotSize() - tableinfo.alreadyBet());

    const float64 maxStack = tableinfo.table->GetAllChips();

    FoldGainModel FG(tableinfo.chipDenom());
	//FG.bTraceEnable = true;

    // TODO(from yuzisee): MEAN (callcumu) vs. RANK vs "pessimistic" to:
    //   + match call and fold?
    //   + distinguish between (information given to opponent)
    // ef() is pessimistic
    // NOTE: below that opponents = 1, so RANK is not necessary here.
    FG.waitLength.setW( foldcumu_caching->nearest_winPCT_given_rank(1.0 - 1.0/Ne) ); //If you're past the flop, we need definitely consider only the true number of opponents
    FG.waitLength.amountSacrificeVoluntary = estSacrifice; //rarity() already implies the Ne
	FG.waitLength.amountSacrificeForced = 0; //estSacrifice*rarity() already implies a forced avgBlinds
    FG.waitLength.bankroll = maxStack;
    FG.waitLength.opponents = 1;
    FG.waitLength.prevPot = tableinfo.table->GetPrevPotSize();
    FG.waitLength.meanConv = foldcumu_caching;
    const float64 riskprice = FG.FindZero(tableinfo.table->GetMinRaise() + tableinfo.callBet(),maxStack/2, true);


    FG.f(riskprice);
    if( FG.n > 0 )
    {
        return riskprice;
    }else
    {
		FG.f(riskprice+tableinfo.chipDenom());
		if( FG.n > 0 )
		{
			return ( riskprice+tableinfo.chipDenom() );
		}else
		{
			return maxStack;
		}
    }


}

// Count the number of hands that would be committed along with mine if I made a huge bet right now.
// Arguments:
//   fBettor:
//     The player considering the bet myBetSize
//   fTable:
//     A view of the table
//   myBetSize:
//     The size of bet being considered
//   fReceiver:
//     Evaluate the situation where this overbet has gotten around to player "fReceiver".
// Returns:
//   We want to know how many opponents would player "fReceiver" expect to be against such an overbet?
//   It would only really be more than 1.0 if there were a sizeable amount of chips already committed into the pot.
static float64 handsCommitted(playernumber_t fBettor, const HoldemArena & fTable, float64 myBetSize, playernumber_t fReceiver) {
    float64 result = 1.0; // Include fBettor's hypothetical bet completely.

    playernumber_t pIndex = fBettor;
    // Loop over every opponent of fBettor ...
    do
    {
        fTable.incrIndex(pIndex);

        // ... but only showdown-eligible opponents
        if (!fTable.IsInHand(pIndex)) {
            continue;
        }

        // ... who have the potential to call your bet ...
        if (fTable.ViewPlayer(pIndex)->GetMoney() < myBetSize) {
            continue;
        }

        // ... and don't include fReceiver.
        if (pIndex == fReceiver) {
            continue;
        }

        // Finally, skip fBettor as well.
        // We've already included that bet as 1.0 regardless of the actual bet on the table, since this is the hypothesis bet.
        if (pIndex == fBettor) {
            continue;
        }


            // Get existing bet...
            const float64 commitmentLevel = fTable.ViewPlayer(pIndex)->GetBetSize();

            if (myBetSize >= commitmentLevel) {
                // ... and count it as up to 1.0 player (if we're calling them)
                result += commitmentLevel / myBetSize;
            } else {
                // You're betting less than the high bet? You must be all in!
                // Count whoever you'd be calling to be playing as well anyway.
                result += 1.0;
            }
    }while( pIndex != fBettor);

    return result;
}

// Inputs:
//   //betSize1: the bet size the opponent has just made (this is usually just fTable.ViewPlayer(pIndex)->GetBetSize(), but in the case of StateModel::g_raised often we want to simulate whether they have already raised themselves)
//   betSize: the bet size the opponent has to call
// Outputs:
//   fHandsToBeat
void OpponentHandOpportunity::query(const float64 betSize) {
    if (fLastBetSize == betSize) {
        return;
    }
    fLastBetSize = betSize;

    const float64 tableStrength = fTable.NumberAtFirstActionOfRound().inclAllIn();
    const float64 tableStrengthToBeat = tableStrength - 1.0;

    const float64 avgBlinds = fTable.GetBlindValues().OpportunityPerHand(fTable.NumberAtTable());

    playernumber_t pIndex = fIdx;
    fTable.incrIndex(pIndex);
    float64 totalOpposingHandOpportunityCount = 0.0;
    float64 totalOpposingHandOpportunityCount_dbetSize = 0.0;
    // Loop over every opponent ...
    while( pIndex != fIdx)
    {
        // ... but only showdown-eligible opponents
        if (fTable.IsInHand(pIndex)) {

            const float64 theirMoney = fTable.ViewPlayer(pIndex)->GetMoney();
            const float64 betSizeFacingThem = (betSize < theirMoney) ? betSize : theirMoney;

            if (betSizeFacingThem > fTable.ViewPlayer(pIndex)->GetBetSize()) {
                // They would face an action on this bet, so see whether our overbet gives them an opportunity to profitably fold

                // If (betSize < theirMoney) they would not have been considered for allOpponents, so the existing value of allOpponents is fine.
                const float64 opponentsFacingThem = handsCommitted(fIdx, fTable, betSize, pIndex);

                // Now, take 1.0 - E[min{1.0 / oppN, 1.0 - oppW}] === 1.0 - \sum_oppW Pr{oppW} min{1.0 / waitLength.n(w:oppW), 1.0 - oppW}


                // We only need FoldWaitLengthModel, technically, but FoldGain helps us guess a derivative.
                FoldGainModel FG(fTable.GetChipDenom()/2);

                // TODO(from joseph_huang): Should we assume that the opponents also choose mean when {1 == handsShowdown()} and rank otherwise??
                // I guess technically this is mean but they know what you have, so pessimistically we're talking about them trying to beat only you.
                // The problem is, we usually talk about being pessimistic to prevent overbets when _you_ have a good hand.
                // In that case, e_opp would be counter-productive. Let's consider callcumu or even rank here instead.
                FG.waitLength.meanConv =
                //(opponentsFacingThem > 1.0) ?
                0
                //: &(fCore.callcumu)
                ;
                // ( 1 / (x+1) )  ^ (1/x)
                FG.waitLength.bankroll = fTable.ViewPlayer(pIndex)->GetMoney();
                FG.waitLength.amountSacrificeForced = avgBlinds;
                FG.waitLength.setAmountSacrificeVoluntary( fTable.ViewPlayer(pIndex)->GetBetSize()
    #ifdef SACRIFICE_COMMITTED
                + fTable.ViewPlayer(pIndex)->GetVoluntaryContribution()
    #endif
                - FG.waitLength.amountSacrificeForced // exclude forced portion since it wasn't voluntary.
                );
                FG.waitLength.opponents = opponentsFacingThem;
                FG.waitLength.setW( pow(1.0 / tableStrength, 1.0 / FG.waitLength.opponents) ); // As a baseline, set this so that the overall showdown win percentage required is "1.0 / tableStrength" per person after pow(..., opponents);
                FG.waitLength.prevPot = fTable.GetPrevPotSize();

                const float64 foldGain = FG.f(betSizeFacingThem); // Calling this will invoke query which will populate FG.n
                if (foldGain > 0.0) {
                    // This opponent can profit from folding.

                    const float64 foldN = FG.n * FG.waitLength.rarity();
                    const float64 d_foldN_dbetSize_almost = FG.fd(betSizeFacingThem, foldGain) / foldGain * foldN; // Do something weird here to approximate FG.d_n_dbetSize. Take the rate of change of foldgain and multiply that as a fraction into n. That means, if increasing betSize by 1.0 would increase foldGain by 2%, assume it increases n by 2% too. At least they are in the same direction. TODO(from joseph_huang): A proper derivative here?
                    totalOpposingHandOpportunityCount += 1.0 + foldN; // Add one for the hand they have, and one more for every fold they are (on average) afforded
                    totalOpposingHandOpportunityCount_dbetSize += d_foldN_dbetSize_almost;

                } else {
                    // The user's best chance to win is playing now.
                    // If they continue to fold they will lose more than they could gain.
                    // They contribute only the hand they have.
                    totalOpposingHandOpportunityCount += 1.0;
                    // totalOpposingHandOpportunityCount_dbetSize += 0.0;
                }
            } else {
                // You are checking or calling their bet. They contribute only the hand they have.
                // This is not an overbet situation where we need to be pessimistic.
                totalOpposingHandOpportunityCount += 1.0;
                // totalOpposingHandOpportunityCount_dbetSize += 0.0;
            }
        }

        fTable.incrIndex(pIndex);
    }

    // Return no less than tableStrength.
    if (totalOpposingHandOpportunityCount < tableStrengthToBeat) {
        fHandsToBeat = tableStrengthToBeat;
        f_d_HandsToBeat_dbetSize = 0.0;
    } else {
        fHandsToBeat = totalOpposingHandOpportunityCount;
        f_d_HandsToBeat_dbetSize = totalOpposingHandOpportunityCount_dbetSize;
    }
}
