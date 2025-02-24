# Copyright 2022 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import os
import sys

import pexpect # pylint: disable=import-error

from py_utils import retry_util
from telemetry.core import cast_interface
from telemetry.internal.backends.chrome import cast_browser_backend
from telemetry.internal.backends.chrome import minidump_finder

# _CAST_DEPLOY_PATH is relative to _CAST_ROOT
_CAST_DEPLOY_PATH = 'data/debug/google'
_CAST_ROOT = '/apps/castshell'
_CWR_ZIP = 'core_runtime_package.zip'


class RemoteCastBrowserBackend(cast_browser_backend.CastBrowserBackend):
  def __init__(self, cast_platform_backend, browser_options,
               browser_directory, profile_directory, casting_tab):
    super().__init__(
        cast_platform_backend,
        browser_options=browser_options,
        browser_directory=browser_directory,
        profile_directory=profile_directory,
        casting_tab=casting_tab)
    self._ip_addr = cast_platform_backend.ip_addr

  def _ReadReceiverName(self):
    return self._receiver_name

  def _SendCommand(self, ssh, command, prompt=None):
    """Uses ssh session to send command.

    If one of the expected prompts is output, return the prompt. If not, a
    ExceptionPexpect exception is thrown."""

    prompt = prompt or []
    prompt.extend([ssh.PROMPT, 'bash-.*[$#]'])
    ssh.sendline(command)
    return ssh.expect(prompt)

  def _StopNativeCast(self):
    ssh = self._platform_backend.GetSSHSession()
    self._SendCommand(ssh, 'systemctl stop castshell')
    self._SendCommand(ssh, 'killall core_runtime_bin')

  def _StopSDKCast(self):
    ssh = self._platform_backend.GetSSHSession()
    self._SendCommand(ssh, 'killall core_runtime')
    self._SendCommand(ssh, 'killall cast_core_platform_app')
    self._SendCommand(ssh, 'killall cast_sample_app')

  def _StopConjure(self):
    ssh = self._platform_backend.GetSSHSession()
    conjure_block = '/tmp/conjure_block.sh'

    # Put temporary script on top of the Conjure start script and kill process:
    if not self._CheckExistenceOnDevice(ssh, conjure_block):
      self._SendCommand(
          ssh,
          r'echo -e "#\!/bin/sh\nwhile true ; do sleep 5 ; done" >%s' %
          conjure_block)
    self._SendCommand(
        ssh,
        'mount -o bind %s /vendor/cast_root/application/chromium/conjure.sh' %
        conjure_block)
    self._SendCommand(ssh, 'killall conjure')

  def _StopCast(self):
    self._StopNativeCast()
    self._StopSDKCast()
    self._StopConjure()

  def _GetCastDirSSHSession(self, overwrite=False):
    """
    Get ssh session with _CAST_ROOT as root and _CAST_DEPLOY_PATH as cwd.

    Args:
      overwrite: if set to True, remove and recreate _CAST_DEPLOY_PATH.
    """
    ssh = self._platform_backend.GetSSHSession()
    self._SendCommand(ssh, 'su', ['Password:'])
    self._SendCommand(ssh, cast_interface.SSH_PWD)
    self._SendCommand(ssh, 'setenforce 0')
    self._SendCommand(ssh, 'chroot %s' % _CAST_ROOT)
    if overwrite:
      self._SendCommand(ssh, 'rm -rf %s' % _CAST_DEPLOY_PATH)
    self._SendCommand(ssh, 'mkdir -p %s' % _CAST_DEPLOY_PATH)
    self._SendCommand(ssh, 'cd %s' % _CAST_DEPLOY_PATH)
    return ssh

  def _ScpToDevice(self, source, dest):
    scp_opts = [
        '-o UserKnownHostsFile=/dev/null', '-o ConnectTimeout=30',
        '-o ServerAliveInterval=2', '-o ServerAliveCountMax=2',
        '-o StrictHostKeyChecking=no'
    ]
    scp_cmd = 'scp %s %s %s' % (
        ' '.join(scp_opts),
        source,
        '%s@%s:%s' % (cast_interface.SSH_USER, self._ip_addr, dest),
    )
    pexpect.run(
        scp_cmd,
        events={'(?i)password:': '%s\n' % cast_interface.SSH_PWD},
        timeout=300,
        logfile=sys.stdout.buffer)

  def _InstallCastCore(self):
    ssh = self._GetCastDirSSHSession(overwrite=True)
    cast_core_tgz = 'sdk_runtime_vizio_castos_armv7a.tgz'
    self._ScpToDevice(os.path.join(self._output_dir, cast_core_tgz),
                      os.path.join(_CAST_ROOT, _CAST_DEPLOY_PATH))

    # gzip command requires a separate ssh session.
    gz_ssh = self._platform_backend.GetSSHSession()
    self._SendCommand(gz_ssh, 'cd %s' % os.path.join(_CAST_ROOT,
                                                     _CAST_DEPLOY_PATH))
    self._SendCommand(gz_ssh, 'gzip -d %s' % cast_core_tgz)

    cast_core_tar = cast_core_tgz.replace('tgz', 'tar')
    self._SendCommand(ssh, 'tar xf %s' % cast_core_tar)
    self._SendCommand(ssh, 'rm -rf %s' % cast_core_tar)
    self._SendCommand(ssh, 'chmod 0755 cast_core/bin run_cast.sh')

  def _CheckExistenceOnDevice(self, ssh, dest):
    """Check file/dir exists on device."""
    resp = self._SendCommand(
        ssh,
        '[[ -a %s ]]; echo result_$?' % dest,
        ['result_0', 'result_1'])
    if resp == 0:
      return True
    return False

  def _InstallCastWebRuntime(self):
    ssh = self._platform_backend.GetSSHSession()
    deploy_path = os.path.join(_CAST_ROOT, _CAST_DEPLOY_PATH)
    self._SendCommand(ssh, 'cd %s && umask 0022' % deploy_path)
    if self._CheckExistenceOnDevice(ssh, 'cast_runtime'):
      self._SendCommand(ssh, 'rm -rf cast_runtime')
    self._SendCommand(ssh, 'mkdir cast_runtime && chmod 0755 cast_runtime')
    self._ScpToDevice(
        os.path.join(self._runtime_exe, _CWR_ZIP),
        os.path.join(deploy_path, 'cast_runtime'))
    self._SendCommand(
        ssh,
        'cd cast_runtime && unzip %(cwr)s && rm %(cwr)s' %
        {'cwr': _CWR_ZIP})
    self._SendCommand(
        ssh,
        'find . -type f | xargs chmod 0644 && ' \
        'chmod 0755 core_runtime dumpstate lib/*')

  @retry_util.RetryOnException(pexpect.exceptions.TIMEOUT, retries=3)
  def _SetReceiverName(self, env_var, retries=None):
    """Sets the receiver name to Cast[self._ip_addr]"""
    del retries # Handled by decorator.
    rename_ssh = self._GetCastDirSSHSession()
    rename_command = env_var + ['./cast_core/bin/cast_control_cli']
    self._SendCommand(rename_ssh, ' '.join(rename_command),
                      ['.*Cast control is connected.*'])
    self._receiver_name = 'Cast%s' % self._ip_addr
    rename_ssh.sendline('n %s' % self._receiver_name)

  def Start(self, startup_args):
    self._StopCast()
    self._InstallCastCore()
    self._InstallCastWebRuntime()

    self._dump_finder = minidump_finder.MinidumpFinder(
        self.browser.platform.GetOSName(),
        self.browser.platform.GetArchName())
    self._cast_core_process = self._GetCastDirSSHSession()
    env_var = [
        'LD_LIBRARY_PATH=/system/lib:/lib',
        'CAST_OEM_ROOT=__empty__'
        'CAST_HOME=%s' % os.path.join('/', _CAST_DEPLOY_PATH, 'cast_home'),
    ]
    cast_core_command = env_var + [
        './run_cast.sh', 'core',
        '--remote-debugging-port=%d' % cast_browser_backend.DEVTOOLS_PORT,
    ]
    self._cast_core_process.sendline(' '.join(cast_core_command))

    self._browser_process = self._GetCastDirSSHSession()
    runtime_command = env_var + ['./run_cast.sh', 'rt']
    self._browser_process.sendline(' '.join(runtime_command))
    self._discovery_mode = True
    self._SetReceiverName(env_var)
    self._WaitForSink()

  def GetPid(self):
    return self._browser_process.pid

  def Background(self):
    raise NotImplementedError

  def Close(self):
    self._StopSDKCast()
    super().Close()
