# -*- coding: utf-8 -*-
import webapp2
from webapp2_extras import sessions

import test_base

app = webapp2.WSGIApplication(config={
    'webapp2_extras.sessions': {
        'secret_key': 'my-super-secret',
    },
})


class TestSecureCookieSession(test_base.BaseTestCase):
    factory = sessions.SecureCookieSessionFactory

    def test_config(self):
        app = webapp2.WSGIApplication()
        req = webapp2.Request.blank('/')
        req.app = app
        self.assertRaises(Exception, sessions.SessionStore, req)

        # Just to set a special config.
        app = webapp2.WSGIApplication()
        req = webapp2.Request.blank('/')
        req.app = app
        store = sessions.SessionStore(req, config={
            'secret_key': 'my-super-secret',
            'cookie_name': 'foo'
        })
        session = store.get_session(factory=self.factory)
        session['bar'] = 'bar'
        rsp = webapp2.Response()
        store.save_sessions(rsp)
        self.assertTrue(rsp.headers['Set-Cookie'].startswith('foo='))

    def test_get_save_session(self):
        # Round 1 -------------------------------------------------------------

        req = webapp2.Request.blank('/')
        req.app = app
        store = sessions.SessionStore(req)

        session = store.get_session(factory=self.factory)

        rsp = webapp2.Response()
        # Nothing changed, we want to test anyway.
        store.save_sessions(rsp)

        session['a'] = 'b'
        session['c'] = 'd'
        session['e'] = 'f'

        store.save_sessions(rsp)

        # Round 2 -------------------------------------------------------------

        cookies = rsp.headers.get('Set-Cookie')
        req = webapp2.Request.blank('/', headers=[('Cookie', cookies)])
        req.app = app
        store = sessions.SessionStore(req)

        session = store.get_session(factory=self.factory)
        self.assertEqual(session['a'], 'b')
        self.assertEqual(session['c'], 'd')
        self.assertEqual(session['e'], 'f')

        session['g'] = 'h'

        rsp = webapp2.Response()
        store.save_sessions(rsp)

        # Round 3 -------------------------------------------------------------

        cookies = rsp.headers.get('Set-Cookie')
        req = webapp2.Request.blank('/', headers=[('Cookie', cookies)])
        req.app = app
        store = sessions.SessionStore(req)

        session = store.get_session(factory=self.factory)
        self.assertEqual(session['a'], 'b')
        self.assertEqual(session['c'], 'd')
        self.assertEqual(session['e'], 'f')
        self.assertEqual(session['g'], 'h')

        self.assertRaises(KeyError, session.pop, 'foo')

    def test_flashes(self):

        # Round 1 -------------------------------------------------------------

        req = webapp2.Request.blank('/')
        req.app = app
        store = sessions.SessionStore(req)

        session = store.get_session(factory=self.factory)
        flashes = session.get_flashes()
        self.assertEqual(flashes, [])
        session.add_flash('foo')

        rsp = webapp2.Response()
        store.save_sessions(rsp)

        # Round 2 -------------------------------------------------------------

        cookies = rsp.headers.get('Set-Cookie')
        req = webapp2.Request.blank('/', headers=[('Cookie', cookies)])
        req.app = app
        store = sessions.SessionStore(req)

        session = store.get_session(factory=self.factory)

        flashes = session.get_flashes()
        self.assertEqual(flashes, [[u'foo', None]])

        flashes = session.get_flashes()
        self.assertEqual(flashes, [])

        session.add_flash('bar')
        session.add_flash('baz', 'important')

        rsp = webapp2.Response()
        store.save_sessions(rsp)

        # Round 3 -------------------------------------------------------------

        cookies = rsp.headers.get('Set-Cookie')
        req = webapp2.Request.blank('/', headers=[('Cookie', cookies)])
        req.app = app
        store = sessions.SessionStore(req)

        session = store.get_session(factory=self.factory)

        flashes = session.get_flashes()
        self.assertEqual(flashes, [[u'bar', None], [u'baz', 'important']])

        flashes = session.get_flashes()
        self.assertEqual(flashes, [])

        rsp = webapp2.Response()
        store.save_sessions(rsp)

        # Round 4 -------------------------------------------------------------

        cookies = rsp.headers.get('Set-Cookie')
        req = webapp2.Request.blank('/', headers=[('Cookie', cookies)])
        req.app = app
        store = sessions.SessionStore(req)

        session = store.get_session(factory=self.factory)
        flashes = session.get_flashes()
        self.assertEqual(flashes, [])

    def test_set_secure_cookie(self):

        rsp = webapp2.Response()

        # Round 1 -------------------------------------------------------------

        req = webapp2.Request.blank('/')
        req.app = app
        store = sessions.SessionStore(req)

        store.set_secure_cookie('foo', {'bar': 'baz'})
        store.save_sessions(rsp)

        # Round 2 -------------------------------------------------------------

        cookies = rsp.headers.get('Set-Cookie')
        req = webapp2.Request.blank('/', headers=[('Cookie', cookies)])
        req.app = app
        store = sessions.SessionStore(req)
        res = store.get_secure_cookie('foo')
        self.assertEqual(res, {'bar': 'baz'})

    def test_set_session_store(self):
        app = webapp2.WSGIApplication(config={
            'webapp2_extras.sessions': {
                'secret_key': 'my-super-secret',
            }
        })
        req = webapp2.Request.blank('/')
        req.app = app
        store = sessions.SessionStore(req)

        self.assertEqual(len(req.registry), 0)
        sessions.set_store(store, request=req)
        self.assertEqual(len(req.registry), 1)
        s = sessions.get_store(request=req)
        self.assertTrue(isinstance(s, sessions.SessionStore))

    def test_get_session_store(self):
        app = webapp2.WSGIApplication(config={
            'webapp2_extras.sessions': {
                'secret_key': 'my-super-secret',
            }
        })
        req = webapp2.Request.blank('/')
        req.app = app
        self.assertEqual(len(req.registry), 0)
        s = sessions.get_store(request=req)
        self.assertEqual(len(req.registry), 1)
        self.assertTrue(isinstance(s, sessions.SessionStore))

    def test_not_implemented(self):
        req = webapp2.Request.blank('/')
        req.app = app
        store = sessions.SessionStore(req)

        f = sessions.BaseSessionFactory('foo', store)
        self.assertRaises(NotImplementedError, f.get_session)
        self.assertRaises(NotImplementedError, f.save_session, None)

        f = sessions.CustomBackendSessionFactory('foo', store)
        self.assertRaises(NotImplementedError, f._get_by_sid, None)


if __name__ == '__main__':
    test_base.main()
