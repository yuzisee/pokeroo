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

#include "callSituation.h"




class ExactCallD : public virtual ExpectedCallD
{
    private:
        float64 totalexf;
        float64 totaldexf;
    protected:
        static const float64 UNITIALIZED_QUERY;
        float64 queryinput;


        float64 impliedFactor;

        
        float64 facedOdds(float64 pot, float64 bet, float64 wGuess = 0.75);
        float64 facedOddsND(float64 pot, float64 bet, float64 dpot, float64 w, float64 fw, float64 dfw);
        void query(const float64 betSize);
    public:
        ExactCallD(const int8 id, const HoldemArena* base
#ifdef ANTI_PRESSURE_FOLDGAIN
                , const float64 rankPCT
#endif
                        , const CallCumulationD* data, const float64 commit = 0)
    : ExpectedCallD(id,base
#ifdef ANTI_PRESSURE_FOLDGAIN
            ,rankPCT
#endif
                    ,data,commit), impliedFactor(1)
            {
                queryinput = UNITIALIZED_QUERY;
            }

            virtual float64 exf(const float64 betSize);
            virtual float64 dexf(const float64 betSize);

            virtual void SetImpliedFactor(const float64 bonus);
}
;

class ExactCallBluffD : public virtual ExactCallD
{
    protected:
        const CallCumulationD* ea;
        float64 allFoldChance;
        float64 allFoldChanceD;

        void query(const float64 betSize);
    public:
        ExactCallBluffD(const int8 id, const HoldemArena* base
#ifdef ANTI_PRESSURE_FOLDGAIN
                , const float64 rankPCT
#endif
                        , const CallCumulationD* data, const CallCumulationD* foldData, const float64 commit = 0)
    : ExpectedCallD(id,base
#ifdef ANTI_PRESSURE_FOLDGAIN
            ,rankPCT
#endif
                    ,data,commit),ExactCallD(id,base
#ifdef ANTI_PRESSURE_FOLDGAIN
                            ,rankPCT
#endif
                                    ,data,commit), ea(foldData)
                            {
                                queryinput = UNITIALIZED_QUERY;
                            }


                            float64 PushGain();

                            virtual float64 pWin(const float64 betSize);
                            virtual float64 pWinD(const float64 betSize);
}
;

