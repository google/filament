# -*- coding: ascii -*-
#
# Copyright 2007 - 2013
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
 Main setup runner
===================

This module provides a wrapper around the distutils core setup.
"""
__author__ = "Andr\xe9 Malo"
__docformat__ = "restructuredtext en"

import configparser as _config_parser
from distutils import core as _core
import os as _os
import posixpath as _posixpath
import sys as _sys

from _setup import commands as _commands
from _setup import data as _data
from _setup import ext as _ext
from _setup import util as _util
from _setup import shell as _shell


def check_python_version(impl, version_min, version_max):
    """ Check python version """
    if impl == 'python':
        version_info = _sys.version_info
    elif impl == 'pypy':
        version_info = getattr(_sys, 'pypy_version_info', None)
        if not version_info:
            return
    elif impl == 'jython':
        if not 'java' in _sys.platform.lower():
            return
        version_info = _sys.version_info
    else:
        raise AssertionError("impl not in ('python', 'pypy', 'jython')")

    pyversion = list(map(int, version_info[:3]))
    if version_min:
        min_required = list(
            map(int, '.'.join((version_min, '0.0.0')).split('.')[:3])
        )
        if pyversion < min_required:
            raise EnvironmentError("Need at least %s %s (vs. %s)" % (
                impl, version_min, '.'.join(map(str, pyversion))
            ))
    if version_max:
        max_required = list(map(int, version_max.split('.')))
        max_required[-1] += 1
        if pyversion >= max_required:
            raise EnvironmentError("Need at max %s %s (vs. %s)" % (
                impl,
                version_max,
                '.'.join(map(str, pyversion))
            ))


def find_description(docs):
    """
    Determine the package description from DESCRIPTION

    :Parameters:
      `docs` : ``dict``
        Docs config section

    :Return: Tuple of summary, description and license
             (``('summary', 'description', 'license')``)
             (all may be ``None``)
    :Rtype: ``tuple``
    """
    summary = None
    filename = docs.get('meta.summary', 'SUMMARY').strip()
    if filename and _os.path.isfile(filename):
        fp = open(filename, encoding='utf-8')
        try:
            try:
                summary = fp.read().strip().splitlines()[0].rstrip()
            except IndexError:
                summary = ''
        finally:
            fp.close()

    description = None
    filename = docs.get('meta.description', 'DESCRIPTION').strip()
    if filename and _os.path.isfile(filename):
        fp = open(filename, encoding='utf-8')
        try:
            description = fp.read().rstrip()
        finally:
            fp.close()

        if summary is None and description:
            from docutils import core
            summary = core.publish_parts(
                source=description,
                source_path=filename,
                writer_name='html',
            )['title'].encode('utf-8')

    return summary, description


def find_classifiers(docs):
    """
    Determine classifiers from CLASSIFIERS

    :return: List of classifiers (``['classifier', ...]``)
    :rtype: ``list``
    """
    filename = docs.get('meta.classifiers', 'CLASSIFIERS').strip()
    if filename and _os.path.isfile(filename):
        fp = open(filename, encoding='utf-8')
        try:
            content = fp.read()
        finally:
            fp.close()
        content = [item.strip() for item in content.splitlines()]
        return [item for item in content if item and not item.startswith('#')]
    return []


def find_provides(docs):
    """
    Determine provides from PROVIDES

    :return: List of provides (``['provides', ...]``)
    :rtype: ``list``
    """
    filename = docs.get('meta.provides', 'PROVIDES').strip()
    if filename and _os.path.isfile(filename):
        fp = open(filename, encoding='utf-8')
        try:
            content = fp.read()
        finally:
            fp.close()
        content = [item.strip() for item in content.splitlines()]
        return [item for item in content if item and not item.startswith('#')]
    return []


def find_license(docs):
    """
    Determine license from LICENSE

    :return: License text
    :rtype: ``str``
    """
    filename = docs.get('meta.license', 'LICENSE').strip()
    if filename and _os.path.isfile(filename):
        fp = open(filename, encoding='utf-8')
        try:
            return fp.read().rstrip()
        finally:
            fp.close()
    return None


def find_packages(manifest):
    """ Determine packages and subpackages """
    packages = {}
    collect = manifest.get('packages.collect', '').split()
    lib = manifest.get('packages.lib', '.')
    try:
        sep = _os.path.sep
    except AttributeError:
        sep = _os.path.join('1', '2')[1:-1]
    for root in collect:
        for dirpath, _, filenames in _shell.walk(_os.path.join(lib, root)):
            if dirpath.find('.svn') >= 0 or dirpath.find('.git') >= 0:
                continue
            if '__init__.py' in filenames:
                packages[
                    _os.path.normpath(dirpath).replace(sep, '.')
                ] = None
    packages = list(packages.keys())
    packages.sort()
    return packages


def find_data(name, docs):
    """ Determine data files """
    result = []
    if docs.get('extra', '').strip():
        result.append(_data.Documentation(docs['extra'].split(),
            prefix='share/doc/%s' % name,
        ))
    if docs.get('examples.dir', '').strip():
        tpl = ['recursive-include %s *' % docs['examples.dir']]
        if docs.get('examples.ignore', '').strip():
            tpl.extend(["global-exclude %s" % item
                for item in docs['examples.ignore'].split()
            ])
        strip = int(docs.get('examples.strip', '') or 0)
        result.append(_data.Documentation.from_templates(*tpl, **{
            'strip': strip,
            'prefix': 'share/doc/%s' % name,
            'preserve': 1,
        }))
    if docs.get('userdoc.dir', '').strip():
        tpl = ['recursive-include %s *' % docs['userdoc.dir']]
        if docs.get('userdoc.ignore', '').strip():
            tpl.extend(["global-exclude %s" % item
                for item in docs['userdoc.ignore'].split()
            ])
        strip = int(docs.get('userdoc.strip', '') or 0)
        result.append(_data.Documentation.from_templates(*tpl, **{
            'strip': strip,
            'prefix': 'share/doc/%s' % name,
            'preserve': 1,
        }))
    if docs.get('apidoc.dir', '').strip():
        tpl = ['recursive-include %s *' % docs['apidoc.dir']]
        if docs.get('apidoc.ignore', '').strip():
            tpl.extend(["global-exclude %s" % item
                for item in docs['apidoc.ignore'].split()
            ])
        strip = int(docs.get('apidoc.strip', '') or 0)
        result.append(_data.Documentation.from_templates(*tpl, **{
            'strip': strip,
            'prefix': 'share/doc/%s' % name,
            'preserve': 1,
        }))
    if docs.get('man', '').strip():
        result.extend(_data.Manpages.dispatch(docs['man'].split()))
    return result


def make_manifest(manifest, config, docs, kwargs):
    """ Create file list to pack up """
    # pylint: disable = R0912
    kwargs = kwargs.copy()
    kwargs['script_args'] = ['install']
    kwargs['packages'] = list(kwargs.get('packages') or ()) + [
        '_setup', '_setup.py2', '_setup.py3',
    ] + list(manifest.get('packages.extra', '').split() or ())
    _core._setup_stop_after = "commandline"
    try:
        dist = _core.setup(**kwargs)
    finally:
        _core._setup_stop_after = None

    result = ['MANIFEST', 'PKG-INFO', 'setup.py'] + list(config)
    # TODO: work with default values:
    for key in ('classifiers', 'description', 'summary', 'provides',
                'license'):
        filename = docs.get('meta.' + key, '').strip()
        if filename and _os.path.isfile(filename):
            result.append(filename)

    cmd = dist.get_command_obj("build_py")
    cmd.ensure_finalized()
    #from pprint import pprint; pprint(("build_py", cmd.get_source_files()))
    for item in cmd.get_source_files():
        result.append(_posixpath.sep.join(
            _os.path.normpath(item).split(_os.path.sep)
        ))

    cmd = dist.get_command_obj("build_ext")
    cmd.ensure_finalized()
    #from pprint import pprint; pprint(("build_ext", cmd.get_source_files()))
    for item in cmd.get_source_files():
        result.append(_posixpath.sep.join(
            _os.path.normpath(item).split(_os.path.sep)
        ))
    for ext in cmd.extensions:
        if ext.depends:
            result.extend([_posixpath.sep.join(
                _os.path.normpath(item).split(_os.path.sep)
            ) for item in ext.depends])

    cmd = dist.get_command_obj("build_clib")
    cmd.ensure_finalized()
    if cmd.libraries:
        #import pprint; pprint.pprint(("build_clib", cmd.get_source_files()))
        for item in cmd.get_source_files():
            result.append(_posixpath.sep.join(
                _os.path.normpath(item).split(_os.path.sep)
            ))
        for lib in cmd.libraries:
            if lib[1].get('depends'):
                result.extend([_posixpath.sep.join(
                    _os.path.normpath(item).split(_os.path.sep)
                ) for item in lib[1]['depends']])

    cmd = dist.get_command_obj("build_scripts")
    cmd.ensure_finalized()
    #import pprint; pprint.pprint(("build_scripts", cmd.get_source_files()))
    if cmd.get_source_files():
        for item in cmd.get_source_files():
            result.append(_posixpath.sep.join(
                _os.path.normpath(item).split(_os.path.sep)
            ))

    cmd = dist.get_command_obj("install_data")
    cmd.ensure_finalized()
    #from pprint import pprint; pprint(("install_data", cmd.get_inputs()))
    try:
        strings = str
    except NameError:
        strings = (str, str)

    for item in cmd.get_inputs():
        if isinstance(item, strings):
            result.append(item)
        else:
            result.extend(item[1])

    for item in manifest.get('dist', '').split():
        result.append(item)
        if _os.path.isdir(item):
            for filename in _shell.files(item):
                result.append(filename)

    result = list(dict([(item, None) for item in result]).keys())
    result.sort()
    return result


def run(config=('package.cfg',), ext=None, script_args=None, manifest_only=0):
    """ Main runner """
    if ext is None:
        ext = []

    cfg = _util.SafeConfigParser()
    cfg.read(config, encoding='utf-8')
    pkg = dict(cfg.items('package'))
    python_min = pkg.get('python.min') or None
    python_max = pkg.get('python.max') or None
    check_python_version('python', python_min, python_max)
    pypy_min = pkg.get('pypy.min') or None
    pypy_max = pkg.get('pypy.max') or None
    check_python_version('pypy', pypy_min, pypy_max)
    jython_min = pkg.get('jython.min') or None
    jython_max = pkg.get('jython.max') or None
    check_python_version('jython', jython_min, jython_max)

    manifest = dict(cfg.items('manifest'))
    try:
        docs = dict(cfg.items('docs'))
    except _config_parser.NoSectionError:
        docs = {}

    summary, description = find_description(docs)
    scripts = manifest.get('scripts', '').strip() or None
    if scripts:
        scripts = scripts.split()
    modules = manifest.get('modules', '').strip() or None
    if modules:
        modules = modules.split()
    keywords = docs.get('meta.keywords', '').strip() or None
    if keywords:
        keywords = keywords.split()
    revision = pkg.get('version.revision', '').strip()
    if revision:
        revision = "-r%s" % (revision,)

    kwargs = {
        'name': pkg['name'],
        'version': "%s%s" % (
            pkg['version.number'],
            ["", "-dev%s" % (revision,)][_util.humanbool(
                'version.dev', pkg.get('version.dev', 'false')
            )],
        ),
        'provides': find_provides(docs),
        'description': summary,
        'long_description': description,
        'classifiers': find_classifiers(docs),
        'keywords': keywords,
        'author': pkg['author.name'],
        'author_email': pkg['author.email'],
        'maintainer': pkg.get('maintainer.name'),
        'maintainer_email': pkg.get('maintainer.email'),
        'url': pkg.get('url.homepage'),
        'download_url': pkg.get('url.download'),
        'license': find_license(docs),
        'package_dir': {'': manifest.get('packages.lib', '.')},
        'packages': find_packages(manifest),
        'py_modules': modules,
        'ext_modules': ext,
        'scripts': scripts,
        'script_args': script_args,
        'data_files': find_data(pkg['name'], docs),
        'cmdclass': {
            'build'       : _commands.Build,
            'build_ext'   : _commands.BuildExt,
            'install'     : _commands.Install,
            'install_data': _commands.InstallData,
            'install_lib' : _commands.InstallLib,
        }
    }
    for key in ('provides',):
        if key not in _core.setup_keywords:
            del kwargs[key]

    if manifest_only:
        return make_manifest(manifest, config, docs, kwargs)

    # monkey-patch crappy manifest writer away.
    from distutils.command import sdist
    sdist.sdist.get_file_list = sdist.sdist.read_manifest

    return _core.setup(**kwargs)
