.. _auth.i18n:

Authentication with webapp2
===========================

Login with forms
----------------

Login with sessions
-------------------

Login with tokens
-----------------


Custom User model
-----------------
:mod:`webapp2_extras.appengine.auth.models` provides a default ``User`` model
to be used on App Engine, but it can be replaced by any custom model that
implements the required interface. This means that :mod:`webapp2_extras.auth`
can be used with any model you wish -- even non-App Engine models which use,
let's say, ``SQLAlchemy`` or other abstraction layers.

The required interface that a custom user model must implement consists of
only five methods::

    class User(object):

        def get_id(self):
            """Returns this user's unique ID, which can be an integer or string."""

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

        @classmethod
        def create_auth_token(cls, user_id):
            """Creates a new authorization token for a given user ID.

            :param user_id:
                User unique ID.
            :returns:
                A string with the authorization token.
            """

        @classmethod
        def delete_auth_token(cls, user_id, token):
            """Deletes a given authorization token.

            :param user_id:
                User unique ID.
            :param token:
                A string with the authorization token.
            """

Additionally, all values configured for ``user_attributes``, if any, must
be provided by the user object as attributes. These values are stored in the
session, providing a nice way to cache commonly used user information.
