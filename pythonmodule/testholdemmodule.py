#!/usr/bin/env python

# http://snipplr.com/view/7354/get-home-directory-path--in-python-win-lin-other/
import os
userModulePath = os.path.expanduser('~/lib/python')

# http://docs.python.org/install/index.html
# After running:
#	python setup.py build
#	python setup.py install --home=~
import sys
sys.path.append(userModulePath); #This helps Python find the extension in Linux
#But the DLL search path is determined by Windows, not Python
#Most recently,  http://msdn.microsoft.com/en-us/library/ms972822.aspx says:
#"The default behavior now is to look in all the system locations first, then the current directory, and finally any user-defined paths."
#The DLL will be local just be safe. This is a test script anyways.

print "Ready?"

# http://www.python.org/doc/2.5.2/ext/dynamic-linking.html


#http://docs.python.org/library/sys.html#sys.path
#http://docs.python.org/library/os.html?highlight=os#os-file-dir
##if os.path.isdir('run') and os.path.isfile('run/holdemdb.ini'):
##    os.chdir('run')
##else:
##

chip_size = 0.5


import holdem


print os.path.abspath(os.curdir)

if not os.path.isfile('run/holdemdb.ini'):
    raise RuntimeError, "Get a run/holdemdb.ini to avoid a very long calculation (~45 mins)"

os.chdir('run')

a = holdem.HoldemTable(chip_size)

#http://www.mrsneeze.com/mrmen/meetmrmen.html
p1 = a.add_player_clockwise("Mr. Happy",600,None)
p2 = a.add_player_clockwise("Mr. Greedy",1200,None)
b1 = a.add_player_clockwise("Mr. Nosey",300,"gear")
b2 = a.add_player_clockwise("Mr. Sneeze",50,"trap")

# Printing strings: http://www.python.org/doc/2.5.2/tut/node9.html
# Concatenating strings: http://www.skymind.com/~ocrow/python_string/
print ' '.join(['Table created with maximum ', str(a.seat_number), ' seats'])

##
##1. Create a New Table
##2. Add Players and InitializeNew the TableState, or RestoreTableState
##3. You may save the state of the table at this point, if you really want to.
##4. -----
##	4.1 BeginNewHand
##	4.2 ShowHoleCards to all bots who need to know their hand
##	4.3 If you would like to use the deterministic random seed generator, reset it here: ResetDeterministicSeed().
##	4.4 -----
##		 4.4.1a CreateNewCardset() a blank hand to represent the pre-flop and CreateNewBettingRound() with it
##		 4.4.1b Make bets for WhoIsNext_Betting() with PlayerMakesBetTo(), while querying your bots through GetBetDecision()
##		 4.4.1c DeleteFinishBettingRound() and it will report if the high bet was called.
##
##		 4.4.2a If the preflop high bet was called, AppendCard() your flop to the holdem_cardset and then CreateNewBettingRound() with it
##		 4.4.2b Make bets ...
##		 4.4.2c DeleteFinishBettingRound() ...
##
##		 4.4.3 ... If the postflop high-bet was called, play the Turn ... and it will report if the high bet is called.
##
##		 4.4.4 ... If the high-bet was called after the turn, play the River ... and it will report if the high bet is called.
##
##		 4.4.5a If the high-bet was called after the river, CreateNewShowdown()
##		 4.4.5b Each WhoIsNext_Showdown can PlayerShowsHand() or PlayerMucksHand()
##		 4.4.5c DeleteFinishShowdown() will calculate side pots and move money from the pot to the winners. Compare GetMoney() before and after to determine winners.
##
##	4.5 Call FinishHandRefreshPlayers() to complete any final bookkeeping that needs to take place to prepare data structures for the next hand
##	4.6 Here is a good time to shuffle the deck. Retrieve a deterministic seed with GetDeterministicSeed() if you would like.
##	4.7 If you would like to save the game, now is the best time to do so. Call SaveTableState()
##	4.8 [Go back to step 4.1]

a.initialize_table()

serialized_table = a.save_state()

print "Printing save_state"
for c in serialized_table:
    print "0x%#x" % ord(c),
    print ' '.join(['[', c, ']'])

print "Giving some cards to b1/b2"

b1.hole_cards = holdem.HoldemCards().append_cards("Ks 7h")
b2.hole_cards = holdem.HoldemCards().append_cards("8c 7c")

print "Starting a new pot"

pot = a.start_new_pot(7)

print "Starting a betting round"


betting_round = pot.start_betting_round(holdem.HoldemCards())

print "Starting at seat 0 we have dealer, small, big."
print "Next up should be seat #",
print betting_round.which_seat_is_next()

bet_choice = betting_round.auto_make_bet(b2)

print b2.name,
print "has decided to bet",
print bet_choice

#betting_round.player_raises_to(b2,bet_choice)

print "Seat #",
print betting_round.which_seat_is_next(),
print "is next to bet"

betting_round.player_check_calls(p1)
betting_round.player_check_calls(p2)
betting_round.player_check_calls(b1)

print "Betting round finished"
pot.finish_betting_round()


print "Flop..."
next_betting_round = pot.start_betting_round(holdem.HoldemCards().append_cards("As Ac Ad"))
while next_betting_round.which_seat_is_next() != None:
    next_betting_round.player_check_calls(a.players[next_betting_round.which_seat_is_next()])
pot.finish_betting_round()


print "Turn..."
next_betting_round = pot.start_betting_round(holdem.HoldemCards().append_cards("As Ac Ad 2d"))
while next_betting_round.which_seat_is_next() != None:
    next_betting_round.player_check_calls(a.players[next_betting_round.which_seat_is_next()])
pot.finish_betting_round()

print "River..."
next_betting_round = pot.start_betting_round(holdem.HoldemCards().append_cards("As Ac Ad 2d 9c"))
while next_betting_round.which_seat_is_next() != None:
    next_betting_round.player_check_calls(a.players[next_betting_round.which_seat_is_next()])
pot.finish_betting_round()

