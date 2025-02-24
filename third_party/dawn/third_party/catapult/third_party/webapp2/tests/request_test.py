# -*- coding: utf-8 -*-
import StringIO

import webapp2

import test_base


def _norm_req(s):
    return '\r\n'.join(s.strip().replace('\r','').split('\n'))

_test_req = """
POST /webob/ HTTP/1.0
Accept: */*
Cache-Control: max-age=0
Content-Type: multipart/form-data; boundary=----------------------------deb95b63e42a
Host: pythonpaste.org
User-Agent: UserAgent/1.0 (identifier-version) library/7.0 otherlibrary/0.8

------------------------------deb95b63e42a
Content-Disposition: form-data; name="foo"

foo
------------------------------deb95b63e42a
Content-Disposition: form-data; name="bar"; filename="bar.txt"
Content-type: application/octet-stream

these are the contents of the file 'bar.txt'

------------------------------deb95b63e42a--
"""

_test_req2 = """
POST / HTTP/1.0
Content-Length: 0

"""

_test_req = _norm_req(_test_req)
_test_req2 = _norm_req(_test_req2) + '\r\n'


class TestRequest(test_base.BaseTestCase):
    def test_charset(self):
        req = webapp2.Request.blank('/', environ={
            'CONTENT_TYPE': 'text/html; charset=ISO-8859-4',
        })
        self.assertEqual(req.content_type, 'text/html')
        self.assertEqual(req.charset, 'iso-8859-4')

        req = webapp2.Request.blank('/', environ={
            'CONTENT_TYPE': 'application/json; charset="ISO-8859-1"',
        })
        self.assertEqual(req.content_type, 'application/json')
        self.assertEqual(req.charset, 'iso-8859-1')

        req = webapp2.Request.blank('/', environ={
            'CONTENT_TYPE': 'application/json',
        })
        self.assertEqual(req.content_type, 'application/json')
        self.assertEqual(req.charset.lower(), 'utf-8')

        match = webapp2._charset_re.search('text/html')
        if match:
            charset = match.group(1).lower().strip().strip('"').strip()
        else:
            charset = 'utf-8'
        self.assertEqual(charset, 'utf-8')

        match = webapp2._charset_re.search('text/html; charset=ISO-8859-4')
        if match:
            charset = match.group(1).lower().strip().strip('"').strip()
        else:
            charset = 'utf-8'
        self.assertEqual(charset, 'iso-8859-4')

        match = webapp2._charset_re.search('text/html; charset="ISO-8859-4"')
        if match:
            charset = match.group(1).lower().strip().strip('"').strip()
        else:
            charset = 'utf-8'
        self.assertEqual(charset, 'iso-8859-4')

        match = webapp2._charset_re.search('text/html; charset=  "  ISO-8859-4  "  ')
        if match:
            charset = match.group(1).lower().strip().strip('"').strip()
        else:
            charset = 'utf-8'
        self.assertEqual(charset, 'iso-8859-4')

    def test_unicode(self):
        req = webapp2.Request.blank('/?1=2', POST='3=4')

        res = req.GET.get('1')
        self.assertEqual(res, '2')
        self.assertTrue(isinstance(res, unicode))

        res = req.str_GET.get('1')
        self.assertEqual(res, '2')
        self.assertTrue(isinstance(res, str))

        res = req.POST.get('3')
        self.assertEqual(res, '4')
        self.assertTrue(isinstance(res, unicode))

        res = req.str_POST.get('3')
        self.assertEqual(res, '4')
        self.assertTrue(isinstance(res, str))

    def test_cookie_unicode(self):
        import urllib
        import base64

        # With base64 ---------------------------------------------------------

        value = base64.b64encode(u'á'.encode('utf-8'))
        rsp = webapp2.Response()
        rsp.set_cookie('foo', value)

        cookie = rsp.headers.get('Set-Cookie')
        req = webapp2.Request.blank('/', headers=[('Cookie', cookie)])

        self.assertEqual(req.cookies.get('foo'), value)
        self.assertEqual(base64.b64decode(req.cookies.get('foo')).decode('utf-8'), u'á')

        # Without quote -------------------------------------------------------

        # Most recent WebOb versions take care of quoting.
        # (not the version available on App Engine though)

        value = u'föö=bär; föo, bär, bäz=dïng;'
        rsp = webapp2.Response()
        rsp.set_cookie('foo', value)

        cookie = rsp.headers.get('Set-Cookie')
        req = webapp2.Request.blank('/', headers=[('Cookie', cookie)])

        self.assertEqual(req.cookies.get('foo'), value)

        # With quote, hard way ------------------------------------------------

        # Here is our test value.
        x = u'föö'
        # We must store cookies quoted. To quote unicode, we need to encode it.
        y = urllib.quote(x.encode('utf8'))
        # The encoded, quoted string looks ugly.
        self.assertEqual(y, 'f%C3%B6%C3%B6')
        # But it is easy to get it back to our initial value.
        z = urllib.unquote(y).decode('utf8')
        # And it is indeed the same value.
        self.assertEqual(z, x)

        # Set a cookie using the encoded/quoted value.
        rsp = webapp2.Response()
        rsp.set_cookie('foo', y)
        cookie = rsp.headers.get('Set-Cookie')
        self.assertEqual(cookie, 'foo=f%C3%B6%C3%B6; Path=/')

        # Get the cookie back.
        req = webapp2.Request.blank('/', headers=[('Cookie', cookie)])
        self.assertEqual(req.cookies.get('foo'), y)
        # Here is our original value, again. Problem: the value is decoded
        # before we had a chance to unquote it.
        w = urllib.unquote(req.cookies.get('foo').encode('utf8')).decode('utf8')
        # And it is indeed the same value.
        self.assertEqual(w, x)

        # With quote, easy way ------------------------------------------------

        value = u'föö=bär; föo, bär, bäz=dïng;'
        quoted_value = urllib.quote(value.encode('utf8'))
        rsp = webapp2.Response()
        rsp.set_cookie('foo', quoted_value)

        cookie = rsp.headers.get('Set-Cookie')
        req = webapp2.Request.blank('/', headers=[('Cookie', cookie)])

        cookie_value = req.str_cookies.get('foo')
        unquoted_cookie_value = urllib.unquote(cookie_value).decode('utf-8')
        self.assertEqual(cookie_value, quoted_value)
        self.assertEqual(unquoted_cookie_value, value)

    def test_get(self):
        req = webapp2.Request.blank('/?1=2&1=3&3=4', POST='5=6&7=8')

        res = req.get('1')
        self.assertEqual(res, '2')

        res = req.get('1', allow_multiple=True)
        self.assertEqual(res, ['2', '3'])

        res = req.get('8')
        self.assertEqual(res, '')

        res = req.get('8', allow_multiple=True)
        self.assertEqual(res, [])

        res = req.get('8', default_value='9')
        self.assertEqual(res, '9')

    def test_get_with_POST(self):
        req = webapp2.Request.blank('/?1=2&1=3&3=4', POST={5: 6, 7: 8},
                                    unicode_errors='ignore')

        res = req.get('1')
        self.assertEqual(res, '2')

        res = req.get('1', allow_multiple=True)
        self.assertEqual(res, ['2', '3'])

        res = req.get('8')
        self.assertEqual(res, '')

        res = req.get('8', allow_multiple=True)
        self.assertEqual(res, [])

        res = req.get('8', default_value='9')
        self.assertEqual(res, '9')

    def test_arguments(self):
        req = webapp2.Request.blank('/?1=2&3=4', POST='5=6&7=8')

        res = req.arguments()
        self.assertEqual(res, ['1', '3', '5', '7'])

    def test_get_range(self):
        req = webapp2.Request.blank('/')
        res = req.get_range('1', min_value=None, max_value=None, default=None)
        self.assertEqual(res, None)

        req = webapp2.Request.blank('/?1=2')
        res = req.get_range('1', min_value=None, max_value=None, default=0)
        self.assertEqual(res, 2)

        req = webapp2.Request.blank('/?1=foo')
        res = req.get_range('1', min_value=1, max_value=99, default=100)
        self.assertEqual(res, 99)

    def test_issue_3426(self):
        """When the content-type is 'application/x-www-form-urlencoded' and
        POST data is empty the content-type is dropped by Google appengine.
        """
        req = webapp2.Request.blank('/', environ={
            'REQUEST_METHOD': 'GET',
            'CONTENT_TYPE': 'application/x-www-form-urlencoded',
        })
        self.assertEqual(req.method, 'GET')
        self.assertEqual(req.content_type, 'application/x-www-form-urlencoded')

    # XXX: These tests fail when request charset is set to utf-8 by default.
    # Otherwise they pass.
    '''
    def test_get_with_FieldStorage(self):
        if not test_base.check_webob_version(1.0):
            return
        # A valid request without a Content-Length header should still read
        # the full body.
        # Also test parity between as_string and from_string / from_file.
        import cgi
        req = webapp2.Request.from_string(_test_req)
        self.assertTrue(isinstance(req, webapp2.Request))
        self.assertTrue(not repr(req).endswith('(invalid WSGI environ)>'))
        self.assertTrue('\n' not in req.http_version or '\r' in req.http_version)
        self.assertTrue(',' not in req.host)
        self.assertTrue(req.content_length is not None)
        self.assertEqual(req.content_length, 337)
        self.assertTrue('foo' in req.body)
        bar_contents = "these are the contents of the file 'bar.txt'\r\n"
        self.assertTrue(bar_contents in req.body)
        self.assertEqual(req.params['foo'], 'foo')
        bar = req.params['bar']
        self.assertTrue(isinstance(bar, cgi.FieldStorage))
        self.assertEqual(bar.type, 'application/octet-stream')
        bar.file.seek(0)
        self.assertEqual(bar.file.read(), bar_contents)

        bar = req.get_all('bar')
        self.assertEqual(bar[0], bar_contents)

        # out should equal contents, except for the Content-Length header,
        # so insert that.
        _test_req_copy = _test_req.replace('Content-Type',
                            'Content-Length: 337\r\nContent-Type')
        self.assertEqual(str(req), _test_req_copy)

        req2 = webapp2.Request.from_string(_test_req2)
        self.assertTrue('host' not in req2.headers)
        self.assertEqual(str(req2), _test_req2.rstrip())
        self.assertRaises(ValueError,
                          webapp2.Request.from_string, _test_req2 + 'xx')

    def test_issue_5118(self):
        """Unable to read POST variables ONCE self.request.body is read."""
        if not test_base.check_webob_version(1.0):
            return
        import cgi
        req = webapp2.Request.from_string(_test_req)
        fieldStorage = req.POST.get('bar')
        self.assertTrue(isinstance(fieldStorage, cgi.FieldStorage))
        self.assertEqual(fieldStorage.type, 'application/octet-stream')
        # Double read.
        fieldStorage = req.POST.get('bar')
        self.assertTrue(isinstance(fieldStorage, cgi.FieldStorage))
        self.assertEqual(fieldStorage.type, 'application/octet-stream')
        # Now read the body.
        x = req.body
        fieldStorage = req.POST.get('bar')
        self.assertTrue(isinstance(fieldStorage, cgi.FieldStorage))
        self.assertEqual(fieldStorage.type, 'application/octet-stream')
    '''

if __name__ == '__main__':
    test_base.main()
