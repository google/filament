.. _tutorials.gettingstarted.staticfiles:

Using Static Files
==================
Unlike a traditional web hosting environment, Google App Engine does not serve
files directly out of your application's source directory unless configured
to do so. We named our template file ``index.html``, but this does not
automatically make the file available at the URL ``/index.html``.

But there are many cases where you want to serve static files directly to the
web browser. Images, CSS stylesheets, JavaScript code, movies and Flash
animations are all typically stored with a web application and served directly
to the browser. You can tell App Engine to serve specific files directly
without your having to code your own handler.


Using Static Files
------------------
Edit ``helloworld/app.yaml`` and replace its contents with the following:

.. code-block:: yaml

   application: helloworld
   version: 1
   runtime: python27
   api_version: 1
   threadsafe: true

   handlers:
   - url: /stylesheets
     static_dir: stylesheets

   - url: /.*
     script: helloworld.app

The new ``handlers`` section defines two handlers for URLs. When App Engine
receives a request with a URL beginning with ``/stylesheets``, it maps the
remainder of the path to files in the stylesheets directory and, if an
appropriate file is found, the contents of the file are returned to the client.
All other URLs match the ``/`` path, and are handled by the ``helloworld.py``
script.

By default, App Engine serves static files using a MIME type based on the
filename extension. For example, a file with a name ending in ``.css`` will be
served with a MIME type of ``text/css``. You can configure explicit MIME types
with additional options.

URL handler path patterns are tested in the order they appear in ``app.yaml``,
from top to bottom. In this case, the ``/stylesheets`` pattern will match
before the ``/.*`` pattern will for the appropriate paths. For more information
on URL mapping and other options you can specify in ``app.yaml``, see
`the app.yaml reference <http://code.google.com/appengine/docs/python/config/appconfig.html>`_.

Create the directory ``helloworld/stylesheets``. In this new directory, create
a new file named ``main.css`` with the following contents:

.. code-block:: css

   body {
     font-family: Verdana, Helvetica, sans-serif;
     background-color: #DDDDDD;
   }

Finally, edit ``helloworld/index.html`` and insert the following lines just
after the ``<html>`` line at the top:

.. code-block:: html

   <head>
     <link type="text/css" rel="stylesheet" href="/stylesheets/main.css" />
   </head>

Reload the page in your browser. The new version uses the stylesheet.


Next...
-------
The time has come to reveal your finished application to the world.

Continue to :ref:`tutorials.gettingstarted.uploading`.
