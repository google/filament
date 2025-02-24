Beautiful Soup is a library that makes it easy to scrape information
from web pages. It sits atop an HTML or XML parser, providing Pythonic
idioms for iterating, searching, and modifying the parse tree.

# Quick start

```
>>> from bs4 import BeautifulSoup
>>> soup = BeautifulSoup("<p>Some<b>bad<i>HTML")
>>> print(soup.prettify())
<html>
 <body>
  <p>
   Some
   <b>
    bad
    <i>
     HTML
    </i>
   </b>
  </p>
 </body>
</html>
>>> soup.find(text="bad")
'bad'
>>> soup.i
<i>HTML</i>
#
>>> soup = BeautifulSoup("<tag1>Some<tag2/>bad<tag3>XML", "xml")
#
>>> print(soup.prettify())
<?xml version="1.0" encoding="utf-8"?>
<tag1>
 Some
 <tag2/>
 bad
 <tag3>
  XML
 </tag3>
</tag1>
```

To go beyond the basics, [comprehensive documentation is available](http://www.crummy.com/software/BeautifulSoup/bs4/doc/).

# Links

* [Homepage](http://www.crummy.com/software/BeautifulSoup/bs4/)
* [Documentation](http://www.crummy.com/software/BeautifulSoup/bs4/doc/)
* [Discussion group](http://groups.google.com/group/beautifulsoup/)
* [Development](https://code.launchpad.net/beautifulsoup/)
* [Bug tracker](https://bugs.launchpad.net/beautifulsoup/)
* [Complete changelog](https://bazaar.launchpad.net/~leonardr/beautifulsoup/bs4/view/head:/CHANGELOG)

# Note on Python 2 sunsetting

Since 2012, Beautiful Soup has been developed as a Python 2 library
which is automatically converted to Python 3 code as necessary. This
makes it impossible to take advantage of some features of Python
3.

For this reason, I plan to discontinue Beautiful Soup's Python 2
support at some point after December 31, 2020: one year after the
sunset date for Python 2 itself. Beyond that point, new Beautiful Soup
development will exclusively target Python 3. Of course, older
releases of Beautiful Soup, which support both versions, will continue
to be available.

# Supporting the project

If you use Beautiful Soup as part of your professional work, please consider a
[Tidelift subscription](https://tidelift.com/subscription/pkg/pypi-beautifulsoup4?utm_source=pypi-beautifulsoup4&utm_medium=referral&utm_campaign=readme).
This will support many of the free software projects your organization
depends on, not just Beautiful Soup.

If you use Beautiful Soup for personal projects, the best way to say
thank you is to read
[Tool Safety](https://www.crummy.com/software/BeautifulSoup/zine/), a zine I
wrote about what Beautiful Soup has taught me about software
development.

# Building the documentation

The bs4/doc/ directory contains full documentation in Sphinx
format. Run `make html` in that directory to create HTML
documentation.

# Running the unit tests

Beautiful Soup supports unit test discovery from the project root directory:

```
$ nosetests
```

```
$ python -m unittest discover -s bs4
```

If you checked out the source tree, you should see a script in the
home directory called test-all-versions. This script will run the unit
tests under Python 2, then create a temporary Python 3 conversion of
the source and run the unit tests again under Python 3.
