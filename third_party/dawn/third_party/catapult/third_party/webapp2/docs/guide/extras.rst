webapp2_extras
==============
webapp2_extras is a package with common utilities that work well with
webapp2. It includes:

- Localization and internationalization support
- Sessions using secure cookies, memcache or datastore
- Extra route classes -- to match subdomains and other conveniences
- Support for third party libraries: Jinja2 and Mako
- Support for threaded environments, so that you can use webapp2 outside of
  App Engine or in the upcoming App Engine Python 2.7 runtime

Some of these modules (:ref:`api.webapp2_extras.i18n`, :ref:`api.webapp2_extras.jinja2`,
:ref:`api.webapp2_extras.mako` and :ref:`api.webapp2_extras.sessions`) use configuration
values that can be set in the WSGI application. When a config key is not set,
the modules will use the default values they define.

All configuration keys are optional, except ``secret_key`` that must be set
for :ref:`api.webapp2_extras.sessions`. Here is an example that sets the ``secret_key``
configuration and tests that the session is working::

    import webapp2
    from webapp2_extras import sessions

    class BaseHandler(webapp2.RequestHandler):
        def dispatch(self):
            # Get a session store for this request.
            self.session_store = sessions.get_store(request=self.request)

            try:
                # Dispatch the request.
                webapp2.RequestHandler.dispatch(self)
            finally:
                # Save all sessions.
                self.session_store.save_sessions(self.response)

        @webapp2.cached_property
        def session(self):
            # Returns a session using the default cookie key.
            return self.session_store.get_session()

    class HomeHandler(BaseHandler):
        def get(self):
            test_value = self.session.get('test-value')
            if test_value:
                self.response.write('Session has this value: %r.' % test_value)
            else:
                self.session['test-value'] = 'Hello, session world!'
                self.response.write('Session is empty.')

    config = {}
    config['webapp2_extras.sessions'] = {
        'secret_key': 'some-secret-key',
    }

    app = webapp2.WSGIAppplication([
        ('/', HomeHandler),
    ], debug=True, config=config)

    def main():
        app.run()

    if __name__ == '__main__':
        main()
