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

#include "arenaSituation.h"


ExpectedCallD::~ExpectedCallD()
{
}

float64 ExpectedCallD::deadpotFraction() const
{
    return (  table->GetPotSize() / table->ViewPlayer(playerID)->GetMoney()  );
}


float64 ExpectedCallD::betFraction(const float64 betSize) const
{
    return (  betSize / table->ViewPlayer(playerID)->GetMoney()  );
}
/*
float64 EstimateCallD::exf(float64 betSize) const
{
    const float64& x = betFraction(betSize);
    return table->GetNumberInHand() * e->pctWillCall(  x/(2*x+potFraction() )  );
}


float64 EstimateCallD::dexf(float64 betSize) const
{
    const float64& f_pot = potFraction();
    const float64& x = betFraction(betSize);
    return table->GetNumberInHand() * e->pctWillCallD(  x/(2*x+f_pot)  ) * f_pot / (2*x+f_pot) /(2*x+f_pot);
}
*/


void ExactCallD::query(float64 betSize)
{


    totalexf = table->GetPotSize() + betSize;
    //float64 lastexf = totalexf;

    totaldexf = 0;
    //float64 lastdexf = totaldexf;



    int8 pIndex = playerID;

    do{
        table->incrIndex(pIndex);
        if( table->CanStillBet(pIndex) )
        {///Predict how much the bet will be
            float64 oppBankRoll = table->ViewPlayer(pIndex)->GetMoney();
            float64 oppBetAlready = table->ViewPlayer(pIndex)->GetBetSize();
            float64 nextexf;
            float64 nextdexf;


            if( betSize >= oppBankRoll )
            {

                float64 oppBetMake = betSize - oppBetAlready;
                //To understand the above, consider that totalexf ALREADY includes

                nextexf = oppBetAlready + oppBetMake * e->pctWillCall(oppBetMake / (oppBetMake + totalexf) );

                nextdexf = oppBetMake * e->pctWillCallD(  oppBetMake/(oppBetMake+totalexf)  )
                                                * (totalexf - oppBetMake * totaldexf)
                                                 /(oppBetMake + totalexf) /(oppBetMake + totalexf);
            }else
            {///Opponent would be all-in to call this bet
                float64 deadpot = table->GetDeadPotSize();
                float64 effroundpot = (totalexf - deadpot) * oppBankRoll / betSize;
                float64 oppBetMake = oppBankRoll - oppBetAlready;
                nextexf = oppBetAlready + oppBetMake * e->pctWillCall(oppBetMake / (oppBetMake + deadpot + effroundpot) );

                nextdexf = 0;
            }
            //lastexf = nextexf;
            totalexf += nextexf;

            //lastdexf = nextdexf;
            totaldexf += nextdexf;
        }
    }while( pIndex != playerID );

    totalexf = betFraction(totalexf);
    totaldexf = betFraction(totaldexf);
}

float64 ExactCallD::exf(float64 betSize)
{
    //if( queryinput == UNINITIALIZED_QUERY )
    if( queryinput != betSize )
    {
        query(betSize);
        queryinput = betSize;
    }
    return totalexf;
}

float64 ExactCallD::dexf(float64 betSize)
{
//    if( query == UNINITIALIZED_QUERY )
    if( queryinput != betSize )
    {
        query(betSize);
        queryinput = betSize;
    }
    return totaldexf;
}







