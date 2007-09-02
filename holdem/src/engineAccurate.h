/***************************************************************************
 *   Copyright (C) 2007 by Joseph Huang                                    *
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




#ifndef HOLDEM_HandProbabilities
#define HOLDEM_HandProbabilities



class AnalysisDeck : public OrderedDeck
{
	private:

		Hand addend; //temporary
//		double executeRecursive(int,unsigned long,double);
		float64 executeIterative();
		int16 moreCards;
	public:

//		bool bRecursive;

		PlayStats* lastStats;

   		void DeOmitCards(const Hand&);

		void CleanStats();

		//double deals;

		float64 AnalyzeComplete(PlayStats*);
		float64 Analyze(PlayStats*);
		float64 Analyze(PlayStats*,const int8,const uint8,const uint32);
		float64 Analyze(PlayStats*,const DeckLocation&);

		AnalysisDeck() : OrderedDeck()
		{
			//bRecursive = false;
			lastStats = 0;
//			addend.Empty();
		}


		virtual ~DealRemainder();
}
;



#endif // HOLDEM_HandProbabilities


