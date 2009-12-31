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


#ifndef HOLDEM_CallRarity
#define HOLDEM_CallRarity

#include "debug_flags.h"
#include "portability.h"

class OpponentStandard
{
private:
	static const playernumber_t NO_FIRST_ACTION_PLAYER;
protected:
	playernumber_t foldsObserved;
	bool bMyActionOccurred;
	bool bNonBlindNonFoldObserved;
	playernumber_t firstActionPlayers;

	/*
	playernumber_t firstActionPlayers;
    playernumber_t notActedPlayers;
	*/
public:
	void NewRound();
	void SeeFold();
	void SeeNonBlindNonFold(const playernumber_t playersRemaining);

}
;

#endif // HOLDEM_CallRarity

