


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

#ifndef HOLDEM_CombinedStrategy
#define HOLDEM_CombinedStrategy

#include "stratHistory.h"

#undef TREND_STRAT_ENABLE

#ifdef TREND_STRAT_ENABLE
class TrendStrategy : public virtual HistoryStrategy
{
    protected:
        void initT(); //Load file if available

        int8 PassCount( PerformanceHistory &x ){}

        void RestartTrend(){}
        void RestartCycle(float64 seed){}

        uint32 RandomSeed();
    public:
    TrendStrategy(PositionalStrategy** ps, uint8 n) : HistoryStrategy(ps,n)
    {}

    virtual void Serialize( std::ostream& saveFile );
    virtual void Unserialize( std::istream& loadFile );

    virtual void SeeCommunity(const Hand&, const int8);



}
;
#endif

class MultiStrategy : public virtual HistoryStrategy
{
    protected:
        float64 prevMoney;
        void initM(); //Load file if available
    public:


    int8 bGamble;

    MultiStrategy(PositionalStrategy** ps, uint8 n) : HistoryStrategy(ps,n), prevMoney(-1), bGamble(0)
    {
    }

    virtual void SeeCommunity(const Hand&, const int8);
    virtual float64 MakeBet();

    virtual void Serialize( std::ostream& saveFile );
    virtual void Unserialize( std::istream& loadFile );
}
;

#endif

