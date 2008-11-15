#!/usr/bin/env python

import os
userModulePath = os.path.expanduser('~/lib/python')

import sys
sys.path.append(userModulePath);

import holdem
import holdemhelpers


print os.path.abspath(os.curdir)

if not os.path.isfile('run/holdemdb.ini'):
    raise RuntimeError, "Get a run/holdemdb.ini to avoid a very long calculation (~45 mins)"

os.chdir('run')


print "BEGIN"

bot_selections = holdem.HoldemTable.VALID_BOT_TYPES #One of each
initial_chips = 100.0
total_chips = len(bot_selections) * initial_chips
chip_size = 0.1

a = holdem.HoldemTable(chip_size)


for bot in bot_selections:
	a.add_player_clockwise(bot + "Bot",initial_chips,bot)


b = holdemhelpers.HoldemDealer()


a.initialize_table()


while len(a.players_with_chips) > 1:

	b.shuffle()
	for p in a.players_with_chips:
		p.hole_cards = holdem.HoldemCards()
		p.hole_cards.append_cards(b.next_card())
		p.hole_cards.append_cards(b.next_card())

	average_stack = total_chips / len(a.players_with_chips)
	small_blind = average_stack / 55

	pot = a.start_new_pot(small_blind)

	# print dir(pot)
	
	# print "Pot size"
	# print pot.money

	# print "Showdown:"
	# print pot.showdown_started

	community = holdem.HoldemCards()

	betting_round = pot.start_betting_round(community)

	while betting_round.which_seat_is_next() != None:
 		betting_round.auto_make_bet(a.players[betting_round.which_seat_is_next()])

	pot.finish_betting_round()


	if pot.more_betting_rounds_remaining > 0:
		community.append_cards(b.next_card())
		community.append_cards(b.next_card())
		community.append_cards(b.next_card())
		betting_round = pot.start_betting_round(community)
		while betting_round.which_seat_is_next() != None:
	 		betting_round.auto_make_bet(a.players[betting_round.which_seat_is_next()])
		pot.finish_betting_round()

#Turn
	if pot.more_betting_rounds_remaining > 0:
		community.append_cards(b.next_card())
		betting_round = pot.start_betting_round(community)
		while betting_round.which_seat_is_next() != None:
	 		betting_round.auto_make_bet(a.players[betting_round.which_seat_is_next()])
		pot.finish_betting_round()

#River
	if pot.more_betting_rounds_remaining > 0:
		community.append_cards(b.next_card())
		betting_round = pot.start_betting_round(community)
		while betting_round.which_seat_is_next() != None:
	 		betting_round.auto_make_bet(a.players[betting_round.which_seat_is_next()])
		pot.finish_betting_round()

	if pot.more_betting_rounds_remaining != None:
		showdown_round = pot.start_showdown(community)
		while showdown_round.which_seat_is_next() != None:
			next_player = a.players[showdown_round.which_seat_is_next()]
	 		showdown_round.show_hand(next_player,next_player.hole_cards)
		pot.finish_showdown()

	a.finish_pot_refresh_players()

	print a.hand_number
	print a.save_state()
	print len(a.players_with_chips),
	print "players left"

