# -*- coding: utf-8 -*-
"""
    webapp2_extras.auth
    ===================

    Utilities for authentication and authorization.

    :copyright: 2011 by tipfy.org.
    :license: Apache Sotware License, see LICENSE for details.
"""
import logging
import time

import webapp2

from webapp2_extras import security
from webapp2_extras import sessions

#: Default configuration values for this module. Keys are:
#:
#: user_model
#:     User model which authenticates custom users and tokens.
#:     Can also be a string in dotted notation to be lazily imported.
#:     Default is :class:`webapp2_extras.appengine.auth.models.User`.
#:
#: session_backend
#:     Name of the session backend to be used. Default is `securecookie`.
#:
#: cookie_name
#:     Name of the cookie to save the auth session. Default is `auth`.
#:
#: token_max_age
#:     Number of seconds of inactivity after which an auth token is
#:     invalidated. The same value is used to set the ``max_age`` for
#:     persistent auth sessions. Default is 86400 * 7 * 3 (3 weeks).
#:
#: token_new_age
#:     Number of seconds after which a new token is created and written to
#:     the database, and the old one is invalidated.
#:     Use this to limit database writes; set to None to write on all requests.
#:     Default is 86400 (1 day).
#:
#: token_cache_age
#:     Number of seconds after which a token must be checked in the database.
#:     Use this to limit database reads; set to None to read on all requests.
#:     Default is 3600 (1 hour).
#:
#: user_attributes
#:     A list of extra user attributes to be stored in the session.
#      The user object must provide all of them as attributes.
#:     Default is an empty list.
default_config = {
    'user_model':      'webapp2_extras.appengine.auth.models.User',
    'session_backend': 'securecookie',
    'cookie_name':     'auth',
    'token_max_age':   86400 * 7 * 3,
    'token_new_age':   86400,
    'token_cache_age': 3600,
    'user_attributes': [],
}

#: Internal flag for anonymous users.
_anon = object()


class AuthError(Exception):
    """Base auth exception."""


class InvalidAuthIdError(AuthError):
    """Raised when a user can't be fetched given an auth_id."""


class InvalidPasswordError(AuthError):
    """Raised when a user password doesn't match."""


class AuthStore(object):
    """Provides common utilities and configuration for :class:`Auth`."""

    #: Configuration key.
    config_key = __name__

    #: Required attributes stored in a session.
    _session_attributes = ['user_id', 'remember',
                           'token', 'token_ts', 'cache_ts']

    def __init__(self, app, config=None):
        """Initializes the session store.

        :param app:
            A :class:`webapp2.WSGIApplication` instance.
        :param config:
            A dictionary of configuration values to be overridden. See
            the available keys in :data:`default_config`.
        """
        self.app = app
        # Base configuration.
        self.config = app.config.load_config(self.config_key,
            default_values=default_config, user_values=config)

    # User data we're interested in -------------------------------------------

    @webapp2.cached_property
    def session_attributes(self):
        """The list of attributes stored in a session.

        This must be an ordered list of unique elements.
        """
        seen = set()
        attrs = self._session_attributes + self.user_attributes
        return [a for a in attrs if a not in seen and not seen.add(a)]

    @webapp2.cached_property
    def user_attributes(self):
        """The list of attributes retrieved from the user model.

        This must be an ordered list of unique elements.
        """
        seen = set()
        attrs = self.config['user_attributes']
        return [a for a in attrs if a not in seen and not seen.add(a)]

    # User model related ------------------------------------------------------

    @webapp2.cached_property
    def user_model(self):
        """Configured user model."""
        cls = self.config['user_model']
        if isinstance(cls, basestring):
            cls = self.config['user_model'] = webapp2.import_string(cls)

        return cls

    def get_user_by_auth_password(self, auth_id, password, silent=False):
        """Returns a user dict based on auth_id and password.

        :param auth_id:
            Authentication id.
        :param password:
            User password.
        :param silent:
            If True, raises an exception if auth_id or password are invalid.
        :returns:
            A dictionary with user data.
        :raises:
            ``InvalidAuthIdError`` or ``InvalidPasswordError``.
        """
        try:
            user = self.user_model.get_by_auth_password(auth_id, password)
            return self.user_to_dict(user)
        except (InvalidAuthIdError, InvalidPasswordError):
            if not silent:
                raise

            return None

    def get_user_by_auth_token(self, user_id, token):
        """Returns a user dict based on user_id and auth token.

        :param user_id:
            User id.
        :param token:
            Authentication token.
        :returns:
            A tuple ``(user_dict, token_timestamp)``. Both values can be None.
            The token timestamp will be None if the user is invalid or it
            is valid but the token requires renewal.
        """
        user, ts = self.user_model.get_by_auth_token(user_id, token)
        return self.user_to_dict(user), ts

    def create_auth_token(self, user_id):
        """Creates a new authentication token.

        :param user_id:
            Authentication id.
        :returns:
            A new authentication token.
        """
        return self.user_model.create_auth_token(user_id)

    def delete_auth_token(self, user_id, token):
        """Deletes an authentication token.

        :param user_id:
            User id.
        :param token:
            Authentication token.
        """
        return self.user_model.delete_auth_token(user_id, token)

    def user_to_dict(self, user):
        """Returns a dictionary based on a user object.

        Extra attributes to be retrieved must be set in this module's
        configuration.

        :param user:
            User object: an instance the custom user model.
        :returns:
            A dictionary with user data.
        """
        if not user:
            return None

        user_dict = dict((a, getattr(user, a)) for a in self.user_attributes)
        user_dict['user_id'] = user.get_id()
        return user_dict

    # Session related ---------------------------------------------------------

    def get_session(self, request):
        """Returns an auth session.

        :param request:
            A :class:`webapp2.Request` instance.
        :returns:
            A session dict.
        """
        store = sessions.get_store(request=request)
        return store.get_session(self.config['cookie_name'],
                                 backend=self.config['session_backend'])

    def serialize_session(self, data):
        """Serializes values for a session.

        :param data:
            A dict with session data.
        :returns:
            A list with session data.
        """
        try:
            assert len(data) >= len(self.session_attributes)
            return [data.get(k) for k in self.session_attributes]
        except AssertionError:
            logging.warning(
                'Invalid user data: %r. Expected attributes: %r.' %
                (data, self.session_attributes))
            return None

    def deserialize_session(self, data):
        """Deserializes values for a session.

        :param data:
            A list with session data.
        :returns:
            A dict with session data.
        """
        try:
            assert len(data) >= len(self.session_attributes)
            return dict(zip(self.session_attributes, data))
        except AssertionError:
            logging.warning(
                'Invalid user data: %r. Expected attributes: %r.' %
                (data, self.session_attributes))
            return None

    # Validators --------------------------------------------------------------

    def validate_password(self, auth_id, password, silent=False):
        """Validates a password.

        Passwords are used to log-in using forms or to request auth tokens
        from services.

        :param auth_id:
            Authentication id.
        :param password:
            Password to be checked.
        :param silent:
            If True, raises an exception if auth_id or password are invalid.
        :returns:
            user or None
        :raises:
            ``InvalidAuthIdError`` or ``InvalidPasswordError``.
        """
        return self.get_user_by_auth_password(auth_id, password, silent=silent)

    def validate_token(self, user_id, token, token_ts=None):
        """Validates a token.

        Tokens are random strings used to authenticate temporarily. They are
        used to validate sessions or service requests.

        :param user_id:
            User id.
        :param token:
            Token to be checked.
        :param token_ts:
            Optional token timestamp used to pre-validate the token age.
        :returns:
            A tuple ``(user_dict, token)``.
        """
        now = int(time.time())
        delete = token_ts and ((now - token_ts) > self.config['token_max_age'])
        create = False

        if not delete:
            # Try to fetch the user.
            user, ts = self.get_user_by_auth_token(user_id, token)
            if user:
                # Now validate the real timestamp.
                delete = (now - ts) > self.config['token_max_age']
                create = (now - ts) > self.config['token_new_age']

        if delete or create or not user:
            if delete or create:
                # Delete token from db.
                self.delete_auth_token(user_id, token)

                if delete:
                    user = None

            token = None

        return user, token

    def validate_cache_timestamp(self, cache_ts, token_ts=None):
        """Validates a cache timestamp.

        :param cache_ts:
            Token timestamp to validate the cache age.
        :param token_ts:
            Token timestamp to validate the token age.
        :returns:
            True if it is valid, False otherwise.
        """
        now = int(time.time())
        valid = (now - cache_ts) < self.config['token_cache_age']

        if valid and token_ts:
            valid2 = (now - token_ts) < self.config['token_max_age']
            valid3 = (now - token_ts) < self.config['token_new_age']
            valid = valid2 and valid3

        return valid


class Auth(object):
    """Authentication provider for a single request."""

    #: A :class:`webapp2.Request` instance.
    request = None
    #: An :class:`AuthStore` instance.
    store = None
    #: Cached user for the request.
    _user = None

    def __init__(self, request):
        """Initializes the auth provider for a request.

        :param request:
            A :class:`webapp2.Request` instance.
        """
        self.request = request
        self.store = get_store(app=request.app)

    # Retrieving a user -------------------------------------------------------

    def _user_or_none(self):
        return self._user if self._user is not _anon else None

    def get_user_by_session(self, save_session=True):
        """Returns a user based on the current session.

        :param save_session:
            If True, saves the user in the session if authentication succeeds.
        :returns:
            A user dict or None.
        """
        if self._user is None:
            data = self.get_session_data(pop=True)
            if not data:
                self._user = _anon
            else:
                self._user = self.get_user_by_token(
                    user_id=data['user_id'], token=data['token'],
                    token_ts=data['token_ts'], cache=data,
                    cache_ts=data['cache_ts'], remember=data['remember'],
                    save_session=save_session)

        return self._user_or_none()

    def get_user_by_token(self, user_id, token, token_ts=None, cache=None,
                          cache_ts=None, remember=False, save_session=True):
        """Returns a user based on an authentication token.

        :param user_id:
            User id.
        :param token:
            Authentication token.
        :param token_ts:
            Token timestamp, used to perform pre-validation.
        :param cache:
            Cached user data (from the session).
        :param cache_ts:
            Cache timestamp.
        :param remember:
            If True, saves permanent sessions.
        :param save_session:
            If True, saves the user in the session if authentication succeeds.
        :returns:
            A user dict or None.
        """
        if self._user is not None:
            assert (self._user is not _anon and
                    self._user['user_id'] == user_id and
                    self._user['token'] == token)
            return self._user_or_none()

        if cache and cache_ts:
            valid = self.store.validate_cache_timestamp(cache_ts, token_ts)
            if valid:
                self._user = cache
            else:
                cache_ts = None

        if self._user is None:
            # Fetch and validate the token.
            self._user, token = self.store.validate_token(user_id, token,
                                                          token_ts=token_ts)

        if self._user is None:
            self._user = _anon
        elif save_session:
            if not token:
                token_ts = None

            self.set_session(self._user, token=token, token_ts=token_ts,
                             cache_ts=cache_ts, remember=remember)

        return self._user_or_none()

    def get_user_by_password(self, auth_id, password, remember=False,
                             save_session=True, silent=False):
        """Returns a user based on password credentials.

        :param auth_id:
            Authentication id.
        :param password:
            User password.
        :param remember:
            If True, saves permanent sessions.
        :param save_session:
            If True, saves the user in the session if authentication succeeds.
        :param silent:
            If True, raises an exception if auth_id or password are invalid.
        :returns:
            A user dict or None.
        :raises:
            ``InvalidAuthIdError`` or ``InvalidPasswordError``.
        """
        if save_session:
            # During a login attempt, invalidate current session.
            self.unset_session()

        self._user = self.store.validate_password(auth_id, password,
                                                  silent=silent)
        if not self._user:
            self._user = _anon
        elif save_session:
            # This always creates a new token with new timestamp.
            self.set_session(self._user, remember=remember)

        return self._user_or_none()

    # Storing and removing user from session ----------------------------------

    @webapp2.cached_property
    def session(self):
        """Auth session."""
        return self.store.get_session(self.request)

    def set_session(self, user, token=None, token_ts=None, cache_ts=None,
                    remember=False, **session_args):
        """Saves a user in the session.

        :param user:
            A dictionary with user data.
        :param token:
            A unique token to be persisted. If None, a new one is created.
        :param token_ts:
            Token timestamp. If None, a new one is created.
        :param cache_ts:
            Token cache timestamp. If None, a new one is created.
        :remember:
            If True, session is set to be persisted.
        :param session_args:
            Keyword arguments to set the session arguments.
        """
        now = int(time.time())
        token = token or self.store.create_auth_token(user['user_id'])
        token_ts = token_ts or now
        cache_ts = cache_ts or now
        if remember:
            max_age = self.store.config['token_max_age']
        else:
            max_age = None

        session_args.setdefault('max_age', max_age)
        # Create a new dict or just update user?
        # We are doing the latter, and so the user dict will always have
        # the session metadata (token, timestamps etc). This is easier to test.
        # But we could store only user_id and custom user attributes instead.
        user.update({
            'token':    token,
            'token_ts': token_ts,
            'cache_ts': cache_ts,
            'remember': int(remember),
        })
        self.set_session_data(user, **session_args)
        self._user = user

    def unset_session(self):
        """Removes a user from the session and invalidates the auth token."""
        self._user = None
        data = self.get_session_data(pop=True)
        if data:
            # Invalidate current token.
            self.store.delete_auth_token(data['user_id'], data['token'])

    def get_session_data(self, pop=False):
        """Returns the session data as a dictionary.

        :param pop:
            If True, removes the session.
        :returns:
            A deserialized session, or None.
        """
        func = self.session.pop if pop else self.session.get
        rv = func('_user', None)
        if rv is not None:
            data = self.store.deserialize_session(rv)
            if data:
                return data
            elif not pop:
                self.session.pop('_user', None)

        return None

    def set_session_data(self, data, **session_args):
        """Sets the session data as a list.

        :param data:
            Deserialized session data.
        :param session_args:
            Extra arguments for the session.
        """
        data = self.store.serialize_session(data)
        if data is not None:
            self.session['_user'] = data
            self.session.container.session_args.update(session_args)


# Factories -------------------------------------------------------------------


#: Key used to store :class:`AuthStore` in the app registry.
_store_registry_key = 'webapp2_extras.auth.Auth'
#: Key used to store :class:`Auth` in the request registry.
_auth_registry_key = 'webapp2_extras.auth.Auth'


def get_store(factory=AuthStore, key=_store_registry_key, app=None):
    """Returns an instance of :class:`AuthStore` from the app registry.

    It'll try to get it from the current app registry, and if it is not
    registered it'll be instantiated and registered. A second call to this
    function will return the same instance.

    :param factory:
        The callable used to build and register the instance if it is not yet
        registered. The default is the class :class:`AuthStore` itself.
    :param key:
        The key used to store the instance in the registry. A default is used
        if it is not set.
    :param app:
        A :class:`webapp2.WSGIApplication` instance used to store the instance.
        The active app is used if it is not set.
    """
    app = app or webapp2.get_app()
    store = app.registry.get(key)
    if not store:
        store = app.registry[key] = factory(app)

    return store


def set_store(store, key=_store_registry_key, app=None):
    """Sets an instance of :class:`AuthStore` in the app registry.

    :param store:
        An instance of :class:`AuthStore`.
    :param key:
        The key used to retrieve the instance from the registry. A default
        is used if it is not set.
    :param request:
        A :class:`webapp2.WSGIApplication` instance used to retrieve the
        instance. The active app is used if it is not set.
    """
    app = app or webapp2.get_app()
    app.registry[key] = store


def get_auth(factory=Auth, key=_auth_registry_key, request=None):
    """Returns an instance of :class:`Auth` from the request registry.

    It'll try to get it from the current request registry, and if it is not
    registered it'll be instantiated and registered. A second call to this
    function will return the same instance.

    :param factory:
        The callable used to build and register the instance if it is not yet
        registered. The default is the class :class:`Auth` itself.
    :param key:
        The key used to store the instance in the registry. A default is used
        if it is not set.
    :param request:
        A :class:`webapp2.Request` instance used to store the instance. The
        active request is used if it is not set.
    """
    request = request or webapp2.get_request()
    auth = request.registry.get(key)
    if not auth:
        auth = request.registry[key] = factory(request)

    return auth


def set_auth(auth, key=_auth_registry_key, request=None):
    """Sets an instance of :class:`Auth` in the request registry.

    :param auth:
        An instance of :class:`Auth`.
    :param key:
        The key used to retrieve the instance from the registry. A default
        is used if it is not set.
    :param request:
        A :class:`webapp2.Request` instance used to retrieve the instance. The
        active request is used if it is not set.
    """
    request = request or webapp2.get_request()
    request.registry[key] = auth
