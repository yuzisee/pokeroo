from holdem import *

class HoldemArena:
    "Class Interface for the holdem C extension"
    numberOfSeats = 0
    HoldemArenaCptr = ''

    def __init__(self, seats):
        self.numberOfSeats = seats

    def GetBigBlind(self):
        return 'hello world'

    def AddPlayerClockwise(self):
	return HoldemArenaAddHuman()

    HoldemArenaBotStyles = set(['Trap','Normal','Action','Gear','Multi']);

    def AddBotClockwise(self, style):
	return HoldemArenaBotHuman(style)

class HoldemArenaPlayer:
    "Player sitting in one seat of a HoldemArena"
    HoldemArenaPlayerCptr = ''
    HoldemArenaPlayerStrategyCptr = ''

    def IsBot(self)
        return HoldemArenaPlayerCptr != '' && HoldemArenaPlayerStrategyCptr == ''

#    def GetSeatNumber(self):
