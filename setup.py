#!/usr/bin/env python3
#-*- coding:utf-8 -*-

package = 'lunar'
version = '0.2'

import sys
from distutils.core import setup
from distutils.extension import Extension

cmdclass = {}

source_files = ['src/jlunar.cpp', 'src/jlunarJQ.cpp', 'src/pylunar.cpp']

if sys.version_info < (3, 9):
	print("\033[31mError: Python version must greater than or equal to 3.9\033[0m")
	exit(-1)

#安装
setup(
	name="lunar",
	version="0.2",
	description="Python bindings for lunar",
	author="陈逸少",
	author_email="jmchxy@gmail.com",
	url="https://github.com/xyzchen/pylunar",
	download_url="https://github.com/xyzchen/pylunar/archive/master.zip",
	cmdclass=cmdclass,
	ext_modules=[Extension("lunar", source_files)]
)
