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
	def __init__(self, text, alignment = Qt.Alignment):
		card_image_deck = CardImageDeck.get_instance(self)
		QTextEdit.__init__(self,None)

		new_hello_label = QLabel(text)
		new_hello_label.setAlignment(alignment)
		new_hello_label.setStyleSheet("QWidget { background-color: %s }" % QColor(165, 250, 225).name())
		self.addWidget(new_hello_label)

		#new_line_edit = QLineEdit('00000')
		#self.addWidget(new_line_edit)



class DisplayManyLabelsTopRow(QHBoxLayout):
	def __init__(self, num_labels, alignment = Qt.Alignment):
		QHBoxLayout.__init__(self, None)


		for cell_num in range(0,num_labels):
			self.addLayout(DisplayInfo('Hello, from hbox entry '+str(cell_num)+' of '+str(num_labels),alignment))


#It seems like each spot in a layout can contain either a widget or another layout

class DisplayManyLabelsLayout(QGridLayout):
	NUMBER_OF_LABELS = 13

	def __init__(self, parent=None):
		QGridLayout.__init__(self, parent)

		self.setSpacing(1) #Spacing between grid cells is 1 pixel

		non_corner_labels = self.NUMBER_OF_LABELS - 4

		if non_corner_labels < 0:
			raise NotImplementedError, "Not yet implemented"

		inner_rows = int(non_corner_labels) / 4
		columns = 2 + int(non_corner_labels) / 4 + 1

		top_bottom_labels = self.NUMBER_OF_LABELS - inner_rows*2
		top_labels = top_bottom_labels / 2 + 1
		bottom_labels = top_bottom_labels / 2

#Setting styles: http://www.zetcode.com/tutorials/pyqt4/widgets/

		for row_num in range(1,inner_rows+1):
			self.addLayout(DisplayInfo('Hello, from row '+str(row_num)+' column 0',Qt.AlignVCenter | Qt.AlignLeft),row_num,0) #Take the cell at row row_num, column 0
			self.addLayout(DisplayInfo('Hello, from row '+str(row_num)+' column 2',Qt.AlignVCenter | Qt.AlignRight),row_num,columns-1) #Take the cell at row row_num, column (columns-1)


		#Top labels
		self.addLayout(DisplayManyLabelsTopRow(top_labels,Qt.AlignHCenter | Qt.AlignBottom),0,0,1,columns) #Span all columns, and one row at the top

		#Bottom labels
		self.addLayout(DisplayManyLabelsTopRow(bottom_labels,Qt.AlignHCenter | Qt.AlignTop),inner_rows+1,0,1,columns) #Span all columns, and one row at the bottom


		#Center green
		if inner_rows > 0 and columns > 2:
			test_label = QLabel('QLabel')
			test_label.setStyleSheet("QWidget { background-color: %s }" % QColor(0, 150, 50).name())
			self.addWidget(test_label,1,1,inner_rows,columns-2) #Span (rows-2) rows and 1 column, starting at row 1, column 1





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


if __name__ == "__main__":
	app = QApplication(sys.argv)

	qb = QuitButtonWindow()
	qb.show()


	sys.exit(app.exec_())


