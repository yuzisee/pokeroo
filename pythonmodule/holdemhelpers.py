
import holdem
import random

class HoldemDealer(object):
	def __init__(self):
		self.cardlist = []
		self.cards_dealt_before_shuffling = 0
		for r in holdem.Cards.CARD_RANKS:
			for s in holdem.Cards.CARD_SUITS:
				self.cardlist.append(r + s)

	def shuffle(self):
		self.cards_dealt_before_shuffling = 0
		card_count = len(self.cardlist)
		# Swap randomly selected card i with randomly selected card j
		for i in range(card_count):
			j = random.randrange(i, card_count)
			[self.cardlist[i], self.cardlist[j]] = [self.cardlist[j], self.cardlist[i]]

	def next_card(self):
		if self.cards_dealt_before_shuffling >= len(self.cardlist):
			return None
		else:
			c = self.cardlist.pop(0)
			self.cardlist.append(c)
			self.cards_dealt_before_shuffling += 1
			return c

