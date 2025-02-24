from importlib import import_module

from setuptools import setup, find_packages

setup(
    name = 'ijson',
    version = import_module('ijson').__version__,
    author = 'Ivan Sagalaev',
    author_email = 'maniac@softwaremaniacs.org',
    url = 'https://github.com/isagalaev/ijson',
    license = 'BSD',
    description = 'Iterative JSON parser with a standard Python iterator interface',
    long_description = open('README.rst').read(),

    classifiers = [
        'Development Status :: 5 - Production/Stable',
        'License :: OSI Approved :: BSD License',
        'Programming Language :: Python :: 2',
        'Programming Language :: Python :: 3',
        'Topic :: Software Development :: Libraries :: Python Modules',
    ],

    packages = find_packages(),
)
