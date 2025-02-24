# Copyright 2015 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

"""Provides fakes for several of Telemetry's internal objects.

These allow code like story_runner and Benchmark to be run and tested
without compiling or starting a browser. Class names prepended with an
underscore are intended to be implementation details, and should not
be subclassed; however, some, like FakeBrowser, have public APIs that
may need to be called in tests.
"""
from __future__ import absolute_import
import types

from telemetry.core import debug_data
from telemetry.core import exceptions
from telemetry.internal.backends.chrome_inspector import inspector_websocket
from telemetry.internal.backends.chrome_inspector import websocket
from telemetry.internal.browser import browser_options as browser_options_module
from telemetry.internal.platform import system_info
from telemetry.page import shared_page_state
from telemetry.util import image_util
from telemetry.util import wpr_modes
from telemetry.testing import test_utils
from telemetry.testing.internal import fake_gpu_info


# Classes and functions which are intended to be part of the public
# fakes API.

class FakePlatformBackend():
  def __init__(self, os_name):
    self._os_name = os_name

  def GetOSName(self):
    return self._os_name


class FakePlatform():
  def __init__(self, os_name='', os_version_name='', arch_name=''):
    self._network_controller = None
    self._tracing_controller = None
    self._arch_name = arch_name or 'FakeArchitecture'
    self._os_name = os_name or 'FakeOS'
    self._os_version_name = os_version_name or 'FakeVersion'
    self._device_type_name = 'abc'
    self._is_svelte = False
    self._is_aosp = True
    self._get_os_version_detail_string = 'OsVersionString'
    self._platform_backend = FakePlatformBackend('FakeOS')

  @property
  def is_host_platform(self):
    raise NotImplementedError

  @property
  def network_controller(self):
    if self._network_controller is None:
      self._network_controller = _FakeNetworkController()
    return  self._network_controller

  @property
  def tracing_controller(self):
    if self._tracing_controller is None:
      self._tracing_controller = _FakeTracingController()
    return  self._tracing_controller

  def Initialize(self):
    pass

  def FlushDnsCache(self):
    pass

  def SetPerformanceMode(self, mode):
    pass

  def CanMonitorThermalThrottling(self):
    return False

  def IsThermallyThrottled(self):
    return False

  def HasBeenThermallyThrottled(self):
    return False

  def GetArchName(self):
    return self._arch_name

  def SetOSName(self, name):
    self._os_name = name

  def GetOSVersionName(self):
    return self._os_version_name

  def SetOSVersionName(self, os_version_name):
    self._os_version_name = os_version_name

  def GetOSName(self):
    return self._os_name

  def GetDeviceId(self):
    return None

  def GetSystemLog(self):
    return None

  def RestartTsProxyServerOnRemotePlatforms(self):
    pass

  def StopAllLocalServers(self):
    pass

  def WaitForBatteryTemperature(self, _):
    pass

  # TODO(rnephew): Investigate moving from setters to @property.
  def SetDeviceTypeName(self, name):
    self._device_type_name = name

  def GetDeviceTypeName(self):
    return self._device_type_name

  def SetIsSvelte(self, b):
    assert isinstance(b, bool)
    self._is_svelte = b

  def IsSvelte(self):
    if self._os_name != 'android':
      raise NotImplementedError
    return self._is_svelte

  def SetIsAosp(self, b):
    assert isinstance(b, bool)
    self._is_aosp = b

  def IsAosp(self):
    return self._is_aosp and self._os_name == 'android'

  def SetOsVersionDetailString(self, v):
    self._get_os_version_detail_string = v

  def GetOSVersionDetailString(self):
    return self._get_os_version_detail_string

  def GetTypExpectationsTags(self):
    return test_utils.sanitizeTypExpectationsTags(
        [self.GetOSName(), self.GetOSVersionName()])


class FakeLinuxPlatform(FakePlatform):
  def __init__(self):
    super().__init__()
    self.screenshot_png_data = None
    self.http_server_directories = []
    self.http_server = FakeHTTPServer()

  @property
  def is_host_platform(self):
    return True

  def GetDeviceTypeName(self):
    return 'Desktop'

  def GetArchName(self):
    return 'x86_64'

  def GetSystemTotalPhysicalMemory(self):
    return 8 * (1024 ** 3)

  def GetOSName(self):
    return 'linux'

  def GetOSVersionName(self):
    return 'trusty'

  def GetOSVersionDetailString(self):
    return ''

  def CanTakeScreenshot(self):
    return bool(self.screenshot_png_data)

  def TakeScreenshot(self, file_path):
    if not self.CanTakeScreenshot():
      raise NotImplementedError
    img = image_util.FromBase64Png(self.screenshot_png_data)
    image_util.WritePngFile(img, file_path)
    return True

  def SetHTTPServerDirectories(self, paths):
    self.http_server_directories.append(paths)


class FakeHTTPServer():
  def UrlOf(self, url):
    del url  # unused
    return 'file:///foo'


class FakeForwarder():
  def Close(self):
    pass


class FakeForwarderFactory():
  def __init__(self):
    self.raise_exception_on_create = False
    self.host_ip = '127.0.0.1'

  def Create(self, local_port, remote_port, reverse=False):
    del local_port  # Unused.
    del remote_port  # Unused.
    del reverse  # Unused.
    if self.raise_exception_on_create:
      raise exceptions.IntentionalException
    return FakeForwarder()


class FakePossibleBrowser():
  def __init__(self, execute_on_startup=None,
               execute_after_browser_creation=None,
               arch_name='', os_name='', os_version_name='', browser_type=''):
    if os_name:
      self._returned_browser = FakeBrowser(
          FakePlatform(os_name, os_version_name, arch_name), browser_type)
    else:
      self._returned_browser = FakeBrowser(
          FakeLinuxPlatform(), browser_type)
    self.browser_type = browser_type or 'linux'
    self.supports_tab_control = False
    self.execute_on_startup = execute_on_startup
    self.execute_after_browser_creation = execute_after_browser_creation
    self.browser_options = None  # This is set in SetUpEnvironment.
    self.target_os = os_name

  @property
  def returned_browser(self):
    """The browser object that will be returned through later API calls."""
    return self._returned_browser

  def Create(self):
    if self.execute_on_startup is not None:
      self.execute_on_startup()
    if self.execute_after_browser_creation is not None:
      self.execute_after_browser_creation(self._returned_browser)
    return self.returned_browser

  def SetUpEnvironment(self, browser_options):
    self.browser_options = browser_options

  def CleanUpEnvironment(self):
    self.browser_options = None

  @property
  def platform(self):
    """The platform object from the returned browser.

    To change this or set it up, change the returned browser's
    platform.
    """
    return self.returned_browser.platform

  def BrowserSession(self, options):
    del options
    return self.returned_browser

  def GetTypExpectationsTags(self):
    tags = self.platform.GetTypExpectationsTags()
    return tags + test_utils.sanitizeTypExpectationsTags([self.browser_type])


class FakeSharedPageState(shared_page_state.SharedPageState):
  def _GetPossibleBrowser(self):
    p = FakePossibleBrowser()
    self.ConfigurePossibleBrowser(p)
    return p

  def ConfigurePossibleBrowser(self, possible_browser):
    """Override this to configure the PossibleBrowser.

    Can make changes to the browser's configuration here via e.g.:
       possible_browser.returned_browser.returned_system_info = ...
    """


  def DidRunStory(self, results):  # pylint:disable=useless-super-delegation
    # TODO(kbr): add a test which throws an exception from DidRunStory
    # to verify the fix from https://crrev.com/86984d5fc56ce00e7b37ebe .
    super().DidRunStory(results)


class FakeSystemInfo(system_info.SystemInfo):
  def __init__(self, model_name='', gpu_dict=None, command_line=''):
    if gpu_dict is None:
      gpu_dict = fake_gpu_info.FAKE_GPU_INFO
    super().__init__(model_name, gpu_dict, command_line)


class _FakeBrowserFinderOptions(browser_options_module.BrowserFinderOptions):
  def __init__(self, execute_on_startup,
               execute_after_browser_creation, *args, **kwargs):
    browser_options_module.BrowserFinderOptions.__init__(self, *args, **kwargs)
    self.fake_possible_browser = \
      FakePossibleBrowser(
          execute_on_startup=execute_on_startup,
          execute_after_browser_creation=execute_after_browser_creation)
    self.capture_screen_video = False

def CreateBrowserFinderOptions(browser_type=None, execute_on_startup=None,
                               execute_after_browser_creation=None):
  """Creates fake browser finder options for discovering a browser."""
  return _FakeBrowserFinderOptions(
      browser_type=browser_type,
      execute_on_startup=execute_on_startup,
      execute_after_browser_creation=execute_after_browser_creation)


class FakeApp():

  def __init__(self, platform=None):
    if not platform:
      self._platform = FakePlatform()
    else:
      self._platform = platform
    self.standard_output = ''
    self.recent_minidump_path = None

  @property
  def platform(self):
    return self._platform

  @platform.setter
  def platform(self, incoming):
    """Allows overriding of the fake browser's platform object."""
    assert isinstance(incoming, FakePlatform)
    self._platform = incoming

  def GetStandardOutput(self):
    return self.standard_output

  def GetMostRecentMinidumpPath(self):
    return self.recent_minidump_path

  def GetRecentMinidumpPathWithTimeout(self, timeout_s=15, oldest_ts=None):
    del timeout_s, oldest_ts
    return self.recent_minidump_path

  def CollectDebugData(self, log_level):
    del log_level
    return debug_data.DebugData()

# Internal classes. Note that end users may still need to both call
# and mock out methods of these classes, but they should not be
# subclassed.

class FakeBrowser(FakeApp):
  def __init__(self, platform, browser_type=''):
    super().__init__(platform)
    self._tabs = _FakeTabList(self)
    # Fake the creation of the first tab.
    self._tabs.New()
    self._returned_system_info = FakeSystemInfo()
    self._platform = platform
    self._browser_type = browser_type or 'release'
    self._is_crashed = False

  @property
  def returned_system_info(self):
    """The object which will be returned from calls to GetSystemInfo."""
    return self._returned_system_info

  @returned_system_info.setter
  def returned_system_info(self, incoming):
    """Allows overriding of the returned SystemInfo object.

    Incoming argument must be an instance of FakeSystemInfo."""
    assert isinstance(incoming, FakeSystemInfo)
    self._returned_system_info = incoming

  @property
  def browser_type(self):
    """The browser_type this browser claims to be ('debug', 'release', etc.)"""
    return self._browser_type

  @browser_type.setter
  def browser_type(self, incoming):
    """Allows setting of the browser_type."""
    self._browser_type = incoming

  def Close(self):
    self._is_crashed = False

  def GetSystemInfo(self):
    return self.returned_system_info

  @property
  def supports_tab_control(self):
    return True

  @property
  def tabs(self):
    return self._tabs

  @property
  def _platform_backend(self):
    return self._platform._platform_backend

  def DumpStateUponFailure(self):
    pass

  def CleanupUnsymbolizedMinidumps(self, fatal=False):
    del fatal

  def GetTypExpectationsTags(self):
    tags = self.platform.GetTypExpectationsTags()
    return tags + test_utils.sanitizeTypExpectationsTags([self.browser_type])

  def __enter__(self):
    return self

  def __exit__(self, *args):
    pass


class _FakeTracingController():
  def __init__(self):
    self._is_tracing = False

  def StartTracing(self, tracing_config, timeout=20):
    self._is_tracing = True
    del tracing_config
    del timeout

  def StopTracing(self):
    self._is_tracing = False

  @property
  def is_tracing_running(self):
    return self._is_tracing

  def ClearStateIfNeeded(self):
    pass


class _FakeNetworkController():
  def __init__(self):
    self.wpr_mode = None

  @property
  def is_open(self):
    return self.wpr_mode is not None

  @property
  def use_live_traffic(self):
    return self.wpr_mode == wpr_modes.WPR_OFF

  def Open(self, wpr_mode=None):
    self.wpr_mode = wpr_mode if wpr_mode is not None else wpr_modes.WPR_REPLAY

  def UpdateTrafficSettings(
      self, round_trip_latency_ms=None,
      download_bandwidth_kbps=None, upload_bandwidth_kbps=None):
    pass

  def Close(self):
    self.StopReplay()
    self.wpr_mode = None

  def StartReplay(self, *args, **kwargs):
    del args  # Unused.
    del kwargs  # Unused.
    assert self.is_open

  def StopReplay(self):
    pass


class _FakeTab():
  def __init__(self, browser, tab_id):
    self._browser = browser
    self._tab_id = str(tab_id)
    self._collect_garbage_count = 0
    self.test_png = None

  @property
  def collect_garbage_count(self):
    return self._collect_garbage_count

  @property
  def id(self):
    return self._tab_id

  @property
  def browser(self):
    return self._browser

  def WaitForDocumentReadyStateToBeComplete(self, timeout=0):
    pass

  def Navigate(self, url, script_to_evaluate_on_commit=None,
               timeout=0):
    del script_to_evaluate_on_commit, timeout # unused
    if url == 'chrome://crash':
      self.browser._is_crashed = True
      raise Exception

  def WaitForDocumentReadyStateToBeInteractiveOrBetter(self, timeout=0):
    pass

  def WaitForFrameToBeDisplayed(self, timeout=0):
    pass

  def IsAlive(self):
    return True

  def CloseConnections(self):
    pass

  def CollectGarbage(self):
    self._collect_garbage_count += 1

  def Close(self):
    pass

  @property
  def screenshot_supported(self):
    return self.test_png is not None

  def Screenshot(self):
    assert self.screenshot_supported, 'Screenshot is not supported'
    return image_util.FromBase64Png(self.test_png)


class _FakeTabList():
  _current_tab_id = 0

  def __init__(self, browser):
    self._tabs = []
    self._browser = browser

  def New(self, timeout=300):
    del timeout  # unused
    type(self)._current_tab_id += 1
    t = _FakeTab(self._browser, type(self)._current_tab_id)
    self._tabs.append(t)
    return t

  def __iter__(self):
    return self._tabs.__iter__()

  def __len__(self):
    return len(self._tabs)

  def __getitem__(self, index):
    if self._tabs[index].browser._is_crashed:
      raise Exception
    return self._tabs[index]

  def GetTabById(self, identifier):
    """The identifier of a tab can be accessed with tab.id."""
    for tab in self._tabs:
      if tab.id == identifier:
        return tab
    return None


class FakeInspectorWebsocket():
  _NOTIFICATION_EVENT = 1
  _NOTIFICATION_CALLBACK = 2

  """A fake InspectorWebsocket.

  A fake that allows tests to send pregenerated data. Normal
  InspectorWebsockets allow for any number of domain handlers. This fake only
  allows up to 1 domain handler, and assumes that the domain of the response
  always matches that of the handler.
  """
  def __init__(self, mock_timer):
    self._mock_timer = mock_timer
    self._notifications = []
    self._response_handlers = {}
    self._pending_callbacks = {}
    self._handler = None

  def RegisterDomain(self, _, handler):
    self._handler = handler

  def AddEvent(self, method, params, time):
    if self._notifications:
      assert self._notifications[-1][1] < time, (
          'Current response is scheduled earlier than previous response.')
    response = {'method': method, 'params': params}
    self._notifications.append((response, time, self._NOTIFICATION_EVENT))

  def AddAsyncResponse(self, method, result, time):
    if self._notifications:
      assert self._notifications[-1][1] < time, (
          'Current response is scheduled earlier than previous response.')
    response = {'method': method, 'result': result}
    self._notifications.append((response, time, self._NOTIFICATION_CALLBACK))

  def AddResponseHandler(self, method, handler):
    self._response_handlers[method] = handler

  def SyncRequest(self, request, *args, **kwargs):
    del args, kwargs  # unused
    handler = self._response_handlers[request['method']]
    return handler(request) if handler else None

  def AsyncRequest(self, request, callback):
    self._pending_callbacks.setdefault(request['method'], []).append(callback)

  def SendAndIgnoreResponse(self, request):
    pass

  def Connect(self, _):
    pass

  def DispatchNotifications(self, timeout):
    current_time = self._mock_timer.time()
    if not self._notifications:
      self._mock_timer.SetTime(current_time + timeout + 1)
      raise inspector_websocket.WebSocketException(
          websocket.WebSocketTimeoutException())

    response, time, kind = self._notifications[0]
    if time - current_time > timeout:
      self._mock_timer.SetTime(current_time + timeout + 1)
      raise inspector_websocket.WebSocketException(
          websocket.WebSocketTimeoutException())

    self._notifications.pop(0)
    self._mock_timer.SetTime(time + 1)
    if kind == self._NOTIFICATION_EVENT:
      self._handler(response)
    elif kind == self._NOTIFICATION_CALLBACK:
      callback = self._pending_callbacks.get(response['method']).pop(0)
      callback(response)
    else:
      raise Exception('Unexpected response type')


class FakeTimer():
  """ A fake timer to fake out the timing for a module.
    Args:
      module: module to fake out the time
  """
  def __init__(self, module=None):
    self._elapsed_time = 0
    self._module = module
    self._actual_time = None
    if module:
      assert isinstance(module, types.ModuleType)
      self._actual_time = module.time
      self._module.time = self

  def sleep(self, time):
    self._elapsed_time += time

  def time(self):
    return self._elapsed_time

  def SetTime(self, time):
    self._elapsed_time = time

  def __del__(self):
    self.Restore()

  def Restore(self):
    if self._module:
      self._module.time = self._actual_time
      self._module = None
      self._actual_time = None


class FakeParsedArgsForStoryFilter():
  def __init__(
      self, story_filter=None, story_filter_exclude=None,
      story_tag_filter=None, story_tag_filter_exclude=None,
      story_shard_begin_index=None,
      story_shard_end_index=None,
      run_full_story_set=None,
      run_abridged_story_set=None,
      run_disabled_stories=False, stories=None, shard_indexes=None):
    self.story_filter = story_filter
    self.story_filter_exclude = story_filter_exclude
    self.story_tag_filter = story_tag_filter
    self.story_tag_filter_exclude = story_tag_filter_exclude
    self.story_shard_begin_index = (
        story_shard_begin_index)
    self.story_shard_end_index = (
        story_shard_end_index)
    self.run_disabled_stories = run_disabled_stories
    self.run_full_story_set = run_full_story_set
    self.run_abridged_story_set = run_abridged_story_set
    self.stories = stories
    self.story_shard_indexes = shard_indexes
