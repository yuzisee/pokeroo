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



#include "arena.h"

#include "stratCombined.h"


#define VERBOSE_DESTRUCTOR

//Number of core bot types (excluding aggregate bot types)
#define NUMBER_OF_BOTS_COMBINED 6

//Number of bot types including aggregate bot types
#define NUMBER_OF_BOTS_TOTAL 8

#ifdef DEBUGASSERT
#include <typeinfo>
#endif



void HoldemArena::UnserializeRoundStart(std::istream & fileLoadState)
{

int16 numericValue;



            fileLoadState >> numericValue;

	    playernumber_t playersToLoad = static_cast<int8>(numericValue);

            fileLoadState.ignore(1,'\n');


            for( int8 i=0;i<playersToLoad;++i )
            {
                float64 pMoney = HoldemUtil::ReadFloat64( fileLoadState );

                char botType;
                fileLoadState >> botType;
                fileLoadState.ignore(1,'_');

                std::string playerName;
                //http://www.cplusplus.com/reference/string/getline.html
                //If the delimiter is found, it is extracted and discarded, i.e. it is not stored and the next input operation will begin after it.
                std::getline(fileLoadState,playerName,'\n');

                if( botType == '~' )
                {
                    #ifdef DEBUASSERT
                    if( botType != pTypes[i] )
                    {
                        std::cerr << "Players added manually must match their saved player types!";
                        exit(1);
                    }
                    #endif
                    if( pMoney <= 0 ){
                        --livePlayers;
                        p[i]->myMoney = -1;
                    }else
                    {
                        p[i]->myMoney = pMoney;
                    }
                }
                else if( botType == ' ' )
                {
                    AddHumanOpponent(playerName.c_str(),pMoney);
                }
                else
                {
                    AddStrategyBot(playerName.c_str(),pMoney,botType);
                }
            }


            fileLoadState.ignore(1,'n');
            fileLoadState >> handnum ;




            fileLoadState.ignore(1,'@');
            fileLoadState >> numericValue;

	    curDealer = static_cast<int8>(numericValue);

            fileLoadState.ignore(1,'^');
            smallestChip = HoldemUtil::ReadFloat64( fileLoadState );





}


void HoldemArena::SerializeRoundStart(std::ostream & fileSaveState)
{
    fileSaveState << (int)nextNewPlayer << "\n";

    for( int8 i=0;i<nextNewPlayer;++i )
    {
        float64 pMoney =  p[i]->myMoney;
        if( pMoney < 0 ) pMoney = -1; //When Unserializing money less than 0, it is set to -1 already. To assist the AddPlayer function, we do that here.
        HoldemUtil::WriteFloat64( fileSaveState, pMoney );
        fileSaveState << pTypes[i];
        fileSaveState << "_" << p[i]->GetIdent() <<  endl;
    }

   	fileSaveState << "n" << handnum;

    fileSaveState << "@" << (int)curDealer;

    fileSaveState << "^" << flush;
    HoldemUtil::WriteFloat64( fileSaveState, smallestChip );



}

playernumber_t HoldemArena::AddHumanOpponent(const char* const id, float64 money)
{

    playernumber_t addedIndex = AddPlayer(id, money, 0);
    if( addedIndex >= 0 )
    {
        pTypes[addedIndex] = ' ';
    }
    return addedIndex;
}

playernumber_t HoldemArena::AddStrategyBot(const char* const id, float64 money, char botType)
{
    PlayerStrategy * botStrat = 0;
    MultiStrategy * combined = 0;

	//EARLY RETURN!!
	if( botType == 'R' ) return AddStrategyBot(	 id
												,money
												,randomBotType(id)
											   );

    switch( botType )
    {
    case 'A':
        botStrat = new ImproveGainStrategy(2);
        break;
    case 'C':
        botStrat = new DeterredGainStrategy();
        break;
    case 'D':
        botStrat = new DeterredGainStrategy(1);
        break;
    case 'N':
        botStrat = new ImproveGainStrategy(0);
        break;
    case 'S':
        botStrat = new DeterredGainStrategy(2);
        break;
    case 'T':
        botStrat = new ImproveGainStrategy(1);
        break;
    case 'G':
    case 'M':
    	PositionalStrategy *(children[NUMBER_OF_BOTS_COMBINED]);

        children[0] = new ImproveGainStrategy(0); //Norm
        children[1] = new ImproveGainStrategy(1); //Trap
        children[2] = new ImproveGainStrategy(2); //Action
        children[3] = new DeterredGainStrategy(); //Com
        children[4] = new DeterredGainStrategy(1);//Danger
        children[5] = new DeterredGainStrategy(2);//Space

        combined = new MultiStrategy(children,NUMBER_OF_BOTS_COMBINED);
        if( botType == 'M' ) combined->bGamble = 0;
        if( botType == 'G' ) combined->bGamble = 1;

        botStrat = combined; //This is an upcast. We will require a dynamic_cast later (refA)
        break;
    default:
        #ifdef DEBUGASSERT
            std::cerr << "Unknown bot type " << (char)(botType) << "being added!";
            exit(1);
        #endif
        botStrat = 0;
        break;
    }

    playernumber_t addedIndex = -1;
    if( botStrat )
    {
        addedIndex = AddPlayer(id, money, botStrat);
        if( addedIndex >= 0 )
        {
            pTypes[addedIndex] = botType;
        }
    }

    return addedIndex;
}


char HoldemArena::randomBotType(const char *key_null_termninated)
{
	char validBotTypes[NUMBER_OF_BOTS_TOTAL] = {'N','T','A','C','D','S','M','G'};
	playernumber_t validBotTypeFreq[NUMBER_OF_BOTS_TOTAL] = {0};
	vector<char> minFreqBotTypes;
    uint32 botTypeSeed = jenkins_one_at_a_time_hash(key_null_termninated);
	playernumber_t minFrequency = SEATS_AT_TABLE; //Start at SEATS_AT_TABLE and reduce down to minimum.

	//
	//Count existing bots by type.
	//

	for(uint8 botTypeIdx = 0;botTypeIdx<NUMBER_OF_BOTS_TOTAL;++botTypeIdx)
	{
		char tallyingBotType = validBotTypes[botTypeIdx];
		for(playernumber_t n=0;(n<SEATS_AT_TABLE) && p[n];++n)
		{
			char pBotType = GetPlayerBotType(n);
			if( pBotType == tallyingBotType )
			{
				++validBotTypeFreq[botTypeIdx];
				break;
			}
		}
		//We have counted all the bots that are of type tallyingBotType.
		//Is it the minimum so far?
		if( validBotTypeFreq[botTypeIdx] < minFrequency )
		{
			minFrequency = validBotTypeFreq[botTypeIdx];
			minFreqBotTypes.clear();
		}
		//Keep track of the list of minimum frequency botTypes.
		if( validBotTypeFreq[botTypeIdx] == minFrequency )
		{
		    minFreqBotTypes.push_back(validBotTypes[botTypeIdx]);
		}
	}

	//
	//Randomly select a bot type that has minFrequency frequency.
	//
	return minFreqBotTypes[ botTypeSeed % minFreqBotTypes.size() ];


    #if 0
	std::cout << "Type selection: ";
	for(size_t n=0;n<minFreqBotTypes.size();++n)
	{
	    std::cout << minFreqBotTypes[n];
	}
	std::cout << " | " << botTypeSeed << " % " << minFreqBotTypes.size() << " = " << minFreqBotTypes[ botTypeSeed % minFreqBotTypes.size() ] << std::endl;
    #endif //debugging only...
}

playernumber_t HoldemArena::ManuallyAddPlayer(const char* const id, const float64 money, PlayerStrategy* newStrat)
{
    playernumber_t addedIndex = AddPlayer(id, money, newStrat);
    if( addedIndex >= 0 )
    {
        pTypes[addedIndex] = '~';
    }
    return addedIndex;
}

playernumber_t HoldemArena::AddPlayer(const char* const id, const float64 money, PlayerStrategy* newStrat)
{

#ifdef DEBUGASSERT
	if( curIndex != -1 || nextNewPlayer >= SEATS_AT_TABLE)
	{
	    std::cerr << "Cannot add more players" << endl;
	    exit(1);
	}
#endif


    Player* newP = new Player(money, id,newStrat, INVALID);

	if( newStrat )
	{
	    newStrat->Link(newP, this, nextNewPlayer);
	}

	p[nextNewPlayer] = newP;  //p.push_back( newP );


    allChips += money;

	++nextNewPlayer;

	if( money > 0 ) ++livePlayers;

	return (nextNewPlayer-1);
}



/*

//Things we need to clean up here:
//  +  The player has an optional strat, and if that strat is a MultiStrategy, it has a list of children strats
// (1) We need to delete the strat if it exists
// (2) For each child strat, we need to delete it
// (3) We need to delete the children strat list if it exists (THE HISTORYSTRATEGY DOES THIS ALREADY)
//
//Note: If a player is not a bot, it won't need any of (1) or (2) or (3).
//      If a player is a bot but not a MultiStrategy, it won't have (2)
static void DeletePlayer(struct holdem_player player_to_delete)
{
	void ** children = player_to_delete.pstrat_children; //...2
	void * strat = player_to_delete.pstrat_ptr; //...1

	if( strat ) //if the player is a bot,
	{
		PlayerStrategy * pStrat = reinterpret_cast<PlayerStrategy *>(strat); //...1
		if( !children )
		{
			delete pStrat; //(1) deleted
		}else
		{
			PositionalStrategy ** mChildren = reinterpret_cast<PositionalStrategy **>(children); //...2

			MultiStrategy * mStrat = dynamic_cast<MultiStrategy *>(pStrat); //This is the downcast for (refA).
																			//We're casting further down the inheritance tree toward children/subclasses.
																			//Actually, as long as the destructors are virtual, we don't need to downcast just to delete.
			delete mStrat; //(1) deleted

			for(playernumber_t n=0;n<NUMBER_OF_BOTS_COMBINED;++n)
			{
				delete mChildren[n]; //(2) deleted
			}

			delete [] mChildren; //(3) deleted
		}
	}

}

*/

void HoldemArena::FreePlayer(Player* playerToDelete, char botType)
{
    PlayerStrategy * pStrat = playerToDelete->myStrat;

    if( botType )
    {
#ifdef VERBOSE_DESTRUCTOR
        std::cerr << "Player was autogenerated." << endl;
#endif // VERBOSE_DESTRUCTOR
        if( pStrat )
        {
            if( botType == 'G' || botType == 'M' )
            {

                MultiStrategy * mStrat = dynamic_cast<MultiStrategy *>(pStrat); //(refA)

                PositionalStrategy ** mStratChildren = mStrat->strats;

                playernumber_t substrats = mStrat->stratcount;
                for( playernumber_t n=0;n<substrats;++n )
                {
#ifdef VERBOSE_DESTRUCTOR
                    std::cerr << "Deleting child #" << (int)n << "@" << mStratChildren[n] << endl;
#endif // VERBOSE_DESTRUCTOR
                    delete mStratChildren[n];
                }
            }
		#ifdef DEBUGASSERT
	    else if( typeid( pStrat ) == typeid( MultiStrategy ) )
        {
       		std::cerr << "Multistrategy bots need to have their children freed!" << endl;
			exit(1);
		}
       	#endif

#ifdef VERBOSE_DESTRUCTOR
            std::cerr << "Deleting bot strategy @" << pStrat << " which will free mStratChildren" << endl;
#endif // VERBOSE_DESTRUCTOR
            delete pStrat;
        }
    }
#ifdef VERBOSE_DESTRUCTOR
    std::cerr << "Deleting player" << endl;
#endif // VERBOSE_DESTRUCTOR
    delete playerToDelete;
}


HoldemArena::~HoldemArena()
{
#ifdef VERBOSE_DESTRUCTOR
    std::cerr << "arenaManagement.cpp Destructor:" << endl;
#endif // VERBOSE_DESTRUCTOR

#ifdef GLOBAL_AICACHE_SPEEDUP
    if( communityBuffer != 0 )
    {
        delete communityBuffer;
        communityBuffer = 0;
    }
#endif

    for(int8 n=0;n<SEATS_AT_TABLE;++n)
    {
#ifdef VERBOSE_DESTRUCTOR
        std::cerr << "At seat #" << (int)n  << "..." << endl;
#endif // VERBOSE_DESTRUCTOR
        if( p[n] )
        {
        #ifdef DEBUGASSERT
            if( n >= nextNewPlayer )
            {
                std::cerr << "Mismatch between added players and existing players!";
                exit(1);
            }
        #endif
            FreePlayer(p[n], pTypes[n]);
        }
    }

	//delete [] p;
}
