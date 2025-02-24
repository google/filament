from webapp2_extras import auth
from webapp2_extras.appengine.auth import models

from google.appengine.ext.ndb import model

import test_base


class UniqueConstraintViolation(Exception):
    pass


class User(model.Model):
    username = model.StringProperty(required=True)
    auth_id = model.StringProperty()
    email = model.StringProperty()


class TestAuthModels(test_base.BaseTestCase):

    def setUp(self):
        super(TestAuthModels, self).setUp()
        self.register_model('User', models.User)
        self.register_model('UserToken', models.UserToken)
        self.register_model('Unique', models.Unique)

    def test_get(self):
        m = models.User
        success, user = m.create_user(auth_id='auth_id_1', password_raw='foo')
        self.assertEqual(success, True)
        self.assertTrue(user is not None)
        self.assertTrue(user.password is not None)

        # user.key.id() is required to retrieve the auth token
        user_id = user.key.id()

        token = m.create_auth_token(user_id)

        self.assertEqual(m.get_by_auth_id('auth_id_1'), user)
        self.assertEqual(m.get_by_auth_id('auth_id_2'), None)

        u, ts = m.get_by_auth_token(user_id, token)
        self.assertEqual(u, user)
        u, ts = m.get_by_auth_token('fake_user_id', token)
        self.assertEqual(u, None)

        u = m.get_by_auth_password('auth_id_1', 'foo')
        self.assertEqual(u, user)
        self.assertRaises(auth.InvalidPasswordError,
                          m.get_by_auth_password, 'auth_id_1', 'bar')
        self.assertRaises(auth.InvalidAuthIdError,
                          m.get_by_auth_password, 'auth_id_2', 'foo')

    def test_create_user(self):
        m = models.User
        success, info = m.create_user(auth_id='auth_id_1', password_raw='foo')
        self.assertEqual(success, True)
        self.assertTrue(info is not None)
        self.assertTrue(info.password is not None)

        success, info = m.create_user(auth_id='auth_id_1')
        self.assertEqual(success, False)
        self.assertEqual(info, ['auth_id'])

        # 3 extras and unique properties; plus 1 extra and not unique.
        extras = ['foo', 'bar', 'baz']
        values = dict((v, v + '_value') for v in extras)
        values['ding'] = 'ding_value'
        success, info = m.create_user(auth_id='auth_id_2',
                                      unique_properties=extras, **values)
        self.assertEqual(success, True)
        self.assertTrue(info is not None)
        for prop in extras:
            self.assertEqual(getattr(info, prop), prop + '_value')
        self.assertEqual(info.ding, 'ding_value')

        # Let's do it again.
        success, info = m.create_user(auth_id='auth_id_3',
                                      unique_properties=extras, **values)
        self.assertEqual(success, False)
        self.assertEqual(info, extras)

    def test_add_auth_ids(self):
        m = models.User
        success, new_user = m.create_user(auth_id='auth_id_1', password_raw='foo')
        self.assertEqual(success, True)
        self.assertTrue(new_user is not None)
        self.assertTrue(new_user.password is not None)

        success, new_user_2 = m.create_user(auth_id='auth_id_2', password_raw='foo')
        self.assertEqual(success, True)
        self.assertTrue(new_user is not None)
        self.assertTrue(new_user.password is not None)

        success, info = new_user.add_auth_id('auth_id_3')
        self.assertEqual(success, True)
        self.assertEqual(info.auth_ids, ['auth_id_1', 'auth_id_3'])

        success, info = new_user.add_auth_id('auth_id_2')
        self.assertEqual(success, False)
        self.assertEqual(info, ['auth_id'])

    def test_token(self):
        m = models.UserToken

        auth_id = 'foo'
        subject = 'bar'
        token_1 = m.create(auth_id, subject, token=None)
        token = token_1.token

        token_2 = m.get(user=auth_id, subject=subject, token=token)
        self.assertEqual(token_2, token_1)

        token_3 = m.get(subject=subject, token=token)
        self.assertEqual(token_3, token_1)

        m.get_key(auth_id, subject, token).delete()

        token_2 = m.get(user=auth_id, subject=subject, token=token)
        self.assertEqual(token_2, None)

        token_3 = m.get(subject=subject, token=token)
        self.assertEqual(token_3, None)

    def test_user_token(self):
        m = models.User
        auth_id = 'foo'

        token = m.create_auth_token(auth_id)
        self.assertTrue(m.validate_auth_token(auth_id, token))
        m.delete_auth_token(auth_id, token)
        self.assertFalse(m.validate_auth_token(auth_id, token))

        token = m.create_signup_token(auth_id)
        self.assertTrue(m.validate_signup_token(auth_id, token))
        m.delete_signup_token(auth_id, token)
        self.assertFalse(m.validate_signup_token(auth_id, token))


class TestUniqueModel(test_base.BaseTestCase):
    def setUp(self):
        super(TestUniqueModel, self).setUp()
        self.register_model('Unique', models.Unique)

    def test_single(self):
        def create_user(username):
            # Assemble the unique scope/value combinations.
            unique_username = 'User.username:%s' % username

            # Create the unique username, auth_id and email.
            success = models.Unique.create(unique_username)

            if success:
                user = User(username=username)
                user.put()
                return user
            else:
                raise UniqueConstraintViolation('Username %s already '
                    'exists' % username)

        user = create_user('username_1')
        self.assertRaises(UniqueConstraintViolation, create_user, 'username_1')

        user = create_user('username_2')
        self.assertRaises(UniqueConstraintViolation, create_user, 'username_2')

    def test_multi(self):
        def create_user(username, auth_id, email):
            # Assemble the unique scope/value combinations.
            unique_username = 'User.username:%s' % username
            unique_auth_id = 'User.auth_id:%s' % auth_id
            unique_email = 'User.email:%s' % email

            # Create the unique username, auth_id and email.
            uniques = [unique_username, unique_auth_id, unique_email]
            success, existing = models.Unique.create_multi(uniques)

            if success:
                user = User(username=username, auth_id=auth_id, email=email)
                user.put()
                return user
            else:
                if unique_username in existing:
                    raise UniqueConstraintViolation('Username %s already '
                        'exists' % username)
                if unique_auth_id in existing:
                    raise UniqueConstraintViolation('Auth id %s already '
                        'exists' % auth_id)
                if unique_email in existing:
                    raise UniqueConstraintViolation('Email %s already '
                        'exists' % email)

        user = create_user('username_1', 'auth_id_1', 'email_1')
        self.assertRaises(UniqueConstraintViolation, create_user, 'username_1', 'auth_id_2', 'email_2')
        self.assertRaises(UniqueConstraintViolation, create_user, 'username_2', 'auth_id_1', 'email_2')
        self.assertRaises(UniqueConstraintViolation, create_user, 'username_2', 'auth_id_2', 'email_1')

        user = create_user('username_2', 'auth_id_2', 'email_2')
        self.assertRaises(UniqueConstraintViolation, create_user, 'username_2', 'auth_id_1', 'email_1')
        self.assertRaises(UniqueConstraintViolation, create_user, 'username_1', 'auth_id_2', 'email_1')
        self.assertRaises(UniqueConstraintViolation, create_user, 'username_1', 'auth_id_1', 'email_2')

    def test_delete_multi(self):
        rv = models.Unique.create_multi(('foo', 'bar', 'baz'))
        self.assertEqual(rv, (True, []))
        rv = models.Unique.create_multi(('foo', 'bar', 'baz'))
        self.assertEqual(rv, (False, ['foo', 'bar', 'baz']))

        models.Unique.delete_multi(('foo', 'bar', 'baz'))

        rv = models.Unique.create_multi(('foo', 'bar', 'baz'))
        self.assertEqual(rv, (True, []))


if __name__ == '__main__':
    test_base.main()
