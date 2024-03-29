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

#undef TREND_STRAT_ENABLE

#ifndef HOLDEM_CombinedStrategy
#define HOLDEM_CombinedStrategy

#include "stratHistory.h"


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
        bool bHandShown;
        float64 prevMoney;
        bool initM(); //Load file if available
    public:

    int8 bGamble;

    MultiStrategy(std::string stateFilename, PositionalStrategy** ps, uint8 n)
    : HistoryStrategy(stateFilename, ps,n)
    , bHandShown(false), prevMoney(-1), bGamble(0)
    {
    }

    virtual void SeeOppHand(const int8, const Hand&);
    virtual void SeeCommunity(const Hand&, const int8);
    virtual float64 MakeBet();

    void Serialize( std::ostream& saveFile ) override final;
    void Unserialize( std::istream& loadFile ) override final;
}
;

#endif

