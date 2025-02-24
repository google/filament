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
 C extension tools
===================

C extension tools.
"""
__author__ = u"Andr\xe9 Malo"
__docformat__ = "restructuredtext en"
__test__ = False

from distutils import core as _core
from distutils import errors as _distutils_errors
import os as _os
import posixpath as _posixpath
import shutil as _shutil
import tempfile as _tempfile

from _setup import commands as _commands
from _setup.util import log


def _install_finalizer(installer):
    if installer.without_c_extensions:
        installer.distribution.ext_modules = []

def _build_finalizer(builder):
    if builder.without_c_extensions:
        builder.extensions = []


class Extension(_core.Extension):
    """
    Extension with prerequisite check interface

    If your check is cacheable (during the setup run), override
    `cached_check_prerequisites`, `check_prerequisites` otherwise.

    :IVariables:
      `cached_check` : ``bool``
        The cached check result
    """
    cached_check = None

    def __init__(self, *args, **kwargs):
        """ Initialization """
        if kwargs.has_key('depends'):
            self.depends = kwargs['depends'] or []
        else:
            self.depends = []
        _core.Extension.__init__(self, *args, **kwargs)

        # add include path
        included = _posixpath.join('_setup', 'include')
        if included not in self.include_dirs:
            self.include_dirs.append(included)

        # add cext.h to the dependencies
        cext_h = _posixpath.join(included, 'cext.h')
        if cext_h not in self.depends:
            self.depends.append(cext_h)

        _commands.add_option('install_lib', 'without-c-extensions',
            help_text='Don\'t install C extensions',
            inherit='install',
        )
        _commands.add_finalizer('install_lib', 'c-extensions',
            _install_finalizer
        )
        _commands.add_option('build_ext', 'without-c-extensions',
            help_text='Don\'t build C extensions',
            inherit=('build', 'install_lib'),
        )
        _commands.add_finalizer('build_ext', 'c-extensions', _build_finalizer)

    def check_prerequisites(self, build):
        """
        Check prerequisites

        The check should cover all dependencies needed for the extension to
        be built and run. The method can do the following:

        - return a false value: the extension will be built
        - return a true value: the extension will be skipped. This is useful
          for optional extensions
        - raise an exception. This is useful for mandatory extensions

        If the check result is cacheable (during the setup run), override
        `cached_check_prerequisites` instead.

        :Parameters:
          `build` : `BuildExt`
            The extension builder

        :Return: Skip the extension?
        :Rtype: ``bool``
        """
        if self.cached_check is None:
            log.debug("PREREQ check for %s" % self.name)
            self.cached_check = self.cached_check_prerequisites(build)
        else:
            log.debug("PREREQ check for %s (cached)" % self.name)
        return self.cached_check

    def cached_check_prerequisites(self, build):
        """
        Check prerequisites

        The check should cover all dependencies needed for the extension to
        be built and run. The method can do the following:

        - return a false value: the extension will be built
        - return a true value: the extension will be skipped. This is useful
          for optional extensions
        - raise an exception. This is useful for mandatory extensions

        If the check result is *not* cacheable (during the setup run),
        override `check_prerequisites` instead.

        :Parameters:
          `build` : `BuildExt`
            The extension builder

        :Return: Skip the extension?
        :Rtype: ``bool``
        """
        # pylint: disable = W0613
        log.debug("Nothing to check for %s!" % self.name)
        return False


class ConfTest(object):
    """
    Single conftest abstraction

    :IVariables:
      `_tempdir` : ``str``
        The tempdir created for this test

      `src` : ``str``
        Name of the source file

      `target` : ``str``
        Target filename

      `compiler` : ``CCompiler``
        compiler instance

      `obj` : ``list``
        List of object filenames (``[str, ...]``)
    """
    _tempdir = None

    def __init__(self, build, source):
        """
        Initialization

        :Parameters:
          `build` : ``distuils.command.build_ext.build_ext``
            builder instance

          `source` : ``str``
            Source of the file to compile
        """
        self._tempdir = tempdir = _tempfile.mkdtemp()
        src = _os.path.join(tempdir, 'conftest.c')
        fp = open(src, 'w')
        try:
            fp.write(source)
        finally:
            fp.close()
        self.src = src
        self.compiler = compiler = build.compiler
        self.target = _os.path.join(tempdir, 'conftest')
        self.obj = compiler.object_filenames([src], output_dir=tempdir)

    def __del__(self):
        """ Destruction """
        self.destroy()

    def destroy(self):
        """ Destroy the conftest leftovers on disk """
        tempdir, self._tempdir = self._tempdir, None
        if tempdir is not None:
            _shutil.rmtree(tempdir)

    def compile(self, **kwargs):
        """
        Compile the conftest

        :Parameters:
          `kwargs` : ``dict``
            Optional keyword parameters for the compiler call

        :Return: Was the compilation successful?
        :Rtype: ``bool``
        """
        kwargs['output_dir'] = self._tempdir
        try:
            self.compiler.compile([self.src], **kwargs)
        except _distutils_errors.CompileError:
            return False
        return True

    def link(self, **kwargs):
        r"""
        Link the conftest

        Before you can link the conftest objects they need to be `compile`\d.

        :Parameters:
          `kwargs` : ``dict``
            Optional keyword parameters for the linker call

        :Return: Was the linking successful?
        :Rtype: ``bool``
        """
        try:
            self.compiler.link_executable(self.obj, self.target, **kwargs)
        except _distutils_errors.LinkError:
            return False
        return True

    def pipe(self, mode="r"):
        r"""
        Execute the conftest binary and connect to it using a pipe

        Before you can pipe to or from the conftest binary it needs to
        be `link`\ed.

        :Parameters:
          `mode` : ``str``
            Pipe mode - r/w

        :Return: The open pipe
        :Rtype: ``file``
        """
        return _os.popen(self.compiler.executable_filename(self.target), mode)
