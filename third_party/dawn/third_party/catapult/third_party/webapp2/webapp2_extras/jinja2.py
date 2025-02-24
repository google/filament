# -*- coding: utf-8 -*-
"""
    webapp2_extras.jinja2
    =====================

    Jinja2 template support for webapp2.

    Learn more about Jinja2: http://jinja.pocoo.org/

    :copyright: 2011 by tipfy.org.
    :license: Apache Sotware License, see LICENSE for details.
"""
from __future__ import absolute_import

import jinja2

import webapp2

#: Default configuration values for this module. Keys are:
#:
#: template_path
#:     Directory for templates. Default is `templates`.
#:
#: compiled_path
#:     Target for compiled templates. If set, uses the loader for compiled
#:     templates in production. If it ends with a '.zip' it will be treated
#:     as a zip file. Default is None.
#:
#: force_compiled
#:     Forces the use of compiled templates even in the development server.
#:
#: environment_args
#:     Keyword arguments used to instantiate the Jinja2 environment. By
#:     default autoescaping is enabled and two extensions are set:
#:     ``jinja2.ext.autoescape`` and ``jinja2.ext.with_``. For production it
#:     may be a good idea to set 'auto_reload' to False -- we don't need to
#:     check if templates changed after deployed.
#:
#: globals
#:     Extra global variables for the Jinja2 environment.
#:
#: filters
#:     Extra filters for the Jinja2 environment.
default_config = {
    'template_path': 'templates',
    'compiled_path': None,
    'force_compiled': False,
    'environment_args': {
        'autoescape': True,
        'extensions': [
            'jinja2.ext.autoescape',
            'jinja2.ext.with_',
        ],
    },
    'globals': None,
    'filters': None,
}


class Jinja2(object):
    """Wrapper for configurable and cached Jinja2 environment.

    To used it, set it as a cached property in a base `RequestHandler`::

        import webapp2

        from webapp2_extras import jinja2

        class BaseHandler(webapp2.RequestHandler):

            @webapp2.cached_property
            def jinja2(self):
                # Returns a Jinja2 renderer cached in the app registry.
                return jinja2.get_jinja2(app=self.app)

            def render_response(self, _template, **context):
                # Renders a template and writes the result to the response.
                rv = self.jinja2.render_template(_template, **context)
                self.response.write(rv)

    Then extended handlers can render templates directly::

        class MyHandler(BaseHandler):
            def get(self):
                context = {'message': 'Hello, world!'}
                self.render_response('my_template.html', **context)
    """

    #: Configuration key.
    config_key = __name__

    #: Loaded configuration.
    config = None

    def __init__(self, app, config=None):
        """Initializes the Jinja2 object.

        :param app:
            A :class:`webapp2.WSGIApplication` instance.
        :param config:
            A dictionary of configuration values to be overridden. See
            the available keys in :data:`default_config`.
        """
        self.config = config = app.config.load_config(self.config_key,
            default_values=default_config, user_values=config,
            required_keys=None)
        kwargs = config['environment_args'].copy()
        enable_i18n = 'jinja2.ext.i18n' in kwargs.get('extensions', [])

        if 'loader' not in kwargs:
            template_path = config['template_path']
            compiled_path = config['compiled_path']
            use_compiled = not app.debug or config['force_compiled']

            if compiled_path and use_compiled:
                # Use precompiled templates loaded from a module or zip.
                kwargs['loader'] = jinja2.ModuleLoader(compiled_path)
            else:
                # Parse templates for every new environment instances.
                kwargs['loader'] = jinja2.FileSystemLoader(template_path)

        # Initialize the environment.
        env = jinja2.Environment(**kwargs)

        if config['globals']:
            env.globals.update(config['globals'])

        if config['filters']:
            env.filters.update(config['filters'])

        if enable_i18n:
            # Install i18n.
            from webapp2_extras import i18n
            env.install_gettext_callables(
                lambda x: i18n.gettext(x),
                lambda s, p, n: i18n.ngettext(s, p, n),
                newstyle=True)
            env.filters.update({
                'format_date':      i18n.format_date,
                'format_time':      i18n.format_time,
                'format_datetime':  i18n.format_datetime,
                'format_timedelta': i18n.format_timedelta,
            })

        self.environment = env

    def render_template(self, _filename, **context):
        """Renders a template and returns a response object.

        :param _filename:
            The template filename, related to the templates directory.
        :param context:
            Keyword arguments used as variables in the rendered template.
            These will override values set in the request context.
        :returns:
            A rendered template.
        """
        return self.environment.get_template(_filename).render(**context)

    def get_template_attribute(self, filename, attribute):
        """Loads a macro (or variable) a template exports.  This can be used to
        invoke a macro from within Python code.  If you for example have a
        template named `_foo.html` with the following contents:

        .. sourcecode:: html+jinja

           {% macro hello(name) %}Hello {{ name }}!{% endmacro %}

        You can access this from Python code like this::

            hello = get_template_attribute('_foo.html', 'hello')
            return hello('World')

        This function comes from `Flask`.

        :param filename:
            The template filename.
        :param attribute:
            The name of the variable of macro to acccess.
        """
        template = self.environment.get_template(filename)
        return getattr(template.module, attribute)


# Factories -------------------------------------------------------------------


#: Key used to store :class:`Jinja2` in the app registry.
_registry_key = 'webapp2_extras.jinja2.Jinja2'


def get_jinja2(factory=Jinja2, key=_registry_key, app=None):
    """Returns an instance of :class:`Jinja2` from the app registry.

    It'll try to get it from the current app registry, and if it is not
    registered it'll be instantiated and registered. A second call to this
    function will return the same instance.

    :param factory:
        The callable used to build and register the instance if it is not yet
        registered. The default is the class :class:`Jinja2` itself.
    :param key:
        The key used to store the instance in the registry. A default is used
        if it is not set.
    :param app:
        A :class:`webapp2.WSGIApplication` instance used to store the instance.
        The active app is used if it is not set.
    """
    app = app or webapp2.get_app()
    jinja2 = app.registry.get(key)
    if not jinja2:
        jinja2 = app.registry[key] = factory(app)

    return jinja2


def set_jinja2(jinja2, key=_registry_key, app=None):
    """Sets an instance of :class:`Jinja2` in the app registry.

    :param store:
        An instance of :class:`Jinja2`.
    :param key:
        The key used to retrieve the instance from the registry. A default
        is used if it is not set.
    :param request:
        A :class:`webapp2.WSGIApplication` instance used to retrieve the
        instance. The active app is used if it is not set.
    """
    app = app or webapp2.get_app()
    app.registry[key] = jinja2
