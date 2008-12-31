import Image
import ImageQt
from PyQt4.QtGui import *

class CardImageDeck(dict):
    _deck_instance = None
    
    def get_instance(self):
        if _deck_instance == None:
            _deck_instance = CardImageDeck()
        return _deck_instance
    
    _CARD_LENGTH = 123
    _CARD_WIDTH = 79
    _CARDS_IMAGE = Image.open('cards.png')
    _IMAGE_RANK_SYMBOL = ['A','2','3','4','5','6', '7','8', '9', 'T','J', 'Q', 'K']
    _IMAGE_SUIT_SYMBOL = ['c','d','h','s']

    def __init__(self):
        for x in range(12):
            for y in range(4):
                card_image_region = (x * _CARD_LENGTH, (x + 1) * _CARD_LENGTH, y * _CARD_LENGTH, (y + 1) * _CARD_LENGTH)
                cropped_card_image = _CARDS_IMAGE.crop(card_image_region)
                card_image = CardImage(cropped_card_image)
                self[self._card_symbol(x, y)] = card_image
        card_back_image_region = (2 * _CARD_LENGTH, 3 * _CARD_LENGTH, 4 * _CARD_LENGTH, 5 * _CARD_LENGTH)
        self['back'] = CardImage(card_back_image_region)
        
    def _card_symbol(self, rank, suit):
        return _IMAGE_RANK_SYMBOL[rank] + _IMAGE_SUIT_SYMBOL[suit]

class CardImage(QPixmap):
    def __init__(self, card_image):
        self.fromImage(ImageQt.ImageQt(card_image))