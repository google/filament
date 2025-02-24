# -*- coding: utf-8 -*-
"""
    webapp2_extras.mako
    ===================

    Mako template support for webapp2.

    Learn more about Mako: http://www.makotemplates.org/

    :copyright: 2011 by tipfy.org.
    :license: Apache Sotware License, see LICENSE for details.
"""
from __future__ import absolute_import

from mako import lookup

import webapp2

#: Default configuration values for this module. Keys are:
#:
#: template_path
#:     Directory for templates. Default is `templates`.
default_config = {
    'template_path': 'templates',
}


class Mako(object):
    """Wrapper for configurable and cached Mako environment.

    To used it, set it as a cached property in a base `RequestHandler`::

        import webapp2

        from webapp2_extras import mako

        class BaseHandler(webapp2.RequestHandler):

            @webapp2.cached_property
            def mako(self):
                # Returns a Mako renderer cached in the app registry.
                return mako.get_mako(app=self.app)

            def render_response(self, _template, **context):
                # Renders a template and writes the result to the response.
                rv = self.mako.render_template(_template, **context)
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
        self.config = config = app.config.load_config(self.config_key,
            default_values=default_config, user_values=config,
            required_keys=None)

        directories = config.get('template_path')
        if isinstance(directories, basestring):
            directories = [directories]

        self.environment = lookup.TemplateLookup(directories=directories,
                                                 output_encoding='utf-8',
                                                 encoding_errors='replace')

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
        template = self.environment.get_template(_filename)
        return template.render_unicode(**context)


# Factories -------------------------------------------------------------------


#: Key used to store :class:`Mako` in the app registry.
_registry_key = 'webapp2_extras.mako.Mako'


def get_mako(factory=Mako, key=_registry_key, app=None):
    """Returns an instance of :class:`Mako` from the app registry.

    It'll try to get it from the current app registry, and if it is not
    registered it'll be instantiated and registered. A second call to this
    function will return the same instance.

    :param factory:
        The callable used to build and register the instance if it is not yet
        registered. The default is the class :class:`Mako` itself.
    :param key:
        The key used to store the instance in the registry. A default is used
        if it is not set.
    :param app:
        A :class:`webapp2.WSGIApplication` instance used to store the instance.
        The active app is used if it is not set.
    """
    app = app or webapp2.get_app()
    mako = app.registry.get(key)
    if not mako:
        mako = app.registry[key] = factory(app)

    return mako


def set_mako(mako, key=_registry_key, app=None):
    """Sets an instance of :class:`Mako` in the app registry.

    :param store:
        An instance of :class:`Mako`.
    :param key:
        The key used to retrieve the instance from the registry. A default
        is used if it is not set.
    :param request:
        A :class:`webapp2.WSGIApplication` instance used to retrieve the
        instance. The active app is used if it is not set.
    """
    app = app or webapp2.get_app()
    app.registry[key] = mako
