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


#include "aiInformation.h"
#include "engine.h"


#define DEBUG_MEMFAIL
//#define DEBUGNEWCALLSTATS
//#define DEBUGNEWORDER


#include <algorithm>




///Rather than truely 'diff'ing the hands, this diffs against only the COMMON cards
void TriviaDeck::DiffHand(const Hand& h)
{
    OrderedDeck::dealtHand[0] ^= OrderedDeck::dealtHand[0] & h.SeeCards(0);
	OrderedDeck::dealtHand[1] ^= OrderedDeck::dealtHand[1] & h.SeeCards(1);
	OrderedDeck::dealtHand[2] ^= OrderedDeck::dealtHand[2] & h.SeeCards(2);
	OrderedDeck::dealtHand[3] ^= OrderedDeck::dealtHand[3] & h.SeeCards(3);

}

void CommunityCallStats::Compare(const float64 occ)
{
        #ifdef DEBUGASSERT
            if( showdownIndex >= showdownCount )
            {
                std::cerr << "DEBUG" << endl;
                exit(1);
            }
        #endif

    PocketHand & newEntry = myHands[showdownIndex];


    //showdownMax += occ;
    const float64 incr = groupRepeated*occ;
    showdownMax += incr;
    newEntry.repeated = incr;
    newEntry.abRepeated = groupRepeated;

	

    if( indexHistory[0] > indexHistory[1] ) ///Because the 0...51 count is suit-major, we should match all pairs
    {
//std::cerr << (int)(indexHistory[0]) << "\t" << (int)(indexHistory[1]) << endl;
	
        newEntry.a = indexHistory[1];
        newEntry.b = indexHistory[0];
    }else // indexHistory[0] <= indexHistory[1]
    {
	
        newEntry.a = indexHistory[0];
        newEntry.b = indexHistory[1];
    }

    ///TODO: WTF If I use Reset(oppStrength) then it fails to SetUnique correctly the valueset field.
    newEntry.result.strength = oppStrength.strength;
    newEntry.result.valueset = oppStrength.valueset;
    newEntry.result.revtiebreak = newEntry.a*52+newEntry.b;
    ++showdownIndex;
    if( incr > 1 )
    {
        showdownCount -= static_cast<int32>(incr - 1);
    }
    //showProgressUpdate();
}


void CommunityCallStats::setCurrentGroupOcc(const float64 occ)
{
    groupRepeated = occ;
    #ifdef DEBUGNEWORDER
    std::cout << "Set " << occ << std::endl;
    #endif
}

void CommunityCallStats::mynoAddCard(const DeckLocation& cardinfo, const int16 undoIndex)
{
    indexHistory[undoIndex] = cardinfo.GetIndex();
        #ifdef DEBUGNEWORDER
            if( undoIndex == 1 )
            {
                std::cout << std::endl << "i[0]=" << ( (indexHistory[0] / 10)) << ((indexHistory[0] % 10));
                std::cout << "\ti[1]=" << ( (indexHistory[1] / 10)) << ( (indexHistory[1] % 10)) << "\t";
                DeckLocation firstcard;
                firstcard.SetByIndex(indexHistory[0]);
                HoldemUtil::PrintCard(std::cout,firstcard.Suit,firstcard.Value);
                HoldemUtil::PrintCard(std::cout,cardinfo.Suit,cardinfo.Value);
                std::cout << std::endl;
            }
        #endif
}

void CommunityCallStats::fillMyWins(StatResult ** table)
{

        #ifdef DEBUGNEWCALLSTATS
            float64 onlyPrintIfWrong = 0;
            float64 onlyPrintIfWrongR = 0;
        #endif

    ///Going from index 0, worst hands first.
    int32 curgroupStart;
    int32 curgroupAfter = 0;
    float64 curgroupRepeated; ///This is the number of hands that will split with this one
    float64 prevRepeated = 0;

    while( curgroupAfter < showdownCount )
    {
        curgroupStart = curgroupAfter;
        ++curgroupAfter;
        curgroupRepeated =  myHands[curgroupStart].repeated;
	
	if( curgroupAfter != showdownCount )
	{
		///We group by equal hand strength.
		while( myHands[curgroupAfter].result == myHands[curgroupStart].result )
		{
		curgroupRepeated += myHands[curgroupAfter].repeated;
		++curgroupAfter;
		if( curgroupAfter == showdownCount ) break;
		}
		///At the end of this loop, myHands[curgroupStart...(curgroupAfter-1)] would
		///be all and exactly all the hands that would split with one of them
	}



        int32 splitgroupStart;
        int32 splitgroupAfter = curgroupStart;
        float64 splitgroupRepeated; ///This is the number of hands that share the same hole cards

        while(splitgroupAfter < curgroupAfter)
        {
            splitgroupStart = splitgroupAfter;
            ++splitgroupAfter;
            splitgroupRepeated = myHands[splitgroupStart].repeated;
	    
	    if( splitgroupAfter != curgroupAfter )
	    {
		///We group by identical pockets
		while( myHands[splitgroupAfter].result.bIdenticalTo(myHands[splitgroupStart].result) )
		{
			splitgroupRepeated += myHands[splitgroupAfter].repeated;
			++splitgroupAfter;
			if( splitgroupAfter == curgroupAfter ) break;
		}
	    }

            float64 prevsplitRepeated = 0;
            for(int32 curgroupItr=splitgroupStart;curgroupItr<splitgroupAfter;++curgroupItr)
            {
                const PocketHand &tableEntry = myHands[curgroupItr];
                int8 carda = tableEntry.a;
                int8 cardb = tableEntry.b;

                StatResult now;


                now.wins = prevRepeated;
                now.splits = curgroupRepeated;
                now.loss = showdownMax - curgroupRepeated - prevRepeated;
                now.repeated = tableEntry.abRepeated;

                ///In this situation, we can use .repeated as the number "seen so far"

                const float64 thisOcc = tableEntry.repeated / tableEntry.abRepeated;

                StatResult *(&destination) = table[carda*52+cardb];
                if( destination == 0 )
                {
                    now.splits -= splitgroupRepeated;
                    now.loss -= myChancesEach - splitgroupRepeated;
			
                    destination = new StatResult;
                    *destination = now * thisOcc;

			#ifdef DEBUG_MEMFAIL
				std::cout << "New table[" << (int)(carda) << "," << (int)(cardb) << "=" << carda*52+cardb << "] entry " << destination << endl;
			#endif
                }else
                {
                    now.wins -= destination->repeated - prevsplitRepeated;
                    now.splits -= splitgroupRepeated;
                    now.loss -= myChancesEach - splitgroupRepeated - destination->repeated + prevsplitRepeated;
                    *destination = (*destination) + ( now * thisOcc );
                }

                prevsplitRepeated += tableEntry.repeated;


                ///Sum{wins,loss,splits,pct}   Average{repeated}
                    #ifdef DEBUGNEWCALLSTATS
                        if( now.wins + now.splits + now.loss != onlyPrintIfWrong )
                        {
                            std::cout << now.wins + now.splits + now.loss << "  = Sum{wins,loss,splits,pct} = Average{repeated}" << std::endl;
                            onlyPrintIfWrong = now.wins + now.splits + now.loss;
                        }
                    #endif
            }

        }

        prevRepeated += curgroupRepeated;
    }
}

void CommunityCallStats::Analyze()
{
    #ifdef DEBUGASSERT
        const int32 countedDealt = static_cast<int32>(showdownMax);
        int8 cardsAvail = realCardsAvailable(7 - moreCards);
        const int32 expectedDealt = cardsAvail*(cardsAvail-1)/2 * HoldemUtil::nchoosep<int32>(cardsAvail-2,moreCards-2) ;
        if( showdownIndex != showdownCount )
        {
            std::cerr << "PLEASE CHECK CommunityCallStats::Compare();" << endl;
            exit(1);
        }
        if( expectedDealt != countedDealt )
        {
            myStrength.DisplayHand(std::cerr);
            oppStrength.DisplayHand(std::cerr);
            std::cerr << "Maybe you didn't deal out proper hands" << endl;
            exit(1);
        }
    #endif
        #ifdef PROGRESSUPDATE
            std::cerr << "Analyzing...                    \r" << flush;
        #endif
    if( !bSortedHands )
    {
        std::sort(myHands,myHands+showdownIndex);
        bSortedHands = true;
    }
        #ifdef PROGRESSUPDATE
            std::cerr << "Deciding.....                    \r" << endl;
        #endif
///Strongest hands are at the higher indices.

    StatResult ** table = new StatResult*[52*52];
    for( int16 i=0 ; i < 52*52 ; ++i )
    {
        table[i] = 0;
    }



    fillMyWins(table);


    TriviaDeck iHave;
    iHave.OmitCards(myStrength);
    //iHave.sortSuits();
    int32 statIndex = 0;
    for( int16 i=0 ; i < 52*52 && statIndex < statCount ; ++i )
    {
        if( table[i] != 0 )
        {

            //OrderedDeck oHave;
            DeckLocation temp;
            Hand oHave;
            temp.SetByIndex(i%52);oHave.AddToHand(temp);
            temp.SetByIndex(i/52);oHave.AddToHand(temp);
            //oHave.OmitCards(oppStrength);
            //oHave.sortSuits();

            //if( !(iHave == oHave) )
            iHave.DiffHand(oHave);
            OrderedDeck emptyDeck;
            if( !(iHave == emptyDeck) )
            {
                /*(table[i])->genPCT();
                if( statIndex > 0 && ( myWins[statIndex-1] == *(table[i]) ) )
                {///Combine to reduce array size, thereby speeding up the sort later on
                    const float64 newRepeated = myWins[statIndex-1].repeated + (table[i])->repeated / myChancesEach;
                    myWins[statIndex-1] = myWins[statIndex-1] * myWins[statIndex-1].repeated
                                            +
                                          *(table[i]) * (table[i])->repeated / myChancesEach;
                    myWins[statIndex-1] = myWins[statIndex-1] / newRepeated;
                    myWins[statIndex-1].repeated = newRepeated;
                    //myWins[statIndex-1].genPCT();
                    --statCount;
                        #ifdef DEBUGNEWCALLSTATS
                            cout << "*" << flush;
                        #endif
                }else*/
                {
                    myWins[statIndex] = *(table[i]);
                    myWins[statIndex].repeated /= myChancesEach;
                    ++statIndex;
                        #ifdef DEBUGNEWCALLSTATS
                            std::cout << "+" << std::flush;
                        #endif
                }

                    #ifdef DEBUGNEWCALLSTATS
                        //float64 ttt = myWins[statIndex].loss+myWins[statIndex].splits+myWins[statIndex].wins;
                        //if( ttt >= myChancesEach - 2 || ttt == 0 )
                                HandPlus uPrint;
                                uPrint.SetUnique(oHave);
                                uPrint.DisplayHand(std::cout);
                                std::cout.precision(4);
                                std::cout << "{" << (statIndex-1) << "}" << myWins[statIndex-1].loss << " l + " <<
                                myWins[statIndex-1].splits << " s + " << myWins[statIndex-1].wins << " w = " <<
                                (myWins[statIndex-1].wins + (myWins[statIndex-1].splits/2)) << " (T:"
                                << (myWins[statIndex-1].splits + myWins[statIndex-1].loss + myWins[statIndex-1].wins )
                                << ")\tx;"<< myWins[statIndex-1].repeated << std::endl;

                    #endif

            }else
            {
                --statCount;
            }
		#ifdef DEBUG_MEMFAIL
			std::cout << "Free table[" << i <<  "] entry " << table[i] << endl;
		#endif

	    delete table[i];
        }
    }
    delete [] table;


    //myChancesEach = HoldemUtil::nchoosep<float64>(realCardsAvailable(7-moreCards) - 2,moreCards);
    ///Notice that although the above line corresponds to the ideal solution,
    ///we tend to generate the result correponding to the line below, due to
    ///the fact that the iHave.diff~emptyDeck comparison isn't as easily acheived when
    ///performing our "pyramid" algorithm. We instead fall back on iHave~oHave.
    const float64 dealout = HoldemUtil::nchoosep<float64>(realCardsAvailable(7-moreCards),2) * HoldemUtil::nchoosep<float64>(realCardsAvailable(7-moreCards)-2,moreCards-2);
    myChancesEach = ( dealout - myChancesEach )*myChancesEach;

    CallStats::Analyze();
}

void CommunityCallStats::showProgressUpdate() const
{
    if (statGroup == 0 ) std::cerr << endl << endl;
    std::cerr << "I: " << showdownIndex << "/" << showdownCount << "  \b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\r" << flush;

}

const int8 CommunityCallStats::realCardsAvailable(const int8 cardsInCommunity) const
{
    return 52 - cardsInCommunity;
}

void CommunityCallStats::initCC(const int8 cardsInCommunity)
{

	int8 cardsAvail = realCardsAvailable(cardsInCommunity);
	int8 preCardsAvail = cardsAvail - 2;


    int32 oppHands = cardsAvail*(cardsAvail-1)/2;
    showdownCount = oppHands * HoldemUtil::nchoosep<int32>(cardsAvail - 2,5-cardsInCommunity);

	#ifdef DEBUG_MEMFAIL
		std::cout << "Requesting " << showdownCount << std::endl;
	#endif
    myHands = new PocketHand[ showdownCount ];
	#ifdef DEBUG_MEMFAIL
		std::cout << "Received @ " << myHands << std::endl;
	#endif

    oppHands = preCardsAvail*(preCardsAvail-1)/2;
    myTotalChances = oppHands;
    myChancesEach = HoldemUtil::nchoosep<float64>(cardsAvail - 2,5-cardsInCommunity);
    //const float64 & t_f  = myChancesEach;
    showdownIndex = 0;
    showdownMax = 0;
    bSortedHands = false;

}

CommunityCallStats::~CommunityCallStats()
{
#ifdef DEBUG_MEMFAIL
std::cout << "Releasing " << std::flush;
#endif
    if( myHands != 0 )
    {
		#ifdef DEBUG_MEMFAIL
			std::cout << "@ " << myHands << std::endl;
		#endif
        delete [] myHands;
        myHands = 0;
    }
}



