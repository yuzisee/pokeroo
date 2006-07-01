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

#include "aiCache.h"
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
const int8 StatsManager::CACHEABLESTAGE = 0;
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

string StatsManager::dbFileName(const Hand& withCommunity, const Hand& onlyCommunity)
{
    if( "" == baseDataPath ) initPath();

    TriviaDeck o;
    o.OmitCards(withCommunity);
    o.DiffHand(onlyCommunity);
    o.sortSuits();

    return baseDataPath + o.NamePockets() + ".holdem";

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
    return !(dataf.bad());
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

        //StatResult *cachedAvg = reinterpret_cast<StatResult*>(cachebuf);
        dataf.read(cachebuf,cachebufSize);
        return !(dataf.bad());
        //*myAvg = *cachedAvg;

        //myAvg->genPCT();
    }
    return !(dataf.bad());
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
    return true;
}

void StatsManager::Query(StatResult* myAvg, DistrShape* dPCT, DistrShape* dWL, const CommunityPlus& withCommunity, const CommunityPlus& onlyCommunity, int8 n)
{
    string datafilename = "";
    if( CACHEABLESTAGE >= n )
    {
        datafilename = dbFileName(withCommunity, onlyCommunity);
        ifstream dataserial(datafilename.c_str());
        if( dataserial.is_open() )
        {
            if( UnserializeW( dataserial, myAvg, dPCT, dWL ) )
            {
                cout << "Reading from file" << endl;
                dataserial.close();
                return;
            }else
            {
                cout << "Error reading " << datafilename << endl;
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
            SerializeW( newdata, ds.myAvg, *(ds.myDistrPCT), *(ds.myDistrWL) );
        }
    }
    if( 0 != myAvg ) *myAvg = ds.myAvg;
    if( 0 != dPCT ) *dPCT = *(ds.myDistrPCT);
    if( 0 != dWL ) *dWL = *(ds.myDistrWL);

}

void StatsManager::Query(CallCumulation& q, const CommunityPlus& withCommunity, const CommunityPlus& onlyCommunity, int8 n)
{
    DealRemainder myStatBuilder;
    myStatBuilder.UndealAll();
    myStatBuilder.OmitCards(withCommunity);

    CallStats ds(withCommunity, onlyCommunity,n);
    myStatBuilder.AnalyzeComplete(&ds);
}


string TriviaDeck::NamePockets() const
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

const void TriviaDeck::DiffHand(const Hand& h)
{
    OrderedDeck::dealtHand[0] ^= OrderedDeck::dealtHand[0] & h.SeeCards(0);
	OrderedDeck::dealtHand[1] ^= OrderedDeck::dealtHand[1] & h.SeeCards(1);
	OrderedDeck::dealtHand[2] ^= OrderedDeck::dealtHand[2] & h.SeeCards(2);
	OrderedDeck::dealtHand[3] ^= OrderedDeck::dealtHand[3] & h.SeeCards(3);

}

