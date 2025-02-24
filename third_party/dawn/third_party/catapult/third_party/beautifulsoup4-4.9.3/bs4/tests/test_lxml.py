"""Tests to ensure that the lxml tree builder generates good trees."""

import re
import warnings

try:
    import lxml.etree
    LXML_PRESENT = True
    LXML_VERSION = lxml.etree.LXML_VERSION
except ImportError, e:
    LXML_PRESENT = False
    LXML_VERSION = (0,)

if LXML_PRESENT:
    from bs4.builder import LXMLTreeBuilder, LXMLTreeBuilderForXML

from bs4 import (
    BeautifulSoup,
    BeautifulStoneSoup,
    )
from bs4.element import Comment, Doctype, SoupStrainer
from bs4.testing import skipIf
from bs4.tests import test_htmlparser
from bs4.testing import (
    HTMLTreeBuilderSmokeTest,
    XMLTreeBuilderSmokeTest,
    SoupTest,
    skipIf,
)

@skipIf(
    not LXML_PRESENT,
    "lxml seems not to be present, not testing its tree builder.")
class LXMLTreeBuilderSmokeTest(SoupTest, HTMLTreeBuilderSmokeTest):
    """See ``HTMLTreeBuilderSmokeTest``."""

    @property
    def default_builder(self):
        return LXMLTreeBuilder

    def test_out_of_range_entity(self):
        self.assertSoupEquals(
            "<p>foo&#10000000000000;bar</p>", "<p>foobar</p>")
        self.assertSoupEquals(
            "<p>foo&#x10000000000000;bar</p>", "<p>foobar</p>")
        self.assertSoupEquals(
            "<p>foo&#1000000000;bar</p>", "<p>foobar</p>")

    def test_entities_in_foreign_document_encoding(self):
        # We can't implement this case correctly because by the time we
        # hear about markup like "&#147;", it's been (incorrectly) converted into
        # a string like u'\x93'
        pass
        
    # In lxml < 2.3.5, an empty doctype causes a segfault. Skip this
    # test if an old version of lxml is installed.

    @skipIf(
        not LXML_PRESENT or LXML_VERSION < (2,3,5,0),
        "Skipping doctype test for old version of lxml to avoid segfault.")
    def test_empty_doctype(self):
        soup = self.soup("<!DOCTYPE>")
        doctype = soup.contents[0]
        self.assertEqual("", doctype.strip())

    def test_beautifulstonesoup_is_xml_parser(self):
        # Make sure that the deprecated BSS class uses an xml builder
        # if one is installed.
        with warnings.catch_warnings(record=True) as w:
            soup = BeautifulStoneSoup("<b />")
        self.assertEqual(u"<b/>", unicode(soup.b))
        self.assertTrue("BeautifulStoneSoup class is deprecated" in str(w[0].message))

    def test_tracking_line_numbers(self):
        # The lxml TreeBuilder cannot keep track of line numbers from
        # the original markup. Even if you ask for line numbers, we
        # don't have 'em.
        #
        # This means that if you have a tag like <sourceline> or
        # <sourcepos>, attribute access will find it rather than
        # giving you a numeric answer.
        soup = self.soup(
            "\n   <p>\n\n<sourceline>\n<b>text</b></sourceline><sourcepos></p>",
            store_line_numbers=True
        )
        self.assertEqual("sourceline", soup.p.sourceline.name)
        self.assertEqual("sourcepos", soup.p.sourcepos.name)
        
@skipIf(
    not LXML_PRESENT,
    "lxml seems not to be present, not testing its XML tree builder.")
class LXMLXMLTreeBuilderSmokeTest(SoupTest, XMLTreeBuilderSmokeTest):
    """See ``HTMLTreeBuilderSmokeTest``."""

    @property
    def default_builder(self):
        return LXMLTreeBuilderForXML

    def test_namespace_indexing(self):
        # We should not track un-prefixed namespaces as we can only hold one
        # and it will be recognized as the default namespace by soupsieve,
        # which may be confusing in some situations. When no namespace is provided
        # for a selector, the default namespace (if defined) is assumed.

        soup = self.soup(
            '<?xml version="1.1"?>\n'
            '<root>'
            '<tag xmlns="http://unprefixed-namespace.com">content</tag>'
            '<prefix:tag xmlns:prefix="http://prefixed-namespace.com">content</tag>'
            '</root>'
        )
        self.assertEqual(
            soup._namespaces,
            {'xml': 'http://www.w3.org/XML/1998/namespace', 'prefix': 'http://prefixed-namespace.com'}
        )
