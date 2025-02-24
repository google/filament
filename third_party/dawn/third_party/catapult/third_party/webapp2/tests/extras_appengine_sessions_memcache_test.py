# -*- coding: utf-8 -*-
import webapp2
from webapp2_extras import sessions
from webapp2_extras import sessions_memcache

import test_base


app = webapp2.WSGIApplication(config={
    'webapp2_extras.sessions': {
        'secret_key': 'my-super-secret',
    },
})


class TestMemcacheSession(test_base.BaseTestCase):
    #factory = sessions_memcache.MemcacheSessionFactory

    def test_get_save_session(self):

        # Round 1 -------------------------------------------------------------

        req = webapp2.Request.blank('/')
        req.app = app
        store = sessions.SessionStore(req)

        session = store.get_session(backend='memcache')

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

        session = store.get_session(backend='memcache')
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

        session = store.get_session(backend='memcache')
        self.assertEqual(session['a'], 'b')
        self.assertEqual(session['c'], 'd')
        self.assertEqual(session['e'], 'f')
        self.assertEqual(session['g'], 'h')

    def test_flashes(self):

        # Round 1 -------------------------------------------------------------

        req = webapp2.Request.blank('/')
        req.app = app
        store = sessions.SessionStore(req)

        session = store.get_session(backend='memcache')
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

        session = store.get_session(backend='memcache')

        flashes = session.get_flashes()
        self.assertEqual(flashes, [(u'foo', None)])

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

        session = store.get_session(backend='memcache')

        flashes = session.get_flashes()
        self.assertEqual(flashes, [(u'bar', None), (u'baz', 'important')])

        flashes = session.get_flashes()
        self.assertEqual(flashes, [])

        rsp = webapp2.Response()
        store.save_sessions(rsp)

        # Round 4 -------------------------------------------------------------

        cookies = rsp.headers.get('Set-Cookie')
        req = webapp2.Request.blank('/', headers=[('Cookie', cookies)])
        req.app = app
        store = sessions.SessionStore(req)

        session = store.get_session(backend='memcache')
        flashes = session.get_flashes()
        self.assertEqual(flashes, [])

if __name__ == '__main__':
    test_base.main()
