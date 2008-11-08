#!/usr/bin/env python

# http://snipplr.com/view/7354/get-home-directory-path--in-python-win-lin-other/
import os
userModulePath = os.path.expanduser('~/lib/python')

# http://docs.python.org/install/index.html
# After running:
#	python setup.py build
#	python setup.py install --home=~
import sys
sys.path.append(userModulePath); #This helps Python find the extension in Linux
#But the DLL search path is determined by Windows, not Python
#Most recently,  http://msdn.microsoft.com/en-us/library/ms972822.aspx says:
#"The default behavior now is to look in all the system locations first, then the current directory, and finally any user-defined paths."
#The DLL will be local just be safe. This is a test script anyways.

print "Ready?"

# http://www.python.org/doc/2.5.2/ext/dynamic-linking.html



chip_size = 0.5

import holdem
a = holdem.HoldemTable(chip_size)


# Printing strings: http://www.python.org/doc/2.5.2/tut/node9.html
# Concatenating strings: http://www.skymind.com/~ocrow/python_string/
print ' '.join(['seat_number = ', str(a.seat_number),  'n'])

