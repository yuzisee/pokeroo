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


#include "stratCombined.h"
#include "stratPosition.h"
//#include "stratTournament.h"
#include "stratManual.h"
#include "stratThreshold.h"
#include "arena.h"
#include "aiCache.h"
#include "functionmodel.h"
#include "aiInformation.h"
#include <algorithm>


#define AUTOEXTRATOKEN "restore.txt"

#define REQUEST_USER_BLINDSIZE

#ifndef WINRELEASE
	#ifndef NO_LOG_FILES
        ///Toggle the define below depending on debugging
		#define REGULARINTOLOG
	#endif
#endif
#define DEBUGSITUATION
//#define SUPERINTOLOG



using std::cout;
using std::endl;
using std::flush;

#define LARGESTPROCNUM 675

//FIRST_DEAL = 6 expects 46
//FIRST_DEAL = 5 expects 1081
//FIRST_DEAL = 4 expects 17296
//Linux DEBUG_SEED = 4 and FIRST_DEAL = 4 yields 17296 (deals 18472)

const int FIRST_DEAL = 5;

char * myPlayerName = 0;

void genW(CommunityPlus& h1, CommunityPlus& h2)
{

#ifdef DEBUGSITUATION
	cout << "Cards available to me" << endl;
	h1.DisplayHand(cout);
	cout << endl;
#endif

#ifdef DEBUGSITUATION
	cout << "Cards in community" << endl;
	h2.DisplayHand(cout);
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

cout.precision(10);

    cout << endl << "AVG "  << myWins.loss << " l + "
            << myWins.splits << " s + " << myWins.wins << " w = " <<
            myWins.loss+myWins.splits+myWins.wins
            << "\tx;"<< myWins.repeated   <<endl;

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

#ifdef DEBUGSITUATION
	cout << "Cards available to me" << endl;
	h1.DisplayHand(cout);
	cout << endl;
#endif

#ifdef DEBUGSITUATION
	cout << "Cards in community" << endl;
	h2.DisplayHand(cout);
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
    //StatsManager::QueryOffense( calc,h1, h2,0);
    StatsManager::QueryDefense( calc,h1, h2,0);


    cout << endl << "=============Reduced=============" << endl;
	cout.precision(16);
	size_t vectorLast = calc.cumulation.size();
	for(size_t i=0;i<vectorLast;i++)
	{
		cout << endl << "{" << i << "}" << calc.cumulation[i].loss << " l +\t"
				<< calc.cumulation[i].splits << " s +\t" << calc.cumulation[i].wins << " w =\t" <<
				calc.cumulation[i].pct
				<< " pct\tx;"<< calc.cumulation[i].repeated <<flush;
	}

/*
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

#ifdef DEBUG_TWOPAIR
void printPairStuff(const CommunityPlus &hx, const char* strName)
{
cout << strName << endl;
hx.DisplayHand( cout );
cout << (int)(hx.bestPair) << " then " << (int)(hx.nextbestPair) << endl;
}


#endif


void genCMD(uint16 procnum)
{

        uint16 handnum = procnum % 338;///procnum is 0 .. 675 , handnum is 0 .. 337
	    procnum = procnum/338;///0 or 1

	    uint16 card1 = handnum / 26; ///0 to 12
	    uint16 card2 = handnum % 26; ///0 to 25


	    ///accomodate the suit-major ordering of HoldemUtil
	    card1 *= 4;
	    card2 *= 2;

        if( card1 != card2 )
        {
            cout << "Cache generate: " << handnum << " " << procnum << endl;

            DeckLocation hands[2];

            hands[0].Suit = 0;
            hands[1].Suit = HoldemUtil::CardSuit( card2 )/2;

            hands[0].Rank = HoldemUtil::CardRank( card1 )+1;
            hands[1].Rank = HoldemUtil::CardRank( card2 )+1;

            if( hands[0].Rank <= hands[1].Rank )
            {
                hands[0].Value = HoldemUtil::CARDORDER[hands[0].Rank];
                hands[1].Value = HoldemUtil::CARDORDER[hands[1].Rank];

                cout << (int)(hands[0].Suit) << "\t" << (int)(hands[0].Rank) << "\t" << hands[0].Value << endl;
                cout << (int)(hands[1].Suit) << "\t" << (int)(hands[1].Rank) << "\t" << hands[1].Value << endl;


                CommunityPlus h1;
                h1.AddToHand(hands[0]);
                h1.AddToHand(hands[1]);



                if( procnum == 0 )
                {
                    h1.DisplayHand(cout);
                    genC(h1);
                    h1.DisplayHand(cout);
                    return;
                }

                if( procnum == 1 )
                {
                    h1.DisplayHandBig(cout);
                    genW(h1);
                    h1.DisplayHandBig(cout);
                    return;
                }
            }
        }

}

void goCMD(char* str, int lastGen)
{
    int genNum = atoi(str);
    while(genNum <= lastGen)
    {
        genCMD(genNum);
        ++genNum;
    }
}



void testAnything()
{

    DeckLocation acecard;

/*


    CommunityPlus smokes, holysmokes;
    acecard.SetByIndex(HoldemUtil::ParseCard('2','s'));
    smokes.AddToHand(acecard);
    acecard.SetByIndex(HoldemUtil::ParseCard('3','s'));
    smokes.AddToHand(acecard);
    acecard.SetByIndex(HoldemUtil::ParseCard('4','s'));
    smokes.AddToHand(acecard);
    acecard.SetByIndex(HoldemUtil::ParseCard('3','h'));
    smokes.AddToHand(acecard);
    acecard.SetByIndex(HoldemUtil::ParseCard('3','c'));
    smokes.AddToHand(acecard);
    acecard.SetByIndex(HoldemUtil::ParseCard('2','h'));
    smokes.AddToHand(acecard);
    acecard.SetByIndex(HoldemUtil::ParseCard('2','c'));
    smokes.AddToHand(acecard);

    smokes.evaluateStrength();
    smokes.DisplayHandBig(std::cout);

    exit(1);
//2s 3s 4s
//2s 3s 4s : 3c3h
//2s 3s 4s 2h 2c : 3c3h

//  2s 2h 2c 3s 4s : 3c3h


*/
/*    genCMD(200); */

    CommunityPlus withCommunity;
    CommunityPlus onlyCommunity;
//Ks Kd
    acecard.SetByIndex(HoldemUtil::ParseCard('7','s'));
    withCommunity.AddToHand(acecard);
    acecard.SetByIndex(HoldemUtil::ParseCard('J','h'));
    withCommunity.AddToHand(acecard);

//const int num_in_community = 0;

const int num_in_community = 3;
    ////4h 6h Kc

    acecard.SetByIndex(HoldemUtil::ParseCard('2','d'));
    onlyCommunity.AddToHand(acecard);
    withCommunity.AddToHand(acecard);
    acecard.SetByIndex(HoldemUtil::ParseCard('6','c'));
    onlyCommunity.AddToHand(acecard);
    withCommunity.AddToHand(acecard);
    acecard.SetByIndex(HoldemUtil::ParseCard('9','h'));
    onlyCommunity.AddToHand(acecard);
    withCommunity.AddToHand(acecard);


    CallCumulationD calc;
    StatsManager::QueryOffense(calc,withCommunity,onlyCommunity,num_in_community);
/*
    StatsManager::QueryDefense( calc,withCommunity, onlyCommunity,num_in_community);


    cout << endl << "=============Reduced=============" << endl;
	cout.precision(10);
	size_t vectorLast = calc.cumulation.size();
	StatResult myWins;
	for(size_t i=0;i<vectorLast;i++)
	{
	    float64 nn = (calc.cumulation[i].repeated - (i ? calc.cumulation[i-1].repeated : 0));
	    myWins = myWins + (calc.cumulation[i] * nn);
		cout << endl << "{" << i << "}" << calc.cumulation[i].loss * nn * 2097572400.0 << " l +\t"
				<< calc.cumulation[i].splits * nn * 2097572400.0 << " s +\t" << calc.cumulation[i].wins * nn * 2097572400.0 << " w =\t" <<
				calc.cumulation[i].pct
				<< " pct\tx;"<< nn * 2097572400.0 <<flush;
	}
    myWins.repeated = 0;
*/
/*
    StatResult myWins;
    DistrShape myDistrPCT(0);
    StatsManager::Query( &myWins,&myDistrPCT,0,withCommunity, onlyCommunity,num_in_community);





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
*/
/*
cout << endl << "AVG "  << myWins.loss << " l + "
            << myWins.splits << " s + " << myWins.wins << " w = " <<
            myWins.loss+myWins.splits+myWins.wins
            << "\tx;"<< myWins.repeated   <<endl;
*/

    exit(1);

       	    //testNewCallStats();
	    //testDeal();


	    //testC();
}

std::string testPlay(char headsUp = 'G', std::ostream& gameLog = cout)
{
    #ifdef DEBUGSAVE_EXTRATOKEN
    char ExtraTokenNameBuffer[DEBUGSAVE_EXTRATOKEN] = "P";
#endif
    #ifdef AUTOEXTRATOKEN
    char ExtraTokenNameBuffer[32] = "P1";
    #endif

    bool bLoadGame = false;

    if( headsUp == 1 )
    {
        bLoadGame = true;
    }

	const float64 AUTO_CHIP_COUNT = 100.0;

    uint32 blindIncrFreq = 40;
    uint32 tokenRandomizer;
    const float64 startingMoney= 1500;
    if( headsUp == 'L' )
    {
        #ifdef AUTOEXTRATOKEN
        std::ifstream fRestoreName(AUTOEXTRATOKEN);
        fRestoreName.getline(ExtraTokenNameBuffer, 32, '\n');
        fRestoreName >> blindIncrFreq;
        fRestoreName >> tokenRandomizer;
        //By default, leading whitespace (carriage returns, tabs, spaces) is ignored by cin.
        //http://www.augustcouncil.com/~tgibson/tutorial/iotips.html
        fRestoreName.close();
        #endif

        bLoadGame = true;
        headsUp = 'P';
#if defined(DEBUGSAVE_EXTRATOKEN) || defined(AUTOEXTRATOKEN)
        myPlayerName = ExtraTokenNameBuffer;
#endif
    }

    float64 smallBlindChoice;
    if( headsUp == 'P' )
    {
        #ifdef REQUEST_USER_BLINDSIZE

            if( !bLoadGame )
            {
                std::cerr << "You will start with "<< startingMoney <<" chips.\nPlease enter the initial big blind:" << std::endl;
                std::cin >> smallBlindChoice;
                std::cin.sync();
                std::cin.clear();
                smallBlindChoice /= 2;

                std::cerr << endl << "Blinds will start at " << smallBlindChoice << "/" << smallBlindChoice*2 << ".\nPlease enter how many hands before blinds increase:" << std::endl;
                std::cin >> blindIncrFreq;
                std::cin.sync();
                std::cin.clear();

                std::ofstream storePlayerName(AUTOEXTRATOKEN,std::ios::app);
                storePlayerName << blindIncrFreq << endl;
                tokenRandomizer = ((uint32)(startingMoney/smallBlindChoice));
                storePlayerName << tokenRandomizer << endl;
                storePlayerName.close();
            }else
            {
                //If you try to load a game without the savegame file, what's the default?
                smallBlindChoice=startingMoney/tokenRandomizer;
            }
        #else
            smallBlindChoice=2;
        #endif


    }else
    {
        smallBlindChoice=AUTO_CHIP_COUNT/200;
    }
	BlindStructure b(smallBlindChoice,smallBlindChoice*2.0);
	StackPlayerBlinds bg(AUTO_CHIP_COUNT, 9, b.BigBlind() / startingMoney);
    SitAndGoBlinds sg(b.SmallBlind(),b.BigBlind(),blindIncrFreq);
		#ifdef REGULARINTOLOG
            std::ios::openmode gamelogMode = std::ios::trunc;
            if( bLoadGame ) gamelogMode = std::ios::app;
			std::ofstream gameOutput("gamelog.txt",gamelogMode);
			HoldemArena myTable(&bg, gameOutput,true, true);
		#else
	HoldemArena myTable(&sg, gameLog,true, true);
		#endif
	//ThresholdStrategy stagStrat(0.5);
	UserConsoleStrategy consolePlay;
	//ConsoleStrategy manPlay[3];
    MultiThresholdStrategy drainFold(3,3);
    MultiThresholdStrategy pushAll(4,4);
	MultiThresholdStrategy pushFold(0,2);
	MultiThresholdStrategy tightPushFold(1,0);
	//ConsoleStepStrategy watchPlay;
	CorePositionalStrategy RankGeom(0);
	CorePositionalStrategy MeanGeom(1);
	CorePositionalStrategy WorseAlgb(2);
	CorePositionalStrategy RankAlgb(3);
	CorePositionalStrategy MeanAlgb(4);
	CorePositionalStrategy PotCommittalRankGeom(5);
	CorePositionalStrategy PotCommittalMeanGeom(6);
	CorePositionalStrategy HybridGeom(7);
	CorePositionalStrategy HybridAlgb(8);
	CorePositionalStrategy RankGeomBluff(9);
	CorePositionalStrategy MeanGeomBluff(10);
	CorePositionalStrategy WorseAlgbBluff(11);
	CorePositionalStrategy HybridGeomBluff(14);


    DeterredGainStrategy StrikeFold(1);
    //Set 1
	DeterredGainStrategy FutureFoldA;
  	ImproveGainStrategy XFoldA(0);
  	ImproveGainStrategy ImproveA(1);
	ImproveGainStrategy ReallyImproveA(2);



    //TrendStrategies
    DeterredGainStrategy DangerT(1), DangerTR(1);
    DeterredGainStrategy ComT, ComTR;
    ImproveGainStrategy NormT(0), NormTR(0);
    ImproveGainStrategy TrapT(1), TrapTR(1);
    ImproveGainStrategy AceT(2), AceTR(2);
    CorePositionalStrategy SpaceT(10), SpaceTR(10);


    PositionalStrategy *(multiT[6]) = {&DangerT, &ComT, &NormT, &TrapT, &AceT , &SpaceT};
    PositionalStrategy *(multiTR[6]) = {&DangerTR, &ComTR, &NormTR, &TrapTR, &AceTR, &SpaceTR};

    MultiStrategy MultiT(multiT,6);
    MultiT.bGamble = 0;
    MultiStrategy MultiTR(multiTR,6);
    MultiTR.bGamble = 1;


    //TournamentStrategy asterisk;
    //TournamentStrategy gruff(1);

    if( headsUp == 'P' )
    {



        if( myPlayerName == 0 ){ myTable.AddHuman("P1", startingMoney, &consolePlay); }
        else{
            myTable.AddHuman(myPlayerName, startingMoney, &consolePlay);
#ifdef DEBUGSAVE_EXTRATOKEN
            myTable.EXTRATOKEN = myPlayerName;
#endif
        }
    }else
    {
        //myTable.AddPlayer("q4", &pushAll);
        myTable.AddBot("i4", AUTO_CHIP_COUNT, &drainFold);
        //myTable.AddPlayer("X3", &pushFold);
        //myTable.AddPlayer("A3", &tightPushFold);

    }

	const uint32 NUM_OPPONENTS = 8;
    const uint32 rand765 = 1 + (blindIncrFreq + tokenRandomizer)^(blindIncrFreq*tokenRandomizer);
    const uint32 rand8432 = 1 + labs(blindIncrFreq - tokenRandomizer)^(blindIncrFreq*tokenRandomizer);
    const uint32 rand8 = rand8432%8;
    const uint32 rand432 = rand8432/8;
    const uint32 randSeed = rand8 + (rand765%(7*6*5))*8 + (rand432%(4*3*2))*(8*7*6*5);
    uint8 i;
    int8 * opponentorder;

	switch(headsUp)
    {
        case 'P':
            //cout << randNum << "+" << randStep << "i" << endl;
            opponentorder = HoldemUtil::Permute(NUM_OPPONENTS,randSeed);
            for(i=0;i<NUM_OPPONENTS;++i)
            {
                //cout << i << endl;
                switch(opponentorder[i])
                {
                    case 0:
                        myTable.AddBot("TrapBotV", startingMoney, &ImproveA);
                        break;
                    case 1:
                        myTable.AddBot("ConservativeBotV", startingMoney, &FutureFoldA);
                        break;
                    case 2:
                        myTable.AddBot("NormalBotV",startingMoney, &XFoldA);
                        break;
                    case 3:
                        myTable.AddBot("SpaceBotV", startingMoney, &MeanGeomBluff);
                        break;
                    case 4:
                        myTable.AddBot("ActionBotV",startingMoney, &ReallyImproveA);
                        break;
                    case 5:
                        myTable.AddBot("DangerBotV",startingMoney, &StrikeFold);
                        break;
                    case 6:
                        myTable.AddBot("MultiBotV", startingMoney, &MultiT);
                        break;
                    case 7:
                        myTable.AddBot("GearBotV", startingMoney, &MultiTR);
                        break;
                }

            }//End of for(i...

            delete [] opponentorder;
                //myTable.AddPlayer("NormalBotIV", &MeanGeomBluff); /* riskymode = 10 */
                //myTable.AddPlayer("NormalBotIV", &RankGeomBluff); /* riskymode = 9 */
            break;
        case 'M':
            myTable.AddBot("M2", AUTO_CHIP_COUNT,  &RankGeom); /* riskymode = 0 */
            break;
        case 'G':
            myTable.AddBot("G2",AUTO_CHIP_COUNT, &MeanGeom); /* riskymode = 1 */
            break;
        default:
            //myTable.AddPlayer("RankGeom", &RankGeom); /* riskymode = 0 */
            //myTable.AddPlayer("MeanGeom", &MeanGeom); /* riskymode = 1 */
            //myTable.AddPlayer("WorseAlgb", &WorseAlgb); /* riskymode = 2 */
            //myTable.AddPlayer("RankAlgb", &RankAlgb); /* riskymode = 3 */
            //myTable.AddPlayer("MeanAlgb", &MeanAlgb); /* riskymode = 4 */
            //myTable.AddPlayer("RankGeomPC", &PotCommittalRankGeom); /* riskymode = 5 */
            //myTable.AddPlayer("MeanGeomPC", &PotCommittalMeanGeom); /* riskymode = 6 */
            //myTable.AddPlayer("HybridGeom", &HybridGeom); /* riskymode = 7 */
            //myTable.AddPlayer("HybridAlgb", &HybridAlgb); /* riskymode = 8 */
			//myTable.AddPlayer("RankGeomBluff", &RankGeomBluff); /* riskymode = 9 */
			//myTable.AddPlayer("MeanGeomBluff", &MeanGeomBluff); /* riskymode = 10 */
			//myTable.AddPlayer("WorseAlgbBluff", &WorseAlgbBluff); /* riskymode = 11 */
			//myTable.AddPlayer("HybridGeomBluff", &HybridGeomBluff); /* riskymode = 14 */

            //myTable.AddPlayer("TrapBotII", &DistrScaleP);
            //myTable.AddPlayer("ComBotII", &FutureFoldP);
		//myTable.AddPlayer("SpaceBotII", &AutoSetP);
		//myTable.AddPlayer("TrapIII", &DistrScaleA);

        myTable.AddBot("GearBotV", AUTO_CHIP_COUNT, &MultiTR);
        myTable.AddBot("MultiBotV", AUTO_CHIP_COUNT, &MultiT);

		myTable.AddBot("DangerV", AUTO_CHIP_COUNT, &StrikeFold);
        myTable.AddBot("ComV", AUTO_CHIP_COUNT, &FutureFoldA);
        myTable.AddBot("NormV", AUTO_CHIP_COUNT, &XFoldA);
        myTable.AddBot("TrapV", AUTO_CHIP_COUNT, &ImproveA);
		myTable.AddBot("AceV", AUTO_CHIP_COUNT, &ReallyImproveA);


        myTable.AddBot("SpaceV", AUTO_CHIP_COUNT, &MeanGeomBluff);


        //myTable.AddPlayer("SpaceIV", &AutoSetA);
/*
        myTable.AddPlayer("ComR", &FutureFoldA_R);
        myTable.AddPlayer("NormR", &XFoldA_R);
        myTable.AddPlayer("TrapR", &ImproveA_R);
		myTable.AddPlayer("AceR", &ReallyImproveA_R);

*/
            break;

    }

#ifdef DEBUGSAVEGAME

if( bLoadGame )
{
#ifdef DEBUGSAVE_EXTRATOKEN
    myTable.EXTRATOKEN = ExtraTokenNameBuffer;
#endif
std::istream *saveLoc = myTable.LoadState();
if( saveLoc != 0 ) consolePlay.myFifo = saveLoc;

MultiT.handNumber = myTable.handnum;
MultiTR.handNumber = myTable.handnum;

}
#endif




    Player* iWin = (myTable.PlayTable());

#ifdef REGULARINTOLOG
gameOutput.close();
#endif

    if( iWin == 0 )
    {
        return "[No winner]?";
    }else
    {
        return iWin->GetIdent();
    }
}


void superGame(char headsUp = 0)
{

    #ifdef SUPERINTOLOG
        std::ofstream gameOutput("gamelog.txt");
        std::string iWin = testPlay(headsUp, gameOutput);
        gameOutput.close();
    #else
        std::string iWin = testPlay(headsUp);
    #endif


    std::ofstream tourny("batchResults.txt", std::ios::trunc);
    tourny << iWin.c_str() << endl;
    tourny.close();
    for(;;)
    {
        #ifdef SUPERINTOLOG
        gameOutput.open("gamelog.txt");
        iWin = testPlay(headsUp, gameOutput);
        gameOutput.close();
        #else
        iWin = testPlay(headsUp);
        #endif
        //system("pause");
        tourny.open("batchResults.txt", std::ios::app);
        tourny << iWin.c_str() << endl;
        tourny.close();
    }
}

int main(int argc, char* argv[])
{

#ifdef WINRELEASE
	cout << "Final Table: 9 Players" << endl;

		//
	//testHands();


	if( argc == 2 ) //one option
	{
        myPlayerName = argv[1];
        #ifdef AUTOEXTRATOKEN
        std::ofstream storePlayerName(AUTOEXTRATOKEN);
        storePlayerName << argv[1] << endl;
        storePlayerName.close();
        #endif

        testPlay('P');
    }else if( argc == 1 ) //no options, only command by itself
    {


	    testPlay('L');
/*
#ifdef NO_LOG_FILES
	    superGame(0);
#else
   	    testPlay(0);
#endif
*/

    }else
#endif
    {

    int maxGo=LARGESTPROCNUM;
    int n=1;
    cout << "Parsing Command Line Options..." << flush;
    while(n<argc)
    {
        if( *argv[n] == '-' || *argv[n] == '/' )
        {
            switch( argv[n][1] )
            {
                case 'r':
                    #ifdef NO_LOG_FILES
                    superGame(0);
                    #else
                    cout << "testplay()" << flush;
					++n;
                    testPlay(atoi(argv[n]));
                    #endif
                    exit(0);
                    break;
                case 'x':
                    ++n;
                    maxGo = atoi(argv[n]);
                    break;
                case 'e':
                case 'E':
                case 'g':
                case 'G':
                    cout << "goCMD()" << flush;
                    ++n;
                    if( n == argc )
                    {
                        cout << "Please specify a number between 0 and " << LARGESTPROCNUM << " inclusive." << endl;
                        exit(1);
                    }
                    goCMD(argv[n],maxGo);
                    exit(0);
                    break;
            }
        }
        ++n;
    }
    cout << "Done!" << endl;


#ifndef WINRELEASE
testPlay(1);
exit(0);
//   testAnything();


#endif

	}

}

/*
#set tabstop=4 shiftwidth=4 incsearch nowrap
*/
