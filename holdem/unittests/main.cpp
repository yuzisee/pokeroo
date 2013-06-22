//
//  main.cpp
//  UnitTests
//
//  Created by Joseph Huang on 2013-06-22.
//  Copyright (c) 2013 Joseph Huang. All rights reserved.
//

#include <cassert>


#include "aiCache.h"
namespace NamedTriviaDeckTests {
    
    void testNamePockets() {
        
        CommunityPlus onlyCommunity(CommunityPlus::EMPTY_COMPLUS);
        CommunityPlus withCommunity(CommunityPlus::EMPTY_COMPLUS);
        
        // [0] [1] [2] [3] [4] [5] ...
        //  2S  2H  2C  2D  3S  3H ...
        DeckLocation dealt;
        
        // [18] == 6C
        dealt.Value	= 32;
        dealt.Suit = 2;
        dealt.Rank = 5;
        withCommunity.AddToHand(dealt);
        
        // [11] == 4D
        dealt.Value	= 8;
        dealt.Suit = 3;
        dealt.Rank = 3;
        withCommunity.AddToHand(dealt);
        
        
        NamedTriviaDeck o;
        o.OmitCards(withCommunity);
        o.DiffHand(onlyCommunity);
        o.sortSuits();
        
        
        string actual = o.NamePockets();
        string expected = "64x";
        
        assert(expected == actual);
    }
}

int main(int argc, const char * argv[])
{
    // Run all unit tests.
    NamedTriviaDeckTests::testNamePockets();
}

