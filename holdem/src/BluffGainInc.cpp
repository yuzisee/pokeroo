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

#include "BluffGainInc.h"
#include <float.h>

#define RAISED_PWIN




template class AutoScalingFunction<GainModel,GainModelNoRisk>;
template class AutoScalingFunction<GainModel,GainModel>;
template class AutoScalingFunction<GainModelNoRisk,GainModelNoRisk>;

template class AutoScalingFunction<  AutoScalingFunction<GainModel,GainModelNoRisk>  ,  AutoScalingFunction<GainModel,GainModelNoRisk>  >;

template class AutoScalingFunction<   StateModel<  AutoScalingFunction<GainModel,GainModelNoRisk>  ,  AutoScalingFunction<GainModel,GainModelNoRisk>  >
                                      , StateModel<  AutoScalingFunction<GainModel,GainModelNoRisk>  ,  AutoScalingFunction<GainModel,GainModelNoRisk>  >
                                    >;




template class StateModel<GainModel,GainModelNoRisk>;
template class StateModel<  AutoScalingFunction<GainModel,GainModelNoRisk>  ,  AutoScalingFunction<GainModel,GainModelNoRisk>  >;
template class StateModel<GainModel,GainModel>;
template class StateModel<GainModelNoRisk,GainModelNoRisk>;



template<class LL, class RR>
void AutoScalingFunction<LL,RR>::query(float64 sliderx, float64 x)
{

    last_x = x;
    last_sliderx = sliderx;

    if( bLeft )
    {

        yl = left.f(x);
        fd_yl = left.fd(x,yl);

        y = yl; dy = fd_yl;

        #ifdef DEBUG_TRACE_SEARCH
            if(bTraceEnable) std::cout << "\t\t\tbLeft" << std::flush;
        #endif

    }else
    {


        const float64 autoSlope = saturate_upto / (saturate_max - saturate_min) ;
        const float64 slider = (sliderx - saturate_min) * autoSlope ;

        //std::cerr << autoSlope << endl;
        //std::cerr << slider << endl;


        if( slider >= 1 )
        {
            y = right.f(x);
            dy = right.fd(x, yr);

			#ifdef DEBUG_TRACE_SEARCH
				if(bTraceEnable) std::cout << "\t\t\tbMax" << std::flush;
			#endif
        }
        else if( slider <= 0 )
        {
            yl = left.f(x);
            fd_yl = left.fd(x,yl);

            y = yl; dy = fd_yl;

			#ifdef DEBUG_TRACE_SEARCH
				if(bTraceEnable) std::cout << "\t\t\tbMin" << std::flush;
			#endif
        }
        else
        {
            yl = left.f(x);
            yr = right.f(x);

            fd_yl = left.fd(x,yl);
            fd_yr = right.fd(x,yr);


          #ifdef TRANSFORMED_AUTOSCALES
            if( AUTOSCALE_TYPE == LOGARITHMIC_AUTOSCALE )
            {

				const float64 rightWeight = log1p(slider)/log(2.0);
                const float64 leftWeight = 1 - rightWeight;

                y = yl*leftWeight+yr*rightWeight;
                //y = yl*log(2-slider)/log(2)+yr*log(1+slider)/log(2);

                const float64 d_rightWeight_d_slider = 1.0/(slider)/log(2.0);

                dy = fd_yl*leftWeight - yl*autoSlope*d_rightWeight_d_slider   +   fd_yr*rightWeight + yr*autoSlope*d_rightWeight_d_slider;
		    }else
            #ifdef DEBUGASSERT
            if( AUTOSCALE_TYPE == ALGEBRAIC_AUTOSCALE )
            #endif // DEBUGASSERT
            {
		  #endif
                y = yl*(1-slider)+yr*slider;
                dy = fd_yl*(1-slider) - yl*autoSlope   +   fd_yr*slider + yr*autoSlope;

            #ifdef DEBUG_TRACE_SEARCH
				if(bTraceEnable) std::cout << "\t\t\t y(" << x << ") = " << yl << " * " << (1-slider) << " + " <<  yr << " * " << slider << std::endl;
				if(bTraceEnable) std::cout << "\t\t\t dy = " << fd_yl << " * " << (1-slider) << " - " <<  yl << " * " << autoSlope << " + " <<  fd_yr << " * " << slider << " + " <<  yr << " * " << autoSlope << std::endl;
			#endif // DEBUG_TRACE_SEARCH
          #ifdef TRANSFORMED_AUTOSCALES
			}
            #ifdef DEBUGASSERT
            else{
                std::cerr << "AutoScale TYPE MUST BE SPECIFIED" << endl;
				exit(1);
            }
            #endif // DEBUGASSERT
          #endif // TRANSFORMED_AUTOSCALES
        }


    }
}

template<class LL, class RR>
float64 AutoScalingFunction<LL,RR>::f(const float64 x)
{
    if( last_x != x || last_sliderx != x)
    {
        query(x,x);
    }
    return y;
}

template<class LL, class RR>
float64 AutoScalingFunction<LL,RR>::fd(const float64 x, const float64 y_dummy)
{
    if( last_x != x || last_sliderx != x)
    {
        query(x,x);
    }
    return dy;
}

template<class LL, class RR>
float64 AutoScalingFunction<LL,RR>::f_raised(float64 sliderx, const float64 x)
{
    if( last_x != x || last_sliderx != sliderx)
    {
        query(sliderx,x);
    }
    return y;
}

template<class LL, class RR>
float64 AutoScalingFunction<LL,RR>::fd_raised(float64 sliderx, const float64 x, const float64 y_dummy)
{
    if( last_x != x || last_sliderx != sliderx)
    {
        query(sliderx,x);
    }
    return dy;
}





template <class LL, class RR>
StateModel<LL,RR>::~StateModel()
{
    if( bSingle && fp != 0 )
    {
        delete fp;
    }
}


template <class LL, class RR>
float64 StateModel<LL,RR>::g_raised(float64 raisefrom, const float64 betSize)
{
    return fp->f_raised(raisefrom, betSize) + ea.FoldGain();
}

template <class LL, class RR>
float64 StateModel<LL,RR>::gd_raised(float64 raisefrom, const float64 betSize, const float64 y)
{
    return fp->fd_raised(raisefrom, betSize, y - ea.FoldGain());
}

template <class LL, class RR>
float64 StateModel<LL,RR>::f(const float64 betSize)
{
    if( last_x != betSize )
    {

        query(betSize);
    }
    return y;
}

template <class LL, class RR>
float64 StateModel<LL,RR>::fd(const float64 betSize, const float64 y)
{
    if( last_x != betSize )
    {
        query(betSize);
    }
    return dy;
}

template <class LL, class RR>
void StateModel<LL,RR>::query( const float64 betSize )
{


    last_x = betSize;
    const float64 invisiblePercent = quantum / ea.tableinfo->allChips();

///Establish [PushGain] values

	float64 potFoldWin = ea.tableinfo->PushGain();
	const float64 potFoldWinD = 0;
    float64 oppFoldChance = ea.pWin(betSize);
    float64 oppFoldChanceD = ea.pWinD(betSize);

#ifdef DEBUGASSERT
	if( potFoldWin < 0 || oppFoldChance < invisiblePercent ){
		potFoldWin =  1;
		oppFoldChance = 0;
		oppFoldChanceD = 0;
	}
#endif

///Establish [Raised] values

    //Count needed array size
    int32 arraySize = 0;
    while( ea.RaiseAmount(betSize,arraySize) < ea.tableinfo->maxBetAtTable() )
    {
        ++arraySize;
    }
    //This array loops until noRaiseArraySize is the index of the element with RaiseAmount(noRaiseArraySize) == maxBet()
    if(betSize < ea.tableinfo->maxBetAtTable()) ++arraySize; //Now it's the size of the array (unless you're pushing all-in already)

    const bool bCallerWillPush = (arraySize == 1); //If arraySize is 1, then minRaise goes over maxbet. Anybody who can call will just reraise over the top.

    //Create arrays
    float64 * raiseAmount_A = new float64[arraySize];

    float64 * oppRaisedChance_A = new float64[arraySize];
    float64 * oppRaisedChanceD_A = new float64[arraySize];


    float64 * potRaisedWin_A = new float64[arraySize];
    float64 * potRaisedWinD_A = new float64[arraySize];


    float64 lastuptoRaisedChance = 0;
    float64 lastuptoRaisedChanceD = 0;
    float64 newRaisedChance = 0;
    float64 newRaisedChanceD = 0;

	firstFoldToRaise = arraySize;

	for( int32 i=0;i<arraySize; ++i)
    {
#ifdef RAISED_PWIN
        raiseAmount_A[i] = ea.RaiseAmount(betSize,i);


        potRaisedWin_A[i] = g_raised(betSize,raiseAmount_A[i]);
        potRaisedWinD_A[i] = gd_raised(betSize,raiseAmount_A[i],potRaisedWin_A[i]);

        const float64 oppRaisedFoldGain = ea.FoldGain(betSize - ea.tableinfo->alreadyBet(),raiseAmount_A[i]); //You would fold the additional (betSize - ea->alreadyBet() )

        if( potRaisedWin_A[i] < oppRaisedFoldGain )
        {
			if( firstFoldToRaise == arraySize ) firstFoldToRaise = i;
            potRaisedWin_A[i] = oppRaisedFoldGain;
            potRaisedWinD_A[i] = 0;
        }

    }

    for( int32 i=arraySize-1;i>=0; --i)
    {

        if( bCallerWillPush )
        {
            //ASSERT: i == 0 && arraySize == 1 && lastUptoRaisedChance == 0 && oppFoldChance and oppFoldChanceD have already been determined
            newRaisedChance = 1 - oppFoldChance;
            newRaisedChanceD = - oppFoldChanceD;
        }else{
            //Standard calculation
            newRaisedChance = ea.pRaise(betSize,i,firstFoldToRaise);
            newRaisedChanceD = ea.pRaiseD(betSize,i,firstFoldToRaise);
        }

		if( newRaisedChance - lastuptoRaisedChance > invisiblePercent )
		{
			oppRaisedChance_A[i] = newRaisedChance - lastuptoRaisedChance;
			oppRaisedChanceD_A[i] = newRaisedChanceD - lastuptoRaisedChanceD;
			lastuptoRaisedChance = newRaisedChance;
			lastuptoRaisedChanceD = newRaisedChanceD;
		}
		//if( oppRaisedChance_A[i] < invisiblePercent )
		else
#endif
        {
	    raiseAmount_A[i] = 0;
        oppRaisedChance_A[i] = 0;
        oppRaisedChanceD_A[i] = 0;
        potRaisedWin_A[i] = 1;
        potRaisedWinD_A[i] = 0;
        }

            #ifdef DEBUG_TRACE_SEARCH
                if(bTraceEnable)
                {
                    std::cout << "\t\t(oppRaiseChance[" << i << "] , cur, highest) = " << oppRaisedChance_A[i]  << " , "  << newRaisedChance << " , " << lastuptoRaisedChance << std::endl;
                }
            #endif

    }



///Establish [Play] values
	float64 playChance = 1 - oppFoldChance - lastuptoRaisedChance;
	float64 playChanceD = - oppFoldChanceD - lastuptoRaisedChanceD;
    /*
	float64 playChance = 1 - oppFoldChance;
	float64 playChanceD = - oppFoldChanceD;
	for( int32 i=0;i<arraySize;++i )
	{
        playChance -= oppRaisedChance_A[i];
        playChanceD -= oppRaisedChanceD_A[i];
	}*/


    float64 potNormalWin = g_raised(betSize,betSize);
    float64 potNormalWinD = gd_raised(betSize,betSize,potNormalWin);

    if( playChance <= invisiblePercent ) //roundoff, but {playChance == 0} is push-fold for the opponent
    {
        //Correct other odds
        const float64 totalChance = 1 - playChance;
        for( int32 i=arraySize-1;i>=0; --i)
        {
            potRaisedWin_A[i] /= totalChance;
            potRaisedWinD_A[i] /= totalChance;
        }
        oppFoldChance /= totalChance;
        oppFoldChanceD /= totalChance;

        //Remove call odds
        playChance = 0;
        playChanceD = 0;
        potNormalWin = 1;
        potNormalWinD = 0;
    }





///Calculate factors
#ifdef VERBOSE_STATEMODEL_INTERFACE
  #define STATEMODEL_ACCESS
#else
  #define STATEMODEL_ACCESS const float64
#endif

	STATEMODEL_ACCESS gainWithFold = (potFoldWin < DBL_EPSILON) ? 0 : pow(potFoldWin , oppFoldChance);
	const float64 gainWithFoldlnD = oppFoldChance*potFoldWinD/potFoldWin + oppFoldChanceD*log(potFoldWin);
	STATEMODEL_ACCESS gainNormal =  (potNormalWin < DBL_EPSILON) ? 0 : pow( potNormalWin,playChance );


	float64 gainNormallnD = playChance*potNormalWinD/potNormalWin + playChanceD*log(potNormalWin);
    STATEMODEL_ACCESS gainRaised = 1;
    float64 gainRaisedlnD = 0;

#undef STATEMODEL_ACCESS

    for( int32 i=0;i<arraySize;++i )
    {
        gainRaised *= (potRaisedWin_A[i] < DBL_EPSILON) ? 0 : pow( potRaisedWin_A[i],oppRaisedChance_A[i]);

		if( oppRaisedChance_A[i] >= invisiblePercent )
		{
			#ifdef DEBUG_TRACE_SEARCH
				if(bTraceEnable)
				{
				    std::cout << "\t\t\t(potRaisedWinD_A[" << i << "] , oppRaisedChanceD_A[" << i << "] , log...) = " << potRaisedWinD_A[i] << " , " << oppRaisedChanceD_A[i] << " , " <<  log( g_raised(betSize,raiseAmount_A[i]-quantum/2) ) << std::endl;
				}
			#endif

			if( raiseAmount_A[i] >= ea.tableinfo->maxBet()-quantum/2 )
			{
				gainRaisedlnD += oppRaisedChance_A[i]*potRaisedWinD_A[i]/ g_raised(betSize,raiseAmount_A[i]-quantum/2) + oppRaisedChanceD_A[i]*log( g_raised(betSize,raiseAmount_A[i]-quantum/2) );
			}else
			{
				gainRaisedlnD += oppRaisedChance_A[i]*potRaisedWinD_A[i]/potRaisedWin_A[i] + oppRaisedChanceD_A[i]*log(potRaisedWin_A[i]);
			}
		}
    }


	if( betSize >= ea.tableinfo->maxBet() )
	{
		gainNormallnD = playChance*potNormalWinD/g_raised(betSize,betSize-quantum/2) + playChanceD*log(g_raised(betSize,betSize-quantum/2));
	}


		#ifdef DEBUG_TRACE_SEARCH
			if(bTraceEnable)
			{
			    std::cout << "\t\t (gainWithFoldlnD+gainNormallnD+gainRaisedlnD) = " << gainWithFoldlnD << " + " << gainNormallnD << " + " <<  gainRaisedlnD << std::endl;
			    std::cout << "\t\t (gainWithFold   +gainNormal   +gainRaised   ) = " << gainWithFold << " + " << gainNormal << " + " <<  gainRaised << std::endl;
			    std::cout << "\t\t (potNormalWin , playChance) = " << potNormalWin << " , " << playChance << std::endl;
			}
		#endif

///Store results
    y = gainWithFold*gainNormal*gainRaised;

    dy = (gainWithFoldlnD+gainNormallnD+gainRaisedlnD)*y;

    y -= ea.FoldGain();


    delete [] raiseAmount_A;

    delete [] oppRaisedChance_A;
    delete [] oppRaisedChanceD_A;


    delete [] potRaisedWin_A;
    delete [] potRaisedWinD_A;


}
