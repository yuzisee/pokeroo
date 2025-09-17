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

#ifndef HOLDEM_Porting
#define HOLDEM_Porting

#include <cstdint>

typedef std::uint8_t uint8;
typedef std::int8_t int8;

typedef std::uint16_t uint16;
typedef std::int16_t int16;

typedef std::uint32_t uint32;
typedef std::int32_t int32;
/*
ISO C++ doesn't support 'long long'
typedef unsigned long long int uint64;
typedef signed long long int int64;
*/

//typedef long double float128;
typedef double float64;
typedef float float32;


///========================================================

#define HANDNUM_T_STR_MAXCHARS 10
typedef uint32 handnum_t;
typedef int8 playernumber_t;
typedef uint32 cachesize_t; //Note: data cache is stored little-endian too...


#ifdef __cplusplus

// TODO(from joseph): Eventually once C++20 is the default everywhere (even on old systems) we can check `std::endian::native` instead
#define STD_ENDIAN_LITTLE() ([] { \
  const unsigned int n = 1; \
  return *reinterpret_cast<const char*>(&n) == 1; \
}())
// Check the first byte of the integer representation of 1. If it's 1, we know the first byte contains the least significant bit.

class HoldemConstants
{


    /*
        //anywhere there are no hanging/off-cards competitive values will be stored

        //possible strengths of FIVE (from seven) CARD hands
        //0		cannot occur without wild
        //1			high card to order
        //			[TIES: Remove lowest two remaining via VALUESET]
        //2-14		there are 13 pairs
        //			[TIES: Remove pair via VALUESET, treat as "high card to order"]
        //15-92		there are 13*3*2 = 78 twopairs
        //			[TIES: Remove pairs via VALUESET, treat as "high card to order"]
        //93-105	there are 13 trips
        //			[TIES: Remove trip via VALUESET, treat as "high card to order"]
        //106		straight
        //			[TIES: Leave ONLY the AND-AND-ANDed even as VALUESET = CARDSET]
        //(107?)	flush (reserved)
        //			[TIES: treat as "high card to order"]
        //108		boat 13*13 valueset choices
        //			[TIES: Leaving the pair AND-extracted VALUESET is enough]
        //109-121	there are 13 quads
        //			[TIES: Remove quad, remove lowest two remaining]
        //122		straight flush (similarily reserved)
        //			[TIES: treat as "straight"]
    */

public:
    static constexpr uint8 PAIR_ZERO=1, PAIRS_LOW=15, SET_ZERO=92;
    static constexpr uint8 SET_HIGH=105,STRAIGHT=106,FLUSH=107,BOAT=108;
    static constexpr uint8 QUAD_LOW=109,QUAD_HIGH=121,ROYAL=122;
    static constexpr int8 NO_SUIT=-1;
    static constexpr int8 SPADES=0,HEARTS=1,CLUBS=2,DIAMONDS=3;

    //short?
    static constexpr uint32
    CARD_ACELOW=1,CARD_DEUCE=2,CARD_TREY=4,CARD_FOUR=8;
    static constexpr uint32
    CARD_FIVE=16,CARD_SIX=32,CARD_SEVEN=64,CARD_EIGHT=128;
    static constexpr uint32
    CARD_NINE=256,CARD_TEN=512,CARD_JACK=1024;
    static constexpr uint32
    CARD_QUEEN=2048,CARD_KING=4096,CARD_ACEHIGH=8192,CARD_MISC=16384;

    static constexpr uint32
    VALUE_ACELOW=3,VALUE_DEUCE=12;
    static constexpr uint32
    VALUE_TREY=48,VALUE_FOUR=192,VALUE_FIVE=768,VALUE_SIX=3072;
    static constexpr uint32
    VALUE_SEVEN=12288,VALUE_EIGHT=49152,VALUE_NINE=196608;
    static constexpr uint32
    VALUE_TEN=786432,VALUE_JACK=3145728,VALUE_QUEEN=12582912;
    static constexpr uint32
    VALUE_KING=50331648,VALUE_ACEHIGH=201326592,VALUE_MISC=805306368;

	static constexpr uint32
    INCR_ACELOW=1,INCR_DEUCE=4;
    static constexpr uint32
    INCR_TREY=16,INCR_FOUR=64,INCR_FIVE=256,INCR_SIX=1024;
    static constexpr uint32
    INCR_SEVEN=4096,INCR_EIGHT=16384,INCR_NINE=65536;
    static constexpr uint32
    INCR_TEN=262144,INCR_JACK=1048576,INCR_QUEEN=4194304;
    static constexpr uint32
    INCR_KING=16777216,INCR_ACEHIGH=67108864,INCR_MISC=268435456;
};
#endif // __cplusplus




#endif
