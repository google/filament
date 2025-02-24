# -*- coding: utf-8 -*-
"""
    webapp2_extras.config
    =====================

    Configuration object for webapp2.

    This module is deprecated. See :class:`webapp2.WSGIApplication.config`.

    :copyright: 2011 by tipfy.org.
    :license: Apache Sotware License, see LICENSE for details.
"""
from __future__ import absolute_import

import warnings

import webapp2

warnings.warn(DeprecationWarning(
    'webapp2_extras.config is deprecated. '
    'The WSGIApplication uses webapp2.Config instead.'),
    stacklevel=1)

#: Value used for missing default values.
DEFAULT_VALUE = object()

#: Value used for required values.
REQUIRED_VALUE = object()


class Config(dict):
    """A simple configuration dictionary keyed by module name. This is a
    dictionary of dictionaries. It requires all values to be dictionaries
    and applies updates and default values to the inner dictionaries instead
    of the first level one.

    The configuration object can be set as a ``config`` attribute of
    :class:`WSGIApplication`::

        import webapp2
        from webapp2_extras import config as webapp2_config

        my_config = {}

        my_config['my.module'] = {
            'foo': 'bar',
        }

        app = webapp2.WSGIApplication(routes=[
            webapp2.Route('/', name='home', handler=MyHandler)
        ])
        app.config = webapp2_config.Config(my_config)

    Then to read configuration values, get them from the app::

        class MyHandler(RequestHandler):
            def get(self):
                foo = self.app.config['my.module']['foo']

                # ...
    """
    #: Loaded module configurations.
    loaded = None

    def __init__(self, values=None, defaults=None):
        """Initializes the configuration object.

        :param values:
            A dictionary of configuration dictionaries for modules.
        :param defaults:
            A dictionary of configuration dictionaries for initial default
            values. These modules are marked as loaded.
        """
        self.loaded = []
        if values is not None:
            assert isinstance(values, dict)
            for module, config in values.iteritems():
                self.update(module, config)

        if defaults is not None:
            assert isinstance(defaults, dict)
            for module, config in defaults.iteritems():
                self.setdefault(module, config)
                self.loaded.append(module)

    def __getitem__(self, module):
        """Returns the configuration for a module. If it is not already
        set, loads a ``default_config`` variable from the given module and
        updates the configuration with those default values

        Every module that allows some kind of configuration sets a
        ``default_config`` global variable that is loaded by this function,
        cached and used in case the requested configuration was not defined
        by the user.

        :param module:
            The module name.
        :returns:
            A configuration value.
        """
        if module not in self.loaded:
            # Load default configuration and update config.
            values = webapp2.import_string(module + '.default_config',
                                           silent=True)
            if values:
                self.setdefault(module, values)

            self.loaded.append(module)

        try:
            return dict.__getitem__(self, module)
        except KeyError:
            raise KeyError('Module %r is not configured.' % module)

    def __setitem__(self, module, values):
        """Sets a configuration for a module, requiring it to be a dictionary.

        :param module:
            A module name for the configuration, e.g.: `webapp2.ext.i18n`.
        :param values:
            A dictionary of configurations for the module.
        """
        assert isinstance(values, dict), 'Module configuration must be a dict.'
        dict.__setitem__(self, module, SubConfig(module, values))

    def get(self, module, default=DEFAULT_VALUE):
        """Returns a configuration for a module. If default is not provided,
        returns an empty dict if the module is not configured.

        :param module:
            The module name.
        :params default:
            Default value to return if the module is not configured. If not
            set, returns an empty dict.
        :returns:
            A module configuration.
        """
        if default is DEFAULT_VALUE:
            default = {}

        return dict.get(self, module, default)

    def setdefault(self, module, values):
        """Sets a default configuration dictionary for a module.

        :param module:
            The module to set default configuration, e.g.: `webapp2.ext.i18n`.
        :param values:
            A dictionary of configurations for the module.
        :returns:
            The module configuration dictionary.
        """
        assert isinstance(values, dict), 'Module configuration must be a dict.'
        if module not in self:
            dict.__setitem__(self, module, SubConfig(module))

        module_dict = dict.__getitem__(self, module)

        for key, value in values.iteritems():
            module_dict.setdefault(key, value)

        return module_dict

    def update(self, module, values):
        """Updates the configuration dictionary for a module.

        :param module:
            The module to update the configuration, e.g.: `webapp2.ext.i18n`.
        :param values:
            A dictionary of configurations for the module.
        """
        assert isinstance(values, dict), 'Module configuration must be a dict.'
        if module not in self:
            dict.__setitem__(self, module, SubConfig(module))

        dict.__getitem__(self, module).update(values)

    def get_config(self, module, key=None, default=REQUIRED_VALUE):
        """Returns a configuration value for a module and optionally a key.
        Will raise a KeyError if they the module is not configured or the key
        doesn't exist and a default is not provided.

        :param module:
            The module name.
        :params key:
            The configuration key.
        :param default:
            Default value to return if the key doesn't exist.
        :returns:
            A module configuration.
        """
        module_dict = self.__getitem__(module)

        if key is None:
            return module_dict

        return module_dict.get(key, default)


class SubConfig(dict):
    def __init__(self, module, values=None):
        dict.__init__(self, values or ())
        self.module = module

    def __getitem__(self, key):
        try:
            value = dict.__getitem__(self, key)
        except KeyError:
            raise KeyError('Module %r does not have the config key %r' %
                (self.module, key))

        if value is REQUIRED_VALUE:
            raise KeyError('Module %r requires the config key %r to be '
                'set.' % (self.module, key))

        return value

    def get(self, key, default=None):
        if key not in self:
            value = default
        else:
            value = dict.__getitem__(self, key)

        if value is REQUIRED_VALUE:
            raise KeyError('Module %r requires the config key %r to be '
                'set.' % (self.module, key))

        return value
