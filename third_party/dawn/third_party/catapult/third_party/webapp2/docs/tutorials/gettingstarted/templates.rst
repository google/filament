.. _tutorials.gettingstarted.templates:

Using Templates
===============
HTML embedded in code is messy and difficult to maintain. It's better to use a
templating system, where the HTML is kept in a separate file with special
syntax to indicate where the data from the application appears. There are many
templating systems for Python: EZT, Cheetah, ClearSilver, Quixote, and Django
are just a few. You can use your template engine of choice by bundling it with
your application code.

For your convenience, the ``webapp2`` module includes Django's templating
engine. Versions 1.2 and 0.96 are included with the SDK and are part of App
Engine, so you do not need to bundle Django yourself to use it.

See the Django section of `Third-party libraries <http://code.google.com/appengine/docs/python/tools/libraries.html#Django>`_
for information on using supported Django versions.


Using Django Templates
----------------------
Add the following import statements at the top of helloworld/helloworld.py::

    import os
    from google.appengine.ext.webapp import template

Replace the ``MainPage`` handler with code that resembles the following::

    class MainPage(webapp2.RequestHandler):
        def get(self):
            guestbook_name=self.request.get('guestbook_name')
            greetings_query = Greeting.all().ancestor(
                guestbook_key(guestbook_name)).order('-date')
            greetings = greetings_query.fetch(10)

            if users.get_current_user():
                url = users.create_logout_url(self.request.uri)
                url_linktext = 'Logout'
            else:
                url = users.create_login_url(self.request.uri)
                url_linktext = 'Login'

            template_values = {
                'greetings': greetings,
                'url': url,
                'url_linktext': url_linktext,
            }

            path = os.path.join(os.path.dirname(__file__), 'index.html')
            self.response.out.write(template.render(path, template_values))

Finally, create a new file in the ``helloworld`` directory named ``index.html``,
with the following contents:

.. code-block:: html+django

   <html>
     <body>
       {% for greeting in greetings %}
         {% if greeting.author %}
           <b>{{ greeting.author.nickname }}</b> wrote:
         {% else %}
           An anonymous person wrote:
         {% endif %}
         <blockquote>{{ greeting.content|escape }}</blockquote>
       {% endfor %}

       <form action="/sign" method="post">
         <div><textarea name="content" rows="3" cols="60"></textarea></div>
         <div><input type="submit" value="Sign Guestbook"></div>
       </form>

       <a href="{{ url }}">{{ url_linktext }}</a>
     </body>
   </html>

Reload the page, and try it out.

``template.render(path, template_values)`` takes a file path to the template
file and a dictionary of values, and returns the rendered text. The template
uses Django templating syntax to access and iterate over the values, and can
refer to properties of those values. In many cases, you can pass datastore
model objects directly as values, and access their properties from templates.

.. note::
   An App Engine application has read-only access to all of the files uploaded
   with the project, the library modules, and no other files. The current
   working directory is the application root directory, so the path to
   ``index.html`` is simply ``"index.html"``.


Next...
-------
Every web application returns dynamically generated HTML from the application
code, via templates or some other mechanism. Most web applications also need
to serve static content, such as images, CSS stylesheets, or JavaScript files.
For efficiency, App Engine treats static files differently from application
source and data files. You can use App Engine's static files feature to serve
a CSS stylesheet for this application.

Continue to :ref:`tutorials.gettingstarted.staticfiles`.
