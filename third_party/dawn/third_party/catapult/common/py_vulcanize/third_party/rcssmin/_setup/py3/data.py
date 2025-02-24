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
 Data distribution
===================

This module provides tools to simplify data distribution.
"""
__author__ = "Andr\xe9 Malo"
__docformat__ = "restructuredtext en"

from distutils import filelist as _filelist
import os as _os
import posixpath as _posixpath
import sys as _sys

from _setup import commands as _commands


def splitpath(path):
    """ Split a path """
    drive, path = '', _os.path.normpath(path)
    try:
        splitunc = _os.path.splitunc
    except AttributeError:
        pass
    else:
        drive, path = splitunc(path)
    if not drive:
        drive, path = _os.path.splitdrive(path)
    elems = []
    try:
        sep = _os.path.sep
    except AttributeError:
        sep = _os.path.join('1', '2')[1:-1]
    while 1:
        prefix, path = _os.path.split(path)
        elems.append(path)
        if prefix in ('', sep):
            drive = _os.path.join(drive, prefix)
            break
        path = prefix
    elems.reverse()
    return drive, elems


def finalizer(installer):
    """ Finalize install_data """
    data_files = []
    for item in installer.data_files:
        if not isinstance(item, Data):
            data_files.append(item)
            continue
        data_files.extend(item.flatten(installer))
    installer.data_files = data_files


class Data(object):
    """ File list container """

    def __init__(self, files, target=None, preserve=0, strip=0,
                 prefix=None):
        """ Initialization """
        self._files = files
        self._target = target
        self._preserve = preserve
        self._strip = strip
        self._prefix = prefix
        self.fixup_commands()

    def fixup_commands(self):
        pass

    def from_templates(cls, *templates, **kwargs):
        """ Initialize from template """
        files = _filelist.FileList()
        for tpl in templates:
            for line in tpl.split(';'):
                files.process_template_line(line.strip())
        files.sort()
        files.remove_duplicates()
        result = []
        for filename in files.files:
            _, elems = splitpath(filename)
            if '.svn' in elems or '.git' in elems:
                continue
            result.append(filename)
        return cls(result, **kwargs)
    from_templates = classmethod(from_templates)

    def flatten(self, installer):
        """ Flatten the file list to (target, file) tuples """
        # pylint: disable = W0613
        if self._prefix:
            _, prefix = splitpath(self._prefix)
            telems = prefix
        else:
            telems = []

        tmap = {}
        for fname in self._files:
            (_, name), target = splitpath(fname), telems
            if self._preserve:
                if self._strip:
                    name = name[max(0, min(self._strip, len(name) - 1)):]
                if len(name) > 1:
                    target = telems + name[:-1]
            tmap.setdefault(_posixpath.join(*target), []).append(fname)
        return list(tmap.items())


class Documentation(Data):
    """ Documentation container """

    def fixup_commands(self):
        _commands.add_option('install_data', 'without-docs',
            help_text='Do not install documentation files',
            inherit='install',
        )
        _commands.add_finalizer('install_data', 'documentation', finalizer)

    def flatten(self, installer):
        """ Check if docs should be installed at all """
        if installer.without_docs:
            return []
        return Data.flatten(self, installer)


class Manpages(Documentation):
    """ Manpages container """

    def dispatch(cls, files):
        """ Automatically dispatch manpages to their target directories """
        mpmap = {}
        for manpage in files:
            normalized = _os.path.normpath(manpage)
            _, ext = _os.path.splitext(normalized)
            if ext.startswith(_os.path.extsep):
                ext = ext[len(_os.path.extsep):]
            mpmap.setdefault(ext, []).append(manpage)
        return [cls(manpages, prefix=_posixpath.join(
            'share', 'man', 'man%s' % section,
        )) for section, manpages in list(mpmap.items())]
    dispatch = classmethod(dispatch)

    def flatten(self, installer):
        """ Check if manpages are suitable """
        if _sys.platform == 'win32':
            return []
        return Documentation.flatten(self, installer)
