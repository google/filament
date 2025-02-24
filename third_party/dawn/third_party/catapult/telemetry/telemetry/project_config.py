# Copyright 2013 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import absolute_import
from telemetry import benchmark

from py_utils import discover


class ProjectConfig():
  """Contains information about the benchmark runtime environment.

  Attributes:
    top_level_dir: A dir that contains benchmark, page test, and/or story
        set dirs and associated artifacts.
    benchmark_dirs: A list of dirs containing benchmarks.
    benchmark_aliases: A dict of name:alias string pairs to be matched against
        exactly during benchmark selection.
    client_configs: A list of paths to a ProjectDependencies json files.
    default_chrome_root: A path to chromium source directory. Many telemetry
      features depend on chromium source tree's presence and those won't work
      in case this is not specified.
    expectations_file: A path to expectations file.
  """
  def __init__(self, top_level_dir, benchmark_dirs=None,
               benchmark_aliases=None, client_configs=None,
               default_chrome_root=None, expectations_files=None):
    self._top_level_dir = top_level_dir
    self._benchmark_dirs = benchmark_dirs or []
    self._benchmark_aliases = benchmark_aliases or dict()
    self._client_configs = client_configs or []
    self._default_chrome_root = default_chrome_root
    self._expectations_files = expectations_files or []
    self._benchmarks = None

  @property
  def top_level_dir(self):
    return self._top_level_dir

  @property
  def start_dirs(self):
    return self._benchmark_dirs

  @property
  def benchmark_dirs(self):
    return self._benchmark_dirs

  @property
  def benchmark_aliases(self):
    return self._benchmark_aliases

  @property
  def client_configs(self):
    return self._client_configs

  @property
  def default_chrome_root(self):
    return self._default_chrome_root

  @property
  def expectations_files(self):
    return self._expectations_files

  def AdjustStartupFlags(self, args):
    """Returns a new list of adjusted startup flags.

    Subclasses should override this method to change startup flags before use.
    """
    return args

  def GetBenchmarks(self):
    """Return a list of all benchmark classes found in this configuration."""
    if self._benchmarks is None:
      benchmarks = []
      for search_dir in self.benchmark_dirs:
        benchmarks.extend(list(discover.DiscoverClasses(
            search_dir,
            self.top_level_dir,
            benchmark.Benchmark,
            index_by_class_name=True).values()))
      self._benchmarks = benchmarks
    return list(self._benchmarks)

  def GetBenchmarkByName(self, benchmark_name):
    """Find a benchmark by an exact name match.

    Args:
      benchmark_name: The name or a alias of a benchmark.

    Returns:
      The benchmark class if an exact match for the name is found, or None
      otherwise.
    """
    # Allow using aliases to find benchmarks.
    benchmark_name = self.benchmark_aliases.get(benchmark_name, benchmark_name)
    for benchmark_class in self.GetBenchmarks():
      if benchmark_name == benchmark_class.Name():
        return benchmark_class
    return None
