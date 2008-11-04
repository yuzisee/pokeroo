#!/usr/bin/env python




from distutils.core import setup, Extension
import sys

#Choose (getting MinGW and VCC to work together via DLLs, do we need this? http://www.mingw.org/phpwiki-1.3.14/index.php/MSVC-MinGW-DLL)

packagename = 'Holdem'
extensionname = '_holdem'
sourcefiles = ['holdemmodule.c']

# What OS am I? http://docs.python.org/library/sys.html#sys.platform
if sys.platform[:3] == 'win':
    #But we'll use MinGW anyways and link dynamically
    module1 = Extension(extensionname,
#                extra_objects = ['../holdem/holdemdll/Release/holdemDLL.dll'],  # MinGW would be smart enough to figure this out, but we'll do it the proper way below
                extra_objects = ['../holdem/holdemdll/Release/holdemDLL.lib'],
                sources = sourcefiles)
else:
#Assume Posix
    module1 = Extension(extensionname,
                libraries = ['holdem'],
                library_dirs = ['../holdem/lib'],
                sources = sourcefiles)
	
#Possible options to Extension contstructor: http://docs.python.org/distutils/apiref.html?highlight=extension#distutils.core.Extension


#http://docs.python.org/distutils/setupscript.html?highlight=data_files#installing-additional-files
#import os
#import shutil
#shutil.copy ('../holdem/holdemdll/Release/holdemDLL.dll', 'DLLs/holdemDLL.dll')
					
#Building in Windows: http://boodebr.org/main/python/build-windows-extensions

setup(name=packagename,
      version='0.09',
      description='Python C++ Extension of the Holdem AI Interface',
      author='Joseph Huang',
      author_email='yuzisee@gmail.com',
      url='http://opensvn.csie.org/traccgi/Yuzisee/holdemmodule',
	  data_files = [('.',['../holdem/lib/libholdem.so.1','../holdem/holdemdll/Release/holdemDLL.dll'])],
	  py_modules = ['holdem'],
      ext_modules = [module1])

	  
	  
#vim: set expandtab shiftwidth=4 tabstop=8
