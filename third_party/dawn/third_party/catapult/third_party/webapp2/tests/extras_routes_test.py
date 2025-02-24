# -*- coding: utf-8 -*-
import webapp2

from webapp2_extras.routes import (DomainRoute, HandlerPrefixRoute,
    RedirectRoute, NamePrefixRoute, PathPrefixRoute)

import test_base


class HomeHandler(webapp2.RequestHandler):
    def get(self, **kwargs):
        self.response.out.write('home sweet home')


app = webapp2.WSGIApplication([
    #RedirectRoute('/', name='home', handler=HomeHandler),
    RedirectRoute('/redirect-me-easily', redirect_to='/i-was-redirected-easily'),
    RedirectRoute('/redirect-me-easily2', redirect_to='/i-was-redirected-easily', defaults={'_code': 302}),
    RedirectRoute('/redirect-me-easily3', redirect_to='/i-was-redirected-easily', defaults={'_permanent': False}),
    RedirectRoute('/strict-foo', HomeHandler, 'foo-strict', strict_slash=True),
    RedirectRoute('/strict-bar/', HomeHandler, 'bar-strict', strict_slash=True),
    RedirectRoute('/redirect-to-name-destination', name='redirect-to-name-destination', handler=HomeHandler),
    RedirectRoute('/redirect-to-name', redirect_to_name='redirect-to-name-destination'),
])


class TestRedirectRoute(test_base.BaseTestCase):
    def test_route_redirect_to(self):
        route = RedirectRoute('/foo', redirect_to='/bar')
        router = webapp2.Router([route])
        route_match, args, kwargs = router.match(webapp2.Request.blank('/foo'))
        self.assertEqual(route_match, route)
        self.assertEqual(args, ())
        self.assertEqual(kwargs, {'_uri': '/bar'})

    def test_easy_redirect_to(self):
        req = webapp2.Request.blank('/redirect-me-easily')
        rsp = req.get_response(app)
        self.assertEqual(rsp.status_int, 301)
        self.assertEqual(rsp.body, '')
        self.assertEqual(rsp.headers['Location'], 'http://localhost/i-was-redirected-easily')

        req = webapp2.Request.blank('/redirect-me-easily2')
        rsp = req.get_response(app)
        self.assertEqual(rsp.status_int, 302)
        self.assertEqual(rsp.body, '')
        self.assertEqual(rsp.headers['Location'], 'http://localhost/i-was-redirected-easily')

        req = webapp2.Request.blank('/redirect-me-easily3')
        rsp = req.get_response(app)
        self.assertEqual(rsp.status_int, 302)
        self.assertEqual(rsp.body, '')
        self.assertEqual(rsp.headers['Location'], 'http://localhost/i-was-redirected-easily')

    def test_redirect_to_name(self):
        req = webapp2.Request.blank('/redirect-to-name')
        rsp = req.get_response(app)
        self.assertEqual(rsp.status_int, 301)
        self.assertEqual(rsp.body, '')
        self.assertEqual(rsp.headers['Location'], 'http://localhost/redirect-to-name-destination')

    def test_strict_slash(self):
        req = webapp2.Request.blank('/strict-foo')
        rsp = req.get_response(app)
        self.assertEqual(rsp.status_int, 200)
        self.assertEqual(rsp.body, 'home sweet home')

        req = webapp2.Request.blank('/strict-bar/')
        rsp = req.get_response(app)
        self.assertEqual(rsp.status_int, 200)
        self.assertEqual(rsp.body, 'home sweet home')

        # Now the non-strict...

        req = webapp2.Request.blank('/strict-foo/')
        rsp = req.get_response(app)
        self.assertEqual(rsp.status_int, 301)
        self.assertEqual(rsp.body, '')
        self.assertEqual(rsp.headers['Location'], 'http://localhost/strict-foo')

        req = webapp2.Request.blank('/strict-bar')
        rsp = req.get_response(app)
        self.assertEqual(rsp.status_int, 301)
        self.assertEqual(rsp.body, '')
        self.assertEqual(rsp.headers['Location'], 'http://localhost/strict-bar/')

        # Strict slash routes must have a name.

        self.assertRaises(ValueError, RedirectRoute, '/strict-bar/', handler=HomeHandler, strict_slash=True)

    def test_build_only(self):
        self.assertRaises(ValueError, RedirectRoute, '/', handler=HomeHandler, build_only=True)


class TestPrefixRoutes(test_base.BaseTestCase):
    def test_simple(self):
        router = webapp2.Router([
            PathPrefixRoute('/a', [
                webapp2.Route('/', 'a', 'name-a'),
                webapp2.Route('/b', 'a/b', 'name-a/b'),
                webapp2.Route('/c', 'a/c', 'name-a/c'),
                PathPrefixRoute('/d', [
                    webapp2.Route('/', 'a/d', 'name-a/d'),
                    webapp2.Route('/b', 'a/d/b', 'name-a/d/b'),
                    webapp2.Route('/c', 'a/d/c', 'name-a/d/c'),
                ]),
            ])
        ])

        path = '/a/'
        match = ((), {})
        self.assertEqual(router.match(webapp2.Request.blank(path))[1:], match)
        self.assertEqual(router.build(webapp2.Request.blank('/'), 'name-a', match[0], match[1]), path)

        path = '/a/b'
        match = ((), {})
        self.assertEqual(router.match(webapp2.Request.blank(path))[1:], match)
        self.assertEqual(router.build(webapp2.Request.blank('/'), 'name-a/b', match[0], match[1]), path)

        path = '/a/c'
        match = ((), {})
        self.assertEqual(router.match(webapp2.Request.blank(path))[1:], match)
        self.assertEqual(router.build(webapp2.Request.blank('/'), 'name-a/c', match[0], match[1]), path)

        path = '/a/d/'
        match = ((), {})
        self.assertEqual(router.match(webapp2.Request.blank(path))[1:], match)
        self.assertEqual(router.build(webapp2.Request.blank('/'), 'name-a/d', match[0], match[1]), path)

        path = '/a/d/b'
        match = ((), {})
        self.assertEqual(router.match(webapp2.Request.blank(path))[1:], match)
        self.assertEqual(router.build(webapp2.Request.blank('/'), 'name-a/d/b', match[0], match[1]), path)

        path = '/a/d/c'
        match = ((), {})
        self.assertEqual(router.match(webapp2.Request.blank(path))[1:], match)
        self.assertEqual(router.build(webapp2.Request.blank('/'), 'name-a/d/c', match[0], match[1]), path)

    def test_with_variables_name_and_handler(self):
        router = webapp2.Router([
            PathPrefixRoute('/user/<username:\w+>', [
                HandlerPrefixRoute('apps.users.', [
                    NamePrefixRoute('user-', [
                        webapp2.Route('/', 'UserOverviewHandler', 'overview'),
                        webapp2.Route('/profile', 'UserProfileHandler', 'profile'),
                        webapp2.Route('/projects', 'UserProjectsHandler', 'projects'),
                    ]),
                ]),
            ])
        ])

        path = '/user/calvin/'
        match = ((), {'username': 'calvin'})
        self.assertEqual(router.match(webapp2.Request.blank(path))[1:], match)
        self.assertEqual(router.build(webapp2.Request.blank('/'), 'user-overview', match[0], match[1]), path)

        path = '/user/calvin/profile'
        match = ((), {'username': 'calvin'})
        self.assertEqual(router.match(webapp2.Request.blank(path))[1:], match)
        self.assertEqual(router.build(webapp2.Request.blank('/'), 'user-profile', match[0], match[1]), path)

        path = '/user/calvin/projects'
        match = ((), {'username': 'calvin'})
        self.assertEqual(router.match(webapp2.Request.blank(path))[1:], match)
        self.assertEqual(router.build(webapp2.Request.blank('/'), 'user-projects', match[0], match[1]), path)


class TestDomainRoute(test_base.BaseTestCase):
    def test_simple(self):
        router = webapp2.Router([
            DomainRoute('<subdomain>.<:.*>', [
                webapp2.Route('/foo', 'FooHandler', 'subdomain-thingie'),
            ])
        ])

        self.assertRaises(webapp2.exc.HTTPNotFound, router.match, webapp2.Request.blank('/foo'))

        match = router.match(webapp2.Request.blank('http://my-subdomain.app-id.appspot.com/foo'))
        self.assertEqual(match[1:], ((), {'subdomain': 'my-subdomain'}))

        match = router.match(webapp2.Request.blank('http://another-subdomain.app-id.appspot.com/foo'))
        self.assertEqual(match[1:], ((), {'subdomain': 'another-subdomain'}))

        url = router.build(webapp2.Request.blank('/'), 'subdomain-thingie', (), {'_netloc': 'another-subdomain.app-id.appspot.com'})
        self.assertEqual(url, 'http://another-subdomain.app-id.appspot.com/foo')

    def test_with_variables_name_and_handler(self):
        router = webapp2.Router([
            DomainRoute('<subdomain>.<:.*>', [
                PathPrefixRoute('/user/<username:\w+>', [
                    HandlerPrefixRoute('apps.users.', [
                        NamePrefixRoute('user-', [
                            webapp2.Route('/', 'UserOverviewHandler', 'overview'),
                            webapp2.Route('/profile', 'UserProfileHandler', 'profile'),
                            webapp2.Route('/projects', 'UserProjectsHandler', 'projects'),
                        ]),
                    ]),
                ])
            ]),
        ])

        path = 'http://my-subdomain.app-id.appspot.com/user/calvin/'
        match = ((), {'username': 'calvin', 'subdomain': 'my-subdomain'})
        self.assertEqual(router.match(webapp2.Request.blank(path))[1:], match)
        match[1].pop('subdomain')
        match[1]['_netloc'] = 'my-subdomain.app-id.appspot.com'
        self.assertEqual(router.build(webapp2.Request.blank('/'), 'user-overview', match[0], match[1]), path)

        path = 'http://my-subdomain.app-id.appspot.com/user/calvin/profile'
        match = ((), {'username': 'calvin', 'subdomain': 'my-subdomain'})
        self.assertEqual(router.match(webapp2.Request.blank(path))[1:], match)
        match[1].pop('subdomain')
        match[1]['_netloc'] = 'my-subdomain.app-id.appspot.com'
        self.assertEqual(router.build(webapp2.Request.blank('/'), 'user-profile', match[0], match[1]), path)

        path = 'http://my-subdomain.app-id.appspot.com/user/calvin/projects'
        match = ((), {'username': 'calvin', 'subdomain': 'my-subdomain'})
        self.assertEqual(router.match(webapp2.Request.blank(path))[1:], match)
        match[1].pop('subdomain')
        match[1]['_netloc'] = 'my-subdomain.app-id.appspot.com'
        self.assertEqual(router.build(webapp2.Request.blank('/'), 'user-projects', match[0], match[1]), path)

    def test_guide_examples(self):
        router = webapp2.Router([
            DomainRoute(r'www.mydomain.com', [
                webapp2.Route('/path1', 'Path1', 'path1'),
            ]),
            DomainRoute(r'<subdomain:(?!www\.)[^.]+>.mydomain.com', [
                webapp2.Route('/path2', 'Path2', 'path2'),
            ]),
            DomainRoute(r'<:(app-id\.appspot\.com|www\.mydomain\.com)>', [
                webapp2.Route('/path3', 'Path3', 'path3'),
            ]),
            DomainRoute(r'<subdomain:(?!www)[^.]+>.<:(app-id\.appspot\.com|mydomain\.com)>', [
                webapp2.Route('/path4', 'Path4', 'path4'),
            ]),
        ])

        uri1a = 'http://www.mydomain.com/path1'
        uri1b = 'http://sub.mydomain.com/path1'
        uri1c = 'http://www.mydomain.com/invalid-path'

        uri2a = 'http://sub.mydomain.com/path2'
        uri2b = 'http://www.mydomain.com/path2'
        uri2c = 'http://sub.mydomain.com/invalid-path'
        uri2d = 'http://www.mydomain.com/invalid-path'

        uri3a = 'http://app-id.appspot.com/path3'
        uri3b = 'http://www.mydomain.com/path3'
        uri3c = 'http://sub.app-id.appspot.com/path3'
        uri3d = 'http://sub.mydomain.com/path3'
        uri3e = 'http://app-id.appspot.com/invalid-path'
        uri3f = 'http://www.mydomain.com/invalid-path'

        uri4a = 'http://sub.app-id.appspot.com/path4'
        uri4b = 'http://sub.mydomain.com/path4'
        uri4c = 'http://app-id.appspot.com/path4'
        uri4d = 'http://www.app-id.appspot.com/path4'
        uri4e = 'http://www.mydomain.com/path4'
        uri4f = 'http://sub.app-id.appspot.com/invalid-path'
        uri4g = 'http://sub.mydomain.com/invalid-path'

        self.assertEqual(router.match(webapp2.Request.blank(uri1a))[1:], ((), {}))
        self.assertRaises(webapp2.exc.HTTPNotFound, router.match, webapp2.Request.blank(uri1b))
        self.assertRaises(webapp2.exc.HTTPNotFound, router.match, webapp2.Request.blank(uri1c))

        self.assertEqual(router.match(webapp2.Request.blank(uri2a))[1:], ((), {'subdomain': 'sub'}))
        self.assertRaises(webapp2.exc.HTTPNotFound, router.match, webapp2.Request.blank(uri2b))
        self.assertRaises(webapp2.exc.HTTPNotFound, router.match, webapp2.Request.blank(uri2c))
        self.assertRaises(webapp2.exc.HTTPNotFound, router.match, webapp2.Request.blank(uri2d))

        self.assertEqual(router.match(webapp2.Request.blank(uri3a))[1:], ((), {}))
        self.assertEqual(router.match(webapp2.Request.blank(uri3b))[1:], ((), {}))
        self.assertRaises(webapp2.exc.HTTPNotFound, router.match, webapp2.Request.blank(uri3c))
        self.assertRaises(webapp2.exc.HTTPNotFound, router.match, webapp2.Request.blank(uri3d))
        self.assertRaises(webapp2.exc.HTTPNotFound, router.match, webapp2.Request.blank(uri3e))
        self.assertRaises(webapp2.exc.HTTPNotFound, router.match, webapp2.Request.blank(uri3f))

        self.assertEqual(router.match(webapp2.Request.blank(uri4a))[1:], ((), {'subdomain': 'sub'}))
        self.assertEqual(router.match(webapp2.Request.blank(uri4b))[1:], ((), {'subdomain': 'sub'}))
        self.assertRaises(webapp2.exc.HTTPNotFound, router.match, webapp2.Request.blank(uri4c))
        self.assertRaises(webapp2.exc.HTTPNotFound, router.match, webapp2.Request.blank(uri4d))
        self.assertRaises(webapp2.exc.HTTPNotFound, router.match, webapp2.Request.blank(uri4e))
        self.assertRaises(webapp2.exc.HTTPNotFound, router.match, webapp2.Request.blank(uri4f))
        self.assertRaises(webapp2.exc.HTTPNotFound, router.match, webapp2.Request.blank(uri4g))

if __name__ == '__main__':
    test_base.main()