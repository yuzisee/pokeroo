
#include "aiCache.h"
#include "callRarity.h"
#include <cassert>
#include <cmath>
#include <chrono>
#include <iomanip>
#include <iostream>
#include <sstream>

#include "randomDeck.h"


static string spotCheckDb(const struct DeckLocationPair &holeCards, char fileSuffix) {
      const int8_t cardsInCommunity = 0;

          CommunityPlus withCommunity;

          withCommunity.AddToHand(holeCards.first);

          withCommunity.AddToHand(holeCards.second);

          NamedTriviaDeck o;
          o.OmitCards(withCommunity);
          o.DiffHand(CommunityPlus::EMPTY_COMPLUS);
          o.sortSuits();
          string handName = o.NamePockets();

          {
              const std::time_t time_now = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
              std::cout << std::ctime(&time_now);
          }

          std::stringstream holdemjson_data;
          holdemjson_data << std::setprecision(std::numeric_limits<float64>::max_digits10 - 2);
          if (fileSuffix == 'C') {

             // StatResultProbabilities statprob;
             // ^^^ Back during https://github.com/yuzisee/pokeroo/commit/45ac51bd2b0407bbcbf980d66d91acc22f1a0df6
             //     this was StatResultProbabilities but it didn't need to be.
                std::cout << "Computing   " << handName << "   CallStats (.holdemC)\n";

                std::cout.flush(); // Flush for timestamping

            // struct CoreProbabilities statprob_core;
            CallCumulationD statprob_core_handcumu;
            ///Compute CallStats
            StatsManager::QueryDefense(statprob_core_handcumu,withCommunity,CommunityPlus::EMPTY_COMPLUS,cardsInCommunity);


            // EXPORT as .jsonC

            StatsManager::holdemCtoJSON(holdemjson_data, statprob_core_handcumu);
            (std::ofstream(StatsManager::dbFileName(withCommunity, CommunityPlus::EMPTY_COMPLUS,"C.json")) << holdemjson_data.str()).close();

          } else if (fileSuffix == 'W') {

            DistrShape dPCT(DistrShape::newEmptyDistrShape());
                std::cout << "Computing " << handName << " DistrShape (depends on *.holdemW)\n";
                std::cout.flush(); // Flush for timestamping

            ///Compute CommunityCallStats
            StatsManager::Query(&dPCT, withCommunity,CommunityPlus::EMPTY_COMPLUS,cardsInCommunity);


            // EXPORT as .jsonW

            StatsManager::holdemWtoJSON(holdemjson_data, dPCT);
            (std::ofstream(StatsManager::dbFileName(withCommunity, CommunityPlus::EMPTY_COMPLUS,"W.json")) << holdemjson_data.str()).close();
          } else {
            std::cerr << "spotCheckDb(…, " << fileSuffix << ")" << std::endl;
            exit(70); // man sysexits → EX_SOFTWARE
          }

          return handName;
}

/**
 * mode 0 & mode 1 are normal
 *
 * Use with two CPUs:
 * mode 2 means do only if i % 2 == 0
 * mode 3 means do only if i % 2 == 1
 *
 * Use with three CPUs:
 * mode 4 means do only if i % 3 == 0
 * mode 5 means do only if i % 3 == 1
 * mode 6 means do only if i % 3 == 2
 *
 * Use with four CPUs:
 * mode 7 means do only if i % 4 == 0
 * mode 8 means do only if i % 4 == 1
 * mode 9 means do only if i % 4 == 2
 * mode 10 means do only if i % 4 == 3
 *
 * Use with five CPUs:
 * mode 11 means do only if i % 5 == 0
 * ...
 *
 * mode === (1 + 2 + ... + (CPUs-1)) + order
 * mode - order === (1 + CPUs - 1)/2 * (CPUs-1)
 * mode - order === CPUs * (CPUs - 1)/2
 * mode - order === (CPUs^2 - CPUs)/2
 * CPUs^2 - CPUs - 2 mode + 2 order == 0
 * (CPUs - 0.5)^2 - 0.25 - 2 mode + 2 order = 0
 * CPUs = 0.5 +/- sqrt(0.25 + 2 (mode - order))
 * 2 CPUs = 1 +/- sqrt(1 + 8(mode-order))
 *
 * order is at least 1, so subtract it as a baseline
 *
 * CPUs = floor((sqrt(​8*​x-​7)+​1)/​2);
 */
static void regenerateDb(int mode) {
    std::chrono::time_point<std::chrono::system_clock> time_end;

    // This is for easier parallelization
    std::cout << "Mode: " << mode << "\n";

    size_t CPUs = 0;
    size_t offset = 0;
    if (mode <= 1) {
        CPUs = 1;
    } else {
        CPUs = std::floor((std::sqrt(8*mode - 7) + 1)/2);
        offset = mode - CPUs * (CPUs - 1) / 2 - 1;
    }

    std::cout << "CPUs: " << CPUs << " offset: " << offset << "\n";




    std::vector<struct DeckLocationPair> handList;

    // Pocket-pairs
    for (int8_t pocketRank = 0; pocketRank < 13; ++ pocketRank) {
        DeckLocation card1;
        card1.SetByIndex(pocketRank * 4);

        DeckLocation card2;
        card2.SetByIndex(pocketRank * 4 + 1);

        handList.push_back(DeckLocationPair(card1, card2));

    }

    // All non-pairs
    for (int8_t firstCardRank = 0; firstCardRank < 13; ++firstCardRank) {
        for(int8_t secondCardRank = firstCardRank + 1; secondCardRank < 13; ++secondCardRank) {
            // Suited
            {
                DeckLocation card1;
                card1.SetByIndex(firstCardRank * 4);

                DeckLocation card2;
                card2.SetByIndex(secondCardRank * 4 + 1);

                handList.push_back(DeckLocationPair(card1, card2));
            }


            // Not suited
            {
                DeckLocation card1;
                card1.SetByIndex(firstCardRank * 4);

                DeckLocation card2;
                card2.SetByIndex(secondCardRank * 4);

                handList.push_back(DeckLocationPair(card1, card2));

            }
        }

    }


    assert(handList.size() == 169);

    const std::chrono::time_point<std::chrono::system_clock> holdemC_start = std::chrono::system_clock::now();

    size_t counter;
    std::cout << "Begin CallStats.\n";
    counter = 0;
    for (const DeckLocationPair & holeCards : handList) {
        if (counter % CPUs != offset) {
            std::cout << "skip→";
            std::cout.flush();
            ++counter;
            continue;
        }

        const std::chrono::time_point<std::chrono::system_clock> time_start = std::chrono::system_clock::now();
        spotCheckDb(holeCards, 'C');
        time_end = std::chrono::system_clock::now();

        ++counter;

        std::stringstream complete_msg;
        complete_msg << "=== 📊 Complete!   " << static_cast<int>(counter) << " of " << static_cast<int>(handList.size())
                     << "   (by worker #" << static_cast<int>(offset) << "/" << static_cast<int>(CPUs) << " in "
                     << std::chrono::duration_cast<std::chrono::seconds>(time_end - time_start).count() << " seconds"
                     << ", ⏱total " << std::setprecision(3) << (std::chrono::duration_cast<std::chrono::minutes>(time_end - holdemC_start).count() / 60.0) << "h elapsed"
                     << ") ===\n\n";
        std::cout << complete_msg.str();
        std::cout.flush(); // Flush for timestamping
    }

    const double holdemC_hours = (std::chrono::duration_cast<std::chrono::minutes>(time_end - holdemC_start).count() / 60.0);
    const std::chrono::time_point<std::chrono::system_clock> holdemW_start = std::chrono::system_clock::now();

    std::cout << "Begin CommunityCallStats.\n";
    counter = 0;
    for (const DeckLocationPair & holeCards : handList) {
        if (counter % CPUs != offset) {
            std::cout << "skip→";
            std::cout.flush();
            ++counter;
            continue;
        }

        const std::chrono::time_point<std::chrono::system_clock> time_start = std::chrono::system_clock::now();
        spotCheckDb(holeCards, 'W');
        time_end = std::chrono::system_clock::now();

        ++counter;

        std::stringstream complete_msg;
        complete_msg << "=== ❚❙❘ Complete!   " << static_cast<int>(counter) << " of " << static_cast<int>(handList.size())
                     << "   (by worker #" << static_cast<int>(offset) << "/" << static_cast<int>(CPUs) << " in "
                     << std::setprecision(3) << (std::chrono::duration_cast<std::chrono::seconds>(time_end - time_start).count() / 60.0) << " minutes"
                     << ", ⏱total " << std::setprecision(3) << holdemC_hours << " + " << (std::chrono::duration_cast<std::chrono::minutes>(time_end - holdemW_start).count() / 60.0) << "h elapsed"
                     << ") ===\n\n";
        std::cout << complete_msg.str();
        std::cout.flush(); // Flush for timestamping
    }

    const std::time_t time_now = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    std::cout << std::ctime(&time_now);
}


// StatsManager::UnserializeW and StatsManager::SerializeW
/* [22x.holdemW]
 * aarch64 / arm64
00000000  00 00 00 00 00 24 d3 40  d2 33 d7 69 44 9b df 3f  |.....$.@.3.iD..?|
00000010  3d a4 8c 96 ab 6e 93 3f  ea 01 c0 dc d0 2d df 3f  |=....n.?.....-.?|
00000020  00 00 00 00 00 00 f0 3f  7b cc 45 e3 5c 1b e0 3f  |.......?{.E.\..?|
00000030  b4 5c 9a 8a 3e fd ef 3f  00 00 00 00 00 00 00 00  |.\..>..?........|
00000040  76 5f 1a 2d ab 0b 36 3f  00 00 00 00 00 00 00 40  |v_.-..6?.......@|
00000050  b4 5c 9a 8a 3e fd ef 3f  60 76 cf 21 17 67 d1 3f  |.\..>..?`v.!.g.?|
00000060  06 f3 9d 84 5e 63 af 3f  9f 65 ce 86 3e 56 e5 3f  |....^c.?.e..>V.?|
00000070  00 00 00 00 00 00 00 40  91 55 19 0a 4d 5d d3 3f  |.......@.U..M].?|
00000080  26 8e 55 fa db 1c d7 3f  26 ad 77 04 f5 70 a1 3f  |&.U....?&.w..p.?|
00000090  27 be 8d b2 82 5a e3 3f  00 00 00 00 00 60 99 40  |'....Z.?.....`.@|
000000a0  f9 08 9d 4a eb 33 d8 3f  bd 85 75 f7 3a 43 dc 3f  |...J.3.?..u.:C.?|
000000b0→(de)ed 0f 3e 58 3b 93 3f  9a bd 54 c2 87 44 e1 3f  |...>X;.?..T..D.?|
000000c0  00 00 00 00 00 45 cd 40  2d 05 66 b9 15 dd dc 3f  |.....E.@-.f....?|
000000d0  3a 4d 36 75 7d 00 e0 3f  83 2f 2e b5 a6 0c a2 3f  |:M6u}..?./.....?|
000000e0  98 9f ed 3e 70 bd dd 3f  00 00 00 00 00 f0 83 40  |...>p..?.......@|
000000f0  b6 be df aa e2 90 e0 3f  17 72 10 8f c3 f8 e2 3f  |.......?.r.....?|
00000100  64 fb 7b 65 bc d5 b2 3f  f7 1c 80 c8 09 59 d5 3f  |d.{e...?.....Y.?|
00000110  00 00 00 00 00 00 48 40  ce 31 68 55 1f 26 e4 3f  |......H@.1hU.&.?|
00000120  20 f9 31 85 05 31 e8 3f  61 87 70 cb 5a 7e a6 3f  | .1..1.?a.p.Z~.?|
00000130  a9 f9 5b 38 53 9c c9 3f  00 00 00 00 00 00 24 40  |..[8S..?......$@|
00000140→ 5b 7d 8d 5b f8 e4 e8 3f (b1)9d e4 70 b6 01 eb 3f  |[}.[...?...p...?|
00000150→ ce 8c 88 83 7e 55 97 3f [a0]77 fc 6b 76 0e c1 3f  |....~U.?.w.kv..?|
00000160  00 00 00 00 00 20 72 40  e6 bf f2 6a 0c 5f eb 3f  |..... r@...j._.?|
00000170  7b ca c3 c2 0b f2 ed 3f  74 e6 09 26 78 2e 51 3f  |{......?t..&x.Q?|
00000180→(91)84 49 09 e8 2a b0 3f  00 00 00 00 00 50 9f 40  |..I..*.?.....P.@|
00000190  f5 4c cd 60 57 f6 ed 3f  90 27 8c f7 64 cd b9 3f  |.L.`W..?.'..d..?|
000001a0  67 aa 30 b5 0a 32 c4 3f  e2 0f 14 a0 f2 3f e6 bf  |g.0..2.?.....?..|
000001b0  68 7b 74 66 52 ae 01 40  ce d1 6e 4c c8 6c 0a 40  |h{tfR..@..nL.l.@|
000001c0

 * x64
00000000  00 00 00 00 00 24 d3 40  d2 33 d7 69 44 9b df 3f  |.....$.@.3.iD..?|
00000010  3d a4 8c 96 ab 6e 93 3f  ea 01 c0 dc d0 2d df 3f  |=....n.?.....-.?|
00000020  00 00 00 00 00 00 f0 3f  7b cc 45 e3 5c 1b e0 3f  |.......?{.E.\..?|
00000030  b4 5c 9a 8a 3e fd ef 3f  00 00 00 00 00 00 00 00  |.\..>..?........|
00000040  76 5f 1a 2d ab 0b 36 3f  00 00 00 00 00 00 00 40  |v_.-..6?.......@|
00000050  b4 5c 9a 8a 3e fd ef 3f  60 76 cf 21 17 67 d1 3f  |.\..>..?`v.!.g.?|
00000060  06 f3 9d 84 5e 63 af 3f  9f 65 ce 86 3e 56 e5 3f  |....^c.?.e..>V.?|
00000070  00 00 00 00 00 00 00 40  91 55 19 0a 4d 5d d3 3f  |.......@.U..M].?|
00000080  26 8e 55 fa db 1c d7 3f  26 ad 77 04 f5 70 a1 3f  |&.U....?&.w..p.?|
00000090  27 be 8d b2 82 5a e3 3f  00 00 00 00 00 60 99 40  |'....Z.?.....`.@|
000000a0  f9 08 9d 4a eb 33 d8 3f  bd 85 75 f7 3a 43 dc 3f  |...J.3.?..u.:C.?|
000000b0→(dd)ed 0f 3e 58 3b 93 3f  9a bd 54 c2 87 44 e1 3f  |...>X;.?..T..D.?|
000000c0  00 00 00 00 00 45 cd 40  2d 05 66 b9 15 dd dc 3f  |.....E.@-.f....?|
000000d0  3a 4d 36 75 7d 00 e0 3f  83 2f 2e b5 a6 0c a2 3f  |:M6u}..?./.....?|
000000e0  98 9f ed 3e 70 bd dd 3f  00 00 00 00 00 f0 83 40  |...>p..?.......@|
000000f0  b6 be df aa e2 90 e0 3f  17 72 10 8f c3 f8 e2 3f  |.......?.r.....?|
00000100  64 fb 7b 65 bc d5 b2 3f  f7 1c 80 c8 09 59 d5 3f  |d.{e...?.....Y.?|
00000110  00 00 00 00 00 00 48 40  ce 31 68 55 1f 26 e4 3f  |......H@.1hU.&.?|
00000120  20 f9 31 85 05 31 e8 3f  61 87 70 cb 5a 7e a6 3f  | .1..1.?a.p.Z~.?|
00000130  a9 f9 5b 38 53 9c c9 3f  00 00 00 00 00 00 24 40  |..[8S..?......$@|
00000140→ 5b 7d 8d 5b f8 e4 e8 3f (b2)9d e4 70 b6 01 eb 3f  |[}.[...?...p...?|
00000150→ ce 8c 88 83 7e 55 97 3f [9e]77 fc 6b 76 0e c1 3f  |....~U.?.w.kv..?|
00000160  00 00 00 00 00 20 72 40  e6 bf f2 6a 0c 5f eb 3f  |..... r@...j._.?|
00000170  7b ca c3 c2 0b f2 ed 3f  74 e6 09 26 78 2e 51 3f  |{......?t..&x.Q?|
00000180→(90)84 49 09 e8 2a b0 3f  00 00 00 00 00 50 9f 40  |..I..*.?.....P.@|
00000190  f5 4c cd 60 57 f6 ed 3f  90 27 8c f7 64 cd b9 3f  |.L.`W..?.'..d..?|
000001a0  67 aa 30 b5 0a 32 c4 3f  e2 0f 14 a0 f2 3f e6 bf  |g.0..2.?.....?..|
000001b0  68 7b 74 66 52 ae 01 40  ce d1 6e 4c c8 6c 0a 40  |h{tfR..@..nL.l.@|
000001c0

There might have been an x32 version before that as well
https://github.com/yuzisee/pokeroo/commit/3041337be97ca5e4d43cde9f37650b1acfff2b60
*/

int main(int argc, const char * argv[]) {

  const std::chrono::time_point<std::chrono::system_clock> start_time = std::chrono::system_clock::now();
  const std::time_t t_c = std::chrono::system_clock::to_time_t(start_time);
  std::cout << std::ctime(&t_c) << " ▸ " << argv[0] << std::endl;

  if (argc == 1) {
    // No arguments other than the executable name itself.

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
  } else {

    // Regenerate the DB (striped, in case you want to run multiple times on separate threads)
    const int mode = std::stoi(argv[1]); // atoi(argv[1])
    // see also `std::strtol`

    /*
      const std::string trueStr("true");
      char* github_actions_env;
      github_actions_env = getenv ("GITHUB_ACTIONS");
      if (github_actions_env && (trueStr == github_actions_env)) {

      }
    */

    const bool spot_check_regression_test = (mode < 0);
    if (spot_check_regression_test) {
      std::cout << "↓ If you run into issues, reproduce locally by running:" << std::endl;
      std::cout << "HOLDEMDB_PATH=" << StatsManager::dbFolderPath() << " " << argv[0] << " " << mode << std::endl;
      DeckLocation card1;
      DeckLocation card2;
      // This is for continuous integration testing: We'll quickly run 22

      const int16_t cardidx1 = (-mode) / 100;
      const int16_t cardidx2 = (-mode) % 100;

      // TODO(from joseph): Would it make more sense to use HoldemUtil::ReadCard (or even HoldemUtil::ParseCard)

      card1.SetByIndex(cardidx1);
      card2.SetByIndex(cardidx2);
      const string name1 = spotCheckDb(DeckLocationPair(card1, card2), 'C');
      const string name2 = spotCheckDb(DeckLocationPair(card1, card2), 'W');

      const std::chrono::time_point<std::chrono::system_clock> completion = std::chrono::system_clock::now();
      // https://stackoverflow.com/questions/7889136/stdchrono-and-cout
      std::cout << "✅`" << name1 << ".holdem{C,W}` altogether completed in " << std::chrono::duration_cast<std::chrono::seconds>(completion - start_time).count() << " seconds → " << name2 << endl;

    } else {
      regenerateDb(mode);
    }
  }

}
