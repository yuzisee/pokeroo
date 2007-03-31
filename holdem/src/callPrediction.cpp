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

#include "callPrediction.h"
#include <math.h>




//#define DEBUGWATCHPARMS
#ifdef DEBUGWATCHPARAMS
#include <iostream>
#endif

const float64 ExactCallD::UNITIALIZED_QUERY = -1;


void ExactCallD::SetImpliedFactor(const float64 bonus)
{
    impliedFactor = bonus;
}

float64 ExactCallD::facedOdds(float64 pot, float64 bet, float64 wGuess)
{
    //oppBetMake / (oppBetMake + totalexf)
    const int8 N = handsDealt();
    const float64 fNRank = e->pctWillCall(1 - wGuess);
    const float64 avgBlind = (table->GetBigBlind() + table->GetBigBlind()) * ( N - 2 )/ N / N;
    return (
            (bet -  avgBlind / fNRank ) / (bet + pot)
           );
}

float64 ExactCallD::facedOddsND(float64 pot, float64 bet, float64 dpot, float64 w, float64 fw, float64 dfw)
{
    const int8 N = handsDealt();
    const float64 fNRank = 1 - e->pctWillCall(1 - w);
    const float64 avgBlind = (table->GetBigBlind() + table->GetBigBlind()) * ( N - 2 )/ N / N;    

    //    (pot - bet * dpot)
    //    /(bet + pot) /(bet + pot);
    
    //to
    /*
    =
    {1 - ( dpot + 1 ) f(w) }
    over

            (
                ( pot + betSize ) f'(w)
                +
                    {avgBlind * pctWillCall' ( 1 - w ) }
                    over
                    ( 1 - pctWillCall ( 1 - w ) )^2
            )
    */

    
    //Not
    //           ( pot - dpot * (bet - avgBlind / (1 - fRank) ) ) / (bet + pot) / (bet + pot)
    
    return (
                (1 - fw*(dpot + 1))
                /
                (   (pot+bet)*dfw
                    +
                    ( avgBlind * e->pctWillCallD(1-w) ) / fNRank / fNRank
                )
           );
}


void ExactCallD::query(const float64 betSize)
{
    const float64 significance = 1/static_cast<float64>( handsDealt()-1 );
    const float64 myexf = betSize;
    const float64 mydexf = 1;

    float64 overexf = 0;
    float64 overdexf = 0;

    totalexf = table->GetPotSize() - table->ViewPlayer(playerID)->GetBetSize()  +  myexf;

    //float64 lastexf = totalexf;


    totaldexf = mydexf;
    //float64 lastdexf = totaldexf;

    int8 pIndex = playerID;
    table->incrIndex(pIndex);
    while( pIndex != playerID )
    {
        float64 nextexf = 0;
        float64 nextdexf = 0;
        const float64 oppBetAlready = table->ViewPlayer(pIndex)->GetBetSize();

        if( table->CanStillBet(pIndex) )
        {///Predict how much the bet will be
            const float64 oppBankRoll = table->ViewPlayer(pIndex)->GetMoney();

            if( betSize < oppBankRoll )
            {

                const float64 oppBetMake = betSize - oppBetAlready;
                //To understand the above, consider that totalexf includes already made bets

                if( oppBetMake <= 0 )
                {
                    nextexf = 0;
                    nextdexf = 1;
                }else
                {
#ifdef DEBUGWATCHPARMS
                    const float64 vodd = pow(  oppBetMake / (oppBetMake + totalexf), significance);
                    const float64 willCall = e->pctWillCall( pow(  oppBetMake / (oppBetMake + totalexf)  , significance  ) );
                    const float64 willCallD = e->pctWillCallD(   pow(  oppBetMake / (oppBetMake + totalexf)  , significance  )  );

/*
                    std::cout << willCall << " ... " << willCallD << std::endl;
                    std::cout << "significance " << significance << std::endl;
                    std::cout << "(oppBetMake + totalexf) " << (oppBetMake + totalexf) << std::endl;
                    std::cout << "(oppBetMake ) " << (oppBetMake ) << std::endl;
                    std::cout << "(betSize) " << (oppBetMake) << std::endl;
                    std::cout << "(oppBetAlready) " << (oppBetAlready) << std::endl;
*/
#endif

                    const float64 wn = facedOdds(totalexf,oppBetMake); //f(w) = w^n
                    const float64 w = pow( wn, significance );         //f-1(w) = w
                    nextexf = e->pctWillCall( w );
                    
                    
                    nextdexf = nextexf + oppBetMake * e->pctWillCallD(  w  )  
                            * facedOddsND( totalexf,oppBetMake,totaldexf,w, wn, wn/w / significance );
                    //              *  pow(  oppBetMake / (oppBetMake + totalexf)  , significance - 1 ) * significance
//                                * (totalexf - oppBetMake * totaldexf)
//                                 /(oppBetMake + totalexf) /(oppBetMake + totalexf);
                    nextexf *= oppBetMake;

                }
            }else
            {///Opponent would be all-in to call this bet
                const float64 oldpot = table->GetPrevPotSize();
                const float64 effroundpot = (totalexf - oldpot) * oppBankRoll / betSize;
                const float64 oppBetMake = oppBankRoll - oppBetAlready;


                nextexf = oppBetMake * e->pctWillCall( pow( facedOdds(oldpot + effroundpot,oppBetMake),significance) );

                nextdexf = 0;

            }


            //lastexf = nextexf;
            totalexf += nextexf;

            //lastdexf = nextdexf;
            totaldexf += nextdexf;

        }

        const float64 oppInPot = oppBetAlready + nextexf;
        if( oppInPot > betSize )
        {
            overexf += oppInPot - betSize;
            overdexf += nextdexf;
        }

        table->incrIndex(pIndex);
    }

    totalexf = totalexf - myexf - overexf;
    totaldexf = totaldexf - mydexf - overdexf;

    if( totalexf < 0 ) totalexf = 0; //Due to rounding error in overexf?
    if( totaldexf < 0 ) totaldexf = 0; //Due to rounding error in overexf?

}

void ExactCallBluffD::query(const float64 betSize)
{
    ExactCallD::query(betSize);

    float64 countMayFold = table->GetNumberInHand() - 1 ;
//    const float64 countToBeat = table->GetNumberAtTable() - 1 ;
//    float64 countToBeat = table->GetNumberAtTable() - table->GetNumberInHand() + 1 ;
//	float64 countMayFold = 1 ;
    const float64 myexf = betSize;
    const float64 mydexf = 1;


    float64 nextFold;
    float64 nextFoldPartial;
    allFoldChance = 1;
    allFoldChanceD = 0;
    const float64 origPot = table->GetPotSize() - table->ViewPlayer(playerID)->GetBetSize()  +  myexf;
    const float64 origPotD = mydexf;


    if( betSize < minRaiseTo() - chipDenom()/4 || callBet() >= table->GetMaxShowdown() )
    {
        allFoldChance = 0;
        allFoldChanceD = 0;
        nextFold = 0;
        nextFoldPartial = 0;
    }

    int8 pIndex = playerID;
    table->incrIndex(pIndex);
    while( pIndex != playerID && allFoldChance > 0)
    {

        const float64 oppBetAlready = table->ViewPlayer(pIndex)->GetBetSize();

        const float64 significanceLinear =/* 1/countToBeat ;
                const float64 significance =*/ 1/countMayFold;

        if( table->CanStillBet(pIndex) )
        {///Predict how much the bet will be
            const float64 oppBankRoll = table->ViewPlayer(pIndex)->GetMoney();

            if( betSize < oppBankRoll )
            {

                const float64 oppBetMake = betSize - oppBetAlready;
                //To understand the above, consider that totalexf includes already made bets



/*

                const float64 nextFoldB = ea->pctWillCall( pow(  oppBetMake / (oppBetMake + origPot)  , significance  ) );
                const float64 nextFoldPartialB = ea->pctWillCallD(   pow(  oppBetMake / (oppBetMake + origPot)  , significance  )  )
                *  pow(  oppBetMake / (oppBetMake + origPot)  , significance - 1 ) * significance
                * (origPot - oppBetMake * origPotD)
                                                 /(oppBetMake + origPot) /(oppBetMake + origPot);
*/
                
                const float64 wn = facedOdds(origPot,oppBetMake);
                const float64 w = pow( wn, significanceLinear );
                
                
//                const float64 nextFoldF = 1 -
                nextFold = w;
//                        pow(  oppBetMake / (oppBetMake + origPot)  , significanceLinear  );
//				const float64 nextFoldPartialF = -
                nextFoldPartial =
                        facedOddsND( origPot,oppBetMake,origPotD,w, wn, wn/w / significanceLinear );
//                        pow(  oppBetMake / (oppBetMake + origPot)  , significanceLinear - 1 ) * significanceLinear
//                        * (origPot - oppBetMake * origPotD)
//                    /(oppBetMake + origPot) /(oppBetMake + origPot);


/*
#ifndef GEOM_COMBO_FOLDPCT

                nextFold = 1 - (nextFoldB + nextFoldF)/2;
                nextFoldPartial = -(nextFoldPartialB + nextFoldPartialF)/2;

#else

                if( nextFoldB <= 0 || nextFoldF <= 0 )
                {//If they are not going to call no matter what
                nextFold = 1; //So they will fold no matter what
                nextFoldPartial = 0;
						//Betting more won't improve this player's chance of folding, which is already 100%
            }else
                {
                nextFold = sqrt(nextFoldB*nextFoldF);
                nextFoldPartial = -nextFold*(nextFoldPartialB/nextFoldB + nextFoldPartialF/nextFoldF)/2;
                nextFold = 1 - nextFold;
            }

#endif
*/

            }else
            {///Opponent would be all-in to call this bet
                const float64 oldpot = table->GetPrevPotSize();
                const float64 effroundpot = (origPot - oldpot) * oppBankRoll / betSize;
                const float64 oppBetMake = oppBankRoll - oppBetAlready;

                if( oppBankRoll < minRaiseTo() - chipDenom()/4 )
                {
                    allFoldChance = 0;
                    allFoldChanceD = 0;
                    nextFold = 0;
                }else
                {
/*
                    const float64 nextFoldB = e->pctWillCall( pow(oppBetMake / (oppBetMake + oldpot + effroundpot),significance) );
*/
//					const float64 nextFoldF = 1 -
                    
                    nextFold = pow( facedOdds(oldpot + effroundpot,oppBetMake),significanceLinear) ;
//                            pow(oppBetMake / (oppBetMake + oldpot + effroundpot),significanceLinear );
/*
#ifndef GEOM_COMBO_FOLDPCT
                    nextFold = 1 - (nextFoldB + nextFoldF)/2;
#else
                    nextFold = 1-sqrt(nextFoldB*nextFoldF);
#endif
*/
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
            if( oppBetAlready > 0 )
            {//Must have gone all-in just now, or at least this round
                allFoldChance = 0;
                allFoldChanceD = 0;
            }
        }


        table->incrIndex(pIndex);
    }

    allFoldChanceD *= allFoldChance;

}

float64 ExactCallBluffD::PushGain()
{
    const float64 baseFraction = betFraction(table->GetPotSize() - alreadyBet());

#ifdef BLIND_ADJUSTED_FOLD
    const float64 rawWinFreq = (1.0 / table->GetNumberAtTable()) ;
    const float64 blindsPow = rawWinFreq*(1.0 - 1.0 / table->GetNumberAtTable());

    const float64 bigBlindFraction = betFraction( table->GetBigBlind() );
    const float64 smallBlindFraction = betFraction( table->GetSmallBlind() );
#ifdef CONSISTENT_AGG
    const float64 handFreq = 1/(1-handRarity);
    if( handRarity >= 1 ) //all hands are better than this one
    {
           //1326 is the number of hands possible
        return (1 + baseFraction + (bigBlindFraction+smallBlindFraction)*blindsPow*1326);
    }
    const float64 totalFG = (1 + baseFraction + (bigBlindFraction+smallBlindFraction)*blindsPow*handFreq);

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
    if( queryinput != betSize )
    {
        query(betSize);
        queryinput = betSize;
    }
    ///Try pow(,impliedFactor) maybe
    return allFoldChance;//*impliedFactor;
}

float64 ExactCallBluffD::pWinD(const float64 betSize)
{
    if( queryinput != betSize )
    {
        query(betSize);
        queryinput = betSize;
    }
    return allFoldChanceD;//*impliedFactor;
}



float64 ExactCallD::exf(const float64 betSize)
{
    //if( queryinput == UNINITIALIZED_QUERY )
    if( queryinput != betSize )
    {
        query(betSize);
        queryinput = betSize;
    }
    return totalexf*impliedFactor;
}

float64 ExactCallD::dexf(const float64 betSize)
{
//    if( query == UNINITIALIZED_QUERY )
    if( queryinput != betSize )
    {
        query(betSize);
        queryinput = betSize;
    }
    return totaldexf*impliedFactor;
}





