/***************************************************************************
*   Copyright (C) 2005 by Joseph Huang                                    *
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


#include "arenaSave.h"

void SerializeRandomDeck::Unserialize( std::istream& inFile )
{
    lastDealtPos = -1;
    firstDealtPos = DECKSIZE-1;
    bDeckEmpty = false;

    for(uint8 i=0;i<DECKSIZE;++i)
    {
        //std::cout << "i=" << (int)i << endl;
        deckOrder[i] = HoldemUtil::ReadCard( inFile ) ;
    }
}

void SerializeRandomDeck::LogDeckState(std::ostream& outFile)
{
    for(uint8 i=0;i<DECKSIZE;++i)
    {
        ++lastDealtPos;
        lastDealtPos %= DECKSIZE;

        const int8 nextCard = deckOrder[lastDealtPos];
        HoldemUtil::PrintCard( outFile, nextCard );
    }
}

void SerializeRandomDeck::LoggedShuffle(std::ostream& outFile)
{
    RandomDeck::ShuffleDeck( );
    LogDeckState( outFile );
}

void SerializeRandomDeck::LoggedShuffle(std::ostream& outFile, float64 seedShift)
{
    RandomDeck::ShuffleDeck( seedShift );
    LogDeckState( outFile );

}

