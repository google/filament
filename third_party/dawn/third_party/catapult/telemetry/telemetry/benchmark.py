# Copyright 2014 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import absolute_import
import logging
import six

from telemetry import decorators
from telemetry.internal import story_runner
from telemetry.internal.util import command_line
from telemetry.page import legacy_page_test
from telemetry.story import expectations as expectations_module
from telemetry.web_perf import story_test
from telemetry.web_perf import timeline_based_measurement

Info = decorators.Info

# TODO(crbug.com/859524): remove this once we update all the benchmarks in
# tools/perf to use Info decorator.
Owner = decorators.Info  # pylint: disable=invalid-name


class InvalidOptionsError(Exception):
  """Raised for invalid benchmark options."""


class Benchmark(command_line.Command):
  """Base class for a Telemetry benchmark.

  A benchmark packages a measurement and a PageSet together.
  Benchmarks default to using TBM unless you override the value of
  Benchmark.test, or override the CreatePageTest method.

  New benchmarks should override CreateStorySet.
  """
  options = {}
  page_set = None
  test = timeline_based_measurement.TimelineBasedMeasurement
  SUPPORTED_PLATFORMS = [expectations_module.ALL]
  SUPPORTED_PLATFORM_TAGS = []

  def __init__(self, max_failures=None):
    """Creates a new Benchmark.

    Args:
      max_failures: The number of story run's failures before bailing from
        executing subsequent page runs. If None, we never bail.
    """
    self._max_failures = max_failures
    # TODO: There should be an assertion here that checks that only one of
    # the following is true:
    # * It's a TBM benchmark, with CreateCoreTimelineBasedMeasurementOptions
    #   defined.
    # * It's a legacy benchmark, with either CreatePageTest defined or
    #   Benchmark.test set.
    # See https://github.com/catapult-project/catapult/issues/3708

  def CanRunOnPlatform(self, platform, finder_options):
    """Figures out if the benchmark is meant to support this platform.

    This is based on the SUPPORTED_PLATFORMS class member of the benchmark.

    This method should not be overriden or called outside of the Telemetry
    framework.

    Note that finder_options object in practice sometimes is actually not
    a BrowserFinderOptions object but a PossibleBrowser object.
    The key is that it can be passed to ShouldDisable, which only uses
    finder_options.browser_type, which is available on both PossibleBrowser
    and BrowserFinderOptions.
    """
    for p in self.SUPPORTED_PLATFORMS:
      # This is reusing StoryExpectation code, so it is a bit unintuitive. We
      # are trying to detect the opposite of the usual case in StoryExpectations
      # so we want to return True when ShouldDisable returns true, even though
      # we do not want to disable.
      if p.ShouldDisable(platform, finder_options):
        return True
    return False

  def Run(self, args):
    """Do not override this method.

    Args:
      args: a browser_options.BrowserFinderOptions instance.

    Returns:
      An exit code from exit_codes module describing what happened.
    """
    args.target_platforms = self.GetSupportedPlatformNames(
        self.SUPPORTED_PLATFORMS)
    return story_runner.RunBenchmark(self, args)

  @property
  def max_failures(self):
    return self._max_failures

  @classmethod
  def Name(cls):
    return '%s.%s' % (cls.__module__.split('.')[-1], cls.__name__)

  @classmethod
  def AddCommandLineArgs(cls, parser):
    group = parser.add_argument_group(f'{cls.Name()} test options')
    cls.AddBenchmarkCommandLineArgs(group)

  @classmethod
  def AddBenchmarkCommandLineArgs(cls, parser):
    del parser  # unused

  @classmethod
  def GetSupportedPlatformNames(cls, supported_platforms):
    """Returns a list of platforms supported by this benchmark.

    Returns:
      A set of names of supported platforms. The supported platforms are a list
      of strings that would match possible values from platform.GetOsName().
    """
    result = set()
    for p in supported_platforms:
      result.update(p.GetSupportedPlatformNames())
    return frozenset(result)

  @classmethod
  def SetArgumentDefaults(cls, parser):
    default_values = parser.get_default_values()
    invalid_options = [o for o in cls.options if not hasattr(default_values, o)]
    if invalid_options:
      raise InvalidOptionsError('Invalid benchmark options: %s' %
                                ', '.join(invalid_options))
    parser.set_defaults(**cls.options)

  @classmethod
  def ProcessCommandLineArgs(cls, parser, args):
    pass

  def CustomizeOptions(self, finder_options, possible_browser=None):
    """Add options that are required by this benchmark."""

  def SetExtraBrowserOptions(self, options):
    """Set extra browser command line options"""

  def GetBugComponents(self):
    """Return the benchmark's Monorail component as a string."""
    return decorators.GetComponent(self)

  def GetOwners(self):
    """Return the benchmark's owners' emails in a list."""
    return decorators.GetEmails(self)

  def GetDocumentationLinks(self):
    """Return the benchmark's documentation links.

    Returns:
      A list of [title, url] pairs. This is the form that allows Dashboard
      to display links properly.
    """
    links = []
    url = decorators.GetDocumentationLink(self)
    if url is not None:
      links.append(['Benchmark documentation link', url])
    return links

  def GetInfoBlurb(self):
    """Return any info blurb associated with the the benchmark"""
    return decorators.GetInfoBlurb(self)

  def CreateCoreTimelineBasedMeasurementOptions(self):
    """Return the base TimelineBasedMeasurementOptions for this Benchmark.

    Additional chrome and atrace categories can be appended when running the
    benchmark with the --extra-chrome-categories and --extra-atrace-categories
    flags.

    Override this method to configure a TimelineBasedMeasurement benchmark. If
    this is not a TimelineBasedMeasurement benchmark, override CreatePageTest
    for PageTest tests. Do not override both methods.
    """
    return timeline_based_measurement.Options()

  def _GetTimelineBasedMeasurementOptions(self, options):
    """Return all timeline based measurements for the curren benchmark run.

    This includes the benchmark-configured measurements in
    CreateCoreTimelineBasedMeasurementOptions as well as the user-flag-
    configured options from --extra-chrome-categories and
    --extra-atrace-categories.
    """
    tbm_options = self.CreateCoreTimelineBasedMeasurementOptions()
    if options and options.extra_chrome_categories:
      # If Chrome tracing categories for this benchmark are not already
      # enabled, there is probably a good reason why. Don't change whether
      # Chrome tracing is enabled.
      assert tbm_options.config.enable_chrome_trace, (
          'This benchmark does not support Chrome tracing.')
      tbm_options.config.chrome_trace_config.category_filter.AddFilterString(
          options.extra_chrome_categories)
    if options and options.extra_atrace_categories:
      # Many benchmarks on Android run without atrace by default. Hopefully the
      # user understands that atrace is only supported on Android when setting
      # this option.
      tbm_options.config.enable_atrace_trace = True

      categories = tbm_options.config.atrace_config.categories
      if isinstance(categories, six.string_types):
        # Categories can either be a list or comma-separated string.
        # https://github.com/catapult-project/catapult/issues/3712
        categories = categories.split(',')
      for category in options.extra_atrace_categories.split(','):
        if category not in categories:
          categories.append(category)
      tbm_options.config.atrace_config.categories = categories
    if options and options.enable_systrace:
      tbm_options.config.chrome_trace_config.SetEnableSystrace()
    legacy_json_format = options and options.legacy_json_trace_format
    if legacy_json_format:
      tbm_options.config.chrome_trace_config.SetJsonTraceFormat()
    else:
      tbm_options.config.chrome_trace_config.SetProtoTraceFormat()
    if options and options.experimental_system_tracing:
      assert not legacy_json_format
      logging.warning('Enabling experimental system tracing!')
      tbm_options.config.enable_experimental_system_tracing = True
      tbm_options.config.system_trace_config.EnableChrome(
          chrome_trace_config=tbm_options.config.chrome_trace_config)
    if options and options.experimental_system_data_sources:
      assert not legacy_json_format
      tbm_options.config.enable_experimental_system_tracing = True
      tbm_options.config.system_trace_config.EnablePower()
      tbm_options.config.system_trace_config.EnableSysStatsCpu()
      tbm_options.config.system_trace_config.EnableFtraceCpu()
      tbm_options.config.system_trace_config.EnableFtraceSched()
      logging.warning('Not running Perfetto profiling (callstack sampling) '
                      'even though --experimental_system_data_sources was '
                      'enabled. Please manually update the benchmark to '
                      'override PerfBenchmarkWithProfiling, if profiling is '
                      'needed.')

    if options and options.force_sideload_perfetto:
      assert tbm_options.config.enable_experimental_system_tracing
      tbm_options.config.force_sideload_perfetto = True

    # TODO(crbug.com/1012687): Remove or adjust the following warnings as the
    # development of TBMv3 progresses.
    tbmv3_metrics = [
        m[6:]
        for m in tbm_options.GetTimelineBasedMetrics()
        if m.startswith('tbmv3:')
    ]
    if tbmv3_metrics:
      if legacy_json_format:
        logging.warning(
            'Selected TBMv3 metrics will not be computed because they are not '
            "supported in Chrome's JSON trace format.")
      else:
        logging.warning(
            'The following TBMv3 metrics have been selected to run: %s. '
            'Please note that TBMv3 is an experimental feature in active '
            'development, and may not be supported in the future in its '
            'current form. Follow crbug.com/1012687 for updates and to '
            'discuss your use case before deciding to rely on this feature.',
            ', '.join(tbmv3_metrics))
    return tbm_options

  def CreatePageTest(self, options):  # pylint: disable=unused-argument
    """Return the PageTest for this Benchmark.

    Override this method for PageTest tests.
    Override, CreateCoreTimelineBasedMeasurementOptions to configure
    TimelineBasedMeasurement tests. Do not override both methods.

    Args:
      options: a browser_options.BrowserFinderOptions instance

    Returns:
      |test()| if |test| is a PageTest class.
      Otherwise, a TimelineBasedMeasurement instance.
    """
    is_page_test = issubclass(self.test, legacy_page_test.LegacyPageTest)
    is_story_test = issubclass(self.test, story_test.StoryTest)
    if not is_page_test and not is_story_test:
      raise TypeError('"%s" is not a PageTest or a StoryTest.' %
                      self.test.__name__)
    if is_page_test:
      # TODO: assert that CreateCoreTimelineBasedMeasurementOptions is not
      # defined. That's incorrect for a page test. See
      # https://github.com/catapult-project/catapult/issues/3708
      return self.test()  # pylint: disable=no-value-for-parameter

    opts = self._GetTimelineBasedMeasurementOptions(options)
    return self.test(opts)

  def CreateStorySet(self, options):
    """Creates the instance of StorySet used to run the benchmark.

    Can be overridden by subclasses.
    """
    del options  # unused
    # TODO(aiolos, nednguyen, eakufner): replace class attribute page_set with
    # story_set.
    if not self.page_set:
      raise NotImplementedError('This test has no "page_set" attribute.')
    return self.page_set()  # pylint: disable=not-callable
