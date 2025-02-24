= Introduction =

  >>> from bs4 import BeautifulSoup
  >>> soup = BeautifulSoup("<p>Some<b>bad<i>HTML")
  >>> print soup.prettify()
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
  u'bad'

  >>> soup.i
  <i>HTML</i>

  >>> soup = BeautifulSoup("<tag1>Some<tag2/>bad<tag3>XML", "xml")
  >>> print soup.prettify()
  <?xml version="1.0" encoding="utf-8">
  <tag1>
   Some
   <tag2 />
   bad
   <tag3>
    XML
   </tag3>
  </tag1>

= Full documentation =

The bs4/doc/ directory contains full documentation in Sphinx
format. Run "make html" in that directory to create HTML
documentation.

= Running the unit tests =

Beautiful Soup supports unit test discovery from the project root directory:

 $ nosetests

 $ python -m unittest discover -s bs4 # Python 2.7 and up

If you checked out the source tree, you should see a script in the
home directory called test-all-versions. This script will run the unit
tests under Python 2.7, then create a temporary Python 3 conversion of
the source and run the unit tests again under Python 3.

= Links =

Homepage: http://www.crummy.com/software/BeautifulSoup/bs4/
Documentation: http://www.crummy.com/software/BeautifulSoup/bs4/doc/
               http://readthedocs.org/docs/beautiful-soup-4/
Discussion group: http://groups.google.com/group/beautifulsoup/
Development: https://code.launchpad.net/beautifulsoup/
Bug tracker: https://bugs.launchpad.net/beautifulsoup/
