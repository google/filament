from setuptools import (
    setup,
    find_packages,
)
import sys

with open("README.md", "r") as fh:
    long_description = fh.read()

setup(
    name="beautifulsoup4",
    # NOTE: We can't import __version__ from bs4 because bs4/__init__.py is Python 2 code,
    # and converting it to Python 3 means going through this code to run 2to3.
    # So we have to specify it twice for the time being.
    version = '4.9.3',
    author="Leonard Richardson",
    author_email='leonardr@segfault.org',
    url="http://www.crummy.com/software/BeautifulSoup/bs4/",
    download_url = "http://www.crummy.com/software/BeautifulSoup/bs4/download/",
    description="Screen-scraping library",
    install_requires=[
        "soupsieve >1.2; python_version>='3.0'",
        "soupsieve >1.2, <2.0; python_version<'3.0'",
    ],
    long_description=long_description,
    long_description_content_type="text/markdown",
    license="MIT",
    packages=find_packages(exclude=['tests*']),
    extras_require = {
        'lxml' : [ 'lxml'],
        'html5lib' : ['html5lib'],
    },
    use_2to3 = True,
    classifiers=["Development Status :: 5 - Production/Stable",
                 "Intended Audience :: Developers",
                 "License :: OSI Approved :: MIT License",
                 "Programming Language :: Python",
                 "Programming Language :: Python :: 2.7",
                 'Programming Language :: Python :: 3',
                 "Topic :: Text Processing :: Markup :: HTML",
                 "Topic :: Text Processing :: Markup :: XML",
                 "Topic :: Text Processing :: Markup :: SGML",
                 "Topic :: Software Development :: Libraries :: Python Modules",
             ],
)
