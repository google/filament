# -*- coding: ascii -*-
#
# Copyright 2007, 2008, 2009, 2010, 2011
# Andr\xe9 Malo or his licensors, as applicable
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
"""
===================
 Command extenders
===================

Command extenders.
"""
__author__ = "Andr\xe9 Malo"
__docformat__ = "restructuredtext en"
__test__ = False

from distutils import fancy_getopt as _fancy_getopt
from distutils import log
from distutils.command import build as _build
from distutils.command import build_ext as _build_ext
from distutils.command import install as _install
from distutils.command import install_data as _install_data
from distutils.command import install_lib as _install_lib
import os as _os

_option_defaults = {}
_option_inherits = {}
_option_finalizers = {}
_command_mapping = {
    'install': 'Install',
    'install_data': 'InstallData',
    'install_lib': 'InstallLib',
    'build': 'Build',
    'build_ext': 'BuildExt',
}


def add_option(command, long_name, help_text, short_name=None, default=None,
               inherit=None):
    """ Add an option """
    try:
        command_class = globals()[_command_mapping[command]]
    except KeyError:
        raise ValueError("Unknown command %r" % (command,))
    for opt in command_class.user_options:
        if opt[0] == long_name:
            break
    else:
        opt = (long_name, short_name, help_text)
        command_class.user_options.append(opt)
        if not long_name.endswith('='):
            command_class.boolean_options.append(long_name)
            attr_name = _fancy_getopt.translate_longopt(long_name)
        else:
            attr_name = _fancy_getopt.translate_longopt(long_name[:-1])
        if command not in _option_defaults:
            _option_defaults[command] = []
        if inherit is not None:
            if isinstance(inherit, str):
                inherit = [inherit]
            for i_inherit in inherit:
                add_option(
                    i_inherit, long_name, help_text, short_name, default
                )
            default = None
            if command not in _option_inherits:
                _option_inherits[command] = []
            for i_inherit in inherit:
                for i_command, opt_name in _option_inherits[command]:
                    if i_command == i_inherit and opt_name == attr_name:
                        break
                else:
                    _option_inherits[command].append((i_inherit, attr_name))
        _option_defaults[command].append((attr_name, default))


def add_finalizer(command, key, func):
    """ Add finalizer """
    if command not in _option_finalizers:
        _option_finalizers[command] = {}
    if key not in _option_finalizers[command]:
        _option_finalizers[command][key] = func


class Install(_install.install):
    """ Extended installer to reflect the additional data options """
    user_options = _install.install.user_options + [
        ('single-version-externally-managed', None,
            "Compat option. Does not a thing."),
    ]
    boolean_options = _install.install.boolean_options + [
        'single-version-externally-managed'
    ]

    def initialize_options(self):
        """ Prepare for new options """
        _install.install.initialize_options(self)
        self.single_version_externally_managed = None
        if 'install' in _option_defaults:
            for opt_name, default in _option_defaults['install']:
                setattr(self, opt_name, default)

    def finalize_options(self):
        """ Finalize options """
        _install.install.finalize_options(self)
        if 'install' in _option_inherits:
            for parent, opt_name in _option_inherits['install']:
                self.set_undefined_options(parent, (opt_name, opt_name))
        if 'install' in _option_finalizers:
            for func in list(_option_finalizers['install'].values()):
                func(self)


class InstallData(_install_data.install_data):
    """ Extended data installer """
    user_options = _install_data.install_data.user_options + []
    boolean_options = _install_data.install_data.boolean_options + []

    def initialize_options(self):
        """ Prepare for new options """
        _install_data.install_data.initialize_options(self)
        if 'install_data' in _option_defaults:
            for opt_name, default in _option_defaults['install_data']:
                setattr(self, opt_name, default)

    def finalize_options(self):
        """ Finalize options """
        _install_data.install_data.finalize_options(self)
        if 'install_data' in _option_inherits:
            for parent, opt_name in _option_inherits['install_data']:
                self.set_undefined_options(parent, (opt_name, opt_name))
        if 'install_data' in _option_finalizers:
            for func in list(_option_finalizers['install_data'].values()):
                func(self)


class InstallLib(_install_lib.install_lib):
    """ Extended lib installer """
    user_options = _install_lib.install_lib.user_options + []
    boolean_options = _install_lib.install_lib.boolean_options + []

    def initialize_options(self):
        """ Prepare for new options """
        _install_lib.install_lib.initialize_options(self)
        if 'install_lib' in _option_defaults:
            for opt_name, default in _option_defaults['install_lib']:
                setattr(self, opt_name, default)

    def finalize_options(self):
        """ Finalize options """
        _install_lib.install_lib.finalize_options(self)
        if 'install_lib' in _option_inherits:
            for parent, opt_name in _option_inherits['install_lib']:
                self.set_undefined_options(parent, (opt_name, opt_name))
        if 'install_lib' in _option_finalizers:
            for func in list(_option_finalizers['install_lib'].values()):
                func(self)


class BuildExt(_build_ext.build_ext):
    """
    Extended extension builder class

    This class allows extensions to provide a ``check_prerequisites`` method
    which is called before actually building it. The method takes the
    `BuildExt` instance and returns whether the extension should be skipped or
    not.
    """

    def initialize_options(self):
        """ Prepare for new options """
        _build_ext.build_ext.initialize_options(self)
        if 'build_ext' in _option_defaults:
            for opt_name, default in _option_defaults['build_ext']:
                setattr(self, opt_name, default)

    def finalize_options(self):
        """ Finalize options """
        _build_ext.build_ext.finalize_options(self)
        if 'build_ext' in _option_inherits:
            for parent, opt_name in _option_inherits['build_ext']:
                self.set_undefined_options(parent, (opt_name, opt_name))
        if 'build_ext' in _option_finalizers:
            for func in list(_option_finalizers['build_ext'].values()):
                func(self)

    def build_extension(self, ext):
        """
        Build C extension - with extended functionality

        The following features are added here:

        - ``ext.check_prerequisites`` is called before the extension is being
          built. See `Extension` for details. If the method does not exist,
          simply no check will be run.
        - The macros ``EXT_PACKAGE`` and ``EXT_MODULE`` will be filled (or
          unset) depending on the extensions name, but only if they are not
          already defined.

        :Parameters:
          `ext` : `Extension`
            The extension to build. If it's a pure
            ``distutils.core.Extension``, simply no prequisites check is
            applied.

        :Return: whatever ``distutils.command.build_ext.build_ext`` returns
        :Rtype: any
        """
        # handle name macros
        macros = dict(ext.define_macros or ())
        tup = ext.name.split('.')
        if len(tup) == 1:
            pkg, mod = None, tup[0]
        else:
            pkg, mod = '.'.join(tup[:-1]), tup[-1]
        if pkg is not None and 'EXT_PACKAGE' not in macros:
            ext.define_macros.append(('EXT_PACKAGE', pkg))
        if 'EXT_MODULE' not in macros:
            ext.define_macros.append(('EXT_MODULE', mod))
        if pkg is None:
            macros = dict(ext.undef_macros or ())
            if 'EXT_PACKAGE' not in macros:
                ext.undef_macros.append('EXT_PACKAGE')

        # handle prereq checks
        try:
            checker = ext.check_prerequisites
        except AttributeError:
            pass
        else:
            if checker(self):
                log.info("Skipping %s extension" % ext.name)
                return

        return _build_ext.build_ext.build_extension(self, ext)


class Build(_build.build):

    def initialize_options(self):
        """ Prepare for new options """
        _build.build.initialize_options(self)
        if 'build' in _option_defaults:
            for opt_name, default in _option_defaults['build']:
                setattr(self, opt_name, default)

    def finalize_options(self):
        """ Finalize options """
        _build.build.finalize_options(self)
        if 'build' in _option_inherits:
            for parent, opt_name in _option_inherits['build']:
                self.set_undefined_options(parent, (opt_name, opt_name))
        if 'build' in _option_finalizers:
            for func in list(_option_finalizers['build'].values()):
                func(self)
