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

#ifndef HOLDEM_AI
#define HOLDEM_AI

#include "debug_flags.h"


#include "holdem2.h"

#include "inferentials.h"



struct StatRequest
{
    StatRequest() : bNewSet(false)
    {}
    bool bNewSet;
}
;


class PlayStats
{
    protected:

		virtual void countWin(const float64);
		virtual void countSplit(const float64);
		virtual void countLoss(const float64);

		CommunityPlus myStrength;
		CommunityPlus oppStrength;

        float64 myChancesEach;
        float64 myTotalChances; //check if myTotalChances*myChancesEach was actually the number dealt
        int32 statCount;
		StatResult* myWins;

		int16 currentCard;



    public:


		int16 moreCards;
		int32 statGroup;

        virtual void Analyze() = 0;
		virtual void Compare(const float64 occ);

        const CommunityPlus & SeeCommunityOnly() { return oppStrength; }
        const CommunityPlus & SeeCommunityAndHand() { return myStrength; }

		PlayStats(const CommunityPlus& withcommunity, const CommunityPlus& onlycommunity)
		{

			myWins = 0;

			myStrength.SetUnique(withcommunity);
			oppStrength.SetUnique(onlycommunity);

			statGroup = -1;
			currentCard = 0;
		}


		//Returns whether or not you need to reset addend

		virtual StatRequest NewCard(const DeckLocation, const float64 occ) = 0;
		virtual void DropCard(const DeckLocation) = 0;
        virtual ~PlayStats();


		#ifdef PROGRESSUPDATE
		float64 handsComputed;
		#endif

}
;


class CallStats : virtual public PlayStats
{
    friend class StatsManager;
    //friend void StatsManager::Query(CallCumulation& q, const CommunityPlus& withCommunity, const CommunityPlus& onlyCommunity, int8 n);
private:
	void initC(const int8);
protected:
	CommunityPlus* myUndo;
	CommunityPlus* oppUndo;



	CallCumulation* calc;

    virtual int8 realCardsAvailable(const int8 cardsInCommunity) const;
    virtual void showProgressUpdate() const;
    virtual void setCurrentGroupOcc(const float64 occ);
    virtual void mynoAddCard(const DeckLocation& cardinfo, const int16 undoIndex){}
    virtual void myAddCard(const DeckLocation& cardinfo, const int16 undoIndex);
	virtual void myEval();
	virtual void myRevert(const int16 undoIndex);
public:

    //double myCallPct(double); //give pct of HIS percieved bankroll and returns chance to call
    
    /**
     * Analyze()
     * 
     * Discussion:
     *   Once this->myWins has been populated with raw sampled outcomes, this will reorder and accumulate them into a cumulative histogram for O(log(n)) lookup.
     */
    virtual void Analyze();
    
    virtual void DropCard(const DeckLocation);
    virtual StatRequest NewCard(const DeckLocation, const float64 occ);

	CallStats(const CommunityPlus& hP, const CommunityPlus& onlycommunity,
		int8 cardsInCommunity) : PlayStats(hP,onlycommunity)
	{
		initC(cardsInCommunity);
	}
	~CallStats();

/*
    #ifdef DUMP_CSV_PLOTS
    void dump_csv_plot(const char * dump_filename);
    #endif
*/
}
;

class WinStats : virtual public PlayStats
{
private:
	int8 cardsToNextBet;
	void initW(const int8);
	void clearDistr();
protected:
	virtual void countWin(const float64);
	virtual void countSplit(const float64);
	virtual void countLoss(const float64);
	CommunityPlus* myUndo;
	CommunityPlus* oppUndo;

	float64 oppReps;

    DistrShape *myDistr;
	StatResult myAvg;
public:
    const DistrShape& getDistr();
    const StatResult& avgStat();

	virtual void Analyze();
	virtual StatRequest NewCard(const DeckLocation, const float64 occ);
	virtual void DropCard(const DeckLocation);

	WinStats(const CommunityPlus& myP, const CommunityPlus& cP, const int8 cardsInCommunity)
    : PlayStats(myP, cP)
    ,
    oppReps(1.0)
    ,
    myDistr(0)
	{
		initW(cardsInCommunity);
	}
    ~WinStats();


}
;




#endif
