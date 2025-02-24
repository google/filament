# -*- coding: utf-8 -*-
"""
    webapp2_extras.appengine.auth.models
    ====================================

    Auth related models.

    :copyright: 2011 by tipfy.org.
    :license: Apache Sotware License, see LICENSE for details.
"""
import time

try:
    from ndb import model
except ImportError: # pragma: no cover
    from google.appengine.ext.ndb import model

from webapp2_extras import auth
from webapp2_extras import security


class Unique(model.Model):
    """A model to store unique values.

    The only purpose of this model is to "reserve" values that must be unique
    within a given scope, as a workaround because datastore doesn't support
    the concept of uniqueness for entity properties.

    For example, suppose we have a model `User` with three properties that
    must be unique across a given group: `username`, `auth_id` and `email`::

        class User(model.Model):
            username = model.StringProperty(required=True)
            auth_id = model.StringProperty(required=True)
            email = model.StringProperty(required=True)

    To ensure property uniqueness when creating a new `User`, we first create
    `Unique` records for those properties, and if everything goes well we can
    save the new `User` record::

        @classmethod
        def create_user(cls, username, auth_id, email):
            # Assemble the unique values for a given class and attribute scope.
            uniques = [
                'User.username.%s' % username,
                'User.auth_id.%s' % auth_id,
                'User.email.%s' % email,
            ]

            # Create the unique username, auth_id and email.
            success, existing = Unique.create_multi(uniques)

            if success:
                # The unique values were created, so we can save the user.
                user = User(username=username, auth_id=auth_id, email=email)
                user.put()
                return user
            else:
                # At least one of the values is not unique.
                # Make a list of the property names that failed.
                props = [name.split('.', 2)[1] for name in uniques]
                raise ValueError('Properties %r are not unique.' % props)

    Based on the idea from http://goo.gl/pBQhB
    """

    @classmethod
    def create(cls, value):
        """Creates a new unique value.

        :param value:
            The value to be unique, as a string.

            The value should include the scope in which the value must be
            unique (ancestor, namespace, kind and/or property name).

            For example, for a unique property `email` from kind `User`, the
            value can be `User.email:me@myself.com`. In this case `User.email`
            is the scope, and `me@myself.com` is the value to be unique.
        :returns:
            True if the unique value was created, False otherwise.
        """
        entity = cls(key=model.Key(cls, value))
        txn = lambda: entity.put() if not entity.key.get() else None
        return model.transaction(txn) is not None

    @classmethod
    def create_multi(cls, values):
        """Creates multiple unique values at once.

        :param values:
            A sequence of values to be unique. See :meth:`create`.
        :returns:
            A tuple (bool, list_of_keys). If all values were created, bool is
            True and list_of_keys is empty. If one or more values weren't
            created, bool is False and the list contains all the values that
            already existed in datastore during the creation attempt.
        """
        # Maybe do a preliminary check, before going for transactions?
        # entities = model.get_multi(keys)
        # existing = [entity.key.id() for entity in entities if entity]
        # if existing:
        #    return False, existing

        # Create all records transactionally.
        keys = [model.Key(cls, value) for value in values]
        entities = [cls(key=key) for key in keys]
        func = lambda e: e.put() if not e.key.get() else None
        created = [model.transaction(lambda: func(e)) for e in entities]

        if created != keys:
            # A poor man's "rollback": delete all recently created records.
            model.delete_multi(k for k in created if k)
            return False, [k.id() for k in keys if k not in created]

        return True, []

    @classmethod
    def delete_multi(cls, values):
        """Deletes multiple unique values at once.

        :param values:
            A sequence of values to be deleted.
        """
        return model.delete_multi(model.Key(cls, v) for v in values)


class UserToken(model.Model):
    """Stores validation tokens for users."""

    created = model.DateTimeProperty(auto_now_add=True)
    updated = model.DateTimeProperty(auto_now=True)
    user = model.StringProperty(required=True, indexed=False)
    subject = model.StringProperty(required=True)
    token = model.StringProperty(required=True)

    @classmethod
    def get_key(cls, user, subject, token):
        """Returns a token key.

        :param user:
            User unique ID.
        :param subject:
            The subject of the key. Examples:

            - 'auth'
            - 'signup'
        :param token:
            Randomly generated token.
        :returns:
            ``model.Key`` containing a string id in the following format:
            ``{user_id}.{subject}.{token}.``
        """
        return model.Key(cls, '%s.%s.%s' % (str(user), subject, token))

    @classmethod
    def create(cls, user, subject, token=None):
        """Creates a new token for the given user.

        :param user:
            User unique ID.
        :param subject:
            The subject of the key. Examples:

            - 'auth'
            - 'signup'
        :param token:
            Optionally an existing token may be provided.
            If None, a random token will be generated.
        :returns:
            The newly created :class:`UserToken`.
        """
        user = str(user)
        token = token or security.generate_random_string(entropy=128)
        key = cls.get_key(user, subject, token)
        entity = cls(key=key, user=user, subject=subject, token=token)
        entity.put()
        return entity

    @classmethod
    def get(cls, user=None, subject=None, token=None):
        """Fetches a user token.

        :param user:
            User unique ID.
        :param subject:
            The subject of the key. Examples:

            - 'auth'
            - 'signup'
        :param token:
            The existing token needing verified.
        :returns:
            A :class:`UserToken` or None if the token does not exist.
        """
        if user and subject and token:
            return cls.get_key(user, subject, token).get()

        assert subject and token, \
            'subject and token must be provided to UserToken.get().'
        return cls.query(cls.subject == subject, cls.token == token).get()


class User(model.Expando):
    """Stores user authentication credentials or authorization ids."""

    #: The model used to ensure uniqueness.
    unique_model = Unique
    #: The model used to store tokens.
    token_model = UserToken

    created = model.DateTimeProperty(auto_now_add=True)
    updated = model.DateTimeProperty(auto_now=True)
    # ID for third party authentication, e.g. 'google:username'. UNIQUE.
    auth_ids = model.StringProperty(repeated=True)
    # Hashed password. Not required because third party authentication
    # doesn't use password.
    password = model.StringProperty()

    def get_id(self):
        """Returns this user's unique ID, which can be an integer or string."""
        return self._key.id()

    def add_auth_id(self, auth_id):
        """A helper method to add additional auth ids to a User

        :param auth_id:
            String representing a unique id for the user. Examples:

            - own:username
            - google:username
        :returns:
            A tuple (boolean, info). The boolean indicates if the user
            was saved. If creation succeeds, ``info`` is the user entity;
            otherwise it is a list of duplicated unique properties that
            caused creation to fail.
        """
        self.auth_ids.append(auth_id)
        unique = '%s.auth_id:%s' % (self.__class__.__name__, auth_id)
        ok = self.unique_model.create(unique)
        if ok:
            self.put()
            return True, self
        else:
            return False, ['auth_id']

    @classmethod
    def get_by_auth_id(cls, auth_id):
        """Returns a user object based on a auth_id.

        :param auth_id:
            String representing a unique id for the user. Examples:

            - own:username
            - google:username
        :returns:
            A user object.
        """
        return cls.query(cls.auth_ids == auth_id).get()

    @classmethod
    def get_by_auth_token(cls, user_id, token):
        """Returns a user object based on a user ID and token.

        :param user_id:
            The user_id of the requesting user.
        :param token:
            The token string to be verified.
        :returns:
            A tuple ``(User, timestamp)``, with a user object and
            the token timestamp, or ``(None, None)`` if both were not found.
        """
        token_key = cls.token_model.get_key(user_id, 'auth', token)
        user_key = model.Key(cls, user_id)
        # Use get_multi() to save a RPC call.
        valid_token, user = model.get_multi([token_key, user_key])
        if valid_token and user:
            timestamp = int(time.mktime(valid_token.created.timetuple()))
            return user, timestamp

        return None, None

    @classmethod
    def get_by_auth_password(cls, auth_id, password):
        """Returns a user object, validating password.

        :param auth_id:
            Authentication id.
        :param password:
            Password to be checked.
        :returns:
            A user object, if found and password matches.
        :raises:
            ``auth.InvalidAuthIdError`` or ``auth.InvalidPasswordError``.
        """
        user = cls.get_by_auth_id(auth_id)
        if not user:
            raise auth.InvalidAuthIdError()

        if not security.check_password_hash(password, user.password):
            raise auth.InvalidPasswordError()

        return user

    @classmethod
    def validate_token(cls, user_id, subject, token):
        """Checks for existence of a token, given user_id, subject and token.

        :param user_id:
            User unique ID.
        :param subject:
            The subject of the key. Examples:

            - 'auth'
            - 'signup'
        :param token:
            The token string to be validated.
        :returns:
            A :class:`UserToken` or None if the token does not exist.
        """
        return cls.token_model.get(user=user_id, subject=subject,
                                   token=token) is not None

    @classmethod
    def create_auth_token(cls, user_id):
        """Creates a new authorization token for a given user ID.

        :param user_id:
            User unique ID.
        :returns:
            A string with the authorization token.
        """
        return cls.token_model.create(user_id, 'auth').token

    @classmethod
    def validate_auth_token(cls, user_id, token):
        return cls.validate_token(user_id, 'auth', token)

    @classmethod
    def delete_auth_token(cls, user_id, token):
        """Deletes a given authorization token.

        :param user_id:
            User unique ID.
        :param token:
            A string with the authorization token.
        """
        cls.token_model.get_key(user_id, 'auth', token).delete()

    @classmethod
    def create_signup_token(cls, user_id):
        entity = cls.token_model.create(user_id, 'signup')
        return entity.token

    @classmethod
    def validate_signup_token(cls, user_id, token):
        return cls.validate_token(user_id, 'signup', token)

    @classmethod
    def delete_signup_token(cls, user_id, token):
        cls.token_model.get_key(user_id, 'signup', token).delete()

    @classmethod
    def create_user(cls, auth_id, unique_properties=None, **user_values):
        """Creates a new user.

        :param auth_id:
            A string that is unique to the user. Users may have multiple
            auth ids. Example auth ids:

            - own:username
            - own:email@example.com
            - google:username
            - yahoo:username

            The value of `auth_id` must be unique.
        :param unique_properties:
            Sequence of extra property names that must be unique.
        :param user_values:
            Keyword arguments to create a new user entity. Since the model is
            an ``Expando``, any provided custom properties will be saved.
            To hash a plain password, pass a keyword ``password_raw``.
        :returns:
            A tuple (boolean, info). The boolean indicates if the user
            was created. If creation succeeds, ``info`` is the user entity;
            otherwise it is a list of duplicated unique properties that
            caused creation to fail.
        """
        assert user_values.get('password') is None, \
            'Use password_raw instead of password to create new users.'

        assert not isinstance(auth_id, list), \
            'Creating a user with multiple auth_ids is not allowed, ' \
            'please provide a single auth_id.'

        if 'password_raw' in user_values:
            user_values['password'] = security.generate_password_hash(
                user_values.pop('password_raw'), length=12)

        user_values['auth_ids'] = [auth_id]
        user = cls(**user_values)

        # Set up unique properties.
        uniques = [('%s.auth_id:%s' % (cls.__name__, auth_id), 'auth_id')]
        if unique_properties:
            for name in unique_properties:
                key = '%s.%s:%s' % (cls.__name__, name, user_values[name])
                uniques.append((key, name))

        ok, existing = cls.unique_model.create_multi(k for k, v in uniques)
        if ok:
            user.put()
            return True, user
        else:
            properties = [v for k, v in uniques if k in existing]
            return False, properties
