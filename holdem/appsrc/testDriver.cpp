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


#include "../src/stratCombined.h"
#include "../src/stratPosition.h"
#include "../src/stratManual.h"
#include "../src/stratThreshold.h"
#include "../src/arena.h"
#include "../src/aiCache.h"
#include "../src/functionmodel.h"
#include "../src/aiInformation.h"

#include <ctime>
#include <algorithm>
#include <string.h>
//#include <direct.h>


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
#ifdef REPRODUCIBLE
		my.ResetDRseed();
#endif

	}
#if defined(GRAPHMONEY) && defined(DEBUGSAVEGAME)
    else
    {
	    my.LoadBeginInitialState();
    }
#endif


}




static void PlayGameInner(HoldemArena & my, SerializeRandomDeck * tableDealer)
{

	if( my.PlayRound_BeginHand() == -1 ) return;

	CommunityPlus myFlop;
	my.RequestCards(tableDealer,3,myFlop, "Please enter the flop (no whitespace): ");
    if( my.PlayRound_Flop(myFlop) == -1 ) return;


	DeckLocation myTurn = my.RequestCard(tableDealer);
    if( my.PlayRound_Turn(myFlop,myTurn) == -1 ) return;

	DeckLocation myRiver = my.RequestCard(tableDealer);
    int8 playerToReveal = my.PlayRound_River(myFlop,myTurn,myRiver);
    if( playerToReveal == -1 ) return;


	CommunityPlus finalCommunity;
	finalCommunity.SetUnique(myFlop);
	finalCommunity.AddToHand(myTurn);
	finalCommunity.AddToHand(myRiver);


	my.PlayShowdown(finalCommunity,playerToReveal);
}


static void SaveStateShuffleNextHand(HoldemArena & my,BlindStructure & blindController, SerializeRandomDeck * d, float64 randRem)
{


#ifndef DEBUGSAVEGAME
	if( d ) d->ShuffleDeck( randRem );


	return;
#endif

	std::ofstream newSaveState(DEBUGSAVEGAME);
		//First save the round state
		//Then shuffle the deck, and append it's state to the savegame file.
  	my.SerializeRoundStart(newSaveState);
    blindController.Serialize(newSaveState);
	if( d )
	{
		d->UndealAll();
		d->LoggedShuffle(newSaveState, randRem);
		newSaveState << endl;
	}

	newSaveState.close();

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

static Player* PlayGameLoop(HoldemArena & my,BlindStructure & blindController, BlindValues firstBlind, SerializeRandomDeck * tableDealer, bool bLoadedGame, ifstream & closeFile)
{


    struct BlindUpdate thisRoundBlinds;



    InitGameLoop(my, bLoadedGame); //handnum is now 1


    // Blinds and deck order are restored during a bLoadedGame.
    // When starting fresh, we initialize these objects here.
    if( !bLoadedGame )
	{
		//Shuffle the deck here with an arbitrary seed
		float64 randRem = time(0);
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

		my.BeginNewHands(thisRoundBlinds);


        my.DealAllHands(tableDealer);

#ifdef DEBUGASSERT
		if( my.GetDRseed() != 1 )
		{
			std::cerr << "randRem is out of control!" << endl;
			std::cerr << "We ResetDRseed at the end of this while loop, and it's never expected to change until PlayGameInner. What happenned?" << endl;
			exit(1);
		}
#endif
		my.ResetDRseed();

		PlayGameInner(my, tableDealer);
#ifdef DEBUG_SINGLE_HAND
		exit(0);
#endif
		my.RefreshPlayers(); ///New Hand (handnum is also incremented now)


		if( closeFile.is_open() ) closeFile.close();
#ifdef DEBUGSAVEGAME
    #ifdef RELOAD_LAST_HAND
	        if( NumberAtTable() > 1 )
    #endif
        	{
				SaveStateShuffleNextHand(my, blindController, tableDealer, my.GetDRseed() );
	            my.ResetDRseed();
			}
#endif

	}


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







static std::string testPlay(char headsUp = 'G', std::ostream& gameLog = cout)
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
		#ifdef REGULARINTOLOG
            std::ios::openmode gamelogMode = std::ios::trunc;
            if( bLoadGame ) gamelogMode = std::ios::app;
			std::ofstream gameOutput("gamelog.txt",gamelogMode);
			HoldemArena myTable(smallBlindChoice, gameOutput,true, true);
		#else
            HoldemArena myTable(smallBlindChoice, gameLog,true, true);
		#endif


	SerializeRandomDeck * tableDealer = 0;
	SerializeRandomDeck internalDealer;

	#ifndef EXTERNAL_DEALER
		internalDealer.ShuffleDeck(static_cast<float64>(   myTable.NumberAtTable()   ));
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
		myTable.UnserializeRoundStart(loadFile);
        SELECTED_BLIND_MODEL.UnSerialize( loadFile );
        if( tableDealer )  tableDealer->Unserialize( loadFile ); //Restore state of deck as well


        std::istream *saveLoc = &loadFile;
        if( saveLoc != 0 )
        {
            consolePlay.myFifo = saveLoc;
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
                opponentorder = Permute(NUM_OPPONENTS,randSeed);
                for(i=0;i<NUM_OPPONENTS;++i)
                {
                    //cout << i << endl;
                    switch(opponentorder[i])
                    {
                        case 0:
                            myTable.AddStrategyBot("TrapBotV", startingMoney, 'T');
                            break;
                        case 1:
                            myTable.AddStrategyBot("ConservativeBotV", startingMoney, 'C');
                            break;
                        case 2:
                            myTable.AddStrategyBot("NormalBotV",startingMoney, 'N');
                            break;
                        case 3:
                            myTable.AddStrategyBot("SpaceBotV", startingMoney, 'S');//&MeanGeomBluff);
                            break;
                        case 4:
                            myTable.AddStrategyBot("ActionBotV",startingMoney, 'A');
                            break;
                        case 5:
                            myTable.AddStrategyBot("DangerBotV",startingMoney, 'D');
                            break;
                        case 6:
                            myTable.AddStrategyBot("MultiBotV", startingMoney, 'M');
                            break;
                        case 7:
                            myTable.AddStrategyBot("GearBotV", startingMoney, 'G');
                            break;
                    }

                }//End of for(i...

                delete [] opponentorder;

                break;


            default:

            myTable.AddStrategyBot("GearBotV", AUTO_CHIP_COUNT, 'G');
            myTable.AddStrategyBot("MultiBotV", AUTO_CHIP_COUNT, 'M');

            myTable.AddStrategyBot("DangerV", AUTO_CHIP_COUNT, 'D');
            myTable.AddStrategyBot("ComV", AUTO_CHIP_COUNT, 'C');
            myTable.AddStrategyBot("NormV", AUTO_CHIP_COUNT, 'N');
            myTable.AddStrategyBot("TrapV", AUTO_CHIP_COUNT, 'T');
            myTable.AddStrategyBot("AceV", AUTO_CHIP_COUNT, 'A');


            myTable.AddStrategyBot("SpaceV", AUTO_CHIP_COUNT, 'S');//&MeanGeomBluff);


                break;

        }
    }
///===========================
///   Finish adding players
///===========================




    Player* iWin = PlayGameLoop(myTable,SELECTED_BLIND_MODEL, b,tableDealer, bLoadGame, loadFile);

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
testPlay(1); //Will autodetect bLoadGame
exit(0);


#endif

	}

}

/*
#set tabstop=4 shiftwidth=4 incsearch nowrap
*/