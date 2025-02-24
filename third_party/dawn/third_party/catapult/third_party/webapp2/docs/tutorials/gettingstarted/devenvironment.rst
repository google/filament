.. _tutorials.gettingstarted.devenvironment:

The Development Environment
===========================
You develop and upload Python applications for Google App Engine using the App
Engine Python software development kit (SDK).

The Python SDK includes a web server application that simulates the App Engine
environment, including a local version of the datastore, Google Accounts, and
the ability to fetch URLs and send email directly from your computer using the
App Engine APIs. The Python SDK runs on any computer with Python 2.5, and
versions are available for Windows, Mac OS X and Linux. The Python SDK is
not compatible with Python 3.

The Python SDK for Windows and Mac includes Google App Engine Launcher, an
application that runs on your computer and provides a graphical interface that
simplifies many common App Engine development tasks.

If necessary, download and install Python 2.5 for your platform from
`the Python web site <http://www.python.org/>`_. Mac OS X 10.5 Leopard users
already have Python 2.5 installed.

`Download the App Engine SDK <http://code.google.com/appengine/downloads.html>`_.
Follow the instructions on the download page to install the SDK on your
computer.

For this tutorial, you will use two commands from the SDK:

- `dev_appserver.py <http://code.google.com/appengine/docs/python/tools/devserver.html>`_, the development web server
- `appcfg.py <http://code.google.com/appengine/docs/python/tools/uploadinganapp.html>`_, for uploading your app to App Engine

Windows and Mac users can run Google App Engine Launcher and simply click the
Run and Deploy buttons instead of using these commands.

For Windows users: The Windows installer puts these commands in the command
path. After installation, you can run these commands from a command prompt.

For Mac users: You can put these commands in the command path by selecting
"Make Symlinks..." from the "GoogleAppEngineLauncher" menu.

If you are using the Zip archive version of the SDK, you will find these
commands in the ``google_appengine`` directory.


Next...
-------
The local development environment lets you develop and test complete App Engine
applications before showing them to the world. Let's write some code.

Continue to :ref:`tutorials.gettingstarted.helloworld`.
