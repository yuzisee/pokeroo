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

#define DEBUGSITUATION


#include "stratManual.h"
#include "stratThreshold.h"
#include "arena.h"
#include "aiCache.h"
#include "engine.h"
#include "portability.h"
#include "functionmodel.h"
#include "arenaSituation.h"

using std::cout;
using std::endl;
using std::flush;

//FIRST_DEAL = 6 expects 46
//FIRST_DEAL = 5 expects 1081
//FIRST_DEAL = 4 expects 17296
//Linux DEBUG_SEED = 4 and FIRST_DEAL = 4 yields 17296 (deals 18472)

const int FIRST_DEAL = 5;

void genW(CommunityPlus& h1, CommunityPlus& h2)
{
    DealRemainder deal;

#ifdef DEBUGSITUATION
	cout << "Cards available to me" << endl;
	h1.ShowHand(false);
	cout << endl;
#endif

#ifdef DEBUGSITUATION
	cout << "Cards in community" << endl;
	h2.ShowHand(false);
	cout << endl;

	cout << endl;
#endif

    cout << "position" << endl;
    TriviaDeck td;
    td.OmitCards(h1);
    td.DiffHand(h2);
    td.sortSuits();
    cout << td.NamePockets() << endl;

    //WinStats ds(h1, h2,FIRST_DEAL-2);
    StatResult myWins;
    DistrShape myDistrPCT(0);
    StatsManager::Query( &myWins,&myDistrPCT,0,h1, h2,0);
    //deal.OmitCards(h1);
    // deal.AnalyzeComplete(&ds);

    cout << endl << "AVG "  << myWins.loss << " l + "
            << myWins.splits << " s + " << myWins.wins << " w = " <<
            myWins.loss+myWins.splits+myWins.wins
            << "\t×"<< myWins.repeated   <<endl;

    cout << "myAvg.genPCT " << myWins.pct << "!"  << endl;
    cout << "(Mean) " << myDistrPCT.mean * 100 << "%"  << endl;
    cout << endl << "Adjusted improve? " << myDistrPCT.improve * 100 << "%"  << endl;
    cout << "Worst:" << myDistrPCT.worst *100 << "%" << endl;
    cout << "Standard Deviations:" << myDistrPCT.stdDev*100 << "%" << endl;
    cout << "Average Absolute Fluctuation:" << myDistrPCT.avgDev*100 << "%" << endl;
    cout << "Skew:" << myDistrPCT.skew*100 << "%" << endl;
    cout << "Kurtosis:" << (myDistrPCT.kurtosis)*100 << "%" << endl;

    cout << endl;

cout << "Finish." << endl;
}

void genW(CommunityPlus& h1)
{
    CommunityPlus h2;
    genW(h1,h2);
}

void testHT(int bonus = 0, int bOffsuit = 0)
{
	HandPlus h1;
	RandomDeck rd;
	rd.ShuffleDeck(bonus);

	//CallCumulationMap map;

	short cardcount=0;
	if( bOffsuit == 0 )
	{
		while(cardcount < FIRST_DEAL)
		{
			rd.DealCard(h1);
			++cardcount;
		}
		cout << "Cards dealt" << endl;
		h1.HandPlus::DisplayHandBig();
		cout << endl;
	}else
	{
		unsigned char carda = (rand()%(13));
		unsigned char cardb = (rand()%(13));
		cardcount = 2;
		if( bOffsuit == 1 )
		{
			h1.AddToHand(0,carda,HoldemUtil::CARDORDER[carda]);
			h1.AddToHand(1,cardb,HoldemUtil::CARDORDER[cardb]);
		}
		else if( bOffsuit == 2 )
		{
			h1.AddToHand(0,carda,HoldemUtil::CARDORDER[carda]);
			h1.AddToHand(1,carda,HoldemUtil::CARDORDER[carda]);
		}
		else if( bOffsuit == -1 )
		{
			h1.AddToHand(0,carda,HoldemUtil::CARDORDER[carda]);
			h1.AddToHand(0,cardb,HoldemUtil::CARDORDER[cardb]);
		}
	}


	//map.BuildTable(h1);


}

void testW()
{

    	RandomDeck rd;
    	rd.ShuffleDeck();
    CommunityPlus h1, h2;
    	short cardcount=2;
    while(cardcount > 0)
	{

		if (rd.DealCard(h1) > 0)
		{
			if (cardcount > 2)	h2.AddToHand(rd.dealt);
			--cardcount;
		}
		printf("%d %lu\n",rd.dealt.Suit,rd.dealt.Value);


    }
    genW(h1,h2);
}



void genC(CommunityPlus& h1, CommunityPlus& h2)
{

     DealRemainder deal;

#ifdef DEBUGSITUATION
	cout << "Cards available to me" << endl;
	h1.ShowHand(false);
	cout << endl;
#endif

#ifdef DEBUGSITUATION
	cout << "Cards in community" << endl;
	h2.ShowHand(false);
	cout << endl;

	cout << endl;
#endif

    cout << "position" << endl;
    TriviaDeck td;
    td.OmitCards(h1);
    td.DiffHand(h2);
    td.sortSuits();
    cout << td.NamePockets() << endl;

    //WinStats ds(h1, h2,FIRST_DEAL-2);
    CallCumulationD calc;
    StatsManager::Query( calc,h1, h2,0);
    //deal.OmitCards(h1);
    // deal.AnalyzeComplete(&ds);

    cout << endl << "=============Reduced=============" << endl;
	cout.precision(4);
	size_t vectorLast = calc.cumulation.size();
	for(size_t i=0;i<vectorLast;i++)
	{
		cout << endl << "{" << i << "}" << calc.cumulation[i].loss << " l +\t"
				<< calc.cumulation[i].splits << " s +\t" << calc.cumulation[i].wins << " w =\t" <<
				calc.cumulation[i].pct
				<< " pct\t×"<< calc.cumulation[i].repeated <<flush;
	}


	cout << endl << "Confirm that -0.5:1 odds receives " << calc.pctWillCall(2) <<  endl;
	cout << endl << "Confirm that .0101:1 odds receives " << calc.pctWillCall(.99) << endl;
	cout << endl << "Confirm that .333:1 odds receives " << calc.pctWillCall(.75) <<  endl;
	cout << endl << "Confirm that 1:1 odds receives " << calc.pctWillCall(.5) << endl;
	cout << endl << "Confirm that heads up 1.333x pot odds receives " << calc.pctWillCall(.3)  << endl;
	cout << endl << "Confirm that heads up 98x pot odds (ie. I bet 1/98th of the pot) receives " << calc.pctWillCall(.01) << endl;
	cout << endl << "Confirm that inf:1 odds receives " << calc.pctWillCall(0) <<  endl;
	cout << endl << "Confirm that -2:1 odds receives " << calc.pctWillCall(-1) << endl;

	cout << endl << "Confirm that -0.5:1 odds slope " << calc.pctWillCallD(2) <<  endl;
	cout << endl << "Confirm that .0101:1 odds slope " << calc.pctWillCallD(.99) << endl;
	cout << endl << "Confirm that .333:1 odds slope " << calc.pctWillCallD(.75) <<  endl;
	cout << endl << "Confirm that 1:1 odds slope " << calc.pctWillCallD(.5) << endl;
	cout << endl << "Confirm that heads up 1.333x pot odds slope " << calc.pctWillCallD(.3)  << endl;
	cout << endl << "Confirm that heads up 98x pot odds (ie. I bet 1/98th of the pot) slope " << calc.pctWillCallD(.01) << endl;
	cout << endl << "Confirm that inf:1 odds slope " << calc.pctWillCallD(0) <<  endl;
	cout << endl << "Confirm that -2:1 odds slope " << calc.pctWillCallD(-1) << endl;

/*
	cout << endl << "Confirm that -0.5:1 odds receives " << calc.pctWillCallDEBUG(2,.5) <<  endl;
	cout << endl << "Confirm that .0101:1 odds receives " << calc.pctWillCallDEBUG(.99,.5) << endl;
	cout << endl << "Confirm that .333:1 odds receives " << calc.pctWillCallDEBUG(.75,.5) <<  endl;
	cout << endl << "Confirm that 1:1 odds receives " << calc.pctWillCallDEBUG(.5,.5) << endl;
	cout << endl << "Confirm that heads up 1.333x pot odds receives " << calc.pctWillCallDEBUG(.3,.5)  << endl;
	cout << endl << "Confirm that heads up 98x pot odds (ie. I bet 1/98th of the pot) receives " << calc.pctWillCallDEBUG(.01,.5) << endl;
	cout << endl << "Confirm that inf:1 odds receives " << calc.pctWillCallDEBUG(0,.5) <<  endl;
	cout << endl << "Confirm that -2:1 odds receives " << calc.pctWillCallDEBUG(-1,.5) << endl;
*/

cout << "Finish." << endl;
}
void genC(CommunityPlus& h1)
{
    CommunityPlus h2;
    genC(h1,h2);
}

void testC()
{

    RandomDeck rd;
    rd.ShuffleDeck();
    CommunityPlus h1, h2;
    short cardcount=2;
    while(cardcount > 0){
		if (rd.DealCard(h1) > 0){
			if (cardcount > 2)	h2.AddToHand(rd.dealt);
			--cardcount;
		}
		printf("%d %lu\n",rd.dealt.Suit,rd.dealt.Value);
    }
    genC(h1,h2);
}

void testHands()
{
    	RandomDeck rd;
    	rd.ShuffleDeck();
    CommunityPlus h1, h2;
    	short cardcount=7;
    while(cardcount > 0)
	{

		if (rd.DealCard(h1) > 0)
		{
			if (cardcount <= 5)	h2.AddToHand(rd.dealt);
			--cardcount;
		}
		printf("%d %lu\n",rd.dealt.Suit,rd.dealt.Value);


    }
    DealRemainder deal;

    DummyStats ds(h1, h2,5);
    deal.OmitCards(h1);
     deal.AnalyzeComplete(&ds);
}

void testDR()
{

	DealRemainder deal;
	HandPlus h1, h2, dum;
	short cardcount=0;
	int unsigned lastCardCount=0;

	//OrderedDeck rd;
	RandomDeck rd;

	while(cardcount < FIRST_DEAL)
	{
		lastCardCount=cardcount;

		rd.DealCard(dum);
		rd.DealCard(dum);

		/*deal.dealtValue = HoldemUtil::CARDORDER[ (rand() % 13) + 1 ];
		deal.dealtSuit = rand() % 4;*/

		deal.dealt = rd.dealt;

		if (deal.DealCard(h1) > 0)
		{
			if (cardcount >= 2)	h2.AddToHand(deal.dealt);
		}
		printf("%d %lu\n",deal.dealt.Suit,deal.dealt.Value);
        //if(HoldemConstants::CARD_ACELOW != deal.dealtValue)
        //{h1.AddToHand(deal.dealtSuit,deal.dealtValue);}
		cardcount = 0;
		for(int gs=0;gs<4;++gs)
			for(int g=1;g<=14;++g)
		{
			if((h1.SeeCards(gs) & (1 << g)) > 0) ++cardcount;
		}
        //if(lastCardCount==cardcount)
	}

#ifdef DEBUGSITUATION
	cout << "Cards available to me" << endl;
	h1.ShowHand(true);
	cout << endl;
#endif
	/*
	a.FillShowHand(h1.cardset,false);printf("\n");
	CommunityPlus cm(h1.cardset,h1.suitCount);
	cm.DisplayHandBig();printf("\n");
	cm.DisplayHand();printf("\n");
	*/
#ifdef DEBUGSITUATION
	cout << "Cards in community" << endl;
	h2.ShowHand(true);
	cout << endl;

//	a.FillShowHand(h2.cardset,false);
	cout << endl;
#endif
	if( (--(--cardcount)) < 0 ) cardcount = 0;

	//DummyStats ps(h1,h2, cardcount);
#ifdef DEBUGSITUATION
	cout << endl << "Analyze! " << cardcount << endl;
#endif


//for (int i=0;i<2;++i)
//{
//	if( i == 1 )deal.bRecursive = true;
//	if( i == 0 )deal.bRecursive = false;

	CommunityPlus m, co;
	//h1.DisplayHandBig();
	m.SetUnique(h1);


			/*		cout << "b" << m.bestPair << " n" << m.nextbestPair << endl;
					cout << m.threeOfAKind << endl;
					m.HandPlus::DisplayHandBig();
					cout <<endl<<endl;
h2.DisplayHandBig();*/
co.SetUnique(h2);
					/*cout << "b" << co.bestPair << " n" << co.nextbestPair << endl;
					cout << co.threeOfAKind << endl;
					co.HandPlus::DisplayHandBig();
*/

/*
	WinCallStats ws(m,co,cardcount);
*/

	CallStats ps(m,co,cardcount);

cout << "In" << endl;
double r = deal.Analyze(&ps,deal.BaseDealtSuit(),deal.BaseDealtRank(),deal.BaseDealtValue());
cout << "Returned from there" << endl;

#ifdef DEBUGSITUATION
	cout << endl << "Finished " << r << "\t" << flush;
//	cout << endl << "Deals " << deal.deals << "\t" << endl;

	cout << endl << "Confirm that -0.5:1 odds receives " << ps.pctWillCall(2) <<  endl;
	cout << endl << "Confirm that .0101:1 odds receives " << ps.pctWillCall(.99) << endl;
	cout << endl << "Confirm that .333:1 odds receives " << ps.pctWillCall(.75) <<  endl;
	cout << endl << "Confirm that 1:1 odds receives " << ps.pctWillCall(.5) << endl;
	cout << endl << "Confirm that heads up 1.333x pot receives " << ps.pctWillCall(.3)  << endl;
	cout << endl << "Confirm that heads up 98x pot receives " << ps.pctWillCall(.01) << endl;
	cout << endl << "Confirm that inf:1 odds receives " << ps.pctWillCall(0) <<  endl;
	cout << endl << "Confirm that -2:1 odds receives " << ps.pctWillCall(-1) << endl;
#endif
/*
#ifdef DEBUGSITUATION

cout << endl << "Finished " << deal.AnalyzeComplete(&ws) << endl;
	//cout << endl << "Finished " << r << "\t" << flush;
//	cout << endl << "Deals " << deal.deals << "\t" << endl;

	cout << endl << "Confirm that -0.5:1 odds receives " << ws.pctWillCall(2) <<  endl;
	cout << endl << "Confirm that .0101:1 odds receives " << ws.pctWillCall(.99) << endl;
	cout << endl << "Confirm that .333:1 odds receives " << ws.pctWillCall(.75) <<  endl;
	cout << endl << "Confirm that 1:1 odds receives " << ws.pctWillCall(.5) << endl;
	cout << endl << "Confirm that heads up 1.333x pot receives " << ws.pctWillCall(.3)  << endl;
	cout << endl << "Confirm that heads up 98x pot receives " << ws.pctWillCall(.01) << endl;
	cout << endl << "Confirm that inf:1 odds receives " << ws.pctWillCall(0) <<  endl;
	cout << endl << "Confirm that -2:1 odds receives " << ws.pctWillCall(-1) << endl;


#endif
//}



#ifdef DEBUGSITUATION
	cout << endl << "Review callstats " << r << "\t" << flush;
//	cout << endl << "Deals " << deal.deals << "\t" << endl;

	cout << endl << "Confirm that -0.5:1 odds receives " << ps.pctWillCall(2) <<  endl;
	cout << endl << "Confirm that .0101:1 odds receives " << ps.pctWillCall(.99) << endl;
	cout << endl << "Confirm that .333:1 odds receives " << ps.pctWillCall(.75) <<  endl;
	cout << endl << "Confirm that 1:1 odds receives " << ps.pctWillCall(.5) << endl;
	cout << endl << "Confirm that heads up 1.333x pot receives " << ps.pctWillCall(.3)  << endl;
	cout << endl << "Confirm that heads up 98x pot receives " << ps.pctWillCall(.01) << endl;
	cout << endl << "Confirm that inf:1 odds receives " << ps.pctWillCall(0) <<  endl;
	cout << endl << "Confirm that -2:1 odds receives " << ps.pctWillCall(-1) << endl;
#endif
*/

}


void testPlay()
{
	BlindStructure b(1.25,2.5);
	HoldemArena myTable(&b, true);
	//ThresholdStrategy stagStrat(0.5);
	UserConsoleStrategy consolePlay;
	ConsoleStrategy manPlay[3];
	MultiThresholdStrategy pushFold;
	//ConsoleStepStrategy watchPlay;

	//myTable.AddPlayer("Stag", &stagStrat);
	myTable.AddPlayer("N1", manPlay);
	myTable.AddPlayer("N2", manPlay+1);
	myTable.AddPlayer("X3", &pushFold);
	myTable.AddPlayer("P1", &consolePlay);
	//myTable.AddPlayer("P1", manPlay);

	myTable.PlayGame();
}

void genCMD(uint16 procnum)
{
	    uint16 handnum = procnum % 338;///From 0-675 to 0-337
	    procnum = procnum/338;///0 or 1

	    uint16 card1 = handnum / 26; ///0 to 12
	    uint16 card2 = handnum % 26; ///0 to 25


	    ///accomodate the suit-major ordering of HoldemUtil
	    card1 *= 4;
	    card2 *= 2;

        if( card1 != card2 )
        {
            DeckLocation hands[2];

            hands[0].Suit = 0;
            hands[1].Suit = HoldemUtil::CardSuit( card2 )/2;

            hands[0].Rank = HoldemUtil::CardRank( card1 )+1;
            hands[1].Rank = HoldemUtil::CardRank( card2 )+1;

            hands[0].Value = HoldemUtil::CARDORDER[hands[0].Rank];
            hands[1].Value = HoldemUtil::CARDORDER[hands[1].Rank];

            cout << (int)(hands[0].Suit) << "\t" << (int)(hands[0].Rank) << "\t" << hands[0].Value << endl;
            cout << (int)(hands[1].Suit) << "\t" << (int)(hands[1].Rank) << "\t" << hands[1].Value << endl;


            CommunityPlus h1;
            h1.AddToHand(hands[0]);
            h1.AddToHand(hands[1]);



            if( procnum == 0 )
            {
                //h1.DisplayHand();
                genC(h1);
                return;
            }

            if( procnum == 1 )
            {
                //h1.DisplayHandBig();
                genW(h1);
                return;
            }
        }

}

void goCMD(int argc, char* argv)
{
 genCMD(atoi(argv));
}


void testFunctions()
{
    DummyFunctionModel m(0.0001);
    cout << m.FindMax(0,1.4) << endl;
    cout << m.FindZero(0,1.4) << endl;

//	StatResult t = GainModel::ComposeBreakdown(0.5,0.7);
	//StatResult t = GainModel::ComposeBreakdown(0.6,0.7);
	StatResult t = GainModel::ComposeBreakdown(0.4,0.45);
		cout << endl << t.loss << " l +\t"
				<< t.splits << " s +\t" << t.wins << " w =\t" <<
				t.pct
				<< " pct\t"<< t.genPeripheral() <<flush;
}

void testPosition()
{
    CommunityPlus h1, h2;









//Community
    h2.AddToHand(HoldemConstants::HEARTS, 12, HoldemConstants::CARD_KING );
    h2.AddToHand(HoldemConstants::DIAMONDS, 7, HoldemConstants::CARD_EIGHT );
    h2.AddToHand(HoldemConstants::CLUBS, 4, HoldemConstants::CARD_FIVE );

    h1.SetUnique(h2);

//Hole cards
    h1.AddToHand(HoldemConstants::SPADES, 12, HoldemConstants::CARD_KING );
    h1.AddToHand(HoldemConstants::DIAMONDS, 13, HoldemConstants::CARD_ACEHIGH );
    const uint8 dealtCommunityNumber=3;












#ifdef DEBUGSITUATION
	cout << "Cards available to me" << endl;
	h1.ShowHand(false);
	cout << endl;
#endif

#ifdef DEBUGSITUATION
	cout << "Cards with community" << endl;
	h2.ShowHand(false);
	cout << endl;

	cout << endl;
#endif

    //WinStats ds(h1, h2,FIRST_DEAL-2);
    DistrShape myDistrPCT(0);
    DistrShape myDistrWL(0);
	CallCumulationD o;
    StatsManager::Query(0,&myDistrPCT,&myDistrWL,h1, h2,dealtCommunityNumber);
    StatsManager::Query(o,h1, h2,dealtCommunityNumber);
	//deal.OmitCards(h1);
    // deal.AnalyzeComplete(&ds);

   /* cout << endl << "AVG "  << myWins.loss << " l + "
            << myWins.splits << " s + " << myWins.wins << " w = " <<
            myWins.loss+myWins.splits+myWins.wins
            << "\t×"<< myWins.repeated   <<endl;

    cout << "myAvg.genPCT " << myWins.pct << "!"  << endl;*/
    cout << "(Mean) " << myDistrPCT.mean * 100 << "%"  << endl;
    cout << endl << "Adjusted improve? " << myDistrPCT.improve * 100 << "%"  << endl;
    cout << "Worst:" << myDistrPCT.worst *100 << "%" << endl;
    cout << "Standard Deviations:" << myDistrPCT.stdDev*100 << "%" << endl;
    cout << "Average Absolute Fluctuation:" << myDistrPCT.avgDev*100 << "%" << endl;
    cout << "Skew:" << myDistrPCT.skew*100 << "%" << endl;
    cout << "Kurtosis:" << (myDistrPCT.kurtosis)*100 << "%" << endl;



	BlindStructure b(0.005,0.01);
	HoldemArena myTable(&b, true);
    UserConsoleStrategy testDummy;
    UserConsoleStrategy testDummy2;//CAN'T ADD THE SAME STRATEGY TWICE!
	myTable.AddPlayer("TestDummy",1, &testDummy);
	//myTable.AddPlayer("TestDummyOpponent",1, &testDummy2);

    ExactCallD myExpectedCall(0, &myTable, &o);


///TODO compare with revision 46
	GainModel g(GainModel::ComposeBreakdown(myDistrPCT.mean,myDistrWL.mean),&myExpectedCall);
	float64 turningPoint = g.FindMax(0,1);
        #ifdef DEBUG_GAIN
            std::ofstream excel("functionlog.csv");
            g.breakdown(40,excel,0,0.4);
            //g.breakdownE(40,excel);
            excel.close();

            cout << endl << endl;

            cout << "Goal bet " << turningPoint << endl;
            cout << "Fold bet " << g.FindZero(turningPoint,1) << endl;
        #endif


	GainModel gm(GainModel::ComposeBreakdown(myDistrPCT.worst,myDistrWL.worst),&myExpectedCall);
	turningPoint = gm.FindMax(0,1);

        #ifdef DEBUG_GAIN
            cout << "Minimum target " << turningPoint << endl;
            cout << "Safe fold bet " << gm.FindZero(turningPoint,1) << endl;

            excel.open("functionlog.safe.csv");
            gm.breakdown(40,excel,0,0.4);
            //g.breakdownE(40,excel);
            excel.close();
        #endif

    cout << endl;
}

int main(int argc, char* argv[])
{
	cout << "test" << endl;
	/*testHT(91, 1);
	testHT(62, 1);
	testHT(33, 1);
	testHT(84, 1);
	testHT(55, 1);
	testHT(6, 1);
	testHT(55, 2);
	testHT(6, 2);
	testHT(291, -1);
	testHT(262, -1);
	testHT(933, -1);*/
	//testHT();


	//
	//testHands();


	if( argc == 2 )
	{
        goCMD(2,argv[1]);
    }else if( argc == 4 )
    {
        uint16 i=675;
		while(1)
        {
             genCMD(i);
			 if( i == 0 ) break;
			 --i;
        }
	}else
	{

	    ///Play this hand on force-random
	    ///Check pre-flop and push all-in after the flop.
	    ///Now, monitor how the money is divided between N2 and X1.
	    testPosition();
	    //testPlay();
	    //testFunctions();
		//testC();
		//goCMD(2,"505");

	}
	//testPlay();

}

