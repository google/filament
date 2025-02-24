from __future__ import absolute_import
from distutils.core import setup

try:
    from distutils.command.build_py import build_py_2to3 as build_py
except ImportError:
    # 2.x
    from distutils.command.build_py import build_py

setup(name="beautifulsoup4",
      version = "4.3.2",
      author="Leonard Richardson",
      author_email='leonardr@segfault.org',
      url="http://www.crummy.com/software/BeautifulSoup/bs4/",
      download_url = "http://www.crummy.com/software/BeautifulSoup/bs4/download/",
      long_description="""Beautiful Soup sits atop an HTML or XML parser, providing Pythonic idioms for iterating, searching, and modifying the parse tree.""",
      license="MIT",
      packages=['bs4', 'bs4.builder', 'bs4.tests'],
      cmdclass = {'build_py':build_py},
      classifiers=["Development Status :: 4 - Beta",
                   "Intended Audience :: Developers",
                   "License :: OSI Approved :: MIT License",
                   "Programming Language :: Python",
                   'Programming Language :: Python :: 3',
                   "Topic :: Text Processing :: Markup :: HTML",
                   "Topic :: Text Processing :: Markup :: XML",
                   "Topic :: Text Processing :: Markup :: SGML",
                   "Topic :: Software Development :: Libraries :: Python Modules",
                   ],
)
