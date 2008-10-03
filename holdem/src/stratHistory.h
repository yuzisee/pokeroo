
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

class PlayerStrategy;
class PositionalStrategy;

class PerformanceHistory
{
    public:

        int32 nonZeroWinLose;
        int32 numHandsAboveBelow;
        float64 totalMoneyDelta;
        uint16 rank;
        int8 sortMode; //Sorting arrays with varying sortmodes is not allowed.
        int32 score;

        int16 id;

        PerformanceHistory() : nonZeroWinLose(0), numHandsAboveBelow(0), totalMoneyDelta(0), rank(0), sortMode(0), score(0)
        {}

    const PerformanceHistory & operator=(const PerformanceHistory& a)
    {
        id = a.id;
        score = a.score;
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

    static void SortAndOffset( PerformanceHistory * array, const uint16 num );



}
;


class HistoryStrategy : public virtual PlayerStrategy
{
    protected:
        PositionalStrategy ** strats; //Generated at construction (constant during a game load)
        PerformanceHistory * picks; //Must be loaded/saved
        int16 currentStrategy; //May need to be loaded/saved
        uint8 stratcount; //Generated at construction...


        void init(PositionalStrategy** ps, uint8 n);




    public:
		handnum_t handNumber; //Should be loaded/saved

        static PerformanceHistory UnserializeOne( std::istream& loadFile );
        static void SerializeOne( std::ostream& saveFile, const PerformanceHistory & ph );

        HistoryStrategy(PositionalStrategy** ps, uint8 n) : strats(0), picks(0), currentStrategy(-1), stratcount(n), handNumber(0)
        {
            init(ps,n);
        }

        ~HistoryStrategy();

    virtual void SeeOppHand(const int8, const Hand&) = 0;
    virtual void SeeAction(const HoldemAction&) {};
    virtual void FinishHand();

    virtual void SaveState();
    virtual bool LoadState();
    virtual void Serialize( std::ostream& saveFile ) = 0;
    virtual void Unserialize( std::istream& loadFile ) = 0;

}
;

#endif

