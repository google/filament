import webapp2
from webapp2_extras import sessions

from webapp2_extras import auth
from webapp2_extras.appengine.auth import models

from google.appengine.ext.ndb import model

import test_base


class TestAuth(test_base.BaseTestCase):

    def setUp(self):
        super(TestAuth, self).setUp()
        self.register_model('User', models.User)
        self.register_model('UserToken', models.UserToken)
        self.register_model('Unique', models.Unique)

    def _check_token(self, user_id, token, subject='auth'):
        rv = models.UserToken.get(user=user_id, subject=subject, token=token)
        return rv is not None

    def test_get_user_by_session(self):
        app = webapp2.WSGIApplication(config={
            'webapp2_extras.sessions': {
                'secret_key': 'foo',
            }
        })
        req = webapp2.Request.blank('/')
        rsp = webapp2.Response()
        req.app = app
        s = auth.get_store(app=app)
        a = auth.Auth(request=req)
        session_store = sessions.get_store(request=req)

        # This won't work.
        a.set_session_data({})
        self.assertEqual(a.session.get('_user'), None)

        # This won't work.
        a.session['_user'] = {}
        self.assertEqual(a.get_session_data(), None)
        self.assertEqual(a.session.get('_user'), None)

        # Create a user.
        m = models.User
        success, user = m.create_user(auth_id='auth_id',
                                      password_raw='password')

        user_id = user.key.id()

        # Get user with session. An anonymous_user is returned.
        rv = a.get_user_by_session()
        self.assertTrue(rv is None)

        # Login with password. User dict is returned.
        rv = a.get_user_by_password('auth_id', 'password')
        self.assertEqual(rv['user_id'], user_id)

        # Save sessions.
        session_store.save_sessions(rsp)

        # Get user with session. Voila!
        cookies = rsp.headers.get('Set-Cookie')
        req = webapp2.Request.blank('/', headers=[('Cookie', cookies)])
        rsp = webapp2.Response()
        req.app = app
        a = auth.Auth(request=req)

        # only auth_id is returned when there're no
        # custom user attributes defined.
        rv = a.get_user_by_session()
        self.assertEqual(rv['user_id'], user_id)

        # If we call get_user_by_token() now, the same user is returned.
        rv2 = a.get_user_by_token(rv['user_id'], rv['token'])
        self.assertTrue(rv is rv2)

        # Let's get it again and check that token is the same.
        token = rv['token']
        a._user = None
        rv = a.get_user_by_session()
        self.assertEqual(rv['user_id'], user_id)
        self.assertEqual(rv['token'], token)

        # Now let's force token to be renewed and check that we have a new one.
        s.config['token_new_age'] = -300
        a._user = None
        rv = a.get_user_by_session()
        self.assertEqual(rv['user_id'], user_id)
        self.assertNotEqual(rv['token'], token)

        # Now let's force token to be invalid.
        s.config['token_max_age'] = -300
        a._user = None
        rv = a.get_user_by_session()
        self.assertEqual(rv, None)

    def test_get_user_by_password(self):
        app = webapp2.WSGIApplication(config={
            'webapp2_extras.sessions': {
                'secret_key': 'foo',
            }
        })
        req = webapp2.Request.blank('/')
        req.app = app
        s = auth.get_store(app=app)
        a = auth.get_auth(request=req)
        session_store = sessions.get_store(request=req)

        m = models.User
        success, user = m.create_user(auth_id='auth_id',
                                      password_raw='password')

        user_id = user.key.id()
        # Lets test the cookie max_age when we use remember=True or False.
        rv = a.get_user_by_password('auth_id', 'password', remember=True)
        self.assertEqual(rv['user_id'], user_id)
        self.assertEqual(session_store.sessions['auth'].session_args['max_age'],
                         86400 * 7 * 3)

        # Now remember=False.
        rv = a.get_user_by_password('auth_id', 'password')
        self.assertEqual(rv['user_id'], user_id)
        self.assertEqual(session_store.sessions['auth'].session_args['max_age'],
                         None)

        # User was set so getting it from session will return the same one.
        rv = a.get_user_by_session()
        self.assertEqual(rv['user_id'], user_id)

        # Now try a failed password submission: user will be unset.
        rv = a.get_user_by_password('auth_id', 'password_2', silent=True)
        self.assertTrue(rv is None)

        # And getting by session will no longer work.
        rv = a.get_user_by_session()
        self.assertTrue(rv is None)

    def test_validate_password(self):
        app = webapp2.WSGIApplication()
        req = webapp2.Request.blank('/')
        req.app = app
        s = auth.get_store(app=app)

        m = models.User
        success, user = m.create_user(auth_id='auth_id',
                                      password_raw='foo')

        u = s.validate_password('auth_id', 'foo')
        self.assertEqual(u, s.user_to_dict(user))
        self.assertRaises(auth.InvalidPasswordError,
                          s.validate_password, 'auth_id', 'bar')
        self.assertRaises(auth.InvalidAuthIdError,
                          s.validate_password, 'auth_id_2', 'foo')

    def test_validate_token(self):
        app = webapp2.WSGIApplication()
        req = webapp2.Request.blank('/')
        req.app = app
        s = auth.get_store(app=app)

        rv = s.validate_token('auth_id', 'token')
        self.assertEqual(rv, (None, None))

        # Expired timestamp.
        rv = s.validate_token('auth_id', 'token', -300)
        self.assertEqual(rv, (None, None))

        m = models.User
        success, user = m.create_user(auth_id='auth_id',
                                      password_raw='foo')

        user_id = user.key.id()
        token = m.create_auth_token(user_id)
        rv = s.validate_token(user_id, token)
        self.assertEqual(rv, (s.user_to_dict(user), token))
        # Token must still be there.
        self.assertTrue(self._check_token(user_id, token))

        # Expired timestamp.
        token = m.create_auth_token(user_id)
        rv = s.validate_token(user_id, token, -300)
        self.assertEqual(rv, (None, None))
        # Token must have been deleted.
        self.assertFalse(self._check_token(user_id, token))

        # Force expiration.
        token = m.create_auth_token(user_id)
        s.config['token_max_age'] = -300
        rv = s.validate_token(user_id, token)
        self.assertEqual(rv, (None, None))
        # Token must have been deleted.
        self.assertFalse(self._check_token(user_id, token))

        # Revert expiration, force renewal.
        token = m.create_auth_token(user_id)
        s.config['token_max_age'] = 86400 * 7 * 3
        s.config['token_new_age'] = -300
        rv = s.validate_token(user_id, token)
        self.assertEqual(rv, (s.user_to_dict(user), None))
        # Token must have been deleted.
        self.assertFalse(self._check_token(user_id, token))

    def test_set_auth_store(self):
        app = webapp2.WSGIApplication()
        req = webapp2.Request.blank('/')
        req.app = app
        store = auth.AuthStore(app)

        self.assertEqual(len(app.registry), 0)
        auth.set_store(store, app=app)
        self.assertEqual(len(app.registry), 1)
        s = auth.get_store(app=app)
        self.assertTrue(isinstance(s, auth.AuthStore))

    def test_get_auth_store(self):
        app = webapp2.WSGIApplication()
        req = webapp2.Request.blank('/')
        req.app = app
        self.assertEqual(len(app.registry), 0)
        s = auth.get_store(app=app)
        self.assertEqual(len(app.registry), 1)
        self.assertTrue(isinstance(s, auth.AuthStore))

    def test_set_auth(self):
        app = webapp2.WSGIApplication()
        req = webapp2.Request.blank('/')
        req.app = app
        a = auth.Auth(req)

        self.assertEqual(len(req.registry), 0)
        auth.set_auth(a, request=req)
        self.assertEqual(len(req.registry), 1)
        a = auth.get_auth(request=req)
        self.assertTrue(isinstance(a, auth.Auth))

    def test_get_auth(self):
        app = webapp2.WSGIApplication()
        req = webapp2.Request.blank('/')
        req.app = app
        self.assertEqual(len(req.registry), 0)
        a = auth.get_auth(request=req)
        self.assertEqual(len(req.registry), 1)
        self.assertTrue(isinstance(a, auth.Auth))

    '''
    def test_set_callables(self):
        app = webapp2.WSGIApplication()
        req = webapp2.Request.blank('/')
        req.app = app
        s = auth.get_store(app=app)

        def validate_password(store, auth_id, password):
            self.assertTrue(store is s)
            self.assertEqual(auth_id, 'auth_id')
            self.assertEqual(password, 'password')
            return 'validate_password'

        def validate_token(store, auth_id, token, token_ts=None):
            self.assertTrue(store is s)
            self.assertEqual(auth_id, 'auth_id')
            self.assertEqual(token, 'token')
            self.assertEqual(token_ts, 'token_ts')
            return 'validate_token'

        s.set_password_validator(validate_password)
        rv = s.validate_password('auth_id', 'password')
        self.assertEqual(rv, 'validate_password')

        s.set_token_validator(validate_token)
        rv = s.validate_token('auth_id', 'token', 'token_ts')
        self.assertEqual(rv, 'validate_token')
    '''

    def test_extended_user(self):
        class MyUser(models.User):
            newsletter = model.BooleanProperty()
            age = model.IntegerProperty()

        auth_id = 'own:username'
        success, info = MyUser.create_user(auth_id, newsletter=True, age=22)
        self.assertTrue(success)

        app = webapp2.WSGIApplication(config={
            'webapp2_extras.auth': {
                'user_model': MyUser,
            }
        })
        s = auth.get_store(app=app)
        user = s.user_model.get_by_auth_id(auth_id)
        self.assertEqual(info, user)
        self.assertEqual(user.age, 22)
        self.assertTrue(user.newsletter is True)


if __name__ == '__main__':
    test_base.main()
