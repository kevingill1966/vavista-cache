
import os, sys
import os.path

from distutils.core import setup
from distutils.command.install_data import install_data
from setuptools import Extension, find_packages

MAJOR_VERSION = '1'
MINOR_VERSION = '0a1'

cache_include='/opt/cache/dev/cpp/include/'
cache_lib='/opt/cache/bin/'

ext = []
data_files=[]

_cache = Extension('vavista.cache._cache',
    define_macros = [('MAJOR_VERSION', MAJOR_VERSION),
                     ('MINOR_VERSION', MINOR_VERSION)],
    include_dirs = ['/usr/include/python2.%d' % sys.version_info[1], cache_include],
    libraries = ['python2.%d' % sys.version_info[1], "cache", "rt", "dl", "c", "m"],
    library_dirs = [cache_lib],
    sources = ['src/vavista/cache/_cache.c'])

ext.append(_cache)

setup(
    name='vavista-cache',
    version='%s.%s' % (MAJOR_VERSION, MINOR_VERSION),
    author='Kevin Gill',
    author_email='kevin.gill@openapp.ie',
    license="TO BE DETERMINED",
    platforms=["linux"],
    url='http://www.python.org/doc/current/ext/building.html',
    description='VAVista Intersystems Cache Interface',
    long_description='''
    This is a wrapper for Cache access. It should provide an indentical
    interface to vavista-gtm.
    ''',
    namespace_packages=['vavista'],
    packages=find_packages('src'),
    package_dir={'': 'src'},
    #data_files=[('vavista/gtm', ['src/vavista/gtm/calltab.ci', 'src/vavista/gtm/vavistagtm.m']),],
    ext_modules=ext,
    include_package_data=True,
    test_suite = "vavista.cache.tests",
    zip_safe=False)

