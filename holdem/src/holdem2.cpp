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



#include <iostream>
#include "holdem2.h"

using std::endl;

const CommunityPlus CommunityPlus::EMPTY_COMPLUS;

void CommunityPlus::PrintInterpretHand(std::ostream& targetFile) const
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
		targetFile << cardNameSingular[highCardNum] << "-High " << flush;

        if (strength == HoldemConstants::ROYAL
                || strength == HoldemConstants::STRAIGHT)
        {
			targetFile << "Straight" << flush;
        }
        if (strength == HoldemConstants::ROYAL
                || strength == HoldemConstants::FLUSH)
        {
			targetFile << "Flush " << flush;
        }
        return;
    }

    if (strength == HoldemConstants::BOAT)
    {
        targetFile << "Full House: ";
		targetFile << cardNamePlural[ (tempv / HoldemConstants::CARD_ACEHIGH) -1] ;
        targetFile << " over ";
        targetFile << cardNamePlural[ (tempv % HoldemConstants::CARD_ACEHIGH) -1] << flush;
        return;
    }

    if (strength >= HoldemConstants::QUAD_LOW
            && strength <= HoldemConstants::QUAD_HIGH)
    {
        targetFile << "Quad ";
        targetFile << cardNamePlural[ strength - HoldemConstants::QUAD_LOW ] << flush;
        return;
    }

    if (strength > HoldemConstants::SET_ZERO
            && strength <= HoldemConstants::SET_HIGH)
    {
        targetFile << "Trip ";
        targetFile << cardNamePlural[ strength - HoldemConstants::SET_ZERO - 1 ] << flush;
        return;
    }

    if (strength >= HoldemConstants::PAIRS_LOW)
    {
        int16 tempStrength = strength - HoldemConstants::PAIRS_LOW - 1;
        //It gets tricky... two pairs stores valueset a triangular/parabolic number
        int16 tempSum = -1;
        int16 tempbestpair=1;
        int16 tempnextbestpair=0;

        while(tempSum+tempbestpair<=tempStrength)
        {
            tempSum = tempSum + tempbestpair;
			++tempbestpair;
        }
        tempnextbestpair = tempStrength - tempSum;
        targetFile << cardNamePlural[ tempbestpair ];
        targetFile << " and ";
        targetFile << cardNamePlural[ tempnextbestpair] << flush;
        return;
    }

    if (strength > HoldemConstants::PAIR_ZERO)
    {
        targetFile << "Pair of " <<
			cardNamePlural[ strength - 1 - HoldemConstants::PAIR_ZERO ] << flush;
        return;
    }
    targetFile << "Nothing" << flush;
}

int8 CommunityPlus::CardsInSuit(const int8 a) const
{
	return (flushCount[a] + 5);
}

void CommunityPlus::DisplayHand(std::ostream& targetFile) const
{
	HandPlus::DisplayHand(targetFile);
	targetFile << "\t";
	targetFile << (int)(strength);
	targetFile << " : ";
	targetFile << valueset;
	targetFile << endl;
}

void CommunityPlus::DisplayHandText(std::ostream& targetFile) const
{
	HandPlus::DisplayHand(targetFile);
	targetFile << "\t";
    PrintInterpretHand(targetFile);
	targetFile << endl;
}

void CommunityPlus::DisplayHandBig(std::ostream& targetFile) const
{
	targetFile << endl;
    if (strength < 10)
		targetFile << "0";
    if (strength < 100)
		targetFile << "0";
	targetFile << static_cast<int>(strength) << "\t\t";
    PrintInterpretHand(targetFile);
    targetFile << endl;

	HandPlus::DisplayHandBig(targetFile);
}

void CommunityPlus::cleanLastTwo()
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


void CommunityPlus::evaluateStrength()
{


    //The (1st) 2nd to 14th bit must be the ONLY ones with data...
    //outer bits stay zero please


    //Pre-emptive STRAIGHT
    uint32 straights = prestraight;
    straights |= straights >> 13;
    straights &= straights << 1;
    straights &= straights << 1;
    straights &= straights << 1;
    straights &= straights << 1; //the high card of any straight remains

    strength = 0;
    //EVALUATE STRAIGHT FLUSHE


	///ASSUMPTION: You can't have two straight flushes
	///Valueset must not be used in the detection of straight flushes
    if( straights > 0 )
    {
        if( bFlushSuit >= 0 ) //(bFlushSuit != -1) implies bFlushSuit == 0 .. 3
        {

            uint32 sflush;


            sflush = cardset[bFlushSuit];

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


        }
    }


    //EVALUATE QUAD
    uint32 quads = cardset[0] & cardset[1] & cardset[2] & cardset[3];

    if (quads > 0)
    {
        prestraight &= ~quads;


        //There is four-of-a-kind
        strength = HoldemConstants::QUAD_HIGH;

        while( (quads & HoldemConstants::CARD_ACEHIGH) == 0)
        {
			--strength;
            quads <<= 1;
        }

		valueset = 13;
        while( (prestraight & HoldemConstants::CARD_ACEHIGH) == 0 )
		{
			--valueset;
            prestraight <<= 1;

		}

        return;
    }


    if (threeOfAKind != 0 && bestPair != 0)
    {
        //There is a full-house
        strength = HoldemConstants::BOAT;
        valueset = bestPair + threeOfAKind * HoldemConstants::CARD_ACEHIGH;
        return;
    }

    //extra debug checkpoint
	///ASSUMPTION: You can't have two flushes
    if( bFlushSuit >= 0 ) //(bFlushSuit != -1) implies bFlushSuit == 0 .. 3
    {

            //There is a flush
            strength = HoldemConstants::FLUSH;
            valueset = cardset[bFlushSuit];

            //Just like cleanLastTwo()
            int8 shiftCount = 1; //for consistency at least with
            //straights, the other hand that
            //uses valueset = tempcardset
            valueset >>= shiftCount;
            while (flushCount[bFlushSuit] > 0) //While there is a SURPLUS of cards in the flush suit
            {
                while ((valueset & HoldemConstants::CARD_ACELOW) == 0)
                {
                    ++shiftCount;
                    valueset >>= 1;
                }
                flushCount[bFlushSuit]--;
                valueset &= ~1;
            }
            valueset <<= shiftCount;

            return;
    }

    //TEST STRAIGHT
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
        bestPair-=2; //A TREY becomes 0
        nextbestPair-=2; //A DEUCE becomes -1
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



///HEAVILY INEFFICIENT. Avoid Use
void CommunityPlus::RemoveFromHand(
	const int8 aSuit,const uint8 aIndex,const uint32 aCard)
{
#ifdef DEBUGASSERT
	std::cerr << "HEAVILY INEFFICIENT. Avoid Use" << endl;
	exit(1);
#else

	HandPlus::RemoveFromHand(aSuit,aIndex,aCard);
	--flushCount[aSuit];
	populateValueset();
	preEvalStrength();
#endif
}

///aIndex is NOT RANK
void CommunityPlus::AddToHand(
	const int8 aSuit,const uint8 aIndex,const uint32 aCard)
{
	HandPlus::AddToHand(aSuit,aIndex,aCard);
    prestraight |= aCard;

	++flushCount[aSuit];
    //ASSUMPTION: You can have at most one flush
    if( flushCount[aSuit] >= 0 )
    {
        bFlushSuit = aSuit;
    }

	if(HoldemUtil::VALUEORDER[aIndex] == (HoldemUtil::VALUEORDER[aIndex] & valueset))
	{//Three of a kind


		if( aIndex > threeOfAKind )
		{

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


			///Then shift-insert the old threeOfAKind as a higher pair if possible
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

	}
	else
	{///Not three of a kind, but we are adding one of aIndex (valueset already updated)
		//We can screw this up safely if there is a legitimate quad, since two quads in seven cards is impossible.
		//ASSUMPTION: Omaha might need a different system?


		if( HoldemUtil::INCRORDER[aIndex] != (HoldemUtil::VALUEORDER[aIndex] & valueset) )
		{///This card matches another! (It couldn't be a trip because of above)


			if( aIndex > bestPair )
			{
				nextbestPair = bestPair;
				bestPair = aIndex;
			}
			else if ( aIndex > nextbestPair )
			{
				nextbestPair = aIndex;
			}

		}

	}
}

void CommunityPlus::AppendUnique(const Hand& h)
{
	HandPlus::AppendUnique(h);
	preEvalStrength();
}

void CommunityPlus::AppendUnique(const HandPlus& h)
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

void CommunityPlus::AppendUnique(const CommunityPlus& h)
{
	HandPlus::AppendUnique(h);

    prestraight |= h.prestraight;

	for(int8 cd=0;cd<4;++cd)
	{
		flushCount[cd] += h.flushCount[cd] + 5;
        if( flushCount[cd] >= 0 )
        {
            bFlushSuit = cd;
        }
	}

	///Just determine improvements to the pairs. That is, find new pairs
	///and compare them to the current pairs.

	if( threeOfAKind < h.threeOfAKind )
	{
		threeOfAKind = h.threeOfAKind;
	}
	/*Note: Since quads override trips in evaluateStrength, we don't have to worry
	about incorrect trips due to achieving a quad	*/
	unsigned long mockValueset = valueset;
    for(unsigned char i=13;i>threeOfAKind;--i)
    {

    	if( (mockValueset & HoldemConstants::VALUE_ACEHIGH) >> 26 == 3 )
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
    	mockValueset <<= 2;
    }

///threeOfAKind is now undoubtedly the highest trip available

	if( h.bestPair != threeOfAKind && h.bestPair > bestPair )
	{
		nextbestPair = bestPair;
		bestPair = h.bestPair;
	}

//Note: Here "fixed" means "corrected"
	if( threeOfAKind > 0 )
	{///Only bestPair is fixed
		mockValueset = valueset;
		for(unsigned char i=13;i>bestPair;--i)
		{

			if( (mockValueset & HoldemConstants::VALUE_ACEHIGH) >> 26 >= 2 && i != threeOfAKind )
			{
				bestPair = i;
			}
			mockValueset <<= 2;
		}
	}
	else
	{///Both pairs are fixed, AND you know threeOfAKind == 0.

		if( h.nextbestPair > nextbestPair )
		{
			nextbestPair = h.nextbestPair;
		}
		mockValueset = valueset;
		///We count downwards here to find if there is BEST a pair better than what we assume currently
		for(unsigned char i=13;i>bestPair;--i)
		{

			if( (mockValueset & HoldemConstants::VALUE_ACEHIGH) >> 26 >= 2 ) //== would be fine, there ARE NO trips
			{
				nextbestPair = bestPair;
				bestPair = i;
			}else //Leave mockValueset at bestPair
			{
                mockValueset <<= 2;
			}
		}

		for(unsigned char i=bestPair-1;i>nextbestPair;--i)
		{
		    ///Why does this decrement before and the above loop decrements after?
		    ///Think about it. We are skipping the scenario where i=bestPair which is the case after the above loop
            mockValueset <<= 2;
			if( (mockValueset & HoldemConstants::VALUE_ACEHIGH) >> 26 >= 2 ) //== would be fine, there ARE NO trips
			{
				nextbestPair = i;
			}

		}
	}

}

void CommunityPlus::SetEmpty()
{
	HandPlus::SetEmpty();
	flushCount[0] = -5;
	flushCount[1] = -5;
	flushCount[2] = -5;
	flushCount[3] = -5;
	threeOfAKind = 0;
	bestPair = 0;
	nextbestPair = 0;
	strength = 0;
    prestraight = 0;
    bFlushSuit = -1;
}

void CommunityPlus::SetUnique(const Hand& h)
{
	HandPlus::SetUnique(h);
	//set valueset
	preEvalStrength();
}

void CommunityPlus::preEvalStrength()
{
	uint32 tempforflush[4];
    for(int8 i=0;i<4;++i)
    {
        flushCount[i] = -5;
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
		flushCount[0] += static_cast<int8>(tempforflush[0] & 1);
        tempforflush[1] >>= 1;
        flushCount[1] += static_cast<int8>(tempforflush[1] & 1);
        tempforflush[2] >>= 1;
        flushCount[2] += static_cast<int8>(tempforflush[2] & 1);
        tempforflush[3] >>= 1;
        flushCount[3] += static_cast<int8>(tempforflush[3] & 1);

    }

    //This is slightly less efficient than it could be, but use of this function at all is discouraged anyway
    if( 0 <= flushCount[0] ) bFlushSuit = 0;
    if( 0 <= flushCount[1] ) bFlushSuit = 1;
    if( 0 <= flushCount[2] ) bFlushSuit = 2;
    if( 0 <= flushCount[3] ) bFlushSuit = 3;

    prestraight = cardset[0] | cardset[1] | cardset[2] | cardset[3];
}

void CommunityPlus::SetUnique(const HandPlus& h)
{
	HandPlus::SetUnique(h);

	preEvalStrength();

}

void CommunityPlus::SetUnique(const CommunityPlus& h)
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
    prestraight = h.prestraight;
    bFlushSuit = h.bFlushSuit;
}


CommunityPlus::CommunityPlus()
{
	SetEmpty();
}


