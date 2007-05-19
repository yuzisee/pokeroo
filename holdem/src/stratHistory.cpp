
#include "stratHistory.h"
#include <algorithm>

/*
const int8 PerformanceHistory::SORT_NUM_WINLOSE = 1;
const int8 PerformanceHistory::SORT_ABOVE_BELOW = 2;
const int8 PerformanceHistory::SORT_TOTAL_DELTA = 3;
const int8 PerformanceHistory::SORT_RANK = 0;
*/

void PerformanceHistory::SortAndOffset( PerformanceHistory * array, uint8 num )
{
    float64 averageTotalDelta=0;
    int32 averageWinLose=0;
    int32 averageAboveBelow=0;

    uint8 potentialRank;

    int32 lastAboveBelow;
    int32 lastWinLose;
    float64 lastTotalDelta;

    ///Set up sort mode, and calculate averages (totals)
    for(uint8 i=0;i<num;++i)
    {
        averageTotalDelta += array[i].totalMoneyDelta;
        averageWinLose += array[i].nonZeroWinLose;
        averageAboveBelow += array[i].numHandsAboveBelow;

        //array[i].rank = 0;
        array[i].sortMode = SORT_ABOVE_BELOW;
    }

    ///Sort by Above/Below
    std::sort(array,array+num);//Ascending by default
                                //  http://www.ddj.com/dept/cpp/184403792

    ///Note that the BEST score will receive a rank of 1.

    ///Convert totals to averages
    averageTotalDelta /= num;
    averageWinLose /= num;
    averageAboveBelow /= num;


    ///Rank and offset above/below
    potentialRank = 1;
    lastAboveBelow = array[num-1].numHandsAboveBelow; //Force a tie to put the best at rank 1

    for( uint8 i=num;i>0;--i )
    { //Let i be the index of the element of which you are comparing to, for a tie
      //We iterate from the last (best) element to the first(worst)
        int32 & x = array[i-1].numHandsAboveBelow;
        x -= averageAboveBelow;

        if( x < lastAboveBelow )
        {//Worse than previous
            lastAboveBelow = x;
            potentialRank = num-i+1;
        }

        array[i-1].rank = potentialRank;

        array[i-1].sortMode = SORT_NUM_WINLOSE;
    }

    ///Sort by Win/Lose
    std::sort(array,array+num);

///Rank and offset Win/Lose
    potentialRank = 1;
    lastWinLose = array[num-1].nonZeroWinLose; //Force a tie to put the best at rank 1

    for( uint8 i=num;i>0;--i )
    { //Let i be the index of the element of which you are comparing to, for a tie
      //We iterate from the last (best) element to the first(worst)
        int32 & x = array[i-1].nonZeroWinLose;
        x -= averageWinLose;

        if( x < lastWinLose )
        {//Worse than previous
            lastWinLose = x;
            potentialRank = num-i+1;
        }

        //Only keep the worst rank in all categories
        if( potentialRank > array[i-1].rank ){ array[i-1].rank = potentialRank; }

        array[i-1].sortMode = SORT_TOTAL_DELTA;

    }


    ///Sort by TotalDelta
    std::sort(array,array+num);

///Rank and offset TotalDelta
    potentialRank = 1;
    lastTotalDelta = array[num-1].totalMoneyDelta; //Force a tie to put the best at rank 1

    for( uint8 i=num;i>0;--i )
    { //Let i be the index of the element of which you are comparing to, for a tie
      //We iterate from the last (best) element to the first(worst)
        float64 & x = array[i-1].totalMoneyDelta;
        x -= averageTotalDelta;

        if( x < lastTotalDelta )
        {//Worse than previous (not good enough to tie, must be given a unique rank)
            averageTotalDelta = x;
            potentialRank = num-i+1;
        }

        //Only keep the worst rank in all categories
        if( potentialRank > array[i-1].rank ){ array[i-1].rank = potentialRank; }

        array[i-1].sortMode = SORT_RANK;

    }


    ///Sort by WorstOfRank
    std::sort(array,array+num);


}



void HistoryStrategy::SerializeOne( std::ostream& saveFile, const PerformanceHistory & ph )
{
    saveFile << (int16)(ph.id) << endl;
    saveFile << ph.nonZeroWinLose << " Non-Zero W/L" << endl;
}

PerformanceHistory HistoryStrategy::UnserializeOne( std::istream& loadFile )
{
    PerformanceHistory restored;
    int16 tempInt;
    loadFile >> tempInt;
    restored.id = tempInt;
    return restored;
}
