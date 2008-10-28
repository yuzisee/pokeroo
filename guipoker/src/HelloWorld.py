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

		self.setGeometry(300, 300, 250, 150)
		self.setWindowTitle('Icon')

		quit = QPushButton('Close', self)
		quit.setGeometry(10, 10, 60, 35)

		self.connect(quit, QtCore.SIGNAL('clicked()'),
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


