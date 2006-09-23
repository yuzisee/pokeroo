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

#include "stratPosition.h"
#include "stratManual.h"
#include "stratThreshold.h"
#include "arena.h"
#include "aiCache.h"
#include "engine.h"
#include "portability.h"
#include "functionmodel.h"
#include "arenaSituation.h"
#include "aiInformation.h"
#include <algorithm>

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
    NamedTriviaDeck td;
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
    NamedTriviaDeck td;
    td.OmitCards(h1);
    td.DiffHand(h2);
    td.sortSuits();
    cout << td.NamePockets() << endl;

    //WinStats ds(h1, h2,FIRST_DEAL-2);
    CallCumulationD calc;
    StatsManager::QueryOffense( calc,h1, h2,0);
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


void testFunction()
{
    RandomDeck rd;
    RandomDeck rd2;
    rd.ShuffleDeck();
    cout << "Dealing" << endl;
    float64* a = new float64[50*50*50*50];
    float64* c = new float64[50*50*50*50];
    float64 asum = 0;
    int qcount = 0;
    Hand x;
    a[0] = rd.DealCard(x) + rd2.DealCard(x);
    c[0] = a[0];
    for(int32 i=1;i<50*50*50*50;++i)
    {
        x.Empty();
        a[i] = (a[i-1]*17 + (rd.dealt.Value+5)/(rd.dealt.Suit+5)*19)/41;
        if( a[i] > 1e30 || a[i] < -1e30 )
        {
            a[i] = rd.dealt.Rank + rd2.dealt.Value/(rd2.dealt.Suit+13);
            if( rd.DealCard(x) * rd2.DealCard(x) == 0 )
            {
                rd.ShuffleDeck();
                rd2.ShuffleDeck();
            }
        }else
        {
            a[i] -= rd.dealt.Rank ;
            a[i] += rd.DealCard(x) + rd2.DealCard(x);
            if( rd.DealCard(x) * rd2.DealCard(x) == 0 )
            {
                rd.ShuffleDeck();
                rd2.ShuffleDeck();
            }
        }
        if( rd.dealt.GetIndex() > rd2.dealt.GetIndex() )
        {
            asum = (asum * 11 + a[i] * 2) / 13;
        }else
        {
            float64 z = 1;
            unsigned long long b = (*reinterpret_cast<unsigned long long*>(a+i)) ^ (*reinterpret_cast<unsigned long long*>(a+i-1));
            b ^= *reinterpret_cast<unsigned long long*>(&z);
            asum = *reinterpret_cast<float64*>(&b);
        }
        a[i] += asum;
        if( qcount == 0 )
        {
            qcount = 50*50*50;
            cout << i << "\r" << flush;
        }
        --qcount;
        c[i] = a[i];
    }

    for(int32 i=100000;i<100050;++i)
    {
        cout << i << " " << a[i];
        if( a[i] == a[i-1] ) cout << endl;
    }
    cout << "Sorting" << endl;

    std::sort(a,a+50*50*50*50);
    cout << "Done" << endl;
    int32 i;
    for( i=100000;i<100050;++i)
    {
        cout << i << " " << a[i];
        if( a[i] == a[i-1] ) cout << endl;
    }
    for(int32 j=0;j<50*50*50*50;++j)
    {
        if( c[j] == a[i] )
        {
            cout << (j-1) << " c " << c[j-1] << endl;
            cout << j << " c " << c[j] << endl;
        }

    }

    delete [] a;
    delete [] c;
    cout << "Cleanup" << endl;
    /*
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
				*/
}

void testNewCallStats()
{
    CommunityPlus h1, h2;








//Community

    //h2.AddToHand(HoldemConstants::HEARTS, 11, HoldemConstants::CARD_QUEEN );
    //h2.AddToHand(HoldemConstants::HEARTS, 3, HoldemConstants::CARD_FOUR );
    //h2.AddToHand(HoldemConstants::SPADES, 1, HoldemConstants::CARD_DEUCE );

    //h2.AddToHand(HoldemConstants::CLUBS, 9, HoldemConstants::CARD_TEN );
    //h2.AddToHand(HoldemConstants::CLUBS, 1, HoldemConstants::CARD_DEUCE );
    h1.SetUnique(h2);

//Hole cards
    h1.AddToHand(HoldemConstants::SPADES, 13, HoldemConstants::CARD_ACEHIGH );
    h1.AddToHand(HoldemConstants::CLUBS, 13, HoldemConstants::CARD_ACEHIGH );
    const uint8 dealtCommunityNumber=0;


    /*CommunityPlus emptyCards;
    PreflopCallStats pfcs(h1, emptyCards);
    pfcs.AutoPopulate();
    pfcs.Analyze();*/

    DealRemainder myStatBuilder;
    myStatBuilder.UndealAll();
    myStatBuilder.OmitCards(h2); ///Very smart, omit h2 NOT h1, because the opponent can think you have the cards you have

    CommunityCallStats ds(h1, h2,dealtCommunityNumber);
    myStatBuilder.AnalyzeComplete(&ds);
}

void testPosition()
{
    CommunityPlus h1, h2;




/*
    h2.AddToHand(HoldemConstants::HEARTS, 11, HoldemConstants::CARD_QUEEN );
    h2.AddToHand(HoldemConstants::HEARTS, 3, HoldemConstants::CARD_FOUR );
    h2.AddToHand(HoldemConstants::SPADES, 1, HoldemConstants::CARD_DEUCE );
    h1.SetUnique(h2);

    h1.AddToHand(HoldemConstants::CLUBS, 13, HoldemConstants::CARD_ACEHIGH );
    h1.AddToHand(HoldemConstants::CLUBS, 1, HoldemConstants::CARD_DEUCE );
    const uint8 dealtCommunityNumber=3;

    */

/*

//Community
    h2.AddToHand(HoldemConstants::DIAMONDS, 12, HoldemConstants::CARD_KING );
    //h2.AddToHand(HoldemConstants::HEARTS, 11, HoldemConstants::CARD_QUEEN );
    h2.AddToHand(HoldemConstants::SPADES, 8, HoldemConstants::CARD_NINE );
    //h2.AddToHand(HoldemConstants::HEARTS, 3, HoldemConstants::CARD_FOUR );
    h2.AddToHand(HoldemConstants::SPADES, 1, HoldemConstants::CARD_DEUCE );
    h1.SetUnique(h2);
*/
//Hole cards
    //h1.AddToHand(HoldemConstants::SPADES, 11, HoldemConstants::CARD_QUEEN );
    //h1.AddToHand(HoldemConstants::CLUBS, 13, HoldemConstants::CARD_ACEHIGH );
    h1.AddToHand(HoldemConstants::HEARTS, 3, HoldemConstants::CARD_FOUR );
    h1.AddToHand(HoldemConstants::CLUBS, 7, HoldemConstants::CARD_EIGHT );
    const uint8 dealtCommunityNumber=0;












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
    StatsManager::QueryOffense(o,h1, h2,dealtCommunityNumber);
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


    float64 chipDenom = .25;
	BlindStructure b(chipDenom,chipDenom*2);
	DebugArena myTable(&b, true);
    UserConsoleStrategy testDummy[3];

	myTable.SetDeadPot(0);
	const float64 t_chipCount = 87.5;
	const float64 t_myBet = 0;//myTable.GetSmallBlind();
    myTable.SetBet(  myTable.AddPlayer("TestDummy",t_chipCount, testDummy) , t_myBet );
    //myTable.SetBet(  myTable.AddPlayer("X3",125, testDummy+1) ,  myTable.GetSmallBlind() );


    const bool bCallAllIn = false;
    float64 oppBet = 87.5;
    if ( !bCallAllIn ) oppBet = myTable.GetBigBlind() ;
    StatResult worstStatResult = GainModel::ComposeBreakdown(myDistrPCT.worst,myDistrWL.worst);
    StatResult myChoiceStatResult = worstStatResult;
    if( bCallAllIn ) myChoiceStatResult = GainModel::ComposeBreakdown(myDistrPCT.mean,myDistrWL.mean);

    myTable.SetBet(  myTable.AddPlayer("TestDummyOpponent3",87.5, testDummy+2) , oppBet );


    ExactCallD myExpectedCall(0, &myTable, &o);
    //ZeroCallD myExpectedCall(0, &myTable, &flat);




///TODO compare with revision 46
	GainModel g(myChoiceStatResult,&myExpectedCall);
        #ifdef DEBUG_GAIN
            std::ofstream excel("functionlog.csv");
            if( !excel.is_open() ) std::cerr << "\n!functionlog.cvs file access denied" << std::endl;
            g.breakdown((t_chipCount-myTable.PeekCallBet())/myTable.GetChipDenom(),excel,myTable.PeekCallBet(),t_chipCount);
            //myExpectedCall.breakdown(0.005,excel);

            excel.close();
        #endif

        float64 turningPoint = g.FindBestBet();
        #ifdef DEBUG_GAIN
            cout << endl << endl;

            cout << "Goal bet " << turningPoint << endl;
            cout << "Fold bet " << g.FindZero(turningPoint,t_chipCount) << endl;
            cout << myTable.PeekCallBet() << " = " << g.f(myTable.PeekCallBet()) << endl;
        #endif

	GainModelReverse gr(myChoiceStatResult,&myExpectedCall);
        #ifdef DEBUG_GAIN
            excel.open ("functionlog.reverse.csv");
            if( !excel.is_open() ) std::cerr << "\n!functionlog.cvs file access denied" << std::endl;
            gr.breakdown((t_chipCount-myTable.PeekCallBet())/myTable.GetChipDenom(),excel,myTable.PeekCallBet(),t_chipCount);
            //myExpectedCall.breakdown(0.005,excel);

            excel.close();
        #endif

        turningPoint = gr.FindBestBet();
        #ifdef DEBUG_GAIN
            cout << endl << endl;

            cout << "R.Goal bet " << turningPoint << endl;
            cout << "R.Fold bet " << gr.FindZero(turningPoint,t_chipCount) << endl;
            cout << myTable.PeekCallBet() << " = " << gr.f(myTable.PeekCallBet()) << endl;
        #endif


	GainModelNoRisk gno(worstStatResult,&myExpectedCall);
	GainModelReverseNoRisk grno(worstStatResult,&myExpectedCall);
	SlidingPairFunction gp(&g,&gr,0.5,&myExpectedCall);
	SlidingPairFunction ap(&gno,&grno,0.5,&myExpectedCall);
	SlidingPairFunction gpp(&gp,&ap,0.5,&myExpectedCall);


        #ifdef DEBUG_GAIN
            excel.open("functionlog.norisk.csv");
            if( !excel.is_open() ) std::cerr << "\n!functionlog.safe.cvs file access denied" << std::endl;
            gno.breakdown((t_chipCount-myTable.PeekCallBet())/myTable.GetChipDenom(),excel,myTable.PeekCallBet(),t_chipCount);
            //g.breakdownE(40,excel);
            excel.close();
turningPoint = gno.FindBestBet();
            cout << "Algb target " << turningPoint << endl;
            cout << "Algb fold bet " << gno.FindZero(turningPoint,t_chipCount) << endl;
            cout << myTable.PeekCallBet() << " = " << gno.f(myTable.PeekCallBet()) << endl;


            excel.open("functionlog.new.csv");
            if( !excel.is_open() ) std::cerr << "\n!functionlog.safe.cvs file access denied" << std::endl;
            grno.breakdown((t_chipCount-myTable.PeekCallBet())/myTable.GetChipDenom(),excel,myTable.PeekCallBet(),t_chipCount);
            //g.breakdownE(40,excel);
            excel.close();
turningPoint = grno.FindBestBet();
            cout << "Algb.rev target " << turningPoint << endl;
            cout << "Algb.rev fold bet " << grno.FindZero(turningPoint,t_chipCount) << endl;
            cout << myTable.PeekCallBet() << flush;
            cout << " = " << flush;
            cout << grno.f(myTable.PeekCallBet());
            cout << endl;

            cout << "GP target " << flush;
            excel.open("functionlog.GP.csv");

            if( !excel.is_open() ) std::cerr << "\n!functionlog.safe.cvs file access denied" << std::endl;
            gp.breakdown((t_chipCount-myTable.PeekCallBet())/myTable.GetChipDenom(),excel,myTable.PeekCallBet(),t_chipCount);
            //g.breakdownE(40,excel);
            excel.close();
turningPoint = gp.FindBestBet();
            cout << turningPoint << endl;
            cout << "GP fold bet " << gp.FindZero(turningPoint,t_chipCount) << endl;
            cout << myTable.PeekCallBet() << " = " << gp.f(myTable.PeekCallBet()) << endl;


            excel.open("functionlog.AP.csv");
            if( !excel.is_open() ) std::cerr << "\n!functionlog.safe.cvs file access denied" << std::endl;
            ap.breakdown((t_chipCount-myTable.PeekCallBet())/myTable.GetChipDenom(),excel,myTable.PeekCallBet(),t_chipCount);
            //g.breakdownE(40,excel);
            excel.close();
turningPoint = ap.FindBestBet();
            cout << "AP target " << turningPoint << endl;
            cout << "AP fold bet " << ap.FindZero(turningPoint,t_chipCount) << endl;
            cout << myTable.PeekCallBet() << " = " << ap.f(myTable.PeekCallBet()) << endl;


            excel.open("functionlog.safe.csv");
            if( !excel.is_open() ) std::cerr << "\n!functionlog.safe.cvs file access denied" << std::endl;
            gpp.breakdown((t_chipCount-myTable.PeekCallBet())/myTable.GetChipDenom(),excel,myTable.PeekCallBet(),t_chipCount);
            //g.breakdownE(40,excel);
            excel.close();
turningPoint = gpp.FindBestBet();
            cout << "Call target " << turningPoint << endl;
            cout << "Safe fold bet " << gpp.FindZero(turningPoint,t_chipCount) << endl;
            cout << myTable.PeekCallBet() << " = " << gpp.f(myTable.PeekCallBet()) << endl;
        #endif

    cout << endl;
}


void testPlay()
{
	BlindStructure b(.5,1);
	GeomPlayerBlinds bg(b.SmallBlind()/2,b.BigBlind()/2,2,2);
	HoldemArena myTable(&bg, true, true);
	//ThresholdStrategy stagStrat(0.5);
	UserConsoleStrategy consolePlay;
	ConsoleStrategy manPlay[3];
	MultiThresholdStrategy pushFold;
	MultiThresholdStrategy tightPushFold(1);
	//ConsoleStepStrategy watchPlay;
	PositionalStrategy smartConserveDefence(0);
	PositionalStrategy smartGambleDefence(1);
	PositionalStrategy smartConserveOffence(2);
	PositionalStrategy smartGambleOffence(3);

	//myTable.AddPlayer("Stag", &stagStrat);
	//myTable.AddPlayer("N1", manPlay);
	//myTable.AddPlayer("M2", &smartConserveDefence); /* riskymode = 0 */
	//myTable.AddPlayer("S2", &smartGambleOffence); /* riskymode = 3 */
	myTable.AddPlayer("G2", &smartGambleDefence); /* riskymode = 1 */
	//myTable.AddPlayer("V2", &smartConserveOffence); /* riskymode = 2 */
	//myTable.AddPlayer("N2", manPlay+1);
	//myTable.AddPlayer("X3", &pushFold);
	myTable.AddPlayer("A3", &tightPushFold);
	//myTable.AddPlayer("P1", &consolePlay);

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
	    //testPosition();
	    testPlay();
        //testFunction();
        //testNewCallStats();

	    //testC();
		//goCMD(2,"505");

	}
	//testPlay();

}

