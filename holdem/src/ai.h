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


//#define DEBUGCALLPART
#define DEBUGCALLPCT

#ifndef HOLDEM_AI
#define HOLDEM_AI

#include "holdem2.h"

#include "inferentials.h"


#define PROGRESSUPDATE
//#define SUPERPROGRESSUPDATE
//#define DEBUG_TESTDEALINTERFACE

struct StatRequest
{
    StatRequest() : bNewHand(false), bTareOcc(false) {}
    bool bNewHand;
    bool bTareOcc;
}
;


class PlayStats
{
    protected:

		const virtual void countWin(const float64);
		const virtual void countSplit(const float64);
		const virtual void countLoss(const float64);

		CommunityPlus myStrength;
		CommunityPlus oppStrength;

        float64 myChancesEach;
        float64 myTotalChances; //check if myTotalChances*myChancesEach was actually the number dealt
        int32 statCount;
		StatResult* myWins;

		int16 currentCard;



        //std::vector<int32> myWins;
        //std::vector<int32> myReps;
        //std::vector<int32> oppWins;
        //std::vector<int32> oppReps;

    public:

		int16 moreCards;
		int32 statGroup;

        const virtual void Analyze() = 0;
		const virtual void Compare(const float64 occ);



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
		const virtual void DropCard(const DeckLocation) = 0;
        virtual ~PlayStats();


			#ifdef DEBUGCALLPART
				void virtual debugPrint() {}
				const virtual DeckLocation* debugViewD(int8 i) { return 0;}
			#endif
}
;


class CallStats : virtual public PlayStats
{
    friend class StatsManager;
    //friend void StatsManager::Query(CallCumulation& q, const CommunityPlus& withCommunity, const CommunityPlus& onlyCommunity, int8 n);
private:
	const void initC(const int8);
protected:
	CommunityPlus* myUndo;
	CommunityPlus* oppUndo;

	CallCumulation* calc;

    const virtual int8 realCardsAvailable(const int8 cardsInCommunity) const;
    const virtual void showProgressUpdate() const;
    const virtual void setCurrentGroupOcc(const float64 occ);
    const virtual void mynoAddCard(const DeckLocation& cardinfo, const int16 undoIndex){}
    const virtual void myAddCard(const DeckLocation& cardinfo, const int16 undoIndex);
	const virtual void myEval();
	const virtual void myRevert(const int16 undoIndex);
public:
	float64 pctWillCall(const float64) const;

    //double myCallPct(double); //give pct of HIS percieved bankroll and returns chance to call
    const virtual void Analyze();
    const virtual void DropCard(const DeckLocation);
    virtual StatRequest NewCard(const DeckLocation, const float64 occ);

	CallStats(const CommunityPlus& hP, const CommunityPlus& onlycommunity,
		int8 cardsInCommunity) : PlayStats(hP,onlycommunity)
	{
		initC(cardsInCommunity);
	}
	~CallStats();

		#ifdef DEBUGCALLPART
			void debugPrint()
			{
				cout << endl << "CallStats repeated=" << myWins[statGroup].repeated;
				oppStrength.DisplayHandBig(cout);
				myStrength.DisplayHandBig(cout);
			}
		#endif
}
;

class WinStats : virtual public PlayStats
{
private:
	short cardsToNextBet;
	const void initW(const int8);
	void clearDistr();
protected:
	const virtual void countWin(const float64);
	const virtual void countSplit(const float64);
	const virtual void countLoss(const float64);
	CommunityPlus* myUndo;
	CommunityPlus* oppUndo;

    DistrShape *myDistrPCT;
	DistrShape *myDistrWL;
	StatResult myAvg;
public:
    const DistrShape& pctDistr();
    const DistrShape& wlDistr();
    const StatResult& avgStat();

	const virtual void Analyze();
	virtual StatRequest NewCard(const DeckLocation, const float64 occ);
	const virtual void DropCard(const DeckLocation);

	WinStats(const CommunityPlus& myP, const CommunityPlus& cP,
		const int8 cardsInCommunity) : PlayStats(myP, cP), myDistrPCT(0), myDistrWL(0)
	{
		initW(cardsInCommunity);
	}
    ~WinStats();

//	float64 myImproveChance;


//	StatResult myWorst;
//	StatResult myBest;

}
;


#ifdef DEBUG_TESTDEALINTERFACE

class DummyStats : virtual public PlayStats
{
	public:
		const virtual void Analyze(){};
		virtual StatRequest NewCard(const DeckLocation dk, const float64 occ)
		{
		    ++currentCard;
            //cout << "@" << currentCard << "\t" << (int)(dk.GetIndex()) << endl;
		    if( currentCard == 1 )
		    {
		        myStrength.SetUnique(oppStrength);
		        myStrength.AddToHand(dk);
		    }
		    else if ( currentCard == 2 )
		    {
		        CommunityPlus viewer;
		        viewer.SetUnique(myStrength);
		        viewer.AddToHand(dk);
		        viewer.evaluateStrength();
		        viewer.DisplayHandBig(cout);
		    }
			StatRequest a;
			a.bNewHand = false;
			a.bTareOcc = false;
			return a;
		};
		const virtual void DropCard(const DeckLocation)
		{
		    --currentCard;

		};
		DummyStats(const CommunityPlus& holeCards, const CommunityPlus& community, const int8 cardsInCommunity)
				: PlayStats(holeCards, community)
		{
			moreCards = 7 - cardsInCommunity;
			statGroup = 0;
			myWins = new StatResult[1];
			myWins[0].wins = 0;
			myWins[0].loss = 0;
			myWins[0].splits = 0;
			myWins[0].repeated = 0;
            oppStrength.SetEmpty();
            oppStrength.SetUnique(community);
            oppStrength.DisplayHandBig(cout);
            currentCard=0;
		}

}
;
#endif



#endif
