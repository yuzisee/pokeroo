

#include "arena.h"

#include "stratCombined.h"

#define NUMBER_OF_BOTS_COMBINED 6

typedef PositionalStrategy * stratPtr;



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
                    p[i]->myMoney = pMoney;
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
        if( pMoney < 0 ) pMoney = 0;
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
    PositionalStrategy **children = 0;

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
        children = new stratPtr[NUMBER_OF_BOTS_COMBINED];

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
            std::cerr << "Unknown bot type being added!";
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
// (3) We need to delete the children strat list if it exists
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
        std::cerr << "Player was autogenerated." << endl;
        if( pStrat )
        {
            if( typeid( pStrat ) == typeid( MultiStrategy ) )
            {
                MultiStrategy * mStrat = dynamic_cast<MultiStrategy *>(pStrat); //(refA)

                PositionalStrategy ** mStratChildren = mStrat->strats;

                playernumber_t substrats = mStrat->stratcount;
                for( playernumber_t n=0;n<substrats;++n )
                {
                    std::cerr << "Deleting child #" << (int)n << endl;
                    delete mStratChildren[n];
                }

                std::cerr << "Deleting PositionalStrategy **" << endl;
                delete [] mStratChildren;
            }

            std::cerr << "Deleting bot strategy" << endl;
            delete pStrat;
        }
    }
    std::cerr << "Deleting player" << endl;
    delete playerToDelete;
}


HoldemArena::~HoldemArena()
{
    std::cerr << "Destructor:" << endl;

#ifdef GLOBAL_AICACHE_SPEEDUP
    if( communityBuffer != 0 )
    {
        delete communityBuffer;
        communityBuffer = 0;
    }
#endif

    for(int8 n=0;n<SEATS_AT_TABLE;++n)
    {
        std::cerr << "At seat #" << (int)n  << "..." << endl;
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
