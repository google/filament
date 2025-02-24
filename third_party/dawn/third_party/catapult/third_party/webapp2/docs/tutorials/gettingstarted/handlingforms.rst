.. _tutorials.gettingstarted.handlingforms:

Handling Forms with webapp2
===========================
If we want users to be able to post their own greetings, we need a way to
process information submitted by the user with a web form. The ``webapp2``
framework makes processing form data easy.


Handling Web Forms With webapp2
-------------------------------
Replace the contents of ``helloworld/helloworld.py`` with the following::

    import cgi

    from google.appengine.api import users
    import webapp2

    class MainPage(webapp2.RequestHandler):
        def get(self):
            self.response.out.write("""
              <html>
                <body>
                  <form action="/sign" method="post">
                    <div><textarea name="content" rows="3" cols="60"></textarea></div>
                    <div><input type="submit" value="Sign Guestbook"></div>
                  </form>
                </body>
              </html>""")

    class Guestbook(webapp2.RequestHandler):
        def post(self):
            self.response.out.write('<html><body>You wrote:<pre>')
            self.response.out.write(cgi.escape(self.request.get('content')))
            self.response.out.write('</pre></body></html>')

    application = webapp2.WSGIApplication([
        ('/', MainPage),
        ('/sign', Guestbook)
    ], debug=True)

    def main():
        application.run()

    if __name__ == "__main__":
        main()

Reload the page to see the form, then try submitting a message.

This version has two handlers: ``MainPage``, mapped to the URL ``/``, displays
a web form. ``Guestbook``, mapped to the URL ``/sign``, displays the data
submitted by the web form.

The ``Guestbook`` handler has a ``post()`` method instead of a ``get()``
method. This is because the form displayed by ``MainPage`` uses the HTTP POST
method (``method="post"``) to submit the form data. If for some reason you
need a single handler to handle both GET and POST actions to the same URL, you
can define a method for each action in the same class.

The code for the ``post()`` method gets the form data from ``self.request``.
Before displaying it back to the user, it uses ``cgi.escape()`` to escape
HTML special characters to their character entity equivalents. ``cgi`` is a
module in the standard Python library; see `the documentation for cgi <http://www.python.org/doc/2.5.2/lib/module-cgi.html>`_
for more information.

.. note::
   The App Engine environment includes the entire Python 2.5 standard library.
   However, not all actions are allowed. App Engine applications run in a
   restricted environment that allows App Engine to scale them safely.
   For example, low-level calls to the operating system, networking operations,
   and some filesystem operations are not allowed, and will raise an error
   when attempted. For more information, see `The Python Runtime Environment <http://code.google.com/appengine/docs/python/>`_.


Next...
-------
Now that we can collect information from the user, we need a place to put it
and a way to get it back.

Continue to :ref:`tutorials.gettingstarted.usingdatastore`.
