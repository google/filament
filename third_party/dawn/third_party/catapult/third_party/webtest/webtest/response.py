# -*- coding: utf-8 -*-
import re
from json import loads

from webtest import forms
from webtest import utils
from webtest.compat import print_stderr
from webtest.compat import splittype
from webtest.compat import splithost
from webtest.compat import PY3
from webtest.compat import urlparse
from webtest.compat import to_bytes

from six import string_types
from six import binary_type
from six import text_type

from bs4 import BeautifulSoup

import webob


class TestResponse(webob.Response):
    """
    Instances of this class are returned by
    :class:`~webtest.app.TestApp` methods.
    """

    request = None
    _forms_indexed = None
    parser_features = 'html.parser'

    @property
    def forms(self):
        """
        Returns a dictionary containing all the forms in the pages as
        :class:`~webtest.forms.Form` objects. Indexes are both in
        order (from zero) and by form id (if the form is given an id).

        See :doc:`forms` for more info on form objects.
        """
        if self._forms_indexed is None:
            self._parse_forms()
        return self._forms_indexed

    @property
    def form(self):
        """
        If there is only one form on the page, return it as a
        :class:`~webtest.forms.Form` object; raise a TypeError is
        there are no form or multiple forms.
        """
        forms_ = self.forms
        if not forms_:
            raise TypeError(
                "You used response.form, but no forms exist")
        if 1 in forms_:
            # There is more than one form
            raise TypeError(
                "You used response.form, but more than one form exists")
        return forms_[0]

    @property
    def testbody(self):
        self.decode_content()
        if self.charset:
            try:
                return self.text
            except UnicodeDecodeError:
                return self.body.decode(self.charset, 'replace')
        return self.body.decode('ascii', 'replace')

    _tag_re = re.compile(r'<(/?)([:a-z0-9_\-]*)(.*?)>', re.S | re.I)

    def _parse_forms(self):
        forms_ = self._forms_indexed = {}
        form_texts = [str(f) for f in self.html('form')]
        for i, text in enumerate(form_texts):
            form = forms.Form(self, text, self.parser_features)
            forms_[i] = form
            if form.id:
                forms_[form.id] = form

    def _follow(self, **kw):
        location = self.headers['location']
        abslocation = urlparse.urljoin(self.request.url, location)
        type_, rest = splittype(abslocation)
        host, path = splithost(rest)
        # @@: We should test that it's not a remote redirect
        return self.test_app.get(abslocation, **kw)

    def follow(self, **kw):
        """
        If this response is a redirect, follow that redirect.  It is an
        error if it is not a redirect response. Any keyword
        arguments are passed to :class:`webtest.app.TestApp.get`. Returns
        another :class:`TestResponse` object.
        """
        assert 300 <= self.status_int < 400, (
            "You can only follow redirect responses (not %s)"
            % self.status)
        return self._follow(**kw)

    def maybe_follow(self, **kw):
        """
        Follow all redirects. If this response is not a redirect, do nothing.
        Any keyword arguments are passed to :class:`webtest.app.TestApp.get`.
        Returns another :class:`TestResponse` object.
        """
        remaining_redirects = 100  # infinite loops protection
        response = self

        while 300 <= response.status_int < 400 and remaining_redirects:
            response = response._follow(**kw)
            remaining_redirects -= 1

        assert remaining_redirects > 0, "redirects chain looks infinite"
        return response

    def click(self, description=None, linkid=None, href=None,
              index=None, verbose=False,
              extra_environ=None):
        """
        Click the link as described.  Each of ``description``,
        ``linkid``, and ``url`` are *patterns*, meaning that they are
        either strings (regular expressions), compiled regular
        expressions (objects with a ``search`` method), or callables
        returning true or false.

        All the given patterns are ANDed together:

        * ``description`` is a pattern that matches the contents of the
          anchor (HTML and all -- everything between ``<a...>`` and
          ``</a>``)

        * ``linkid`` is a pattern that matches the ``id`` attribute of
          the anchor.  It will receive the empty string if no id is
          given.

        * ``href`` is a pattern that matches the ``href`` of the anchor;
          the literal content of that attribute, not the fully qualified
          attribute.

        If more than one link matches, then the ``index`` link is
        followed.  If ``index`` is not given and more than one link
        matches, or if no link matches, then ``IndexError`` will be
        raised.

        If you give ``verbose`` then messages will be printed about
        each link, and why it does or doesn't match.  If you use
        ``app.click(verbose=True)`` you'll see a list of all the
        links.

        You can use multiple criteria to essentially assert multiple
        aspects about the link, e.g., where the link's destination is.
        """
        found_html, found_desc, found_attrs = self._find_element(
            tag='a', href_attr='href',
            href_extract=None,
            content=description,
            id=linkid,
            href_pattern=href,
            index=index, verbose=verbose)
        return self.goto(str(found_attrs['uri']), extra_environ=extra_environ)

    def clickbutton(self, description=None, buttonid=None, href=None,
                    index=None, verbose=False):
        """
        Like :meth:`~webtest.response.TestResponse.click`, except looks
        for link-like buttons.
        This kind of button should look like
        ``<button onclick="...location.href='url'...">``.
        """
        found_html, found_desc, found_attrs = self._find_element(
            tag='button', href_attr='onclick',
            href_extract=re.compile(r"location\.href='(.*?)'"),
            content=description,
            id=buttonid,
            href_pattern=href,
            index=index, verbose=verbose)
        return self.goto(str(found_attrs['uri']))

    def _find_element(self, tag, href_attr, href_extract,
                      content, id,
                      href_pattern,
                      index, verbose):
        content_pat = utils.make_pattern(content)
        id_pat = utils.make_pattern(id)
        href_pat = utils.make_pattern(href_pattern)

        def printlog(s):
            if verbose:
                print(s)

        found_links = []
        total_links = 0
        for element in self.html.find_all(tag):
            el_html = str(element)
            el_content = element.decode_contents()
            attrs = element
            if verbose:
                printlog('Element: %r' % el_html)
            if not attrs.get(href_attr):
                printlog('  Skipped: no %s attribute' % href_attr)
                continue
            el_href = attrs[href_attr]
            if href_extract:
                m = href_extract.search(el_href)
                if not m:
                    printlog("  Skipped: doesn't match extract pattern")
                    continue
                el_href = m.group(1)
            attrs['uri'] = el_href
            if el_href.startswith('#'):
                printlog('  Skipped: only internal fragment href')
                continue
            if el_href.startswith('javascript:'):
                printlog('  Skipped: cannot follow javascript:')
                continue
            total_links += 1
            if content_pat and not content_pat(el_content):
                printlog("  Skipped: doesn't match description")
                continue
            if id_pat and not id_pat(attrs.get('id', '')):
                printlog("  Skipped: doesn't match id")
                continue
            if href_pat and not href_pat(el_href):
                printlog("  Skipped: doesn't match href")
                continue
            printlog("  Accepted")
            found_links.append((el_html, el_content, attrs))
        if not found_links:
            raise IndexError(
                "No matching elements found (from %s possible)"
                % total_links)
        if index is None:
            if len(found_links) > 1:
                raise IndexError(
                    "Multiple links match: %s"
                    % ', '.join([repr(anc) for anc, d, attr in found_links]))
            found_link = found_links[0]
        else:
            try:
                found_link = found_links[index]
            except IndexError:
                raise IndexError(
                    "Only %s (out of %s) links match; index %s out of range"
                    % (len(found_links), total_links, index))
        return found_link

    def goto(self, href, method='get', **args):
        """
        Go to the (potentially relative) link ``href``, using the
        given method (``'get'`` or ``'post'``) and any extra arguments
        you want to pass to the :meth:`webtest.app.TestApp.get` or
        :meth:`webtest.app.TestApp.post` methods.

        All hostnames and schemes will be ignored.
        """
        scheme, host, path, query, fragment = urlparse.urlsplit(href)
        # We
        scheme = host = fragment = ''
        href = urlparse.urlunsplit((scheme, host, path, query, fragment))
        href = urlparse.urljoin(self.request.url, href)
        method = method.lower()
        assert method in ('get', 'post'), (
            'Only "get" or "post" are allowed for method (you gave %r)'
            % method)

        # encode unicode strings for the outside world
        if not PY3 and getattr(self, '_use_unicode', False):
            def to_str(s):
                if isinstance(s, text_type):
                    return s.encode(self.charset)
                return s

            href = to_str(href)

            if 'params' in args:
                args['params'] = [tuple(map(to_str, p))
                                  for p in args['params']]

            if 'upload_files' in args:
                args['upload_files'] = [map(to_str, f)
                                        for f in args['upload_files']]

            if 'content_type' in args:
                args['content_type'] = to_str(args['content_type'])

        if method == 'get':
            method = self.test_app.get
        else:
            method = self.test_app.post
        return method(href, **args)

    _normal_body_regex = re.compile(to_bytes(r'[ \n\r\t]+'))

    @property
    def normal_body(self):
        """
        Return the whitespace-normalized body
        """
        if getattr(self, '_normal_body', None) is None:
            self._normal_body = self._normal_body_regex.sub(b' ', self.body)
        return self._normal_body

    _unicode_normal_body_regex = re.compile('[ \\n\\r\\t]+')

    @property
    def unicode_normal_body(self):
        """
        Return the whitespace-normalized body, as unicode
        """
        if not self.charset:
            raise AttributeError(
                ("You cannot access Response.unicode_normal_body "
                 "unless charset is set"))
        if getattr(self, '_unicode_normal_body', None) is None:
            self._unicode_normal_body = self._unicode_normal_body_regex.sub(
                ' ', self.testbody)
        return self._unicode_normal_body

    def __contains__(self, s):
        """
        A response 'contains' a string if it is present in the body
        of the response.  Whitespace is normalized when searching
        for a string.
        """
        if not self.charset and isinstance(s, text_type):
            s = s.encode('utf8')
        if isinstance(s, binary_type):
            return s in self.body or s in self.normal_body
        return s in self.testbody or s in self.unicode_normal_body

    def mustcontain(self, *strings, **kw):
        """mustcontain(*strings, no=[])

        Assert that the response contains all of the strings passed
        in as arguments.

        Equivalent to::

            assert string in res

        Can take a `no` keyword argument that can be a string or a
        list of strings which must not be present in the response.
        """
        if 'no' in kw:
            no = kw['no']
            del kw['no']
            if isinstance(no, string_types):
                no = [no]
        else:
            no = []
        if kw:
            raise TypeError(
                "The only keyword argument allowed is 'no'")
        for s in strings:
            if not s in self:
                print_stderr("Actual response (no %r):" % s)
                print_stderr(str(self))
                raise IndexError(
                    "Body does not contain string %r" % s)
        for no_s in no:
            if no_s in self:
                print_stderr("Actual response (has %r)" % no_s)
                print_stderr(str(self))
                raise IndexError(
                    "Body contains bad string %r" % no_s)

    def __str__(self):
        simple_body = str('\n').join([l for l in self.testbody.splitlines()
                                     if l.strip()])
        headers = [(n.title(), v)
                   for n, v in self.headerlist
                   if n.lower() != 'content-length']
        headers.sort()
        output = str('Response: %s\n%s\n%s') % (
            self.status,
            str('\n').join([str('%s: %s') % (n, v) for n, v in headers]),
            simple_body)
        if not PY3 and isinstance(output, text_type):
            output = output.encode(self.charset or 'utf8', 'replace')
        return output

    def __unicode__(self):
        output = str(self)
        if PY3:
            return output
        return output.decode(self.charset or 'utf8', 'replace')

    def __repr__(self):
        # Specifically intended for doctests
        if self.content_type:
            ct = ' %s' % self.content_type
        else:
            ct = ''
        if self.body:
            br = repr(self.body)
            if len(br) > 18:
                br = br[:10] + '...' + br[-5:]
                br += '/%s' % len(self.body)
            body = ' body=%s' % br
        else:
            body = ' no body'
        if self.location:
            location = ' location: %s' % self.location
        else:
            location = ''
        return ('<' + self.status + ct + location + body + '>')

    @property
    def html(self):
        """
        Returns the response as a `BeautifulSoup
        <http://www.crummy.com/software/BeautifulSoup/documentation.html>`_
        object.

        Only works with HTML responses; other content-types raise
        AttributeError.
        """
        if 'html' not in self.content_type:
            raise AttributeError(
                "Not an HTML response body (content-type: %s)"
                % self.content_type)
        soup = BeautifulSoup(self.testbody, self.parser_features)
        return soup

    @property
    def xml(self):
        """
        Returns the response as an `ElementTree
        <http://python.org/doc/current/lib/module-xml.etree.ElementTree.html>`_
        object.

        Only works with XML responses; other content-types raise
        AttributeError
        """
        if 'xml' not in self.content_type:
            raise AttributeError(
                "Not an XML response body (content-type: %s)"
                % self.content_type)
        try:
            from xml.etree import ElementTree
        except ImportError:  # pragma: no cover
            try:
                import ElementTree
            except ImportError:
                try:
                    from elementtree import ElementTree  # NOQA
                except ImportError:
                    raise ImportError(
                        ("You must have ElementTree installed "
                         "(or use Python 2.5) to use response.xml"))
        # ElementTree can't parse unicode => use `body` instead of `testbody`
        return ElementTree.XML(self.body)

    @property
    def lxml(self):
        """
        Returns the response as an `lxml object
        <http://codespeak.net/lxml/>`_.  You must have lxml installed
        to use this.

        If this is an HTML response and you have lxml 2.x installed,
        then an ``lxml.html.HTML`` object will be returned; if you
        have an earlier version of lxml then a ``lxml.HTML`` object
        will be returned.
        """
        if 'html' not in self.content_type and \
           'xml' not in self.content_type:
            raise AttributeError(
                "Not an XML or HTML response body (content-type: %s)"
                % self.content_type)
        try:
            from lxml import etree
        except ImportError:  # pragma: no cover
            raise ImportError(
                "You must have lxml installed to use response.lxml")
        try:
            from lxml.html import fromstring
        except ImportError:  # pragma: no cover
            fromstring = etree.HTML
        ## FIXME: would be nice to set xml:base, in some fashion
        if self.content_type == 'text/html':
            return fromstring(self.testbody, base_url=self.request.url)
        else:
            return etree.XML(self.testbody, base_url=self.request.url)

    @property
    def json(self):
        """
        Return the response as a JSON response.  You must have `simplejson
        <http://goo.gl/B9g6s>`_ installed to use this, or be using a Python
        version with the json module.

        The content type must be one of json type to use this.
        """
        if not self.content_type.endswith(('+json', '/json')):
            raise AttributeError(
                "Not a JSON response body (content-type: %s)"
                % self.content_type)
        return loads(self.testbody)

    @property
    def pyquery(self):
        """
        Returns the response as a `PyQuery <http://pyquery.org/>`_ object.

        Only works with HTML and XML responses; other content-types raise
        AttributeError.
        """
        if 'html' not in self.content_type and 'xml' not in self.content_type:
            raise AttributeError(
                "Not an HTML or XML response body (content-type: %s)"
                % self.content_type)
        try:
            from pyquery import PyQuery
        except ImportError:  # pragma: no cover
            raise ImportError(
                "You must have PyQuery installed to use response.pyquery")
        d = PyQuery(self.testbody)
        return d

    def showbrowser(self):
        """
        Show this response in a browser window (for debugging purposes,
        when it's hard to read the HTML).
        """
        import webbrowser
        import tempfile
        f = tempfile.NamedTemporaryFile(prefix='webtest-page',
                                        suffix='.html')
        name = f.name
        f.close()
        f = open(name, 'w')
        if PY3:
            f.write(self.body.decode(self.charset or 'ascii', 'replace'))
        else:
            f.write(self.body)
        f.close()
        if name[0] != '/':  # pragma: no cover
            # windows ...
            url = 'file:///' + name
        else:
            url = 'file://' + name
        webbrowser.open_new(url)
