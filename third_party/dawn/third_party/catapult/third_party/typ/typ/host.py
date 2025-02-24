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

import io
import logging
import multiprocessing
import os
import shutil
import subprocess
import sys
import tempfile
import time

from typ import python_2_3_compat


if sys.version_info.major == 2:  # pragma: python2
    from urllib2 import urlopen, Request
else:  # pragma: python3
    # pylint: disable=E0611
    assert sys.version_info.major == 3
    from urllib.request import urlopen, Request  # pylint: disable=F0401,E0611


class Host(object):
    python_interpreter = sys.executable
    is_python3 = bool(sys.version_info.major == 3)

    pathsep = os.pathsep
    sep = os.sep
    env = os.environ

    _orig_stdout = sys.stdout
    _orig_stderr = sys.stderr

    def __init__(self):
        self.logger = logging.getLogger()
        self._orig_logging_handlers = None
        self.stdout = sys.stdout
        self.stderr = sys.stderr
        self.stdin = sys.stdin
        self.env = os.environ
        self.platform = sys.platform

    def abspath(self, *comps):
        return os.path.abspath(self.join(*comps))

    def add_to_path(self, *comps):
        absolute_path = self.abspath(*comps)
        if absolute_path not in sys.path:
            sys.path.insert(0, absolute_path)

    def basename(self, path):
        return os.path.basename(path)

    def call(self, argv, stdin=None, env=None):
        if stdin:
            stdin_pipe = subprocess.PIPE
        else:
            stdin_pipe = None
        proc = subprocess.Popen(argv, stdout=subprocess.PIPE,
                                stderr=subprocess.PIPE, stdin=stdin_pipe,
                                env=env)
        if stdin_pipe:
            proc.stdin.write(stdin.encode('utf-8'))
        stdout, stderr = proc.communicate()

        # pylint type checking bug - pylint: disable=E1103
        return proc.returncode, stdout.decode('utf-8'), stderr.decode('utf-8')

    def call_inline(self, argv, env=None):
        if isinstance(self.stdout, _TeedStream):  # pragma: no cover
            ret, out, err = self.call(argv, env)
            self.print_(out, end='')
            self.print_(err, end='', stream=self.stderr)
            return ret
        return subprocess.call(argv, stdin=self.stdin, stdout=self.stdout,
                               stderr=self.stderr, env=env)

    def chdir(self, *comps):
        return os.chdir(self.join(*comps))

    def cpu_count(self):
        return multiprocessing.cpu_count()

    def dirname(self, *comps):
        return os.path.dirname(self.join(*comps))

    def exists(self, *comps):
        return os.path.exists(self.join(*comps))

    def files_under(self, top):
        all_files = []
        for root, _, files in os.walk(top):
            for f in files:
                relpath = self.relpath(os.path.join(root, f), top)
                all_files.append(relpath)
        return all_files

    def getcwd(self):
        return os.getcwd()

    def getenv(self, key, default=None):
        return self.env.get(key, default)

    def getpid(self):
        return os.getpid()

    def for_mp(self):
        return None

    def isdir(self, *comps):
        return os.path.isdir(os.path.join(*comps))

    def isfile(self, *comps):
        return os.path.isfile(os.path.join(*comps))

    def join(self, *comps):
        return os.path.join(*comps)

    def maybe_make_directory(self, *comps):
        path = self.abspath(self.join(*comps))
        try:
            # Once `typ` drops python2 support, use the `exist_ok=True` keyword
            # argument instead of catching the exception.
            os.makedirs(path)
        except OSError:
            pass

    def mktempfile(self, delete=True):
        return tempfile.NamedTemporaryFile(delete=delete)

    def mkdtemp(self, **kwargs):
        return tempfile.mkdtemp(**kwargs)

    def mtime(self, *comps):
        return os.stat(self.join(*comps)).st_mtime

    def print_(self, msg='', end='\n', stream=None):
        stream = stream or self.stdout
        message = str(msg) + end
        encoding = stream.encoding or 'ascii'
        if sys.version_info.major == 2:
            stream.write(message)
        else:
            stream.write(
                message.encode(encoding,
                               errors='backslashreplace').decode(encoding))
        stream.flush()

    def read_text_file(self, *comps):
        return self._read(comps, 'r')

    def read_binary_file(self, *comps):
        return self._read(comps, 'rb')

    def _read(self, comps, mode):
        path = self.join(*comps)
        with open(path, mode) as f:
            return f.read()

    def realpath(self, *comps):
        return os.path.realpath(os.path.join(*comps))

    def relpath(self, path, start):
        return os.path.relpath(path, start)

    def remove(self, *comps):
        os.remove(self.join(*comps))

    def rmtree(self, path):
        shutil.rmtree(path, ignore_errors=True)

    def splitext(self, path):
        return os.path.splitext(path)

    def time(self):
        return time.time()

    def append_text_file(self, path, content):
        return self._write(path, content, mode='a')

    def write_text_file(self, path, contents):
        return self._write(path, contents, mode='w')

    def write_binary_file(self, path, contents):
        return self._write(path, contents, mode='wb')

    def _write(self, path, contents, mode):
        with open(path, mode) as f:
            f.write(contents)

    def fetch(self, url, data=None, headers=None):
        headers = headers or {}
        return urlopen(Request(url, data.encode('utf8'), headers))

    def terminal_width(self):
        """Returns 0 if the width cannot be determined."""
        try:
            if sys.platform == 'win32':  # pragma: win32
                # From http://code.activestate.com/recipes/ \
                #   440694-determine-size-of-console-window-on-windows/
                from ctypes import windll, create_string_buffer

                STDERR_HANDLE = -12
                handle = windll.kernel32.GetStdHandle(STDERR_HANDLE)

                SCREEN_BUFFER_INFO_SZ = 22
                buf = create_string_buffer(SCREEN_BUFFER_INFO_SZ)

                if windll.kernel32.GetConsoleScreenBufferInfo(handle, buf):
                    import struct
                    fields = struct.unpack("hhhhHhhhhhh", buf.raw)
                    left = fields[5]
                    right = fields[7]

                    # Note that we return 1 less than the width since writing
                    # into the rightmost column automatically performs a
                    # line feed.
                    return right - left
                return 0
            else:  # pragma: no win32
                import fcntl
                import struct
                import termios
                packed = fcntl.ioctl(self.stderr.fileno(),
                                     termios.TIOCGWINSZ, '\0' * 8)
                _, columns, _, _ = struct.unpack('HHHH', packed)
                return columns
        except Exception:
            return 0

    def _tap_output(self):
        self.stdout = sys.stdout = _TeedStream(self.stdout)
        self.stderr = sys.stderr = _TeedStream(self.stderr)

    def _untap_output(self):
        assert isinstance(self.stdout, _TeedStream)
        self.stdout = sys.stdout = self.stdout.stream
        self.stderr = sys.stderr = self.stderr.stream

    def capture_output(self, divert=True):
        self._tap_output()
        self._orig_logging_handlers = self.logger.handlers
        if self._orig_logging_handlers:
            self.logger.handlers = [logging.StreamHandler(self.stderr)]
        self.stdout.capture(divert)
        self.stderr.capture(divert)

    def restore_output(self):
        assert isinstance(self.stdout, _TeedStream)
        out, err = (self.stdout.restore(), self.stderr.restore())
        out = python_2_3_compat.bytes_to_str(out)
        err = python_2_3_compat.bytes_to_str(err)
        self.logger.handlers = self._orig_logging_handlers
        self._untap_output()
        return out, err


class _TeedStream(io.StringIO):

    def __init__(self, stream):
        super(_TeedStream, self).__init__()
        self.stream = stream
        self.capturing = False
        self.diverting = False

    @property
    def encoding(self):
        return self.stream.encoding

    def write(self, msg, *args, **kwargs):
        if self.capturing:
            if (sys.version_info.major == 2 and
                    isinstance(msg, str)):  # pragma: python2
                msg = unicode(msg)
            super(_TeedStream, self).write(msg, *args, **kwargs)
        if not self.diverting:
            self.stream.write(msg, *args, **kwargs)

    def flush(self):
        if self.capturing:
            super(_TeedStream, self).flush()
        if not self.diverting:
            self.stream.flush()

    def capture(self, divert=True):
        self.truncate(0)
        self.capturing = True
        self.diverting = divert

    def restore(self):
        msg = self.getvalue()
        self.truncate(0)
        self.capturing = False
        self.diverting = False
        return msg
