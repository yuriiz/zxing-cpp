import unittest
from os import walk
from os.path import join, splitext

from setuptools import Extension, setup


def test_suite():
    test_loader = unittest.TestLoader()
    test_suite = test_loader.discover('tests')
    return test_suite

setup(name='ZXing',
      version='1.0',
      description='ZXing-cpp for Python',
      author='Iurii Zolotko',
      author_email='yurii.zolotko@gmail.com',
      url='https://github.com/yuriiz/zxing-cpp',
      packages=[],
      test_suite='setup.test_suite',
      ext_modules=[Extension('zxing', [
          'pyzxing.cpp',
          'cli/src/ImageReaderSource.cpp',
          'cli/src/jpgd.cpp',
          'cli/src/lodepng.cpp',
      ] + [
          join(path, f)
          for path, _, files in walk(join('core', 'src'))
          for f in files
          if splitext(f)[-1] in ('.cpp', '.cc')
      ], include_dirs=[join('core', 'src')])]
      )
