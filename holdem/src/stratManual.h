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




#ifndef HOLDEM_ConsoleStrat
#define HOLDEM_ConsoleStrat

#define INFOASSIST

#include "arena.h"
#ifdef DEBUGSAVEGAME
#include <fstream>
#endif


///This class is used only for debugging, but the derived class UserConsoleStrategy allows for human players
class ConsoleStrategy : public PlayerStrategy
{
    private:
        #ifdef INFOASSIST
            StatResult winMean;
            DistrShape detailPCT;
            float64 rarity;
        #endif
	protected:
        Hand comBuf;
		bool bNoPrint;
		#ifdef INFOASSIST
            int8 bComSize;
		#endif
		void printCommunity();
		void printActions();
		void showSituation();
	public:


            std::istream *myFifo;


		ConsoleStrategy() : PlayerStrategy()
		#ifdef INFOASSIST
            , detailPCT(0)
		#endif
            , bNoPrint(false)

            ,myFifo(&(std::cin))

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
		virtual void FinishHand();

}
;

#endif

