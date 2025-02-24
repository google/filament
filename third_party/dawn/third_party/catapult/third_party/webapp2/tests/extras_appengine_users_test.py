# -*- coding: utf-8 -*-
import os

import webapp2
from webapp2_extras import users

import test_base


def set_current_user(email, user_id, is_admin=False):
    os.environ['USER_EMAIL'] = email or ''
    os.environ['USER_ID'] = user_id or ''
    os.environ['USER_IS_ADMIN'] = '1' if is_admin else '0'


class LoginRequiredHandler(webapp2.RequestHandler):
    @users.login_required
    def get(self):
        self.response.write('You are logged in.')

    @users.login_required
    def post(self):
        self.response.write('You are logged in.')


class AdminRequiredHandler(webapp2.RequestHandler):
    @users.admin_required
    def get(self):
        self.response.write('You are admin.')

    @users.admin_required
    def post(self):
        self.response.write('You are admin.')


app = webapp2.WSGIApplication([
    ('/login_required', LoginRequiredHandler),
    ('/admin_required', AdminRequiredHandler),
])


class TestUsers(test_base.BaseTestCase):
    def test_login_required_allowed(self):
        set_current_user('foo@bar.com', 'foo@bar.com')
        req = webapp2.Request.blank('/login_required')

        rsp = req.get_response(app)
        self.assertEqual(rsp.status_int, 200)
        self.assertEqual(rsp.body, 'You are logged in.')

    def test_login_required_302(self):
        req = webapp2.Request.blank('/login_required')

        rsp = req.get_response(app)
        self.assertEqual(rsp.status_int, 302)
        self.assertEqual(rsp.headers.get('Location'),
            'https://www.google.com/accounts/Login?continue=http%3A//localhost/login_required')

    def test_login_required_post(self):
        req = webapp2.Request.blank('/login_required')
        req.method = 'POST'

        rsp = req.get_response(app)
        self.assertEqual(rsp.status_int, 400)

    def test_admin_required_allowed(self):
        set_current_user('foo@bar.com', 'foo@bar.com', is_admin=True)
        req = webapp2.Request.blank('/admin_required')

        rsp = req.get_response(app)
        self.assertEqual(rsp.status_int, 200)
        self.assertEqual(rsp.body, 'You are admin.')

    def test_admin_required_not_admin(self):
        set_current_user('foo@bar.com', 'foo@bar.com')
        req = webapp2.Request.blank('/admin_required')

        rsp = req.get_response(app)
        self.assertEqual(rsp.status_int, 403)

    def test_admin_required_302(self):
        req = webapp2.Request.blank('/admin_required')

        rsp = req.get_response(app)
        self.assertEqual(rsp.status_int, 302)
        self.assertEqual(rsp.headers.get('Location'),
            'https://www.google.com/accounts/Login?continue=http%3A//localhost/admin_required')

    def test_admin_required_post(self):
        req = webapp2.Request.blank('/admin_required')
        req.method = 'POST'

        rsp = req.get_response(app)
        self.assertEqual(rsp.status_int, 400)


if __name__ == '__main__':
    test_base.main()
