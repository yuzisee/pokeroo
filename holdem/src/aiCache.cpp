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

//#define DEBUGQUERYW
//#define QUERYTOTALS

#include "aiCache.h"
#include "functionmodel.h"
#include <sstream>
#include <fstream>

using std::stringstream;
using std::ifstream;

/*

std::ifstream file("../test.dat");

http://www.parashift.com/c++-faq-lite/input-output.html#faq-15.5

http://www.parashift.com/c++-faq-lite/serialization.html#faq-36.6

*/



const char* StatsManager::CONFIGFILENAME = "holdemdb.ini";
const int8 StatsManager::CACHEABLESTAGE = DEF_CACHEABLE_MIN;
string StatsManager::baseDataPath = "";
#ifdef GLOBAL_AICACHE_SPEEDUP
CommunityPlus StatsManager::dsCommunity;
#endif

bool StatsManager::readPathFromIni()
{
    ifstream configfile(CONFIGFILENAME);

    if( configfile.is_open() )
	{
		getline (configfile,baseDataPath);
		configfile.close();
        return true;
    } else {
        return false;
    }
}

bool StatsManager::readPathFromEnv()
{
    char* pPath;
    pPath = getenv ("HOLDEMDB_PATH");
    if (pPath)
    {
        baseDataPath = string(pPath);
        return true;
    } else
    {
        return false;
    }
}

void StatsManager::initPath()
{
	if (readPathFromIni() || readPathFromEnv())
    {
		size_t endpos = baseDataPath.find_last_not_of("/\r\n \t");

		if( string::npos == endpos )
		{
	    		std::cerr << "Using default data path of ./ (holdemdb.ini contains an invalid first line)" << std::endl;
			baseDataPath = "./";
		}else
		{
			//Trim trailing whitespace (and trailing slash)
			baseDataPath = baseDataPath.substr( 0, endpos+1 );

			//Append slash if needed
			baseDataPath = baseDataPath + "/";
		}
	}else
	{
        std::cerr << "Using default data path of ./ (Use a holdemdb.ini or HOLDEMDB_PATH env to specify your own path)" << std::endl;
        baseDataPath = "./";
	}
}

string StatsManager::dbFileName(const Hand& withCommunity, const Hand& onlyCommunity, const string label)
{
    if( "" == baseDataPath ) initPath();

    NamedTriviaDeck o;
    o.OmitCards(withCommunity);
    o.DiffHand(onlyCommunity);
    o.sortSuits();

    return baseDataPath + o.NamePockets() + ".holdem" + label;

}

bool StatsManager::unserializeDistrShape(ifstream& dataf, DistrShape* d)
{
    long cachebufSize= sizeof(DistrShape);
    if( 0 == d )
    {
        dataf.seekg(cachebufSize,std::ios::cur);
    }else
    {
        char *cachebuf = reinterpret_cast<char*>(d);
        dataf.read(cachebuf,cachebufSize);
    }
    return !(dataf.bad() || dataf.eof());
}
bool StatsManager::unserializeStatResult(ifstream& dataf, StatResult* d)
{
    long cachebufSize= sizeof(StatResult);
    if( 0 == d )
    {
        dataf.seekg(cachebufSize,std::ios::cur);
    }else
    {
        char *cachebuf = reinterpret_cast<char*>(d);
        dataf.read(cachebuf,cachebufSize);
        //myAvg->genPCT();
    }
    return !(dataf.bad() || dataf.eof());
}

void StatsManager::serializeDistrShape(ofstream& dataf, const DistrShape& d)
{
    const char *cachebuf = reinterpret_cast<const char*>(&d);
	std::streamsize cachebufSize = sizeof(DistrShape);
    dataf.write(cachebuf,cachebufSize);
}

void StatsManager::serializeStatResult(ofstream& dataf, const StatResult& d)
{
    const char *cachebuf = reinterpret_cast<const char*>(&d);
    std::streamsize cachebufSize = sizeof(StatResult);
    dataf.write(cachebuf,cachebufSize);
}

void StatsManager::SerializeW( ofstream& dataf, const DistrShape& dPCT )
{
    serializeDistrShape( dataf, dPCT );
}

bool StatsManager::UnserializeW( ifstream& dataf, DistrShape* dPCT )
{
    if(! (unserializeDistrShape( dataf, dPCT )) ) return false;

    ///It better be the end of the file now....
    char temp[1];
    dataf.read(temp,1);
    return dataf.eof();
}

void StatsManager::SerializeC( ofstream& dataf, const CallCumulation& q )
{
    cachesize_t vcount = static_cast<cachesize_t>(q.cumulation.size());
#ifdef DEBUGASSERT
	if (vcount != q.cumulation.size())
	{
		std::cerr << "Portability error in SerializeC" << endl;
		exit(1);
	}
#endif
    dataf.write(reinterpret_cast<const char*>(&vcount),sizeof(cachesize_t));
    //const vector<StatResult>& targetVector = q.cumulation;
    vector<StatResult>::const_iterator target;
    for(target = q.cumulation.begin();target != q.cumulation.end();++target)
    {
        serializeStatResult(dataf, *target);
    }
    /*
    for(size_t i=0;i<vcount;++i)
    {
        serializeStatResult(q.cumulation[i]);
    }
    */
}

bool StatsManager::UnserializeC( ifstream& dataf,  CallCumulation& q )
{
    cachesize_t vcount;
#ifdef DEBUGASSERT
	if (sizeof(cachesize_t) > sizeof(size_t))
	{
		std::cerr << "Cached size_t could be larger than current arch size_t" << endl;
		exit(1);
	}
#endif
    dataf.read(reinterpret_cast<char*>(&vcount),sizeof(cachesize_t));
    if( dataf.bad() || dataf.eof() ) return false;

    q.cumulation.clear();

    StatResult tempstat;
    for(size_t i=0;i<vcount;++i)
    {
        if(! (unserializeStatResult( dataf, &tempstat )) ) return false;
        q.cumulation.push_back(tempstat);
    }

    ///It better be the end of the file now....
    char temp[1];
    dataf.read(temp,1);
    return dataf.eof();
}

void StatsManager::Query(DistrShape* dPCT,
    const CommunityPlus& withCommunity, const CommunityPlus& onlyCommunity, int8 n)
{
    string datafilename = "";
    if( CACHEABLESTAGE >= n )
    {
        datafilename = dbFileName(withCommunity, onlyCommunity,"W");
        ifstream dataserial(datafilename.c_str(),std::ios::in | std::ios::binary);
        if( dataserial.is_open() )
        {
            if( UnserializeW( dataserial, dPCT ) )
            {
//                cout << "Reading from file" << endl;
                dataserial.close();
                return;
            }else
            {
                std::cerr << "Error reading " << datafilename << endl;
                dataserial.close();
            }
        }else
        {
            #ifdef DEBUGASSERT
            std::cerr << "Cacheable " << datafilename << " being regenereated" << endl;
            #endif
        }
    }

    WinStats ds(withCommunity, onlyCommunity,n);
    DealRemainder myStatBuilder;
    myStatBuilder.UndealAll();
    myStatBuilder.OmitSet(onlyCommunity, withCommunity);


#ifdef QUERYTOTALS
std::streamsize old_precision = std::cout.precision();
std::cout.precision(17);
std::cout << endl << "totalCount: " <<
#endif
    // === Generate everything! ===
    myStatBuilder.AnalyzeComplete(&ds);
#ifdef QUERYTOTALS
std::cout << endl;
std::cout.precision(old_precision);
#endif


    if( "" != datafilename )
    {
        ofstream newdata(datafilename.c_str(),std::ios::out | std::ios::binary | std::ios::trunc );
        if( newdata.is_open() )
        {
            SerializeW( newdata, ds.getDistr() );
        }
    }
     
    if( 0 != dPCT ) *dPCT = ds.getDistr();



        #ifdef DEBUGQUERYW


            std::cout << "Cards available to me" << endl;
            withCommunity.DisplayHand(std::cout);
            std::cout << endl;


            std::cout << "Cards in community" << endl;
            onlyCommunity.DisplayHand(std::cout);
            std::cout << endl;

            std::cout << endl;


            std::cout << "(Mean) " << ds.pctDistr().mean * 100 << "%"  << endl;
            std::cout << endl << "Adjusted improve? " << ds.pctDistr().improve * 100 << "%"  << endl;
            std::cout << "Worst:" << ds.pctDistr().worst *100 << "%" << endl;
            std::cout << "Standard Deviations:" << ds.pctDistr().stdDev*100 << "%" << endl;
            std::cout << "Average Absolute Fluctuation:" << ds.pctDistr().avgDev*100 << "%" << endl;
            std::cout << "Skew:" << ds.pctDistr().skew*100 << "%" << endl;
            std::cout << "Kurtosis:" << (ds.pctDistr().kurtosis)*100 << "%" << endl;

            std::cout << endl;
        #endif


}

void StatsManager::QueryOffense(CallCumulation& q, const CommunityPlus& withCommunity)
{
    PreflopCallStats pfcs(withCommunity, CommunityPlus::EMPTY_COMPLUS);
    pfcs.AutoPopulate();
    pfcs.Analyze();

    const CallCumulation &newC = *(pfcs.calc);
    q = newC;
}

#ifdef GLOBAL_AICACHE_SPEEDUP
void StatsManager::QueryOffense(CallCumulation& q, const CommunityPlus& withCommunity, const CommunityPlus& onlyCommunity, int8 n,  CommunityCallStats ** lastds)
#else
void StatsManager::QueryOffense(CallCumulation& q, const CommunityPlus& withCommunity, const CommunityPlus& onlyCommunity, int8 n)
#endif
{
    string datafilename = "";
    if( CACHEABLESTAGE >= n )
    {
        QueryOffense(q, withCommunity);
        return;
    }

#ifdef GLOBAL_AICACHE_SPEEDUP
    if( lastds == 0 )
    {
#endif
        CommunityCallStats ds(withCommunity, onlyCommunity,n);

        DealRemainder myStatBuilder;
        myStatBuilder.UndealAll();
        myStatBuilder.OmitSet(CommunityPlus::EMPTY_COMPLUS, onlyCommunity); ///Very smart, omit h2 NOT h1, because the opponent can think you have the cards you have


#ifdef QUERYTOTALS
std::streamsize old_precision = std::cout.precision();
std::cout.precision(17);
std::cout << endl << "totalCount: " <<
#endif
        myStatBuilder.AnalyzeComplete(&ds);
#ifdef QUERYTOTALS
std::cout << endl;
std::cout.precision(old_precision);
#endif



        const CallCumulation &newC = *(ds.calc);
        q = newC;
#ifdef GLOBAL_AICACHE_SPEEDUP
    }else
    {///There is a pointer to work with, lastds
        if( *lastds != 0 && onlyCommunity == dsCommunity )
        {
            CommunityCallStats *newds;
            newds = new CommunityCallStats(**lastds,withCommunity,onlyCommunity);
            newds->Analyze();
            const CallCumulation &newC = *(newds->calc);
            q = newC;
            delete newds;
        }else
        {///New community being queried
            if( *lastds != 0 ) delete *lastds;

            *lastds = new CommunityCallStats(withCommunity, onlyCommunity,n);


            DealRemainder myStatBuilder;
            myStatBuilder.UndealAll();
            myStatBuilder.OmitSet(CommunityPlus::EMPTY_COMPLUS, onlyCommunity); ///Very smart, omit h2 NOT h1, because the opponent can think you have the cards you have


#ifdef QUERYTOTALS
std::streamsize old_precision = std::cout.precision();
std::cout.precision(17);
std::cout << endl << "totalCount: " <<
#endif
            myStatBuilder.AnalyzeComplete(*lastds);
#ifdef QUERYTOTALS
std::cout << endl;
std::cout.precision(old_precision);
#endif

            const CallCumulation &newC = *((*lastds)->calc);
            q = newC;
        }

        dsCommunity.SetUnique(onlyCommunity);
    }
#endif


}

void StatsManager::QueryDefense(CallCumulation& q, const CommunityPlus& withCommunity, const CommunityPlus& onlyCommunity, int8 n)
{
    string datafilename = "";
    if( CACHEABLESTAGE >= n )
    {
        datafilename = dbFileName(withCommunity, onlyCommunity,"C");
        ifstream dataserial(datafilename.c_str(),std::ios::in | std::ios::binary);
        if( dataserial.is_open() )
        {

            if( UnserializeC( dataserial, q ) )
            {
//                cout << "Reading from file" << endl;
                dataserial.close();
                return;
            }else
            {
                std::cerr << "Error reading " << datafilename << endl;
                dataserial.close();
            }
        }else
	{
		#ifdef DEBUGASSERT
		std::cerr << "Cacheable " << datafilename << " being regenereated" << endl;
		#endif
	}
    }

    CallStats ds(withCommunity, onlyCommunity,n);

    DealRemainder myStatBuilder;
    myStatBuilder.UndealAll();
    myStatBuilder.OmitSet(onlyCommunity, withCommunity);

    #ifdef PROGRESSUPDATE
    const float handsTotal =
    #endif
    myStatBuilder.AnalyzeComplete(&ds);
    const CallCumulation &newC = *(ds.calc);

    #ifdef PROGRESSUPDATE
    std::cout << endl << "Efficiency: " << ds.handsComputed << " of " << handsTotal << endl;
    #endif

    if( "" != datafilename )
    {
        ofstream newdata(datafilename.c_str(),std::ios::out | std::ios::binary | std::ios::trunc );
        if( newdata.is_open() )
        {
            SerializeC( newdata, newC );
        }
    }
    q = newC;
}

void StatsManager::Query(CallCumulation* offense, CallCumulation* defense, const CommunityPlus& withCommunity, const CommunityPlus& onlyCommunity, int8 n)
{
    if( offense != 0 ) QueryOffense(*offense,withCommunity,onlyCommunity,n);
    if( defense != 0 ) QueryDefense(*defense,withCommunity,onlyCommunity,n);
}

int8 PreflopCallStats::largestDiscount(int8 * discount)
{
    return (  (discount[0]<discount[1]) ? discount[0] : discount[1]);
}

int8 PreflopCallStats::getDiscount(const char carda, const char cardb)
{
    return (
            ( carda == cardb ) ? -1 : 0
           );
}

int8 PreflopCallStats::oppSuitedOcc(int8 * discount, char mySuited )
{
    //dOcc = 4;
    if( mySuited == 'S' )
    {
       /* if( discount[0] + discount[1] < 0 )
        {///We couldn't be in the same suit
            return 3;
        }
        return 4;
        */
        return 4 + largestDiscount(discount);
    }//else
    {///Opp is suited. I cover separate suits. For each collision he can't have that suit
        return 4 + discount[0]+discount[1];
    }
}

int8 PreflopCallStats::popSet(const int8 carda, const int8 cardb)
{
    CommunityPlus oppTempStrength;

    NamedTriviaDeck handOpp;DeckLocation tempOpp;
    tempOpp.SetByIndex(carda);oppTempStrength.AddToHand(tempOpp);
    tempOpp.SetByIndex(cardb);oppTempStrength.AddToHand(tempOpp);
    handOpp.OmitCards(oppTempStrength);
    handOpp.sortSuits();

    NamedTriviaDeck myPockets;
    myPockets.OmitCards(myStrength);
    myPockets.DiffHand(oppStrength);
    myPockets.sortSuits();

    string oppPocketName = handOpp.NamePockets() ;
    string myPocketName = myPockets.NamePockets() ;


    ///Subtract "discount" the cards that YOU are holding
    int8 discount[2];
    discount[0] = getDiscount( myPocketName[0] , oppPocketName[0] );
    discount[1] = getDiscount( myPocketName[0] , oppPocketName[1] );
    discount[0] += getDiscount( myPocketName[1] , oppPocketName[0] );
    discount[1] += getDiscount( myPocketName[1] , oppPocketName[1] );

    bool oppPair = (oppPocketName[0] == oppPocketName[1]);

    int8 dOcc;


    if( oppPocketName[2] == 'S' )
    {
        dOcc = oppSuitedOcc(discount,myPocketName[2]);
    }else ///Opp is OFFSUIT
    {
        if( oppPair )
        {
            dOcc = 4 + largestDiscount(discount);
            dOcc = dOcc * (dOcc - 1) / 2;
        }else
        {
            dOcc = (4 + discount[0]) * (4 + discount[1]);
            dOcc -= oppSuitedOcc(discount,myPocketName[2]);
        }
    }

    //cout << "%" << oppPocketName << "\t" << (int)(dOcc) << "\t\t(" << myPocketName << ")" <<  endl;

    CommunityPlus emptyHand;
    
    // === Write the result into myWins[statGroup] ... ===
    StatResult * entryTarget = myWins+statGroup;
    DistrShape distrShape(DistrShape::newEmptyDistrShape());
    StatsManager::Query(&distrShape, oppTempStrength, emptyHand, 0);
    *entryTarget = distrShape.mean;
    entryTarget->repeated = dOcc;
    
    // === ... and then increment statGroup ===
    ++statGroup;

    return dOcc;
}


void PreflopCallStats::AutoPopulate()
{
    statGroup = 0; // Initialize statGroup here. Each call to popSet() will increment it.


    OrderedDeck myPockets;
    myPockets.OmitCards(myStrength);
        #ifdef SUPERPROGRESSUPDATE
            std::cout << "Analyzing...                    \r" << flush;
        #endif
    
    // for (carda : all 13 cards of one suit)
    for(int8 carda=0;carda<52;carda+=4)
    {
        int8 cardx = carda+1;
        // INVARIANT: (carda,cardx) is now the pocket pair

        popSet(carda,cardx);

        // for (cards : all cards with the same suit as `carda` and value less than `carda`)
        // ...  simultaneously, `cardx` is the offsuit equivalent of `cards`
        for( int8 cards=carda;cards>0;)
        {
            cards -= 4;
            cardx -= 4;

            popSet(carda,cardx); // (carda,cardx) is offsuit
            popSet(carda,cards); // (carda,cards) is suited
        }
    }
    statCount = statGroup;
    //myChancesEach =
        #ifdef SUPERPROGRESSUPDATE
            std::cout << "Deciding.....                    \r" << endl;
        #endif
}

void PreflopCallStats::initPC()
{

	//moreCards = 7;

//    int8 cardsAvail = realCardsAvailable(0);

    myChancesEach = 1;
}


string NamedTriviaDeck::NamePockets() const
{
    stringstream ss;
    string temp;

    uint32 x[2] = {OrderedDeck::dealtHand[firstSuit], OrderedDeck::dealtHand[nextSuit[firstSuit]]};
    int8 val;

    for(val=14;val>=2;--val)
    {
            if ((x[0] & HoldemConstants::CARD_ACEHIGH) != 0)
            {
                ss << HoldemUtil::VALKEY[val];
                x[0] ^= HoldemConstants::CARD_ACEHIGH; ///Eliminate the card we used, so as to quickly check for more cards
                break;
            }
            x[0] <<= 1;
            x[1] <<= 1;
    }

    if( x[0] == 0 )
    {///This suit is empty aside from the card we found. The hand is offsuit.
        for(;val>=2;--val)
        {
                if ((x[1] & HoldemConstants::CARD_ACEHIGH) != 0)
                {
                    ss << HoldemUtil::VALKEY[val];
                    ss << "x";
                    return ss.str();
                }
                x[1] <<= 1;
        }
    }else
    {///This suit has more cards. The hand is suited.
        while(val>=2)
        {
            --val;
            x[0] <<= 1;
            if ((x[0] & HoldemConstants::CARD_ACEHIGH) != 0)
            {
                ss << HoldemUtil::VALKEY[val];
                ss << "S";
                return ss.str();
            }

        }
    }
#ifdef DEBUGASSERT
	std::cerr << "Invalid NamePockets()";
	exit(1);
#endif
    ss << "?";
    return ss.str();
}


