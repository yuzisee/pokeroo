#!/usr/bin/python

#from Tkinter import *
# We will be using things from the qt and sys modules
import math
import sys
from PyQt4.QtGui import *
from PyQt4 import QtCore


#Docs: http://docs.huihoo.com/pyqt/pyqt4/html/qgridlayout.html
#The PyQt4.QtCore module contains Qt which contains AlignVCenter
#See for yourself!
#>>> dir(QtCore)
#>>> dir(QtCore.Qt)

class DisplayManyLabelsTopRow(QHBoxLayout):
	def __init__(self, num_labels, alignment = Qt.AlignHCenter):
		QHBoxLayout.__init__(self, None)


		for cell_num in range(0,num_labels):
			new_hello_label = QLabel('Hello, from hbox entry ' + str(cell_num) + ' of ' + str(num_labels))
			new_hello_label.setAlignment(alignment)
			new_hello_label.setStyleSheet("QWidget { background-color: %s }" % QColor(30, 90, 5).name())
			self.addWidget(new_hello_label)

#It seems like each spot in a layout can contain either a widget or another layout

class DisplayManyLabelsLayout(QGridLayout):
	NUMBER_OF_LABELS = 13

	def __init__(self, parent=None):
		QGridLayout.__init__(self, parent)

		self.setSpacing(1) #Spacing between grid cells is 1 pixel

		non_corner_labels = self.NUMBER_OF_LABELS - 4

		if non_corner_labels < 0:
			raise NotImplementedError, "Not yet implemented"

		rows = 2 + math.floor(non_corner_labels / 4)
		columns = 2 + math.ceil(non_corner_labels / 4)

		top_bottom_labels = non_corner_labels - (rows-2)*2
		top_labels = math.ceil(top_bottom_labels / 2)
		bottom_labels = math.floor(top_bottom_labels / 2)

#Setting styles: http://www.zetcode.com/tutorials/pyqt4/widgets/

		for row_num in range(1,rows-1):
			print

			new_hello_label = QLabel('Hello, from row ' + str(row_num) + ' column 0')
			new_hello_label.setAlignment(Qt.AlignVCenter | Qt.AlignRight)
			new_hello_label.setStyleSheet("QWidget { background-color: %s }" % QColor(10, 200, 25).name())
			self.addWidget(new_hello_label,row_num,0) #Take the cell at row row_num, column 0

			new_hello_label = QLabel('Hello, from row ' + str(row_num) + ' column 2')
			new_hello_label.setAlignment(Qt.AlignVCenter | Qt.AlignLeft)
			new_hello_label.setStyleSheet("QWidget { background-color: %s }" % QColor(10, 200, 25).name())
			self.addWidget(new_hello_label,row_num,2) #Take the cell at row row_num, column 2

		#Top labels
		self.addLayout(DisplayManyLabelsTopRow(top_labels,Qt.AlignHCenter | Qt.AlignBottom),0,0,1,3) #Span all three columns, and one row at the top

		#Bottom labels
		self.addLayout(DisplayManyLabelsTopRow(bottom_labels,Qt.AlignHCenter | Qt.AlignTop),rows-1,0,1,3) #Span all three columns, and one row at the bottom



		if rows > 2 and columns > 2:
			test_label = QLabel('QLabel')
			test_label.setStyleSheet("QWidget { background-color: %s }" % QColor(0, 150, 50).name())
			self.addWidget(test_label,1,1,rows-2,1) #Span (rows-2) rows and 1 column, starting at row 1, column 1





# Following tutorial from: http://www.zetcode.com/tutorials/pyqt4/
# Perhaps I'll try this: http://doc.trolltech.com/4.2/layout.html
class QuitButtonWindow(QWidget):
	def __init__(self, parent=None):
		QWidget.__init__(self, parent)

		#http://doc.trolltech.com/4.4/qgridlayout.html



		self.setGeometry(300, 300, 250, 150)
		self.setWindowTitle('Icon')



  		self.setLayout(DisplayManyLabelsLayout())


		quit_button = QPushButton('Close',self)
		quit_button.setGeometry(2, 8, 60, 35)


		self.connect(quit_button, QtCore.SIGNAL('clicked()'),
			qApp, QtCore.SLOT('quit()'))


class DeckOfCards:

	def __init__(self):
		x = 5

class PokerGUI:


	def __init__(self, master):
		print "init"

	def betAction(self):
		print "bet"

	def checkAction(self):
		print "check"

	def foldAction(self):
		print "fold"


if __name__ == "__main__":
	app = QApplication(sys.argv)

	qb = QuitButtonWindow()
	qb.show()


	sys.exit(app.exec_())


