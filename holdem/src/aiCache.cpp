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

//#define DEBUGQUERYW

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

void StatsManager::initPath()
{
	ifstream configfile(CONFIGFILENAME);

	if( configfile.is_open() )
	{
		getline (configfile,baseDataPath);
		configfile.close();

                if( baseDataPath[baseDataPath.length()-1] != '/' )
                {
                    baseDataPath = baseDataPath + "/";
                }
	}else
	{
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
    size_t cachebufSize= sizeof(DistrShape);
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
    size_t cachebufSize= sizeof(StatResult);
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
    size_t cachebufSize = sizeof(DistrShape);
    dataf.write(cachebuf,cachebufSize);
}

void StatsManager::serializeStatResult(ofstream& dataf, const StatResult& d)
{
    const char *cachebuf = reinterpret_cast<const char*>(&d);
    size_t cachebufSize = sizeof(StatResult);
    dataf.write(cachebuf,cachebufSize);
}

void StatsManager::SerializeW( ofstream& dataf, const StatResult& myAvg, const DistrShape& dPCT, const DistrShape& dWL )
{
    serializeStatResult( dataf, myAvg );
    serializeDistrShape( dataf, dPCT );
    serializeDistrShape( dataf, dWL );
}

bool StatsManager::UnserializeW( ifstream& dataf, StatResult* myAvg, DistrShape* dPCT, DistrShape* dWL )
{

    if(! (unserializeStatResult( dataf, myAvg )) ) return false;
    if(! (unserializeDistrShape( dataf, dPCT )) ) return false;
    if(! (unserializeDistrShape( dataf, dWL )) ) return false;

    ///It better be the end of the file now....
    char temp[1];
    dataf.read(temp,1);
    return dataf.eof();
}

void StatsManager::SerializeC( ofstream& dataf, const CallCumulation& q )
{
    size_t vcount = q.cumulation.size();
    dataf.write(reinterpret_cast<const char*>(&vcount),sizeof(size_t));
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
    size_t vcount;
    dataf.read(reinterpret_cast<char*>(&vcount),sizeof(size_t));
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

void StatsManager::Query(StatResult* myAvg, DistrShape* dPCT, DistrShape* dWL,
    const CommunityPlus& withCommunity, const CommunityPlus& onlyCommunity, int8 n)
{
    string datafilename = "";
    if( CACHEABLESTAGE >= n )
    {
        datafilename = dbFileName(withCommunity, onlyCommunity,"W");
        ifstream dataserial(datafilename.c_str(),std::ios::in | std::ios::binary);
        if( dataserial.is_open() )
        {
            if( UnserializeW( dataserial, myAvg, dPCT, dWL ) )
            {
//                cout << "Reading from file" << endl;
                dataserial.close();
                return;
            }else
            {
                std::cerr << "Error reading " << datafilename << endl;
                dataserial.close();
            }
        }
    }

    DealRemainder myStatBuilder;
    myStatBuilder.UndealAll();
    myStatBuilder.OmitCards(withCommunity);

    WinStats ds(withCommunity, onlyCommunity,n);
    myStatBuilder.AnalyzeComplete(&ds);

    if( "" != datafilename )
    {
        ofstream newdata(datafilename.c_str(),std::ios::out | std::ios::binary | std::ios::trunc );
        if( newdata.is_open() )
        {
            SerializeW( newdata, ds.avgStat(), ds.pctDistr(), ds.wlDistr() );
        }
    }
    if( 0 != myAvg ) *myAvg = ds.avgStat();
    if( 0 != dPCT ) *dPCT = ds.pctDistr();
    if( 0 != dWL ) *dWL = ds.wlDistr();



        #ifdef DEBUGQUERYW


            cout << "Cards available to me" << endl;
            withCommunity.DisplayHand(cout);
            cout << endl;


            cout << "Cards in community" << endl;
            onlyCommunity.DisplayHand(cout);
            cout << endl;

            cout << endl;


            cout << "(Mean) " << ds.pctDistr().mean * 100 << "%"  << endl;
            cout << endl << "Adjusted improve? " << ds.pctDistr().improve * 100 << "%"  << endl;
            cout << "Worst:" << ds.pctDistr().worst *100 << "%" << endl;
            cout << "Standard Deviations:" << ds.pctDistr().stdDev*100 << "%" << endl;
            cout << "Average Absolute Fluctuation:" << ds.pctDistr().avgDev*100 << "%" << endl;
            cout << "Skew:" << ds.pctDistr().skew*100 << "%" << endl;
            cout << "Kurtosis:" << (ds.pctDistr().kurtosis)*100 << "%" << endl;

            cout << endl;
        #endif


}

void StatsManager::QueryOffense(CallCumulation& q, const CommunityPlus& withCommunity)
{
    CommunityPlus emptyHand;
    PreflopCallStats pfcs(withCommunity, emptyHand);
    pfcs.AutoPopulate();
    pfcs.Analyze();

    const CallCumulation &newC = *(pfcs.calc);
    q = newC;
}

void StatsManager::QueryOffense(CallCumulation& q, const CommunityPlus& withCommunity, const CommunityPlus& onlyCommunity, int8 n)
{
    string datafilename = "";
    if( CACHEABLESTAGE >= n )
    {
        QueryOffense(q, withCommunity);
        return;
    }

    DealRemainder myStatBuilder;
    myStatBuilder.UndealAll();
    myStatBuilder.OmitCards(onlyCommunity); ///Very smart, omit h2 NOT h1, because the opponent can think you have the cards you have

    CommunityCallStats ds(withCommunity, onlyCommunity,n);
    myStatBuilder.AnalyzeComplete(&ds);
    const CallCumulation &newC = *(ds.calc);


    q = newC;
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
        }
    }

    DealRemainder myStatBuilder;
    myStatBuilder.UndealAll();
    myStatBuilder.OmitCards(withCommunity);

    CallStats ds(withCommunity, onlyCommunity,n);
    myStatBuilder.AnalyzeComplete(&ds);
    const CallCumulation &newC = *(ds.calc);

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
    StatResult * entryTarget = myWins+statGroup;
    DistrShape pctShape(0);
    DistrShape wlShape(0);
    StatsManager::Query( 0, &pctShape, &wlShape, oppTempStrength, emptyHand, 0);
    *entryTarget = GainModel::ComposeBreakdown(pctShape.mean,wlShape.mean);
    entryTarget->repeated = dOcc;
    ++statGroup;

    return dOcc;
}

void PreflopCallStats::AutoPopulate()
{
    statGroup = 0;


    OrderedDeck myPockets;
    myPockets.OmitCards(myStrength);
        #ifdef SUPERPROGRESSUPDATE
            cout << "Analyzing...                    \r" << flush;
        #endif
    for(int8 carda=0;carda<52;carda+=4)
    {
        int8 cardx = carda+1;

        popSet(carda,cardx);

        for( int8 cards=carda;cards>0;)
        {
            cards -= 4;
            cardx -= 4;

            popSet(carda,cardx);
            popSet(carda,cards);
        }
    }
    statCount = statGroup;
    //myChancesEach =
        #ifdef SUPERPROGRESSUPDATE
            cout << "Deciding.....                    \r" << endl;
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
    ss << "?";
    return ss.str();
}


