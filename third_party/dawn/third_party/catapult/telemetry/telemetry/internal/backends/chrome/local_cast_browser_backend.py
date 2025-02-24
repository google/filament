# Copyright 2022 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import json
import os
import subprocess
import tempfile

from telemetry.internal.backends.chrome import cast_browser_backend
from telemetry.internal.backends.chrome import minidump_finder


_RUNTIME_CONFIG_TEMPLATE = """
{{
  "grpc": {{
   "cast_core_service_endpoint": "unix:/tmp/cast/grpc/core-service",
   "platform_service_endpoint": "unix:/tmp/cast/grpc/platform-service"
 }},
 "runtimes": [
   {{
     "name": "Cast Web Runtime",
     "type": "CAST_WEB",
     "executable": "{runtime_dir}",
     "args": [
       "--no-sandbox",
       "--no-wifi",
       "--runtime-service-path=%runtime_endpoint%",
       "--cast-core-runtime-id=%runtime_id%",
       "--allow-running-insecure-content",
       "--minidump-path=/tmp/cast/minidumps",
       "--disable-audio-output",
       "--ozone-platform=x11"
     ],
     "capabilities": {{
       "video_supported": true,
       "audio_supported": true,
       "metrics_recorder_supported": true,
       "applications": {{
         "supported": [],
         "unsupported": [
           "CA5E8412",
           "85CDB22F", "8E6C866D"
         ]
       }}
     }}
   }}
 ]
}}
"""

CAST_CORE_CONFIG_PATH = os.path.join(
    os.getenv("HOME"), '.config', 'cast_shell', '.eureka.conf')

class CastRuntime():
  def __init__(self, root_dir, runtime_dir, log_file):
    self._root_dir = root_dir
    self._runtime_dir = runtime_dir
    self._log_file = log_file
    self._runtime_process = None
    self._app_exe = os.path.join(root_dir, 'blaze-bin', 'third_party',
                                 'castlite', 'public', 'sdk', 'samples',
                                 'platform_app', 'platform_app')
    self._config_file = None

  def Start(self):
    self._config_file = tempfile.NamedTemporaryFile('w+')
    self._config_file.write(
        _RUNTIME_CONFIG_TEMPLATE.format(runtime_dir=self._runtime_dir))
    self._config_file.flush()

    runtime_command = [
        self._app_exe,
        '--config', self._config_file.name
    ]
    with open(os.devnull) as devnull, open(self._log_file, 'w') as log_file:
      self._runtime_process = subprocess.Popen(runtime_command,
                                               stdin=devnull,
                                               stdout=log_file,
                                               stderr=subprocess.STDOUT)
    return self._runtime_process

  def Close(self):
    if self._runtime_process:
      self._runtime_process.kill()
      self._runtime_process = None
    if self._config_file:
      self._config_file.close()
      self._config_file = None


class LocalCastBrowserBackend(cast_browser_backend.CastBrowserBackend):
  def __init__(self, cast_platform_backend, browser_options,
               browser_directory, profile_directory, casting_tab):
    super().__init__(
        cast_platform_backend,
        browser_options=browser_options,
        browser_directory=browser_directory,
        profile_directory=profile_directory,
        casting_tab=casting_tab)
    self._web_runtime = CastRuntime(self._output_dir,
                                    self._runtime_exe,
                                    self._runtime_log_file)

  def _ReadReceiverName(self):
    if not self._receiver_name:
      with open(CAST_CORE_CONFIG_PATH) as f:
        self._receiver_name = json.load(f)['eureka-name']
    return self._receiver_name

  def Start(self, startup_args):
    self._dump_finder = minidump_finder.MinidumpFinder(
        self.browser.platform.GetOSName(),
        self.browser.platform.GetArchName())
    cast_core_command = [
        os.path.join(self._output_dir, 'blaze-bin', 'third_party', 'castlite',
                     'public', 'sdk', 'core', 'samples', 'cast_core'),
        '--force_all_apps_discoverable',
        '--remote-debugging-port=%d' % cast_browser_backend.DEVTOOLS_PORT,
    ]
    original_dir = os.getcwd()
    try:
      os.chdir(self._output_dir)
      with open(os.devnull) as devnull, \
        open(self._cast_core_log_file, 'w') as log_file:
        self._cast_core_process = subprocess.Popen(cast_core_command,
                                                   stdin=devnull,
                                                   stdout=log_file,
                                                   stderr=subprocess.STDOUT)
      self._browser_process = self._web_runtime.Start()
    finally:
      os.chdir(original_dir)
    self._discovery_mode = True
    self._WaitForSink()

  def Background(self):
    raise NotImplementedError

  def Close(self):
    self._web_runtime.Close()
    if self._cast_core_process:
      self._cast_core_process.kill()
      self._cast_core_process = None
    super().Close()
