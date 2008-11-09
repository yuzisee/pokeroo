#!/usr/bin/python

#from Tkinter import *
# We will be using things from the qt and sys modules
import sys
from PyQt4.QtGui import *
from PyQt4 import QtCore


# Following tutorial from: http://www.zetcode.com/tutorials/pyqt4/
# Perhaps I'll try this: http://doc.trolltech.com/4.2/layout.html
class QuitButtonWindow(QWidget):
	def __init__(self, parent=None):
		QWidget.__init__(self, parent)

		#http://doc.trolltech.com/4.4/qgridlayout.html



		self.setGeometry(300, 300, 250, 150)
		self.setWindowTitle('Icon')

		my_layout = QGridLayout()
		my_layout.setSpacing(5) #Spacing between grid cells is 5

		quit_button = QPushButton('Close')
		quit_button.setGeometry(10, 10, 60, 35)

#Setting styles: http://www.zetcode.com/tutorials/pyqt4/widgets/
		test_label = QLabel('QLabel')
		test_label.setStyleSheet("QWidget { background-color: %s }" % QColor(0, 150, 50).name())

		hello_label = QLabel('Hello')
		hello_label.setStyleSheet("QWidget { background-color: %s }" % QColor(10, 200, 25).name())

		my_layout.addWidget(test_label,0,0,5,4) #Span 5 rows and 4 columns
		my_layout.addWidget(hello_label,7,4) #Take the cell at row 7 column 4
		my_layout.addWidget(quit_button,7,3, 2,2) #Start at row 7, column 3, span 2 rows and 2 columns

		self.setLayout(my_layout)

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


