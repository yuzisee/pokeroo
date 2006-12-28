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

#ifndef HOLDEM_SaveGameDeck
#define HOLDEM_SaveGameDeck

#include <iostream>
#include "randomDeck.h"

class SerializeRandomDeck : public RandomDeck
{
private:
    virtual void LogDeckState(std::ostream&);
public:
    SerializeRandomDeck(bool autoshuffle = false) : RandomDeck(autoshuffle) {}
    void Unserialize( std::istream& inFile );
    
    virtual void LoggedShuffle(std::ostream&);
    virtual void LoggedShuffle(std::ostream&,float64);
}
;


#endif

/*
 Arena must save:
 #if defined(DEBUGSPECIFIC) || defined(GRAPHMONEY)
 uint32 handnum
 #endif
 float64 smallestChip;


 blinds big and small. Raising level will be preserved
 player money,
 Can be derived >> livePlayers
 Can be derived >> float64 allChips;
 curDealer
 
 */