
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

#ifndef HOLDEM_HistoryStrategy
#define HOLDEM_HistoryStrategy

#include "portability.h"
#include "stratPosition.h"

#define SORT_NUM_WINLOSE 1
#define SORT_ABOVE_BELOW 2
#define SORT_TOTAL_DELTA 3
#define SORT_RANK 0


class PerformanceHistory
{
    public:

        int32 nonZeroWinLose;
        int32 numHandsAboveBelow;
        float64 totalMoneyDelta;
        uint8 rank;
        int8 sortMode; //Sorting arrays with varying sortmodes is not allowed.

        char id;

        PerformanceHistory() : nonZeroWinLose(0), numHandsAboveBelow(0), totalMoneyDelta(0), rank(0), sortMode(0)
        {}

    const PerformanceHistory & operator=(const PerformanceHistory& a)
    {
        id = a.id;
        nonZeroWinLose = a.nonZeroWinLose;
        numHandsAboveBelow = a.numHandsAboveBelow;
        totalMoneyDelta = a.totalMoneyDelta;
        rank = a.rank;
        return *this;
    }



        PerformanceHistory(const PerformanceHistory& o)
        {
            (*this) = o;
            sortMode = o.sortMode;
        }

	bool operator> (const PerformanceHistory& x) const
	{
        switch( sortMode )
        {
            case SORT_NUM_WINLOSE:
                return nonZeroWinLose > x.nonZeroWinLose;
                break;
            case SORT_ABOVE_BELOW:
                return numHandsAboveBelow > x.numHandsAboveBelow;
                break;
            case SORT_TOTAL_DELTA:
                return totalMoneyDelta > x.totalMoneyDelta;
                break;
            case SORT_RANK:
            default:
                return rank > x.rank;
                break;
        }
	}
	bool operator< (const PerformanceHistory& x) const
	{
        switch( sortMode )
        {
            case SORT_NUM_WINLOSE:
                return nonZeroWinLose < x.nonZeroWinLose;
                break;
            case SORT_ABOVE_BELOW:
                return numHandsAboveBelow < x.numHandsAboveBelow;
                break;
            case SORT_TOTAL_DELTA:
                return totalMoneyDelta < x.totalMoneyDelta;
                break;
            case SORT_RANK:
            default:
                return rank < x.rank;
                break;
        }

	}
	bool operator== (const PerformanceHistory& x) const
	{
        switch( sortMode )
        {
            case SORT_NUM_WINLOSE:
                return nonZeroWinLose == x.nonZeroWinLose;
                break;
            case SORT_ABOVE_BELOW:
                return numHandsAboveBelow == x.numHandsAboveBelow;
                break;
            case SORT_TOTAL_DELTA:
                return totalMoneyDelta == x.totalMoneyDelta;
                break;
            case SORT_RANK:
            default:
                return rank == x.rank;
                break;
        }
	}

    static void SortAndOffset( PerformanceHistory * array, uint8 num );



}
;


class HistoryStrategy : public virtual PlayerStrategy
{
    protected:
        PerformanceHistory * picks;
        uint8 stratcount;
        uint32 handNumber;

        void init(PlayerStrategy* ps, uint8 n);

        void SerializeOne( std::ostream& saveFile, const PerformanceHistory & ph );

        PerformanceHistory UnserializeOne( std::istream& loadFile );

    public:
        HistoryStrategy(PlayerStrategy* ps, uint8 n) : stratcount(n), handNumber(0)
        {
            init(ps,n);
        }
}
;

class TrendStrategy : public virtual HistoryStrategy
{
    protected:
        void initT();
    public:
    TrendStrategy(PlayerStrategy* ps, uint8 n) : HistoryStrategy(ps,n)
    {}

    void Serialize( std::ostream& saveFile );

}
;

class MultiStrategy : public virtual HistoryStrategy
{
    protected:
        void initM();
    public:
    MultiStrategy(PlayerStrategy* ps, uint8 n) : HistoryStrategy(ps,n)
    {}

    void Serialize( std::ostream& saveFile );
}
;

#endif

