# -*- coding: utf-8 -*-
import webob
import webob.exc

import webapp2

import test_base


class TestMiscellaneous(test_base.BaseTestCase):

    def test_abort(self):
        self.assertRaises(webob.exc.HTTPOk, webapp2.abort, 200)
        self.assertRaises(webob.exc.HTTPCreated, webapp2.abort, 201)
        self.assertRaises(webob.exc.HTTPAccepted, webapp2.abort, 202)
        self.assertRaises(webob.exc.HTTPNonAuthoritativeInformation, webapp2.abort, 203)
        self.assertRaises(webob.exc.HTTPNoContent, webapp2.abort, 204)
        self.assertRaises(webob.exc.HTTPResetContent, webapp2.abort, 205)
        self.assertRaises(webob.exc.HTTPPartialContent, webapp2.abort, 206)
        self.assertRaises(webob.exc.HTTPMultipleChoices, webapp2.abort, 300)
        self.assertRaises(webob.exc.HTTPMovedPermanently, webapp2.abort, 301)
        self.assertRaises(webob.exc.HTTPFound, webapp2.abort, 302)
        self.assertRaises(webob.exc.HTTPSeeOther, webapp2.abort, 303)
        self.assertRaises(webob.exc.HTTPNotModified, webapp2.abort, 304)
        self.assertRaises(webob.exc.HTTPUseProxy, webapp2.abort, 305)
        self.assertRaises(webob.exc.HTTPTemporaryRedirect, webapp2.abort, 307)
        self.assertRaises(webob.exc.HTTPClientError, webapp2.abort, 400)
        self.assertRaises(webob.exc.HTTPUnauthorized, webapp2.abort, 401)
        self.assertRaises(webob.exc.HTTPPaymentRequired, webapp2.abort, 402)
        self.assertRaises(webob.exc.HTTPForbidden, webapp2.abort, 403)
        self.assertRaises(webob.exc.HTTPNotFound, webapp2.abort, 404)
        self.assertRaises(webob.exc.HTTPMethodNotAllowed, webapp2.abort, 405)
        self.assertRaises(webob.exc.HTTPNotAcceptable, webapp2.abort, 406)
        self.assertRaises(webob.exc.HTTPProxyAuthenticationRequired, webapp2.abort, 407)
        self.assertRaises(webob.exc.HTTPRequestTimeout, webapp2.abort, 408)
        self.assertRaises(webob.exc.HTTPConflict, webapp2.abort, 409)
        self.assertRaises(webob.exc.HTTPGone, webapp2.abort, 410)
        self.assertRaises(webob.exc.HTTPLengthRequired, webapp2.abort, 411)
        self.assertRaises(webob.exc.HTTPPreconditionFailed, webapp2.abort, 412)
        self.assertRaises(webob.exc.HTTPRequestEntityTooLarge, webapp2.abort, 413)
        self.assertRaises(webob.exc.HTTPRequestURITooLong, webapp2.abort, 414)
        self.assertRaises(webob.exc.HTTPUnsupportedMediaType, webapp2.abort, 415)
        self.assertRaises(webob.exc.HTTPRequestRangeNotSatisfiable, webapp2.abort, 416)
        self.assertRaises(webob.exc.HTTPExpectationFailed, webapp2.abort, 417)
        self.assertRaises(webob.exc.HTTPInternalServerError, webapp2.abort, 500)
        self.assertRaises(webob.exc.HTTPNotImplemented, webapp2.abort, 501)
        self.assertRaises(webob.exc.HTTPBadGateway, webapp2.abort, 502)
        self.assertRaises(webob.exc.HTTPServiceUnavailable, webapp2.abort, 503)
        self.assertRaises(webob.exc.HTTPGatewayTimeout, webapp2.abort, 504)
        self.assertRaises(webob.exc.HTTPVersionNotSupported, webapp2.abort, 505)

        # Invalid use 500 as default.
        self.assertRaises(KeyError, webapp2.abort, 0)
        self.assertRaises(KeyError, webapp2.abort, 999999)
        self.assertRaises(KeyError, webapp2.abort, 'foo')

    def test_import_string(self):
        self.assertEqual(webapp2.import_string('webob.exc'), webob.exc)
        self.assertEqual(webapp2.import_string('webob'), webob)

        self.assertEqual(webapp2.import_string('asdfg', silent=True), None)
        self.assertEqual(webapp2.import_string('webob.asdfg', silent=True), None)

        self.assertRaises(webapp2.ImportStringError, webapp2.import_string, 'asdfg')
        self.assertRaises(webapp2.ImportStringError, webapp2.import_string, 'webob.asdfg')

    def test_to_utf8(self):
        res = webapp2._to_utf8('ábcdéf'.decode('utf-8'))
        self.assertEqual(isinstance(res, str), True)

        res = webapp2._to_utf8('abcdef')
        self.assertEqual(isinstance(res, str), True)

    '''
    # removed to simplify the codebase.
    def test_to_unicode(self):
        res = webapp2.to_unicode(unicode('foo'))
        self.assertEqual(isinstance(res, unicode), True)

        res = webapp2.to_unicode('foo')
        self.assertEqual(isinstance(res, unicode), True)
    '''

    def test_http_status_message(self):
        self.assertEqual(webapp2.Response.http_status_message(404), 'Not Found')
        self.assertEqual(webapp2.Response.http_status_message(500), 'Internal Server Error')
        self.assertRaises(KeyError, webapp2.Response.http_status_message, 9999)

    def test_cached_property(self):
        count = [0]

        class Foo(object):
            @webapp2.cached_property
            def bar(self):
                count[0] += 1
                return count[0]

        self.assertTrue(isinstance(Foo.bar, webapp2.cached_property))

        foo = Foo()
        self.assertEqual(foo.bar, 1)
        self.assertEqual(foo.bar, 1)
        self.assertEqual(foo.bar, 1)

    def test_redirect(self):
        app = webapp2.WSGIApplication()
        req = webapp2.Request.blank('/')
        req.app = app
        app.set_globals(app=app, request=req)
        rsp = webapp2.redirect('http://www.google.com/', code=301, body='Weee')
        self.assertEqual(rsp.status_int, 301)
        self.assertEqual(rsp.body, 'Weee')
        self.assertEqual(rsp.headers.get('Location'), 'http://www.google.com/')

    def test_redirect_to(self):
        app = webapp2.WSGIApplication([
            webapp2.Route('/home', handler='', name='home'),
        ])
        req = webapp2.Request.blank('/')
        req.app = app
        app.set_globals(app=app, request=req)

        rsp = webapp2.redirect_to('home', _code=301, _body='Weee')
        self.assertEqual(rsp.status_int, 301)
        self.assertEqual(rsp.body, 'Weee')
        self.assertEqual(rsp.headers.get('Location'), 'http://localhost/home')


if __name__ == '__main__':
    test_base.main()
