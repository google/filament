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
=================
 Shell utilities
=================

Shell utilities.
"""

from __future__ import generators
from __future__ import print_function
__author__ = u"Andr\xe9 Malo"
__docformat__ = "restructuredtext en"

import errno as _errno
import fnmatch as _fnmatch
import os as _os
import shutil as _shutil
import sys as _sys
import tempfile as _tempfile

cwd = _os.path.dirname(_os.path.abspath(_sys.argv[0]))

class ExitError(RuntimeError):
    """ Exit error """
    def __init__(self, code):
        RuntimeError.__init__(self, code)
        self.code = code
        self.signal = None


class SignalError(ExitError):
    """ Signal error """
    def __init__(self, code, signal):
        ExitError.__init__(self, code)
        import signal as _signal
        self.signal = signal
        for key, val in vars(_signal).iteritems():
            if key.startswith('SIG') and not key.startswith('SIG_'):
                if val == signal:
                    self.signalstr = key[3:]
                    break
        else:
            self.signalstr = '%04d' % signal


def native(path):
    """ Convert slash path to native """
    path = _os.path.sep.join(path.split('/'))
    return _os.path.normpath(_os.path.join(cwd, path))


def cp(src, dest):
    """ Copy src to dest """
    _shutil.copy2(native(src), native(dest))


def cp_r(src, dest):
    """ Copy -r src to dest """
    _shutil.copytree(native(src), native(dest))


def rm(dest):
    """ Remove a file """
    try:
        _os.unlink(native(dest))
    except OSError, e:
        if _errno.ENOENT != e.errno:
            raise

def rm_rf(dest):
    """ Remove a tree """
    dest = native(dest)
    if _os.path.exists(dest):
        for path in files(dest, '*'):
            _os.chmod(native(path), 0644)
        _shutil.rmtree(dest)


try:
    mkstemp = _tempfile.mkstemp
except AttributeError:
    # helpers stolen from 2.4 tempfile module
    try:
        import fcntl as _fcntl
    except ImportError:
        def _set_cloexec(fd):
            """ Set close-on-exec (not implemented, but not an error) """
            # pylint: disable = W0613
            pass
    else:
        def _set_cloexec(fd):
            """ Set close-on-exec """
            try:
                flags = _fcntl.fcntl(fd, _fcntl.F_GETFD, 0)
            except IOError:
                pass
            else:
                # flags read successfully, modify
                flags |= _fcntl.FD_CLOEXEC
                _fcntl.fcntl(fd, _fcntl.F_SETFD, flags)

    _text_openflags = _os.O_RDWR | _os.O_CREAT | _os.O_EXCL
    _text_openflags |= getattr(_os, 'O_NOINHERIT', 0)
    _text_openflags |= getattr(_os, 'O_NOFOLLOW', 0)

    _bin_openflags = _text_openflags
    _bin_openflags |= getattr(_os, 'O_BINARY', 0)

    def mkstemp(suffix="", prefix=_tempfile.gettempprefix(), dir=None,
                text=False):
        """ Create secure temp file """
        # pylint: disable = W0622
        if dir is None:
            dir = _tempfile.gettempdir()
        if text:
            flags = _text_openflags
        else:
            flags = _bin_openflags
        count = 100
        while count > 0:
            j = _tempfile._counter.get_next() # pylint: disable = E1101, W0212
            fname = _os.path.join(dir, prefix + str(j) + suffix)
            try:
                fd = _os.open(fname, flags, 0600)
            except OSError, e:
                if e.errno == _errno.EEXIST:
                    count -= 1
                    continue
                raise
            _set_cloexec(fd)
            return fd, _os.path.abspath(fname)
        raise IOError, (_errno.EEXIST, "No usable temporary file name found")


def _pipespawn(argv, env):
    """ Pipe spawn """
    # pylint: disable = R0912
    import pickle as _pickle
    fd, name = mkstemp('.py')
    try:
        _os.write(fd, (r"""
import os
import pickle
try:
    import subprocess
except ImportError:
    subprocess = None
import sys

argv = pickle.loads(%(argv)s)
env = pickle.loads(%(env)s)
if 'X_JYTHON_WA_PATH' in env:
    env['PATH'] = env['X_JYTHON_WA_PATH']

if subprocess is None:
    pid = os.spawnve(os.P_NOWAIT, argv[0], argv, env)
    result = os.waitpid(pid, 0)[1]
else:
    p = subprocess.Popen(argv, env=env)
    result = p.wait()
    if result < 0:
        print "\n%%d 1" %% (-result)
        sys.exit(2)

if result == 0:
    sys.exit(0)
signalled = getattr(os, 'WIFSIGNALED', None)
if signalled is not None:
    if signalled(result):
        print "\n%%d %%d" %% (os.WTERMSIG(result), result & 7)
        sys.exit(2)
print "\n%%d" %% (result & 7,)
sys.exit(3)
        """.strip() + "\n") % {
            'argv': repr(_pickle.dumps(argv)),
            'env': repr(_pickle.dumps(env)),
        })
        fd, _ = None, _os.close(fd)
        if _sys.platform == 'win32':
            argv = []
            for arg in [_sys.executable, name]:
                if ' ' in arg or arg.startswith('"'):
                    arg = '"%s"' % arg.replace('"', '\\"')
                argv.append(arg)
            argv = ' '.join(argv)
            shell = True
            close_fds = False
        else:
            argv = [_sys.executable, name]
            shell = False
            close_fds = True

        res = 0
        try:
            import subprocess
        except ImportError:
            import popen2 as _popen2
            proc = _popen2.Popen3(argv, False)
            try:
                proc.tochild.close()
                result = proc.fromchild.read()
            finally:
                res = proc.wait()
        else:
            if 'X_JYTHON_WA_PATH' in env:
                env['PATH'] = env['X_JYTHON_WA_PATH']

            proc = subprocess.Popen(argv,
                shell=shell,
                stdin=subprocess.PIPE,
                stdout=subprocess.PIPE,
                close_fds=close_fds,
                env=env,
            )
            try:
                proc.stdin.close()
                result = proc.stdout.read()
            finally:
                res = proc.wait()
        if res != 0:
            if res == 2:
                signal, code = map(int, result.splitlines()[-1].split())
                raise SignalError(code, signal)
            elif res == 3:
                code = int(result.splitlines()[-1].strip())
                raise ExitError(code)
            raise ExitError(res)

        return result
    finally:
        try:
            if fd is not None:
                _os.close(fd)
        finally:
            _os.unlink(name)


def _filepipespawn(infile, outfile, argv, env):
    """ File Pipe spawn """
    try:
        import subprocess
    except ImportError:
        subprocess = None
    import pickle as _pickle
    fd, name = mkstemp('.py')
    try:
        _os.write(fd, ("""
import os
import pickle
import sys

infile = pickle.loads(%(infile)s)
outfile = pickle.loads(%(outfile)s)
argv = pickle.loads(%(argv)s)
env = pickle.loads(%(env)s)

if infile is not None:
    infile = open(infile, 'rb')
    os.dup2(infile.fileno(), 0)
    infile.close()
if outfile is not None:
    outfile = open(outfile, 'wb')
    os.dup2(outfile.fileno(), 1)
    outfile.close()

pid = os.spawnve(os.P_NOWAIT, argv[0], argv, env)
result = os.waitpid(pid, 0)[1]
sys.exit(result & 7)
        """.strip() + "\n") % {
            'infile': repr(_pickle.dumps(_os.path.abspath(infile))),
            'outfile': repr(_pickle.dumps(_os.path.abspath(outfile))),
            'argv': repr(_pickle.dumps(argv)),
            'env': repr(_pickle.dumps(env)),
        })
        fd, _ = None, _os.close(fd)
        if _sys.platform == 'win32':
            argv = []
            for arg in [_sys.executable, name]:
                if ' ' in arg or arg.startswith('"'):
                    arg = '"%s"' % arg.replace('"', '\\"')
                argv.append(arg)
            argv = ' '.join(argv)
            close_fds = False
            shell = True
        else:
            argv = [_sys.executable, name]
            close_fds = True
            shell = False

        if subprocess is None:
            pid = _os.spawnve(_os.P_NOWAIT, argv[0], argv, env)
            return _os.waitpid(pid, 0)[1]
        else:
            p = subprocess.Popen(
                argv, env=env, shell=shell, close_fds=close_fds
            )
            return p.wait()
    finally:
        try:
            if fd is not None:
                _os.close(fd)
        finally:
            _os.unlink(name)


def spawn(*argv, **kwargs):
    """ Spawn a process """
    try:
        import subprocess
    except ImportError:
        subprocess = None

    if _sys.platform == 'win32':
        newargv = []
        for arg in argv:
            if not arg or ' ' in arg or arg.startswith('"'):
                arg = '"%s"' % arg.replace('"', '\\"')
            newargv.append(arg)
        argv = newargv
        close_fds = False
        shell = True
    else:
        close_fds = True
        shell = False

    env = kwargs.get('env')
    if env is None:
        env = dict(_os.environ)
    if 'X_JYTHON_WA_PATH' in env:
        env['PATH'] = env['X_JYTHON_WA_PATH']

    echo = kwargs.get('echo')
    if echo:
        print(' '.join(argv))
    filepipe = kwargs.get('filepipe')
    if filepipe:
        return _filepipespawn(
            kwargs.get('stdin'), kwargs.get('stdout'), argv, env
        )
    pipe = kwargs.get('stdout')
    if pipe:
        return _pipespawn(argv, env)

    if subprocess is None:
        pid = _os.spawnve(_os.P_NOWAIT, argv[0], argv, env)
        return _os.waitpid(pid, 0)[1]
    else:
        p = subprocess.Popen(argv, env=env, shell=shell, close_fds=close_fds)
        return p.wait()


try:
    walk = _os.walk
except AttributeError:
    # copy from python 2.4 sources (modulo docs and comments)
    def walk(top, topdown=True, onerror=None):
        """ directory tree walker """
        # pylint: disable = C0103
        join, isdir, islink = _os.path.join, _os.path.isdir, _os.path.islink
        listdir, error = _os.listdir, _os.error

        try:
            names = listdir(top)
        except error, err:
            if onerror is not None:
                onerror(err)
            return

        dirs, nondirs = [], []
        for name in names:
            if isdir(join(top, name)):
                dirs.append(name)
            else:
                nondirs.append(name)

        if topdown:
            yield top, dirs, nondirs
        for name in dirs:
            path = join(top, name)
            if not islink(path):
                for x in walk(path, topdown, onerror):
                    yield x
        if not topdown:
            yield top, dirs, nondirs


def files(base, wildcard='[!.]*', recursive=1, prune=('.git', '.svn', 'CVS')):
    """ Determine a filelist """
    for dirpath, dirnames, filenames in walk(native(base)):
        for item in prune:
            if item in dirnames:
                dirnames.remove(item)

        filenames.sort()
        for name in _fnmatch.filter(filenames, wildcard):
            dest = _os.path.join(dirpath, name)
            if dest.startswith(cwd):
                dest = dest.replace(cwd, '', 1)
            aslist = []
            head, tail = _os.path.split(dest)
            while tail:
                aslist.append(tail)
                head, tail = _os.path.split(head)
            aslist.reverse()
            dest = '/'.join(aslist)
            yield dest

        if not recursive:
            break
        dirnames.sort()


def dirs(base, wildcard='[!.]*', recursive=1, prune=('.git', '.svn', 'CVS')):
    """ Determine a filelist """
    for dirpath, dirnames, filenames in walk(native(base)):
        for item in prune:
            if item in dirnames:
                dirnames.remove(item)

        dirnames.sort()
        for name in _fnmatch.filter(dirnames, wildcard):
            dest = _os.path.join(dirpath, name)
            if dest.startswith(cwd):
                dest = dest.replace(cwd, '', 1)
            aslist = []
            head, tail = _os.path.split(dest)
            while tail:
                aslist.append(tail)
                head, tail = _os.path.split(head)
            aslist.reverse()
            dest = '/'.join(aslist)
            yield dest

        if not recursive:
            break


def frompath(executable):
    """ Find executable in PATH """
    # Based on distutils.spawn.find_executable.
    path = _os.environ.get('PATH', '')
    paths = [
        _os.path.expanduser(item)
        for item in path.split(_os.pathsep)
    ]
    ext = _os.path.splitext(executable)[1]
    exts = ['']
    if _sys.platform == 'win32' or _os.name == 'os2':
        eext = ['.exe', '.bat', '.py']
        if ext not in eext:
            exts.extend(eext)

    for ext in exts:
        if not _os.path.isfile(executable + ext):
            for path in paths:
                fname = _os.path.join(path, executable + ext)
                if _os.path.isfile(fname):
                    # the file exists, we have a shot at spawn working
                    return fname
        else:
            return executable + ext

    return None
