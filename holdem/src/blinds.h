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


#ifndef HOLDEM_Blinds
#define HOLDEM_Blinds

#include "holdemutil.h"
#include "debug_flags.h"


//See also arena:h HoldemAction::SMALLBLIND, HoldemAction::BIGBLIND
#define PLAYERS_IN_BLIND 2

class BlindValues
{
    public:
        const static playernumber_t playersInBlind; //PLAYERS_IN_BLIND Constant
        float64 anteSize;
        float64 blindSize[PLAYERS_IN_BLIND];

        BlindValues()
        {
            anteSize = 0;
            for(playernumber_t n=0;n<PLAYERS_IN_BLIND;++n)
            {
                blindSize[n] = 0;
            }
        }

        BlindValues(const BlindValues & o)
        {
            *this = o;
        }

        BlindValues & operator=(const BlindValues & o)
        {
            anteSize = o.anteSize;
            for(playernumber_t n=0;n<PLAYERS_IN_BLIND;++n)
            {
                blindSize[n] = o.blindSize[n];
            }

            return *this;
        }


        const float64 & GetSmallBlind() const { return blindSize[0]; } //Small Blind
        const float64 & GetBigBlind() const { return blindSize[1]; } //Big Blind

        float64 AverageForcedBetPerHand(const playernumber_t livePlayers) const
        {
            float64 blindsPerButton = 0;
            for(playernumber_t n=0;n<PLAYERS_IN_BLIND;++n)
            {
                blindsPerButton += blindSize[n];
            }

            const float64 averageBlind = blindsPerButton / livePlayers;
            const float64 averageForced = averageBlind + anteSize;

            return averageForced;
        }

        float64 OpportunityPerHand(const playernumber_t livePlayers) const
        {
            const float64 averageForced = AverageForcedBetPerHand(livePlayers);
            //Each round is worth averageForced of opportunity cost.
            //Flat odds to win each hand before cards are dealt, is 1.0/livePlayers.
            // That's one hand on average each time the button goes around.
            // Your EV/equity here is +averageForced/livePlayers.
            //Likewise, someone else is going to win the other livePlayers-1 hands.
            // That's gives you an EV/equity of -averageForced*(livePlayers-1)/livePlayers
            //Adding these together means the net opportunity of each hand is:
            // + averageForced/livePlayers - averageForced*(livePlayers-1)/livePlayers
            // = averageForced*(1.0-livePlayers+1)/livePlayers
            // = averageForced*(2.0-livePlayers)/livePlayers

            //Opportunity cost is the negative of net opportunity
            const float64 averageOpporunityCost = (averageForced*(livePlayers - 2))/livePlayers;
            return averageOpporunityCost;


        }
        //(tableinfo->table->GetBigBlind() + tableinfo->table->GetSmallBlind()) * ( N - 2 )/ N / N;


        void SetSmallBigBlind(float64 small) { blindSize[0] = small; blindSize[1] = small*2.0; }

        const BlindValues operator*(const float64& fx) const
        {
            BlindValues a(*this);
            a *= fx;
            return a;
        }


        const BlindValues & operator*=(const float64& fx)
        {
            this->anteSize *= fx;
            for(playernumber_t n=0;n<PLAYERS_IN_BLIND;++n)
            {
                this->blindSize[n] *= fx;
            }

            return *this;
        }
}
;

struct BlindUpdate
{
    playernumber_t playersLeft;
    handnum_t handNumber;
    float64 timeSoFar;

    BlindValues b;
    bool bNew;
}
;

class BlindStructure
{
	public:
        virtual struct BlindUpdate UpdateSituation(struct BlindUpdate currentBlinds, struct BlindUpdate updateSituation) = 0;
        virtual void Serialize(std::ostream & saveToFile) = 0;
        virtual void UnSerialize(std::istream & restoreFromFile) = 0;
}
;

/*
class FixedBlinds : virtual public BlindStructure
{
	public:
    virtual struct BlindUpdate UpdateSituation(struct BlindUpdate currentBlinds, struct BlindUpdate updateSituation);
}
;
*/

class StackPlayerBlinds : virtual public BlindStructure
{
	protected:
		const float64 totalChips;
		const float64 smallBlindRatio;
	public:
		StackPlayerBlinds(float64 allstack, float64 sbratio) : totalChips(allstack), smallBlindRatio(sbratio) {}

    virtual struct BlindUpdate UpdateSituation(struct BlindUpdate currentBlinds, struct BlindUpdate updateSituation);
    virtual void Serialize(std::ostream & saveToFile){}
    virtual void UnSerialize(std::istream & restoreFromFile){}
}
;

class GeomPlayerBlinds : virtual public BlindStructure
{
	protected:
		const float64 incrRatio;
	public:
		GeomPlayerBlinds(float64 ratio)
	: incrRatio(ratio) {}

    virtual struct BlindUpdate UpdateSituation(struct BlindUpdate currentBlinds, struct BlindUpdate updateSituation);
    virtual void Serialize(std::ostream & saveToFile){}
    virtual void UnSerialize(std::istream & restoreFromFile){}
}
;

class SitAndGoBlinds : virtual public BlindStructure
{
    private:
    static float64 fibIncr(float64 a, float64 b);

    void InitializeHist(const float64 small,const float64 big);

    protected:
    float64 cur;
    float64 hist[3];
    const handnum_t handPeriod;
    handnum_t nextUpdate;

    public:


    SitAndGoBlinds(float64 small, float64 big, handnum_t afterHands)
	: handPeriod(afterHands)
    {
        InitializeHist(small,big);
    }

    virtual struct BlindUpdate UpdateSituation(struct BlindUpdate currentBlinds, struct BlindUpdate updateSituation);
    virtual void Serialize(std::ostream & saveToFile);
    virtual void UnSerialize(std::istream & restoreFromFile);
}
;

#endif
