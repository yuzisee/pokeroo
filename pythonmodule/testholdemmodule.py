#!/usr/bin/env python

# http://snipplr.com/view/7354/get-home-directory-path--in-python-win-lin-other/
import os
userModulePath = os.path.expanduser('~/lib/python')

# http://docs.python.org/install/index.html
# After running:
#	python setup.py build
#	python setup.py install --home=~
import sys
sys.path.append(userModulePath);

print "Ready?"
print "SetMoney!",

# http://www.python.org/doc/2.5.2/ext/dynamic-linking.html
from holdem import *
print SetMoney()

# Printing strings: http://www.python.org/doc/2.5.2/tut/node9.html
# Concatenating strings: http://www.skymind.com/~ocrow/python_string/
print ' '.join(['GetMoney = ', str(get_money())])

