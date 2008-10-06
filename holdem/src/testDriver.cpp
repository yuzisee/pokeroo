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


char * myPlayerName = 0;






Player* PlayGameLoop(HoldemArena & my,SerializeRandomDeck * tableDealer)
{

	my.BeginInitialState();

	while(my.NumberAtTable() > 1)
	{

		my.BeginNewHands(tableDealer);
        my.DealAllHands(tableDealer);
		my.PlayGame(tableDealer);
#ifdef DEBUG_SINGLE_HAND
		exit(0);
#endif
		my.RefreshPlayers(); ///New Hand

	}

	return my.FinalizeReportWinner();

}









std::string testPlay(char headsUp = 'G', std::ostream& gameLog = cout)
{

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
#if defined(AUTOEXTRATOKEN)
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
//        smallBlindChoice=AUTO_CHIP_COUNT/200;
        smallBlindChoice=AUTO_CHIP_COUNT/30;
    }
	BlindStructure b(smallBlindChoice,smallBlindChoice*2.0);
	StackPlayerBlinds bg(AUTO_CHIP_COUNT, 9, b.BigBlind() / AUTO_CHIP_COUNT  / 2);
    SitAndGoBlinds sg(b.SmallBlind(),b.BigBlind(),blindIncrFreq);
		#ifdef REGULARINTOLOG
            std::ios::openmode gamelogMode = std::ios::trunc;
            if( bLoadGame ) gamelogMode = std::ios::app;
			std::ofstream gameOutput("gamelog.txt",gamelogMode);
			HoldemArena myTable(&sg, gameOutput,true, true);
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

	DeterredGainStrategy DeterredRank(2);



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
    DeterredGainStrategy SpaceT(2), SpaceTR(2); //CorePositionalStrategy SpaceT(10), SpaceTR(10);



    PositionalStrategy *(multiT[6]) = {&DangerT, &ComT, &NormT, &TrapT, &AceT , &SpaceT};
    PositionalStrategy *(multiTR[6]) = {&DangerTR, &ComTR, &NormTR, &TrapTR, &AceTR, &SpaceTR};

        std::ofstream legendOutput("legend.txt");
        legendOutput << "0\t Danger\r\n" ;
        legendOutput << "1\t Com\r\n" ;
        legendOutput << "2\t Norm\r\n" ;
        legendOutput << "3\t Trap\r\n" ;
        legendOutput << "4\t Ace\r\n" ;
        legendOutput << "5\t Space\r\n" ;
        legendOutput.close();

    MultiStrategy MultiT(multiT,6);
    MultiT.bGamble = 0;
    MultiStrategy MultiTR(multiTR,6);
    MultiTR.bGamble = 1;



    if( headsUp == 'P' )
    {



        if( myPlayerName == 0 )
		{
			myTable.AddPlayer("P1", startingMoney, &consolePlay);
		}else
		{
            myTable.AddPlayer(myPlayerName, startingMoney, &consolePlay);
        }
    }else
    {
        //myTable.AddPlayer("q4", &pushAll);
        myTable.AddPlayer("i4", AUTO_CHIP_COUNT, &drainFold);
        //myTable.AddPlayer("X3", &pushFold);
        //myTable.AddPlayer("A3", &tightPushFold);

    }

	const uint32 NUM_OPPONENTS = 8;
    const uint32 rand765 = 1 + ((blindIncrFreq + tokenRandomizer)^(blindIncrFreq*tokenRandomizer));
    const uint32 rand8432 = 1 + (labs(blindIncrFreq - tokenRandomizer)^(blindIncrFreq*tokenRandomizer));
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
                        myTable.AddPlayer("TrapBotV", startingMoney, &ImproveA);
                        break;
                    case 1:
                        myTable.AddPlayer("ConservativeBotV", startingMoney, &FutureFoldA);
                        break;
                    case 2:
                        myTable.AddPlayer("NormalBotV",startingMoney, &XFoldA);
                        break;
                    case 3:
                        myTable.AddPlayer("SpaceBotV", startingMoney, &DeterredRank);//&MeanGeomBluff);
                        break;
                    case 4:
                        myTable.AddPlayer("ActionBotV",startingMoney, &ReallyImproveA);
                        break;
                    case 5:
                        myTable.AddPlayer("DangerBotV",startingMoney, &StrikeFold);
                        break;
                    case 6:
                        myTable.AddPlayer("MultiBotV", startingMoney, &MultiT);
                        break;
                    case 7:
                        myTable.AddPlayer("GearBotV", startingMoney, &MultiTR);
                        break;
                }

            }//End of for(i...

            delete [] opponentorder;

            break;


        default:

        myTable.AddPlayer("GearBotV", AUTO_CHIP_COUNT, &MultiTR);
        myTable.AddPlayer("MultiBotV", AUTO_CHIP_COUNT, &MultiT);

		myTable.AddPlayer("DangerV", AUTO_CHIP_COUNT, &StrikeFold);
        myTable.AddPlayer("ComV", AUTO_CHIP_COUNT, &FutureFoldA);
        myTable.AddPlayer("NormV", AUTO_CHIP_COUNT, &XFoldA);
        myTable.AddPlayer("TrapV", AUTO_CHIP_COUNT, &ImproveA);
		myTable.AddPlayer("AceV", AUTO_CHIP_COUNT, &ReallyImproveA);


        myTable.AddPlayer("SpaceV", AUTO_CHIP_COUNT, &DeterredRank);//&MeanGeomBluff);


            break;

    }


	SerializeRandomDeck * tableDealer = 0;
	SerializeRandomDeck internalDealer;

	#ifndef EXTERNAL_DEALER
		internalDealer.ShuffleDeck(static_cast<float64>(   myTable.NumberAtTable()   ));
		tableDealer = &internalDealer;
	#endif




#ifdef DEBUGSAVEGAME

if( bLoadGame )
{
	std::istream *saveLoc = myTable.LoadState(tableDealer);
	if( saveLoc != 0 )
	{
		consolePlay.myFifo = saveLoc;
		MultiT.handNumber = myTable.handnum;
		MultiTR.handNumber = myTable.handnum;
	}

}
#endif




    Player* iWin = PlayGameLoop(myTable,tableDealer);

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
