from PyQt4.QtGui import *
from PyQt4.QtCore import *

class CardImageDeck():
    _CARD_WIDTH = 79
    _CARD_HEIGHT = 123
    _CARD_SCALE = 0.5
    _IMAGE_RANK_SYMBOLS = ['A','2','3','4','5','6', '7','8', '9', 'T','J', 'Q', 'K']
    _IMAGE_SUIT_SYMBOLS = ['c','d','h','s']
    
    def __init__(self, filename):
        self.card_labels = {}
        cards_image_string = QString.fromAscii(filename)
        cards_image = QImage(cards_image_string)
        for x in range(12):
            for y in range(4):
                symbol = self._IMAGE_RANK_SYMBOLS[x] + self._IMAGE_SUIT_SYMBOLS[y]
                card_image = self._extract_single_card(cards_image, x, y)
                self.card_labels[symbol] = card_image
        card_back_image = self._extract_single_card(cards_image, 2, 4) 
        self.card_labels['back'] = card_back_image

    def get_card_qlabel(self, symbol):
        if symbol in self.card_labels:
            return QLabel(self.card_labels[symbol]._image)
        else:
            return QLabel(symbol);

    
    def _extract_single_card(self, card_image, x, y):
        cards_image = QPixmap.fromImage(card_image)
        #find the cropped area for the card in the image
        card_image_region = QRect(x * self._CARD_WIDTH, y * self._CARD_HEIGHT, self._CARD_WIDTH, self._CARD_HEIGHT)
        #crop the image
        extracted_card_image = cards_image.copy(card_image_region)
        #scale the image
        return extracted_card_image.scaledToHeight(self._CARD_HEIGHT * self._CARD_SCALE)
        #MAKE SYMBOL AND IMAGE CLASS ATTRIBUTES
    