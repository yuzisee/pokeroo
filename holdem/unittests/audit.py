#!/usr/bin/env python
"""A script for generating a regression test.

Usage:
  Run this script, paste in the gamelog for a hand, and press Ctrl+d

  The only remaining edits you'll have to make:
   + the bot type
   + hand number
   + bot's hole cards
   + which player is the bot

"""

import math
import re
import sys


PLAYER_NAME_CHIP_COUNT = re.compile('^\s*\[([^ ]+) \$([0-9.]+)\]')
PLAYER_CALLS = re.compile('^(.*) calls \$([0-9.]+) \(\$[0-9.]+\)')
PLAYER_RAISES = re.compile('^(.*) raises.*to \$([0-9.]+) \(\$[0-9.]+\)')



#//  [0][1][2][3][4][5][6][7][8][9]
#//0. 2S 2H 2C 2D 3S 3H 3C 3D 4S 4H
#//1. 4C 4D 5S 5H 5C 5D 6S 6H 6C 6D
#//2. 7S 7H 7C 7D 8S 8H 8C 8D 9S 9H
#//3. 9C 9D TS TH TC TD JS JH JC JD
#//4. QS QH QC QD KS KH KC KD AS AH
#//5. AC AD

VALUE_START = {'2': 0, '3': 4, '4': 8, '5': 12, '6': 16,
               '7': 20, '8': 24, '9': 28, 'T': 32, 'J': 36,
               'Q': 40, 'K': 44, 'A': 48}

SUIT_PLUS = {'s': 0, 'h': 1, 'c': 2, 'd': 3}

def cardnum(s):
    return VALUE_START[s[0]] + SUIT_PLUS[s[1]]

class Player(object):
    def __init__(self, ident, chipcount):
        """Create a player to track what bets it makes

        Parameters:
          ident:
            (string) Player name
          chipcount:
            (float) Starting chip count

        """

        self.bets = [];
        self.ident = ident
        self.chipcount = chipcount

    def add_bet(self, amount):
        """Store a bet made.

        Parameters:
          amount:
            (float)
            The bet "to" amount. If calling, you can also put NaN.

        """
        assert isinstance(amount, float)
        self.bets.append(amount)

    def print_as_code(self, idx):

      print('')
      if all(b == 0.0 for b in self.bets):
        print('FixedReplayPlayerStrategy d{0}(foldOnly);'.format(idx))
      else:

        bet_strings = []
        for b in self.bets:
            if math.isnan(b):
                bet_strings.append('std::numeric_limits<float64>::signaling_NaN()')
            else:
                bet_strings.append(str(b))

        print('static const float64 a' + str(idx) + '[] = {')
        print(',\n'.join(bet_strings))
        print('};')
        print('FixedReplayPlayerStrategy d{0}(VectorOf(a{0}));'.format(idx))
      print('myTable.ManuallyAddPlayer("{0}", {1}, &d{2});'.format(self.ident, self.chipcount, idx))
      print('')


def write_table(small_blind, dealer):
    print('const playernumber_t dealer = {0};'.format(dealer))
    print('')
    print('struct BlindValues b;')
    print('b.SetSmallBigBlind({0});'.format(small_blind))
    print('')
    print('HoldemArena myTable(b.GetSmallBlind(), true, true);')
    print('myTable.setSmallestChip({0});'.format(small_blind))
    print('')
    print('const std::vector<float64> foldOnly(1, 0.0);')
    print('')


if __name__ == "__main__":
  sys.stderr.write('Press Ctrl+D to end')
  initializing = True
  players = {}
  for line in sys.stdin:

     if line.startswith('Preflop'):
        # This is the beginning of the hand. Clear everything.
        player_order = []
        assert len(players) == 0
        flop = None
        turn = None
        river = None
        assert initializing
     elif initializing:
        # Read out the player names and chip counts so we can get started.
        next_player_line = PLAYER_NAME_CHIP_COUNT.match(line)
        if next_player_line:
            p = Player(next_player_line.group(1), float(next_player_line.group(2)))
            players[p.ident] = p
            player_order.append(p)
            print('// Add player   {0}   ${1}'.format(p.ident, p.chipcount))
            p = None
        else:
            # Not a line describing a player.
            # In that case, either we're before the first player listing, or we've finished all player listing.
            # An easy way to check is to see whether we have any players at all yet...
            if players:
                # We already saw some players. Since this line isn't a player listing, we must be done!
                dealer = len(players) - 1
                initializing = False
     elif line.startswith('Flop:'):
        # This is the beginning of the flop
        flop = line.split()[1:4]
     elif line.startswith('Turn:'):
        # This is the beginning of the turn
        turn = line.split()[4]
     elif line.startswith('River:'):
        # This is the beginning of the turn
        river = line.split()[5]
     elif not initializing:
        # Okay, we aren't initializing anymore.
        # Read out player bets and store them.
        if ' posts SB of $' in line:
            small_blind = line.split('$')[1].split()[0]
            continue
        elif ' posts BB of $' in line:
            continue
        elif ' folds' in line:
            b = line.split()
            assert len(b) == 2
            players[b[0]].add_bet(0.0)
        elif ' checks' in line:
            b = line.split()
            assert len(b) == 2
            players[b[0]].add_bet(float('nan'))
        else:
            # TODO(from joseph): Does this work? It missed the `158.0` by P3 in unittests/main.cpp#testRegression_028
            b = PLAYER_CALLS.match(line)
            if b:
                players[b.group(1)].add_bet(float(b.group(2)))
            b = PLAYER_RAISES.match(line)
            if b:
                players[b.group(1)].add_bet(float(b.group(2)))



  print('')
  print('')
  print('PureGainStrategy bot("0.txt", <#4#>);')
  print('PlayerStrategy * const botToTest = &bot;')
  print('')
  write_table(small_blind, dealer)



  for n,p in enumerate(player_order):
      p.print_as_code(n)

  print("""

        DeckLocation card;

        {
// ==========================================================================
            CommunityPlus handToTest; // .. ..

            card.SetByIndex(<#card1#>);
            handToTest.AddToHand(card);

            card.SetByIndex(<#card2#>);
            handToTest.AddToHand(card);

            botToTest->StoreDealtHand(handToTest);
        }


        myTable.BeginInitialState(<#handnum#>);

// ==========================================================================


        myTable.BeginNewHands(std::cout, b, false, dealer);

""")


  if flop is None:
      print('myTable.PlayRound_BeginHand(std::cout)')
  else:
      print('assert(  myTable.PlayRound_BeginHand(std::cout)  !=  -1);')
      print('// Flop: ' + ' '.join(flop))
      print("""
        CommunityPlus myFlop;

        card.SetByIndex({0});
        myFlop.AddToHand(card);

        card.SetByIndex({1});
        myFlop.AddToHand(card);

        card.SetByIndex({2});
        myFlop.AddToHand(card);
""".format(cardnum(flop[0]), cardnum(flop[1]), cardnum(flop[2])))
      if turn is None:
        print('myTable.PlayRound_Flop(myFlop, std::cout);')
      else:
        print('assert(myTable.PlayRound_Flop(myFlop, std::cout) != -1);')
        print('// Turn: ' + turn)
        print("""
        DeckLocation myTurn;
        myTurn.SetByIndex({0});
        """.format(cardnum(turn)))

        if river is None:
          print('myTable.PlayRound_Turn(myFlop, myTurn, std::cout);')
        else:
          print('assert(  myTable.PlayRound_Turn(myFlop, myTurn, std::cout)  !=  -1);')
          print('// River: ' + river)
          print("""
        DeckLocation myRiver;
        myRiver.SetByIndex({0});
          """.format(cardnum(river)))
          print('myTable.PlayRound_River(myFlop, myTurn, myRiver, std::cout);')
