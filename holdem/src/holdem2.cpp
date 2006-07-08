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




//#define DEBUGFLUSH
//#define DEBUGBOAT
//#define DEBUGPAIR

#include <iostream>
#include "holdem2.h"

using std::cout;
using std::endl;

const void CommunityPlus::PrintInterpretHand() const
{
    unsigned long tempv = valueset;
    const char cardNamePlural[13][8]
    =
        {"Deuces","Threes","Fours","Fives","Sixes","Sevens",
         "Eights","Nines","Tens","Jacks","Queens","Kings","Aces"
        };
    const char cardNameSingular[13][8]
    =
        {"Two","Three","Four","Five","Six","Seven",
         "Eight","Nine","Ten","Jack","Queen","King","Ace"
        };

    //Straights or Flushes
    if (strength == HoldemConstants::ROYAL
            || strength == HoldemConstants::STRAIGHT
            || strength == HoldemConstants::FLUSH)
    {
        int8 highCardNum = 12;
        while ((tempv & HoldemConstants::CARD_ACEHIGH) == 0)
        {
            highCardNum--;
            tempv <<= 1;
        }
		cout << cardNameSingular[highCardNum] << "-High " << flush;

        if (strength == HoldemConstants::ROYAL
                || strength == HoldemConstants::STRAIGHT)
        {
			cout << "Straight" << flush;
        }
        if (strength == HoldemConstants::ROYAL
                || strength == HoldemConstants::FLUSH)
        {
			cout << "Flush " << flush;
        }
        return;
    }

    if (strength == HoldemConstants::BOAT)
    {
        cout << "Full House: ";
		cout << cardNamePlural[ (tempv / HoldemConstants::CARD_ACEHIGH) -1] ;
        cout << " over ";
        cout << cardNamePlural[ (tempv % HoldemConstants::CARD_ACEHIGH) -1] << flush;
        return;
    }

    if (strength >= HoldemConstants::QUAD_LOW
            && strength <= HoldemConstants::QUAD_HIGH)
    {
        cout << "Quad ";
        cout << cardNamePlural[ strength - HoldemConstants::QUAD_LOW ] << flush;
        return;
    }

    if (strength > HoldemConstants::SET_ZERO
            && strength <= HoldemConstants::SET_HIGH)
    {
        cout << "Trip ";
        cout << cardNamePlural[ strength - HoldemConstants::SET_ZERO - 1 ] << flush;
        return;
    }

    if (strength >= HoldemConstants::PAIRS_LOW)
    {
        int16 tempStrength = strength - HoldemConstants::PAIRS_LOW - 1;
        //It gets tricky... two pairs stores valueset a triangular/parabolic number
        int16 tempSum = -1;
        int16 tempbestpair=1;
        int16 tempnextbestpair=0;
        //for (int16 i=1;(i*i+i+2)/2<=tempv;++i);
        while(tempSum+tempbestpair<=tempStrength)
        {
            tempSum = tempSum + tempbestpair;
			++tempbestpair;
        }
        tempnextbestpair = tempStrength - tempSum;
        cout << cardNamePlural[ tempbestpair ];
        cout << " and ";
        cout << cardNamePlural[ tempnextbestpair] << flush;
        return;
    }

    if (strength > HoldemConstants::PAIR_ZERO)
    {
        cout << "Pair of " <<
			cardNamePlural[ strength - 1 - HoldemConstants::PAIR_ZERO ] << flush;
        return;
    }
    cout << "Nothing" << flush;
}

const int8 CommunityPlus::CardsInSuit(const int8 a) const
{
	return (flushCount[a] + 5);
}

const void CommunityPlus::DisplayHand() const
{
	HandPlus::DisplayHand();
	cout << "\t" << static_cast<int>(strength) << " : " << valueset << endl;
}

const void CommunityPlus::DisplayHandBig() const
{
	cout << endl;
    if (strength < 10)
		cout << "0";
    if (strength < 100)
		cout << "0";
	cout << static_cast<int>(strength) << "\t\t";
    PrintInterpretHand();
    cout << endl;

	HandPlus::DisplayHandBig();
}

const void CommunityPlus::cleanLastTwo()
{
	const int8 VALUESETSHIFT = 2;
    //THIS MEANS WE ALWAYS HAVE SEVEN CARDS!!
    int8 shiftCount = 0;
    int8 cleanLeft = 2;
    valueset >>= VALUESETSHIFT;
    while (cleanLeft > 0)
    {
        while ((valueset & HoldemConstants::VALUE_ACELOW) == 0)
        {
            shiftCount+=2;
            valueset >>= 2;
        }
        cleanLeft--;
        valueset--;
    }
    valueset <<= shiftCount + VALUESETSHIFT;
}


const void CommunityPlus::evaluateStrength()
{

	//unsigned long tempcardset[4];

    //The (1st) 2nd to 14th bit must be the ONLY ones with data...
    //outer bits stay zero please

    strength = 0;
    //EVALUATE STRAIGHT FLUSHE
    uint32 sflush;
    //	unsigned long flush[4];
	///ASSUMPTION: You can't have two straight flushes
	///Valueset must not be used in the detection of straight flushes
    for(int8 i=0;i<4;++i)
    {
    	//tempcardset[i] = cardset[i];
        sflush = cardset[i];
        //		flush[i] = sflush;  //also prep for flush part later
        sflush |= sflush >> 13;//first add ace-low if ace-high exists

        sflush &= sflush << 1;
        sflush &= sflush << 1;
        sflush &= sflush << 1;
        sflush &= sflush << 1; //the high card of any straight remains

        if (sflush > 0)
        {
            //in case there is a 6 card straight or whatever:
            sflush &= ~((sflush >> 1) & sflush);

            strength = HoldemConstants::ROYAL;
            valueset = sflush;
            return;
        }
        //		flushCount[i] = -5;
    }

	//Pre-emptive STRAIGHT
	uint32 straights = cardset[0] | cardset[1] | cardset[2] | cardset[3];
    //EVALUATE QUAD
    uint32 quads = cardset[0] & cardset[1] & cardset[2] & cardset[3];

    if (quads > 0)
    {
        straights &= ~quads;


        //There is four-of-a-kind
        strength = HoldemConstants::QUAD_HIGH;

        while( (quads & HoldemConstants::CARD_ACEHIGH) == 0)
        {
			--strength;
            quads <<= 1;
        }

		valueset = 13;
		while( (straights & HoldemConstants::CARD_ACEHIGH) == 0 )
		{
			--valueset;
			straights <<= 1;
				#ifdef DEBUGASSERT
				//cout << "INFINTE LOOP: Quad-no-kicker!" << endl;
				#endif
		}

        return;
    }

    ///EVALUATE MATCHES, FLUSH
///TODO: CONFIRM THIS IS HERE ^^^


    if (threeOfAKind != 0 && bestPair != 0)
    {
        //There is a full-house
        strength = HoldemConstants::BOAT;
        valueset = bestPair + threeOfAKind * HoldemConstants::CARD_ACEHIGH;
        return;
    }

    //extra debug checkpoint
	///ASSUMPTION: You can't have two flushes
    for(int8 i=0;i<4;++i)
    {
        if  (flushCount[i] >= 0)
        {
				#ifdef DEBUGFLUSH
					cout << "f" << flushCount[i] << endl;
				#endif
            //There is a flush
            strength = HoldemConstants::FLUSH;
            valueset = cardset[i];

            //Just like cleanLastTwo()
            int8 shiftCount = 1; //for consistency at least with
            //straights, the other hand that
            //uses valueset = tempcardset
            valueset >>= shiftCount;
            while (flushCount[i] > 0)
            {
                while ((valueset & HoldemConstants::CARD_ACELOW) == 0)
                {
                    ++shiftCount;
                    valueset >>= 1;
                }
                flushCount[i]--;
                valueset &= ~1;
            }
            valueset <<= shiftCount;

            #ifdef DEBUGFLUSH
				DisplayHandBig();
            #endif
            return;
        }
    }
    //EVALUATE STRAIGHT
    straights |= straights >> 13;
    straights &= straights << 1;
    straights &= straights << 1;
    straights &= straights << 1;
    straights &= straights << 1; //the high card of any straight remains
    if (straights > 0)
    {
        straights &= ~((straights >> 1) & straights);
        strength = HoldemConstants::STRAIGHT;
        valueset = straights;
        return;
    }

    //EVALUATE THREE-OF-A-KIND
    if( threeOfAKind != 0 )
    {
        strength = HoldemConstants::SET_ZERO + threeOfAKind;
        valueset &= ~HoldemUtil::VALUEORDER[threeOfAKind];
        cleanLastTwo();
        return;
    }

    //EVALUATE TWO-PAIR, PAIR, HIGH CARD...
    if (nextbestPair != 0)
    {
        valueset &= ~HoldemUtil::VALUEORDER[nextbestPair];
        valueset &= ~HoldemUtil::VALUEORDER[bestPair];
        bestPair-=2;
        nextbestPair-=2;
        strength = ((bestPair*bestPair+bestPair)>>1) + nextbestPair
                                                        + HoldemConstants::PAIRS_LOW + 1;
        cleanLastTwo();
        return;
    }

    if (bestPair != 0)
    {
        strength = bestPair+HoldemConstants::PAIR_ZERO;
        valueset &= ~HoldemUtil::VALUEORDER[bestPair];
        cleanLastTwo();
        return;
    }

    strength = 1;
    cleanLastTwo();
    //COMPLETE!
}

/*
void CommunityPlus::constructDefaultCommunity()
{

    std::ifstream existTester;
    existTester.open( "comdata.ini" );

    //only do if the file doesn't exist
    if( existTester.fail() )
    {
        existTester.clear( std::ios::failbit );
        std::ofstream outputFile;
        outputFile.open( "comdata.ini", std::ofstream::out);
        if(outputFile)
        {
            //HERE
        }
        outputFile.close();
    }
    else
    {
        existTester.close();
    }
}


CommunityPlus::CommunityPlus(const int cardNum, const char* cards)
{
    //HOPEFULLY cardNum is always 7
    tempcardset[0] = 0;
    tempcardset[1] = 0;
    tempcardset[2] = 0;
    tempcardset[3] = 0;
    *(this->flushCount) = 0;
    *(this->flushCount+1) = 0;
    *(this->flushCount+2) = 0;
    *(this->flushCount+3) = 0;

    for(int i=0;i<cardNum;++i)
    {
        ++flushCount[HoldemUtil::CardSuitInt(cards[i])];
        //substitute += for |= to remove protection from duplicate cards
        tempcardset[ HoldemUtil::CardSuitInt(cards[i]) ]
        |=
            HoldemUtil::CARDORDER[ HoldemUtil::CardRankInt(cards[i]) + 1 ];
    }

    cardset[0] = tempcardset[0];
    cardset[1] = tempcardset[1];
    cardset[2] = tempcardset[2];
    cardset[3] = tempcardset[3];
    evaluateStrength();
}
*/

///HEAVILY INEFFICIENT. Avoid Use
const void CommunityPlus::RemoveFromHand(
	const int8 aSuit,const uint8 aIndex,const uint32 aCard)
{
#ifdef DEBUGASSERT
	cout << "HEAVILY INEFFICIENT. Avoid Use" << endl;
#endif

	HandPlus::RemoveFromHand(aSuit,aIndex,aCard);
	--flushCount[aSuit];
	populateValueset();
	preEvalStrength();
}

///aIndex is NOT RANK
const void CommunityPlus::AddToHand(
	const int8 aSuit,const uint8 aIndex,const uint32 aCard)
{
	HandPlus::AddToHand(aSuit,aIndex,aCard);
	++flushCount[aSuit];
	if(HoldemUtil::VALUEORDER[aIndex] == (HoldemUtil::VALUEORDER[aIndex] & valueset))
	{
			#ifdef DEBUGBOAT
				if( aIndex == 1 )
				{
					cout << "b" << bestPair << " n" << nextbestPair << endl;
					cout << threeOfAKind << endl;
					HandPlus::DisplayHandBig();
				}
			#endif

		///This addition formed a better three-of-a-kind
		///aIndex used to be paired, so we must un-pair it...
		if( aIndex == bestPair )
		{
			bestPair = nextbestPair;
			nextbestPair = 0;
		}
		else if( aIndex == nextbestPair )
		{
			nextbestPair = 0;
		}

		if( aIndex > threeOfAKind )
		{
			///First insert the old threeOfAKind as a higher pair if possible
			if( threeOfAKind > bestPair )
			{
				nextbestPair = bestPair;
				bestPair = threeOfAKind;
			}
			else if ( threeOfAKind > nextbestPair )
			{
				nextbestPair = threeOfAKind;
			}
			threeOfAKind = aIndex;
		}

			#ifdef DEBUGBOAT
				if( aIndex == 1 )
				{
					cout << "b" << bestPair << " n" << nextbestPair << endl;
					cout << threeOfAKind << endl;
				}
			#endif
	}
	else
	{///Not three of a kind, but we are adding one of aIndex (valueset already updated)
		//We can screw this up safely if there is a legitimate quad.


		if( HoldemUtil::INCRORDER[aIndex] != (HoldemUtil::VALUEORDER[aIndex] & valueset) )
		{///This card matches another! (It couldn't be a trip because of above)

				#ifdef DEBUGPAIR
					cout << "b" << bestPair << " n" << nextbestPair << endl;
					cout << threeOfAKind << endl;
					HandPlus::DisplayHandBig();
				#endif


			if( aIndex > bestPair )
			{
				nextbestPair = bestPair;
				bestPair = aIndex;
			}
			else if ( aIndex > nextbestPair )
			{
				nextbestPair = aIndex;
			}

				#ifdef DEBUGPAIR
					cout << "b" << bestPair << " n" << nextbestPair << endl;
					cout << threeOfAKind << endl;
				#endif
		}

	}
}

const void CommunityPlus::AppendUnique(const Hand& h)
{
	HandPlus::AppendUnique(h);
	preEvalStrength();
}

const void CommunityPlus::AppendUnique(const HandPlus& h)
{
	uint32 hValueset = h.valueset;
	for(uint8 i=1;i<=13;++i)
	{
		hValueset >>= 2;
		if ( (hValueset & 3) != 0 )
		{
			for(int8 cd=0;cd<4;++cd)
			{
				uint32 theCard = cardset[cd] & HoldemUtil::CARDORDER[i];
				if( theCard > 0)
				{
					AddToHand(cd,i,theCard);
				}
			}
		}
	}
}

const void CommunityPlus::AppendUnique(const CommunityPlus& h)
{
	HandPlus::AppendUnique(h);
	for(int cd=0;cd<4;++cd)
	{
		flushCount[cd] += h.flushCount[cd] + 5;
	}

	///Just determine improvements to the pairs. That is, find new pairs
	///and compare them to the current pairs.

	if( threeOfAKind < h.threeOfAKind )
	{
		threeOfAKind = h.threeOfAKind;
	}
	/*Note: Since quads override trips in evealuateStrength, we don't have to worry
	about incorrect trips due to achieving a quad	*/
	unsigned long mockValueset = valueset;
    for(unsigned char i=13;i>threeOfAKind;--i)
    {
    	mockValueset >>= 2;
    	if( (mockValueset & 3) == 3 )
    	{
			threeOfAKind = i;
			if( bestPair == i )
			{
				bestPair = nextbestPair;
				nextbestPair = 0;
			}
			else if( nextbestPair == i )
			{
				nextbestPair = 0;
			}
			break; //Unnecessary, I'm not sure if this slows things down or not
    	}
    }

///threeOfAKind is now undoubtedly the highest trip available

	if( h.bestPair != threeOfAKind && h.bestPair > bestPair )
	{
		nextbestPair = bestPair;
		bestPair = h.bestPair;
	}


	if( threeOfAKind > 0 )
	{///Only bestPair is fixed
		mockValueset = valueset;
		for(unsigned char i=13;i>bestPair;--i)
		{
			mockValueset >>= 2;
			if( (mockValueset & 3) >= 2 && i != threeOfAKind )
			{
				bestPair = i;
			}
		}
	}
	else
	{///Both pairs are fixed, AND you know threeOfAKind == 0.

		if( h.nextbestPair > nextbestPair )
		{
			nextbestPair = h.nextbestPair;
		}
		mockValueset = valueset;
		///We count downwards here to find the BEST pair first.
		for(unsigned char i=13;i>bestPair;--i)
		{

			if( (mockValueset & HoldemConstants::VALUE_ACEHIGH) >> 26 >= 2 ) //== would be fine, there ARE NO trips
			{
				nextbestPair = bestPair;
				bestPair = i;
			}
			mockValueset <<= 2;
		}

		for(unsigned char i=bestPair-1;i>nextbestPair;--i)
		{

			if( (mockValueset & HoldemConstants::VALUE_ACEHIGH) >> 26 >= 2 ) //== would be fine, there ARE NO trips
			{
				nextbestPair = i;
			}
			mockValueset <<= 2;
		}
	}

}

const void CommunityPlus::Empty()
{
	HandPlus::Empty();
	flushCount[0] = -5;
	flushCount[1] = -5;
	flushCount[2] = -5;
	flushCount[3] = -5;
	threeOfAKind = 0;
	bestPair = 0;
	nextbestPair = 0;
}

const void CommunityPlus::SetUnique(const Hand& h)
{
	HandPlus::SetUnique(h);
	//set valueset
	preEvalStrength();
}

const void CommunityPlus::preEvalStrength()
{
	unsigned long tempforflush[4];
    for(int8 i=0;i<4;++i)
    {
        flushCount[i] = -5;
        ///TODO: Confirm if the flush needs: cardset[] (>> 1)
        tempforflush[i] = cardset[i];
    }

///If it was last a triple and not a pair, the old set becomes the current pair
//I guess this is FALSE by default.

    uint32 mockValueset = valueset;
    for(uint8 i=1;i<=13;++i)
    {
		mockValueset >>= 2;

        if( (mockValueset & 3) == 3 )
        {
        	if( threeOfAKind > bestPair )
        	{
        		nextbestPair = bestPair;
        		bestPair = threeOfAKind;
        	}
        	///Once you have a three of a kind, the second best pair doesn't matter

        	threeOfAKind = (i);
        }
        else if( (mockValueset & 2) == 2 )
        {
        	nextbestPair = bestPair;
			bestPair = i;
        }



		tempforflush[0] >>= 1;
		flushCount[0] += tempforflush[0] & 1;
        tempforflush[1] >>= 1;
        flushCount[1] += tempforflush[1] & 1;
        tempforflush[2] >>= 1;
        flushCount[2] += tempforflush[2] & 1;
        tempforflush[3] >>= 1;
        flushCount[3] += tempforflush[3] & 1;

    }
}

const void CommunityPlus::SetUnique(const HandPlus& h)
{
	HandPlus::SetUnique(h);

	preEvalStrength();

}

const void CommunityPlus::SetUnique(const CommunityPlus& h)
{
	HandPlus::SetUnique(h);

	flushCount[0] = h.flushCount[0];
	flushCount[1] = h.flushCount[1];
	flushCount[2] = h.flushCount[2];
	flushCount[3] = h.flushCount[3];
	threeOfAKind = h.threeOfAKind;
	bestPair = h.bestPair;
	nextbestPair = h.nextbestPair;
	strength = h.strength;
}


CommunityPlus::CommunityPlus()
{
	Empty();
}
/*
CommunityPlus::CommunityPlus(const uint32 tempcardset[4])
{
    Refill(tempcardset);
}

const void CommunityPlus::Refill(const uint32 tempcardset[4])
{
	threeOfAKind = 0;
	bestPair = 0;
	nextbestPair = 0;

    unsigned long flush[4];
    for(int i=0;i<4;++i)
    {
        flushCount[i] = -5;
        flush[i] = (cardset[i] = (*(this->tempcardset + i) = tempcardset[i]));
    }
    for(int i=1;i<=13;++i)
    {
        //check for flushes
        flushCount[0] += flush[0] & 1;
        flush[0] >>= 1;
        flushCount[1] += flush[1] & 1;
        flush[1] >>= 1;
        flushCount[2] += flush[2] & 1;
        flush[2] >>= 1;
        flushCount[3] += flush[3] & 1;
        flush[3] >>= 1;
    }
    evaluateStrength();
}*/

