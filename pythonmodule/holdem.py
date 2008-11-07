from _holdem import *



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
class HoldemArena:
    "Class Interface for the holdem C extension"
    _c_holdem_table = None
    players = []
    _table_initialized = False
    _current_pot = None
    seat_number = 10 #See holdem/src/debug_flags.h, for now SEATS_AT_TABLE is fixed to 10.
    
    valid_bot_types = frozenset(['trap','normal','action','gear','multi', 'danger','space', 'com']);

    def __init__(self, smallest_chip_amount):
        self._c_holdem_table = create_new_table(self.seat_number,  smallest_chip_amount)

    def __del__(self):
        delete_table_and_players(self._c_holdem_table)

    def add_player_clockwise(self,  player_name, starting_money,  bot_type):
        if _table_initialized:
            raise AssertionError,  "The HoldemArena table has already been initialized; add all players before initializing"
        
        if bot_type == None:
            seat_number = add_a_human_opponent(self._c_holdem_table,  player_name,  starting_money)
            player_is_bot = False
        elif bot_type.lower() in valid_bot_types:
            seat_number = add_a_strategy_bot(self._c_holdem_table,  player_name,  starting_money,  bot_type[0])
            player_is_bot = True
        else:
            raise KeyError,  "Create a human player with bot_type == None, or select a bot type from valid_bot_types"
            
        new_player = HoldemArenaPlayer(self._c_holdem_table[0],  seat_number,  player_is_bot)
        players.append(new_player)
        
        if players[seat_number] != new_player:
            raise IndexError, "I've lost track of how many players are at the table!"
        
        return new_player

    def restore_state(self,  state_str):
        if _table_initialized:
            raise AssertionError,  "You must restore the state once instead of calling initialize_table"
        
        _table_initialized = True
        restore_table_state(state_str, self._c_holdem_table)
        
    def initialize_table(self):
        if _table_initialized:
            raise AssertionError,  "The HoldemArena table was already initialized, you can only initialize_table once"
        
        _table_initialized = True
        initialize_new_table_state(self._c_holdem_table)

    def save_state(self):
        if not _table_initialized:
            raise AssertionError,  "You must initialize the table first"
        
        if self._current_pot == None:    
            return save_table_state(self._c_holdem_table)
        else:
            raise AssertionError,  "Finish the playing out the previous pot before starting saving the game state"

    def start_new_pot(self,  small_blind,  next_dealer=-1):
        if not _table_initialized:
            raise AssertionError,  "You must initialize the table first"
            
        if self._current_pot == None:
            self._current_pot = HoldemArenaPot(self._c_holdem_table[0], small_blind,  next_dealer)
            return self._current_pot
        else:
            raise AssertionError,  "Finish the playing out the previous pot before starting a new one"
    
    

#begin_new_hands s#di
#finish_hand_refresh_players s#
class HoldemArenaPot:
    _c_holdem_table_ptr = None
    _bot_shown_hole_cards = []
    _current_event = None
    called_player = -1
    
    def __init__(self, holdem_table_voidptr,  my_small_blind,  my_next_dealer):
        self._c_holdem_table_ptr = holdem_table_voidptr
        begin_new_hands(self._c_holdem_table_ptr,  my_small_blind,  my_next_dealer)
    
    @property
    def money(self):
        """Get the total size of the pot so far."""
        return get_pot_size(self._c_holdem_table_ptr)
    
    
    def size_from_previous_rounds(self):
        """Get the pot size from just after the previous betting round ended."""        
        return get_previous_rounds_pot_size(self._c_holdem_table_ptr)
    
    def start_betting_round(self,  community_cards):
        _bot_shown_hole_cards
        if _current_event == None:
            called_player = -1
            #How do I know who has been dealt their hands?
            #Also, how do I finish_hand_refresh_players?

    def finish_betting_round(self):
        if _current_event == None:
            raise AssertionError, "Start a betting round before finishing one"
            
        self.called_player = _current_event._finish()
        _current_event = None
        return self.called_player

#bot_receives_hole_cards s#i(s#i)
#
#get_money s#i 
#set_money s#id 
#get_current_round_bet s#i 
#get_previous_rounds_bet s#i 
#get_bot_bet_decision s#i 
class HoldemArenaPlayer:
    "Player sitting in one seat of a HoldemArena"
    _c_holdem_table_ptr = None
    is_bot = None
    seat_number = -1

    def __init__(self, holdem_table_voidptr,  my_seat_number,  bot):
        self._c_holdem_table_ptr = holdem_table_voidptr
        self.seat_number = my_seat_number
        self.is_bot = bot

    #http://docs.python.org/library/functions.html?highlight=property

    def receives_hole_cards(self, hole_cards):
        bot_receives_hole_cards(self._c_holdem_table_ptr, self.seat_number, hole_cards._c_holdem_cardset)
        
    def calculate_bet_decision(self):
        return get_bot_bet_decision(self._c_holdem_table_ptr, self.seat_number)
    

    def _getmoney(self):
        return get_money(self._c_holdem_table_ptr, self.seat_number)
    def _setmoney(self,  new_money):
        set_money(self._c_holdem_table_ptr, self.seat_number, new_money)
    money = property(_getmoney, _setmoney)

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
class HoldemArenaCards:
    "Hole cards for a HoldemArenaPlayer or community cards for a HoldemArena "
    _c_holdem_cardset = None
    
    valid_card_ranks = frozenset(['2','3','4','5','6', '7','8', '9', 'T','J', 'Q', 'K', 'A']);
    valid_card_suits = frozenset(['s','h','c','d']);

    def __init__(self):
        self._c_holdem_cardset = create_new_cardset()

    def __del__(self):
        delete_cardset(self._c_holdem_cardset)

    def append_card(self, card_rank, card_suit):
        if card_rank in valid_card_ranks and card_suit in valid_card_suits:
            self._c_holdem_cardset = append_card_to_cardset(self._c_holdem_cardset,  card_rank,  card_suit)
        else:
            raise AssertionError,  "card_rank (resp. card_suit) must be present in valid_card_ranks (resp. valid_card_suits)"

    def append_cards(self,  card_str):
        card_str = card_str.strip()
        while card_str.length() > 0:
            card_rank = card_str[0].upper()
            card_suit = card_str[1].lower()
            append_card(self, card_rank,  card_suit)
            card_str = card_str.strip()


#create_new_betting_round s#(s#i) 
#i: delete_finish_betting_round s# 
#player_makes_bet s#id 
#who_is_next_to_bet s# 
class HoldemArenaBettingRound:
    "Class Interface for betting_round from the holdem C extension"
    _c_betting_round_event = None
    
    def __init__(self, holdem_table_voidptr,  community_cards):
        self._c_betting_round_event = create_new_betting_round(holdem_table_voidptr, community_cards)
    
    def who_is_next(self):
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
class HoldemArenaShowdownRound:
    "Class Interface for showdown_round from the holdem C extension"
    _c_showdown_event = None
    _community_cards = None
    
    def __init__(self, holdem_table_voidptr,  community_cards):
        self._community_cards = community_cards
        self._c_holdem_table_ptr = holdem_table_voidptr
        self._c_betting_round_event = create_new_showdown(holdem_table_voidptr, community_cards)
    
    def who_is_next(self):
        player_to_act = who_is_next_in_showdown(self._c_betting_round_event)
        if player_to_act == -1:
            return None
        else:
            return player_to_act
    
    def show_hand(self,  player,  player_hand):
        player_shows_hand(self._c_betting_round_event, player.seat_number, player_hand, self._community_cards)
    
    def muck_hand(self,  player):
        player_mucks_hand(self._c_betting_round_event, player.seat_number)
            
    def _finish(self, holdem_table_voidptr):
        called_player = delete_finish_showdown(holdem_table_voidptr, self._c_betting_round_event)
        self._c_showdown_event = None
        self._community_cards = None

    def _player_makes_bet(self, seat_number, amount):
        player_makes_bet(self._c_betting_round_event, seat_number, amount)

