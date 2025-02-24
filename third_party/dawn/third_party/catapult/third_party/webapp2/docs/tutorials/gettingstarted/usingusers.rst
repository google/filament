.. _tutorials.gettingstarted.usingusers:

Using the Users Service
=======================
Google App Engine provides several useful services based on Google
infrastructure, accessible by applications using libraries included with the
SDK. One such service is the Users service, which lets your application
integrate with Google user accounts. With the Users service, your users can
use the Google accounts they already have to sign in to your application.
Let's use the Users service to personalize this application's greeting.


Using Users
-----------
Edit ``helloworld/helloworld.py`` again, and replace its contents with the
following::

    from google.appengine.api import users
    import webapp2

    class MainPage(webapp2.RequestHandler):
        def get(self):
            user = users.get_current_user()

            if user:
                self.response.headers['Content-Type'] = 'text/plain'
                self.response.out.write('Hello, ' + user.nickname())
            else:
                self.redirect(users.create_login_url(self.request.uri))

    application = webapp2.WSGIApplication([
        ('/', MainPage)
    ], debug=True)

    def main():
        application.run()

    if __name__ == "__main__":
        main()

Reload the page in your browser. Your application redirects you to the local
version of the Google sign-in page suitable for testing your application.
You can enter any username you'd like in this screen, and your application
will see a fake ``User`` object based on that username.

When your application is running on App Engine, users will be directed to the
Google Accounts sign-in page, then redirected back to your application after
successfully signing in or creating an account.


The Users API
-------------
Let's take a closer look at the new pieces.

If the user is already signed in to your application, ``get_current_user()``
returns the ``User`` object for the user. Otherwise, it returns ``None``::

    user = users.get_current_user()

If the user has signed in, display a personalized message, using the nickname
associated with the user's account::

    if user:
        self.response.headers['Content-Type'] = 'text/plain'
        self.response.out.write('Hello, ' + user.nickname())

If the user has not signed in, tell ``webapp2`` to redirect the user's browser
to the Google account sign-in screen. The redirect includes the URL to this
page (``self.request.uri``) so the Google account sign-in mechanism will send
the user back here after the user has signed in or registered for a new
account::

    self.redirect(users.create_login_url(self.request.uri))

For more information about the Users API, see the `Users reference <http://code.google.com/appengine/docs/python/users/>`_.


Next...
-------
Our application can now greet visiting users by name. Let's add a feature that
will let users greet each other.

Continue to :ref:`tutorials.gettingstarted.handlingforms`.
