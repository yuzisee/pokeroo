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




#ifndef HOLDEM_ConsoleStrat
#define HOLDEM_ConsoleStrat


#include "arena.h"
#ifdef DEBUGSAVEGAME
#include <fstream>
#endif


class ConsoleStrategy : public PlayerStrategy
{

	protected:
		Hand comBuf;
		bool bNoPrint;
		void printCommunity();
		void printActions();
		void showSituation();
	public:
        
        #ifdef DEBUGSAVEGAME
            std::istream *myFifo;
        #endif
            
		ConsoleStrategy() : PlayerStrategy(), bNoPrint(false)
        #ifdef DEBUGSAVEGAME
            ,myFifo(&(std::cin))
        #endif
        {}

		virtual void SeeCommunity(const Hand&, const int8);

		virtual float64 MakeBet();

		virtual void SeeOppHand(const int8, const Hand&){};

		virtual void SeeAction(const HoldemAction&){};
}
;

class ConsoleStepStrategy : public ConsoleStrategy
{
	public:
	virtual float64 MakeBet();
}
;

class UserConsoleStrategy : virtual public ConsoleStrategy
{
	protected:
		float64 queryAction();
        #ifdef DEBUGSAVEGAME
        static std::ofstream logFile;
        #endif
	public:
        virtual ~UserConsoleStrategy();
		virtual float64 MakeBet();
		virtual void SeeAction(const HoldemAction&);
		virtual void SeeCommunity(const Hand&, const int8);

}
;

#endif

