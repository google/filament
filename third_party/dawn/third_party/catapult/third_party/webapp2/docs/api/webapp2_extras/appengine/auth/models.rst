.. _api.webapp2_extras.appengine.auth.models:

User Models
===========
.. module:: webapp2_extras.appengine.auth.models

A collection of user related models for :ref:`api.webapp2_extras.auth`.

.. warning::
   This is an experimental module. The API is subject to changes.

.. autoclass:: User
   :members: get_by_auth_id, get_by_auth_token, get_by_auth_password,
             create_user, validate_token,
             create_auth_token, validate_auth_token, delete_auth_token,
             create_signup_token, validate_signup_token, delete_signup_token

.. autoclass:: UserToken
   :members: get_key, create, get

.. autoclass:: Unique
   :members: create, create_multi, delete_multi


.. _NDB: http://code.google.com/p/appengine-ndb-experiment/
