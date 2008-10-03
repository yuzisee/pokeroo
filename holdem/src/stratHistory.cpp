

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


#define LOAD_PAST_HANDS "saves/"
#define LOAD_PAST_HANDS_LEN 8

#include "stratHistory.h"
#include <algorithm>
#include <string.h>







/*
const int8 PerformanceHistory::SORT_NUM_WINLOSE = 1;
const int8 PerformanceHistory::SORT_ABOVE_BELOW = 2;
const int8 PerformanceHistory::SORT_TOTAL_DELTA = 3;
const int8 PerformanceHistory::SORT_RANK = 0;
*/

void PerformanceHistory::SortAndOffset( PerformanceHistory * parray, const uint16 num )
{
    float64 averageTotalDelta=0;
    int32 averageWinLose=0;
    int32 averageAboveBelow=0;

    uint16 potentialRank;

    int32 lastAboveBelow;
    int32 lastWinLose;
    float64 lastTotalDelta;

    ///Set up sort mode, and calculate averages (totals)
    for(uint8 i=0;i<num;++i)
    {
        averageTotalDelta += parray[i].totalMoneyDelta;
        averageWinLose += parray[i].nonZeroWinLose;
        averageAboveBelow += parray[i].numHandsAboveBelow;

        //array[i].rank = 0;
        parray[i].sortMode = SORT_ABOVE_BELOW;
    }

    ///Sort by Above/Below
    std::sort(parray,parray+num);//Ascending by default
                                //  http://www.ddj.com/dept/cpp/184403792

    ///Note that the BEST score will receive a rank of 1.

    ///Convert totals to averages
    averageTotalDelta /= num;
    averageWinLose /= num;
    averageAboveBelow /= num;


    ///Rank and offset above/below
    potentialRank = 1;
    lastAboveBelow = parray[num-1].numHandsAboveBelow; //Force a tie to put the best at rank 1

    for( uint16 i=num;i>0;--i )
    { //Let i be the index of the element of which you are comparing to, for a tie
      //We iterate from the last (best) element to the first(worst)
        int32 & x = parray[i-1].numHandsAboveBelow;

        if( x < lastAboveBelow )
        {//Worse than previous
            lastAboveBelow = x;
            potentialRank = num-i+1;
        }

        x -= averageAboveBelow;

        parray[i-1].rank = potentialRank*num;

        parray[i-1].sortMode = SORT_NUM_WINLOSE;
    }

    ///Sort by Win/Lose
    std::sort(parray,parray+num);

///Rank and offset Win/Lose
    potentialRank = 1;
    lastWinLose = parray[num-1].nonZeroWinLose; //Force a tie to put the best at rank 1

    for( uint16 i=num;i>0;--i )
    { //Let i be the index of the element of which you are comparing to, for a tie
      //We iterate from the last (best) element to the first(worst)
        int32 & x = parray[i-1].nonZeroWinLose;

        if( x < lastWinLose )
        {//Worse than previous
            lastWinLose = x;
            potentialRank = num-i+1;
        }

        x -= averageWinLose;

        //Only keep the worst rank in all categories
        parray[i-1].rank /= num;
        if( potentialRank > parray[i-1].rank )
        {
            parray[i-1].rank += potentialRank*num - 1;
        }else
        {
            parray[i-1].rank *= num;
            parray[i-1].rank += potentialRank - 1;
        }
        //At this point, potentialRank * num is the rank, (potentialRank % num)+1 is tiebreaker

        parray[i-1].sortMode = SORT_TOTAL_DELTA;

    }


    ///Sort by TotalDelta
    std::sort(parray,parray+num);

///Rank and offset TotalDelta
    potentialRank = 1;
    lastTotalDelta = parray[num-1].totalMoneyDelta; //Force a tie to put the best at rank 1

    for( uint16 i=num;i>0;--i )
    { //Let i be the index of the element of which you are comparing to, for a tie
      //We iterate from the last (best) element to the first(worst)
        float64 & x = parray[i-1].totalMoneyDelta;


        if( x < lastTotalDelta )
        {//Worse than previous (not good enough to tie, must be given a unique rank)
            lastTotalDelta = x;
            potentialRank = num-i+1;
        }

        x -= averageTotalDelta;

        //Only keep the worst rank in all categories
        const uint16 tieBreaker = (parray[i-1].rank % (num)) + 1;
        const uint16 interimRank = parray[i-1].rank / (num);
        if( potentialRank > interimRank )
        {
            parray[i-1].rank = potentialRank*num*2;
            parray[i-1].rank += (interimRank + tieBreaker) - 2;
        }else
        {//interimRank is still the worst rank
            parray[i-1].rank = (interimRank*num*2) + (potentialRank + tieBreaker) - 2;
        }


        parray[i-1].sortMode = SORT_RANK;

    }

//INVARIANT: Each rank is now num*worstRank + avg(otherRanks)
    ///Sort by WorstOfRank
    std::sort(parray,parray+num);


}

HistoryStrategy::~HistoryStrategy()
{
    if( picks != 0 ){ delete [] picks; }
    if( strats != 0 ){ delete [] strats; }
}

void HistoryStrategy::init(PositionalStrategy** ps, uint8 n)
{
    strats = new PositionalStrategy*[n];
    picks = new PerformanceHistory[n];
    for(uint8 i=0;i<n;++i)
    {
        strats[i] = ps[i];
        strats[i]->Link(this);
        picks[i].id = i;
    }

}

void HistoryStrategy::FinishHand()
{
    strats[picks[currentStrategy].id]->ReleaseLogFile();
}

void HistoryStrategy::SerializeOne( std::ostream& saveFile, const PerformanceHistory & ph )
{
    saveFile << ph.id << endl;
    saveFile << ph.score << " Misc." << endl;
    saveFile << (int16)(ph.rank) << " ==Rank" << endl;
    saveFile << ph.nonZeroWinLose << " Non-Zero W/L" << endl;
    saveFile << ph.numHandsAboveBelow << " Pushed - Revealed" << endl;
    HoldemUtil::WriteFloat64( saveFile, ph.totalMoneyDelta );
    saveFile << " Total Money Delta (" << ph.totalMoneyDelta << ")" << endl;
}

PerformanceHistory HistoryStrategy::UnserializeOne( std::istream& loadFile )
{
    char HUMAN_DATA_BUFFER[25+17+7+5]; //Longest string(24), maximum precision(17), exponent/negative/decimal(7), margin(5)
    PerformanceHistory restored;
    int16 tempInt;


    loadFile >> tempInt;
    restored.id = tempInt;

    loadFile >> restored.score;
    loadFile.getline(HUMAN_DATA_BUFFER,25+17+7+5);
    loadFile >> tempInt; //rank
    loadFile.getline(HUMAN_DATA_BUFFER,25+17+7+5);

    loadFile >> restored.nonZeroWinLose;
    loadFile.getline(HUMAN_DATA_BUFFER,25+17+7+5);
    loadFile >> restored.numHandsAboveBelow;
    loadFile.getline(HUMAN_DATA_BUFFER,25+17+7+5);
    restored.totalMoneyDelta = HoldemUtil::ReadFloat64( loadFile );
    loadFile.getline(HUMAN_DATA_BUFFER,25+17+7+5);


    return restored;
}


bool HistoryStrategy::LoadState( )
{
    ifstream loadFile( (ViewPlayer().GetIdent() + ".state" ).c_str() );
    if( loadFile.is_open() )
    {
        Unserialize( loadFile );
        loadFile.close();
        return true;
    }else
    {
        loadFile.close();
        return false;
    }
}

void HistoryStrategy::SaveState( )
{

        ofstream saveFile( (ViewPlayer().GetIdent() + ".state" ).c_str() );
        Serialize( saveFile );
        saveFile.close();

        #ifdef LOAD_PAST_HANDS
        char handnumbasename[200];
        char handnumstr[20];

        const handnum_t handNumberToPlayNext = handNumber+1;
	HoldemArena::FileNumberString(handNumberToPlayNext,handnumstr);
        handnumstr[19] = '\0'; //Just to be safe

        strncpy(handnumbasename,"./" LOAD_PAST_HANDS, 2+LOAD_PAST_HANDS_LEN);
        strncat(handnumbasename, (ViewPlayer().GetIdent() + "-").c_str(), ViewPlayer().GetIdent().length() + 1);
        strncat(handnumbasename, handnumstr, 20);

        saveFile.open( handnumbasename );
        Serialize( saveFile );
        saveFile.close();
        #endif

}
