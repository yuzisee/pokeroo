from _holdem import *


#Class attributes are all static: http://diveintopython.org/object_oriented_framework/class_attributes.html

#types: http://docs.python.org/library/stdtypes.html


#create_new_table id
#delete_table_and_players s#i
#add_a_human_opponent (s#i)sd
#add_a_strategy_bot (s#i)sdc
#
#get_pot_size s#
#get_previous_rounds_pot_size s#
#get_bet_to_call s#
#
#initialize_new_table_state s#
#restore_table_state ss#
#save_table_state s#
class HoldemTable(object):
    "Class Interface for the holdem C extension"
    VALID_BOT_TYPES = frozenset(['trap','normal','action','gear','multi', 'danger','space', 'com']);

    def __init__(self, smallest_chip_amount):
        self.seat_number = 10 #See holdem/src/debug_flags.h, for now SEATS_AT_TABLE is fixed to 10.
        self.players = []
        self._table_initialized = False
        self._current_pot = None
        self._c_holdem_table = create_new_table(self.seat_number,  smallest_chip_amount)

    def __del__(self):
        if self._current_pot != None:
            self._current_pot._finish() #The program is likely exiting, let's try to free whatever we can

        delete_table_and_players(self._c_holdem_table)

    def add_player_clockwise(self,  player_name, starting_money,  bot_type):
        if self._table_initialized:
            raise AssertionError,  "The HoldemArena table has already been initialized; add all players before initializing"

        if bot_type == None:
            seat_number = add_a_human_opponent(self._c_holdem_table,  player_name,  starting_money)
            player_is_bot = False
        elif bot_type.lower() in self.VALID_BOT_TYPES:
            seat_number = add_a_strategy_bot(self._c_holdem_table,  player_name,  starting_money,  bot_type[0].upper())
            player_is_bot = True
        else:
            raise KeyError,  "Create a human player with bot_type == None, or select a bot type from self.VALID_BOT_TYPES"

        new_player = HoldemPlayer(self._c_holdem_table[0],  seat_number,  player_is_bot)
        self.players.append(new_player)

        if self.players[seat_number] != new_player:
            raise IndexError, "I've lost track of how many players are at the table!"

        return new_player

    def restore_state(self,  state_str):
        if self._table_initialized:
            raise AssertionError,  "You must restore the state once instead of calling initialize_table"

        self._table_initialized = True
        restore_table_state(state_str, self._c_holdem_table)

    def initialize_table(self):
        if self._table_initialized:
            raise AssertionError,  "The HoldemArena table was already initialized, you can only initialize_table once"

        self._table_initialized = True
        initialize_new_table_state(self._c_holdem_table[0])

    def save_state(self):
        if not self._table_initialized:
            raise AssertionError,  "You must initialize the table first"

        if self._current_pot != None:
            raise AssertionError,  "Finish the playing out the previous pot before starting saving the game state"

        return save_table_state(self._c_holdem_table[0])



    def start_new_pot(self, small_blind, override_next_dealer=None):
        if not self._table_initialized:
            raise AssertionError,  "You must initialize the table first"

        if self._current_pot != None:
            raise AssertionError,  "Finish the playing out the previous pot before starting a new one"

        if override_next_dealer == None:
            next_dealer = -1
        elif override_next_dealer in players:
            next_dealer = override_next_dealer.seat_number
        else:
            raise ValueError,  "override_next_dealer must be a valid player"

        bots = []
        for bot in self.players:
            if bot.is_bot:
                if bot.hole_cards:
                    bots.append(bot)
                else:
                    raise AssertionError, "All bots must be given their hole cards before starting the pot"

        self._current_pot = HoldemPot(self._c_holdem_table[0], small_blind,  next_dealer)
        #Instantiating a HoldemPot also calls begin_new_hands
        for bot in bots:
            bot._process_hole_cards()

        return self._current_pot



    def finish_pot_refresh_players(self):
        if self._current_pot == None:
            raise AssertionError, "Start a new pot; when all betting/showdown rounds have completed, then call this function"

        if not self._current_pot.is_finished():
            raise AssertionError, "Wait for the final betting round or showdown to complete before refreshing players"

        self._current_pot._finish()
        self._current_pot = None

        for bot in players:
            if bot.is_bot:
                bot.hole_cards = None


#begin_new_hands s#di
#finish_hand_refresh_players s#
class HoldemPot(object):

    def __init__(self, holdem_table_voidptr,  my_small_blind,  my_next_dealer):
        self._current_event = None
        self._showdown_started = False
        self._called_player = None #initialized to none, set to -1 when the pot is over
        self._c_holdem_table_ptr = holdem_table_voidptr
        begin_new_hands(self._c_holdem_table_ptr,  my_small_blind,  my_next_dealer)

    @property
    def money(self):
        """Get the total size of the pot so far."""
        return get_pot_size(self._c_holdem_table_ptr)


    def size_from_previous_rounds(self):
        """Get the pot size from just after the previous betting round ended."""
        return get_previous_rounds_pot_size(self._c_holdem_table_ptr)

    def start_betting_round(self, community_cards):
        """Returns a HoldemBettingRound object"""
        if self._showdown_started:
            raise AssertionError, "Betting rounds can't be started once the showdown begins"

        if self._current_event != None:
            raise AssertionError, "Betting round already started"

        if self._called_player == -1:
            raise AssertionError, "All players folded in the last betting round"

        self._current_event = HoldemBettingRound(self._c_holdem_table_ptr,community_cards)
        return self._current_event

    def finish_betting_round(self):
        if self._current_event == None:
            raise AssertionError, "Start a betting round before finishing one"

        if self._showdown_started:
            raise AssertionError, "There is no betting round to finish"

        if self._current_event.which_seat_is_next() != None:
            raise AssertionError, "Wait for all bets to be made and which_seat_is_next() == None"

        self._called_player = self._current_event._finish()
        self._current_event = None
        if self._called_player == -1:
            return None
        else:
            return self.called_player

    def start_showdown(self):
        """Returns a HoldemShowdownRound object"""
        if self._showdown_started:
            raise AssertionError, "Showdown already started"

        if self._called_player == -1:
            raise AssertionError, "All players have already folded"

        self._current_event = HoldemShowdownRound(self._c_holdem_table_ptr, self._called_player)
        return self._current_event

    def finish_showdown(self):
        if not self._showdown_started:
            raise AssertionError, "Showdown not yet started"

        if not self._current_event:
            raise AssertionError, "No showdown taking place"

        if self._current_event.which_seat_is_next() != None:
            raise AssertionError, "Wait for all players to act and which_seat_is_next() == None"

        self._current_event._finish()
        self._current_event = None
        self._called_player = -1

    def is_finished(self):
        return (self._called_player == -1)

    def _finish(self):
        if self._current_event != None:
            #The program is likely exiting, let's try to free whatever we can
            self._current_event._finish()
            betting_round_started_before = True
        elif self._called_player == None:
            #We've never finished a betting round, and we aren't in one!
            #And again, the program is likely exiting.
            betting_round_started_before = False
        else:
            betting_round_started_before = True

        if betting_round_started_before:
            finish_hand_refresh_players(self._c_holdem_table_ptr)

        self._c_holdem_table_ptr = None

#bot_receives_hole_cards s#i(s#i)
#
#get_money s#i
#set_money s#id
#get_current_round_bet s#i
#get_previous_rounds_bet s#i
#get_bot_bet_decision s#i
class HoldemPlayer(object):
    "Player sitting in one seat of a HoldemArena"

    def __init__(self, holdem_table_voidptr,  my_seat_number,  bot):
        self._hole_cards = None
        self._c_holdem_table_ptr = holdem_table_voidptr
        self.seat_number = my_seat_number
        self._is_bot = bot

    #http://docs.python.org/library/functions.html?highlight=property

    def _process_hole_cards(self):
        bot_receives_hole_cards(self._c_holdem_table_ptr, self.seat_number, self.hole_cards.c_ptr)

    def calculate_bet_decision(self):
        return get_bot_bet_decision(self._c_holdem_table_ptr, self.seat_number)


    def _getholecards(self):
        if not self.is_bot:
            raise AssertionError, "Hole cards are only assigned to bots"
        else:
            return self._hole_cards
    def _setholecards(self,  cards):
        if not self.is_bot:
            raise AssertionError, "Hole cards are only assigned to bots"
        else:
            self._hole_cards = cards
    hole_cards = property(_getholecards, _setholecards)

    def _getmoney(self):
        return get_money(self._c_holdem_table_ptr, self.seat_number)
    def _setmoney(self,  new_money):
        set_money(self._c_holdem_table_ptr, self.seat_number, new_money)
    money = property(_getmoney, _setmoney)

    @property
    def is_bot(self):
        return self._is_bot

    @property
    def current_round_bet(self):
        """Get the current bet that the player has made this round."""
        return get_current_round_bet(self._c_holdem_table_ptr, self.seat_number)

    @property
    def previous_rounds_bet(self):
        """Get the total bets that the player has made in all previous rounds."""
        return get_previous_rounds_bet(self._c_holdem_table_ptr, self.seat_number)


#create_new_cardset
#append_card_to_cardset (s#i)cc
#delete_cardset (s#i)
class HoldemCards(object):
    "Hole cards for a HoldemPlayer or community cards for a HoldemArena "

    VALID_CARD_RANKS = frozenset(['2','3','4','5','6', '7','8', '9', 'T','J', 'Q', 'K', 'A']);
    VALID_CARD_SUITS = frozenset(['s','h','c','d']);

    def __init__(self):
        self._c_holdem_cardset = create_new_cardset()

    def __del__(self):
        delete_cardset(self.c_ptr)

    def _valid_card_pair(self,card_rank,card_suit):
        return (card_rank in self.VALID_CARD_RANKS and card_suit in self.VALID_CARD_SUITS)

    def append_card(self, card_rank, card_suit):
        if not self._valid_card_pair(card_rank, card_suit):
            raise AssertionError,  "card_rank (resp. card_suit) must be present in VALID_CARD_RANKS (resp. VALID_CARD_SUITS)"

        self._c_holdem_cardset = append_card_to_cardset(self.c_ptr,  card_rank,  card_suit)
        return self

    def append_cards(self,  cards_str):
        for next_card in cards_str.strip().split():
            #determine order
            if not self._valid_card_pair(next_card[0],next_card[1]):
                next_card.reverse()

            #append cards
            self.append_card(next_card[0], next_card[1])

        return self

    @property
    def c_ptr(self):
        return self._c_holdem_cardset

#create_new_betting_round s#(s#i)
#i: delete_finish_betting_round s#
#player_makes_bet s#id
#who_is_next_to_bet s#
class HoldemBettingRound(object):
    "Class Interface for betting_round from the holdem C extension"
    _c_betting_round_event = None

    def __init__(self, holdem_table_voidptr,  community_cards):
        self._c_betting_round_event = create_new_betting_round(holdem_table_voidptr, community_cards.c_ptr)

    def which_seat_is_next(self):
        return who_is_next_to_bet(self._c_betting_round_event)

    def raise_by(self,  player,  amount):
        _player_makes_bet(player.seat_number,  amount + bet_to_call())

    def raise_to(self,  player,  amount):
        _player_makes_bet(player.seat_number,  amount)

    def check_call(self,  player):
        _player_makes_bet(player.seat_number,  bet_to_call())

    def check_fold(self,  player):
        _player_makes_bet(player.seat_number,  0)

    def fold(self,  player):
        _player_makes_bet(player.seat_number,  -1)

    @property
    def bet_to_call(self):
        """Get the largest bet so far this round."""
        return get_bet_to_call(self._c_holdem_table_ptr)

    def _finish(self):
        called_player = delete_finish_betting_round(self._c_betting_round_event)
        self._c_betting_round_event = None
        return called_player

    def _player_makes_bet(self, seat_number, amount):
        player_makes_bet(self._c_betting_round_event, seat_number, amount)


#who_is_next_in_showdown s#
#player_shows_hand s#i(s#i)(s#i)
#player_mucks_hand s#i
#create_new_showdown s#i(s#i)
#delete_finish_showdown s#s#
class HoldemShowdownRound(object):
    "Class Interface for showdown_round from the holdem C extension"
    _c_showdown_event = None
    _community_cards = None

    def __init__(self, holdem_table_voidptr, called_player, community_cards):
        self._community_cards = community_cards
        self._c_holdem_table_ptr = holdem_table_voidptr
        self._c_betting_round_event = create_new_showdown(holdem_table_voidptr, called_player, community_cards.c_ptr)

    def which_seat_is_next(self):
        player_to_act = who_is_next_in_showdown(self._c_betting_round_event)
        if player_to_act == -1:
            return None
        else:
            return player_to_act

    #Bots can always just show their hand. Internally the table will auto-muck when appropriate.
    def show_hand(self,  player,  player_hand):
        player_shows_hand(self._c_betting_round_event, player.seat_number, player_hand, self._community_cards.c_ptr)

    def muck_hand(self,  player):
        player_mucks_hand(self._c_betting_round_event, player.seat_number)

    def _finish(self, holdem_table_voidptr):
        called_player = delete_finish_showdown(holdem_table_voidptr, self._c_betting_round_event)
        self._c_showdown_event = None
        self._community_cards = None


