# -*- coding: utf-8 -*-
"""
    webapp2_extras.appengine.users
    ==============================

    Helpers for google.appengine.api.users.

    :copyright: 2011 tipfy.org.
    :license: Apache Sotware License, see LICENSE for details.
"""
from google.appengine.api import users


def login_required(handler_method):
    """A decorator to require that a user be logged in to access a handler.

    To use it, decorate your get() method like this::

        @login_required
        def get(self):
            user = users.get_current_user(self)
            self.response.out.write('Hello, ' + user.nickname())

    We will redirect to a login page if the user is not logged in. We always
    redirect to the request URI, and Google Accounts only redirects back as
    a GET request, so this should not be used for POSTs.
    """
    def check_login(self, *args, **kwargs):
        if self.request.method != 'GET':
            self.abort(400, detail='The login_required decorator '
                'can only be used for GET requests.')

        user = users.get_current_user()
        if not user:
            return self.redirect(users.create_login_url(self.request.url))
        else:
            handler_method(self, *args, **kwargs)

    return check_login


def admin_required(handler_method):
    """A decorator to require that a user be an admin for this application
    to access a handler.

    To use it, decorate your get() method like this::

        @admin_required
        def get(self):
            user = users.get_current_user(self)
            self.response.out.write('Hello, ' + user.nickname())

    We will redirect to a login page if the user is not logged in. We always
    redirect to the request URI, and Google Accounts only redirects back as
    a GET request, so this should not be used for POSTs.
    """
    def check_admin(self, *args, **kwargs):
        if self.request.method != 'GET':
            self.abort(400, detail='The admin_required decorator '
                'can only be used for GET requests.')

        user = users.get_current_user()
        if not user:
            return self.redirect(users.create_login_url(self.request.url))
        elif not users.is_current_user_admin():
            self.abort(403)
        else:
            handler_method(self, *args, **kwargs)

    return check_admin
