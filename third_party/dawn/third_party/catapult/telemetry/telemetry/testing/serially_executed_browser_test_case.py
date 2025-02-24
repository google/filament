# Copyright 2016 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import absolute_import
import inspect
import logging
import re
import unittest

from py_utils import cloud_storage
from telemetry.internal.browser import browser_finder
from telemetry.internal.browser import browser_finder_exceptions
from telemetry.internal.platform import android_device
from telemetry.internal.results import artifact_logger
from telemetry.testing import browser_test_context
from typ import json_results
from typ import test_case

DEFAULT_LOG_FORMAT = (
    '(%(levelname)s) %(asctime)s %(module)s.%(funcName)s:%(lineno)d  '
    '%(message)s')


class SeriallyExecutedBrowserTestCase(test_case.TestCase):

  browser = None

  def __init__(self, methodName):
    super().__init__(methodName)
    self._private_methodname = methodName

  def shortName(self):
    """Returns the method name this test runs, without the package prefix."""
    return self._private_methodname

  def set_artifacts(self, artifacts):
    super().set_artifacts(artifacts)
    artifact_logger.RegisterArtifactImplementation(artifacts)

  def setUp(self):
    if self.browser and self.ShouldPerformMinidumpCleanupOnSetUp():
      self.browser.CleanupUnsymbolizedMinidumps()

  def tearDown(self):
    if self.browser and self.ShouldPerformMinidumpCleanupOnTearDown():
      self.browser.CleanupUnsymbolizedMinidumps(fatal=True)

  def ShouldPerformMinidumpCleanupOnSetUp(self):
    return True

  def ShouldPerformMinidumpCleanupOnTearDown(self):
    return True

  @classmethod
  def Name(cls):
    return cls.__name__

  def CanRunInParallel(self):
    return False

  @classmethod
  def AddCommandlineArgs(cls, parser):
    pass

  @classmethod
  def SetUpProcess(cls):
    """ Set up testing logic before running the test case.
    This is guaranteed to be called only once for all the tests before the test
    suite runs.
    """
    finder_options = browser_test_context.GetCopy().finder_options
    cls._finder_options = finder_options

    # Set up logging based on the verbosity passed from the parent to
    # the child process.
    if finder_options.verbosity >= 2:
      logging.getLogger().setLevel(logging.DEBUG)
    elif finder_options.verbosity:
      logging.getLogger().setLevel(logging.INFO)
    else:
      logging.getLogger().setLevel(logging.WARNING)
    logging.basicConfig(format=DEFAULT_LOG_FORMAT)

    cls.platform = None
    cls.browser = None
    cls._browser_to_create = None
    cls._browser_options = None

  @classmethod
  def SetBrowserOptions(cls, browser_options):
    """Sets the browser option for the browser to create.

    Args:
      browser_options: Browser options object for the browser we want to test.
    """
    cls._browser_options = browser_options
    cls._browser_to_create = browser_finder.FindBrowser(browser_options)
    if not cls._browser_to_create:
      raise browser_finder_exceptions.BrowserFinderException(
          'Cannot find browser of type %s. \n\nAvailable browsers:\n%s\n' % (
              browser_options.browser_options.browser_type,
              '\n'.join(browser_finder.GetAllAvailableBrowserTypes(
                  browser_options))))
    if not cls.platform:
      cls.platform = cls._browser_to_create.platform
      cls.platform.SetPerformanceMode(android_device.HIGH_PERFORMANCE_MODE)
      cls.platform.network_controller.Open(
          browser_options.browser_options.wpr_mode)
    else:
      assert cls.platform == cls._browser_to_create.platform, (
          f'All browser launches within same test suite must use browsers on '
          f'the same platform ({cls.platform} vs '
          f'{cls._browser_to_create.platform})')

  @classmethod
  def StartWPRServer(cls, archive_path=None, archive_bucket=None):
    """Start a webpage replay server.

    Args:
      archive_path: Path to the WPR file. If there is a corresponding sha1 file,
          this archive will be automatically downloaded from Google Storage.
      archive_bucket: The bucket to look for the WPR archive.
    """
    assert cls._browser_options, (
        'Browser options must be set with |SetBrowserOptions| prior to '
        'starting WPR')
    assert not cls.browser, 'WPR must be started prior to browser being started'

    cloud_storage.GetIfChanged(archive_path, archive_bucket)
    cls.platform.network_controller.StartReplay(archive_path)

  @classmethod
  def StopWPRServer(cls):
    cls.platform.network_controller.StopReplay()

  @classmethod
  def StartBrowser(cls):
    assert cls._browser_options, (
        'Browser options must be set with |SetBrowserOptions| prior to '
        'starting WPR')
    assert not cls.browser, 'Browser is started. Must close it first'

    try:
      # TODO(crbug.com/803104): Note cls._browser_options actually is a
      # FinderOptions object, and we need to access the real browser_option's
      # contained inside.
      cls._browser_to_create.SetUpEnvironment(
          cls._browser_options.browser_options)
      cls.browser = cls._browser_to_create.Create()
      specifiers = set(cls.GetPlatformTags(cls.browser) +
                       cls._browser_to_create.GetTypExpectationsTags())
      if cls.child.has_expectations and specifiers:
        logging.info(
            'The following expectations condition tags were generated %s' %
            str(list(specifiers)))
        cls.child.expectations.set_tags(specifiers)
    except Exception:
      cls._browser_to_create.CleanUpEnvironment()
      raise

  @classmethod
  def StopBrowser(cls):
    assert cls.browser, 'Browser is not started'
    try:
      cls.browser.Close()
      cls.browser = None
    finally:
      cls._browser_to_create.CleanUpEnvironment()

  @classmethod
  def TearDownProcess(cls):
    """ Tear down the testing logic after running the test cases.
    This is guaranteed to be called only once for all the tests after the test
    suite finishes running.
    """
    if cls.platform:
      cls.platform.StopAllLocalServers()
      cls.platform.network_controller.Close()
      cls.platform.SetPerformanceMode(android_device.NORMAL_PERFORMANCE_MODE)
    if cls.browser:
      cls.StopBrowser()

  @classmethod
  def SetStaticServerDirs(cls, dirs_path):
    assert cls.platform
    assert isinstance(dirs_path, list)
    cls.platform.SetHTTPServerDirectories(dirs_path)

  @classmethod
  def UrlOfStaticFilePath(cls, file_path):
    return cls.platform.http_server.UrlOf(file_path)

  @classmethod
  def LocalhostUrlOfStaticFilePath(cls, file_path):
    return cls.platform.http_server.LocalhostUrlOf(file_path)

  @classmethod
  def ExpectationsFiles(cls):
    """Subclasses can override this class method to return a list of absolute
    paths to the test expectations files.

    Returns:
    A list of test expectations file paths. The paths must be absolute.
    """
    return []

  @classmethod
  def IgnoredTags(cls):
    """Subclasses can override this class method to return a list of typ
    expectation tags to ignore for the purposes of tag validation.

    This is so that expectation files don't need to get cluttered with
    unnecessary tags, e.g. if two tags are functionally equivalent or if some
    tag is completely unnecessary for a particular test type.

    Returns:
      A list of typ tags.
    """
    return []

  def GetExpectationsForTest(self):
    """Subclasses can override this method to return a tuple containing a set
    of expected results and a flag indicating if the test has the RetryOnFailure
    expectation. Tests members may want to know the test expectation in order to
    modify its behavior for certain expectations. For instance GPU tests want
    to avoid symbolizing any crash dumps in the case of expected test failures
    or when tests are being retried because they are expected to be flaky.

    Returns:
    A tuple containing set of expected results for a test and a boolean value
    indicating if the test contains the RetryOnFailure expectation. When there
    are no expectations files passed to typ, then a tuple of
    (set(['PASS']), False) should be returned from this function.
    """
    exp = self.child.expectations_for(self)
    return exp.results, exp.should_retry_on_failure

  @classmethod
  def GetPlatformTags(cls, browser):
    """This method uses the Browser instances's platform member variable to get
    the operating system, operating system version and browser type tags.
    Example tags for  operating system are 'linux' and 'mac'. Example tags
    for the operating system version are 'mojave' and  'vista'. Example tags
    for browser type are 'debug' and 'release'. If a None value or empty string
    is retrieved from the browser's platform member variable, then it will be
    filtered out.

    Args:
    Browser instance returned from the possible_browser.BrowserSession() method.

    Returns:
    A list of tags derived from the Browser instance's platform member variable.
    """
    return browser.GetTypExpectationsTags()

  @classmethod
  def GetTagConflictChecker(cls):
    """Gets the non-standard tag checking function to use.

    Passed on to typ for use when parsing expectation files. The default value
    of None will cause the default conflict checker to be used.

    If overridden, the returned callable should take two tags as strings and
    return a boolean indicating whether the two tags conflict or not.
    """
    return None

  @staticmethod
  def GetJSONResultsDelimiter():
    """This method returns the path delimiter that will be used to seperate
    a test name into parts. By default, the delimiter is '.'
    """
    return json_results.DEFAULT_TEST_SEPARATOR


def LoadAllTestsInModule(module):
  """ Load all tests & generated browser tests in a given module.

  This is supposed to be invoke in load_tests() method of your test modules that
  use browser_test_runner framework to discover & generate the tests to be
  picked up by the test runner. Here is the example of how your test module
  should looks like:

  ################## my_awesome_browser_tests.py  ################
  import sys

  from telemetry.testing import serially_executed_browser_test_case
  ...

  class TestSimpleBrowser(
      serially_executed_browser_test_case.SeriallyExecutedBrowserTestCase):
  ...
  ...

  def load_tests(loader, tests, pattern):
    return serially_executed_browser_test_case.LoadAllTestsInModule(
        sys.modules[__name__])
  #################################################################

  Args:
    module: the module which contains test cases classes.

  Returns:
    an instance of unittest.TestSuite, which contains all the tests & generated
    test cases to be run.
  """
  suite = unittest.TestSuite()
  test_context = browser_test_context.GetCopy()
  if not test_context:
    return suite
  for _, obj in inspect.getmembers(module):
    if (inspect.isclass(obj) and
        issubclass(obj, SeriallyExecutedBrowserTestCase)):
      # We bail out early if this class doesn't match the targeted
      # test_class in test_context to avoid calling GenerateTestCases
      # for tests that we don't intend to run. This is to avoid possible errors
      # in GenerateTestCases as the test class may define custom options in
      # the finder_options object, and hence would raise error if they can't
      # find their custom options in finder_options object.
      if test_context.test_class != obj:
        continue
      for test in GenerateTestCases(
          test_class=obj, finder_options=test_context.finder_options):
        if test.id() in test_context.test_case_ids_to_run:
          suite.addTest(test)
  return suite


def _GenerateTestMethod(based_method, args):
  return lambda self: based_method(self, *args)


_TEST_GENERATOR_PREFIX = 'GenerateTestCases_'
# Accept any test name as long as it does not have whitespace or * in it.
_VALID_TEST_NAME_RE = re.compile(r'^[^*\s]*$')

def _ValidateTestMethodname(test_name):
  assert _VALID_TEST_NAME_RE.search(test_name)


def GenerateTestCases(test_class, finder_options):
  test_cases = []
  for name, method in inspect.getmembers(
      test_class,
      predicate=lambda x: inspect.isfunction(x) or inspect.ismethod(x)):
    if name.startswith('test'):
      # Do not allow method names starting with "test" in these
      # subclasses, to avoid collisions with Python's unit test runner.
      raise Exception('Name collision with Python\'s unittest runner: %s' %
                      name)
    if name.startswith('Test'):
      # Pass these through for the time being. We may want to rethink
      # how they are handled in the future.
      test_cases.append(test_class(name))
    elif name.startswith(_TEST_GENERATOR_PREFIX):
      based_method_name = name[len(_TEST_GENERATOR_PREFIX):]
      assert hasattr(test_class, based_method_name), (
          '%s is specified but based method %s does not exist' %
          (name, based_method_name))
      based_method = getattr(test_class, based_method_name)
      for generated_test_name, args in method(finder_options):
        _ValidateTestMethodname(generated_test_name)
        setattr(test_class, generated_test_name, _GenerateTestMethod(
            based_method, args))
        test_cases.append(test_class(generated_test_name))
  return test_cases
