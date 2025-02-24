"""Tests to ensure that the html5lib tree builder generates good trees."""

import warnings

try:
    from bs4.builder import HTML5TreeBuilder
    HTML5LIB_PRESENT = True
except ImportError, e:
    HTML5LIB_PRESENT = False
from bs4.element import SoupStrainer
from bs4.testing import (
    HTML5TreeBuilderSmokeTest,
    SoupTest,
    skipIf,
)

@skipIf(
    not HTML5LIB_PRESENT,
    "html5lib seems not to be present, not testing its tree builder.")
class HTML5LibBuilderSmokeTest(SoupTest, HTML5TreeBuilderSmokeTest):
    """See ``HTML5TreeBuilderSmokeTest``."""

    @property
    def default_builder(self):
        return HTML5TreeBuilder

    def test_soupstrainer(self):
        # The html5lib tree builder does not support SoupStrainers.
        strainer = SoupStrainer("b")
        markup = "<p>A <b>bold</b> statement.</p>"
        with warnings.catch_warnings(record=True) as w:
            soup = self.soup(markup, parse_only=strainer)
        self.assertEqual(
            soup.decode(), self.document_for(markup))

        self.assertTrue(
            "the html5lib tree builder doesn't support parse_only" in
            str(w[0].message))

    def test_correctly_nested_tables(self):
        """html5lib inserts <tbody> tags where other parsers don't."""
        markup = ('<table id="1">'
                  '<tr>'
                  "<td>Here's another table:"
                  '<table id="2">'
                  '<tr><td>foo</td></tr>'
                  '</table></td>')

        self.assertSoupEquals(
            markup,
            '<table id="1"><tbody><tr><td>Here\'s another table:'
            '<table id="2"><tbody><tr><td>foo</td></tr></tbody></table>'
            '</td></tr></tbody></table>')

        self.assertSoupEquals(
            "<table><thead><tr><td>Foo</td></tr></thead>"
            "<tbody><tr><td>Bar</td></tr></tbody>"
            "<tfoot><tr><td>Baz</td></tr></tfoot></table>")

    def test_xml_declaration_followed_by_doctype(self):
        markup = '''<?xml version="1.0" encoding="utf-8"?>
<!DOCTYPE html>
<html>
  <head>
  </head>
  <body>
   <p>foo</p>
  </body>
</html>'''
        soup = self.soup(markup)
        # Verify that we can reach the <p> tag; this means the tree is connected.
        self.assertEqual(b"<p>foo</p>", soup.p.encode())

    def test_reparented_markup(self):
        markup = '<p><em>foo</p>\n<p>bar<a></a></em></p>'
        soup = self.soup(markup)
        self.assertEqual(u"<body><p><em>foo</em></p><em>\n</em><p><em>bar<a></a></em></p></body>", soup.body.decode())
        self.assertEqual(2, len(soup.find_all('p')))


    def test_reparented_markup_ends_with_whitespace(self):
        markup = '<p><em>foo</p>\n<p>bar<a></a></em></p>\n'
        soup = self.soup(markup)
        self.assertEqual(u"<body><p><em>foo</em></p><em>\n</em><p><em>bar<a></a></em></p>\n</body>", soup.body.decode())
        self.assertEqual(2, len(soup.find_all('p')))

    def test_reparented_markup_containing_identical_whitespace_nodes(self):
        """Verify that we keep the two whitespace nodes in this
        document distinct when reparenting the adjacent <tbody> tags.
        """
        markup = '<table> <tbody><tbody><ims></tbody> </table>'
        soup = self.soup(markup)
        space1, space2 = soup.find_all(string=' ')
        tbody1, tbody2 = soup.find_all('tbody')
        assert space1.next_element is tbody1
        assert tbody2.next_element is space2

    def test_reparented_markup_containing_children(self):
        markup = '<div><a>aftermath<p><noscript>target</noscript>aftermath</a></p></div>'
        soup = self.soup(markup)
        noscript = soup.noscript
        self.assertEqual("target", noscript.next_element)
        target = soup.find(string='target')

        # The 'aftermath' string was duplicated; we want the second one.
        final_aftermath = soup.find_all(string='aftermath')[-1]

        # The <noscript> tag was moved beneath a copy of the <a> tag,
        # but the 'target' string within is still connected to the
        # (second) 'aftermath' string.
        self.assertEqual(final_aftermath, target.next_element)
        self.assertEqual(target, final_aftermath.previous_element)
        
    def test_processing_instruction(self):
        """Processing instructions become comments."""
        markup = b"""<?PITarget PIContent?>"""
        soup = self.soup(markup)
        assert str(soup).startswith("<!--?PITarget PIContent?-->")

    def test_cloned_multivalue_node(self):
        markup = b"""<a class="my_class"><p></a>"""
        soup = self.soup(markup)
        a1, a2 = soup.find_all('a')
        self.assertEqual(a1, a2)
        assert a1 is not a2

    def test_foster_parenting(self):
        markup = b"""<table><td></tbody>A"""
        soup = self.soup(markup)
        self.assertEqual(u"<body>A<table><tbody><tr><td></td></tr></tbody></table></body>", soup.body.decode())

    def test_extraction(self):
        """
        Test that extraction does not destroy the tree.

        https://bugs.launchpad.net/beautifulsoup/+bug/1782928
        """

        markup = """
<html><head></head>
<style>
</style><script></script><body><p>hello</p></body></html>
"""
        soup = self.soup(markup)
        [s.extract() for s in soup('script')]
        [s.extract() for s in soup('style')]

        self.assertEqual(len(soup.find_all("p")), 1)

    def test_empty_comment(self):
        """
        Test that empty comment does not break structure.

        https://bugs.launchpad.net/beautifulsoup/+bug/1806598
        """

        markup = """
<html>
<body>
<form>
<!----><input type="text">
</form>
</body>
</html>
"""
        soup = self.soup(markup)
        inputs = []
        for form in soup.find_all('form'):
            inputs.extend(form.find_all('input'))
        self.assertEqual(len(inputs), 1)

    def test_tracking_line_numbers(self):
        # The html.parser TreeBuilder keeps track of line number and
        # position of each element.
        markup = "\n   <p>\n\n<sourceline>\n<b>text</b></sourceline><sourcepos></p>"
        soup = self.soup(markup)
        self.assertEqual(2, soup.p.sourceline)
        self.assertEqual(5, soup.p.sourcepos)
        self.assertEqual("sourceline", soup.p.find('sourceline').name)

        # You can deactivate this behavior.
        soup = self.soup(markup, store_line_numbers=False)
        self.assertEqual("sourceline", soup.p.sourceline.name)
        self.assertEqual("sourcepos", soup.p.sourcepos.name)

    def test_special_string_containers(self):
        # The html5lib tree builder doesn't support this standard feature,
        # because there's no way of knowing, when a string is created,
        # where in the tree it will eventually end up.
        pass
