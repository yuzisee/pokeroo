from PyQt4.QtGui import *
from PyQt4.QtCore import *

class CardImageDeck(dict):
    def __init__(self, filename):
        cards_image_string = QString.fromAscii(filename)
        cards_image = QImage(cards_image_string)
        for x in range(12):
            for y in range(4):
                card_image = CardImage(cards_image, x, y)
                self[card_image.symbol] = card_image
        card_back_image = CardImage(cards_image, 2, 4) 
        self[card_back_image.symbol] = card_back_image

class CardImage:
    _CARD_WIDTH = 79
    _CARD_HEIGHT = 123
    _IMAGE_RANK_SYMBOLS = ['A','2','3','4','5','6', '7','8', '9', 'T','J', 'Q', 'K']
    _IMAGE_SUIT_SYMBOLS = ['c','d','h','s']
    
    def __init__(self, card_image, x, y):
        self.symbol = 'back' #default symbol is 'back'
        if y < len(self._IMAGE_SUIT_SYMBOLS):
            self.symbol = self._IMAGE_RANK_SYMBOLS[x] + self._IMAGE_SUIT_SYMBOLS[y]
        cards_image = QPixmap.fromImage(card_image)
        card_image_region = QRect(x * self._CARD_WIDTH, y * self._CARD_HEIGHT, self._CARD_WIDTH, self._CARD_HEIGHT)
        self._image = cards_image.copy(card_image_region)
        #MAKE SYMBOL AND IMAGE CLASS ATTRIBUTES
        
    def get_image(self, height):
        return self._image.scaledToHeight(height)