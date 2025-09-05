
#include "aiCache.h"
#include <iostream>

#include "randomDeck.h"

int main(int argc, const char * argv[]) {
  CommunityPlus withCommunity;
  #ifdef PROGRESSUPDATE
  std::cout << "Ready" << std::endl;
  #endif
  RandomDeck r;
  r.ShuffleDeck();
  #ifdef SUPERPROGRESSUPDATE
    std::cout << "TODO(from joseph): We shouldn't need this stwp" << std::endl;
    r.DealCard(withCommunity);
    std::cout << "1" << std::endl;
    r.DealCard(withCommunity);
    std::cout << "2" << std::endl;
  #else
    r.DealCard(withCommunity);
    r.DealCard(withCommunity);
  #endif

  PreflopCallStats pfcs(withCommunity, CommunityPlus::EMPTY_COMPLUS);
  std::cout << "3! Regenerating holdemdb… " << std::endl;
  pfcs.AutoPopulate();

}
