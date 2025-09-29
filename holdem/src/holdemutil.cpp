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



// holdem2.0.cpp : Defines the entry point for the console application.
//


/*
>> is 'written as numeral' shift
little-endian, since b[0] is less significant than b[1]
FIND A WAY TO CONVERT A 7 CARD cardset or valueset to the optimal 5-card equivelant!
*/

/*
Find all the strengths avaliable for one community.
Then find your strength and simply compare the unsigned long.
*/

/*

Figure out effect of changing one single card.
Should we keep unsigned short cardNeeded?
How many unique hand strengths are there?
about 13^6?
You could list the strengths in order, what are their occurrances (given that the community is there)

*/

/*
Start with
Straight Flush (AND-SHIFT x; 5)
Quads (AND the cardsets)
BUILD Valueset
if not full house,
FLUSH (ADD-SHIFT x; 13)
STRAIGHT (OR the cardsets, then AND-SHIFT x; 5)
*/
//#include <fstream>
#include "holdem2.h"
#include <stdexcept>

using std::endl;

constexpr uint32 HoldemUtil::PRIMES[14] =
	{1,2,3,5,7,11,13,17,19,23,29,31,37,41};



constexpr char HoldemUtil::VALKEY[16] =
	{'?','?','2','3','4','5','6','7','8','9','T','J','Q','K','A', '?'};
constexpr char HoldemUtil::ALT_VALKEY[16] =
	{'?','?','2','3','4','5','6','7','8','9','t','j','q','k','a', '?'};
constexpr char HoldemUtil::SUITKEY[5] =
	{'s','h','c','d','?'};



constexpr uint32 HoldemUtil::CARDORDER[14] =
    {
        HoldemConstants::CARD_ACELOW,HoldemConstants::CARD_DEUCE,HoldemConstants::CARD_TREY
        ,HoldemConstants::CARD_FOUR,HoldemConstants::CARD_FIVE,HoldemConstants::CARD_SIX
        ,HoldemConstants::CARD_SEVEN,HoldemConstants::CARD_EIGHT,HoldemConstants::CARD_NINE
        ,HoldemConstants::CARD_TEN,HoldemConstants::CARD_JACK,HoldemConstants::CARD_QUEEN
        ,HoldemConstants::CARD_KING,HoldemConstants::CARD_ACEHIGH
    };

constexpr uint32 HoldemUtil::VALUEORDER[14] =
    {
        HoldemConstants::VALUE_ACELOW,HoldemConstants::VALUE_DEUCE,HoldemConstants::VALUE_TREY
        ,HoldemConstants::VALUE_FOUR,HoldemConstants::VALUE_FIVE,HoldemConstants::VALUE_SIX
        ,HoldemConstants::VALUE_SEVEN,HoldemConstants::VALUE_EIGHT,HoldemConstants::VALUE_NINE
        ,HoldemConstants::VALUE_TEN,HoldemConstants::VALUE_JACK,HoldemConstants::VALUE_QUEEN
        ,HoldemConstants::VALUE_KING,HoldemConstants::VALUE_ACEHIGH
    };

constexpr uint32 HoldemUtil::INCRORDER[14] =
    {
        HoldemConstants::INCR_ACELOW,HoldemConstants::INCR_DEUCE,HoldemConstants::INCR_TREY
        ,HoldemConstants::INCR_FOUR,HoldemConstants::INCR_FIVE,HoldemConstants::INCR_SIX
        ,HoldemConstants::INCR_SEVEN,HoldemConstants::INCR_EIGHT,HoldemConstants::INCR_NINE
        ,HoldemConstants::INCR_TEN,HoldemConstants::INCR_JACK,HoldemConstants::INCR_QUEEN
        ,HoldemConstants::INCR_KING,HoldemConstants::INCR_ACEHIGH
    };

const Hand Hand::EMPTY_HAND;

float64 HoldemUtil::ReadFloat64(std::istream & loadFile)
{
    float64 result;
    uint32 *pMoneyU = reinterpret_cast< uint32* >( &result );
    loadFile >> *pMoneyU;
    loadFile.ignore(1,'x');
    loadFile >> *(pMoneyU+1);
    loadFile.ignore(1,':');
    return result;
}

//We can use a union instead to clean up the warnings.
void HoldemUtil::WriteFloat64( std::ostream& saveFile, const float64 v )
{
    const uint32 *pMoneyU = reinterpret_cast< const uint32* >( &v );
    saveFile << *pMoneyU << "x" << *(pMoneyU+1) << ":" << flush;
}


///Convert single digit to character, where '-' is for 0
uint8 HoldemUtil::cleanz(const uint32 j)
{
	if( j == 0 )
	{
		return '-';
	}
	else
	{
		return (static_cast<unsigned char>(j)+48);
	}
}

template<typename T> T constexpr HoldemUtil::nchoosep_selftest(const int32 n, int32 p) {
			FASTPATH_NCHOOSEP_IMPL

	    std::cerr << "nchoosep( " << n << " , " << p << " )" << std::endl;
     #if defined(__cpp_exceptions) || defined(__EXCEPTIONS) || defined(_CPPUNWIND)
       throw std::runtime_error("I thought we only ever hit these nchoosep cases...");
     #else
       exit(1);
     #endif
	  }

template int64_t HoldemUtil::nchoosep_selftest<int64_t>(const int32, int32);
template int32 HoldemUtil::nchoosep_selftest<int32>(const int32, int32);
template float64 HoldemUtil::nchoosep_selftest<float64>(const int32, int32);

void HandPlus::DisplayHand(std::ostream& logFile) const
{

	uint32 temp[4];

    for(int8 i=0;i<4;++i)
    {
        temp[i] = cardset[i];
    }

    for(int8 val=2;val<=14;++val)
    {
        for(int8 suit=0;suit<4;++suit)
        {
            temp[suit] >>= 1;
            if ((temp[suit] & 1) == 1)
                logFile << HoldemUtil::VALKEY[val] << HoldemUtil::SUITKEY[suit]
						<< " " << flush;
        }
    }
}


void HandPlus::DisplayHandBig(std::ostream& logFile) const
{
	uint32 temp[4];
    uint32 tempv = valueset;

    for(int8 i=0;i<4;++i)
    {
        temp[i] = cardset[i];
    }
    //top row
    for(int8 suit=0;suit<4;++suit)
    {
        for(int8 val=2;val<=14;++val)
        {
            temp[suit] >>= 1;
            if ((temp[suit] & 1) == 1)
                logFile << HoldemUtil::VALKEY[val] << " " << flush;
        }
    }
	logFile << "\t" << flush;
    for(int8 val=14;val>=2;val--) logFile << HoldemUtil::VALKEY[val] << flush;
	logFile << endl;

    for(int8 i=0;i<4;++i)
    {
        temp[i] = cardset[i];
    }
    //second row
    for(int8 suit=0;suit<4;++suit)
    {
        for(int8 val=2;val<=14;++val)
        {
            temp[suit] >>= 1;
            if ((temp[suit] & 1) == 1)
                logFile << HoldemUtil::SUITKEY[suit] << " " << flush;
        }
    }

    logFile << "\t" << flush;


    for(int8 val=0;val<=12;++val)
    {
		logFile << HoldemUtil::cleanz((tempv & HoldemConstants::VALUE_ACEHIGH) >> 26) << flush;
     	tempv <<= 2;
    }

	logFile << endl;
}


uint32 HandPlus::getValueset() const
{
	return valueset;
}

void Hand::SetEmpty()
{
    cardset[0] = 0;
    cardset[1] = 0;
    cardset[2] = 0;
    cardset[3] = 0;
}

bool Hand::IsEmpty() const
{
    return (cardset[0] == 0)
    && (cardset[1] == 0)
    && (cardset[2] == 0)
    && (cardset[3] == 0);
}

void HandPlus::SetEmpty()
{
	Hand::SetEmpty();
	valueset = 0;
	cardset[0] = 0;
	cardset[1] = 0;
	cardset[2] = 0;
	cardset[3] = 0;
}

bool HandPlus::IsEmpty() const
{
    return (valueset == 0);
}

void HandPlus::SetUnique(const HandPlus& h)
{
	Hand::SetUnique(h);
	valueset = h.valueset;
}

void HandPlus::SetUnique(const Hand& h)
{
	Hand::SetUnique(h);
	populateValueset();
}

void HandPlus::AppendUnique(const Hand& h)
{
	Hand::AppendUnique(h);
	populateValueset();
}

void HandPlus::AppendUnique(const HandPlus& h)
{
	Hand::AppendUnique(h);
	valueset += h.valueset;
}

void HandPlus::RemoveFromHand(
	const int8 aSuit,const uint8 aIndex,const uint32 aCard)
{
	Hand::RemoveFromHand(aSuit,aIndex,aCard);
	valueset-=HoldemUtil::INCRORDER[aIndex];
}

///aIndex is NOT RANK
void HandPlus::AddToHand(
	const int8 aSuit,const uint8 aIndex,const uint32 aCard)
{
	Hand::AddToHand(aSuit,aIndex,aCard);
	valueset+=HoldemUtil::INCRORDER[aIndex];

}

///NO ERROR PREVENTION
void Hand::AppendUnique(const Hand& h)
{
	for(int8 i=0;i<4;++i)
	{
		*(cardset+i) |= h.cardset[i];;
	}
}

void Hand::ResetCardset(const uint32 * const cardArray)
{
  #ifdef HARDCORE_SPEEDUP
     memcpy(cardset, cardArray, sizeof(cardset));
  #else
    cardset[0] = cardArray[0];
	cardset[1] = cardArray[1];
	cardset[2] = cardArray[2];
	cardset[3] = cardArray[3];
	#endif
}

void Hand::SetUnique(const Hand& h)
{
    ResetCardset( h.cardset );
}

void HandPlus::populateValueset()
{
	valueset = 0;
    //The ACE_LOW is always zero in valueset.

	for(int8 i=13;i>=1;--i)
	{
		valueset += (cardset[0] & HoldemUtil::CARDORDER[i]);
		valueset += (cardset[1] & HoldemUtil::CARDORDER[i]);
		valueset += (cardset[2] & HoldemUtil::CARDORDER[i]);
		valueset += (cardset[3] & HoldemUtil::CARDORDER[i]);
		valueset <<= 1;
	}

}

Hand::~Hand()
{
}

const Hand& Hand::operator=(const Hand& h)
{
    SetUnique(h);
	return *this;
}

bool Hand::operator==(const Hand& h) const
{
	return	(cardset[0] == h.cardset[0]) &&
			(cardset[1] == h.cardset[1]) &&
			(cardset[2] == h.cardset[2]) &&
			(cardset[3] == h.cardset[3]) ;
}

/*const int Hand::matchCount[7] =
    {
        24,12,0,4,0,0,1
    };


const int Hand::Occurrences() const
{
int reoc=0;
if (cardset[0] == cardset[1]) ++reoc;
if (cardset[0] == cardset[2]) ++reoc;
if (cardset[0] == cardset[3]) ++reoc;
if (cardset[1] == cardset[2]) ++reoc;
if (cardset[1] == cardset[3]) ++reoc;
if (cardset[2] == cardset[3]) ++reoc;

    return matchCount[reoc];
}
*/
