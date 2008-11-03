#!/usr/bin/env python




from distutils.core import setup, Extension
import sys

#Choose (getting MinGW and VCC to work together via DLLs, do we need this? http://www.mingw.org/phpwiki-1.3.14/index.php/MSVC-MinGW-DLL)
linkstatically = False
linkdynamically = True


if (linkstatically and linkdynamically) or (not linkstatically and not linkdynamically):
	raise Exception,  "Link dynamically or statically?? Choose only one please"

packagename = 'Holdem'
extensionname = '_holdem'
sourcefiles = ['holdemmodule.c']

# What OS am I? http://docs.python.org/library/sys.html#sys.platform
if sys.platform[:3] == 'win':
    #But we'll use MinGW anyways.
    if linkstatically:
        raise Exception,  "Not sure where VC++ static objects are yet.\Try setting linkdynamically=True at the top of this file."
    elif linkdynamically:
        module1 = Extension(extensionname,
#                    extra_objects = ['../holdem/holdemdll/Release/holdemDLL.dll'],  # MinGW is smart enough to figure this out
                    extra_objects = ['../holdem/holdemdll/Release/holdemDLL.lib'],
                    sources = sourcefiles)
else:
#Assume Posix
    if linkstatically:
        module1 = Extension(extensionname,
                    extra_objects = ['../lib/holdem.a'],
                    sources = sourcefiles)
    elif linkdynamically:
        module1 = Extension(extensionname,
                    libraries = ['holdem.so'],
                    library_dirs = ['../lib'],
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
	  data_files = [('.',['../holdem/holdemdll/Release/holdemDLL.dll'])],
#	  py_modules = ['holdem'],
      ext_modules = [module1])

	  
	  
#vim: set expandtab shiftwidth=4 tabstop=8
