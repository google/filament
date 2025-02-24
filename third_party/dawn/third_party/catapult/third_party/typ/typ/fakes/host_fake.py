# Copyright 2014 Dirk Pranke. All rights reserved.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#    http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

import copy
import io
import logging
import sys

from typ import python_2_3_compat
from typ.host import _TeedStream


is_python3 = bool(sys.version_info.major == 3)

if is_python3:  # pragma: python3
    # pylint: disable=redefined-builtin,invalid-name
    unicode = str


class FakeHost(object):
    # "too many instance attributes" pylint: disable=R0902
    # "redefining built-in" pylint: disable=W0622
    # "unused arg" pylint: disable=W0613

    python_interpreter = 'python'
    is_python3 = bool(sys.version_info.major == 3)

    def __init__(self):
        self.logger = logging.getLogger()
        self.stdin = io.StringIO()
        self.stdout = io.StringIO()
        self.stderr = io.StringIO()
        self.platform = 'linux2'
        self.env = {}
        self.sep = '/'
        self.dirs = set([])
        self.files = {}
        self.fetches = []
        self.fetch_responses = {}
        self.written_files = {}
        self.last_tmpdir = None
        self.current_tmpno = 0
        self.mtimes = {}
        self.cmds = []
        self.cwd = '/tmp'
        self._orig_logging_handlers = []

    def __getstate__(self):
        d = copy.copy(self.__dict__)
        del d['stderr']
        del d['stdout']
        del d['stdin']
        del d['logger']
        del d['_orig_logging_handlers']
        return d

    def __setstate__(self, d):
        for k, v in d.items():
            setattr(self, k, v)
        self.logger = logging.getLogger()
        self.stdin = io.StringIO()
        self.stdout = io.StringIO()
        self.stderr = io.StringIO()

    def abspath(self, *comps):
        relpath = self.join(*comps)
        if relpath.startswith('/'):
            return relpath
        return self.join(self.cwd, relpath)

    def add_to_path(self, *comps):
        absolute_path = self.abspath(*comps)
        if absolute_path not in sys.path:
            sys.path.append(absolute_path)

    def basename(self, path):
        return path.split(self.sep)[-1]

    def call(self, argv, stdin=None, env=None):
        self.cmds.append(argv)
        return 0, '', ''

    def call_inline(self, argv):
        return self.call(argv)[0]

    def chdir(self, *comps):
        path = self.join(*comps)
        if not path.startswith('/'):
            path = self.join(self.cwd, path)
        self.cwd = path

    def cpu_count(self):
        return 1

    def dirname(self, path):
        return '/'.join(path.split('/')[:-1])

    def exists(self, *comps):
        path = self.abspath(*comps)
        return ((path in self.files and self.files[path] is not None) or
                path in self.dirs)

    def files_under(self, top):
        files = []
        top = self.abspath(top)
        for f in self.files:
            if self.files[f] is not None and f.startswith(top):
                files.append(self.relpath(f, top))
        return files

    def for_mp(self):
        return self

    def getcwd(self):
        return self.cwd

    def getenv(self, key, default=None):
        return self.env.get(key, default)

    def getpid(self):
        return 1

    def isdir(self, *comps):
        path = self.abspath(*comps)
        return path in self.dirs

    def isfile(self, *comps):
        path = self.abspath(*comps)
        return path in self.files and self.files[path] is not None

    def join(self, *comps):
        p = ''
        for c in comps:
            if c in ('', '.'):
                continue
            elif c.startswith('/'):
                p = c
            elif p:
                p += '/' + c
            else:
                p = c

        # Handle ./
        p = p.replace('/./', '/')

        # Handle ../
        while '/..' in p:
            comps = p.split('/')
            idx = comps.index('..')
            comps = comps[:idx-1] + comps[idx+1:]
            p = '/'.join(comps)
        return p

    def maybe_make_directory(self, *comps):
        path = self.abspath(self.join(*comps))
        if path not in self.dirs:
            self.dirs.add(path)

    def mktempfile(self, delete=True):
        curno = self.current_tmpno
        self.current_tmpno += 1
        f = io.StringIO()
        f.name = '__im_tmp/tmpfile_%u' % curno
        return f

    def mkdtemp(self, suffix='', prefix='tmp', dir=None, **_kwargs):
        if dir is None:
            dir = self.sep + '__im_tmp'
        curno = self.current_tmpno
        self.current_tmpno += 1
        self.last_tmpdir = self.join(dir, '%s_%u_%s' % (prefix, curno, suffix))
        self.dirs.add(self.last_tmpdir)
        return self.last_tmpdir

    def mtime(self, *comps):
        return self.mtimes.get(self.join(*comps), 0)

    def print_(self, msg='', end='\n', stream=None):
        stream = stream or self.stdout
        message = msg + end
        encoding = stream.encoding or 'utf8'
        stream.write(
            message.encode(encoding,
                           errors='backslashreplace').decode(encoding))
        stream.flush()

    def read_binary_file(self, *comps):
        return self._read(comps)

    def read_text_file(self, *comps):
        return self._read(comps)

    def _read(self, comps):
        return self.files[self.abspath(*comps)]

    def realpath(self, *comps):
        return self.abspath(*comps)

    def relpath(self, path, start):
        return path.replace(start + '/', '')

    def remove(self, *comps):
        path = self.abspath(*comps)
        self.files[path] = None
        self.written_files[path] = None

    def rmtree(self, *comps):
        path = self.abspath(*comps)
        for f in self.files:
            if f.startswith(path):
                self.files[f] = None
                self.written_files[f] = None
        self.dirs.remove(path)

    def terminal_width(self):
        return 80

    def splitext(self, path):
        idx = path.rfind('.')
        if idx == -1:
            return (path, '')
        return (path[:idx], path[idx:])

    def time(self):
        return 0

    def write_binary_file(self, path, contents):
        self._write(path, contents)

    def write_text_file(self, path, contents):
        self._write(path, contents)

    def append_text_file(self, path, contents):
        full_path = self.abspath(path)
        self.maybe_make_directory(self.dirname(full_path))
        self.files[full_path] = self.files.get(full_path, '') + contents

    def _write(self, path, contents):
        full_path = self.abspath(path)
        self.maybe_make_directory(self.dirname(full_path))
        self.files[full_path] = contents
        self.written_files[full_path] = contents

    def fetch(self, url, data=None, headers=None):
        resp = self.fetch_responses.get(url, FakeResponse(unicode(''), url))
        self.fetches.append((url, data, headers, resp))
        return resp

    def _tap_output(self):
        self.stdout = _TeedStream(self.stdout)
        self.stderr = _TeedStream(self.stderr)
        if True:
            sys.stdout = self.stdout
            sys.stderr = self.stderr

    def _untap_output(self):
        assert isinstance(self.stdout, _TeedStream)
        self.stdout = self.stdout.stream
        self.stderr = self.stderr.stream
        if True:
            sys.stdout = self.stdout
            sys.stderr = self.stderr

    def capture_output(self, divert=True):
        self._tap_output()
        self._orig_logging_handlers = self.logger.handlers
        if self._orig_logging_handlers:
            self.logger.handlers = [logging.StreamHandler(self.stderr)]
        self.stdout.capture(divert=divert)
        self.stderr.capture(divert=divert)

    def restore_output(self):
        assert isinstance(self.stdout, _TeedStream)
        out, err = (self.stdout.restore(), self.stderr.restore())
        out = python_2_3_compat.bytes_to_str(out)
        err = python_2_3_compat.bytes_to_str(err)
        self.logger.handlers = self._orig_logging_handlers
        self._untap_output()
        return out, err


class FakeResponse(io.StringIO):

    def __init__(self, response, url, code=200):
        io.StringIO.__init__(self, response)
        self._url = url
        self.code = code

    def geturl(self):
        return self._url

    def getcode(self):
        return self.code
