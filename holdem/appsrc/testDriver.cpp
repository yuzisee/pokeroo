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


#include "../src/stratCombined.h"
#include "../src/stratPosition.h"
#include "../src/arena.h"
#include "../src/aiCache.h"
#include "../src/functionmodel.h"
#include "../src/aiInformation.h"
#include "stratThreshold.h"
#include "stratManual.h"

#include <ctime>
#include <algorithm>
#include <string.h>
#include <iomanip>
#include <sstream>

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



#include <stdio.h>  /* defines FILENAME_MAX */
#ifdef WINDOWS
#include <direct.h>
#define GetCwd _getcwd
#else
#include <unistd.h>
#define GetCwd getcwd
#endif


using std::cout;
using std::endl;
using std::flush;


char * myPlayerName = 0;


#ifdef GRAPHMONEY
    std::ofstream scoreboard;
#endif

#ifdef DEBUGHOLECARDS
    std::ofstream holecardsData;
#endif



static void InitGameLoop(HoldemArena & my, bool bLoadGame)
{

#ifdef DEBUGASSERT
	my.AssertInitialState();
#endif


#ifdef DEBUGSAVEGAME
	if( !bLoadGame )
#endif
	{
		my.BeginInitialState(); //handnum is set to 1 here

            #ifdef GRAPHMONEY

                scoreboard.open(GRAPHMONEY);
                scoreboard << "#Hand";
                for(int8 i=0;i<my.GetTotalPlayers();++i)
                {
                    scoreboard << "," << my.ViewPlayer(i)->GetIdent();
                }
                scoreboard << endl;
                scoreboard << "0";
                for(int8 i=0;i<my.GetTotalPlayers();++i)
                {
                    scoreboard << "," << my.ViewPlayer(i)->GetMoney();
                }
                scoreboard << endl;
            #endif // GRAPHMONEY

	}
#if defined(GRAPHMONEY) && defined(DEBUGSAVEGAME)
    else
    {

        #ifdef GRAPHMONEY
            scoreboard.open(GRAPHMONEY , std::ios::app);
        #endif
    }
#endif

		my.ResetDRseed();

        #ifdef DEBUGHOLECARDS
        holecardsData.open( DEBUGHOLECARDS, std::ios::app);
        #endif

}






static void SaveStateShuffleNextHand(HoldemArena & my,BlindStructure & blindController, SerializeRandomDeck * d, float64 randRem)
{

	uint32 shuffleSeed = RandomDeck::Float64ToUint32Seed(d->RandomSmallInteger(),randRem);


#ifdef DEBUGSAVEGAME
	std::ofstream newSaveState(DEBUGSAVEGAME);
		//First save the round state
		//Then shuffle the deck, and append it's state to the savegame file.
  	my.SerializeRoundStart(newSaveState);
    blindController.Serialize(newSaveState);
	if( d )
	{
		d->UndealAll();
		d->LoggedShuffle(newSaveState, shuffleSeed);
		newSaveState << endl;
	}

	newSaveState.close();
#else
	if( d ) d->ShuffleDeck( shuffleSeed );


	return;
#endif // DEBUGSAVEGAME, else

#if defined(DEBUGSAVEGAME_ALL) && defined(GRAPHMONEY)
            char handnumtxt
//[12] = "";            char namebase//
[23+12] = "./" DEBUGSAVEGAME_ALL "/" DEBUGSAVEGAME "-";

			HoldemArena::FileNumberString(my.handnum,handnumtxt + strlen(handnumtxt));
            handnumtxt[23+12-1] = '\0'; //just to be safe

    std::ofstream allSaveState( handnumtxt );
		//Rewrite into a second file
	my.SerializeRoundStart(allSaveState);
	blindController.Serialize(allSaveState);
	if( d )
	{
		d->LogDeckState( allSaveState );
		allSaveState << endl;
	}

    if( !allSaveState.is_open() ) std::cerr << handnumtxt << " not saved." << endl;

	allSaveState.close();
#endif



}


static struct BlindUpdate UninitializedBlinds()
{
    struct BlindUpdate b;
    b.bNew = false;

    b.handNumber = 0;
    b.timeSoFar = 0.0;

    return b;
}

static struct BlindUpdate myBlindState(HoldemArena & my)
{
    struct BlindUpdate b;
    b.bNew = false;

    b.timeSoFar = 0.0;

    b.handNumber = my.handnum;
    b.playersLeft = my.NumberAtTable();

    return b;
}

static Player* PlayGameLoop(std::ostream& gamelog, HoldemArena & my,BlindStructure & blindController, BlindValues firstBlind, SerializeRandomDeck * tableDealer, bool bLoadedGame, ifstream & closeFile)
{


    struct BlindUpdate thisRoundBlinds;



    InitGameLoop(my, bLoadedGame); //handnum is now 1


    // Blinds and deck order are restored during a bLoadedGame.
    // When starting fresh, we initialize these objects here.
    if( !bLoadedGame )
	{
		//Shuffle the deck here with an arbitrary seed
		time_t randRem = time(0);
		SaveStateShuffleNextHand(my, blindController, tableDealer, randRem);

		//Initialize the blinds with a starting value
		thisRoundBlinds = UninitializedBlinds();
		thisRoundBlinds.b = firstBlind;
        thisRoundBlinds.playersLeft = my.NumberAtTable();
	}

#ifdef DEBUGASSERT
	            my.ResetDRseed();
#endif

	while(my.NumberAtTable() > 1)
	{
	    struct BlindUpdate lastRoundBlinds = thisRoundBlinds;
        thisRoundBlinds = blindController.UpdateSituation( lastRoundBlinds , myBlindState(my) );

    #ifdef DEBUGHOLECARDS
        holecardsData <<
        "############ Hand " << my.handnum << " " <<
        "############" << endl;

    #endif

		my.BeginNewHands(gamelog, thisRoundBlinds.b, thisRoundBlinds.bNew);


        my.DealAllHands(tableDealer,
#ifdef DEBUGHOLECARDS
                        holecardsData
#else
                        std::cout
#endif // DEBUGHOLECARDS
                        );

#ifdef DEBUGASSERT
		if( my.GetDRseed() != 1 )
		{
			std::cerr << "randRem is out of control! " << my.GetDRseed() << endl;
			std::cerr << "We ResetDRseed at the end of this while loop, and it's never expected to change until PlayGameInner. What happenned?" << endl;
			exit(1);
		}
#endif
		my.ResetDRseed();

        HoldemArena::PlayGameInner(my, tableDealer, gamelog);
#ifdef DEBUG_SINGLE_HAND
		exit(0);
#endif



#ifdef GRAPHMONEY

    scoreboard << my.handnum << flush;

    for(int8 i=0;i<my.GetTotalPlayers();++i)
    {


            if( my.ViewPlayer(i)->GetMoney() < 0 )
            {
                scoreboard << ",0";
            }else
            {
                scoreboard << "," << flush;
                scoreboard.precision(10);
                scoreboard << my.ViewPlayer(i)->GetMoney() << flush;
            }


    }
    scoreboard << endl;
#endif // GRAPHMONEY


		my.RefreshPlayers(&gamelog); ///New Hand (handnum is also incremented now)


		if( closeFile.is_open() ) { closeFile.close(); }
#ifdef DEBUGSAVEGAME
    #ifdef RELOAD_LAST_HAND
	        if( NumberAtTable() > 1 )
    #endif
        	{
				SaveStateShuffleNextHand(my, blindController, tableDealer, my.GetDRseed() );
	            my.ResetDRseed();
			}
#else // #ifdef DEBUGSAVEGAME, else

        // TODO(from joseph_huang): DRSEED WHAT????
            SaveStateShuffleNextHand(my, blindController, tableDealer, my.GetDRseed() );
            my.ResetDRseed();

#endif // #ifdef DEBUGSAVEGAME, else
	}

#ifdef GRAPHMONEY
    scoreboard.close();
#endif


	return my.FinalizeReportWinner();

}


void PermuteExisting(int8 * array, uint8 count, uint32 seed)
{
    //For example, say count is 8
    for(uint8 i=0;i<=count-2;++i) //We'd only loop until sourceIndex, which is i, reaches the second last index
    {
        int8 swaptemp;
        const uint8& sourceIndex = i; //For example, this would be index 0
        const uint8 seedComponent = seed % (count-i); //For example, this would be (seed%8) which is from 0 to 7

        //Now we swap 0 with seed-value between 0 and 7.
        if( seedComponent != 0  )
        {
            const uint8 destIndex = i + seedComponent;

            swaptemp = array[sourceIndex];
            array[sourceIndex] = array[destIndex];
            array[destIndex] = swaptemp;
        }

        seed = seed / (count-i); //Divide out the 8
    }
}

int8 * Permute(uint8 count, uint32 seed)
{
    int8 * permutation = new int8[count];
    for( int8 i=count-1;i>=0;--i )
    {
        permutation[i] = i;
    }
    PermuteExisting(permutation,count,seed);
    return permutation;
}







static std::string testPlay(std::string gameId, char headsUp = 'G', std::ostream& gameLog = cout)
{

    //mkdir("saves");

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
                std::cin.clear();
                std::cin.sync();
                smallBlindChoice /= 2;

                std::cerr << endl << "Blinds will start at " << smallBlindChoice << "/" << smallBlindChoice*2 << ".\nPlease enter how many hands before blinds increase:" << std::endl;
                std::cin >> blindIncrFreq;
		std::cin.ignore(2,'\n'); //Don't leave lingering whitespace, sometimes you can't trust sync()
                std::cin.clear();
                std::cin.sync();

                std::ofstream storePlayerName(AUTOEXTRATOKEN,std::ios::app);
                storePlayerName << blindIncrFreq << endl;
                tokenRandomizer = ((uint32)(startingMoney/smallBlindChoice));
                storePlayerName << tokenRandomizer << endl;
                storePlayerName.close();
            }else
            {
                //If you try to load a game without the savegame file, this is the default?
                smallBlindChoice=startingMoney/tokenRandomizer;
            }
        #else
            smallBlindChoice=2;
        #endif


    }else
    {
//        smallBlindChoice=AUTO_CHIP_COUNT/200;
        smallBlindChoice=AUTO_CHIP_COUNT/35;
    }
    BlindValues b;
    b.SetSmallBigBlind(smallBlindChoice);

	StackPlayerBlinds bg(AUTO_CHIP_COUNT*9, smallBlindChoice / AUTO_CHIP_COUNT);
    SitAndGoBlinds sg(b.GetSmallBlind(),b.GetBigBlind(),blindIncrFreq);

HoldemArena myTable(smallBlindChoice,true, true);

	SerializeRandomDeck * tableDealer = 0;
	SerializeRandomDeck internalDealer;

	#ifndef EXTERNAL_DEALER
		internalDealer.ShuffleDeck(static_cast<uint32>(   myTable.NumberAtTable()   ));
		tableDealer = &internalDealer;
	#endif


///==========================
///   Begin adding players...
///==========================
    //ThresholdStrategy stagStrat(0.5);
    UserConsoleStrategy consolePlay;
    //ConsoleStrategy manPlay[3];

        MultiThresholdStrategy drainFold(3,3);
        MultiThresholdStrategy pushAll(4,4);
        MultiThresholdStrategy pushFold(0,2);
        MultiThresholdStrategy tightPushFold(1,0);




        if( headsUp == 'P' )
        {



            if( myPlayerName == 0 )
            {
                myTable.ManuallyAddPlayer("P1", startingMoney, &consolePlay);
            }else
            {
                myTable.ManuallyAddPlayer(myPlayerName, startingMoney, &consolePlay);
            }
        }else
        {
            //myTable.AddPlayer("q4", &pushAll);
            myTable.ManuallyAddPlayer("i4", AUTO_CHIP_COUNT, &drainFold);
            //myTable.AddPlayer("X3", &pushFold);
            //myTable.AddPlayer("A3", &tightPushFold);

        }

///======================
///   ...and load them
///======================

#ifdef WINRELEASE
    #define SELECTED_BLIND_MODEL bg
#else
    #define SELECTED_BLIND_MODEL sg
#endif

std::ifstream loadFile;
#ifdef DEBUGSAVEGAME

if( bLoadGame )
{
    //We want to load the game, so open the file and load state
    loadFile.open(DEBUGSAVEGAME);
    //
    if( loadFile.is_open() )
    {
		myTable.UnserializeRoundStart(loadFile, ".", gameId);
        SELECTED_BLIND_MODEL.UnSerialize( loadFile );
        if( tableDealer )  tableDealer->Unserialize( loadFile ); //Restore state of deck as well


        std::istream *saveLoc = &loadFile;
        if( saveLoc != 0 )
        {
            consolePlay.myFifos.SetFileStream(saveLoc);
        }

	}
	else if( headsUp == 1 ) {
	    bLoadGame = false; //Autodetect bLoadGame
	}
	else
	{
        std::cerr << "Load state requested, couldn't open file" << endl;
		exit(1);
    }




}
#endif

    if( !bLoadGame )
    {



        const uint32 NUM_OPPONENTS = 8;
        const uint32 rand765 = 1 + ((blindIncrFreq + tokenRandomizer)^(blindIncrFreq*tokenRandomizer));
        const uint64_t rand8432 = 1 + (labs(blindIncrFreq - tokenRandomizer)^(blindIncrFreq*tokenRandomizer));
        const uint32 rand8 = rand8432%8;
        const uint32 rand432 = static_cast<uint32>(rand8432/8);
        const uint32 randSeed = rand8 + (rand765%(7*6*5))*8 + (rand432%(4*3*2))*(8*7*6*5);
        uint8 i;
        int8 * opponentorder;

        switch(headsUp)
        {
            case 'P':
                //cout << randNum << "+" << randStep << "i" << endl;
                opponentorder = Permute(NUM_OPPONENTS,randSeed);
                for(i=0;i<NUM_OPPONENTS;++i)
                {
                    // The code here only runs if !bLoadGame, as you can see above.
                    //In this mode, we randomly assign order and randomly assign names.
                    //In all cases, when loading a game the bot types are unserialized (see HoldemArena::AddStrategyBot and HoldemArena::pTypes)
                    switch(opponentorder[i])
                    {
                        case 0:
                            myTable.AddStrategyBot(gameId, ".", "TrapBotV", startingMoney, 'R');
                            break;
                        case 1:
                            myTable.AddStrategyBot(gameId, ".", "ConservativeBotV", startingMoney, 'R');
                            break;
                        case 2:
                            myTable.AddStrategyBot(gameId, ".", "NormalBotV",startingMoney, 'R');
                            break;
                        case 3:
                            myTable.AddStrategyBot(gameId, ".", "SpaceBotV", startingMoney, 'R');
                            break;
                        case 4:
                            myTable.AddStrategyBot(gameId, ".", "ActionBotV",startingMoney, 'R');
                            break;
                        case 5:
                            myTable.AddStrategyBot(gameId, ".", "DangerBotV",startingMoney, 'R');
                            break;
                        case 6:
                            myTable.AddStrategyBot(gameId, ".", "MultiBotV", startingMoney, 'R');
                            break;
                        case 7:
                            myTable.AddStrategyBot(gameId, ".", "GearBotV", startingMoney, 'R');
                            break;
                    }

                }//End of for(i...

                delete [] opponentorder;

                break;

            case 0:
                // This is supergame mode.

                opponentorder = Permute(NUM_OPPONENTS,randSeed);
                for(i=0;i<NUM_OPPONENTS;++i)
                {
                    switch(opponentorder[i])
                    {
                        case 0:
                            myTable.AddStrategyBot(gameId, ".", "D0", AUTO_CHIP_COUNT, 'C');
                            break;
                        case 1:
                            myTable.AddStrategyBot(gameId, ".", "D2", AUTO_CHIP_COUNT, 'S');
                            break;
                        case 2:
                            myTable.AddStrategyBot(gameId, ".", "P0", AUTO_CHIP_COUNT, 'N');
                            break;
                        case 3:
                            myTable.AddStrategyBot(gameId, ".", "P2", AUTO_CHIP_COUNT, 'D');
                            break;
                        case 4:
                            myTable.AddStrategyBot(gameId, ".", "P3", AUTO_CHIP_COUNT, 'T');
                            break;
                        case 5:
                            myTable.AddStrategyBot(gameId, ".", "P4", AUTO_CHIP_COUNT, 'A');
                            break;
                        case 6:
                            myTable.AddStrategyBot(gameId, ".", "Gear", AUTO_CHIP_COUNT, 'G');
                            break;
                        case 7:
                            myTable.AddStrategyBot(gameId, ".", "Multi", AUTO_CHIP_COUNT, 'M');
                            break;
                    }

                }//End of for(i...

                delete [] opponentorder;


                break;


            default:

            myTable.AddStrategyBot(gameId, ".", "GearBotR", AUTO_CHIP_COUNT, 'R');
            myTable.AddStrategyBot(gameId, ".", "MultiBotR", AUTO_CHIP_COUNT, 'R');

            myTable.AddStrategyBot(gameId, ".", "DangerR", AUTO_CHIP_COUNT, 'R');
            myTable.AddStrategyBot(gameId, ".", "ComR", AUTO_CHIP_COUNT, 'R');
            myTable.AddStrategyBot(gameId, ".", "NormR", AUTO_CHIP_COUNT, 'R');
            myTable.AddStrategyBot(gameId, ".", "TrapR", AUTO_CHIP_COUNT, 'R');
            myTable.AddStrategyBot(gameId, ".", "AceR", AUTO_CHIP_COUNT, 'R');


            myTable.AddStrategyBot(gameId, ".", "SpaceR", AUTO_CHIP_COUNT, 'R');//&MeanGeomBluff);


                break;

        }
    }
///===========================
///   Finish adding players
///===========================



#ifdef REGULARINTOLOG
    std::ios::openmode gamelogMode = std::ios::trunc;
    if( bLoadGame ) gamelogMode = std::ios::app;
    std::ofstream gameOutput("gamelog.txt",gamelogMode);
    Player* iWin = PlayGameLoop(gameOutput, myTable,SELECTED_BLIND_MODEL, b,tableDealer, bLoadGame, loadFile);
#else
    Player* iWin = PlayGameLoop(gameLog, myTable,SELECTED_BLIND_MODEL, b,tableDealer, bLoadGame, loadFile);
#endif

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


static void superGame(char headsUp = 0)
{

    #ifdef SUPERINTOLOG
        std::ofstream gameOutput("gamelog.txt");
        std::string iWin = testPlay("0000000", headsUp, gameOutput);
        gameOutput.close();
    #else
        std::string iWin = testPlay("0000000", headsUp);
    #endif

    

    std::ofstream tourny("batchResults.txt", std::ios::trunc);
    tourny << iWin.c_str() << endl;
    tourny.close();

    size_t gameNum = 0;
    for(;;)
    {

        std::ostringstream gameIdStr; //output string stream
        gameIdStr << std::setfill('0') << std::setw(8) << gameNum;

        #ifdef SUPERINTOLOG
        gameOutput.open("gamelog.txt");
        iWin = testPlay(gameIdStr, headsUp, gameOutput);
        gameOutput.close();
        #else
        iWin = testPlay(gameIdStr.str(), headsUp);
        #endif
        //system("pause");
        tourny.open("batchResults.txt", std::ios::app);
        tourny << iWin.c_str() << endl;
        tourny.close();
    }
}


// For standard debugging and regression testing, undef WINRELEASE set env['HOLDEMDB_PATH'] and then run with no arguments and we'll fall through to testPlay(1) below
// Are you profiling? Remove WINRELEASE and run with -r
int main(int argc, char* argv[])
{
    
    char cCurrentPath[FILENAME_MAX];
    cCurrentPath[0] = ':';
    cCurrentPath[1] = '\0';
    
    GetCwd(cCurrentPath, sizeof(cCurrentPath));

    std::cerr << "Current working directory is " << cCurrentPath << std::endl;
    
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

        testPlay("game", 'P');
    }else if( argc == 1 ) //no options, only command by itself
    {


	    testPlay("game", 'L');


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
                    testPlay("game", atoi(argv[n]));
                    #endif
                    exit(0);
                    break;
                    
                case 'c': // For debugging, set two arbitrary hand values and compute the CallStats.
                    CallCumulationD foldcumu;
                    CommunityPlus onlyCommunity;
                    CommunityPlus withCommunity;
                    DeckLocation tempOpp;
                    tempOpp.SetByIndex(34);withCommunity.AddToHand(tempOpp);
                    tempOpp.SetByIndex(33);withCommunity.AddToHand(tempOpp);
                    withCommunity.DisplayHand(std::cout);
                    StatsManager::QueryDefense(foldcumu,withCommunity,onlyCommunity,0);
                    exit(0);
                    break;
            }
        }
        ++n;
    }
    cout << "Done!" << endl;


#ifndef WINRELEASE
testPlay("game", 1); //Will autodetect bLoadGame
exit(0);


#endif

	}

}

//  [0][1][2][3][4][5][6][7][8][9]
//0. 2S 2H 2C 2D 3S 3H 3C 3D 4S 4H
//1. 4C 4D 5S 5H 5C 5D 6S 6H 6C 6D
//2. 7S 7H 7C 7D 8S 8H 8C 8D 9S 9H
//3. 9C 9D TS TH TC TD JS JH JC JD
//4. QS QH QC QD KS KH KC KD AS AH
//5. AC AD

/*
#set tabstop=4 shiftwidth=4 incsearch nowrap
*/
