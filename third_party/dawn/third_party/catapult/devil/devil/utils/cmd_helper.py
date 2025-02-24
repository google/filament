# Copyright (c) 2012 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
"""A wrapper for subprocess to make calling shell commands easier."""

import codecs
import logging
import os
import pipes
import select
import signal
import string
import subprocess
import sys
import time

CATAPULT_ROOT_PATH = os.path.abspath(
    os.path.join(os.path.dirname(__file__), '..', '..', '..'))
SIX_PATH = os.path.join(CATAPULT_ROOT_PATH, 'third_party', 'six')

if SIX_PATH not in sys.path:
  sys.path.append(SIX_PATH)

import six

from devil import base_error

logger = logging.getLogger(__name__)

_SafeShellChars = frozenset(string.ascii_letters + string.digits + '@%_-+=:,./')

# Cache the string-escape codec to ensure subprocess can find it
# later. Return value doesn't matter.
if six.PY2:
  codecs.lookup('string-escape')


def SingleQuote(s):
  """Return an shell-escaped version of the string using single quotes.

  Reliably quote a string which may contain unsafe characters (e.g. space,
  quote, or other special characters such as '$').

  The returned value can be used in a shell command line as one token that gets
  to be interpreted literally.

  Args:
    s: The string to quote.

  Return:
    The string quoted using single quotes.
  """
  return pipes.quote(s)


def DoubleQuote(s):
  """Return an shell-escaped version of the string using double quotes.

  Reliably quote a string which may contain unsafe characters (e.g. space
  or quote characters), while retaining some shell features such as variable
  interpolation.

  The returned value can be used in a shell command line as one token that gets
  to be further interpreted by the shell.

  The set of characters that retain their special meaning may depend on the
  shell implementation. This set usually includes: '$', '`', '\', '!', '*',
  and '@'.

  Args:
    s: The string to quote.

  Return:
    The string quoted using double quotes.
  """
  if not s:
    return '""'
  if all(c in _SafeShellChars for c in s):
    return s
  return '"' + s.replace('"', '\\"') + '"'


def ShrinkToSnippet(cmd_parts, var_name, var_value):
  """Constructs a shell snippet for a command using a variable to shrink it.

  Takes into account all quoting that needs to happen.

  Args:
    cmd_parts: A list of command arguments.
    var_name: The variable that holds var_value.
    var_value: The string to replace in cmd_parts with $var_name

  Returns:
    A shell snippet that does not include setting the variable.
  """

  def shrink(value):
    parts = (x and SingleQuote(x) for x in value.split(var_value))
    with_substitutions = ('"$%s"' % var_name).join(parts)
    return with_substitutions or "''"

  return ' '.join(shrink(part) for part in cmd_parts)


def Popen(args,
          stdin=None,
          stdout=None,
          stderr=None,
          shell=None,
          cwd=None,
          env=None):
  # preexec_fn isn't supported on windows.
  # pylint: disable=unexpected-keyword-arg
  if sys.platform == 'win32':
    close_fds = (stdin is None and stdout is None and stderr is None)
    preexec_fn = None
  else:
    close_fds = True
    preexec_fn = lambda: signal.signal(signal.SIGPIPE, signal.SIG_DFL)

  if six.PY2:
    return subprocess.Popen(  # pylint: disable=subprocess-popen-preexec-fn
        args=args,
        cwd=cwd,
        stdin=stdin,
        stdout=stdout,
        stderr=stderr,
        shell=shell,
        close_fds=close_fds,
        env=env,
        preexec_fn=preexec_fn)

  # opens stdout in text mode, so that caller side always get 'str',
  # and there will be no type mismatch error.
  # Ignore any decoding error, so that caller will not crash due to
  # uncaught exception. Decoding errors are unavoidable, as we
  # do not know the encoding of the output, and in some output there
  # will be multiple encodings (e.g. adb logcat)
  return subprocess.Popen(  # pylint: disable=subprocess-popen-preexec-fn
      args=args,
      cwd=cwd,
      stdin=stdin,
      stdout=stdout,
      stderr=stderr,
      shell=shell,
      close_fds=close_fds,
      env=env,
      preexec_fn=preexec_fn,
      universal_newlines=True,
      encoding='utf-8',
      errors='ignore')


def Call(args, stdout=None, stderr=None, shell=None, cwd=None, env=None):
  pipe = Popen(
      args, stdout=stdout, stderr=stderr, shell=shell, cwd=cwd, env=env)
  pipe.communicate()
  return pipe.wait()


def RunCmd(args, cwd=None):
  """Opens a subprocess to execute a program and returns its return value.

  Args:
    args: A string or a sequence of program arguments. The program to execute is
      the string or the first item in the args sequence.
    cwd: If not None, the subprocess's current directory will be changed to
      |cwd| before it's executed.

  Returns:
    Return code from the command execution.
  """
  logger.debug("%s %s", str(args), cwd or '')
  return Call(args, cwd=cwd)


def GetCmdOutput(args, cwd=None, shell=False, env=None):
  """Open a subprocess to execute a program and returns its output.

  Args:
    args: A string or a sequence of program arguments. The program to execute is
      the string or the first item in the args sequence.
    cwd: If not None, the subprocess's current directory will be changed to
      |cwd| before it's executed.
    shell: Whether to execute args as a shell command.
    env: If not None, a mapping that defines environment variables for the
      subprocess.

  Returns:
    Captures and returns the command's stdout.
    Prints the command's stderr to logger (which defaults to stdout).
  """
  (_, output) = GetCmdStatusAndOutput(args, cwd, shell, env)
  return output


def _ValidateAndLogCommand(args, cwd, shell):
  if isinstance(args, six.string_types):
    if not shell:
      raise Exception('string args must be run with shell=True')
  else:
    if shell:
      raise Exception('array args must be run with shell=False')
    args = ' '.join(SingleQuote(str(c)) for c in args)
  if cwd is None:
    cwd = ''
  else:
    cwd = ':' + cwd
  logger.debug('[host]%s> %s', cwd, args)
  return args


def GetCmdStatusAndOutput(args,
                          cwd=None,
                          shell=False,
                          env=None,
                          merge_stderr=False):
  """Executes a subprocess and returns its exit code and output.

  Args:
    args: A string or a sequence of program arguments. The program to execute is
      the string or the first item in the args sequence.
    cwd: If not None, the subprocess's current directory will be changed to
      |cwd| before it's executed.
    shell: Whether to execute args as a shell command. Must be True if args
      is a string and False if args is a sequence.
    env: If not None, a mapping that defines environment variables for the
      subprocess.
    merge_stderr: If True, captures stderr as part of stdout.

  Returns:
    The 2-tuple (exit code, stdout).
  """
  status, stdout, stderr = GetCmdStatusOutputAndError(
      args, cwd=cwd, shell=shell, env=env, merge_stderr=merge_stderr)

  if stderr:
    logger.critical('STDERR: %s', stderr)
  logger.debug('STDOUT: %s%s', stdout[:4096].rstrip(),
               '<truncated>' if len(stdout) > 4096 else '')
  return (status, stdout)


def StartCmd(args, cwd=None, shell=False, env=None):
  """Starts a subprocess and returns a handle to the process.

  Args:
    args: A string or a sequence of program arguments. The program to execute is
      the string or the first item in the args sequence.
    cwd: If not None, the subprocess's current directory will be changed to
      |cwd| before it's executed.
    shell: Whether to execute args as a shell command. Must be True if args
      is a string and False if args is a sequence.
    env: If not None, a mapping that defines environment variables for the
      subprocess.

  Returns:
    A process handle from subprocess.Popen.
  """
  _ValidateAndLogCommand(args, cwd, shell)
  return Popen(
      args,
      stdout=subprocess.PIPE,
      stderr=subprocess.PIPE,
      shell=shell,
      cwd=cwd,
      env=env)


def GetCmdStatusOutputAndError(args,
                               cwd=None,
                               shell=False,
                               env=None,
                               merge_stderr=False):
  """Executes a subprocess and returns its exit code, output, and errors.

  Args:
    args: A string or a sequence of program arguments. The program to execute is
      the string or the first item in the args sequence.
    cwd: If not None, the subprocess's current directory will be changed to
      |cwd| before it's executed.
    shell: Whether to execute args as a shell command. Must be True if args
      is a string and False if args is a sequence.
    env: If not None, a mapping that defines environment variables for the
      subprocess.
    merge_stderr: If True, captures stderr as part of stdout.

  Returns:
    The 3-tuple (exit code, stdout, stderr).
  """
  _ValidateAndLogCommand(args, cwd, shell)
  stderr = subprocess.STDOUT if merge_stderr else subprocess.PIPE
  pipe = Popen(
      args,
      stdout=subprocess.PIPE,
      stderr=stderr,
      shell=shell,
      cwd=cwd,
      env=env)
  stdout, stderr = pipe.communicate()
  return (pipe.returncode, stdout, stderr)


# TODO (https://crbug.com/1338100): Verify if we can change this type
class TimeoutError(base_error.BaseError):  # pylint: disable=redefined-builtin
  """Module-specific timeout exception."""

  def __init__(self, output=None):
    super(TimeoutError, self).__init__('Timeout')
    self._output = output

  @property
  def output(self):
    return self._output


def _read_and_decode(fd, buffer_size):
  data = os.read(fd, buffer_size)
  if data and six.PY3:
    data = data.decode('utf-8', errors='ignore')
  return data

def _IterProcessStdoutFcntl(process,
                            iter_timeout=None,
                            timeout=None,
                            buffer_size=4096,
                            poll_interval=1):
  """An fcntl-based implementation of _IterProcessStdout."""
  # pylint: disable=too-many-nested-blocks
  import fcntl
  try:
    # Enable non-blocking reads from the child's stdout.
    child_fd = process.stdout.fileno()
    fl = fcntl.fcntl(child_fd, fcntl.F_GETFL)
    fcntl.fcntl(child_fd, fcntl.F_SETFL, fl | os.O_NONBLOCK)

    end_time = (time.time() + timeout) if timeout else None
    iter_end_time = (time.time() + iter_timeout) if iter_timeout else None

    while True:
      if end_time and time.time() > end_time:
        raise TimeoutError()
      if iter_end_time and time.time() > iter_end_time:
        yield None
        iter_end_time = time.time() + iter_timeout

      if iter_end_time:
        iter_aware_poll_interval = min(poll_interval,
                                       max(0, iter_end_time - time.time()))
      else:
        iter_aware_poll_interval = poll_interval

      read_fds, _, _ = select.select([child_fd], [], [],
                                     iter_aware_poll_interval)
      if child_fd in read_fds:
        data = _read_and_decode(child_fd, buffer_size)
        if not data:
          break
        yield data

      if process.poll() is not None:
        # If process is closed, keep checking for output data (because of timing
        # issues).
        while True:
          read_fds, _, _ = select.select([child_fd], [], [],
                                         iter_aware_poll_interval)
          if child_fd in read_fds:
            data = _read_and_decode(child_fd, buffer_size)
            if data:
              yield data
              continue
          break
        break
  finally:
    try:
      if process.returncode is None:
        # Make sure the process doesn't stick around if we fail with an
        # exception.
        process.kill()
    except OSError:
      pass
    process.wait()


def _IterProcessStdoutQueue(process,
                            iter_timeout=None,
                            timeout=None,
                            buffer_size=4096,
                            poll_interval=1):
  """A Queue.Queue-based implementation of _IterProcessStdout.

  TODO(jbudorick): Evaluate whether this is a suitable replacement for
  _IterProcessStdoutFcntl on all platforms.
  """
  # pylint: disable=unused-argument
  if six.PY3:
    import queue
  else:
    import Queue as queue
  import threading

  stdout_queue = queue.Queue()

  def read_process_stdout():
    # TODO(jbudorick): Pick an appropriate read size here.
    while True:
      try:
        output_chunk = _read_and_decode(process.stdout.fileno(), buffer_size)
      except IOError:
        break
      stdout_queue.put(output_chunk, True)
      if not output_chunk and process.poll() is not None:
        break

  reader_thread = threading.Thread(target=read_process_stdout)
  reader_thread.start()

  end_time = (time.time() + timeout) if timeout else None

  try:
    while True:
      if end_time and time.time() > end_time:
        raise TimeoutError()
      try:
        s = stdout_queue.get(True, iter_timeout)
        if not s:
          break
        yield s
      except queue.Empty:
        yield None
  finally:
    try:
      if process.returncode is None:
        # Make sure the process doesn't stick around if we fail with an
        # exception.
        process.kill()
    except OSError:
      pass
    process.wait()
    reader_thread.join()


_IterProcessStdout = (_IterProcessStdoutQueue
                      if sys.platform == 'win32' else _IterProcessStdoutFcntl)
"""Iterate over a process's stdout.

This is intentionally not public.

Args:
  process: The process in question.
  iter_timeout: An optional length of time, in seconds, to wait in
    between each iteration. If no output is received in the given
    time, this generator will yield None.
  timeout: An optional length of time, in seconds, during which
    the process must finish. If it fails to do so, a TimeoutError
    will be raised.
  buffer_size: The maximum number of bytes to read (and thus yield) at once.
  poll_interval: The length of time to wait in calls to `select.select`.
    If iter_timeout is set, the remaining length of time in the iteration
    may take precedence.
Raises:
  TimeoutError: if timeout is set and the process does not complete.
Yields:
  basestrings of data or None.
"""


def GetCmdStatusAndOutputWithTimeout(args,
                                     timeout,
                                     cwd=None,
                                     shell=False,
                                     logfile=None,
                                     env=None):
  """Executes a subprocess with a timeout.

  Args:
    args: List of arguments to the program, the program to execute is the first
      element.
    timeout: the timeout in seconds or None to wait forever.
    cwd: If not None, the subprocess's current directory will be changed to
      |cwd| before it's executed.
    shell: Whether to execute args as a shell command. Must be True if args
      is a string and False if args is a sequence.
    logfile: Optional file-like object that will receive output from the
      command as it is running.
    env: If not None, a mapping that defines environment variables for the
      subprocess.

  Returns:
    The 2-tuple (exit code, output).
  Raises:
    TimeoutError on timeout.
  """
  _ValidateAndLogCommand(args, cwd, shell)
  output = six.StringIO()
  process = Popen(
      args,
      cwd=cwd,
      shell=shell,
      stdout=subprocess.PIPE,
      stderr=subprocess.STDOUT,
      env=env)
  try:
    for data in _IterProcessStdout(process, timeout=timeout):
      if logfile:
        logfile.write(data)
      output.write(data)
  except TimeoutError:
    raise TimeoutError(output.getvalue())

  str_output = output.getvalue()
  logger.debug('STDOUT+STDERR: %s%s', str_output[:4096].rstrip(),
               '<truncated>' if len(str_output) > 4096 else '')
  return process.returncode, str_output


def IterCmdOutputLines(args,
                       iter_timeout=None,
                       timeout=None,
                       cwd=None,
                       shell=False,
                       env=None,
                       check_status=True):
  """Executes a subprocess and continuously yields lines from its output.

  Args:
    args: List of arguments to the program, the program to execute is the first
      element.
    iter_timeout: Timeout for each iteration, in seconds.
    timeout: Timeout for the entire command, in seconds.
    cwd: If not None, the subprocess's current directory will be changed to
      |cwd| before it's executed.
    shell: Whether to execute args as a shell command. Must be True if args
      is a string and False if args is a sequence.
    env: If not None, a mapping that defines environment variables for the
      subprocess.
    check_status: A boolean indicating whether to check the exit status of the
      process after all output has been read.
  Yields:
    The output of the subprocess, line by line.

  Raises:
    CalledProcessError if check_status is True and the process exited with a
      non-zero exit status.
  """
  cmd = _ValidateAndLogCommand(args, cwd, shell)
  process = Popen(
      args,
      cwd=cwd,
      shell=shell,
      env=env,
      stdout=subprocess.PIPE,
      stderr=subprocess.STDOUT)
  return _IterCmdOutputLines(
      process,
      cmd,
      iter_timeout=iter_timeout,
      timeout=timeout,
      check_status=check_status)


def _IterCmdOutputLines(process,
                        cmd,
                        iter_timeout=None,
                        timeout=None,
                        check_status=True):
  buffer_output = ''

  iter_end = None
  cur_iter_timeout = None
  if iter_timeout:
    iter_end = time.time() + iter_timeout
    cur_iter_timeout = iter_timeout

  for data in _IterProcessStdout(
      process, iter_timeout=cur_iter_timeout, timeout=timeout):
    if iter_timeout:
      # Check whether the current iteration has timed out.
      cur_iter_timeout = iter_end - time.time()
      if data is None or cur_iter_timeout < 0:
        yield None
        iter_end = time.time() + iter_timeout
        continue
    else:
      assert data is not None, (
          'Iteration received no data despite no iter_timeout being set. '
          'cmd: %s' % cmd)

    # Construct lines to yield from raw data.
    buffer_output += data
    has_incomplete_line = buffer_output[-1] not in '\r\n'
    lines = buffer_output.splitlines()
    buffer_output = lines.pop() if has_incomplete_line else ''
    for line in lines:
      yield line
      if iter_timeout:
        iter_end = time.time() + iter_timeout

  if buffer_output:
    yield buffer_output
  if check_status and process.returncode:
    raise subprocess.CalledProcessError(process.returncode, cmd)
