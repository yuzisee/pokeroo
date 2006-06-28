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
#include "aiCombo.h"
#include "portability.h"

using std::cout;
using std::endl;
using std::flush;

//FIRST_DEAL = 6 expects 46
//FIRST_DEAL = 5 expects 1081
//FIRST_DEAL = 4 expects 17296
//Linux DEBUG_SEED = 4 and FIRST_DEAL = 4 yields 17296 (deals 18472)

const int FIRST_DEAL = 6;

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
    	short cardcount=FIRST_DEAL;
    while(cardcount > 0)
	{

		if (rd.DealCard(h1) > 0)
		{
			if (cardcount > 2)	h2.AddToHand(rd.dealt);
			--cardcount;
		}
		printf("%d %lu\n",rd.dealt.Suit,rd.dealt.Value);


    }
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

    WinStats ds(h1, h2,FIRST_DEAL-2);
    deal.OmitCards(h1);
     deal.AnalyzeComplete(&ds);
cout << "Finish." << endl;
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
	//ConsoleStepStrategy watchPlay;

	//myTable.AddPlayer("Stag", &stagStrat);
	myTable.AddPlayer("N1", manPlay);
//	myTable.AddPlayer("N2", manPlay+1);
//	myTable.AddPlayer("N3", manPlay+2);
	myTable.AddPlayer("P1", &consolePlay);

	myTable.PlayGame();
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


	//testDR();
	//testHands();
	testW();
	//testPlay();

}

