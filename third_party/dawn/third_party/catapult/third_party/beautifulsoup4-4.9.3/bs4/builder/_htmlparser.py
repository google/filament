# encoding: utf-8
"""Use the HTMLParser library to parse HTML files that aren't too bad."""

# Use of this source code is governed by the MIT license.
__license__ = "MIT"

__all__ = [
    'HTMLParserTreeBuilder',
    ]

from HTMLParser import HTMLParser

try:
    from HTMLParser import HTMLParseError
except ImportError, e:
    # HTMLParseError is removed in Python 3.5. Since it can never be
    # thrown in 3.5, we can just define our own class as a placeholder.
    class HTMLParseError(Exception):
        pass

import sys
import warnings

# Starting in Python 3.2, the HTMLParser constructor takes a 'strict'
# argument, which we'd like to set to False. Unfortunately,
# http://bugs.python.org/issue13273 makes strict=True a better bet
# before Python 3.2.3.
#
# At the end of this file, we monkeypatch HTMLParser so that
# strict=True works well on Python 3.2.2.
major, minor, release = sys.version_info[:3]
CONSTRUCTOR_TAKES_STRICT = major == 3 and minor == 2 and release >= 3
CONSTRUCTOR_STRICT_IS_DEPRECATED = major == 3 and minor == 3
CONSTRUCTOR_TAKES_CONVERT_CHARREFS = major == 3 and minor >= 4


from bs4.element import (
    CData,
    Comment,
    Declaration,
    Doctype,
    ProcessingInstruction,
    )
from bs4.dammit import EntitySubstitution, UnicodeDammit

from bs4.builder import (
    HTML,
    HTMLTreeBuilder,
    STRICT,
    )


HTMLPARSER = 'html.parser'

class BeautifulSoupHTMLParser(HTMLParser):
    """A subclass of the Python standard library's HTMLParser class, which
    listens for HTMLParser events and translates them into calls
    to Beautiful Soup's tree construction API.
    """

    # Strategies for handling duplicate attributes
    IGNORE = 'ignore'
    REPLACE = 'replace'
    
    def __init__(self, *args, **kwargs):
        """Constructor.

        :param on_duplicate_attribute: A strategy for what to do if a
            tag includes the same attribute more than once. Accepted
            values are: REPLACE (replace earlier values with later
            ones, the default), IGNORE (keep the earliest value
            encountered), or a callable. A callable must take three
            arguments: the dictionary of attributes already processed,
            the name of the duplicate attribute, and the most recent value
            encountered.           
        """
        self.on_duplicate_attribute = kwargs.pop(
            'on_duplicate_attribute', self.REPLACE
        )
        HTMLParser.__init__(self, *args, **kwargs)

        # Keep a list of empty-element tags that were encountered
        # without an explicit closing tag. If we encounter a closing tag
        # of this type, we'll associate it with one of those entries.
        #
        # This isn't a stack because we don't care about the
        # order. It's a list of closing tags we've already handled and
        # will ignore, assuming they ever show up.
        self.already_closed_empty_element = []

    def error(self, msg):
        """In Python 3, HTMLParser subclasses must implement error(), although
        this requirement doesn't appear to be documented.

        In Python 2, HTMLParser implements error() by raising an exception,
        which we don't want to do.

        In any event, this method is called only on very strange
        markup and our best strategy is to pretend it didn't happen
        and keep going.
        """
        warnings.warn(msg)
        
    def handle_startendtag(self, name, attrs):
        """Handle an incoming empty-element tag.

        This is only called when the markup looks like <tag/>.

        :param name: Name of the tag.
        :param attrs: Dictionary of the tag's attributes.
        """
        # is_startend() tells handle_starttag not to close the tag
        # just because its name matches a known empty-element tag. We
        # know that this is an empty-element tag and we want to call
        # handle_endtag ourselves.
        tag = self.handle_starttag(name, attrs, handle_empty_element=False)
        self.handle_endtag(name)
        
    def handle_starttag(self, name, attrs, handle_empty_element=True):
        """Handle an opening tag, e.g. '<tag>'

        :param name: Name of the tag.
        :param attrs: Dictionary of the tag's attributes.
        :param handle_empty_element: True if this tag is known to be
            an empty-element tag (i.e. there is not expected to be any
            closing tag).
        """
        # XXX namespace
        attr_dict = {}
        for key, value in attrs:
            # Change None attribute values to the empty string
            # for consistency with the other tree builders.
            if value is None:
                value = ''
            if key in attr_dict:
                # A single attribute shows up multiple times in this
                # tag. How to handle it depends on the
                # on_duplicate_attribute setting.
                on_dupe = self.on_duplicate_attribute
                if on_dupe == self.IGNORE:
                    pass
                elif on_dupe in (None, self.REPLACE):
                    attr_dict[key] = value
                else:
                    on_dupe(attr_dict, key, value)
            else:
                attr_dict[key] = value
            attrvalue = '""'
        #print("START", name)
        sourceline, sourcepos = self.getpos()
        tag = self.soup.handle_starttag(
            name, None, None, attr_dict, sourceline=sourceline,
            sourcepos=sourcepos
        )
        if tag and tag.is_empty_element and handle_empty_element:
            # Unlike other parsers, html.parser doesn't send separate end tag
            # events for empty-element tags. (It's handled in
            # handle_startendtag, but only if the original markup looked like
            # <tag/>.)
            #
            # So we need to call handle_endtag() ourselves. Since we
            # know the start event is identical to the end event, we
            # don't want handle_endtag() to cross off any previous end
            # events for tags of this name.
            self.handle_endtag(name, check_already_closed=False)

            # But we might encounter an explicit closing tag for this tag
            # later on. If so, we want to ignore it.
            self.already_closed_empty_element.append(name)
            
    def handle_endtag(self, name, check_already_closed=True):
        """Handle a closing tag, e.g. '</tag>'
        
        :param name: A tag name.
        :param check_already_closed: True if this tag is expected to
           be the closing portion of an empty-element tag,
           e.g. '<tag></tag>'.
        """
        #print("END", name)
        if check_already_closed and name in self.already_closed_empty_element:
            # This is a redundant end tag for an empty-element tag.
            # We've already called handle_endtag() for it, so just
            # check it off the list.
            #print("ALREADY CLOSED", name)
            self.already_closed_empty_element.remove(name)
        else:
            self.soup.handle_endtag(name)

    def handle_data(self, data):
        """Handle some textual data that shows up between tags."""
        self.soup.handle_data(data)

    def handle_charref(self, name):
        """Handle a numeric character reference by converting it to the
        corresponding Unicode character and treating it as textual
        data.

        :param name: Character number, possibly in hexadecimal.
        """
        # XXX workaround for a bug in HTMLParser. Remove this once
        # it's fixed in all supported versions.
        # http://bugs.python.org/issue13633
        if name.startswith('x'):
            real_name = int(name.lstrip('x'), 16)
        elif name.startswith('X'):
            real_name = int(name.lstrip('X'), 16)
        else:
            real_name = int(name)

        data = None
        if real_name < 256:
            # HTML numeric entities are supposed to reference Unicode
            # code points, but sometimes they reference code points in
            # some other encoding (ahem, Windows-1252). E.g. &#147;
            # instead of &#201; for LEFT DOUBLE QUOTATION MARK. This
            # code tries to detect this situation and compensate.
            for encoding in (self.soup.original_encoding, 'windows-1252'):
                if not encoding:
                    continue
                try:
                    data = bytearray([real_name]).decode(encoding)
                except UnicodeDecodeError, e:
                    pass
        if not data:
            try:
                data = unichr(real_name)
            except (ValueError, OverflowError), e:
                pass
        data = data or u"\N{REPLACEMENT CHARACTER}"
        self.handle_data(data)

    def handle_entityref(self, name):
        """Handle a named entity reference by converting it to the
        corresponding Unicode character and treating it as textual
        data.

        :param name: Name of the entity reference.
        """
        character = EntitySubstitution.HTML_ENTITY_TO_CHARACTER.get(name)
        if character is not None:
            data = character
        else:
            # If this were XML, it would be ambiguous whether "&foo"
            # was an character entity reference with a missing
            # semicolon or the literal string "&foo". Since this is
            # HTML, we have a complete list of all character entity references,
            # and this one wasn't found, so assume it's the literal string "&foo".
            data = "&%s" % name
        self.handle_data(data)

    def handle_comment(self, data):
        """Handle an HTML comment.

        :param data: The text of the comment.
        """
        self.soup.endData()
        self.soup.handle_data(data)
        self.soup.endData(Comment)

    def handle_decl(self, data):
        """Handle a DOCTYPE declaration.

        :param data: The text of the declaration.
        """
        self.soup.endData()
        data = data[len("DOCTYPE "):]
        self.soup.handle_data(data)
        self.soup.endData(Doctype)

    def unknown_decl(self, data):
        """Handle a declaration of unknown type -- probably a CDATA block.

        :param data: The text of the declaration.
        """
        if data.upper().startswith('CDATA['):
            cls = CData
            data = data[len('CDATA['):]
        else:
            cls = Declaration
        self.soup.endData()
        self.soup.handle_data(data)
        self.soup.endData(cls)

    def handle_pi(self, data):
        """Handle a processing instruction.

        :param data: The text of the instruction.
        """
        self.soup.endData()
        self.soup.handle_data(data)
        self.soup.endData(ProcessingInstruction)


class HTMLParserTreeBuilder(HTMLTreeBuilder):
    """A Beautiful soup `TreeBuilder` that uses the `HTMLParser` parser,
    found in the Python standard library.
    """
    is_xml = False
    picklable = True
    NAME = HTMLPARSER
    features = [NAME, HTML, STRICT]

    # The html.parser knows which line number and position in the
    # original file is the source of an element.
    TRACKS_LINE_NUMBERS = True

    def __init__(self, parser_args=None, parser_kwargs=None, **kwargs):
        """Constructor.

        :param parser_args: Positional arguments to pass into 
            the BeautifulSoupHTMLParser constructor, once it's
            invoked.
        :param parser_kwargs: Keyword arguments to pass into 
            the BeautifulSoupHTMLParser constructor, once it's
            invoked.
        :param kwargs: Keyword arguments for the superclass constructor.
        """
        # Some keyword arguments will be pulled out of kwargs and placed
        # into parser_kwargs.
        extra_parser_kwargs = dict()
        for arg in ('on_duplicate_attribute',):
            if arg in kwargs:
                value = kwargs.pop(arg)
                extra_parser_kwargs[arg] = value
        super(HTMLParserTreeBuilder, self).__init__(**kwargs)
        parser_args = parser_args or []
        parser_kwargs = parser_kwargs or {}
        parser_kwargs.update(extra_parser_kwargs)
        if CONSTRUCTOR_TAKES_STRICT and not CONSTRUCTOR_STRICT_IS_DEPRECATED:
            parser_kwargs['strict'] = False
        if CONSTRUCTOR_TAKES_CONVERT_CHARREFS:
            parser_kwargs['convert_charrefs'] = False
        self.parser_args = (parser_args, parser_kwargs)
        
    def prepare_markup(self, markup, user_specified_encoding=None,
                       document_declared_encoding=None, exclude_encodings=None):

        """Run any preliminary steps necessary to make incoming markup
        acceptable to the parser.

        :param markup: Some markup -- probably a bytestring.
        :param user_specified_encoding: The user asked to try this encoding.
        :param document_declared_encoding: The markup itself claims to be
            in this encoding.
        :param exclude_encodings: The user asked _not_ to try any of
            these encodings.

        :yield: A series of 4-tuples:
         (markup, encoding, declared encoding,
          has undergone character replacement)

         Each 4-tuple represents a strategy for converting the
         document to Unicode and parsing it. Each strategy will be tried 
         in turn.
        """
        if isinstance(markup, unicode):
            # Parse Unicode as-is.
            yield (markup, None, None, False)
            return

        # Ask UnicodeDammit to sniff the most likely encoding.
        try_encodings = [user_specified_encoding, document_declared_encoding]
        dammit = UnicodeDammit(markup, try_encodings, is_html=True,
                               exclude_encodings=exclude_encodings)
        yield (dammit.markup, dammit.original_encoding,
               dammit.declared_html_encoding,
               dammit.contains_replacement_characters)

    def feed(self, markup):
        """Run some incoming markup through some parsing process,
        populating the `BeautifulSoup` object in self.soup.
        """
        args, kwargs = self.parser_args
        parser = BeautifulSoupHTMLParser(*args, **kwargs)
        parser.soup = self.soup
        try:
            parser.feed(markup)
            parser.close()
        except HTMLParseError, e:
            warnings.warn(RuntimeWarning(
                "Python's built-in HTMLParser cannot parse the given document. This is not a bug in Beautiful Soup. The best solution is to install an external parser (lxml or html5lib), and use Beautiful Soup with that parser. See http://www.crummy.com/software/BeautifulSoup/bs4/doc/#installing-a-parser for help."))
            raise e
        parser.already_closed_empty_element = []

# Patch 3.2 versions of HTMLParser earlier than 3.2.3 to use some
# 3.2.3 code. This ensures they don't treat markup like <p></p> as a
# string.
#
# XXX This code can be removed once most Python 3 users are on 3.2.3.
if major == 3 and minor == 2 and not CONSTRUCTOR_TAKES_STRICT:
    import re
    attrfind_tolerant = re.compile(
        r'\s*((?<=[\'"\s])[^\s/>][^\s/=>]*)(\s*=+\s*'
        r'(\'[^\']*\'|"[^"]*"|(?![\'"])[^>\s]*))?')
    HTMLParserTreeBuilder.attrfind_tolerant = attrfind_tolerant

    locatestarttagend = re.compile(r"""
  <[a-zA-Z][-.a-zA-Z0-9:_]*          # tag name
  (?:\s+                             # whitespace before attribute name
    (?:[a-zA-Z_][-.:a-zA-Z0-9_]*     # attribute name
      (?:\s*=\s*                     # value indicator
        (?:'[^']*'                   # LITA-enclosed value
          |\"[^\"]*\"                # LIT-enclosed value
          |[^'\">\s]+                # bare value
         )
       )?
     )
   )*
  \s*                                # trailing whitespace
""", re.VERBOSE)
    BeautifulSoupHTMLParser.locatestarttagend = locatestarttagend

    from html.parser import tagfind, attrfind

    def parse_starttag(self, i):
        self.__starttag_text = None
        endpos = self.check_for_whole_start_tag(i)
        if endpos < 0:
            return endpos
        rawdata = self.rawdata
        self.__starttag_text = rawdata[i:endpos]

        # Now parse the data between i+1 and j into a tag and attrs
        attrs = []
        match = tagfind.match(rawdata, i+1)
        assert match, 'unexpected call to parse_starttag()'
        k = match.end()
        self.lasttag = tag = rawdata[i+1:k].lower()
        while k < endpos:
            if self.strict:
                m = attrfind.match(rawdata, k)
            else:
                m = attrfind_tolerant.match(rawdata, k)
            if not m:
                break
            attrname, rest, attrvalue = m.group(1, 2, 3)
            if not rest:
                attrvalue = None
            elif attrvalue[:1] == '\'' == attrvalue[-1:] or \
                 attrvalue[:1] == '"' == attrvalue[-1:]:
                attrvalue = attrvalue[1:-1]
            if attrvalue:
                attrvalue = self.unescape(attrvalue)
            attrs.append((attrname.lower(), attrvalue))
            k = m.end()

        end = rawdata[k:endpos].strip()
        if end not in (">", "/>"):
            lineno, offset = self.getpos()
            if "\n" in self.__starttag_text:
                lineno = lineno + self.__starttag_text.count("\n")
                offset = len(self.__starttag_text) \
                         - self.__starttag_text.rfind("\n")
            else:
                offset = offset + len(self.__starttag_text)
            if self.strict:
                self.error("junk characters in start tag: %r"
                           % (rawdata[k:endpos][:20],))
            self.handle_data(rawdata[i:endpos])
            return endpos
        if end.endswith('/>'):
            # XHTML-style empty tag: <span attr="value" />
            self.handle_startendtag(tag, attrs)
        else:
            self.handle_starttag(tag, attrs)
            if tag in self.CDATA_CONTENT_ELEMENTS:
                self.set_cdata_mode(tag)
        return endpos

    def set_cdata_mode(self, elem):
        self.cdata_elem = elem.lower()
        self.interesting = re.compile(r'</\s*%s\s*>' % self.cdata_elem, re.I)

    BeautifulSoupHTMLParser.parse_starttag = parse_starttag
    BeautifulSoupHTMLParser.set_cdata_mode = set_cdata_mode

    CONSTRUCTOR_TAKES_STRICT = True
