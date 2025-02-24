# -*- coding: utf-8 -*-
"""
Tests for webapp2 webapp2.RequestHandler
"""
import os
import StringIO
import sys
import urllib

import webapp2

import test_base


class BareHandler(object):
    def __init__(self, request, response):
        self.response = response
        response.write('I am not a RequestHandler but I work.')

    def dispatch(self):
        return self.response


class HomeHandler(webapp2.RequestHandler):
    def get(self, **kwargs):
        self.response.out.write('home sweet home')

    def post(self, **kwargs):
        self.response.out.write('home sweet home - POST')


class MethodsHandler(HomeHandler):
    def put(self, **kwargs):
        self.response.out.write('home sweet home - PUT')

    def delete(self, **kwargs):
        self.response.out.write('home sweet home - DELETE')

    def head(self, **kwargs):
        self.response.out.write('home sweet home - HEAD')

    def trace(self, **kwargs):
        self.response.out.write('home sweet home - TRACE')

    def options(self, **kwargs):
        self.response.out.write('home sweet home - OPTIONS')


class RedirectToHandler(webapp2.RequestHandler):
    def get(self, **kwargs):
        return self.redirect_to('route-test', _fragment='my-anchor', year='2010',
                                month='07', name='test', foo='bar')


class RedirectAbortHandler(webapp2.RequestHandler):
    def get(self, **kwargs):
        self.redirect('/somewhere', abort=True)


class BrokenHandler(webapp2.RequestHandler):
    def get(self, **kwargs):
        raise ValueError('booo!')


class BrokenButFixedHandler(BrokenHandler):
    def handle_exception(self, exception, debug_mode):
        # Let's fix it.
        self.response.set_status(200)
        self.response.out.write('that was close!')


def handle_404(request, response, exception):
    response.out.write('404 custom handler')
    response.set_status(404)


def handle_405(request, response, exception):
    response.out.write('405 custom handler')
    response.set_status(405, 'Custom Error Message')
    response.headers['Allow'] = 'GET'


def handle_500(request, response, exception):
    response.out.write('500 custom handler')
    response.set_status(500)


class PositionalHandler(webapp2.RequestHandler):
    def get(self, month, day, slug=None):
        self.response.out.write('%s:%s:%s' % (month, day, slug))


class HandlerWithError(webapp2.RequestHandler):
    def get(self, **kwargs):
        self.response.out.write('bla bla bla bla bla bla')
        self.error(403)


class InitializeHandler(webapp2.RequestHandler):
    def __init__(self):
        pass

    def get(self):
        self.response.out.write('Request method: %s' % self.request.method)


class WebDavHandler(webapp2.RequestHandler):
    def version_control(self):
        self.response.out.write('Method: VERSION-CONTROL')

    def unlock(self):
        self.response.out.write('Method: UNLOCK')

    def propfind(self):
        self.response.out.write('Method: PROPFIND')


class AuthorizationHandler(webapp2.RequestHandler):
    def get(self):
        self.response.out.write('nothing here')

class HandlerWithEscapedArg(webapp2.RequestHandler):
    def get(self, name):
        self.response.out.write(urllib.unquote_plus(name))

def get_redirect_url(handler, **kwargs):
    return handler.uri_for('methods')


app = webapp2.WSGIApplication([
    ('/bare', BareHandler),
    webapp2.Route('/', HomeHandler, name='home'),
    webapp2.Route('/methods', MethodsHandler, name='methods'),
    webapp2.Route('/broken', BrokenHandler),
    webapp2.Route('/broken-but-fixed', BrokenButFixedHandler),
    webapp2.Route('/<year:\d{4}>/<month:\d{1,2}>/<name>', None, name='route-test'),
    webapp2.Route('/<:\d\d>/<:\d{2}>/<:\w+>', PositionalHandler, name='positional'),
    webapp2.Route('/redirect-me', webapp2.RedirectHandler, defaults={'_uri': '/broken'}),
    webapp2.Route('/redirect-me2', webapp2.RedirectHandler, defaults={'_uri': get_redirect_url}),
    webapp2.Route('/redirect-me3', webapp2.RedirectHandler, defaults={'_uri': '/broken', '_permanent': False}),
    webapp2.Route('/redirect-me4', webapp2.RedirectHandler, defaults={'_uri': get_redirect_url, '_permanent': False}),
    webapp2.Route('/redirect-me5', RedirectToHandler),
    webapp2.Route('/redirect-me6', RedirectAbortHandler),
    webapp2.Route('/lazy', 'resources.handlers.LazyHandler'),
    webapp2.Route('/error', HandlerWithError),
    webapp2.Route('/initialize', InitializeHandler),
    webapp2.Route('/webdav', WebDavHandler),
    webapp2.Route('/authorization', AuthorizationHandler),
    webapp2.Route('/escape/<name:.*>', HandlerWithEscapedArg, 'escape'),
], debug=False)

DEFAULT_RESPONSE = """Status: 404 Not Found
content-type: text/html; charset=utf8
Content-Length: 52

404 Not Found

The resource could not be found.

   """

class TestHandler(test_base.BaseTestCase):
    def tearDown(self):
        super(TestHandler, self).tearDown()
        app.error_handlers = {}

    def test_200(self):
        rsp = app.get_response('/')
        self.assertEqual(rsp.status_int, 200)
        self.assertEqual(rsp.body, 'home sweet home')

    def test_404(self):
        req = webapp2.Request.blank('/nowhere')
        rsp = req.get_response(app)
        self.assertEqual(rsp.status_int, 404)

    def test_405(self):
        req = webapp2.Request.blank('/')
        req.method = 'PUT'
        rsp = req.get_response(app)
        self.assertEqual(rsp.status_int, 405)
        self.assertEqual(rsp.headers.get('Allow'), 'GET, POST')

    def test_500(self):
        req = webapp2.Request.blank('/broken')
        rsp = req.get_response(app)
        self.assertEqual(rsp.status_int, 500)

    def test_500_but_fixed(self):
        req = webapp2.Request.blank('/broken-but-fixed')
        rsp = req.get_response(app)
        self.assertEqual(rsp.status_int, 200)
        self.assertEqual(rsp.body, 'that was close!')

    def test_501(self):
        # 501 Not Implemented
        req = webapp2.Request.blank('/methods')
        req.method = 'FOOBAR'
        rsp = req.get_response(app)
        self.assertEqual(rsp.status_int, 501)

    def test_lazy_handler(self):
        req = webapp2.Request.blank('/lazy')
        rsp = req.get_response(app)
        self.assertEqual(rsp.status_int, 200)
        self.assertEqual(rsp.body, 'I am a laaazy view.')

    def test_handler_with_error(self):
        req = webapp2.Request.blank('/error')
        rsp = req.get_response(app)
        self.assertEqual(rsp.status_int, 403)
        self.assertEqual(rsp.body, '')

    def test_debug_mode(self):
        app = webapp2.WSGIApplication([
            webapp2.Route('/broken', BrokenHandler),
        ], debug=True)
        req = webapp2.Request.blank('/broken')
        rsp = req.get_response(app)
        self.assertEqual(rsp.status_int, 500)

    def test_custom_error_handlers(self):
        app.error_handlers = {
            404: handle_404,
            405: handle_405,
            500: handle_500,
        }
        req = webapp2.Request.blank('/nowhere')
        rsp = req.get_response(app)
        self.assertEqual(rsp.status_int, 404)
        self.assertEqual(rsp.body, '404 custom handler')

        req = webapp2.Request.blank('/')
        req.method = 'PUT'
        rsp = req.get_response(app)
        self.assertEqual(rsp.status, '405 Custom Error Message')
        self.assertEqual(rsp.body, '405 custom handler')
        self.assertEqual(rsp.headers.get('Allow'), 'GET')

        req = webapp2.Request.blank('/broken')
        rsp = req.get_response(app)
        self.assertEqual(rsp.status_int, 500)
        self.assertEqual(rsp.body, '500 custom handler')

    def test_methods(self):
        app.debug = True
        req = webapp2.Request.blank('/methods')
        rsp = req.get_response(app)
        self.assertEqual(rsp.status_int, 200)
        self.assertEqual(rsp.body, 'home sweet home')

        req = webapp2.Request.blank('/methods')
        req.method = 'POST'
        rsp = req.get_response(app)
        self.assertEqual(rsp.status_int, 200)
        self.assertEqual(rsp.body, 'home sweet home - POST')

        req = webapp2.Request.blank('/methods')
        req.method = 'PUT'
        rsp = req.get_response(app)
        self.assertEqual(rsp.status_int, 200)
        self.assertEqual(rsp.body, 'home sweet home - PUT')

        req = webapp2.Request.blank('/methods')
        req.method = 'DELETE'
        rsp = req.get_response(app)
        self.assertEqual(rsp.status_int, 200)
        self.assertEqual(rsp.body, 'home sweet home - DELETE')

        req = webapp2.Request.blank('/methods')
        req.method = 'HEAD'
        rsp = req.get_response(app)
        self.assertEqual(rsp.status_int, 200)
        self.assertEqual(rsp.body, '')

        req = webapp2.Request.blank('/methods')
        req.method = 'OPTIONS'
        rsp = req.get_response(app)
        self.assertEqual(rsp.status_int, 200)
        self.assertEqual(rsp.body, 'home sweet home - OPTIONS')

        req = webapp2.Request.blank('/methods')
        req.method = 'TRACE'
        rsp = req.get_response(app)
        self.assertEqual(rsp.status_int, 200)
        self.assertEqual(rsp.body, 'home sweet home - TRACE')
        app.debug = False

    def test_positional(self):
        req = webapp2.Request.blank('/07/31/test')
        rsp = req.get_response(app)
        self.assertEqual(rsp.status_int, 200)
        self.assertEqual(rsp.body, '07:31:test')

        req = webapp2.Request.blank('/10/18/wooohooo')
        rsp = req.get_response(app)
        self.assertEqual(rsp.status_int, 200)
        self.assertEqual(rsp.body, '10:18:wooohooo')

    def test_redirect(self):
        req = webapp2.Request.blank('/redirect-me')
        rsp = req.get_response(app)
        self.assertEqual(rsp.status_int, 301)
        self.assertEqual(rsp.body, '')
        self.assertEqual(rsp.headers['Location'], 'http://localhost/broken')

    def test_redirect_with_callable(self):
        req = webapp2.Request.blank('/redirect-me2')
        rsp = req.get_response(app)
        self.assertEqual(rsp.status_int, 301)
        self.assertEqual(rsp.body, '')
        self.assertEqual(rsp.headers['Location'], 'http://localhost/methods')

    def test_redirect_not_permanent(self):
        req = webapp2.Request.blank('/redirect-me3')
        rsp = req.get_response(app)
        self.assertEqual(rsp.status_int, 302)
        self.assertEqual(rsp.body, '')
        self.assertEqual(rsp.headers['Location'], 'http://localhost/broken')

    def test_redirect_with_callable_not_permanent(self):
        req = webapp2.Request.blank('/redirect-me4')
        rsp = req.get_response(app)
        self.assertEqual(rsp.status_int, 302)
        self.assertEqual(rsp.body, '')
        self.assertEqual(rsp.headers['Location'], 'http://localhost/methods')

    def test_redirect_to(self):
        req = webapp2.Request.blank('/redirect-me5')
        rsp = req.get_response(app)
        self.assertEqual(rsp.status_int, 302)
        self.assertEqual(rsp.body, '')
        self.assertEqual(rsp.headers['Location'], 'http://localhost/2010/07/test?foo=bar#my-anchor')

    def test_redirect_abort(self):
        req = webapp2.Request.blank('/redirect-me6')
        rsp = req.get_response(app)
        self.assertEqual(rsp.status_int, 302)
        self.assertEqual(rsp.body, """302 Moved Temporarily

The resource was found at http://localhost/somewhere; you should be redirected automatically.  """)
        self.assertEqual(rsp.headers['Location'], 'http://localhost/somewhere')

    def test_run(self):
        os.environ['REQUEST_METHOD'] = 'GET'

        app.run()
        #self.assertEqual(sys.stdout.read(), DEFAULT_RESPONSE)

    def test_run_bare(self):
        os.environ['REQUEST_METHOD'] = 'GET'
        app.run(bare=True)

        #self.assertEqual(sys.stdout.read(), DEFAULT_RESPONSE)

    def test_run_debug(self):
        debug = app.debug
        app.debug = True
        os.environ['REQUEST_METHOD'] = 'GET'
        os.environ['PATH_INFO'] = '/'

        res = app.run(bare=True)
        #self.assertEqual(sys.stdout.read(), DEFAULT_RESPONSE)

        app.debug = debug

    '''
    def test_get_valid_methods(self):
        req = webapp2.Request.blank('http://localhost:80/')
        req.app = app
        app.set_globals(app=app, request=req)

        handler = BrokenHandler(req, None)
        handler.app = app
        self.assertEqual(handler.get_valid_methods().sort(), ['GET'].sort())

        handler = HomeHandler(req, None)
        handler.app = app
        self.assertEqual(handler.get_valid_methods().sort(),
            ['GET', 'POST'].sort())

        handler = MethodsHandler(req, None)
        handler.app = app
        self.assertEqual(handler.get_valid_methods().sort(),
            ['GET', 'POST', 'HEAD', 'OPTIONS', 'PUT', 'DELETE', 'TRACE'].sort())
    '''

    def test_uri_for(self):
        class Handler(webapp2.RequestHandler):
            def get(self, *args, **kwargs):
                pass

        req = webapp2.Request.blank('http://localhost:80/')
        req.route = webapp2.Route('')
        req.route_args = tuple()
        req.route_kwargs = {}
        req.app = app
        app.set_globals(app=app, request=req)
        handler = Handler(req, webapp2.Response())
        handler.app = app

        for func in (handler.uri_for,):
            self.assertEqual(func('home'), '/')
            self.assertEqual(func('home', foo='bar'), '/?foo=bar')
            self.assertEqual(func('home', _fragment='my-anchor', foo='bar'), '/?foo=bar#my-anchor')
            self.assertEqual(func('home', _fragment='my-anchor'), '/#my-anchor')
            self.assertEqual(func('home', _full=True), 'http://localhost:80/')
            self.assertEqual(func('home', _full=True, _fragment='my-anchor'), 'http://localhost:80/#my-anchor')
            self.assertEqual(func('home', _scheme='https'), 'https://localhost:80/')
            self.assertEqual(func('home', _scheme='https', _full=False), 'https://localhost:80/')
            self.assertEqual(func('home', _scheme='https', _fragment='my-anchor'), 'https://localhost:80/#my-anchor')

            self.assertEqual(func('methods'), '/methods')
            self.assertEqual(func('methods', foo='bar'), '/methods?foo=bar')
            self.assertEqual(func('methods', _fragment='my-anchor', foo='bar'), '/methods?foo=bar#my-anchor')
            self.assertEqual(func('methods', _fragment='my-anchor'), '/methods#my-anchor')
            self.assertEqual(func('methods', _full=True), 'http://localhost:80/methods')
            self.assertEqual(func('methods', _full=True, _fragment='my-anchor'), 'http://localhost:80/methods#my-anchor')
            self.assertEqual(func('methods', _scheme='https'), 'https://localhost:80/methods')
            self.assertEqual(func('methods', _scheme='https', _full=False), 'https://localhost:80/methods')
            self.assertEqual(func('methods', _scheme='https', _fragment='my-anchor'), 'https://localhost:80/methods#my-anchor')

            self.assertEqual(func('route-test', year='2010', month='0', name='test'), '/2010/0/test')
            self.assertEqual(func('route-test', year='2010', month='07', name='test'), '/2010/07/test')
            self.assertEqual(func('route-test', year='2010', month='07', name='test', foo='bar'), '/2010/07/test?foo=bar')
            self.assertEqual(func('route-test', _fragment='my-anchor', year='2010', month='07', name='test', foo='bar'), '/2010/07/test?foo=bar#my-anchor')
            self.assertEqual(func('route-test', _fragment='my-anchor', year='2010', month='07', name='test'), '/2010/07/test#my-anchor')
            self.assertEqual(func('route-test', _full=True, year='2010', month='07', name='test'), 'http://localhost:80/2010/07/test')
            self.assertEqual(func('route-test', _full=True, _fragment='my-anchor', year='2010', month='07', name='test'), 'http://localhost:80/2010/07/test#my-anchor')
            self.assertEqual(func('route-test', _scheme='https', year='2010', month='07', name='test'), 'https://localhost:80/2010/07/test')
            self.assertEqual(func('route-test', _scheme='https', _full=False, year='2010', month='07', name='test'), 'https://localhost:80/2010/07/test')
            self.assertEqual(func('route-test', _scheme='https', _fragment='my-anchor', year='2010', month='07', name='test'), 'https://localhost:80/2010/07/test#my-anchor')

    def test_extra_request_methods(self):
        allowed_methods_backup = app.allowed_methods
        webdav_methods = ('VERSION-CONTROL', 'UNLOCK', 'PROPFIND')

        for method in webdav_methods:
            # It is still not possible to use WebDav methods...
            req = webapp2.Request.blank('/webdav')
            req.method = method
            rsp = req.get_response(app)
            self.assertEqual(rsp.status_int, 501)

        # Let's extend ALLOWED_METHODS with some WebDav methods.
        app.allowed_methods = tuple(app.allowed_methods) + webdav_methods

        #self.assertEqual(sorted(webapp2.get_valid_methods(WebDavHandler)), sorted(list(webdav_methods)))

        # Now we can use WebDav methods...
        for method in webdav_methods:
            req = webapp2.Request.blank('/webdav')
            req.method = method
            rsp = req.get_response(app)
            self.assertEqual(rsp.status_int, 200)
            self.assertEqual(rsp.body, 'Method: %s' % method)

        # Restore initial values.
        app.allowed_methods = allowed_methods_backup
        self.assertEqual(len(app.allowed_methods), 7)

    def test_escaping(self):
        def get_req(uri):
            req = webapp2.Request.blank(uri)
            app.set_globals(app=app, request=req)
            handler = webapp2.RequestHandler(req, None)
            handler.app = req.app = app
            return req, handler

        req, handler = get_req('http://localhost:80/')
        uri = webapp2.uri_for('escape', name='with space')
        req, handler = get_req(uri)
        rsp = req.get_response(app)
        self.assertEqual(rsp.status_int, 200)
        self.assertEqual(rsp.body, 'with space')

        req, handler = get_req('http://localhost:80/')
        uri = webapp2.uri_for('escape', name='with+plus')
        req, handler = get_req(uri)
        rsp = req.get_response(app)
        self.assertEqual(rsp.status_int, 200)
        self.assertEqual(rsp.body, 'with plus')

        req, handler = get_req('http://localhost:80/')
        uri = webapp2.uri_for('escape', name='with/slash')
        req, handler = get_req(uri)
        rsp = req.get_response(app)
        self.assertEqual(rsp.status_int, 200)
        self.assertEqual(rsp.body, 'with/slash')

    def test_handle_exception_with_error(self):
        class HomeHandler(webapp2.RequestHandler):
            def get(self, **kwargs):
                raise TypeError()

        def handle_exception(request, response, exception):
            raise ValueError()

        app = webapp2.WSGIApplication([
            webapp2.Route('/', HomeHandler, name='home'),
        ], debug=False)
        app.error_handlers[500] = handle_exception

        req = webapp2.Request.blank('/')
        rsp = req.get_response(app)
        self.assertEqual(rsp.status_int, 500)

    def test_handle_exception_with_error_debug(self):
        class HomeHandler(webapp2.RequestHandler):
            def get(self, **kwargs):
                raise TypeError()

        def handle_exception(request, response, exception):
            raise ValueError()

        app = webapp2.WSGIApplication([
            webapp2.Route('/', HomeHandler, name='home'),
        ], debug=True)
        app.error_handlers[500] = handle_exception

        req = webapp2.Request.blank('/')
        rsp = req.get_response(app)
        self.assertEqual(rsp.status_int, 500)

    def test_function_handler(self):
        def my_view(request, *args, **kwargs):
            return webapp2.Response('Hello, function world!')

        def other_view(request, *args, **kwargs):
            return webapp2.Response('Hello again, function world!')

        '''
        def one_more_view(request, response):
            self.assertEqual(request.route_args, ())
            self.assertEqual(request.route_kwargs, {'foo': 'bar'})
            response.write('Hello you too, deprecated arguments world!')
        '''
        def one_more_view(request, *args, **kwargs):
            self.assertEqual(args, ())
            self.assertEqual(kwargs, {'foo': 'bar'})
            return webapp2.Response('Hello you too!')

        app = webapp2.WSGIApplication([
            webapp2.Route('/', my_view),
            webapp2.Route('/other', other_view),
            webapp2.Route('/one-more/<foo>', one_more_view),
            #webapp2.Route('/one-more/<foo>', one_more_view),
        ])

        req = webapp2.Request.blank('/')
        rsp = req.get_response(app)
        self.assertEqual(rsp.status_int, 200)
        self.assertEqual(rsp.body, 'Hello, function world!')

        # Twice to test factory.
        req = webapp2.Request.blank('/')
        rsp = req.get_response(app)
        self.assertEqual(rsp.status_int, 200)
        self.assertEqual(rsp.body, 'Hello, function world!')

        req = webapp2.Request.blank('/other')
        rsp = req.get_response(app)
        self.assertEqual(rsp.status_int, 200)
        self.assertEqual(rsp.body, 'Hello again, function world!')

        # Twice to test factory.
        req = webapp2.Request.blank('/other')
        rsp = req.get_response(app)
        self.assertEqual(rsp.status_int, 200)
        self.assertEqual(rsp.body, 'Hello again, function world!')

        req = webapp2.Request.blank('/one-more/bar')
        rsp = req.get_response(app)
        self.assertEqual(rsp.status_int, 200)
        self.assertEqual(rsp.body, 'Hello you too!')

    def test_custom_method(self):
        class MyHandler(webapp2.RequestHandler):
            def my_method(self):
                self.response.out.write('Hello, custom method world!')

            def my_other_method(self):
                self.response.out.write('Hello again, custom method world!')

        app = webapp2.WSGIApplication([
            webapp2.Route('/', MyHandler, handler_method='my_method'),
            webapp2.Route('/other', MyHandler, handler_method='my_other_method'),
        ])

        req = webapp2.Request.blank('/')
        rsp = req.get_response(app)
        self.assertEqual(rsp.status_int, 200)
        self.assertEqual(rsp.body, 'Hello, custom method world!')

        req = webapp2.Request.blank('/other')
        rsp = req.get_response(app)
        self.assertEqual(rsp.status_int, 200)
        self.assertEqual(rsp.body, 'Hello again, custom method world!')

    def test_custom_method_with_string(self):
        app = webapp2.WSGIApplication([
            webapp2.Route('/', handler='resources.handlers.CustomMethodHandler:custom_method'),
            webapp2.Route('/bleh', handler='resources.handlers.CustomMethodHandler:custom_method'),
        ])

        req = webapp2.Request.blank('/')
        rsp = req.get_response(app)
        self.assertEqual(rsp.status_int, 200)
        self.assertEqual(rsp.body, 'I am a custom method.')

        req = webapp2.Request.blank('/bleh')
        rsp = req.get_response(app)
        self.assertEqual(rsp.status_int, 200)
        self.assertEqual(rsp.body, 'I am a custom method.')

        self.assertRaises(ValueError, webapp2.Route, '/', handler='resources.handlers.CustomMethodHandler:custom_method', handler_method='custom_method')

    def test_factory_1(self):
        app.debug = True
        rsp = app.get_response('/bare')
        self.assertEqual(rsp.status_int, 200)
        self.assertEqual(rsp.body, 'I am not a RequestHandler but I work.')
        app.debug = False

    def test_factory_2(self):
        """Very crazy stuff. Please ignore it."""
        class MyHandler(object):
            def __init__(self, request, response):
                self.request = request
                self.response = response

            def __new__(cls, *args, **kwargs):
                return cls.create_instance(*args, **kwargs)()

            @classmethod
            def create_instance(cls, *args, **kwargs):
                obj = object.__new__(cls)
                if isinstance(obj, cls):
                    obj.__init__(*args, **kwargs)

                return obj

            def __call__(self):
                return self

            def dispatch(self):
                self.response.write('hello')

        app = webapp2.WSGIApplication([
            webapp2.Route('/', handler=MyHandler),
        ])

        req = webapp2.Request.blank('/')
        rsp = req.get_response(app)
        self.assertEqual(rsp.status_int, 200)
        self.assertEqual(rsp.body, 'hello')

    def test_encoding(self):
        class PostHandler(webapp2.RequestHandler):
            def post(self):
                foo = self.request.POST['foo']
                if not foo:
                    foo = 'empty'

                self.response.write(foo)

        app = webapp2.WSGIApplication([
            webapp2.Route('/', PostHandler),
        ], debug=True)

        # foo with umlauts in the vowels.
        value = 'f\xc3\xb6\xc3\xb6'

        rsp = app.get_response('/', POST={'foo': value},
            headers=[('Content-Type', 'application/x-www-form-urlencoded; charset=utf-8')])
        self.assertEqual(rsp.body, 'föö')

        rsp = app.get_response('/', POST={'foo': value},
            headers=[('Content-Type', 'application/x-www-form-urlencoded')])
        self.assertEqual(rsp.body, 'föö')


if __name__ == '__main__':
    test_base.main()
