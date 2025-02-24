.. _tutorials.gettingstarted.helloworld:

Hello, World!
=============
Python App Engine applications communicate with the web server using the
`CGI <http://www.w3.org/CGI/>`_ standard. When the server receives a request
for your application, it runs the application with the request data in
environment variables and on the standard input stream (for POST data).
To respond, the application writes the response to the standard output stream,
including HTTP headers and content.

Let's begin by implementing a tiny application that displays a short message.


Creating a Simple Request Handler
---------------------------------
Create a directory named ``helloworld``. All files for this application reside
in this directory.

Inside the ``helloworld`` directory, create a file named ``helloworld.py``,
and give it the following contents::

    print 'Content-Type: text/plain'
    print ''
    print 'Hello, world!'

This Python script responds to a request with an HTTP header that describes
the content, a blank line, and the message ``Hello, world!``.


Creating the Configuration File
-------------------------------
An App Engine application has a configuration file called ``app.yaml``. Among
other things, this file describes which handler scripts should be used for
which URLs.

Inside the ``helloworld`` directory, create a file named ``app.yaml`` with the
following contents:

.. code-block:: yaml

   application: helloworld
   version: 1
   runtime: python27
   api_version: 1
   threadsafe: true

   handlers:
   - url: /.*
     script: helloworld.py

From top to bottom, this configuration file says the following about this
application:

- The application identifier is ``helloworld``. When you register your
  application with App Engine in the final step, you will select a unique
  identifier, and update this value. This value can be anything during
  development. For now, leave it set to helloworld.
- This is version number ``1`` of this application's code. If you adjust this
  before uploading new versions of your application software, App Engine will
  retain previous versions, and let you roll back to a previous version using
  the administrative console.
- This code runs in the ``python`` runtime environment, version "1".
  Additional runtime environments and languages may be supported in the future.
- Every request to a URL whose path matches the regular expression ``/.*``
  (all URLs) should be handled by the ``helloworld.py`` script.

The syntax of this file is `YAML <http://www.yaml.org/>`_. For a complete list
of configuration options, see
`the app.yaml reference <http://code.google.com/appengine/docs/python/config/appconfig.html>`_.


Testing the Application
-----------------------
With a handler script and configuration file mapping every URL to the handler,
the application is complete. You can now test it with the web server included
with the App Engine SDK.

If you're using the Google App Engine Launcher, you can set up the application
by selecting the **File** menu, **Add Existing Application...**, then selecting
the ``helloworld`` directory. Select the application in the app list, click the
Run button to start the application, then click the Browse button to view it.
Clicking Browse simply loads (or reloads)
`http://localhost:8080/ <http://localhost:8080/>`_ in your default web browser.

If you're not using Google App Engine Launcher, start the web server with the
following command, giving it the path to the ``helloworld`` directory:

.. code-block:: text

   google_appengine/dev_appserver.py helloworld/

The web server is now running, listening for requests on port 8080. You can
test the application by visiting the following URL in your web browser:

    http://localhost:8080/

For more information about running the development web server, including how
to change which port it uses, see `the Dev Web Server reference <http://code.google.com/appengine/docs/python/tools/devserver.html>`_,
or run the command with the option ``--help``.


Iterative Development
---------------------
You can leave the web server running while you develop your application.
The web server knows to watch for changes in your source files and reload
them if necessary.

Try it now: Leave the web server running, then edit ``helloworld.py`` to
change ``Hello, world!`` to something else. Reload
`http://localhost:8080/ <http://localhost:8080/>`_ or click Browse in Google
App Engine Launcher to see the change.

To shut down the web server, make sure the terminal window is active, then
press Control-C (or the appropriate "break" key for your console), or click
Stop in Google App Engine Launcher.

You can leave the web server running for the rest of this tutorial. If you
need to stop it, you can restart it again by running the command above.


Next...
-------
You now have a complete App Engine application! You could deploy this simple
greeting right now and share it with users worldwide. But before we deploy it,
let's consider using a web application framework to make it easier to add
features.

Continue to :ref:`tutorials.gettingstarted.usingwebapp2`.
