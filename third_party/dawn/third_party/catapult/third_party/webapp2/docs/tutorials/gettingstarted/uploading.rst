.. _tutorials.gettingstarted.uploading:

Uploading Your Application
==========================
You create and manage applications in App Engine using the Administration
Console. Once you have registered an application ID for your application, you
upload it to your website using ``appcfg.py``, a command-line tool provided
in the SDK. Or, if you're using Google App Engine Launcher, you can upload
your application by clicking the Deploy button.

.. note::
   Once you register an application ID, you can delete it, but you can't
   re-register that same application ID after it has been deleted. You can
   skip these next steps if you don't want to register an ID at this time.


Registering the Application
---------------------------
You create and manage App Engine web applications from the App Engine
Administration Console, at the following URL:

- `https://appengine.google.com/ <https://appengine.google.com/>`_

Google App Engine Launcher users can reach this URL by clicking the Dashboard
button.

Sign in to App Engine using your Google account. If you do not have a Google
account, you can `create a Google account <https://www.google.com/accounts/>`_
with an email address and password.

To create a new application, click the "Create an Application" button. Follow
the instructions to register an application ID, a name unique to this
application. If you elect to use the free appspot.com domain name, the full
URL for the application will be ``http://application-id.appspot.com/``. You can
also purchase a top-level domain name for your app, or use one that you have
already registered.

Edit the ``app.yaml`` file, then change the value of the ``application:``
setting from ``helloworld`` to your registered application ID.


Uploading the Application
-------------------------
To upload your finished application to Google App Engine, run the following
command::

.. code-block:: text

   appcfg.py update helloworld/

Or click Deploy in Google App Engine Launcher.

Enter your Google username and password at the prompts.

You can now see your application running on App Engine. If you set up a free
appspot.com domain name, the URL for your website begins with your application
ID::

.. code-block:: text

   http://application-id.appspot.com


Congratulations!
----------------
You have completed this tutorial. For more information on the subjects
covered here, see the rest of
`the App Engine documentation <http://code.google.com/appengine/docs/>`_ and
the :ref:`guide.index`.
