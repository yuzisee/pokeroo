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

#ifndef HOLDEM_AI
#define HOLDEM_AI

#include "debug_flags.h"


//#define DEBUGCALLPART
//#define DEBUGCALLPCT
#define DEBUG_AA


#include "holdem2.h"

#include "inferentials.h"



struct StatRequest
{
    StatRequest() : bNewSet(false)
    {}//, bTareOcc(false) {}
    bool bNewSet;
    //bool bTareOcc;
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



        //std::vector<int32> myWins;
        //std::vector<int32> myReps;
        //std::vector<int32> oppWins;
        //std::vector<int32> oppReps;

    public:

#ifdef DEBUG_AA
        bool bDEBUG;
#endif
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
	void initC(const int8);
protected:
	CommunityPlus* myUndo;
	CommunityPlus* oppUndo;

	CallCumulation* calc;

    const virtual int8 realCardsAvailable(const int8 cardsInCommunity) const;
    virtual void showProgressUpdate() const;
    virtual void setCurrentGroupOcc(const float64 occ);
    virtual void mynoAddCard(const DeckLocation& cardinfo, const int16 undoIndex){}
    virtual void myAddCard(const DeckLocation& cardinfo, const int16 undoIndex);
	virtual void myEval();
	virtual void myRevert(const int16 undoIndex);
public:

    //double myCallPct(double); //give pct of HIS percieved bankroll and returns chance to call
    virtual void Analyze();
    virtual void DropCard(const DeckLocation);
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
	void initW(const int8);
	void clearDistr();
protected:
	virtual void countWin(const float64);
	virtual void countSplit(const float64);
	virtual void countLoss(const float64);
	CommunityPlus* myUndo;
	CommunityPlus* oppUndo;

    DistrShape *myDistrPCT;
	DistrShape *myDistrWL;
	StatResult myAvg;
public:
    const DistrShape& pctDistr();
    const DistrShape& wlDistr();
    const StatResult& avgStat();

	virtual void Analyze();
	virtual StatRequest NewCard(const DeckLocation, const float64 occ);
	virtual void DropCard(const DeckLocation);

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
		virtual void Analyze(){};
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
		virtual void DropCard(const DeckLocation)
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
