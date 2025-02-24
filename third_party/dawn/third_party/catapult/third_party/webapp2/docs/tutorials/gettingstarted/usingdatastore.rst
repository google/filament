.. _tutorials.gettingstarted.usingdatastore:

Using the Datastore
===================
Storing data in a scalable web application can be tricky. A user could be
interacting with any of dozens of web servers at a given time, and the user's
next request could go to a different web server than the one that handled the
previous request. All web servers need to be interacting with data that is
also spread out across dozens of machines, possibly in different locations
around the world.

Thanks to Google App Engine, you don't have to worry about any of that.
App Engine's infrastructure takes care of all of the distribution, replication
and load balancing of data behind a simple API -- and you get a powerful
query engine and transactions as well.

The default datastore for an application is now the `High Replication datastore <http://code.google.com/appengine/docs/python/datastore/hr/>`_.
This datastore uses the `Paxos algorithm <http://labs.google.com/papers/paxos_made_live.html>`_
to replicate data across datacenters. The High Replication datastore is
extremely resilient in the face of catastrophic failure.

One of the consequences of this is that the consistency guarantee for the
datastore may differ from what you are familiar with. It also differs slightly
from the Master/Slave datastore, the other datastore option that App Engine
offers. In the example code comments, we highlight some ways this might affect
the design of your app. For more detailed information,
see `Using the High Replication Datastore <http://code.google.com/appengine/docs/python/datastore/hr/overview.html>`_
(HRD).

The datastore writes data in objects known as entities, and each entity has a
key that identifies the entity. Entities can belong to the same entity group,
which allows you to perform a single transaction with multiple entities.
Entity groups have a parent key that identifies the entire entity group.

In the High Replication Datastore, entity groups are also a unit of
consistency. Queries over multiple entity groups may return stale, `eventually consistent <http://en.wikipedia.org/wiki/Eventual_consistency>`_
results. Queries over a single entity group return up-to-date, strongly
consistent, results. Queries over a single entity group are called ancestor
queries. Ancestor queries use the parent key (instead of a specific entity's
key).

The code samples in this guide organize like entities into entity groups, and
use ancestor queries on those entity groups to return strongly consistent
results. In the example code comments, we highlight some ways this might affect
the design of your app. For more detailed information,
see `Using the High Replication Datastore <http://code.google.com/appengine/docs/python/datastore/hr/overview.html>`_.


A Complete Example Using the Datastore
--------------------------------------
Here is a new version of ``helloworld/helloworld.py`` that stores greetings
in the datastore. The rest of this page discusses the new pieces::

    import cgi
    import datetime
    import urllib
    import wsgiref.handlers

    from google.appengine.ext import db
    from google.appengine.api import users
    import webapp2


    class Greeting(db.Model):
      """Models an individual Guestbook entry with an author, content, and date."""
      author = db.UserProperty()
      content = db.StringProperty(multiline=True)
      date = db.DateTimeProperty(auto_now_add=True)


    def guestbook_key(guestbook_name=None):
      """Constructs a datastore key for a Guestbook entity with guestbook_name."""
      return db.Key.from_path('Guestbook', guestbook_name or 'default_guestbook')


    class MainPage(webapp2.RequestHandler):
      def get(self):
        self.response.out.write('<html><body>')
        guestbook_name=self.request.get('guestbook_name')

        # Ancestor Queries, as shown here, are strongly consistent with the High
        # Replication datastore. Queries that span entity groups are eventually
        # consistent. If we omitted the ancestor from this query there would be a
        # slight chance that Greeting that had just been written would not show up
        # in a query.
        greetings = db.GqlQuery("SELECT * "
                                "FROM Greeting "
                                "WHERE ANCESTOR IS :1 "
                                "ORDER BY date DESC LIMIT 10",
                                guestbook_key(guestbook_name))

        for greeting in greetings:
          if greeting.author:
            self.response.out.write(
                '<b>%s</b> wrote:' % greeting.author.nickname())
          else:
            self.response.out.write('An anonymous person wrote:')
          self.response.out.write('<blockquote>%s</blockquote>' %
                                  cgi.escape(greeting.content))

        self.response.out.write("""
              <form action="/sign?%s" method="post">
                <div><textarea name="content" rows="3" cols="60"></textarea></div>
                <div><input type="submit" value="Sign Guestbook"></div>
              </form>
              <hr>
              <form>Guestbook name: <input value="%s" name="guestbook_name">
              <input type="submit" value="switch"></form>
            </body>
          </html>""" % (urllib.urlencode({'guestbook_name': guestbook_name}),
                              cgi.escape(guestbook_name)))


    class Guestbook(webapp2.RequestHandler):
      def post(self):
        # We set the same parent key on the 'Greeting' to ensure each greeting is in
        # the same entity group. Queries across the single entity group will be
        # consistent. However, the write rate to a single entity group should
        # be limited to ~1/second.
        guestbook_name = self.request.get('guestbook_name')
        greeting = Greeting(parent=guestbook_key(guestbook_name))

        if users.get_current_user():
          greeting.author = users.get_current_user()

        greeting.content = self.request.get('content')
        greeting.put()
        self.redirect('/?' + urllib.urlencode({'guestbook_name': guestbook_name}))


    application = webapp2.WSGIApplication([
      ('/', MainPage),
      ('/sign', Guestbook)
    ], debug=True)


    def main():
      application.RUN()


    if __name__ == '__main__':
      main()

Replace ``helloworld/helloworld.py`` with this, then reload
`http://localhost:8080/ <http://localhost:8080/>`_ in your browser. Post a
few messages to verify that messages get stored and displayed correctly.


Storing the Submitted Greetings
-------------------------------
App Engine includes a data modeling API for Python. It's similar to Django's
data modeling API, but uses App Engine's scalable datastore behind the scenes.

For the guestbook application, we want to store greetings posted by users.
Each greeting includes the author's name, the message content, and the date
and time the message was posted so we can display messages in chronological
order.

To use the data modeling API, import the ``google.appengine.ext.db`` module::

    from google.appengine.ext import db

The following defines a data model for a greeting::

    class Greeting(db.Model):
        author = db.UserProperty()
        content = db.StringProperty(multiline=True)
        date = db.DateTimeProperty(auto_now_add=True)

This defines a ``Greeting`` model with three properties: ``author`` whose
value is a ``User`` object, content whose value is a string, and ``date`` whose
value is a ``datetime.datetime``.

Some property constructors take parameters to further configure their behavior.
Giving the ``db.StringProperty`` constructor the ``multiline=True`` parameter
says that values for this property can contain newline characters. Giving the
``db.DateTimeProperty`` constructor a ``auto_now_add=True`` parameter
configures the model to automatically give new objects a ``date`` of the time
the object is created, if the application doesn't otherwise provide a value.
For a complete list of property types and their options, see `the Datastore reference <http://code.google.com/appengine/docs/python/datastore/>`_.

Now that we have a data model for greetings, the application can use the model
to create new ``Greeting`` objects and put them into the datastore. The following
new version of the ``Guestbook`` handler creates new greetings and saves them
to the datastore::

    class Guestbook(webapp2.RequestHandler):
        def post(self):
          guestbook_name = self.request.get('guestbook_name')
          greeting = Greeting(parent=guestbook_key(guestbook_name))

          if users.get_current_user():
            greeting.author = users.get_current_user()

          greeting.content = self.request.get('content')
          greeting.put()
          self.redirect('/?' + urllib.urlencode({'guestbook_name': guestbook_name}))

This new ``Guestbook`` handler creates a new ``Greeting`` object, then sets its
``author`` and ``content`` properties with the data posted by the user.
The parent has an entity kind "Guestbook". There is no need to create the
"Guestbook" entity before setting it to be the parent of another entity. In
this example, the parent is used as a placeholder for transaction and
consistency purposes. See `Entity Groups and Ancestor Paths <http://code.google.com/appengine/docs/python/datastore/entities.html#Entity_Groups_and_Ancestor_Paths>`_
for more information. Objects that share a common `ancestor <http://code.google.com/appengine/docs/python/datastore/queryclass.html#Query_ancestor>`_
belong to the same entity group. It does not set the date property, so date is
automatically set to "now," as we configured the model to do.

Finally, ``greeting.put()`` saves our new object to the datastore. If we had
acquired this object from a query, ``put()`` would have updated the existing
object. Since we created this object with the model constructor, ``put()`` adds
the new object to the datastore.

Because querying in the High Replication datastore is only strongly consistent
within entity groups, we assign all Greetings to the same entity group in this
example by setting the same parent for each Greeting. This means a user will
always see a Greeting immediately after it was written. However, the rate at
which you can write to the same entity group is limited to 1 write to the
entity group per second. When you design a real application you'll need to
keep this fact in mind. Note that by using services such as `Memcache <http://code.google.com/appengine/docs/python/memcache/>`_,
you can mitigate the chance that a user won't see fresh results when querying
across entity groups immediately after a write.


Retrieving the Stored Greetings With GQL
----------------------------------------
The App Engine datastore has a sophisticated query engine for data models.
Because the App Engine datastore is not a traditional relational database,
queries are not specified using SQL. Instead, you can prepare queries using a
SQL-like query language we call GQL. GQL provides access to the App Engine
datastore query engine's features using a familiar syntax.

The following new version of the ``MainPage`` handler queries the datastore
for greetings::

    class MainPage(webapp2.RequestHandler):
        def get(self):
            self.response.out.write('<html><body>')
            guestbook_name=self.request.get('guestbook_name')

            greetings = db.GqlQuery("SELECT * "
                                    "FROM Greeting "
                                    "WHERE ANCESTOR IS :1 "
                                    "ORDER BY date DESC LIMIT 10",
                                    guestbook_key(guestbook_name))


            for greeting in greetings:
                if greeting.author:
                    self.response.out.write('<b>%s</b> wrote:' % greeting.author.nickname())
                else:
                    self.response.out.write('An anonymous person wrote:')
                self.response.out.write('<blockquote>%s</blockquote>' %
                                        cgi.escape(greeting.content))

            # Write the submission form and the footer of the page
            self.response.out.write("""
                  <form action="/sign" method="post">
                    <div><textarea name="content" rows="3" cols="60"></textarea></div>
                    <div><input type="submit" value="Sign Guestbook"></div>
                  </form>
                </body>
              </html>""")

The query happens here::

    greetings = db.GqlQuery("SELECT * "
                            "FROM Greeting "
                            "WHERE ANCESTOR IS :1 "
                            "ORDER BY date DESC LIMIT 10",
                             guestbook_key(guestbook_name))

Alternatively, you can call the ``gql(...)`` method on the ``Greeting`` class,
and omit the ``SELECT * FROM Greeting`` from the query::

    greetings = Greeting.gql("WHERE ANCESTOR IS :1 ORDER BY date DESC LIMIT 10",
                             guestbook_key(guestbook_name))

As with SQL, keywords (such as ``SELECT``) are case insensitive. Names,
however, are case sensitive.

Because the query returns full data objects, it does not make sense to select
specific properties from the model. All GQL queries start with
``SELECT * FROM model`` (or are so implied by the model's ``gql(...)`` method)
so as to resemble their SQL equivalents.

A GQL query can have a ``WHERE`` clause that filters the result set by one or
more conditions based on property values. Unlike SQL, GQL queries may not
contain value constants: Instead, GQL uses parameter binding for all values
in queries. For example, to get only the greetings posted by the current user::

    if users.get_current_user():
        greetings = Greeting.gql(
            "WHERE ANCESTOR IS :1 AND author = :2 ORDER BY date DESC",
            guestbook_key(guestbook_name), users.get_current_user())

You can also use named parameters instead of positional parameters::

    greetings = Greeting.gql("WHERE ANCESTOR = :ancestor AND author = :author ORDER BY date DESC",
                             ancestor=guestbook_key(guestbook_name), author=users.get_current_user())

In addition to GQL, the datastore API provides another mechanism for building
query objects using methods. The query above could also be prepared as follows::

    greetings = Greeting.all()
    greetings.ancestor(guestbook_key(guestbook_name))
    greetings.filter("author =", users.get_current_user())
    greetings.order("-date")

For a complete description of GQL and the query APIs, see the `Datastore reference <http://code.google.com/appengine/docs/python/datastore/>`_.


Clearing the Development Server Datastore
-----------------------------------------
The development web server uses a local version of the datastore for testing
your application, using temporary files. The data persists as long as the
temporary files exist, and the web server does not reset these files unless
you ask it to do so.

If you want the development server to erase its datastore prior to starting up,
use the ``--clear_datastore`` option when starting the server:

.. code-block:: text

   dev_appserver.py --clear_datastore helloworld/


Next...
-------
We now have a working guest book application that authenticates users using
Google accounts, lets them submit messages, and displays messages other users
have left. Because App Engine handles scaling automatically, we will not need
to revisit this code as our application gets popular.

This latest version mixes HTML content with the code for the ``MainPage``
handler. This will make it difficult to change the appearance of the application,
especially as our application gets bigger and more complex. Let's use
templates to manage the appearance, and introduce static files for a CSS
stylesheet.

Continue to :ref:`tutorials.gettingstarted.templates`.
