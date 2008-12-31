#!/usr/bin/python


# We will be using things from the qt and sys modules
import math
import sys
from CardImagesQt import *
from PyQt4.QtGui import *
from PyQt4 import QtCore
from PyQt4.QtCore import Qt


#Docs: http://docs.huihoo.com/pyqt/pyqt4/html/qgridlayout.html
#The PyQt4.QtCore module contains Qt which contains AlignVCenter
#See for yourself!
#>>> dir(QtCore)
#>>> dir(QtCore.Qt)

class DisplayInfo(QVBoxLayout):
	def __init__(self, label_image = None, alignment = Qt.Alignment):
		QTextEdit.__init__(self,None)

		new_hello_label = QLabel('No card image')
		if label_image != None:
			pixmap = label_image.get_image(62)
			new_hello_label.setPixmap(pixmap)

		#new_hello_label.setAlignment(alignment)
		new_hello_label.setStyleSheet("QWidget { background-color: %s }" % QColor(165, 250, 225).name())
		self.addWidget(new_hello_label)
		self.addWidget(QLabel('bet size: 20'))
		#new_line_edit = QLineEdit('00000')
		#self.addWidget(new_line_edit)



class DisplayManyLabelsTopRow(QHBoxLayout):
	def __init__(self, num_labels, alignment = Qt.Alignment):
		QHBoxLayout.__init__(self, None)


		for cell_num in range(0,num_labels):
			self.addLayout(DisplayInfo('Hello, from hbox entry '+str(cell_num)+' of '+str(num_labels),alignment))


#It seems like each spot in a layout can contain either a widget or another layout

class EllipseLayout(QLayout):
	
	def addItem(self, item):
		if not isinstance(item,QLayoutItem):
			raise TypeError, 'This exception is not object oriented, but Ruby is, so use addWidget'
		if len(self._item_list) >= self._max_items:
			print 'too many items'
			return
		self._item_list.append(item)
	
	def sizeHint(self):
		return 1
	
	def setGeometry(self, q_rect):
		if len(self._item_list) == 0:
			return
		num_items = len(self._item_list)
		item_spacing = (2 * math.pi) / num_items  #in radians
		for index in range(num_items):
			mysize = QRect(100+100* math.sin(index * item_spacing), 100+100*math.cos(index * item_spacing), 100, 200);
			self._item_list[index].setGeometry(mysize)
			print repr(mysize.getCoords())
		
	def itemAt(self, index):
		if index < len(self._item_list):
			return self._item_list[index]
		else:
			return None
	
	def count(self):
		return len(self._item_list)
	
	def takeAt(self, index):
		return self._item_list.pop(index)
	#addItem(), sizeHint(), setGeometry(), itemAt() and takeAt(). You should also implement minimumSize() to e
	
	def expandingDirections(self):
		return 0

	def __init__(self, parent=None, max_items = 13):
		super(EllipseLayout, self).__init__(parent)
		self._item_list = []
		self._max_items = max_items

		#Center green

		#test_label = QLabel('QLabel')
		#test_label.setStyleSheet("QWidget { background-color: %s }" % QColor(0, 150, 50).name())
		#self.addWidget(test_label) #Span (rows-2) rows and 1 column, starting at row 1, column 1





# Following tutorial from: http://www.zetcode.com/tutorials/pyqt4/
# Perhaps I'll try this: http://doc.trolltech.com/4.2/layout.html
class QuitButtonWindow(QWidget):
	def __init__(self, parent=None):
		
		QWidget.__init__(self, parent)
		card_image_deck = CardImageDeck('cards.png')

		#http://doc.trolltech.com/4.4/qgridlayout.html



		self.setGeometry(300, 300, 250, 150)
		self.setWindowTitle('Icon')



		#self.setLayout(DisplayManyLabelsLayout())
		card_table = EllipseLayout()
		hoo = QLabel('HOO',self)
		hoo1 = QLabel('HOO1',self)
		hoo2 = QLabel('HOO2',self)
		hoo3 = QLabel('HOO3',self)
		
		self.setLayout(card_table)
		card_table.addWidget(hoo)#DisplayInfo(card_image_deck['back']))
		card_table.addWidget(hoo1)
		card_table.addWidget(hoo2)
		card_table.addWidget(hoo3)

		quit_button = QPushButton('Close',self)
		quit_button.setGeometry(180, 8, 60, 35)


		self.connect(quit_button, QtCore.SIGNAL('clicked()'),
			qApp, QtCore.SLOT('quit()'))


if __name__ == "__main__":
	app = QApplication(sys.argv)

	qb = QuitButtonWindow()
	qb.show()


	sys.exit(app.exec_())


