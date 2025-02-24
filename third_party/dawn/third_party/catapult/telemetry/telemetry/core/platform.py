# Copyright 2012 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
from __future__ import division
from __future__ import absolute_import
import logging as real_logging
import os
import subprocess
import time
import six

from telemetry.core import local_server
from telemetry.core import memory_cache_http_server
from telemetry.core import network_controller
from telemetry.core import tracing_controller
from telemetry.core import util
from telemetry.internal.platform import (platform_backend as
                                         platform_backend_module)

from py_utils import discover

_HOST_PLATFORM = None
# Remote platform is a dictionary from device ids to remote platform instances.
_REMOTE_PLATFORMS = {}


def _InitHostPlatformIfNeeded():
  global _HOST_PLATFORM # pylint: disable=global-statement
  if _HOST_PLATFORM:
    return
  backend = None
  backends = _IterAllPlatformBackendClasses()
  for platform_backend_class in backends:
    if platform_backend_class.IsPlatformBackendForHost():
      backend = platform_backend_class()
      break
  if not backend:
    raise NotImplementedError()
  _HOST_PLATFORM = Platform(backend)


def GetHostPlatform():
  _InitHostPlatformIfNeeded()
  return _HOST_PLATFORM


def _IterAllPlatformBackendClasses():
  platform_dir = os.path.dirname(os.path.realpath(
      platform_backend_module.__file__))
  return six.itervalues(discover.DiscoverClasses(
      platform_dir, util.GetTelemetryDir(),
      platform_backend_module.PlatformBackend))


def GetPlatformForDevice(device, finder_options, logging=real_logging):
  """ Returns a platform instance for the device.
    Args:
      device: a device.Device instance.
  """
  if device.guid in _REMOTE_PLATFORMS:
    return _REMOTE_PLATFORMS[device.guid]
  try:
    for platform_backend_class in _IterAllPlatformBackendClasses():
      if platform_backend_class.SupportsDevice(device):
        _REMOTE_PLATFORMS[device.guid] = (
            platform_backend_class.CreatePlatformForDevice(device,
                                                           finder_options))
        return _REMOTE_PLATFORMS[device.guid]
    return None
  except Exception: # pylint: disable=broad-except
    logging.error('Fail to create platform instance for %s.', device.name)
    raise


class Platform():
  """The platform that the target browser is running on.

  Provides a limited interface to interact with the platform itself, where
  possible. It's important to note that platforms may not provide a specific
  API, so check with IsFooBar() for availability.
  """

  def __init__(self, platform_backend):
    self._platform_backend = platform_backend
    self._platform_backend.InitPlatformBackend()
    self._platform_backend.SetPlatform(self)
    self._network_controller = network_controller.NetworkController(
        self._platform_backend.network_controller_backend)
    self._tracing_controller = tracing_controller.TracingController(
        self._platform_backend.tracing_controller_backend)
    self._local_server_controller = local_server.LocalServerController(
        self._platform_backend)
    self._forwarder = None

  @property
  def is_host_platform(self):
    return self == GetHostPlatform()

  @property
  def network_controller(self):
    """Control network settings and servers to simulate the Web."""
    return self._network_controller

  @property
  def tracing_controller(self):
    return self._tracing_controller

  def Initialize(self):
    pass

  def CanMonitorThermalThrottling(self):
    """Platforms may be able to detect thermal throttling.

    Some fan-less computers go into a reduced performance mode when their heat
    exceeds a certain threshold. Performance tests in particular should use this
    API to detect if this has happened and interpret results accordingly.
    """
    return self._platform_backend.CanMonitorThermalThrottling()

  def GetSystemLog(self):
    return self._platform_backend.GetSystemLog()

  def IsThermallyThrottled(self):
    """Returns True if the device is currently thermally throttled."""
    return self._platform_backend.IsThermallyThrottled()

  def HasBeenThermallyThrottled(self):
    """Returns True if the device has been thermally throttled."""
    return self._platform_backend.HasBeenThermallyThrottled()

  def GetDeviceTypeName(self):
    """Returns a string description of the Platform device, or None.

    Examples: Nexus 7, Nexus 6, Desktop"""
    return self._platform_backend.GetDeviceTypeName()

  def GetArchName(self):
    """Returns a string description of the Platform architecture.

    Examples: x86_64 (posix), AMD64 (win), armeabi-v7a, x86"""
    return self._platform_backend.GetArchName()

  def GetOSName(self):
    """Returns a string description of the Platform OS.

    Examples: WIN, MAC, LINUX, CHROMEOS"""
    return self._platform_backend.GetOSName()

  def GetDeviceId(self):
    """Returns a string identifying the device.

    Examples: 0123456789abcdef"""
    return self._platform_backend.GetDeviceId()

  def GetOSVersionName(self):
    """Returns a logically sortable, string-like description of the Platform OS
    version.

    Examples: VISTA, WIN7, LION, MOUNTAINLION"""
    return self._platform_backend.GetOSVersionName()

  def GetOSVersionDetailString(self):
    """Returns more detailed information about the OS version than
    GetOSVersionName, if available. Otherwise returns the empty string.

    Examples: '10.12.4' on macOS."""
    return self._platform_backend.GetOSVersionDetailString()

  def GetSystemTotalPhysicalMemory(self):
    """Returns an integer with the total physical memory in bytes."""
    return self._platform_backend.GetSystemTotalPhysicalMemory()

  def CanFlushIndividualFilesFromSystemCache(self):
    """Returns true if the disk cache can be flushed for individual files."""
    return self._platform_backend.CanFlushIndividualFilesFromSystemCache()

  def SupportFlushEntireSystemCache(self):
    """Returns true if entire system cache can be flushed.

    Also checks that platform has required privilegues to flush system caches.
    """
    return self._platform_backend.SupportFlushEntireSystemCache()

  def _WaitForPageCacheToBeDropped(self):
    # There seems to be no reliable way to wait for all pages to be dropped from
    # the OS page cache (also known as 'file cache'). There is no guaranteed
    # moment in time when everything is out of page cache. A number of pages
    # will likely be reused before other pages are evicted. While individual
    # files can be watched in limited ways, we choose not to be clever.
    time.sleep(2)

  def FlushEntireSystemCache(self):
    """Flushes the OS's file cache completely.

    This function may require root or administrator access. Clients should
    call SupportFlushEntireSystemCache to check first.
    """
    self._platform_backend.FlushEntireSystemCache()
    self._WaitForPageCacheToBeDropped()

  def FlushSystemCacheForDirectories(self, directories):
    """Flushes the OS's file cache for the specified directory.

    This function does not require root or administrator access."""
    for path in directories:
      self._platform_backend.FlushSystemCacheForDirectory(path)
    self._WaitForPageCacheToBeDropped()

  def FlushDnsCache(self):
    """Flushes the OS's DNS cache completely.

    This function may require root or administrator access."""
    return self._platform_backend.FlushDnsCache()

  def RestartTsProxyServerOnRemotePlatforms(self):
    """Restarts the TsProxyServer on remote platforms.

    If something goes wrong with the connection to the remote device (SSH, adb,
    etc.), then the forwarder between the device and the host will potentially
    break, breaking all further network connectivity. So, restart the server
    and its forwarder.
    """
    self._platform_backend.RestartTsProxyServerOnRemotePlatforms()

  def LaunchApplication(self,
                        application,
                        parameters=None,
                        elevate_privilege=False):
    """"Launches the given |application| with a list of |parameters| on the OS.

    Set |elevate_privilege| to launch the application with root or admin rights.

    Returns:
      A popen style process handle for host platforms.
    """
    return self._platform_backend.LaunchApplication(
        application,
        parameters,
        elevate_privilege=elevate_privilege)

  def StartActivity(self, intent, blocking=False):
    """Starts an activity for the given intent on the device."""
    return self._platform_backend.StartActivity(intent, blocking)

  def CanLaunchApplication(self, application):
    """Returns whether the platform can launch the given application."""
    return self._platform_backend.CanLaunchApplication(application)

  def InstallApplication(self, application, **kwargs):
    """Installs the given application."""
    return self._platform_backend.InstallApplication(application, **kwargs)

  def IsCooperativeShutdownSupported(self):
    """Indicates whether CooperativelyShutdown, below, is supported.
    It is not necessary to implement it on all platforms."""
    return self._platform_backend.IsCooperativeShutdownSupported()

  def CooperativelyShutdown(self, proc, app_name):
    """Cooperatively shut down the given process from subprocess.Popen.

    Currently this is only implemented on Windows. See
    crbug.com/424024 for background on why it was added.

    Args:
      proc: a process object returned from subprocess.Popen.
      app_name: on Windows, is the prefix of the application's window
          class name that should be searched for. This helps ensure
          that only the application's windows are closed.

    Returns True if it is believed the attempt succeeded.
    """
    return self._platform_backend.CooperativelyShutdown(proc, app_name)

  def CanTakeScreenshot(self):
    return self._platform_backend.CanTakeScreenshot()

  # TODO(crbug.com/369490): Implement this on Mac, Linux & Win.
  def TakeScreenshot(self, file_path):
    """ Takes a screenshot of the platform and save to |file_path|.

    Note that this method may not be supported on all platform, so check with
    CanTakeScreenshot before calling this.

    Args:
      file_path: Where to save the screenshot to. If the platform is remote,
        |file_path| is the path on the host platform.

    Returns True if it is believed the attempt succeeded.
    """
    return self._platform_backend.TakeScreenshot(file_path)

  def CanRecordVideo(self):
    return self._platform_backend.CanRecordVideo()

  def StartVideoRecording(self):
    """Starts recording a video on the device.

    Note that this method may not be supported on all platforms, so the caller
    must check with CanRecordVideo before calling this. Once the caller starts
    recording a video using this call, the caller must stop recording the video
    by calling StopVideoRecording() before attempting to start recording another
    video.
    """
    self._platform_backend.StartVideoRecording()

  def StopVideoRecording(self, video_path):
    """Stops recording a video on the device and saves to |video_path|.

    This method must be called only if recording a video had started using a
    call to StartVideoRecording(), and it was not already stopped using a call
    to StopVideoRecording().

    Args:
      video_path: Where to save the video to. If the platform is remote,
        |video_path| is the path on the host platform.
    """
    self._platform_backend.StopVideoRecording(video_path)

  def SetPerformanceMode(self, performance_mode):
    """ Set the performance mode on the platform.

    Note: this can be no-op on certain platforms.
    """
    return self._platform_backend.SetPerformanceMode(performance_mode)

  def StartLocalServer(self, server):
    """Starts a LocalServer and associates it with this platform.
    |server.Close()| should be called manually to close the started server.
    """
    self._local_server_controller.StartServer(server)

  @property
  def http_server(self):
    # TODO(crbug.com/799490): Ownership of the local server should be moved
    # to the network_controller.
    server = self._local_server_controller.GetRunningServer(
        memory_cache_http_server.MemoryCacheDynamicHTTPServer, None)
    if server:
      return server

    return self._local_server_controller.GetRunningServer(
        memory_cache_http_server.MemoryCacheHTTPServer, None)

  def SetHTTPServerDirectories(self, paths, handler_class=None):
    """Returns True if the HTTP server was started, False otherwise."""
    if isinstance(paths, six.string_types):
      paths = {paths}
    paths = set(os.path.realpath(p) for p in paths)

    # If any path is in a subdirectory of another, remove the subdirectory.
    duplicates = set()
    for parent_path in paths:
      for sub_path in paths:
        if parent_path == sub_path:
          continue
        if os.path.commonprefix((parent_path, sub_path)) == parent_path:
          duplicates.add(sub_path)
    paths -= duplicates

    if self.http_server:
      old_handler_class = getattr(self.http_server,
                                  "dynamic_request_handler_class", None)
      if not old_handler_class and not handler_class and \
          self.http_server.paths == paths:
        return False

      if old_handler_class and handler_class \
          and old_handler_class.__name__ == handler_class.__name__ \
          and self.http_server.paths == paths:
        return False

      self.http_server.Close()

    if not paths:
      return False

    if handler_class:
      server = memory_cache_http_server.MemoryCacheDynamicHTTPServer(
          paths, handler_class)
      real_logging.info('MemoryCacheDynamicHTTPServer created')
    else:
      server = memory_cache_http_server.MemoryCacheHTTPServer(paths)
      real_logging.info('MemoryCacheHTTPServer created')

    self.StartLocalServer(server)
    return True

  def StopAllLocalServers(self):
    self._local_server_controller.Close()
    if self._forwarder:
      self._forwarder.Close()

  @property
  def local_servers(self):
    """Returns the currently running local servers."""
    return self._local_server_controller.local_servers

  def WaitForBatteryTemperature(self, temp):
    """Waits for the battery on the device under test to cool down to temp.

    Args:
      temp: temperature target in degrees C.
    """
    return self._platform_backend.WaitForBatteryTemperature(temp)

  def WaitForCpuTemperature(self, temp):
    """Waits for the CPU temperature to be less than temp.

    Args:
      temp: A float containing the maximum temperature to allow
      in degrees c.
    """
    return self._platform_backend.WaitForCpuTemperature(temp)

  def GetTypExpectationsTags(self):
    return self._platform_backend.GetTypExpectationsTags()

  def SupportsIntelPowerGadget(self):
    """Check if Intel Power Gadget is supported.

    Intel Power Gadget is supported on Intel based desktop platforms. Currently
    this function only checks if Intel Power Gadget is installed on the device.
    The assumption is if it is installed, it can run successfully.
    """
    return self._platform_backend.SupportsIntelPowerGadget()

  def RunIntelPowerGadget(self, duration, output_path):
    """Runs Intel Power Gadget and collects power data for a period of time.

    Note that Intel Power Gadget will stop after |duration|, output the
    collected data, and then exit. However, this function returns right away
    after Intel Power Gadget is launched and starts collection power data.

    Also, makes sure SupportsIntelPowerGadget() returns True before calling
    this function.

    Args:
      duration: Seconds that Intel Power Gadget runs and collects power data.
      output_path: Specifies to which file Intel Power Gadget writes the
          collected data.

    Returns True if Intel Power Gadget starts successfully.
    """
    ipg_path = self._platform_backend.GetIntelPowerGadgetPath()
    if not ipg_path:
      real_logging.error('Fail to locate Intel Power Gadget. Please call '
                         'SupportsIntelPowerGadget() before calling this.')
      return False
    command = '"%s" -duration %d -file "%s"' % (ipg_path, duration, output_path)
    subprocess.Popen(command, shell=True, stderr=subprocess.STDOUT)
    return True

  def CollectIntelPowerGadgetResults(self, output_path, skip_duration=0,
                                     sample_duration=0):
    """Processes power data output from Intel Power Gadget.

    Note that the file |output_path| is removed when this function returns.

    Args:
      output_path: Specifies to which file Intel Power Gadget wrote the
          collected data.
      skip_duration: Seconds of data to skip in the beginning of the collected
          data, assuming there are extra noises in the beginning.
      sample_duration: Seconds of data to process and return after the
          |skip_duration|. If this is 0, process data to the end.

    Returns:
      A dictionary in the format of {measurement_name: average_value}.
      Specifically, 'samples' indicates the number of samples that contributed
      to the returned data.
      If an error occurs, an empty disctionary is returned.
    """
    if not os.path.isfile(output_path):
      real_logging.error('Cannot locate output file at ' + output_path)
      return {}

    first_line = True
    samples = 0
    cols = 0
    indices = []
    labels = []
    sums = []
    col_time = None
    for line in open(output_path):
      tokens = [token.strip('" ') for token in line.split(',')]
      if first_line:
        first_line = False
        cols = len(tokens)
        for ii in range(0, cols):
          token = tokens[ii]
          if token.startswith('Elapsed Time'):
            col_time = ii
          elif token.endswith('(Watt)'):
            indices.append(ii)
            labels.append(token[:-len('(Watt)')])
            sums.append(0.0)
        assert col_time
        assert cols > 0
        assert len(indices) > 0
        continue
      if len(tokens) != cols:
        continue
      if skip_duration > 0 and float(tokens[col_time]) < skip_duration:
        continue
      if (sample_duration > 0 and
          float(tokens[col_time]) > skip_duration + sample_duration):
        break
      samples += 1
      for ii, index in enumerate(indices):
        sums[ii] += float(tokens[index])
    results = {'samples': samples}
    if samples > 0:
      for ii in range(0, len(indices)):
        #2To3-division: this line is unchanged as sums[] are floats.
        results[labels[ii]] = sums[ii] / samples

    os.remove(output_path)
    return results
