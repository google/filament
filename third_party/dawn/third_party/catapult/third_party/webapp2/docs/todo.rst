TODO: documentation
===================
Miscelaneous notes abotu things to be documented.

Unordered list of topics to be documented
-----------------------------------------
- routing

  - common regular expressions examples

- sessions

  - basic usage & configuration
  - using multiple sessions in the same request
  - using different backends
  - using flashes
  - updating session arguments (max_age etc)
  - purging db sessions

- i18n (increment existing tutorial)

  - basic usage & configuration
  - loading locale/timezone automatically for each request
  - formatting date/time/datetime
  - formatting currency
  - using i18n in templates

- jinja2 & mako

  - basic usage & configuration
  - setting global filters and variables (using config or factory)

- auth

  - basic usage & configuration
  - setting up 'own auth'
  - making user available automatically on each request
  - purging tokens

- config

  - configuration conventions ("namespaced" configuration for webapp2_extras
    modules)

- tricks

  - configuration in a separate file
  - routes in a separate file
  - reduce verbosity when defining routes (R = webapp2.Route)

Common errors
-------------
- "TypeError: 'unicode' object is not callable": one possible reason is that
  the ``RequestHandler`` returned a string. If the handler returns anything, it
  **must** be a :class:`webapp2.Response` object. Or it must not return
  anything and write to the response instead using ``self.response.write()``.

Secret keys
-----------
Add a note about how to generate strong session secret keys::

    $ openssl genrsa -out ${PWD}/private_rsa_key.pem 2048

Jinja2 factory
--------------
To create Jinja2 with custom filters and global variables::

    from webapp2_extras import jinja2

    def jinja2_factory(app):
        j = jinja2.Jinja2(app)
        j.environment.filters.update({
            'my_filter': my_filter,
        })
        j.environment.globals.update({
            'my_global': my_global,
        })
        return j

    # When you need jinja, get it passing the factory.
    j = jinja2.get_jinja2(factory=jinja2_factory)

Debugging Jinja2
----------------
http://stackoverflow.com/questions/3086091/debug-jinja2-in-google-app-engine/3694434#3694434

Configuration notes
-------------------
Notice that configuration is set primarily in the application. See:

    http://webapp-improved.appspot.com/guide/app.html#config

By convention, modules that are configurable in webapp2 use the module
name as key, to avoid name clashes. Their configuration is then set in
a nested dict. So, e.g., i18n, jinja2 and sessions are configured like this::

    config = {}
    config['webapp2_extras.i18n'] = {
        'default_locale': ...,
    }
    config['webapp2_extras.jinja2'] = {
        'template_path': ...,
    }
    config['webapp2_extras.sessions'] = {
        'secret_key': ...,
    }
    app = webapp2.WSGIApplication(..., config=config)

You only need to set the configuration keys that differ from the default
ones. For convenience, configurable modules have a 'default_config'
variable just for the purpose of documenting the default values, e.g.:

    http://webapp-improved.appspot.com/api/extras.i18n.html#webapp2_extras.i18n.default_config

Cookies, quoting & unicode
--------------------------
http://groups.google.com/group/webapp2/msg/985092351378c43e
http://stackoverflow.com/questions/6839922/unicodedecodeerror-is-raised-when-getting-a-cookie-in-google-app-engine

Marketplace integration
-----------------------

.. code-block:: xml

   <?xml version="1.0" encoding="UTF-8" ?>
   <ApplicationManifest xmlns="http://schemas.google.com/ApplicationManifest/2009">
     <!-- Name and description pulled from message bundles -->
     <Name>Tipfy</Name>
     <Description>A simple application for testing the marketplace.</Description>

     <!-- Support info to show in the marketplace & control panel -->
     <Support>
       <!-- URL for application setup as an optional redirect during the install -->
       <Link rel="setup" href="https://app-id.appspot.com/a/${DOMAIN_NAME}/setup" />

       <!-- URL for application configuration, accessed from the app settings page in the control panel -->
       <Link rel="manage" href="https://app-id.appspot.com/a/${DOMAIN_NAME}/manage" />

       <!-- URL explaining how customers get support. -->
       <Link rel="support" href="https://app-id.appspot.com/a/${DOMAIN_NAME}/support" />

       <!-- URL that is displayed to admins during the deletion process, to specify policies such as data retention, how to claim accounts, etc. -->
       <Link rel="deletion-policy" href="https://app-id.appspot.com/a/${DOMAIN_NAME}/deletion-policy" />
     </Support>

     <!-- Show this link in Google's universal navigation for all users -->
     <Extension id="navLink" type="link">
       <Name>Tipfy</Name>
       <Url>https://app-id.appspot.com/a/${DOMAIN_NAME}/</Url>
       <!-- This app also uses the Calendar API -->
       <Scope ref="Users"/>
       <!--
       <Scope ref="Groups"/>
       <Scope ref="Nicknames"/>
       -->
     </Extension>

     <!-- Declare our OpenID realm so our app is white listed -->
     <Extension id="realm" type="openIdRealm">
       <Url>https://app-id.appspot.com</Url>
     </Extension>

     <!-- Special access to APIs -->
     <Scope id="Users">
       <Url>https://apps-apis.google.com/a/feeds/user/#readonly</Url>
       <Reason>Users can be selected to gain special permissions to access or modify content.</Reason>
     </Scope>
     <!--
       <Scope id="Groups">
       <Url>https://apps-apis.google.com/a/feeds/group/#readonly</Url>
       <Reason></Reason>
     </Scope>
     <Scope id="Nicknames">
       <Url>https://apps-apis.google.com/a/feeds/nickname/#readonly</Url>
       <Reason></Reason>
     </Scope>
     -->
   </ApplicationManifest>
