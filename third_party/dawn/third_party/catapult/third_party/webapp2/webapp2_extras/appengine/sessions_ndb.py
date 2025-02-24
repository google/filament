# -*- coding: utf-8 -*-
"""
    webapp2_extras.appengine.sessions_ndb
    =====================================

    Extended sessions stored in datastore using the ndb library.

    :copyright: 2011 by tipfy.org.
    :license: Apache Sotware License, see LICENSE for details.
"""
from __future__ import absolute_import

from google.appengine.api import memcache

try:
    from ndb import model
except ImportError: # pragma: no cover
    from google.appengine.ext.ndb import model

try:
    from ndb.model import PickleProperty
except ImportError: # pragma: no cover
    try:
        from google.appengine.ext.ndb.model import PickleProperty
    except ImportError: # pragma: no cover
        # ndb in SDK 1.6.1 doesn't have PickleProperty.
        import pickle

        class PickleProperty(model.BlobProperty):
            """A Property whose value is any picklable Python object."""

            def _validate(self, value):
                return value

            def _db_set_value(self, v, p, value):
                super(PickleProperty, self)._db_set_value(v, p,
                    pickle.dumps(value))

            def _db_get_value(self, v, p):
                if not v.has_stringvalue():
                    return None

                return pickle.loads(v.stringvalue())


from webapp2_extras import sessions

class Session(model.Model):
    """A model to store session data."""

    #: Save time.
    updated = model.DateTimeProperty(auto_now=True)
    #: Session data, pickled.
    data = PickleProperty()

    @classmethod
    def get_by_sid(cls, sid):
        """Returns a ``Session`` instance by session id.

        :param sid:
            A session id.
        :returns:
            An existing ``Session`` entity.
        """
        data = memcache.get(sid)
        if not data:
            session = model.Key(cls, sid).get()
            if session:
                data = session.data
                memcache.set(sid, data)

        return data

    def _put(self):
        """Saves the session and updates the memcache entry."""
        memcache.set(self._key.id(), self.data)
        super(Session, self).put()


class DatastoreSessionFactory(sessions.CustomBackendSessionFactory):
    """A session factory that stores data serialized in datastore.

    To use datastore sessions, pass this class as the `factory` keyword to
    :meth:`webapp2_extras.sessions.SessionStore.get_session`::

        from webapp2_extras import sessions_ndb

        # [...]

        session = self.session_store.get_session(
            name='db_session', factory=sessions_ndb.DatastoreSessionFactory)

    See in :meth:`webapp2_extras.sessions.SessionStore` an example of how to
    make sessions available in a :class:`webapp2.RequestHandler`.
    """

    #: The session model class.
    session_model = Session

    def _get_by_sid(self, sid):
        """Returns a session given a session id."""
        if self._is_valid_sid(sid):
            data = self.session_model.get_by_sid(sid)
            if data is not None:
                self.sid = sid
                return sessions.SessionDict(self, data=data)

        self.sid = self._get_new_sid()
        return sessions.SessionDict(self, new=True)

    def save_session(self, response):
        if self.session is None or not self.session.modified:
            return

        self.session_model(id=self.sid, data=dict(self.session))._put()
        self.session_store.save_secure_cookie(
            response, self.name, {'_sid': self.sid}, **self.session_args)
