"""Tests to ensure that the html.parser tree builder generates good
trees."""

from pdb import set_trace
import pickle
from bs4.testing import SoupTest, HTMLTreeBuilderSmokeTest
from bs4.builder import HTMLParserTreeBuilder
from bs4.builder._htmlparser import BeautifulSoupHTMLParser

class HTMLParserTreeBuilderSmokeTest(SoupTest, HTMLTreeBuilderSmokeTest):

    default_builder = HTMLParserTreeBuilder

    def test_namespaced_system_doctype(self):
        # html.parser can't handle namespaced doctypes, so skip this one.
        pass

    def test_namespaced_public_doctype(self):
        # html.parser can't handle namespaced doctypes, so skip this one.
        pass

    def test_builder_is_pickled(self):
        """Unlike most tree builders, HTMLParserTreeBuilder and will
        be restored after pickling.
        """
        tree = self.soup("<a><b>foo</a>")
        dumped = pickle.dumps(tree, 2)
        loaded = pickle.loads(dumped)
        self.assertTrue(isinstance(loaded.builder, type(tree.builder)))

    def test_redundant_empty_element_closing_tags(self):
        self.assertSoupEquals('<br></br><br></br><br></br>', "<br/><br/><br/>")
        self.assertSoupEquals('</br></br></br>', "")

    def test_empty_element(self):
        # This verifies that any buffered data present when the parser
        # finishes working is handled.
        self.assertSoupEquals("foo &# bar", "foo &amp;# bar")

    def test_tracking_line_numbers(self):
        # The html.parser TreeBuilder keeps track of line number and
        # position of each element.
        markup = "\n   <p>\n\n<sourceline>\n<b>text</b></sourceline><sourcepos></p>"
        soup = self.soup(markup)
        self.assertEqual(2, soup.p.sourceline)
        self.assertEqual(3, soup.p.sourcepos)
        self.assertEqual("sourceline", soup.p.find('sourceline').name)

        # You can deactivate this behavior.
        soup = self.soup(markup, store_line_numbers=False)
        self.assertEqual("sourceline", soup.p.sourceline.name)
        self.assertEqual("sourcepos", soup.p.sourcepos.name)

    def test_on_duplicate_attribute(self):
        # The html.parser tree builder has a variety of ways of
        # handling a tag that contains the same attribute multiple times.

        markup = '<a class="cls" href="url1" href="url2" href="url3" id="id">'

        # If you don't provide any particular value for
        # on_duplicate_attribute, later values replace earlier values.
        soup = self.soup(markup)
        self.assertEqual("url3", soup.a['href'])
        self.assertEqual(["cls"], soup.a['class'])
        self.assertEqual("id", soup.a['id'])
        
        # You can also get this behavior explicitly.
        def assert_attribute(on_duplicate_attribute, expected):
            soup = self.soup(
                markup, on_duplicate_attribute=on_duplicate_attribute
            )
            self.assertEqual(expected, soup.a['href'])

            # Verify that non-duplicate attributes are treated normally.
            self.assertEqual(["cls"], soup.a['class'])
            self.assertEqual("id", soup.a['id'])
        assert_attribute(None, "url3")
        assert_attribute(BeautifulSoupHTMLParser.REPLACE, "url3")

        # You can ignore subsequent values in favor of the first.
        assert_attribute(BeautifulSoupHTMLParser.IGNORE, "url1")

        # And you can pass in a callable that does whatever you want.
        def accumulate(attrs, key, value):
            if not isinstance(attrs[key], list):
                attrs[key] = [attrs[key]]
            attrs[key].append(value)
        assert_attribute(accumulate, ["url1", "url2", "url3"])            


class TestHTMLParserSubclass(SoupTest):
    def test_error(self):
        """Verify that our HTMLParser subclass implements error() in a way
        that doesn't cause a crash.
        """
        parser = BeautifulSoupHTMLParser()
        parser.error("don't crash")
