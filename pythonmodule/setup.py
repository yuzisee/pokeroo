#!/usr/bin/env python

from distutils.core import setup, Extension

module1 = Extension('holdem',
#		    include_dirs = ['/usr/local/include'],
#                    libraries = ['tcl83'],
#                    library_dirs = ['/usr/local/lib'],
                    sources = ['holdemmodule.c'])


setup(name='Holdem',
      version='0.09',
      description='Python C++ Extension of the Holdem AI Interface',
      author='Joseph Huang',
      author_email='yuzisee@gmail.com',
      url='http://opensvn.csie.org/traccgi/Yuzisee/holdemmodule',
      ext_modules = [module1])

#vim: set expandtab shiftwidth=4 tabstop=8
