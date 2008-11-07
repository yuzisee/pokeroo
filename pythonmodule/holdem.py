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
#restore_table_state ss# 
#save_table_state s# 
#
#initialize_new_table_state s# 
#begin_new_hands s#di 
#bot_receives_hole_cards s#i(s#i) 
#finish_hand_refresh_players s# 
class HoldemArena:
    "Class Interface for the holdem C extension"
    _c_holdem_table = None
    _table_initialized = False
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
            add_a_human_opponent(self._c_holdem_table,  player_name,  starting_money)
        elif bot_type.lower() in valid_bot_types:
            add_a_strategy_bot(self._c_holdem_table,  player_name,  starting_money,  bot_type[0])
        else:
            raise KeyError,  "Create a human player with bot_type == None, or select a bot type from valid_bot_types"

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
        return save_table_state(self._c_holdem_table)

    def player(self,  seat_number):
        return HoldemArenaPlayer(self,  seat_number)
    
    @property
    def pot(self):
        """Get the total size of the pot so far."""
        return get_pot_size(self._c_holdem_table[0])
    
    @property
    def bet_to_call(self):
        """Get the largest bet so far this round."""
        return get_bet_to_call(self._c_holdem_table[0])
        
    def get_previous_rounds_pot(self):
        """Get the pot size from just after the previous betting round ended."""
        return get_previous_rounds_pot_size(self._c_holdem_table[0])
        
#get_money s#i 
#set_money s#id 
#get_current_round_bet s#i 
#get_previous_rounds_bet s#i 
#get_bot_bet_decision s#i 
class HoldemArenaPlayer:
    "Player sitting in one seat of a HoldemArena"
    _arena = None
    seat_number = -1

    def __init__(self, my_arena,  my_seat_number):
        self._arena = my_arena
        self.seat_number = my_seat_number

    #http://docs.python.org/library/functions.html?highlight=property

        
    def calculate_bet_decision(self):
        return get_bot_bet_decision(_arena._c_holdem_table[0], self.seat_number)
    

    def _getmoney(self):
        return get_money(self._arena._c_holdem_table[0], self.seat_number)
    def _setmoney(self,  new_money):
        set_money(self._arena._c_holdem_table[0], self.seat_number, new_money)
    money = property(_getmoney, _setmoney)

    @property
    def current_round_bet(self):
        """Get the current bet that the player has made this round."""
        return get_current_round_bet(self._arena._c_holdem_table[0], self.seat_number)

    @property
    def previous_rounds_bet(self):
        """Get the total bets that the player has made in all previous rounds."""
        return get_previous_rounds_bet(self._arena._c_holdem_table[0], self.seat_number)


#create_new_cardset
#append_card_to_cardset (s#i)cc 
#delete_cardset (s#i) 
class HoldemArenaCards:
    "Hole cards for a HoldemArenaPlayer or community cards for a HoldemArena "
    _c_holdem_cardset = None

    def __init__(self):
        self._c_holdem_cardset = create_new_cardset()

    def __del__(self):
        delete_cardset(self._c_holdem_cardset)

    def append_card(self,  card_str):
        self._c_holdem_cardset = append_card_to_cardset(self._c_holdem_cardset,  card_str[0],  card_str[1])


#create_new_betting_round s#(s#i) 
#i: delete_finish_betting_round s# 
#player_makes_bet s#id 
#who_is_next_to_bet s# 
class HoldemArenaBettingRound:
    "Class Interface for betting_round from the holdem C extension"
    _c_betting_round_event


#who_is_next_in_showdown s# 
#player_shows_hand s#i(s#i)(s#i) 
#player_mucks_hand s#i 
#create_new_showdown s#i(s#i)(s#i) 
#delete_finish_showdown s#s# 
class HoldemArenaShowdownRound:
    "Class Interface for showdown_round from the holdem C extension"
    _c_showdown_event
    


