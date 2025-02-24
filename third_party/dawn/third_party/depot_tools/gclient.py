#!/usr/bin/env python3
# Copyright (c) 2012 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
"""Meta checkout dependency manager for Git."""
# Files
#   .gclient      : Current client configuration, written by 'config' command.
#                   Format is a Python script defining 'solutions', a list whose
#                   entries each are maps binding the strings "name" and "url"
#                   to strings specifying the name and location of the client
#                   module, as well as "custom_deps" to a map similar to the
#                   deps section of the DEPS file below, as well as
#                   "custom_hooks" to a list similar to the hooks sections of
#                   the DEPS file below.
#   .gclient_entries : A cache constructed by 'update' command.  Format is a
#                   Python script defining 'entries', a list of the names
#                   of all modules in the client
#   <module>/DEPS : Python script defining var 'deps' as a map from each
#                   requisite submodule name to a URL where it can be found (via
#                   one SCM)
#
# Hooks
#   .gclient and DEPS files may optionally contain a list named "hooks" to
#   allow custom actions to be performed based on files that have changed in the
#   working copy as a result of a "sync"/"update" or "revert" operation.  This
#   can be prevented by using --nohooks (hooks run by default). Hooks can also
#   be forced to run with the "runhooks" operation.  If "sync" is run with
#   --force, all known but not suppressed hooks will run regardless of the state
#   of the working copy.
#
#   Each item in a "hooks" list is a dict, containing these two keys:
#     "pattern"  The associated value is a string containing a regular
#                expression.  When a file whose pathname matches the expression
#                is checked out, updated, or reverted, the hook's "action" will
#                run.
#     "action"   A list describing a command to run along with its arguments, if
#                any.  An action command will run at most one time per gclient
#                invocation, regardless of how many files matched the pattern.
#                The action is executed in the same directory as the .gclient
#                file.  If the first item in the list is the string "python",
#                the current Python interpreter (sys.executable) will be used
#                to run the command. If the list contains string
#                "$matching_files" it will be removed from the list and the list
#                will be extended by the list of matching files.
#     "name"     An optional string specifying the group to which a hook belongs
#                for overriding and organizing.
#
#   Example:
#     hooks = [
#       { "pattern": "\\.(gif|jpe?g|pr0n|png)$",
#         "action":  ["python", "image_indexer.py", "--all"]},
#       { "pattern": ".",
#         "name": "gyp",
#         "action":  ["python", "src/build/gyp_chromium"]},
#     ]
#
# Pre-DEPS Hooks
#   DEPS files may optionally contain a list named "pre_deps_hooks".  These are
#   the same as normal hooks, except that they run before the DEPS are
#   processed. Pre-DEPS run with "sync" and "revert" unless the --noprehooks
#   flag is used.
#
# Specifying a target OS
#   An optional key named "target_os" may be added to a gclient file to specify
#   one or more additional operating systems that should be considered when
#   processing the deps_os/hooks_os dict of a DEPS file.
#
#   Example:
#     target_os = [ "android" ]
#
#   If the "target_os_only" key is also present and true, then *only* the
#   operating systems listed in "target_os" will be used.
#
#   Example:
#     target_os = [ "ios" ]
#     target_os_only = True
#
# Specifying a target CPU
#   To specify a target CPU, the variables target_cpu and target_cpu_only
#   are available and are analogous to target_os and target_os_only.

__version__ = '0.7'

import copy
import hashlib
import json
import logging
import optparse
import os
import platform
import posixpath
import pprint
import re
import sys
import shutil
import tarfile
import tempfile
import time
import urllib.parse

from collections.abc import Collection, Mapping, Sequence

import detect_host_arch
import download_from_google_storage
import git_common
import gclient_eval
import gclient_paths
import gclient_scm
import gclient_utils
import git_cache
import metrics
import metrics_utils
import scm as scm_git
import setup_color
import subcommand
import subprocess2
import upload_to_google_storage_first_class
from third_party.repo.progress import Progress

# TODO: Should fix these warnings.
# pylint: disable=line-too-long

DEPOT_TOOLS_DIR = os.path.dirname(os.path.abspath(os.path.realpath(__file__)))

# Singleton object to represent an unset cache_dir (as opposed to a disabled
# one, e.g. if a spec explicitly says `cache_dir = None`.)
UNSET_CACHE_DIR = object()

PREVIOUS_CUSTOM_VARS_FILE = '.gclient_previous_custom_vars'
PREVIOUS_SYNC_COMMITS_FILE = '.gclient_previous_sync_commits'

PREVIOUS_SYNC_COMMITS = 'GCLIENT_PREVIOUS_SYNC_COMMITS'

NO_SYNC_EXPERIMENT = 'no-sync'

PRECOMMIT_HOOK_VAR = 'GCLIENT_PRECOMMIT'


class GNException(Exception):
    pass


def ToGNString(value):
    """Returns a stringified GN equivalent of the Python value."""
    if isinstance(value, str):
        if value.find('\n') >= 0:
            raise GNException("Trying to print a string with a newline in it.")
        return '"' + \
            value.replace('\\', '\\\\').replace('"', '\\"').replace('$', '\\$') + \
            '"'

    if isinstance(value, bool):
        if value:
            return "true"
        return "false"

    # NOTE: some type handling removed compared to chromium/src copy.

    raise GNException("Unsupported type when printing to GN.")


class Hook(object):
    """Descriptor of command ran before/after sync or on demand."""
    def __init__(self,
                 action,
                 pattern=None,
                 name=None,
                 cwd=None,
                 condition=None,
                 variables=None,
                 verbose=False,
                 cwd_base=None):
        """Constructor.

    Arguments:
      action (list of str): argv of the command to run
      pattern (str regex): noop with git; deprecated
      name (str): optional name; no effect on operation
      cwd (str): working directory to use
      condition (str): condition when to run the hook
      variables (dict): variables for evaluating the condition
    """
        self._action = gclient_utils.freeze(action)
        self._pattern = pattern
        self._name = name
        self._cwd = cwd
        self._condition = condition
        self._variables = variables
        self._verbose = verbose
        self._cwd_base = cwd_base

    @staticmethod
    def from_dict(d,
                  variables=None,
                  verbose=False,
                  conditions=None,
                  cwd_base=None):
        """Creates a Hook instance from a dict like in the DEPS file."""
        # Merge any local and inherited conditions.
        gclient_eval.UpdateCondition(d, 'and', conditions)
        return Hook(
            d['action'],
            d.get('pattern'),
            d.get('name'),
            d.get('cwd'),
            d.get('condition'),
            variables=variables,
            # Always print the header if not printing to a TTY.
            verbose=verbose or not setup_color.IS_TTY,
            cwd_base=cwd_base)

    @property
    def action(self):
        return self._action

    @property
    def pattern(self):
        return self._pattern

    @property
    def name(self):
        return self._name

    @property
    def condition(self):
        return self._condition

    @property
    def effective_cwd(self):
        cwd = self._cwd_base
        if self._cwd:
            cwd = os.path.join(cwd, self._cwd)
        return cwd

    def matches(self, file_list):
        """Returns true if the pattern matches any of files in the list."""
        if not self._pattern:
            return True
        pattern = re.compile(self._pattern)
        return bool([f for f in file_list if pattern.search(f)])

    def run(self):
        """Executes the hook's command (provided the condition is met)."""
        if (self._condition and not gclient_eval.EvaluateCondition(
                self._condition, self._variables)):
            return

        cmd = list(self._action)

        if cmd[0] == 'vpython3' and _detect_host_os() == 'win':
            cmd[0] += '.bat'

        exit_code = 2
        try:
            start_time = time.time()
            gclient_utils.CheckCallAndFilter(cmd,
                                             cwd=self.effective_cwd,
                                             print_stdout=True,
                                             show_header=True,
                                             always_show_header=self._verbose)
            exit_code = 0
        except (gclient_utils.Error, subprocess2.CalledProcessError) as e:
            # Use a discrete exit status code of 2 to indicate that a hook
            # action failed.  Users of this script may wish to treat hook action
            # failures differently from VC failures.
            print('Error: %s' % str(e), file=sys.stderr)
            sys.exit(exit_code)
        finally:
            elapsed_time = time.time() - start_time
            metrics.collector.add_repeated(
                'hooks', {
                    'action':
                    gclient_utils.CommandToStr(cmd),
                    'name':
                    self._name,
                    'cwd':
                    os.path.relpath(os.path.normpath(self.effective_cwd),
                                    self._cwd_base),
                    'condition':
                    self._condition,
                    'execution_time':
                    elapsed_time,
                    'exit_code':
                    exit_code,
                })
            if elapsed_time > 10:
                print("Hook '%s' took %.2f secs" %
                      (gclient_utils.CommandToStr(cmd), elapsed_time))


class DependencySettings(object):
    """Immutable configuration settings."""
    def __init__(self, parent, url, managed, custom_deps, custom_vars,
                 custom_hooks, deps_file, should_process, relative, condition):
        # These are not mutable:
        self._parent = parent
        self._deps_file = deps_file

        # Post process the url to remove trailing slashes.
        if isinstance(url, str):
            # urls are sometime incorrectly written as proto://host/path/@rev.
            # Replace it to proto://host/path@rev.
            self._url = url.replace('/@', '@')
        elif isinstance(url, (None.__class__)):
            self._url = url
        else:
            raise gclient_utils.Error(
                ('dependency url must be either string or None, '
                 'instead of %s') % url.__class__.__name__)

        # The condition as string (or None). Useful to keep e.g. for flatten.
        self._condition = condition
        # 'managed' determines whether or not this dependency is synced/updated
        # by gclient after gclient checks it out initially.  The difference
        # between 'managed' and 'should_process' is that the user specifies
        # 'managed' via the --unmanaged command-line flag or a .gclient config,
        # where 'should_process' is dynamically set by gclient if it goes over
        # its recursion limit and controls gclient's behavior so it does not
        # misbehave.
        self._managed = managed
        self._should_process = should_process
        # If this is a recursed-upon sub-dependency, and the parent has
        # use_relative_paths set, then this dependency should check out its own
        # dependencies relative to that parent's path for this, rather than
        # relative to the .gclient file.
        self._relative = relative
        # This is a mutable value which has the list of 'target_os' OSes listed
        # in the current deps file.
        self.local_target_os = None

        # These are only set in .gclient and not in DEPS files.
        self._custom_vars = custom_vars or {}
        self._custom_deps = custom_deps or {}
        self._custom_hooks = custom_hooks or []

        # Make any deps_file path platform-appropriate.
        if self._deps_file:
            for sep in ['/', '\\']:
                self._deps_file = self._deps_file.replace(sep, os.sep)

    @property
    def deps_file(self):
        return self._deps_file

    @property
    def managed(self):
        return self._managed

    @property
    def parent(self):
        return self._parent

    @property
    def root(self):
        """Returns the root node, a GClient object."""
        if not self.parent:
            # This line is to signal pylint that it could be a GClient instance.
            return self or GClient(None, None)
        return self.parent.root

    @property
    def should_process(self):
        """True if this dependency should be processed, i.e. checked out."""
        return self._should_process

    @property
    def custom_vars(self):
        return self._custom_vars.copy()

    @property
    def custom_deps(self):
        return self._custom_deps.copy()

    @property
    def custom_hooks(self):
        return self._custom_hooks[:]

    @property
    def url(self):
        """URL after variable expansion."""
        return self._url

    @property
    def condition(self):
        return self._condition

    @property
    def target_os(self):
        if self.local_target_os is not None:
            return tuple(set(self.local_target_os).union(self.parent.target_os))

        return self.parent.target_os

    @property
    def target_cpu(self):
        return self.parent.target_cpu

    def set_url(self, url):
        self._url = url

    def get_custom_deps(self, name, url):
        """Returns a custom deps if applicable."""
        if self.parent:
            url = self.parent.get_custom_deps(name, url)
        # None is a valid return value to disable a dependency.
        return self.custom_deps.get(name, url)


class Dependency(gclient_utils.WorkItem, DependencySettings):
    """Object that represents a dependency checkout."""
    def __init__(self,
                 parent,
                 name,
                 url,
                 managed,
                 custom_deps,
                 custom_vars,
                 custom_hooks,
                 deps_file,
                 should_process,
                 should_recurse,
                 relative,
                 condition,
                 protocol='https',
                 git_dependencies_state=gclient_eval.DEPS,
                 print_outbuf=False):
        gclient_utils.WorkItem.__init__(self, name)
        DependencySettings.__init__(self, parent, url, managed, custom_deps,
                                    custom_vars, custom_hooks, deps_file,
                                    should_process, relative, condition)

        # This is in both .gclient and DEPS files:
        self._deps_hooks = []

        self._pre_deps_hooks = []

        # Calculates properties:
        self._dependencies = []
        self._vars = {}

        # A cache of the files affected by the current operation, necessary for
        # hooks.
        self._file_list = []
        # List of host names from which dependencies are allowed.
        # Default is an empty set, meaning unspecified in DEPS file, and hence
        # all hosts will be allowed. Non-empty set means allowlist of hosts.
        # allowed_hosts var is scoped to its DEPS file, and so it isn't
        # recursive.
        self._allowed_hosts = frozenset()
        self._gn_args_from = None
        # Spec for .gni output to write (if any).
        self._gn_args_file = None
        self._gn_args = []
        # If it is not set to True, the dependency wasn't processed for its
        # child dependency, i.e. its DEPS wasn't read.
        self._deps_parsed = False
        # This dependency has been processed, i.e. checked out
        self._processed = False
        # This dependency had its pre-DEPS hooks run
        self._pre_deps_hooks_ran = False
        # This dependency had its hook run
        self._hooks_ran = False
        # This is the scm used to checkout self.url. It may be used by
        # dependencies to get the datetime of the revision we checked out.
        self._used_scm = None
        self._used_revision = None
        # The actual revision we ended up getting, or None if that information
        # is unavailable
        self._got_revision = None
        # Whether this dependency should use relative paths.
        self._use_relative_paths = False

        # recursedeps is a mutable value that selectively overrides the default
        # 'no recursion' setting on a dep-by-dep basis.
        #
        # It will be a dictionary of {deps_name: depfile_namee}
        self.recursedeps = {}

        # Whether we should process this dependency's DEPS file.
        self._should_recurse = should_recurse

        # Whether we should sync git/cipd dependencies and hooks from the
        # DEPS file.
        # This is set based on skip_sync_revisions and must be done
        # after the patch refs are applied.
        # If this is False, we will still run custom_hooks and process
        # custom_deps, if any.
        self._should_sync = True

        self._known_dependency_diff = None
        self._dependency_index_state = None

        self._OverrideUrl()
        # This is inherited from WorkItem.  We want the URL to be a resource.
        if self.url and isinstance(self.url, str):
            # The url is usually given to gclient either as https://blah@123
            # or just https://blah.  The @123 portion is irrelevant.
            self.resources.append(self.url.split('@')[0])

        # Controls whether we want to print git's output when we first clone the
        # dependency
        self.print_outbuf = print_outbuf

        self.protocol = protocol
        self.git_dependencies_state = git_dependencies_state

        if not self.name and self.parent:
            raise gclient_utils.Error('Dependency without name')

    def _OverrideUrl(self):
        """Resolves the parsed url from the parent hierarchy."""
        parsed_url = self.get_custom_deps(
          self._name.replace(os.sep, posixpath.sep) \
            if self._name else self._name, self.url)
        if parsed_url != self.url:
            logging.info('Dependency(%s)._OverrideUrl(%s) -> %s', self._name,
                         self.url, parsed_url)
            self.set_url(parsed_url)
            return

        if self.url is None:
            logging.info('Dependency(%s)._OverrideUrl(None) -> None',
                         self._name)
            return

        if not isinstance(self.url, str):
            raise gclient_utils.Error('Unknown url type')

        # self.url is a local path
        path, at, rev = self.url.partition('@')
        if os.path.isdir(path):
            return

        # self.url is a URL
        parsed_url = urllib.parse.urlparse(self.url)
        if parsed_url[0] or re.match(r'^\w+\@[\w\.-]+\:[\w\/]+', parsed_url[2]):
            return

        # self.url is relative to the parent's URL.
        if not path.startswith('/'):
            raise gclient_utils.Error(
                'relative DEPS entry \'%s\' must begin with a slash' % self.url)

        parent_url = self.parent.url
        parent_path = self.parent.url.split('@')[0]
        if os.path.isdir(parent_path):
            # Parent's URL is a local path. Get parent's URL dirname and append
            # self.url.
            parent_path = os.path.dirname(parent_path)
            parsed_url = parent_path + path.replace('/', os.sep) + at + rev
        else:
            # Parent's URL is a URL. Get parent's URL, strip from the last '/'
            # (equivalent to unix dirname) and append self.url.
            parsed_url = parent_url[:parent_url.rfind('/')] + self.url

        logging.info('Dependency(%s)._OverrideUrl(%s) -> %s', self.name,
                     self.url, parsed_url)
        self.set_url(parsed_url)

    def PinToActualRevision(self):
        """Updates self.url to the revision checked out on disk."""
        if self.url is None:
            return
        url = None
        scm = self.CreateSCM()
        if scm.name == 'cipd':
            revision = scm.revinfo(None, None, None)
            package = self.GetExpandedPackageName()
            url = '%s/p/%s/+/%s' % (scm.GetActualRemoteURL(None), package,
                                    revision)
        if scm.name == 'gcs':
            url = self.url

        if os.path.isdir(scm.checkout_path):
            revision = scm.revinfo(None, None, None)
            url = '%s@%s' % (gclient_utils.SplitUrlRevision(
                self.url)[0], revision)
        self.set_url(url)

    def ToLines(self):
        # () -> Sequence[str]
        """Returns strings representing the deps (info, graphviz line)"""
        s = []
        condition_part = (['    "condition": %r,' %
                           self.condition] if self.condition else [])
        s.extend([
            '  # %s' % self.hierarchy(include_url=False),
            '  "%s": {' % (self.name, ),
            '    "url": "%s",' % (self.url, ),
        ] + condition_part + [
            '  },',
            '',
        ])
        return s

    @property
    def known_dependency_diff(self):
        return self._known_dependency_diff

    @property
    def dependency_index_state(self):
        return self._dependency_index_state

    @property
    def requirements(self):
        """Calculate the list of requirements."""
        requirements = set()
        # self.parent is implicitly a requirement. This will be recursive by
        # definition.
        if self.parent and self.parent.name:
            requirements.add(self.parent.name)

        # For a tree with at least 2 levels*, the leaf node needs to depend
        # on the level higher up in an orderly way.
        # This becomes messy for >2 depth as the DEPS file format is a
        # dictionary, thus unsorted, while the .gclient format is a list thus
        # sorted.
        #
        # Interestingly enough, the following condition only works in the case
        # we want: self is a 2nd level node. 3rd level node wouldn't need this
        # since they already have their parent as a requirement.
        if self.parent and self.parent.parent and not self.parent.parent.parent:
            requirements |= set(i.name for i in self.root.dependencies
                                if i.name)

        if self.name:
            requirements |= set(
                obj.name for obj in self.root.subtree(False)
                if (obj is not self and obj.name
                    and self.name.startswith(posixpath.join(obj.name, ''))))
        requirements = tuple(sorted(requirements))
        logging.info('Dependency(%s).requirements = %s' %
                     (self.name, requirements))
        return requirements

    @property
    def should_recurse(self):
        return self._should_recurse

    def verify_validity(self):
        """Verifies that this Dependency is fine to add as a child of another one.

    Returns True if this entry should be added, False if it is a duplicate of
    another entry.
    """
        logging.info('Dependency(%s).verify_validity()' % self.name)
        if self.name in [s.name for s in self.parent.dependencies]:
            raise gclient_utils.Error(
                'The same name "%s" appears multiple times in the deps section'
                % self.name)
        if not self.should_process:
            # Return early, no need to set requirements.
            return not any(d.name == self.name for d in self.root.subtree(True))

        # This require a full tree traversal with locks.
        siblings = [d for d in self.root.subtree(False) if d.name == self.name]
        for sibling in siblings:
            # Allow to have only one to be None or ''.
            if self.url != sibling.url and bool(self.url) == bool(sibling.url):
                raise gclient_utils.Error(
                    ('Dependency %s specified more than once:\n'
                     '  %s [%s]\n'
                     'vs\n'
                     '  %s [%s]') % (self.name, sibling.hierarchy(),
                                     sibling.url, self.hierarchy(), self.url))
            # In theory we could keep it as a shadow of the other one. In
            # practice, simply ignore it.
            logging.warning("Won't process duplicate dependency %s" % sibling)
            return False
        return True

    def _postprocess_deps(self, deps, rel_prefix):
        # type: (Mapping[str, Mapping[str, str]], str) ->
        #     Mapping[str, Mapping[str, str]]
        """Performs post-processing of deps compared to what's in the DEPS file."""
        # If we don't need to sync, only process custom_deps, if any.
        if not self._should_sync:
            if not self.custom_deps:
                return {}

            processed_deps = {}
            for dep_name, dep_info in self.custom_deps.items():
                if dep_info and not dep_info.endswith('@unmanaged'):
                    if dep_name in deps:
                        # custom_deps that should override an existing deps gets
                        # applied in the Dependency itself with _OverrideUrl().
                        processed_deps[dep_name] = deps[dep_name]
                    else:
                        processed_deps[dep_name] = {
                            'url': dep_info,
                            'dep_type': 'git'
                        }
        else:
            processed_deps = dict(deps)

            # If a line is in custom_deps, but not in the solution, we want to
            # append this line to the solution.
            for dep_name, dep_info in self.custom_deps.items():
                # Don't add it to the solution for the values of "None" and
                # "unmanaged" in order to force these kinds of custom_deps to
                # act as revision overrides (via revision_overrides). Having
                # them function as revision overrides allows them to be applied
                # to recursive dependencies. https://crbug.com/1031185
                if (dep_name not in processed_deps and dep_info
                        and not dep_info.endswith('@unmanaged')):
                    processed_deps[dep_name] = {
                        'url': dep_info,
                        'dep_type': 'git'
                    }

        # Make child deps conditional on any parent conditions. This ensures
        # that, when flattened, recursed entries have the correct restrictions,
        # even if not explicitly set in the recursed DEPS file. For instance, if
        # "src/ios_foo" is conditional on "checkout_ios=True", then anything
        # recursively included by "src/ios_foo/DEPS" should also require
        # "checkout_ios=True".
        if self.condition:
            for value in processed_deps.values():
                gclient_eval.UpdateCondition(value, 'and', self.condition)

        if not rel_prefix:
            return processed_deps

        logging.warning('use_relative_paths enabled.')
        rel_deps = {}
        for d, url in processed_deps.items():
            # normpath is required to allow DEPS to use .. in their
            # dependency local path.
            # We are following the same pattern when use_relative_paths = False,
            # which uses slashes.
            rel_deps[os.path.normpath(os.path.join(rel_prefix, d)).replace(
                os.path.sep, '/')] = url
        logging.warning('Updating deps by prepending %s.', rel_prefix)
        return rel_deps

    def _deps_to_objects(self, deps, use_relative_paths):
        # type: (Mapping[str, Mapping[str, str]], bool) -> Sequence[Dependency]
        """Convert a deps dict to a list of Dependency objects."""
        deps_to_add = []
        cached_conditions = {}

        # TODO(https://crbug.com/343199633): Remove once all packages no longer
        # place content in directories with git content, and all milestone
        # branches have picked it up (m128+).
        gcs_cleanup_blocklist_name = set([
            'src/base/tracing/test/data',
            'src/clank/orderfiles',
            'src/third_party/blink/renderer/core/css/perftest_data',
            'src/third_party/instrumented_libs/binaries',
            'src/third_party/js_code_coverage',
            'src/third_party/opus/tests/resources',
            'src/third_party/test_fonts',
            'src/third_party/tfhub_models',
            'src/tools/perf/page_sets/maps_perf_test/',
        ])

        def _should_process(condition):
            if not condition:
                return True
            if condition not in cached_conditions:
                cached_conditions[condition] = gclient_eval.EvaluateCondition(
                    condition, self.get_vars())
            return cached_conditions[condition]

        for name, dep_value in deps.items():
            should_process = self.should_process
            if dep_value is None:
                continue

            condition = dep_value.get('condition')
            dep_type = dep_value.get('dep_type')

            if not self._get_option('process_all_deps', False):
                should_process = should_process and _should_process(condition)

            # The following option is only set by the 'revinfo' command.
            if dep_type in self._get_option('ignore_dep_type', []):
                continue

            if dep_type == 'cipd':
                # TODO(b/345321320): Remove when non_git_sources are properly supported.
                if gclient_utils.IsEnvCog() and (
                        not condition or "non_git_source" not in condition):
                    continue
                cipd_root = self.GetCipdRoot()
                for package in dep_value.get('packages', []):
                    deps_to_add.append(
                        CipdDependency(parent=self,
                                       name=name,
                                       dep_value=package,
                                       cipd_root=cipd_root,
                                       custom_vars=self.custom_vars,
                                       should_process=should_process,
                                       relative=use_relative_paths,
                                       condition=condition))
            elif dep_type == 'gcs':
                if len(dep_value['objects']) == 0:
                    continue

                # Validate that all objects are unique
                object_name_set = {
                    o['object_name']
                    for o in dep_value['objects']
                }
                if len(object_name_set) != len(dep_value['objects']):
                    raise Exception('Duplicate object names detected in {} GCS '
                                    'dependency.'.format(name))
                gcs_root = self.GetGcsRoot()

                gcs_deps = []
                for obj in dep_value['objects']:
                    merged_condition = gclient_utils.merge_conditions(
                        condition, obj.get('condition'))
                    # TODO(b/345321320): Remove when non_git_sources are properly supported.
                    if gclient_utils.IsEnvCog() and (not merged_condition
                                                     or "non_git_source"
                                                     not in merged_condition):
                        continue

                    should_process_object = should_process and _should_process(
                        merged_condition)

                    gcs_deps.append(
                        GcsDependency(parent=self,
                                      name=name,
                                      bucket=dep_value['bucket'],
                                      object_name=obj['object_name'],
                                      sha256sum=obj['sha256sum'],
                                      output_file=obj.get('output_file'),
                                      size_bytes=obj['size_bytes'],
                                      gcs_root=gcs_root,
                                      custom_vars=self.custom_vars,
                                      should_process=should_process_object,
                                      relative=use_relative_paths,
                                      condition=merged_condition))
                deps_to_add.extend(gcs_deps)

                if name in gcs_cleanup_blocklist_name:
                    continue

                # Check if at least one object needs to be downloaded.
                needs_download = any(gcs.IsDownloadNeeded() for gcs in gcs_deps)
                # When IsEnvCog(), gcs sources are already present and are not managed by gclient.
                if not gclient_utils.IsEnvCog(
                ) and needs_download and os.path.exists(gcs_deps[0].output_dir):
                    # Since we don't know what old content to remove, we remove
                    # the entire output_dir. All gcs_deps are expected to have
                    # the same output_dir, so we get the first one, which must
                    # exist.
                    logging.warning(
                        'GCS dependency %s new version, removing old.', name)
                    shutil.rmtree(gcs_deps[0].output_dir)
            else:
                url = dep_value.get('url')
                deps_to_add.append(
                    GitDependency(
                        parent=self,
                        name=name,
                        # Update URL with scheme in protocol_override
                        url=GitDependency.updateProtocol(url, self.protocol),
                        managed=True,
                        custom_deps=None,
                        custom_vars=self.custom_vars,
                        custom_hooks=None,
                        deps_file=self.recursedeps.get(name, self.deps_file),
                        should_process=should_process,
                        should_recurse=name in self.recursedeps,
                        relative=use_relative_paths,
                        condition=condition,
                        protocol=self.protocol))

        # TODO(crbug.com/1341285): Understand why we need this and remove
        # it if we don't.
        deps_to_add.sort(key=lambda x: x.name)
        return deps_to_add

    def ParseDepsFile(self):
        # type: () -> None
        """Parses the DEPS file for this dependency."""
        assert not self.deps_parsed
        assert not self.dependencies

        deps_content = None

        # First try to locate the configured deps file.  If it's missing,
        # fallback to DEPS.
        deps_files = [self.deps_file]
        if 'DEPS' not in deps_files:
            deps_files.append('DEPS')
        for deps_file in deps_files:
            filepath = os.path.join(self.root.root_dir, self.name, deps_file)
            if os.path.isfile(filepath):
                logging.info('ParseDepsFile(%s): %s file found at %s',
                             self.name, deps_file, filepath)
                break
            logging.info('ParseDepsFile(%s): No %s file found at %s', self.name,
                         deps_file, filepath)

        if not os.path.isfile(filepath):
            logging.warning('ParseDepsFile(%s): No DEPS file found', self.name)
            self.add_dependencies_and_close([], [])
            return

        deps_content = gclient_utils.FileRead(filepath)
        logging.debug('ParseDepsFile(%s) read:\n%s', self.name, deps_content)

        local_scope = {}
        if deps_content:
            try:
                local_scope = gclient_eval.Parse(deps_content, filepath,
                                                 self.get_vars(),
                                                 self.get_builtin_vars())
            except SyntaxError as e:
                gclient_utils.SyntaxErrorToError(filepath, e)

        if 'git_dependencies' in local_scope:
            self.git_dependencies_state = local_scope['git_dependencies']

        if 'allowed_hosts' in local_scope:
            try:
                self._allowed_hosts = frozenset(
                    local_scope.get('allowed_hosts'))
            except TypeError:  # raised if non-iterable
                pass
            if not self._allowed_hosts:
                logging.warning("allowed_hosts is specified but empty %s",
                                self._allowed_hosts)
                raise gclient_utils.Error(
                    'ParseDepsFile(%s): allowed_hosts must be absent '
                    'or a non-empty iterable' % self.name)

        self._gn_args_from = local_scope.get('gclient_gn_args_from')
        self._gn_args_file = local_scope.get('gclient_gn_args_file')
        self._gn_args = local_scope.get('gclient_gn_args', [])
        # It doesn't make sense to set all of these, since setting gn_args_from
        # to another DEPS will make gclient ignore any other local gn_args*
        # settings.
        assert not (self._gn_args_from and self._gn_args_file), \
            'Only specify one of "gclient_gn_args_from" or ' \
            '"gclient_gn_args_file + gclient_gn_args".'

        self._vars = local_scope.get('vars', {})
        if self.parent:
            for key, value in self.parent.get_vars().items():
                if key in self._vars:
                    self._vars[key] = value
        # Since we heavily post-process things, freeze ones which should
        # reflect original state of DEPS.
        self._vars = gclient_utils.freeze(self._vars)

        # If use_relative_paths is set in the DEPS file, regenerate
        # the dictionary using paths relative to the directory containing
        # the DEPS file.  Also update recursedeps if use_relative_paths is
        # enabled.
        # If the deps file doesn't set use_relative_paths, but the parent did
        # (and therefore set self.relative on this Dependency object), then we
        # want to modify the deps and recursedeps by prepending the parent
        # directory of this dependency.
        self._use_relative_paths = local_scope.get('use_relative_paths', False)
        rel_prefix = None
        if self._use_relative_paths:
            rel_prefix = self.name
        elif self._relative:
            rel_prefix = os.path.dirname(self.name)

        if 'recursion' in local_scope:
            logging.warning('%s: Ignoring recursion = %d.', self.name,
                            local_scope['recursion'])

        if 'recursedeps' in local_scope:
            for ent in local_scope['recursedeps']:
                if isinstance(ent, str):
                    self.recursedeps[ent] = self.deps_file
                else:  # (depname, depsfilename)
                    self.recursedeps[ent[0]] = ent[1]
            logging.warning('Found recursedeps %r.', repr(self.recursedeps))

            if rel_prefix:
                logging.warning('Updating recursedeps by prepending %s.',
                                rel_prefix)
                rel_deps = {}
                for depname, options in self.recursedeps.items():
                    rel_deps[os.path.normpath(os.path.join(rel_prefix,
                                                           depname)).replace(
                                                               os.path.sep,
                                                               '/')] = options
                self.recursedeps = rel_deps
        # To get gn_args from another DEPS, that DEPS must be recursed into.
        if self._gn_args_from:
            assert self.recursedeps and self._gn_args_from in self.recursedeps, \
                    'The "gclient_gn_args_from" value must be in recursedeps.'

        # If present, save 'target_os' in the local_target_os property.
        if 'target_os' in local_scope:
            self.local_target_os = local_scope['target_os']

        deps = local_scope.get('deps', {})

        # If dependencies are configured within git submodules, add them to
        # deps. We don't add for SYNC since we expect submodules to be in sync.
        if self.git_dependencies_state == gclient_eval.SUBMODULES:
            deps.update(self.ParseGitSubmodules())

        if self.git_dependencies_state != gclient_eval.DEPS:
            # Git submodules are used - get their state.
            self._known_dependency_diff = self.CreateSCM().GetSubmoduleDiff()
            self._dependency_index_state = self.CreateSCM(
            ).GetSubmoduleStateFromIndex()

        deps_to_add = self._deps_to_objects(
            self._postprocess_deps(deps, rel_prefix), self._use_relative_paths)

        # compute which working directory should be used for hooks
        if local_scope.get('use_relative_hooks', False):
            print('use_relative_hooks is deprecated, please remove it from '
                  '%s DEPS. (it was merged in use_relative_paths)' % self.name,
                  file=sys.stderr)

        hooks_cwd = self.root.root_dir
        if self._use_relative_paths:
            hooks_cwd = os.path.join(hooks_cwd, self.name)
        elif self._relative:
            hooks_cwd = os.path.join(hooks_cwd, os.path.dirname(self.name))
        logging.warning('Using hook base working directory: %s.', hooks_cwd)

        # Only add all hooks if we should sync, otherwise just add custom hooks.
        # override named sets of hooks by the custom hooks
        hooks_to_run = []
        if self._should_sync:
            hook_names_to_suppress = [
                c.get('name', '') for c in self.custom_hooks
            ]
            for hook in local_scope.get('hooks', []):
                if hook.get('name', '') not in hook_names_to_suppress:
                    hooks_to_run.append(hook)

        # add the replacements and any additions
        for hook in self.custom_hooks:
            if 'action' in hook:
                hooks_to_run.append(hook)

        if self.should_recurse and deps_to_add:
            self._pre_deps_hooks = [
                Hook.from_dict(hook,
                               variables=self.get_vars(),
                               verbose=True,
                               conditions=self.condition,
                               cwd_base=hooks_cwd)
                for hook in local_scope.get('pre_deps_hooks', [])
            ]

        self.add_dependencies_and_close(deps_to_add,
                                        hooks_to_run,
                                        hooks_cwd=hooks_cwd)
        logging.info('ParseDepsFile(%s) done' % self.name)

    def ParseGitSubmodules(self):
        # type: () -> Mapping[str, str]
        """
    Parses git submodules and returns a dict of path to DEPS git url entries.

    e.g {<path>: <url>@<commit_hash>}
    """
        cwd = os.path.join(self.root.root_dir, self.name)
        filepath = os.path.join(cwd, '.gitmodules')
        if not os.path.isfile(filepath):
            logging.warning('ParseGitSubmodules(): No .gitmodules found at %s',
                            filepath)
            return {}

        # Get .gitmodules fields
        gitmodules_entries = subprocess2.check_output(
            ['git', 'config', '--file', filepath, '-l']).decode('utf-8')

        gitmodules = {}
        for entry in gitmodules_entries.splitlines():
            key, value = entry.split('=', maxsplit=1)

            # git config keys consist of section.name.key, e.g.,
            # submodule.foo.path
            section, submodule_key = key.split('.', maxsplit=1)

            # Only parse [submodule "foo"] sections from .gitmodules.
            if section.strip() != 'submodule':
                continue

            # The name of the submodule can contain '.', hence split from the
            # back.
            submodule, sub_key = submodule_key.rsplit('.', maxsplit=1)

            if submodule not in gitmodules:
                gitmodules[submodule] = {}

            if sub_key in ('url', 'gclient-condition', 'path'):
                gitmodules[submodule][sub_key] = value

        paths = [module['path'] for module in gitmodules.values()]
        commit_hashes = scm_git.GIT.GetSubmoduleCommits(cwd, paths)

        # Structure git submodules into a dict of DEPS git url entries.
        submodules = {}
        for module in gitmodules.values():
            if self._use_relative_paths:
                path = module['path']
            else:
                path = f'{self.name}/{module["path"]}'
            # TODO(crbug.com/1471685): Temporary hack. In case of applied
            # patches where the changes are staged but not committed, any
            # gitlinks from the patch are not returned by `git ls-tree`. The
            # path won't be found in commit_hashes. Use a temporary '0000000'
            # value that will be replaced with w/e is found in DEPS later.
            submodules[path] = {
                'dep_type':
                'git',
                'url':
                '{}@{}'.format(module['url'],
                               commit_hashes.get(module['path'], '0000000'))
            }

            if 'gclient-condition' in module:
                submodules[path]['condition'] = module['gclient-condition']

        return submodules

    def _get_option(self, attr, default):
        obj = self
        while not hasattr(obj, '_options'):
            obj = obj.parent
        return getattr(obj._options, attr, default)

    def add_dependencies_and_close(self, deps_to_add, hooks, hooks_cwd=None):
        """Adds the dependencies, hooks and mark the parsing as done."""
        if hooks_cwd == None:
            hooks_cwd = self.root.root_dir

        for dep in deps_to_add:
            if dep.verify_validity():
                self.add_dependency(dep)
        self._mark_as_parsed([
            Hook.from_dict(h,
                           variables=self.get_vars(),
                           verbose=self.root._options.verbose,
                           conditions=self.condition,
                           cwd_base=hooks_cwd) for h in hooks
        ])

    def findDepsFromNotAllowedHosts(self):
        """Returns a list of dependencies from not allowed hosts.

    If allowed_hosts is not set, allows all hosts and returns empty list.
    """
        if not self._allowed_hosts:
            return []
        bad_deps = []
        for dep in self._dependencies:
            # Don't enforce this for custom_deps.
            if dep.name in self._custom_deps:
                continue
            if isinstance(dep.url, str):
                parsed_url = urllib.parse.urlparse(dep.url)
                if parsed_url.netloc and parsed_url.netloc not in self._allowed_hosts:
                    bad_deps.append(dep)
        return bad_deps

    def FuzzyMatchUrl(self, candidates):
        # type: (Union[Mapping[str, str], Collection[str]]) -> Optional[str]
        """Attempts to find this dependency in the list of candidates.

    It looks first for the URL of this dependency in the list of
    candidates. If it doesn't succeed, and the URL ends in '.git', it will try
    looking for the URL minus '.git'. Finally it will try to look for the name
    of the dependency.

    Args:
      candidates: list, dict. The list of candidates in which to look for this
          dependency. It can contain URLs as above, or dependency names like
          "src/some/dep".

    Returns:
      If this dependency is not found in the list of candidates, returns None.
      Otherwise, it returns under which name did we find this dependency:
       - Its parsed url: "https://example.com/src.git'
       - Its parsed url minus '.git': "https://example.com/src"
       - Its name: "src"
    """
        if self.url:
            origin, _ = gclient_utils.SplitUrlRevision(self.url)
            match = gclient_utils.FuzzyMatchRepo(origin, candidates)
            if match:
                return match
        if self.name in candidates:
            return self.name
        return None

    # Arguments number differs from overridden method
    # pylint: disable=arguments-differ
    def run(
            self,
            revision_overrides,  # type: Mapping[str, str]
            command,  # type: str
            args,  # type: Sequence[str]
            work_queue,  # type: ExecutionQueue
            options,  # type: optparse.Values
            patch_refs,  # type: Mapping[str, str]
            target_branches,  # type: Mapping[str, str]
            skip_sync_revisions,  # type: Mapping[str, str]
    ):
        # type: () -> None
        """Runs |command| then parse the DEPS file."""
        logging.info('Dependency(%s).run()' % self.name)
        assert self._file_list == []
        # When running runhooks, there's no need to consult the SCM.
        # All known hooks are expected to run unconditionally regardless of
        # working copy state, so skip the SCM status check.
        run_scm = command not in ('flatten', 'runhooks', 'recurse', 'validate',
                                  'revinfo', None)
        file_list = [] if not options.nohooks else None
        revision_override = revision_overrides.pop(
            self.FuzzyMatchUrl(revision_overrides), None)
        if not revision_override and not self.managed:
            revision_override = 'unmanaged'
        if run_scm and self.url:
            # Create a shallow copy to mutate revision.
            options = copy.copy(options)
            options.revision = revision_override
            self._used_revision = options.revision
            self._used_scm = self.CreateSCM(out_cb=work_queue.out_cb)
            latest_commit = None
            if command != 'update' or self.GetScmName() != 'git':
                self._got_revision = self._used_scm.RunCommand(
                    command, options, args, file_list)
            else:
                # We are running update.
                try:
                    start = time.time()
                    sync_status = metrics_utils.SYNC_STATUS_FAILURE
                    if self.parent and self.parent.known_dependency_diff is not None:
                        if self._use_relative_paths:
                            path = self.name
                        else:
                            path = self.name[len(self.parent.name) + 1:]
                        current_revision = None
                        if path in self.parent.dependency_index_state:
                            current_revision = self.parent.dependency_index_state[
                                path]
                        if path in self.parent.known_dependency_diff:
                            current_revision = self.parent.known_dependency_diff[
                                path][1]
                        self._used_scm.current_revision = current_revision

                    self._got_revision = self._used_scm.RunCommand(
                        command, options, args, file_list)
                    latest_commit = self._got_revision
                    sync_status = metrics_utils.SYNC_STATUS_SUCCESS
                finally:
                    url, revision = gclient_utils.SplitUrlRevision(self.url)
                    metrics.collector.add_repeated(
                        'git_deps', {
                            'path': self.name,
                            'url': url,
                            'revision': revision,
                            'execution_time': time.time() - start,
                            'sync_status': sync_status,
                        })

            if isinstance(self, GitDependency) and command == 'update':
                patch_repo = self.url.split('@')[0]
                patch_ref = patch_refs.pop(self.FuzzyMatchUrl(patch_refs), None)
                target_branch = target_branches.pop(
                    self.FuzzyMatchUrl(target_branches), None)
                if patch_ref:
                    latest_commit = self._used_scm.apply_patch_ref(
                        patch_repo, patch_ref, target_branch, options,
                        file_list)
                elif latest_commit is None:
                    latest_commit = self._used_scm.revinfo(None, None, None)
                existing_sync_commits = json.loads(
                    os.environ.get(PREVIOUS_SYNC_COMMITS, '{}'))
                existing_sync_commits[self.name] = latest_commit
                os.environ[PREVIOUS_SYNC_COMMITS] = json.dumps(
                    existing_sync_commits)

            if file_list:
                file_list = [
                    os.path.join(self.name, f.strip()) for f in file_list
                ]

            # TODO(phajdan.jr): We should know exactly when the paths are
            # absolute. Convert all absolute paths to relative.
            for i in range(len(file_list or [])):
                # It depends on the command being executed (like runhooks vs
                # sync).
                if not os.path.isabs(file_list[i]):
                    continue
                prefix = os.path.commonprefix(
                    [self.root.root_dir.lower(), file_list[i].lower()])
                file_list[i] = file_list[i][len(prefix):]
                # Strip any leading path separators.
                while file_list[i].startswith(('\\', '/')):
                    file_list[i] = file_list[i][1:]

        # We must check for diffs AFTER any patch_refs have been applied.
        if skip_sync_revisions:
            skip_sync_rev = skip_sync_revisions.pop(
                self.FuzzyMatchUrl(skip_sync_revisions), None)
            self._should_sync = (skip_sync_rev is None
                                 or self._used_scm.check_diff(skip_sync_rev,
                                                              files=['DEPS']))
            if not self._should_sync:
                logging.debug(
                    'Skipping sync for %s. No DEPS changes since last '
                    'sync at %s' % (self.name, skip_sync_rev))
            else:
                logging.debug('DEPS changes detected for %s since last sync at '
                              '%s. Not skipping deps sync' %
                              (self.name, skip_sync_rev))

        if self.should_recurse:
            self.ParseDepsFile()
            gcs_root = self.GetGcsRoot()
            if gcs_root:
                if command == 'revert':
                    gcs_root.clobber()
                elif command == 'update':
                    gcs_root.clobber_deps_with_updated_objects(self.name)

        self._run_is_done(file_list or [])

        # TODO(crbug.com/1339471): If should_recurse is false, ParseDepsFile
        # never gets called meaning we never fetch hooks and dependencies. So
        # there's no need to check should_recurse again here.
        if self.should_recurse:
            if command in ('update', 'revert') and not options.noprehooks:
                self.RunPreDepsHooks()
            # Parse the dependencies of this dependency.
            for s in self.dependencies:
                if s.should_process:
                    work_queue.enqueue(s)
            gcs_root = self.GetGcsRoot()
            if gcs_root and command == 'update':
                gcs_root.resolve_objects(self.name)

        if command == 'recurse':
            # Skip file only checkout.
            scm = self.GetScmName()
            if not options.scm or scm in options.scm:
                cwd = os.path.normpath(
                    os.path.join(self.root.root_dir, self.name))
                # Pass in the SCM type as an env variable.  Make sure we don't
                # put unicode strings in the environment.
                env = os.environ.copy()
                if scm:
                    env['GCLIENT_SCM'] = str(scm)
                if self.url:
                    env['GCLIENT_URL'] = str(self.url)
                env['GCLIENT_DEP_PATH'] = str(self.name)
                if options.prepend_dir and scm == 'git':
                    print_stdout = False

                    def filter_fn(line):
                        """Git-specific path marshaling. It is optimized for git-grep."""
                        def mod_path(git_pathspec):
                            match = re.match('^(\\S+?:)?([^\0]+)$',
                                             git_pathspec)
                            modified_path = os.path.join(
                                self.name, match.group(2))
                            branch = match.group(1) or ''
                            return '%s%s' % (branch, modified_path)

                        match = re.match('^Binary file ([^\0]+) matches$', line)
                        if match:
                            print('Binary file %s matches\n' %
                                  mod_path(match.group(1)))
                            return

                        items = line.split('\0')
                        if len(items) == 2 and items[1]:
                            print('%s : %s' % (mod_path(items[0]), items[1]))
                        elif len(items) >= 2:
                            # Multiple null bytes or a single trailing null byte
                            # indicate git is likely displaying filenames only
                            # (such as with -l)
                            print('\n'.join(
                                mod_path(path) for path in items if path))
                        else:
                            print(line)
                else:
                    print_stdout = True
                    filter_fn = None

                if self.url is None:
                    print('Skipped omitted dependency %s' % cwd,
                          file=sys.stderr)
                elif os.path.isdir(cwd):
                    try:
                        gclient_utils.CheckCallAndFilter(
                            args,
                            cwd=cwd,
                            env=env,
                            print_stdout=print_stdout,
                            filter_fn=filter_fn,
                        )
                    except subprocess2.CalledProcessError:
                        if not options.ignore:
                            raise
                else:
                    print('Skipped missing %s' % cwd, file=sys.stderr)

    def GetScmName(self):
        raise NotImplementedError()

    def CreateSCM(self, out_cb=None):
        raise NotImplementedError()

    def HasGNArgsFile(self):
        return self._gn_args_file is not None

    def WriteGNArgsFile(self):
        lines = ['# Generated from %r' % self.deps_file]
        variables = self.get_vars()
        for arg in self._gn_args:
            value = variables[arg]
            if isinstance(value, gclient_eval.ConstantString):
                value = value.value
            elif isinstance(value, str):
                value = gclient_eval.EvaluateCondition(value, variables)
            lines.append('%s = %s' % (arg, ToGNString(value)))

        # When use_relative_paths is set, gn_args_file is relative to this DEPS
        path_prefix = self.root.root_dir
        if self._use_relative_paths:
            path_prefix = os.path.join(path_prefix, self.name)

        gn_args_path = os.path.join(path_prefix, self._gn_args_file)

        new_content = '\n'.join(lines).encode('utf-8', 'replace')

        if os.path.exists(gn_args_path):
            with open(gn_args_path, 'rb') as f:
                old_content = f.read()
            if old_content == new_content:
                return

        with open(gn_args_path, 'wb') as f:
            f.write(new_content)

    @gclient_utils.lockedmethod
    def _run_is_done(self, file_list):
        # Both these are kept for hooks that are run as a separate tree
        # traversal.
        self._file_list = file_list
        self._processed = True

    def GetHooks(self, options):
        """Evaluates all hooks, and return them in a flat list.

    RunOnDeps() must have been called before to load the DEPS.
    """
        result = []
        if not self.should_process or not self.should_recurse:
            # Don't run the hook when it is above recursion_limit.
            return result
        # If "--force" was specified, run all hooks regardless of what files
        # have changed.
        if self.deps_hooks:
            # TODO(maruel): If the user is using git, then we don't know
            # what files have changed so we always run all hooks. It'd be nice
            # to fix that.
            result.extend(self.deps_hooks)
        for s in self.dependencies:
            result.extend(s.GetHooks(options))
        return result

    def RunHooksRecursively(self, options, progress):
        assert self.hooks_ran == False
        self._hooks_ran = True
        hooks = self.GetHooks(options)
        if progress:
            progress._total = len(hooks)
        for hook in hooks:
            if progress:
                progress.update(extra=hook.name or '')
            hook.run()
        if progress:
            progress.end()

    def RunPreDepsHooks(self):
        assert self.processed
        assert self.deps_parsed
        assert not self.pre_deps_hooks_ran
        assert not self.hooks_ran
        for s in self.dependencies:
            assert not s.processed
        self._pre_deps_hooks_ran = True
        for hook in self.pre_deps_hooks:
            hook.run()

    def GetCipdRoot(self):
        if self.root is self:
            # Let's not infinitely recurse. If this is root and isn't an
            # instance of GClient, do nothing.
            return None
        return self.root.GetCipdRoot()

    def GetGcsRoot(self):
        if self.root is self:
            # Let's not infinitely recurse. If this is root and isn't an
            # instance of GClient, do nothing.
            return None
        return self.root.GetGcsRoot()

    def subtree(self, include_all):
        """Breadth first recursion excluding root node."""
        dependencies = self.dependencies
        for d in dependencies:
            if d.should_process or include_all:
                yield d
        for d in dependencies:
            for i in d.subtree(include_all):
                yield i

    @gclient_utils.lockedmethod
    def add_dependency(self, new_dep):
        self._dependencies.append(new_dep)

    @gclient_utils.lockedmethod
    def _mark_as_parsed(self, new_hooks):
        self._deps_hooks.extend(new_hooks)
        self._deps_parsed = True

    @property
    @gclient_utils.lockedmethod
    def dependencies(self):
        return tuple(self._dependencies)

    @property
    @gclient_utils.lockedmethod
    def deps_hooks(self):
        return tuple(self._deps_hooks)

    @property
    @gclient_utils.lockedmethod
    def pre_deps_hooks(self):
        return tuple(self._pre_deps_hooks)

    @property
    @gclient_utils.lockedmethod
    def deps_parsed(self):
        """This is purely for debugging purposes. It's not used anywhere."""
        return self._deps_parsed

    @property
    @gclient_utils.lockedmethod
    def processed(self):
        return self._processed

    @property
    @gclient_utils.lockedmethod
    def pre_deps_hooks_ran(self):
        return self._pre_deps_hooks_ran

    @property
    @gclient_utils.lockedmethod
    def hooks_ran(self):
        return self._hooks_ran

    @property
    @gclient_utils.lockedmethod
    def allowed_hosts(self):
        return self._allowed_hosts

    @property
    @gclient_utils.lockedmethod
    def file_list(self):
        return tuple(self._file_list)

    @property
    def used_scm(self):
        """SCMWrapper instance for this dependency or None if not processed yet."""
        return self._used_scm

    @property
    @gclient_utils.lockedmethod
    def got_revision(self):
        return self._got_revision

    @property
    def file_list_and_children(self):
        result = list(self.file_list)
        for d in self.dependencies:
            result.extend(d.file_list_and_children)
        return tuple(result)

    def __str__(self):
        out = []
        for i in ('name', 'url', 'custom_deps', 'custom_vars', 'deps_hooks',
                  'file_list', 'should_process', 'processed', 'hooks_ran',
                  'deps_parsed', 'requirements', 'allowed_hosts'):
            # First try the native property if it exists.
            if hasattr(self, '_' + i):
                value = getattr(self, '_' + i, False)
            else:
                value = getattr(self, i, False)
            if value:
                out.append('%s: %s' % (i, value))

        for d in self.dependencies:
            out.extend(['  ' + x for x in str(d).splitlines()])
            out.append('')
        return '\n'.join(out)

    def __repr__(self):
        return '%s: %s' % (self.name, self.url)

    def hierarchy(self, include_url=True, graphviz=False):
        """Returns a human-readable hierarchical reference to a Dependency."""
        def format_name(d):
            if include_url:
                return '%s(%s)' % (d.name, d.url)
            return '"%s"' % d.name  # quotes required for graph dot file.

        out = format_name(self)
        i = self.parent
        while i and i.name:
            out = '%s -> %s' % (format_name(i), out)
            if graphviz:
                # for graphviz we just need each parent->child relationship
                # listed once.
                return out
            i = i.parent
        return out

    def hierarchy_data(self):
        """Returns a machine-readable hierarchical reference to a Dependency."""
        d = self
        out = []
        while d and d.name:
            out.insert(0, (d.name, d.url))
            d = d.parent
        return tuple(out)

    def get_builtin_vars(self):
        linux_names = ['linux', 'unix']
        mac_names = ['mac', 'osx']
        win_names = ['win', 'windows']
        return {
            'checkout_android': 'android' in self.target_os,
            'checkout_chromeos': 'chromeos' in self.target_os,
            'checkout_fuchsia': 'fuchsia' in self.target_os,
            'checkout_ios': 'ios' in self.target_os,
            'checkout_linux': any(n in self.target_os for n in linux_names),
            'checkout_mac': any(n in self.target_os for n in mac_names),
            'checkout_win': any(n in self.target_os for n in win_names),
            'host_os': _detect_host_os(),
            'checkout_arm': 'arm' in self.target_cpu,
            'checkout_arm64': 'arm64' in self.target_cpu,
            'checkout_x86': 'x86' in self.target_cpu,
            'checkout_mips': 'mips' in self.target_cpu,
            'checkout_mips64': 'mips64' in self.target_cpu,
            'checkout_ppc': 'ppc' in self.target_cpu,
            'checkout_s390': 's390' in self.target_cpu,
            'checkout_x64': 'x64' in self.target_cpu,
            'host_cpu': detect_host_arch.HostArch(),
        }

    def get_vars(self):
        """Returns a dictionary of effective variable values
    (DEPS file contents with applied custom_vars overrides)."""
        # Variable precedence (last has highest):
        # - DEPS vars
        # - parents, from first to last
        # - built-in
        # - custom_vars overrides
        result = {}
        result.update(self._vars)
        if self.parent:
            merge_vars(result, self.parent.get_vars())
        # Provide some built-in variables.
        result.update(self.get_builtin_vars())
        merge_vars(result, self.custom_vars)

        return result


_PLATFORM_MAPPING = {
    'cygwin': 'win',
    'darwin': 'mac',
    'linux2': 'linux',
    'linux': 'linux',
    'win32': 'win',
    'aix6': 'aix',
    'zos': 'zos',
}


def merge_vars(result, new_vars):
    for k, v in new_vars.items():
        if k in result:
            if isinstance(result[k], gclient_eval.ConstantString):
                if isinstance(v, gclient_eval.ConstantString):
                    result[k] = v
                else:
                    result[k].value = v
            else:
                result[k] = v
        else:
            result[k] = v


def _detect_host_os():
    if sys.platform in _PLATFORM_MAPPING:
        return _PLATFORM_MAPPING[sys.platform]

    try:
        return os.uname().sysname.lower()
    except AttributeError:
        return sys.platform


class GitDependency(Dependency):
    """A Dependency object that represents a single git checkout."""
    _is_env_cog = None

    @staticmethod
    def _IsCog():
        """Returns true if the env is cog"""
        if GitDependency._is_env_cog is None:
            GitDependency._is_env_cog = gclient_utils.IsEnvCog()

        return GitDependency._is_env_cog

    @staticmethod
    def updateProtocol(url, protocol):
        """Updates given URL's protocol"""
        # only works on urls, skips local paths
        if not url or not protocol or not re.match('([a-z]+)://', url) or \
          re.match('file://', url):
            return url

        return re.sub('^([a-z]+):', protocol + ':', url)

    #override
    def GetScmName(self):
        """Always 'git'."""
        return 'git'

    #override
    def CreateSCM(self, out_cb=None):
        """Create a Wrapper instance suitable for handling this git dependency."""
        if self._IsCog():
            return gclient_scm.CogWrapper()

        return gclient_scm.GitWrapper(self.url,
                                      self.root.root_dir,
                                      self.name,
                                      self.outbuf,
                                      out_cb,
                                      print_outbuf=self.print_outbuf)


class GClient(GitDependency):
    """Object that represent a gclient checkout. A tree of Dependency(), one per
  solution or DEPS entry."""

    DEPS_OS_CHOICES = {
        "aix6": "unix",
        "win32": "win",
        "win": "win",
        "cygwin": "win",
        "darwin": "mac",
        "mac": "mac",
        "unix": "unix",
        "linux": "unix",
        "linux2": "unix",
        "linux3": "unix",
        "android": "android",
        "ios": "ios",
        "fuchsia": "fuchsia",
        "chromeos": "chromeos",
        "zos": "zos",
    }

    DEFAULT_CLIENT_FILE_TEXT = ("""\
solutions = [
  { "name"        : %(solution_name)r,
    "url"         : %(solution_url)r,
    "deps_file"   : %(deps_file)r,
    "managed"     : %(managed)r,
    "custom_deps" : {
    },
    "custom_vars": %(custom_vars)r,
  },
]
""")

    DEFAULT_CLIENT_CACHE_DIR_TEXT = ("""\
cache_dir = %(cache_dir)r
""")

    DEFAULT_SNAPSHOT_FILE_TEXT = ("""\
# Snapshot generated with gclient revinfo --snapshot
solutions = %(solution_list)s
""")

    def __init__(self, root_dir, options):
        # Do not change previous behavior. Only solution level and immediate
        # DEPS are processed.
        self._recursion_limit = 2
        super(GClient, self).__init__(parent=None,
                                      name=None,
                                      url=None,
                                      managed=True,
                                      custom_deps=None,
                                      custom_vars=None,
                                      custom_hooks=None,
                                      deps_file='unused',
                                      should_process=True,
                                      should_recurse=True,
                                      relative=None,
                                      condition=None,
                                      print_outbuf=True)

        self._options = options
        if options.deps_os:
            enforced_os = options.deps_os.split(',')
        else:
            enforced_os = [self.DEPS_OS_CHOICES.get(sys.platform, 'unix')]
        if 'all' in enforced_os:
            enforced_os = self.DEPS_OS_CHOICES.values()
        self._enforced_os = tuple(set(enforced_os))
        self._enforced_cpu = (detect_host_arch.HostArch(), )
        self._root_dir = root_dir
        self._cipd_root = None
        self._gcs_root = None
        self.config_content = None

    def _CheckConfig(self):
        """Verify that the config matches the state of the existing checked-out
        solutions."""
        for dep in self.dependencies:
            if dep.managed and dep.url:
                scm = dep.CreateSCM()
                actual_url = scm.GetActualRemoteURL(self._options)
                if actual_url and not scm.DoesRemoteURLMatch(self._options):
                    mirror = scm.GetCacheMirror()
                    if mirror:
                        mirror_string = '%s (exists=%s)' % (mirror.mirror_path,
                                                            mirror.exists())
                    else:
                        mirror_string = 'not used'
                    raise gclient_utils.Error(
                        '''
Your .gclient file seems to be broken. The requested URL is different from what
is actually checked out in %(checkout_path)s.

The .gclient file contains:
URL: %(expected_url)s (%(expected_scm)s)
Cache mirror: %(mirror_string)s

The local checkout in %(checkout_path)s reports:
%(actual_url)s (%(actual_scm)s)

You should ensure that the URL listed in .gclient is correct and either change
it or fix the checkout.
''' % {
                            'checkout_path': os.path.join(
                                self.root_dir, dep.name),
                            'expected_url': dep.url,
                            'expected_scm': dep.GetScmName(),
                            'mirror_string': mirror_string,
                            'actual_url': actual_url,
                            'actual_scm': dep.GetScmName()
                        })

    def SetConfig(self, content):
        assert not self.dependencies
        config_dict = {}
        self.config_content = content
        try:
            exec(content, config_dict)
        except SyntaxError as e:
            gclient_utils.SyntaxErrorToError('.gclient', e)

        # Append any target OS that is not already being enforced to the tuple.
        target_os = config_dict.get('target_os', [])
        if config_dict.get('target_os_only', False):
            self._enforced_os = tuple(set(target_os))
        else:
            self._enforced_os = tuple(set(self._enforced_os).union(target_os))

        # Append any target CPU that is not already being enforced to the tuple.
        target_cpu = config_dict.get('target_cpu', [])
        if config_dict.get('target_cpu_only', False):
            self._enforced_cpu = tuple(set(target_cpu))
        else:
            self._enforced_cpu = tuple(
                set(self._enforced_cpu).union(target_cpu))

        cache_dir = config_dict.get('cache_dir', UNSET_CACHE_DIR)
        if cache_dir is not UNSET_CACHE_DIR:
            if cache_dir:
                cache_dir = os.path.join(self.root_dir, cache_dir)
                cache_dir = os.path.abspath(cache_dir)

            git_cache.Mirror.SetCachePath(cache_dir)

        if not target_os and config_dict.get('target_os_only', False):
            raise gclient_utils.Error(
                'Can\'t use target_os_only if target_os is '
                'not specified')

        if not target_cpu and config_dict.get('target_cpu_only', False):
            raise gclient_utils.Error(
                'Can\'t use target_cpu_only if target_cpu is '
                'not specified')

        deps_to_add = []
        for s in config_dict.get('solutions', []):
            try:
                deps_to_add.append(
                    GitDependency(
                        parent=self,
                        name=s['name'],
                        # Update URL with scheme in protocol_override
                        url=GitDependency.updateProtocol(
                            s['url'], s.get('protocol_override', None)),
                        managed=s.get('managed', True),
                        custom_deps=s.get('custom_deps', {}),
                        custom_vars=s.get('custom_vars', {}),
                        custom_hooks=s.get('custom_hooks', []),
                        deps_file=s.get('deps_file', 'DEPS'),
                        should_process=True,
                        should_recurse=True,
                        relative=None,
                        condition=None,
                        print_outbuf=True,
                        # Pass protocol_override down the tree for child deps to
                        # use.
                        protocol=s.get('protocol_override', None),
                        git_dependencies_state=self.git_dependencies_state))
            except KeyError:
                raise gclient_utils.Error('Invalid .gclient file. Solution is '
                                          'incomplete: %s' % s)
        metrics.collector.add('project_urls', [
            dep.FuzzyMatchUrl(metrics_utils.KNOWN_PROJECT_URLS)
            for dep in deps_to_add
            if dep.FuzzyMatchUrl(metrics_utils.KNOWN_PROJECT_URLS)
        ])

        self.add_dependencies_and_close(deps_to_add,
                                        config_dict.get('hooks', []))
        logging.info('SetConfig() done')

    def SaveConfig(self):
        gclient_utils.FileWrite(
            os.path.join(self.root_dir, self._options.config_filename),
            self.config_content)

    @staticmethod
    def LoadCurrentConfig(options):
        # type: (optparse.Values) -> GClient
        """Searches for and loads a .gclient file relative to the current working
        dir."""
        if options.spec:
            client = GClient('.', options)
            client.SetConfig(options.spec)
        else:
            if options.verbose:
                print('Looking for %s starting from %s\n' %
                      (options.config_filename, os.getcwd()))
            path = gclient_paths.FindGclientRoot(os.getcwd(),
                                                 options.config_filename)
            if not path:
                if options.verbose:
                    print('Couldn\'t find configuration file.')
                return None
            client = GClient(path, options)
            client.SetConfig(
                gclient_utils.FileRead(
                    os.path.join(path, options.config_filename)))

        if (options.revisions and len(client.dependencies) > 1
                and any('@' not in r for r in options.revisions)):
            print((
                'You must specify the full solution name like --revision %s@%s\n'
                'when you have multiple solutions setup in your .gclient file.\n'
                'Other solutions present are: %s.') %
                  (client.dependencies[0].name, options.revisions[0], ', '.join(
                      s.name for s in client.dependencies[1:])),
                  file=sys.stderr)

        return client

    def SetDefaultConfig(self,
                         solution_name,
                         deps_file,
                         solution_url,
                         managed=True,
                         cache_dir=UNSET_CACHE_DIR,
                         custom_vars=None):
        text = self.DEFAULT_CLIENT_FILE_TEXT
        format_dict = {
            'solution_name': solution_name,
            'solution_url': solution_url,
            'deps_file': deps_file,
            'managed': managed,
            'custom_vars': custom_vars or {},
        }

        if cache_dir is not UNSET_CACHE_DIR:
            text += self.DEFAULT_CLIENT_CACHE_DIR_TEXT
            format_dict['cache_dir'] = cache_dir

        self.SetConfig(text % format_dict)

    def _SaveEntries(self):
        """Creates a .gclient_entries file to record the list of unique checkouts.

        The .gclient_entries file lives in the same directory as .gclient.
        """
        # Sometimes pprint.pformat will use {', sometimes it'll use { ' ... It
        # makes testing a bit too fun.
        result = 'entries = {\n'
        for entry in self.root.subtree(False):
            result += '  %s: %s,\n' % (pprint.pformat(
                entry.name), pprint.pformat(entry.url))
        result += '}\n'
        file_path = os.path.join(self.root_dir, self._options.entries_filename)
        logging.debug(result)
        gclient_utils.FileWrite(file_path, result)

    def _ReadEntries(self):
        """Read the .gclient_entries file for the given client.

        Returns:
            A sequence of solution names, which will be empty if there is the
            entries file hasn't been created yet.
        """
        scope = {}
        filename = os.path.join(self.root_dir, self._options.entries_filename)
        if not os.path.exists(filename):
            return {}
        try:
            exec(gclient_utils.FileRead(filename), scope)
        except SyntaxError as e:
            gclient_utils.SyntaxErrorToError(filename, e)
        return scope.get('entries', {})

    def _ExtractFileJsonContents(self, default_filename):
        # type: (str) -> Mapping[str,Any]
        f = os.path.join(self.root_dir, default_filename)

        if not os.path.exists(f):
            logging.info('File %s does not exist.' % f)
            return {}

        with open(f, 'r') as open_f:
            logging.info('Reading content from file %s' % f)
            content = open_f.read().rstrip()
            if content:
                return json.loads(content)
        return {}

    def _WriteFileContents(self, default_filename, content):
        # type: (str, str) -> None
        f = os.path.join(self.root_dir, default_filename)

        with open(f, 'w') as open_f:
            logging.info('Writing to file %s' % f)
            open_f.write(content)

    def _EnforceSkipSyncRevisions(self, patch_refs):
        # type: (Mapping[str, str]) -> Mapping[str, str]
        """Checks for and enforces revisions for skipping deps syncing."""
        previous_sync_commits = self._ExtractFileJsonContents(
            PREVIOUS_SYNC_COMMITS_FILE)

        if not previous_sync_commits:
            return {}

        # Current `self.dependencies` only contain solutions. If a patch_ref is
        # not for a solution, then it is for a solution's dependency or recursed
        # dependency which we cannot support while skipping sync.
        if patch_refs:
            unclaimed_prs = []
            candidates = []
            for dep in self.dependencies:
                origin, _ = gclient_utils.SplitUrlRevision(dep.url)
                candidates.extend([origin, dep.name])
            for patch_repo in patch_refs:
                if not gclient_utils.FuzzyMatchRepo(patch_repo, candidates):
                    unclaimed_prs.append(patch_repo)
            if unclaimed_prs:
                print(
                    'We cannot skip syncs when there are --patch-refs flags for '
                    'non-solution dependencies. To skip syncing, remove patch_refs '
                    'for: \n%s' % '\n'.join(unclaimed_prs))
                return {}

        # We cannot skip syncing if there are custom_vars that differ from the
        # previous run's custom_vars.
        previous_custom_vars = self._ExtractFileJsonContents(
            PREVIOUS_CUSTOM_VARS_FILE)

        cvs_by_name = {s.name: s.custom_vars for s in self.dependencies}

        skip_sync_revisions = {}
        for name, commit in previous_sync_commits.items():
            previous_vars = previous_custom_vars.get(name)
            if previous_vars == cvs_by_name.get(name) or (
                    not previous_vars and not cvs_by_name.get(name)):
                skip_sync_revisions[name] = commit
            else:
                print(
                    'We cannot skip syncs when custom_vars for solutions have '
                    'changed since the last sync run on this machine.\n'
                    '\nRemoving skip_sync_revision for:\n'
                    'solution: %s, current: %r, previous: %r.' %
                    (name, cvs_by_name.get(name), previous_vars))
        print('no-sync experiment enabled with %r' % skip_sync_revisions)
        return skip_sync_revisions

    # TODO(crbug.com/1340695): Remove handling revisions without '@'.
    def _EnforceRevisions(self):
        """Checks for revision overrides."""
        revision_overrides = {}
        if self._options.head:
            return revision_overrides
        if not self._options.revisions:
            return revision_overrides
        solutions_names = [s.name for s in self.dependencies]
        for index, revision in enumerate(self._options.revisions):
            if not '@' in revision:
                # Support for --revision 123
                revision = '%s@%s' % (solutions_names[index], revision)
            name, rev = revision.split('@', 1)
            revision_overrides[name] = rev
        return revision_overrides

    def _EnforcePatchRefsAndBranches(self):
        # type: () -> Tuple[Mapping[str, str], Mapping[str, str]]
        """Checks for patch refs."""
        patch_refs = {}
        target_branches = {}
        if not self._options.patch_refs:
            return patch_refs, target_branches
        for given_patch_ref in self._options.patch_refs:
            patch_repo, _, patch_ref = given_patch_ref.partition('@')
            if not patch_repo or not patch_ref or ':' not in patch_ref:
                raise gclient_utils.Error(
                    'Wrong revision format: %s should be of the form '
                    'patch_repo@target_branch:patch_ref.' % given_patch_ref)
            target_branch, _, patch_ref = patch_ref.partition(':')
            target_branches[patch_repo] = target_branch
            patch_refs[patch_repo] = patch_ref
        return patch_refs, target_branches

    def _InstallPreCommitHook(self):
        # On Windows, this path is written to the file as
        # "dir\hooks\pre-commit.py" but it gets interpreted as
        # "dirhookspre-commit.py".
        gclient_hook_path = os.path.join(DEPOT_TOOLS_DIR, 'hooks',
                                         'pre-commit.py').replace('\\', '\\\\')
        gclient_hook_content = '\n'.join((
            f'{PRECOMMIT_HOOK_VAR}={gclient_hook_path}',
            f'if [ -f "${PRECOMMIT_HOOK_VAR}" ]; then',
            f'    python3 "${PRECOMMIT_HOOK_VAR}" || exit 1',
            'fi',
        ))

        soln = gclient_paths.GetPrimarySolutionPath()
        if not soln:
            print('Could not find gclient solution.')
            return

        git_dir = os.path.join(soln, '.git')
        if not os.path.exists(git_dir):
            return

        git_hooks_dir = os.path.join(git_dir, 'hooks')
        os.makedirs(git_hooks_dir, exist_ok=True)

        hook = os.path.join(git_dir, 'hooks', 'pre-commit')
        if os.path.exists(hook):
            with open(hook, 'r') as f:
                content = f.read()
            if PRECOMMIT_HOOK_VAR in content:
                print(f'{hook} already contains the gclient pre-commit hook.')
            else:
                print(f'A pre-commit hook already exists at {hook}.\n'
                      f'Please append the following lines to the hook:\n\n'
                      f'{gclient_hook_content}')
            return

        print(f'Creating a pre-commit hook at {hook}')
        with open(hook, 'w') as f:
            f.write('#!/bin/sh\n')
            f.write(f'{gclient_hook_content}\n')
        os.chmod(hook, 0o755)

    def _RemoveUnversionedGitDirs(self):
        """Remove directories that are no longer part of the checkout.

        Notify the user if there is an orphaned entry in their working copy.
        Only delete the directory if there are no changes in it, and
        delete_unversioned_trees is set to true.

        Returns CIPD packages that are no longer versioned.
        """

        entry_names_and_sync = [(i.name, i._should_sync)
                                for i in self.root.subtree(False) if i.url]
        entries = []
        if entry_names_and_sync:
            entries, _ = zip(*entry_names_and_sync)
        full_entries = [
            os.path.join(self.root_dir, e.replace('/', os.path.sep))
            for e in entries
        ]
        no_sync_entries = [
            name for name, should_sync in entry_names_and_sync
            if not should_sync
        ]

        removed_cipd_entries = []
        read_entries = self._ReadEntries()
        # Add known dependency state
        queue = list(self.dependencies)
        while len(queue) > 0:
            dep = queue.pop()
            queue.extend(dep.dependencies)
            if not dep._known_dependency_diff:
                continue

            for k, v in dep._known_dependency_diff.items():
                path = f'{dep.name}/{k}'
                if path in read_entries:
                    continue
                read_entries[path] = f'https://unknown@{v[1]}'


        # We process entries sorted in reverse to ensure a child dir is
        # always deleted before its parent dir.
        # This is especially important for submodules with pinned revisions
        # overwritten by a vars or custom_deps. In this case, if a parent
        # submodule is encountered first in the loop, it cannot tell the
        # difference between modifications from the vars or actual user
        # modifications that should be kept. http://crbug/1486677#c9 for
        # more details.
        for entry in sorted(read_entries, reverse=True):
            prev_url = read_entries[entry]
            if not prev_url:
                # entry must have been overridden via .gclient custom_deps
                continue
            if any(entry.startswith(sln) for sln in no_sync_entries):
                # Dependencies of solutions that skipped syncing would not
                # show up in `entries`.
                continue
            if (':' in entry):
                # This is a cipd package. Don't clean it up, but prepare for
                # return
                if entry not in entries:
                    removed_cipd_entries.append(entry)
                continue
            # Fix path separator on Windows.
            entry_fixed = entry.replace('/', os.path.sep)
            e_dir = os.path.join(self.root_dir, entry_fixed)
            # Use entry and not entry_fixed there.
            if (entry not in entries and
                (not any(path.startswith(entry + '/') for path in entries))
                    and os.path.exists(e_dir)):
                # The entry has been removed from DEPS.
                scm = gclient_scm.GitWrapper(prev_url, self.root_dir,
                                             entry_fixed, self.outbuf)

                # Check to see if this directory is now part of a higher-up
                # checkout.
                scm_root = None
                try:
                    scm_root = gclient_scm.scm.GIT.GetCheckoutRoot(
                        scm.checkout_path)
                except subprocess2.CalledProcessError:
                    pass
                if not scm_root:
                    logging.warning(
                        'Could not find checkout root for %s. Unable to '
                        'determine whether it is part of a higher-level '
                        'checkout, so not removing.' % entry)
                    continue

                versioned_state = None
                # Check if this is a submodule or versioned directory.
                if os.path.abspath(scm_root) == os.path.abspath(e_dir):
                    e_par_dir = os.path.join(e_dir, os.pardir)
                    if gclient_scm.scm.GIT.IsInsideWorkTree(e_par_dir):
                        par_scm_root = gclient_scm.scm.GIT.GetCheckoutRoot(
                            e_par_dir)
                        # rel_e_dir : relative path of entry w.r.t. its parent
                        # repo.
                        rel_e_dir = os.path.relpath(e_dir, par_scm_root)
                        versioned_state = gclient_scm.scm.GIT.IsVersioned(
                            par_scm_root, rel_e_dir)
                        # This is to handle the case of third_party/WebKit migrating
                        # from being a DEPS entry to being part of the main project. If
                        # the subproject is a Git project, we need to remove its .git
                        # folder. Otherwise git operations on that folder will have
                        # different effects depending on the current working directory.
                        if versioned_state == gclient_scm.scm.VERSIONED_DIR:
                            save_dir = scm.GetGitBackupDirPath()
                            # Remove any eventual stale backup dir for the same
                            # project.
                            if os.path.exists(save_dir):
                                gclient_utils.rmtree(save_dir)
                            os.rename(os.path.join(e_dir, '.git'), save_dir)
                            # When switching between the two states (entry/ is a
                            # subproject -> entry/ is part of the outer
                            # project), it is very likely that some files are
                            # changed in the checkout, unless we are jumping
                            # *exactly* across the commit which changed just
                            # DEPS. In such case we want to cleanup any eventual
                            # stale files (coming from the old subproject) in
                            # order to end up with a clean checkout.
                            gclient_scm.scm.GIT.CleanupDir(
                                par_scm_root, rel_e_dir)
                            assert not os.path.exists(
                                os.path.join(e_dir, '.git'))
                            print(
                                '\nWARNING: \'%s\' has been moved from DEPS to a higher '
                                'level checkout. The git folder containing all the local'
                                ' branches has been saved to %s.\n'
                                'If you don\'t care about its state you can safely '
                                'remove that folder to free up space.' %
                                (entry, save_dir))
                            continue

                if scm_root in full_entries:
                    logging.info(
                        '%s is part of a higher level checkout, not removing',
                        scm.GetCheckoutRoot())
                    continue

                file_list = []
                scm.status(self._options, [], file_list)
                modified_files = file_list != []
                if (not self._options.delete_unversioned_trees
                        or (modified_files and not self._options.force)):
                    # There are modified files in this entry. Keep warning until
                    # removed.
                    self.add_dependency(
                        GitDependency(
                            parent=self,
                            name=entry,
                            # Update URL with scheme in protocol_override
                            url=GitDependency.updateProtocol(
                                prev_url, self.protocol),
                            managed=False,
                            custom_deps={},
                            custom_vars={},
                            custom_hooks=[],
                            deps_file=None,
                            should_process=True,
                            should_recurse=False,
                            relative=None,
                            condition=None,
                            protocol=self.protocol))
                    if modified_files and self._options.delete_unversioned_trees:
                        print(
                            '\nWARNING: \'%s\' is no longer part of this client.\n'
                            'Despite running \'gclient sync -D\' no action was taken '
                            'as there are modifications.\nIt is recommended you revert '
                            'all changes or run \'gclient sync -D --force\' next '
                            'time.' % entry_fixed)
                    else:
                        print(
                            '\nWARNING: \'%s\' is no longer part of this client.\n'
                            'It is recommended that you manually remove it or use '
                            '\'gclient sync -D\' next time.' % entry_fixed)
                else:
                    # Delete the entry
                    print('\n________ deleting \'%s\' in \'%s\'' %
                          (entry_fixed, self.root_dir))
                    gclient_utils.rmtree(e_dir)
                    # We restore empty directories of submodule paths.
                    if versioned_state == gclient_scm.scm.VERSIONED_SUBMODULE:
                        gclient_scm.scm.GIT.Capture(
                            ['restore', '--', rel_e_dir], cwd=par_scm_root)
        # record the current list of entries for next time
        self._SaveEntries()
        return removed_cipd_entries

    def RunOnDeps(self,
                  command,
                  args,
                  ignore_requirements=False,
                  progress=True):
        """Runs a command on each dependency in a client and its dependencies.

        Args:
            command: The command to use (e.g., 'status' or 'diff')
            args: list of str - extra arguments to add to the command line.
        """
        if not self.dependencies:
            raise gclient_utils.Error('No solution specified')

        revision_overrides = {}
        patch_refs = {}
        target_branches = {}
        skip_sync_revisions = {}
        # It's unnecessary to check for revision overrides for 'recurse'.
        # Save a few seconds by not calling _EnforceRevisions() in that case.
        if command not in ('diff', 'recurse', 'runhooks', 'status', 'revert',
                           'validate'):
            self._CheckConfig()
            revision_overrides = self._EnforceRevisions()

        if command == 'update':
            patch_refs, target_branches = self._EnforcePatchRefsAndBranches()
            if NO_SYNC_EXPERIMENT in self._options.experiments:
                skip_sync_revisions = self._EnforceSkipSyncRevisions(patch_refs)

        # Store solutions' custom_vars on memory to compare in the next run.
        # All dependencies added later are inherited from the current
        # self.dependencies.
        custom_vars = {
            dep.name: dep.custom_vars
            for dep in self.dependencies if dep.custom_vars
        }
        if custom_vars:
            self._WriteFileContents(PREVIOUS_CUSTOM_VARS_FILE,
                                    json.dumps(custom_vars))

        # Disable progress for non-tty stdout.
        should_show_progress = (setup_color.IS_TTY and not self._options.verbose
                                and progress)
        pm = None
        if should_show_progress:
            if command in ('update', 'revert'):
                pm = Progress('Syncing projects', 1)
            elif command in ('recurse', 'validate'):
                pm = Progress(' '.join(args), 1)
        work_queue = gclient_utils.ExecutionQueue(
            self._options.jobs,
            pm,
            ignore_requirements=ignore_requirements,
            verbose=self._options.verbose)
        for s in self.dependencies:
            if s.should_process:
                work_queue.enqueue(s)
        work_queue.flush(revision_overrides,
                         command,
                         args,
                         options=self._options,
                         patch_refs=patch_refs,
                         target_branches=target_branches,
                         skip_sync_revisions=skip_sync_revisions)

        if revision_overrides:
            print(
                'Please fix your script, having invalid --revision flags will soon '
                'be considered an error.',
                file=sys.stderr)

        if patch_refs:
            raise gclient_utils.Error(
                'The following --patch-ref flags were not used. Please fix it:\n%s'
                % ('\n'.join(patch_repo + '@' + patch_ref
                             for patch_repo, patch_ref in patch_refs.items())))

        # Check whether git should be updated.
        recommendation = git_common.check_git_version()
        if (recommendation and
                os.environ.get('GCLIENT_SUPPRESS_GIT_VERSION_WARNING') != '1'):
            message = (f'{recommendation}\n'
                       'Disable this warning by setting the '
                       'GCLIENT_SUPPRESS_GIT_VERSION_WARNING\n'
                       'environment variable to 1.')
            gclient_utils.AddWarning(message)

        # TODO(crbug.com/1475405): Warn users if the project uses submodules and
        # they have fsmonitor enabled.
        if command == 'update':
            # Check if any of the root dependency have submodules.
            is_submoduled = any(
                map(
                    lambda d: d.git_dependencies_state in
                    (gclient_eval.SUBMODULES, gclient_eval.SYNC),
                    self.dependencies))
            if is_submoduled:
                git_common.warn_submodule()

        # Once all the dependencies have been processed, it's now safe to write
        # out the gn_args_file and run the hooks.
        removed_cipd_entries = []
        if command == 'update' or (command == 'runhooks' and self._IsCog()):
            for dependency in self.dependencies:
                gn_args_dep = dependency
                if gn_args_dep._gn_args_from:
                    deps_map = {
                        dep.name: dep
                        for dep in gn_args_dep.dependencies
                    }
                    gn_args_dep = deps_map.get(gn_args_dep._gn_args_from)
                if gn_args_dep and gn_args_dep.HasGNArgsFile():
                    gn_args_dep.WriteGNArgsFile()

            removed_cipd_entries = self._RemoveUnversionedGitDirs()

        # Sync CIPD dependencies once removed deps are deleted. In case a git
        # dependency was moved to CIPD, we want to remove the old git directory
        # first and then sync the CIPD dep.
        if self._cipd_root:
            self._cipd_root.run(command)
            # It's possible that CIPD removed some entries that are now part of
            # git worktree. Try to checkout those directories
            if removed_cipd_entries:
                for cipd_entry in removed_cipd_entries:
                    cwd = os.path.join(self._root_dir, cipd_entry.split(':')[0])
                    cwd, tail = os.path.split(cwd)
                    if cwd:
                        try:
                            gclient_scm.scm.GIT.Capture(['checkout', tail],
                                                        cwd=cwd)
                        except (subprocess2.CalledProcessError, OSError):
                            # repo of the deleted cipd may also have been deleted.
                            pass

        if not self._options.nohooks:
            if should_show_progress:
                pm = Progress('Running hooks', 1)
            self.RunHooksRecursively(self._options, pm)

        self._WriteFileContents(PREVIOUS_SYNC_COMMITS_FILE,
                                os.environ.get(PREVIOUS_SYNC_COMMITS, '{}'))

        return 0

    def PrintRevInfo(self):
        if not self.dependencies:
            raise gclient_utils.Error('No solution specified')
        # Load all the settings.
        work_queue = gclient_utils.ExecutionQueue(self._options.jobs,
                                                  None,
                                                  False,
                                                  verbose=self._options.verbose)
        for s in self.dependencies:
            if s.should_process:
                work_queue.enqueue(s)
        work_queue.flush({},
                         'revinfo', [],
                         options=self._options,
                         patch_refs=None,
                         target_branches=None,
                         skip_sync_revisions=None)

        def ShouldPrintRevision(dep):
            return (not self._options.filter
                    or dep.FuzzyMatchUrl(self._options.filter))

        if self._options.snapshot:
            json_output = []
            # First level at .gclient
            for d in self.dependencies:
                entries = {}

                def GrabDeps(dep):
                    """Recursively grab dependencies."""
                    for rec_d in dep.dependencies:
                        rec_d.PinToActualRevision()
                        if ShouldPrintRevision(rec_d):
                            entries[rec_d.name] = rec_d.url
                        GrabDeps(rec_d)

                GrabDeps(d)
                json_output.append({
                    'name': d.name,
                    'solution_url': d.url,
                    'deps_file': d.deps_file,
                    'managed': d.managed,
                    'custom_deps': entries,
                })
            if self._options.output_json == '-':
                print(json.dumps(json_output, indent=2, separators=(',', ': ')))
            elif self._options.output_json:
                with open(self._options.output_json, 'w') as f:
                    json.dump(json_output, f)
            else:
                # Print the snapshot configuration file
                print(self.DEFAULT_SNAPSHOT_FILE_TEXT % {
                    'solution_list': pprint.pformat(json_output, indent=2),
                })
        else:
            entries = {}
            for d in self.root.subtree(False):
                if self._options.actual:
                    d.PinToActualRevision()
                if ShouldPrintRevision(d):
                    entries[d.name] = d.url
            if self._options.output_json:
                json_output = {
                    name: {
                        'url': rev.split('@')[0] if rev else None,
                        'rev':
                        rev.split('@')[1] if rev and '@' in rev else None,
                    }
                    for name, rev in entries.items()
                }
                if self._options.output_json == '-':
                    print(
                        json.dumps(json_output,
                                   indent=2,
                                   separators=(',', ': ')))
                else:
                    with open(self._options.output_json, 'w') as f:
                        json.dump(json_output, f)
            else:
                keys = sorted(entries.keys())
                for x in keys:
                    print('%s: %s' % (x, entries[x]))
        logging.info(str(self))

    def ParseDepsFile(self):
        """No DEPS to parse for a .gclient file."""
        raise gclient_utils.Error('Internal error')

    def PrintLocationAndContents(self):
        # Print out the .gclient file.  This is longer than if we just printed
        # the client dict, but more legible, and it might contain helpful
        # comments.
        print('Loaded .gclient config in %s:\n%s' %
              (self.root_dir, self.config_content))

    def GetCipdRoot(self):
        if not self._cipd_root:
            self._cipd_root = gclient_scm.CipdRoot(
                self.root_dir,
                # TODO(jbudorick): Support other service URLs as necessary.
                # Service URLs should be constant over the scope of a cipd
                # root, so a var per DEPS file specifying the service URL
                # should suffice.
                'https://chrome-infra-packages.appspot.com',
                log_level='info' if self._options.verbose else None)
        return self._cipd_root

    def GetGcsRoot(self):
        if not self._gcs_root:
            self._gcs_root = gclient_scm.GcsRoot(self.root_dir)
        return self._gcs_root

    @property
    def root_dir(self):
        """Root directory of gclient checkout."""
        return self._root_dir

    @property
    def enforced_os(self):
        """What deps_os entries that are to be parsed."""
        return self._enforced_os

    @property
    def target_os(self):
        return self._enforced_os

    @property
    def target_cpu(self):
        return self._enforced_cpu


class GcsDependency(Dependency):
    """A Dependency object that represents a single GCS bucket and object"""

    def __init__(self, parent, name, bucket, object_name, sha256sum,
                 output_file, size_bytes, gcs_root, custom_vars, should_process,
                 relative, condition):
        self.bucket = bucket
        self.object_name = object_name
        self.sha256sum = sha256sum
        self.output_file = output_file
        self.size_bytes = size_bytes
        url = f'gs://{self.bucket}/{self.object_name}'
        self._gcs_root = gcs_root
        self._gcs_root.add_object(parent.name, name, object_name)
        super(GcsDependency, self).__init__(parent=parent,
                                            name=f'{name}:{object_name}',
                                            url=url,
                                            managed=None,
                                            custom_deps=None,
                                            custom_vars=custom_vars,
                                            custom_hooks=None,
                                            deps_file=None,
                                            should_process=should_process,
                                            should_recurse=False,
                                            relative=relative,
                                            condition=condition)

    @property
    def output_dir(self):
        return os.path.join(self.root.root_dir, self.name.split(':')[0])

    @property
    def gcs_file_name(self):
        return self.object_name.replace('/', '_')

    @property
    def artifact_output_file(self):
        return os.path.join(self.output_dir, self.output_file
                            or f'.{self.gcs_file_name}')

    @property
    def hash_file(self):
        return os.path.join(self.output_dir, f'.{self.file_prefix}_hash')

    @property
    def migration_toggle_file(self):
        return os.path.join(
            self.output_dir,
            download_from_google_storage.construct_migration_file_name(
                self.object_name))

    @property
    def gcs_file_name(self):
        # Replace forward slashes
        return self.object_name.replace('/', '_')

    @property
    def file_prefix(self):
        # Drop any extensions
        return self.gcs_file_name.replace('.', '_')

    #override
    def verify_validity(self):
        """GCS dependencies allow duplicate name for objects in same directory."""
        logging.info('Dependency(%s).verify_validity()' % self.name)
        return True

    #override
    def run(self, revision_overrides, command, args, work_queue, options,
            patch_refs, target_branches, skip_sync_revisions):
        """Downloads GCS package."""
        logging.info('GcsDependency(%s).run()' % self.name)
        # GCS dependencies do not need to run during runhooks or revinfo.
        if command in ['runhooks', 'revinfo']:
            return
        if not self.should_process:
            return
        self.DownloadGoogleStorage()
        super(GcsDependency,
              self).run(revision_overrides, command, args, work_queue, options,
                        patch_refs, target_branches, skip_sync_revisions)

    def WriteToFile(self, content, file):
        with open(file, 'w') as f:
            f.write(content)
            f.write('\n')

    def IsDownloadNeeded(self):
        """Check if download and extract is needed."""
        if not self.should_process:
            return False
        if not os.path.exists(self.artifact_output_file):
            return True

        existing_hash = None
        if os.path.exists(self.hash_file):
            try:
                with open(self.hash_file, 'r') as f:
                    existing_hash = f.read().rstrip()
            except IOError:
                return True
        else:
            return True

        is_first_class_gcs = os.path.exists(self.migration_toggle_file)
        if not is_first_class_gcs:
            return True

        if existing_hash != self.sha256sum:
            return True
        return False

    def ValidateTarFile(self, tar, prefixes):

        def _validate(tarinfo):
            """Returns false if the tarinfo is something we explicitly forbid."""
            if tarinfo.issym() or tarinfo.islnk():
                # For links, check if the destination is valid.
                if os.path.isabs(tarinfo.linkname):
                    return False
                link_target = os.path.normpath(
                    os.path.join(os.path.dirname(tarinfo.name),
                                 tarinfo.linkname))
                if not any(
                        link_target.startswith(prefix) for prefix in prefixes):
                    return False

            if tarinfo.name == '.':
                return True

            # tarfile for sysroot has paths that start with ./
            cleaned_name = tarinfo.name
            if tarinfo.name.startswith('./') and len(tarinfo.name) > 2:
                cleaned_name = tarinfo.name[2:]
            if ('../' in cleaned_name or '..\\' in cleaned_name or not any(
                    cleaned_name.startswith(prefix) for prefix in prefixes)):
                return False
            return True

        return all(map(_validate, tar.getmembers()))

    def DownloadGoogleStorage(self):
        """Calls GCS."""

        if not self.IsDownloadNeeded():
            return

        files_to_remove = [
            self.hash_file,
            self.artifact_output_file,
            self.migration_toggle_file,
        ]
        for f in files_to_remove:
            if os.path.exists(f):
                os.remove(f)

        # Ensure that the directory exists. Another process/thread may create
        # it, so exist_ok is used.
        os.makedirs(self.output_dir, exist_ok=True)

        gsutil = download_from_google_storage.Gsutil(
            download_from_google_storage.GSUTIL_DEFAULT_PATH)
        if os.getenv('GCLIENT_TEST') == '1':
            if 'no-extract' in self.artifact_output_file:
                with open(self.artifact_output_file, 'w+') as f:
                    f.write('non-extractable file')
            else:
                # Create fake tar file and extracted tar contents
                tmpdir = tempfile.mkdtemp()
                copy_dir = os.path.join(tmpdir, self.gcs_file_name,
                                        'extracted_dir')
                if os.path.exists(copy_dir):
                    shutil.rmtree(copy_dir)
                os.makedirs(copy_dir)
                with open(os.path.join(copy_dir, 'extracted_file'), 'w+') as f:
                    f.write('extracted text')
                with tarfile.open(self.artifact_output_file, "w:gz") as tar:
                    tar.add(copy_dir, arcname=os.path.basename(copy_dir))
        else:
            code, _, err = gsutil.check_call('cp', self.url,
                                             self.artifact_output_file)
            if code and err:
                raise Exception(f'{code}: {err}')
            # Check that something actually downloaded into the path
            if not os.path.exists(self.artifact_output_file):
                raise Exception(
                    f'Nothing was downloaded into {self.artifact_output_file}')

        calculated_sha256sum = ''
        calculated_size_bytes = None
        if os.getenv('GCLIENT_TEST') == '1':
            calculated_sha256sum = 'abcd123'
            calculated_size_bytes = 10000
        else:
            calculated_sha256sum = (
                upload_to_google_storage_first_class.get_sha256sum(
                    self.artifact_output_file))
            calculated_size_bytes = os.path.getsize(self.artifact_output_file)

        if calculated_sha256sum != self.sha256sum:
            raise Exception('sha256sum does not match calculated hash. '
                            '{original} vs {calculated}'.format(
                                original=self.sha256sum,
                                calculated=calculated_sha256sum,
                            ))

        if calculated_size_bytes != self.size_bytes:
            raise Exception('size_bytes does not match calculated size bytes. '
                            '{original} vs {calculated}'.format(
                                original=self.size_bytes,
                                calculated=calculated_size_bytes,
                            ))

        if tarfile.is_tarfile(self.artifact_output_file):
            with tarfile.open(self.artifact_output_file, 'r:*') as tar:
                formatted_names = []
                for name in tar.getnames():
                    if name.startswith('./') and len(name) > 2:
                        formatted_names.append(name[2:])
                    else:
                        formatted_names.append(name)
                possible_top_level_dirs = set(
                    name.split('/')[0] for name in formatted_names)
                is_valid_tar = self.ValidateTarFile(tar,
                                                    possible_top_level_dirs)
                if not is_valid_tar:
                    raise Exception('tarfile contains invalid entries')

                tar_content_file = os.path.join(
                    self.output_dir, f'.{self.file_prefix}_content_names')
                self.WriteToFile(json.dumps(tar.getnames()), tar_content_file)

                tar.extractall(path=self.output_dir)

        if os.getenv('GCLIENT_TEST') != '1':
            code, err = download_from_google_storage.set_executable_bit(
                self.artifact_output_file, self.url, gsutil)
            if code != 0:
                raise Exception(f'{code}: {err}')

        self.WriteToFile(calculated_sha256sum, self.hash_file)
        self.WriteToFile(str(1), self.migration_toggle_file)

    #override
    def GetScmName(self):
        """Always 'gcs'."""
        return 'gcs'

    #override
    def CreateSCM(self, out_cb=None):
        """Create a Wrapper instance suitable for handling this GCS dependency."""
        return gclient_scm.GcsWrapper(self.url, self.root.root_dir, self.name,
                                      self.outbuf, out_cb)


class CipdDependency(Dependency):
    """A Dependency object that represents a single CIPD package."""
    def __init__(self, parent, name, dep_value, cipd_root, custom_vars,
                 should_process, relative, condition):
        package = dep_value['package']
        version = dep_value['version']
        url = urllib.parse.urljoin(cipd_root.service_url,
                                   '%s@%s' % (package, version))
        super(CipdDependency, self).__init__(parent=parent,
                                             name=name + ':' + package,
                                             url=url,
                                             managed=None,
                                             custom_deps=None,
                                             custom_vars=custom_vars,
                                             custom_hooks=None,
                                             deps_file=None,
                                             should_process=should_process,
                                             should_recurse=False,
                                             relative=relative,
                                             condition=condition)
        self._cipd_package = None
        self._cipd_root = cipd_root
        # CIPD wants /-separated paths, even on Windows.
        native_subdir_path = os.path.relpath(
            os.path.join(self.root.root_dir, name), cipd_root.root_dir)
        self._cipd_subdir = posixpath.join(*native_subdir_path.split(os.sep))
        self._package_name = package
        self._package_version = version

    #override
    def run(self, revision_overrides, command, args, work_queue, options,
            patch_refs, target_branches, skip_sync_revisions):
        """Runs |command| then parse the DEPS file."""
        logging.info('CipdDependency(%s).run()' % self.name)
        # GCS dependencies do not need to run during runhooks.
        # TODO(b/349643421): Note, for a GCSDependency we also return early
        # for the `revinfo` command, however doing the same for cipd
        # currently breaks testRevInfoActual() in gclient_cipd_smoketest.py.
        # b/349699772 may be relevant.
        if command == 'runhooks':
            return
        if not self.should_process:
            return
        self._CreatePackageIfNecessary()
        super(CipdDependency,
              self).run(revision_overrides, command, args, work_queue, options,
                        patch_refs, target_branches, skip_sync_revisions)

    def _CreatePackageIfNecessary(self):
        # We lazily create the CIPD package to make sure that only packages
        # that we want (as opposed to all packages defined in all DEPS files
        # we parse) get added to the root and subsequently ensured.
        if not self._cipd_package:
            self._cipd_package = self._cipd_root.add_package(
                self._cipd_subdir, self._package_name, self._package_version)

    def ParseDepsFile(self):
        """CIPD dependencies are not currently allowed to have nested deps."""
        self.add_dependencies_and_close([], [])

    #override
    def verify_validity(self):
        """CIPD dependencies allow duplicate name for packages in same directory."""
        logging.info('Dependency(%s).verify_validity()' % self.name)
        return True

    #override
    def GetScmName(self):
        """Always 'cipd'."""
        return 'cipd'

    def GetExpandedPackageName(self):
        """Return the CIPD package name with the variables evaluated."""
        package = self._cipd_root.expand_package_name(self._package_name)
        if package:
            return package
        return self._package_name

    #override
    def CreateSCM(self, out_cb=None):
        """Create a Wrapper instance suitable for handling this CIPD dependency."""
        self._CreatePackageIfNecessary()
        return gclient_scm.CipdWrapper(self.url,
                                       self.root.root_dir,
                                       self.name,
                                       self.outbuf,
                                       out_cb,
                                       root=self._cipd_root,
                                       package=self._cipd_package)

    def hierarchy(self, include_url=False, graphviz=False):
        if graphviz:
            return ''  # graphviz lines not implemented for cipd deps.
        return self.parent.hierarchy(include_url) + ' -> ' + self._cipd_subdir

    def ToLines(self):
        # () -> Sequence[str]
        """Return a list of lines representing this in a DEPS file."""
        def escape_cipd_var(package):
            return package.replace('{', '{{').replace('}', '}}')

        s = []
        self._CreatePackageIfNecessary()
        if self._cipd_package.authority_for_subdir:
            condition_part = (['    "condition": %r,' %
                               self.condition] if self.condition else [])
            s.extend([
                '  # %s' % self.hierarchy(include_url=False),
                '  "%s": {' % (self.name.split(':')[0], ),
                '    "packages": [',
            ])
            for p in sorted(self._cipd_root.packages(self._cipd_subdir),
                            key=lambda x: x.name):
                s.extend([
                    '      {',
                    '        "package": "%s",' % escape_cipd_var(p.name),
                    '        "version": "%s",' % p.version,
                    '      },',
                ])

            s.extend([
                '    ],',
                '    "dep_type": "cipd",',
            ] + condition_part + [
                '  },',
                '',
            ])
        return s


#### gclient commands.


@subcommand.usage('[command] [args ...]')
@metrics.collector.collect_metrics('gclient recurse')
def CMDrecurse(parser, args):
    """Operates [command args ...] on all the dependencies.

    Change directory to each dependency's directory, and call [command
    args ...] there.  Sets GCLIENT_DEP_PATH environment variable as the
    dep's relative location to root directory of the checkout.

    Examples:
    * `gclient recurse --no-progress -j1 sh -c 'echo "$GCLIENT_DEP_PATH"'`
    print the relative path of each dependency.
    * `gclient recurse --no-progress -j1 sh -c "pwd"`
    print the absolute path of each dependency.
    """
    # Stop parsing at the first non-arg so that these go through to the command
    parser.disable_interspersed_args()
    parser.add_option('-s',
                      '--scm',
                      action='append',
                      default=[],
                      help='Choose scm types to operate upon.')
    parser.add_option('-i',
                      '--ignore',
                      action='store_true',
                      help='Ignore non-zero return codes from subcommands.')
    parser.add_option(
        '--prepend-dir',
        action='store_true',
        help='Prepend relative dir for use with git <cmd> --null.')
    parser.add_option(
        '--no-progress',
        action='store_true',
        help='Disable progress bar that shows sub-command updates')
    options, args = parser.parse_args(args)
    if not args:
        print('Need to supply a command!', file=sys.stderr)
        return 1
    root_and_entries = gclient_utils.GetGClientRootAndEntries()
    if not root_and_entries:
        print(
            'You need to run gclient sync at least once to use \'recurse\'.\n'
            'This is because .gclient_entries needs to exist and be up to date.',
            file=sys.stderr)
        return 1

    # Normalize options.scm to a set()
    scm_set = set()
    for scm in options.scm:
        scm_set.update(scm.split(','))
    options.scm = scm_set

    options.nohooks = True
    client = GClient.LoadCurrentConfig(options)
    if not client:
        raise gclient_utils.Error(
            'client not configured; see \'gclient config\'')
    return client.RunOnDeps('recurse',
                            args,
                            ignore_requirements=True,
                            progress=not options.no_progress)


@subcommand.usage('[args ...]')
@metrics.collector.collect_metrics('gclient fetch')
def CMDfetch(parser, args):
    """Fetches upstream commits for all modules.

    Completely git-specific. Simply runs 'git fetch [args ...]' for each module.
    """
    if gclient_utils.IsEnvCog():
        raise gclient_utils.Error(
            'gclient fetch command is not supported in non-git environment.')
    (options, args) = parser.parse_args(args)
    return CMDrecurse(
        OptionParser(),
        ['--jobs=%d' % options.jobs, '--scm=git', 'git', 'fetch'] + args)


class Flattener(object):
    """Flattens a gclient solution."""
    def __init__(self, client, pin_all_deps=False):
        """Constructor.

        Arguments:
            client (GClient): client to flatten
            pin_all_deps (bool): whether to pin all deps, even if they're not pinned
                in DEPS
        """
        self._client = client

        self._deps_string = None
        self._deps_graph_lines = None
        self._deps_files = set()

        self._allowed_hosts = set()
        self._deps = {}
        self._hooks = []
        self._pre_deps_hooks = []
        self._vars = {}

        self._flatten(pin_all_deps=pin_all_deps)

    @property
    def deps_string(self):
        assert self._deps_string is not None
        return self._deps_string

    @property
    def deps_graph_lines(self):
        assert self._deps_graph_lines is not None
        return self._deps_graph_lines

    @property
    def deps_files(self):
        return self._deps_files

    def _pin_dep(self, dep):
        """Pins a dependency to specific full revision sha.

        Arguments:
            dep (Dependency): dependency to process
        """
        if dep.url is None:
            return

        # Make sure the revision is always fully specified (a hash),
        # as opposed to refs or tags which might change. Similarly,
        # shortened shas might become ambiguous; make sure to always
        # use full one for pinning.
        revision = gclient_utils.SplitUrlRevision(dep.url)[1]
        if not revision or not gclient_utils.IsFullGitSha(revision):
            dep.PinToActualRevision()

    def _flatten(self, pin_all_deps=False):
        """Runs the flattener. Saves resulting DEPS string.

        Arguments:
            pin_all_deps (bool): whether to pin all deps, even if they're not pinned
                in DEPS
        """
        for solution in self._client.dependencies:
            self._add_dep(solution)
            self._flatten_dep(solution)

        if pin_all_deps:
            for dep in self._deps.values():
                self._pin_dep(dep)

        def add_deps_file(dep):
            # Only include DEPS files referenced by recursedeps.
            if not dep.should_recurse:
                return
            deps_file = dep.deps_file
            deps_path = os.path.join(self._client.root_dir, dep.name, deps_file)
            if not os.path.exists(deps_path):
                # gclient has a fallback that if deps_file doesn't exist, it'll
                # try DEPS. Do the same here.
                deps_file = 'DEPS'
                deps_path = os.path.join(self._client.root_dir, dep.name,
                                         deps_file)
                if not os.path.exists(deps_path):
                    return
            assert dep.url
            self._deps_files.add((dep.url, deps_file, dep.hierarchy_data()))

        for dep in self._deps.values():
            add_deps_file(dep)

        gn_args_dep = self._deps.get(self._client.dependencies[0]._gn_args_from,
                                     self._client.dependencies[0])

        self._deps_graph_lines = _DepsToDotGraphLines(self._deps)
        self._deps_string = '\n'.join(
            _GNSettingsToLines(gn_args_dep._gn_args_file, gn_args_dep._gn_args)
            + _AllowedHostsToLines(self._allowed_hosts) +
            _DepsToLines(self._deps) + _HooksToLines('hooks', self._hooks) +
            _HooksToLines('pre_deps_hooks', self._pre_deps_hooks) +
            _VarsToLines(self._vars) + [
                '# %s, %s' % (url, deps_file)
                for url, deps_file, _ in sorted(self._deps_files)
            ] + [''])  # Ensure newline at end of file.

    def _add_dep(self, dep):
        """Helper to add a dependency to flattened DEPS.

        Arguments:
            dep (Dependency): dependency to add
        """
        assert dep.name not in self._deps or self._deps.get(
            dep.name) == dep, (dep.name, self._deps.get(dep.name))
        if dep.url:
            self._deps[dep.name] = dep

    def _flatten_dep(self, dep):
        """Visits a dependency in order to flatten it (see CMDflatten).

        Arguments:
            dep (Dependency): dependency to process
        """
        logging.debug('_flatten_dep(%s)', dep.name)

        assert dep.deps_parsed, (
            "Attempted to flatten %s but it has not been processed." % dep.name)

        self._allowed_hosts.update(dep.allowed_hosts)

        # Only include vars explicitly listed in the DEPS files or gclient
        # solution, not automatic, local overrides (i.e. not all of
        # dep.get_vars()).
        hierarchy = dep.hierarchy(include_url=False)
        for key, value in dep._vars.items():
            # Make sure there are no conflicting variables. It is fine however
            # to use same variable name, as long as the value is consistent.
            assert key not in self._vars or self._vars[key][1] == value, (
                "dep:%s key:%s value:%s != %s" %
                (dep.name, key, value, self._vars[key][1]))
            self._vars[key] = (hierarchy, value)
        # Override explicit custom variables.
        for key, value in dep.custom_vars.items():
            # Do custom_vars that don't correspond to DEPS vars ever make sense?
            # DEPS conditionals shouldn't be using vars that aren't also defined
            # in the DEPS (presubmit actually disallows this), so any new
            # custom_var must be unused in the DEPS, so no need to add it to the
            # flattened output either.
            if key not in self._vars:
                continue
            # Don't "override" existing vars if it's actually the same value.
            if self._vars[key][1] == value:
                continue
            # Anything else is overriding a default value from the DEPS.
            self._vars[key] = (hierarchy + ' [custom_var override]', value)

        self._pre_deps_hooks.extend([(dep, hook)
                                     for hook in dep.pre_deps_hooks])
        self._hooks.extend([(dep, hook) for hook in dep.deps_hooks])

        for sub_dep in dep.dependencies:
            self._add_dep(sub_dep)

        for d in dep.dependencies:
            if d.should_recurse:
                self._flatten_dep(d)


@metrics.collector.collect_metrics('gclient gitmodules')
def CMDgitmodules(parser, args):
    """Adds or updates Git Submodules based on the contents of the DEPS file.

    This command should be run in the root directory of the repo.
    It will create or update the .gitmodules file and include
    `gclient-condition` values. Commits in gitlinks will also be updated.
    If you are running this command to set `gclient-recursedeps` for the first
    time, you will need to delete the .gitmodules file (if any) before running
    this command.
    """
    if gclient_utils.IsEnvCog():
        raise gclient_utils.Error(
            'updating git submodules is not supported. Please upvote '
            'b/340254045 if you require this functionality.')
    parser.add_option('--output-gitmodules',
                      help='name of the .gitmodules file to write to',
                      default='.gitmodules')
    parser.add_option(
        '--deps-file',
        help=
        'name of the deps file to parse for git dependency paths and commits.',
        default='DEPS')
    parser.add_option(
        '--skip-dep',
        action="append",
        help='skip adding gitmodules for the git dependency at the given path',
        default=[])
    options, args = parser.parse_args(args)

    deps_dir = os.path.dirname(os.path.abspath(options.deps_file))
    gclient_path = gclient_paths.FindGclientRoot(deps_dir)
    if not gclient_path:
        logging.error(
            '.gclient not found\n'
            'Make sure you are running this script from a gclient workspace.')
        sys.exit(1)

    deps_content = gclient_utils.FileRead(options.deps_file)
    ls = gclient_eval.Parse(deps_content, options.deps_file, None, None)

    prefix_length = 0
    if not 'use_relative_paths' in ls or ls['use_relative_paths'] != True:
        delta_path = os.path.relpath(deps_dir, os.path.abspath(gclient_path))
        if delta_path:
            prefix_length = len(delta_path.replace(os.path.sep, '/')) + 1

    cache_info = []

    # Git submodules shouldn't use .git suffix since it's not well supported.
    # However, we can't update .gitmodules files since there is no guarantee
    # that user has the latest version of depot_tools, and also they are not on
    # some old branch which contains already contains submodules with .git.
    # This check makes the transition easier.
    strip_git_suffix = True
    # Users may have an outdated depot_tools which doesn't set
    # gclient-recursedeps, which may undo changes made by an up-to-date `gclient
    # gitmodules` run. Users may also have an up-to-date depot_tools but an
    # outdated chromium/src without gclient-recursedeps set which would cause
    # confusion when they run `gclient gitmodules` and see unexpected
    # gclient-recursedeps in the diff. We only want to set gclient-recursedeps
    # if it's already set in the .gitmodules file or if there is no existing
    # .gitmodules file so we know `gclient gitmodules` is being run for the
    # first time.
    set_recursedeps = True
    if os.path.exists(options.output_gitmodules):
        dot_git_pattern = re.compile('^(\s*)url(\s*)=.*\.git$')
        with open(options.output_gitmodules) as f:
            strip_git_suffix = not any(dot_git_pattern.match(l) for l in f)
            set_recursedeps = any(
                'gclient-recursedeps' in l for l in f)

    recursedeps = ls.get('recursedeps')
    with open(options.output_gitmodules, 'w', newline='') as f:
        for path, dep in ls.get('deps').items():
            if path in options.skip_dep:
                continue
            if dep.get('dep_type') != 'git':
                continue
            try:
                url, commit = dep['url'].split('@', maxsplit=1)
            except ValueError:
                logging.error('error on %s; %s, not adding it', path,
                              dep["url"])
                continue
            isRecurseDeps = recursedeps and path in recursedeps
            if prefix_length:
                path = path[prefix_length:]

            if strip_git_suffix:
                if url.endswith('.git'):
                    url = url[:-4]  # strip .git
                url = url.rstrip('/')  # remove trailing slash for consistency

            cache_info += ['--cacheinfo', f'160000,{commit},{path}']
            f.write(f'[submodule "{path}"]\n\tpath = {path}\n\turl = {url}\n')
            if 'condition' in dep:
                f.write(f'\tgclient-condition = {dep["condition"]}\n')
            if isRecurseDeps and set_recursedeps:
                f.write('\tgclient-recursedeps = true\n')
            # Windows has limit how long, so let's chunk those calls.
            if len(cache_info) >= 100:
                subprocess2.call(['git', 'update-index', '--add'] + cache_info)
                cache_info = []

    if cache_info:
        subprocess2.call(['git', 'update-index', '--add'] + cache_info)
    subprocess2.call(['git', 'add', '.gitmodules'])
    print('.gitmodules and gitlinks updated. Please check `git diff --staged` '
          'and commit those staged changes (`git commit` without -a)')


@metrics.collector.collect_metrics('gclient flatten')
def CMDflatten(parser, args):
    """Flattens the solutions into a single DEPS file."""
    parser.add_option('--output-deps', help='Path to the output DEPS file')
    parser.add_option(
        '--output-deps-files',
        help=('Path to the output metadata about DEPS files referenced by '
              'recursedeps.'))
    parser.add_option(
        '--pin-all-deps',
        action='store_true',
        help=('Pin all deps, even if not pinned in DEPS. CAVEAT: only does so '
              'for checked out deps, NOT deps_os.'))
    parser.add_option('--deps-graph-file',
                      help='Provide a path for the output graph file')
    options, args = parser.parse_args(args)

    options.nohooks = True
    options.process_all_deps = True
    client = GClient.LoadCurrentConfig(options)
    if not client:
        raise gclient_utils.Error(
            'client not configured; see \'gclient config\'')

    # Only print progress if we're writing to a file. Otherwise, progress
    # updates could obscure intended output.
    code = client.RunOnDeps('flatten', args, progress=options.output_deps)
    if code != 0:
        return code

    flattener = Flattener(client, pin_all_deps=options.pin_all_deps)

    if options.output_deps:
        with open(options.output_deps, 'w') as f:
            f.write(flattener.deps_string)
    else:
        print(flattener.deps_string)

    if options.deps_graph_file:
        with open(options.deps_graph_file, 'w') as f:
            f.write('\n'.join(flattener.deps_graph_lines))

    deps_files = [{
        'url': d[0],
        'deps_file': d[1],
        'hierarchy': d[2]
    } for d in sorted(flattener.deps_files)]
    if options.output_deps_files:
        with open(options.output_deps_files, 'w') as f:
            json.dump(deps_files, f)

    return 0


def _GNSettingsToLines(gn_args_file, gn_args):
    s = []
    if gn_args_file:
        s.extend([
            'gclient_gn_args_file = "%s"' % gn_args_file,
            'gclient_gn_args = %r' % gn_args,
        ])
    return s


def _AllowedHostsToLines(allowed_hosts):
    """Converts |allowed_hosts| set to list of lines for output."""
    if not allowed_hosts:
        return []
    s = ['allowed_hosts = [']
    for h in sorted(allowed_hosts):
        s.append('  "%s",' % h)
    s.extend([']', ''])
    return s


def _DepsToLines(deps):
    # type: (Mapping[str, Dependency]) -> Sequence[str]
    """Converts |deps| dict to list of lines for output."""
    if not deps:
        return []
    s = ['deps = {']
    for _, dep in sorted(deps.items()):
        s.extend(dep.ToLines())
    s.extend(['}', ''])
    return s


def _DepsToDotGraphLines(deps):
    # type: (Mapping[str, Dependency]) -> Sequence[str]
    """Converts  |deps| dict to list of lines for dot graphs."""
    if not deps:
        return []
    graph_lines = ["digraph {\n\trankdir=\"LR\";"]
    for _, dep in sorted(deps.items()):
        line = dep.hierarchy(include_url=False, graphviz=True)
        if line:
            graph_lines.append("\t%s" % line)
    graph_lines.append("}")
    return graph_lines


def _DepsOsToLines(deps_os):
    """Converts |deps_os| dict to list of lines for output."""
    if not deps_os:
        return []
    s = ['deps_os = {']
    for dep_os, os_deps in sorted(deps_os.items()):
        s.append('  "%s": {' % dep_os)
        for name, dep in sorted(os_deps.items()):
            condition_part = (['      "condition": %r,' %
                               dep.condition] if dep.condition else [])
            s.extend([
                '    # %s' % dep.hierarchy(include_url=False),
                '    "%s": {' % (name, ),
                '      "url": "%s",' % (dep.url, ),
            ] + condition_part + [
                '    },',
                '',
            ])
        s.extend(['  },', ''])
    s.extend(['}', ''])
    return s


def _HooksToLines(name, hooks):
    """Converts |hooks| list to list of lines for output."""
    if not hooks:
        return []
    s = ['%s = [' % name]
    for dep, hook in hooks:
        s.extend([
            '  # %s' % dep.hierarchy(include_url=False),
            '  {',
        ])
        if hook.name is not None:
            s.append('    "name": "%s",' % hook.name)
        if hook.pattern is not None:
            s.append('    "pattern": "%s",' % hook.pattern)
        if hook.condition is not None:
            s.append('    "condition": %r,' % hook.condition)
        # Flattened hooks need to be written relative to the root gclient dir
        cwd = os.path.relpath(os.path.normpath(hook.effective_cwd))
        s.extend(['    "cwd": "%s",' % cwd] + ['    "action": ['] +
                 ['        "%s",' % arg
                  for arg in hook.action] + ['    ]', '  },', ''])
    s.extend([']', ''])
    return s


def _HooksOsToLines(hooks_os):
    """Converts |hooks| list to list of lines for output."""
    if not hooks_os:
        return []
    s = ['hooks_os = {']
    for hook_os, os_hooks in hooks_os.items():
        s.append('  "%s": [' % hook_os)
        for dep, hook in os_hooks:
            s.extend([
                '    # %s' % dep.hierarchy(include_url=False),
                '    {',
            ])
            if hook.name is not None:
                s.append('      "name": "%s",' % hook.name)
            if hook.pattern is not None:
                s.append('      "pattern": "%s",' % hook.pattern)
            if hook.condition is not None:
                s.append('    "condition": %r,' % hook.condition)
            # Flattened hooks need to be written relative to the root gclient
            # dir
            cwd = os.path.relpath(os.path.normpath(hook.effective_cwd))
            s.extend(['    "cwd": "%s",' % cwd] + ['      "action": ['] +
                     ['          "%s",' % arg
                      for arg in hook.action] + ['      ]', '    },', ''])
        s.extend(['  ],', ''])
    s.extend(['}', ''])
    return s


def _VarsToLines(variables):
    """Converts |variables| dict to list of lines for output."""
    if not variables:
        return []
    s = ['vars = {']
    for key, tup in sorted(variables.items()):
        hierarchy, value = tup
        s.extend([
            '  # %s' % hierarchy,
            '  "%s": %r,' % (key, value),
            '',
        ])
    s.extend(['}', ''])
    return s


@metrics.collector.collect_metrics('gclient grep')
def CMDgrep(parser, args):
    """Greps through git repos managed by gclient.

    Runs 'git grep [args...]' for each module.
    """
    # We can't use optparse because it will try to parse arguments sent
    # to git grep and throw an error. :-(
    if not args or re.match('(-h|--help)$', args[0]):
        print(
            'Usage: gclient grep [-j <N>] git-grep-args...\n\n'
            'Example: "gclient grep -j10 -A2 RefCountedBase" runs\n"git grep '
            '-A2 RefCountedBase" on each of gclient\'s git\nrepos with up to '
            '10 jobs.\n\nBonus: page output by appending "|& less -FRSX" to the'
            ' end of your query.',
            file=sys.stderr)
        return 1

    if gclient_utils.IsEnvCog():
        raise gclient_utils.Error('gclient grep command is not supported.')

    jobs_arg = ['--jobs=1']
    if re.match(r'(-j|--jobs=)\d+$', args[0]):
        jobs_arg, args = args[:1], args[1:]
    elif re.match(r'(-j|--jobs)$', args[0]):
        jobs_arg, args = args[:2], args[2:]

    return CMDrecurse(
        parser, jobs_arg + [
            '--ignore', '--prepend-dir', '--no-progress', '--scm=git', 'git',
            'grep', '--null', '--color=Always'
        ] + args)


@metrics.collector.collect_metrics('gclient root')
def CMDroot(parser, args):
    """Outputs the solution root (or current dir if there isn't one)."""
    (options, args) = parser.parse_args(args)
    client = GClient.LoadCurrentConfig(options)
    if client:
        print(os.path.abspath(client.root_dir))
    else:
        print(os.path.abspath('.'))


@subcommand.usage('[url]')
@metrics.collector.collect_metrics('gclient config')
def CMDconfig(parser, args):
    """Creates a .gclient file in the current directory.

    This specifies the configuration for further commands. After update/sync,
    top-level DEPS files in each module are read to determine dependent
    modules to operate on as well. If optional [url] parameter is
    provided, then configuration is read from a specified Subversion server
    URL.
    """
    # We do a little dance with the --gclientfile option.  'gclient config' is
    # the only command where it's acceptable to have both '--gclientfile' and
    # '--spec' arguments.  So, we temporarily stash any --gclientfile parameter
    # into options.output_config_file until after the (gclientfile xor spec)
    # error check.
    parser.remove_option('--gclientfile')
    parser.add_option('--gclientfile',
                      dest='output_config_file',
                      help='Specify an alternate .gclient file')
    parser.add_option('--name',
                      help='overrides the default name for the solution')
    parser.add_option(
        '--deps-file',
        default='DEPS',
        help='overrides the default name for the DEPS file for the '
        'main solutions and all sub-dependencies')
    parser.add_option('--unmanaged',
                      action='store_true',
                      default=False,
                      help='overrides the default behavior to make it possible '
                      'to have the main solution untouched by gclient '
                      '(gclient will check out unmanaged dependencies but '
                      'will never sync them)')
    parser.add_option('--cache-dir',
                      default=UNSET_CACHE_DIR,
                      help='Cache all git repos into this dir and do shared '
                      'clones from the cache, instead of cloning directly '
                      'from the remote. Pass "None" to disable cache, even '
                      'if globally enabled due to $GIT_CACHE_PATH.')
    parser.add_option('--custom-var',
                      action='append',
                      dest='custom_vars',
                      default=[],
                      help='overrides variables; key=value syntax')
    parser.set_defaults(config_filename=None)
    (options, args) = parser.parse_args(args)
    if options.output_config_file:
        setattr(options, 'config_filename',
                getattr(options, 'output_config_file'))
    if ((options.spec and args) or len(args) > 2
            or (not options.spec and not args)):
        parser.error(
            'Inconsistent arguments. Use either --spec or one or 2 args')

    if (options.cache_dir is not UNSET_CACHE_DIR
            and options.cache_dir.lower() == 'none'):
        options.cache_dir = None

    custom_vars = {}
    for arg in options.custom_vars:
        kv = arg.split('=', 1)
        if len(kv) != 2:
            parser.error('Invalid --custom-var argument: %r' % arg)
        custom_vars[kv[0]] = gclient_eval.EvaluateCondition(kv[1], {})

    client = GClient('.', options)
    if options.spec:
        client.SetConfig(options.spec)
    else:
        base_url = args[0].rstrip('/')
        if not options.name:
            name = base_url.split('/')[-1]
            if name.endswith('.git'):
                name = name[:-4]
        else:
            # specify an alternate relpath for the given URL.
            name = options.name
            if not os.path.abspath(os.path.join(os.getcwd(), name)).startswith(
                    os.getcwd()):
                parser.error('Do not pass a relative path for --name.')
            if any(x in ('..', '.', '/', '\\') for x in name.split(os.sep)):
                parser.error(
                    'Do not include relative path components in --name.')

        deps_file = options.deps_file
        client.SetDefaultConfig(name,
                                deps_file,
                                base_url,
                                managed=not options.unmanaged,
                                cache_dir=options.cache_dir,
                                custom_vars=custom_vars)
    client.SaveConfig()
    return 0


@subcommand.epilog("""Example:
  gclient pack > patch.txt
    generate simple patch for configured client and dependences
""")
@metrics.collector.collect_metrics('gclient pack')
def CMDpack(parser, args):
    """Generates a patch which can be applied at the root of the tree.

    Internally, runs 'git diff' on each checked out module and
    dependencies, and performs minimal postprocessing of the output. The
    resulting patch is printed to stdout and can be applied to a freshly
    checked out tree via 'patch -p0 < patchfile'.
    """
    parser.add_option('--deps',
                      dest='deps_os',
                      metavar='OS_LIST',
                      help='override deps for the specified (comma-separated) '
                      'platform(s); \'all\' will process all deps_os '
                      'references')
    parser.remove_option('--jobs')
    (options, args) = parser.parse_args(args)
    # Force jobs to 1 so the stdout is not annotated with the thread ids
    options.jobs = 1
    client = GClient.LoadCurrentConfig(options)
    if not client:
        raise gclient_utils.Error(
            'client not configured; see \'gclient config\'')
    if options.verbose:
        client.PrintLocationAndContents()
    return client.RunOnDeps('pack', args)


@metrics.collector.collect_metrics('gclient status')
def CMDstatus(parser, args):
    """Shows modification status for every dependencies."""
    if gclient_utils.IsEnvCog():
        raise gclient_utils.Error(
            'gclient status command is not supported. Please navigate to '
            'source control view in the activiy bar to view modification '
            'status instead.')
    parser.add_option('--deps',
                      dest='deps_os',
                      metavar='OS_LIST',
                      help='override deps for the specified (comma-separated) '
                      'platform(s); \'all\' will process all deps_os '
                      'references')
    (options, args) = parser.parse_args(args)
    client = GClient.LoadCurrentConfig(options)
    if not client:
        raise gclient_utils.Error(
            'client not configured; see \'gclient config\'')
    if options.verbose:
        client.PrintLocationAndContents()
    return client.RunOnDeps('status', args)


@subcommand.epilog("""Examples:
  gclient sync
      update files from SCM according to current configuration,
      *for modules which have changed since last update or sync*
  gclient sync --force
      update files from SCM according to current configuration, for
      all modules (useful for recovering files deleted from local copy)
  gclient sync --revision src@GIT_COMMIT_OR_REF
      update src directory to GIT_COMMIT_OR_REF

JSON output format:
If the --output-json option is specified, the following document structure will
be emitted to the provided file. 'null' entries may occur for subprojects which
are present in the gclient solution, but were not processed (due to custom_deps,
os_deps, etc.)

{
  "solutions" : {
    "<name>": {  # <name> is the posix-normalized path to the solution.
      "revision": [<git id hex string>|null],
      "scm": ["git"|null],
    }
  }
}
""")
@metrics.collector.collect_metrics('gclient sync')
def CMDsync(parser, args):
    """Checkout/update all modules."""
    parser.add_option('-f',
                      '--force',
                      action='store_true',
                      help='force update even for unchanged modules')
    parser.add_option('-n',
                      '--nohooks',
                      action='store_true',
                      help='don\'t run hooks after the update is complete')
    parser.add_option('-p',
                      '--noprehooks',
                      action='store_true',
                      help='don\'t run pre-DEPS hooks',
                      default=False)
    parser.add_option('-r',
                      '--revision',
                      action='append',
                      dest='revisions',
                      metavar='REV',
                      default=[],
                      help='Enforces git ref/hash for the solutions with the '
                      'format src@rev. The src@ part is optional and can be '
                      'skipped. You can also specify URLs instead of paths '
                      'and gclient will find the solution corresponding to '
                      'the given URL. If a path is also specified, the URL '
                      'takes precedence. -r can be used multiple times when '
                      '.gclient has multiple solutions configured, and will '
                      'work even if the src@ part is skipped. Revision '
                      'numbers (e.g. 31000 or r31000) are not supported.')
    parser.add_option('--patch-ref',
                      action='append',
                      dest='patch_refs',
                      metavar='GERRIT_REF',
                      default=[],
                      help='Patches the given reference with the format '
                      'dep@target-ref:patch-ref. '
                      'For |dep|, you can specify URLs as well as paths, '
                      'with URLs taking preference. '
                      '|patch-ref| will be applied to |dep|, rebased on top '
                      'of what |dep| was synced to, and a soft reset will '
                      'be done. Use --no-rebase-patch-ref and '
                      '--no-reset-patch-ref to disable this behavior. '
                      '|target-ref| is the target branch against which a '
                      'patch was created, it is used to determine which '
                      'commits from the |patch-ref| actually constitute a '
                      'patch.')
    parser.add_option(
        '-t',
        '--download-topics',
        action='store_true',
        help='Downloads and patches locally changes from all open '
        'Gerrit CLs that have the same topic as the changes '
        'in the specified patch_refs. Only works if atleast '
        'one --patch-ref is specified.')
    parser.add_option('--with_branch_heads',
                      action='store_true',
                      help='Clone git "branch_heads" refspecs in addition to '
                      'the default refspecs. This adds about 1/2GB to a '
                      'full checkout. (git only)')
    parser.add_option(
        '--with_tags',
        action='store_true',
        help='Clone git tags in addition to the default refspecs.')
    parser.add_option('-H',
                      '--head',
                      action='store_true',
                      help='DEPRECATED: only made sense with safesync urls.')
    parser.add_option(
        '-D',
        '--delete_unversioned_trees',
        action='store_true',
        help='Deletes from the working copy any dependencies that '
        'have been removed since the last sync, as long as '
        'there are no local modifications. When used with '
        '--force, such dependencies are removed even if they '
        'have local modifications. When used with --reset, '
        'all untracked directories are removed from the '
        'working copy, excluding those which are explicitly '
        'ignored in the repository.')
    parser.add_option(
        '-R',
        '--reset',
        action='store_true',
        help='resets any local changes before updating (git only)')
    parser.add_option('-M',
                      '--merge',
                      action='store_true',
                      help='merge upstream changes instead of trying to '
                      'fast-forward or rebase')
    parser.add_option('-A',
                      '--auto_rebase',
                      action='store_true',
                      help='Automatically rebase repositories against local '
                      'checkout during update (git only).')
    parser.add_option('--deps',
                      dest='deps_os',
                      metavar='OS_LIST',
                      help='override deps for the specified (comma-separated) '
                      'platform(s); \'all\' will process all deps_os '
                      'references')
    parser.add_option('--process-all-deps',
                      action='store_true',
                      help='Check out all deps, even for different OS-es, '
                      'or with conditions evaluating to false')
    parser.add_option('--upstream',
                      action='store_true',
                      help='Make repo state match upstream branch.')
    parser.add_option('--output-json',
                      help='Output a json document to this path containing '
                      'summary information about the sync.')
    parser.add_option(
        '--no-history',
        action='store_true',
        help='GIT ONLY - Reduces the size/time of the checkout at '
        'the cost of no history. Requires Git 1.9+')
    parser.add_option('--shallow',
                      action='store_true',
                      help='GIT ONLY - Do a shallow clone into the cache dir. '
                      'Requires Git 1.9+')
    parser.add_option('--no_bootstrap',
                      '--no-bootstrap',
                      action='store_true',
                      help='Don\'t bootstrap from Google Storage.')
    parser.add_option('--ignore_locks',
                      action='store_true',
                      help='No longer used.')
    parser.add_option('--break_repo_locks',
                      action='store_true',
                      help='No longer used.')
    parser.add_option('--lock_timeout',
                      type='int',
                      default=5000,
                      help='GIT ONLY - Deadline (in seconds) to wait for git '
                      'cache lock to become available. Default is %default.')
    parser.add_option('--no-rebase-patch-ref',
                      action='store_false',
                      dest='rebase_patch_ref',
                      default=True,
                      help='Bypass rebase of the patch ref after checkout.')
    parser.add_option('--no-reset-patch-ref',
                      action='store_false',
                      dest='reset_patch_ref',
                      default=True,
                      help='Bypass calling reset after patching the ref.')
    parser.add_option('--experiment',
                      action='append',
                      dest='experiments',
                      default=[],
                      help='Which experiments should be enabled.')
    (options, args) = parser.parse_args(args)
    client = GClient.LoadCurrentConfig(options)

    if not client:
        raise gclient_utils.Error(
            'client not configured; see \'gclient config\'')

    if options.download_topics and not options.rebase_patch_ref:
        raise gclient_utils.Error(
            'Warning: You cannot download topics and not rebase each patch ref')

    if options.ignore_locks:
        print(
            'Warning: ignore_locks is no longer used. Please remove its usage.')

    if options.break_repo_locks:
        print('Warning: break_repo_locks is no longer used. Please remove its '
              'usage.')

    if options.revisions and options.head:
        # TODO(maruel): Make it a parser.error if it doesn't break any builder.
        print('Warning: you cannot use both --head and --revision')

    if options.verbose:
        client.PrintLocationAndContents()

    if gclient_utils.IsEnvCog():
        ret = client.RunOnDeps('runhooks', args)
    else:
        ret = client.RunOnDeps('update', args)
    if options.output_json:
        slns = {}
        for d in client.subtree(True):
            normed = d.name.replace('\\', '/').rstrip('/') + '/'
            slns[normed] = {
                'revision': d.got_revision,
                'scm': d.used_scm.name if d.used_scm else None,
                'url': str(d.url) if d.url else None,
                'was_processed': d.should_process,
                'was_synced': d._should_sync,
            }
        with open(options.output_json, 'w') as f:
            json.dump({'solutions': slns}, f)
    return ret


CMDupdate = CMDsync


@metrics.collector.collect_metrics('gclient validate')
def CMDvalidate(parser, args):
    """Validates the .gclient and DEPS syntax."""
    options, args = parser.parse_args(args)
    client = GClient.LoadCurrentConfig(options)
    if not client:
        raise gclient_utils.Error(
            'client not configured; see \'gclient config\'')
    rv = client.RunOnDeps('validate', args)
    if rv == 0:
        print('validate: SUCCESS')
    else:
        print('validate: FAILURE')
    return rv


@metrics.collector.collect_metrics('gclient diff')
def CMDdiff(parser, args):
    """Displays local diff for every dependencies."""
    parser.add_option('--deps',
                      dest='deps_os',
                      metavar='OS_LIST',
                      help='override deps for the specified (comma-separated) '
                      'platform(s); \'all\' will process all deps_os '
                      'references')
    (options, args) = parser.parse_args(args)
    client = GClient.LoadCurrentConfig(options)
    if not client:
        raise gclient_utils.Error(
            'client not configured; see \'gclient config\'')
    if options.verbose:
        client.PrintLocationAndContents()
    return client.RunOnDeps('diff', args)


@metrics.collector.collect_metrics('gclient revert')
def CMDrevert(parser, args):
    """Reverts all modifications in every dependencies.

    That's the nuclear option to get back to a 'clean' state. It removes
    anything that shows up in git status."""
    if gclient_utils.IsEnvCog():
        raise gclient_utils.Error(
            'gclient revert command is not supported. Please navigate to '
            'source control view in the activity bar to discard changes '
            'instead.')
    parser.add_option('--deps',
                      dest='deps_os',
                      metavar='OS_LIST',
                      help='override deps for the specified (comma-separated) '
                      'platform(s); \'all\' will process all deps_os '
                      'references')
    parser.add_option('-n',
                      '--nohooks',
                      action='store_true',
                      help='don\'t run hooks after the revert is complete')
    parser.add_option('-p',
                      '--noprehooks',
                      action='store_true',
                      help='don\'t run pre-DEPS hooks',
                      default=False)
    parser.add_option('--upstream',
                      action='store_true',
                      help='Make repo state match upstream branch.')
    parser.add_option('--break_repo_locks',
                      action='store_true',
                      help='No longer used.')
    (options, args) = parser.parse_args(args)
    if options.break_repo_locks:
        print(
            'Warning: break_repo_locks is no longer used. Please remove its ' +
            'usage.')

    # --force is implied.
    options.force = True
    options.reset = False
    options.delete_unversioned_trees = False
    options.merge = False
    client = GClient.LoadCurrentConfig(options)
    if not client:
        raise gclient_utils.Error(
            'client not configured; see \'gclient config\'')
    return client.RunOnDeps('revert', args)


@metrics.collector.collect_metrics('gclient runhooks')
def CMDrunhooks(parser, args):
    """Runs hooks for files that have been modified in the local working copy."""
    parser.add_option('--deps',
                      dest='deps_os',
                      metavar='OS_LIST',
                      help='override deps for the specified (comma-separated) '
                      'platform(s); \'all\' will process all deps_os '
                      'references')
    parser.add_option('-f',
                      '--force',
                      action='store_true',
                      default=True,
                      help='Deprecated. No effect.')
    (options, args) = parser.parse_args(args)
    client = GClient.LoadCurrentConfig(options)
    if not client:
        raise gclient_utils.Error(
            'client not configured; see \'gclient config\'')
    if options.verbose:
        client.PrintLocationAndContents()
    options.force = True
    options.nohooks = False
    return client.RunOnDeps('runhooks', args)


# TODO(crbug.com/1481266): Collect merics for installhooks.
def CMDinstallhooks(parser, args):
    """Installs gclient git hooks.

    Currently only installs a pre-commit hook to drop staged gitlinks. To
    bypass this pre-commit hook once it's installed, set the environment
    variable SKIP_GITLINK_PRECOMMIT=1.
    """
    (options, args) = parser.parse_args(args)
    client = GClient.LoadCurrentConfig(options)
    if not client:
        raise gclient_utils.Error(
            'client not configured; see \'gclient config\'')
    client._InstallPreCommitHook()
    return 0


@metrics.collector.collect_metrics('gclient revinfo')
def CMDrevinfo(parser, args):
    """Outputs revision info mapping for the client and its dependencies.

    This allows the capture of an overall 'revision' for the source tree that
    can be used to reproduce the same tree in the future. It is only useful for
    'unpinned dependencies', i.e. DEPS/deps references without a git hash.
    A git branch name isn't 'pinned' since the actual commit can change.
    """
    parser.add_option('--deps',
                      dest='deps_os',
                      metavar='OS_LIST',
                      help='override deps for the specified (comma-separated) '
                      'platform(s); \'all\' will process all deps_os '
                      'references')
    parser.add_option(
        '-a',
        '--actual',
        action='store_true',
        help='gets the actual checked out revisions instead of the '
        'ones specified in the DEPS and .gclient files')
    parser.add_option('-s',
                      '--snapshot',
                      action='store_true',
                      help='creates a snapshot .gclient file of the current '
                      'version of all repositories to reproduce the tree, '
                      'implies -a')
    parser.add_option(
        '--filter',
        action='append',
        dest='filter',
        help='Display revision information only for the specified '
        'dependencies (filtered by URL or path).')
    parser.add_option('--output-json',
                      help='Output a json document to this path containing '
                      'information about the revisions.')
    parser.add_option(
        '--ignore-dep-type',
        choices=['git', 'cipd', 'gcs'],
        action='append',
        default=[],
        help='Specify to skip processing of a certain type of dep.')
    (options, args) = parser.parse_args(args)
    client = GClient.LoadCurrentConfig(options)
    if not client:
        raise gclient_utils.Error(
            'client not configured; see \'gclient config\'')
    client.PrintRevInfo()
    return 0


@metrics.collector.collect_metrics('gclient getdep')
def CMDgetdep(parser, args):
    """Gets revision information and variable values from a DEPS file.

    If key doesn't exist or is incorrectly declared, this script exits with exit
    code 2."""
    parser.add_option('--var',
                      action='append',
                      dest='vars',
                      metavar='VAR',
                      default=[],
                      help='Gets the value of a given variable.')
    parser.add_option(
        '-r',
        '--revision',
        action='append',
        dest='getdep_revisions',
        metavar='DEP',
        default=[],
        help='Gets the revision/version for the given dependency. '
        'If it is a git dependency, dep must be a path. If it '
        'is a CIPD dependency, dep must be of the form '
        'path:package.')
    parser.add_option(
        '--deps-file',
        default='DEPS',
        # TODO(ehmaldonado): Try to find the DEPS file pointed by
        # .gclient first.
        help='The DEPS file to be edited. Defaults to the DEPS '
        'file in the current directory.')
    (options, args) = parser.parse_args(args)

    if not os.path.isfile(options.deps_file):
        raise gclient_utils.Error('DEPS file %s does not exist.' %
                                  options.deps_file)
    with open(options.deps_file) as f:
        contents = f.read()
    client = GClient.LoadCurrentConfig(options)
    if client is not None:
        builtin_vars = client.get_builtin_vars()
    else:
        logging.warning(
            'Couldn\'t find a valid gclient config. Will attempt to parse the DEPS '
            'file without support for built-in variables.')
        builtin_vars = None
    local_scope = gclient_eval.Exec(contents,
                                    options.deps_file,
                                    builtin_vars=builtin_vars)

    for var in options.vars:
        print(gclient_eval.GetVar(local_scope, var))

    commits = {}
    if local_scope.get(
            'git_dependencies'
    ) == gclient_eval.SUBMODULES and options.getdep_revisions:
        commits.update(
            scm_git.GIT.GetSubmoduleCommits(
                os.getcwd(),
                [path for path in options.getdep_revisions if ':' not in path]))

    for name in options.getdep_revisions:
        if ':' in name:
            name, _, package = name.partition(':')
            if not name or not package:
                parser.error(
                    'Wrong CIPD format: %s:%s should be of the form path:pkg.' %
                    (name, package))
            print(gclient_eval.GetCIPD(local_scope, name, package))
        elif commits:
            print(commits[name])
        else:
            try:
                print(gclient_eval.GetRevision(local_scope, name))
            except KeyError as e:
                print(repr(e), file=sys.stderr)
                sys.exit(2)


@metrics.collector.collect_metrics('gclient setdep')
def CMDsetdep(parser, args):
    """Modifies dependency revisions and variable values in a DEPS file"""
    parser.add_option('--var',
                      action='append',
                      dest='vars',
                      metavar='VAR=VAL',
                      default=[],
                      help='Sets a variable to the given value with the format '
                      'name=value.')
    parser.add_option('-r',
                      '--revision',
                      action='append',
                      dest='setdep_revisions',
                      metavar='DEP@REV',
                      default=[],
                      help='Sets the revision/version for the dependency with '
                      'the format dep@rev. If it is a git dependency, dep '
                      'must be a path and rev must be a git hash or '
                      'reference (e.g. src/dep@deadbeef). If it is a CIPD '
                      'dependency, dep must be of the form path:package and '
                      'rev must be the package version '
                      '(e.g. src/pkg:chromium/pkg@2.1-cr0). '
                      'If it is a GCS dependency, dep must be of the form '
                      'path@object_name,sha256sum,size_bytes,generation?'
                      'object_name2,sha256sum2,size_bytes2,generation2?... '
                      'The number of revision objects for a given path must '
                      'match the current number of revision objects for that '
                      'path, and objects order will be preserved.')
    parser.add_option(
        '--deps-file',
        default='DEPS',
        # TODO(ehmaldonado): Try to find the DEPS file pointed by
        # .gclient first.
        help='The DEPS file to be edited. Defaults to the DEPS '
        'file in the current directory.')
    (options, args) = parser.parse_args(args)
    if args:
        parser.error('Unused arguments: "%s"' % '" "'.join(args))
    if not options.setdep_revisions and not options.vars:
        parser.error(
            'You must specify at least one variable or revision to modify.')

    if not os.path.isfile(options.deps_file):
        raise gclient_utils.Error('DEPS file %s does not exist.' %
                                  options.deps_file)
    with open(options.deps_file) as f:
        contents = f.read()

    client = GClient.LoadCurrentConfig(options)
    if client is not None:
        builtin_vars = client.get_builtin_vars()
    else:
        logging.warning(
            'Couldn\'t find a valid gclient config. Will attempt to parse the DEPS '
            'file without support for built-in variables.')
        builtin_vars = None

    local_scope = gclient_eval.Exec(contents,
                                    options.deps_file,
                                    builtin_vars=builtin_vars)

    # Create a set of all git submodules.
    cwd = os.path.dirname(options.deps_file) or os.getcwd()
    git_modules = None
    is_cog = gclient_utils.IsEnvCog()
    if (not is_cog and 'git_dependencies' in local_scope
            and local_scope['git_dependencies']
            in (gclient_eval.SUBMODULES, gclient_eval.SYNC)):
        cmd = ['git', 'ls-files', '--format=%(objectmode) %(path)']
        with subprocess2.Popen(
                cmd,
                cwd=cwd,
                stdout=subprocess2.PIPE,
                stderr=None if options.verbose else subprocess2.DEVNULL,
                text=True) as p:
            git_modules = {
                line.split()[1].strip()
                for line in p.stdout if line.startswith('160000')
            }
        if p.returncode != 0:
            print('Warning: gitlinks won\'t be updated because computing '
                  'submodules has failed.')
            git_modules = None

    for var in options.vars:
        name, _, value = var.partition('=')
        if not name or not value:
            parser.error(
                'Wrong var format: %s should be of the form name=value.' % var)
        if name in local_scope['vars']:
            gclient_eval.SetVar(local_scope, name, value)
        else:
            gclient_eval.AddVar(local_scope, name, value)

    for revision in options.setdep_revisions:
        name, _, value = revision.partition('@')
        if not name or not value:
            parser.error('Wrong dep format: %s should be of the form dep@rev.' %
                         revision)
        if ':' in name:
            name, _, package = name.partition(':')
            if not name or not package:
                parser.error(
                    'Wrong CIPD format: %s:%s should be of the form path:pkg@version.'
                    % (name, package))
            gclient_eval.SetCIPD(local_scope, name, package, value)
        elif ',' in value:
            objects = []
            raw_objects = value.split('?')
            for o in raw_objects:
                object_info = o.split(',')
                if len(object_info) != 4:
                    parser.error(
                        'All values are required in the revision object: '
                        'object_name, sha256sum, size_bytes and generation.')
                objects.append({
                    'object_name': object_info[0],
                    'sha256sum': object_info[1],
                    'size_bytes': object_info[2],
                    'generation': object_info[3],
                })
            gclient_eval.SetGCS(local_scope, name, objects)
        else:  # git dependencies
            # Update DEPS only when `git_dependencies` == DEPS or SYNC.
            # git_dependencies is defaulted to DEPS when not set.
            if 'git_dependencies' not in local_scope or local_scope[
                    'git_dependencies'] in (gclient_eval.DEPS,
                                            gclient_eval.SYNC):
                gclient_eval.SetRevision(local_scope, name, value)

            # Update git submodules when `git_dependencies` == SYNC or
            # SUBMODULES.
            if git_modules and 'git_dependencies' in local_scope and local_scope[
                    'git_dependencies'] in (gclient_eval.SUBMODULES,
                                            gclient_eval.SYNC):
                if is_cog:
                    parser.error(
                        f'Set git dependency "{name}" is currently not '
                        'supported.')
                git_module_name = name
                if not 'use_relative_paths' in local_scope or \
                    local_scope['use_relative_paths'] != True:
                    deps_dir = os.path.dirname(
                        os.path.abspath(options.deps_file))
                    gclient_path = gclient_paths.FindGclientRoot(deps_dir)
                    delta_path = None
                    if gclient_path:
                        delta_path = os.path.relpath(
                            deps_dir, os.path.abspath(gclient_path))
                    if delta_path:
                        prefix_length = len(delta_path.replace(
                            os.path.sep, '/')) + 1
                        git_module_name = name[prefix_length:]
                # gclient setdep should update the revision, i.e., the gitlink
                # only when the submodule entry is already present within
                # .gitmodules.
                if git_module_name not in git_modules:
                    raise KeyError(
                        f'Could not find any dependency called "{git_module_name}" in '
                        f'.gitmodules.')

                # Update the gitlink for the submodule.
                subprocess2.call([
                    'git', 'update-index', '--add', '--cacheinfo',
                    f'160000,{value},{git_module_name}'
                ],
                                 cwd=cwd)

    with open(options.deps_file, 'wb') as f:
        f.write(gclient_eval.RenderDEPSFile(local_scope).encode('utf-8'))

    if git_modules:
        subprocess2.call(['git', 'add', options.deps_file], cwd=cwd)
        print('Changes have been staged. See changes with `git status`.\n'
              'Use `git commit -m "Manual roll"` to commit your changes. \n'
              'Run gclient sync to update your local dependency checkout.')


@metrics.collector.collect_metrics('gclient verify')
def CMDverify(parser, args):
    """Verifies the DEPS file deps are only from allowed_hosts."""
    (options, args) = parser.parse_args(args)
    client = GClient.LoadCurrentConfig(options)
    if not client:
        raise gclient_utils.Error(
            'client not configured; see \'gclient config\'')
    client.RunOnDeps(None, [])
    # Look at each first-level dependency of this gclient only.
    for dep in client.dependencies:
        bad_deps = dep.findDepsFromNotAllowedHosts()
        if not bad_deps:
            continue
        print("There are deps from not allowed hosts in file %s" %
              dep.deps_file)
        for bad_dep in bad_deps:
            print("\t%s at %s" % (bad_dep.name, bad_dep.url))
        print("allowed_hosts:", ', '.join(dep.allowed_hosts))
        sys.stdout.flush()
        raise gclient_utils.Error(
            'dependencies from disallowed hosts; check your DEPS file.')
    return 0


@subcommand.epilog("""For more information on what metrics are we collecting and
why, please read metrics.README.md or visit https://bit.ly/2ufRS4p""")
@metrics.collector.collect_metrics('gclient metrics')
def CMDmetrics(parser, args):
    """Reports, and optionally modifies, the status of metric collection."""
    parser.add_option('--opt-in',
                      action='store_true',
                      dest='enable_metrics',
                      help='Opt-in to metrics collection.',
                      default=None)
    parser.add_option('--opt-out',
                      action='store_false',
                      dest='enable_metrics',
                      help='Opt-out of metrics collection.')
    options, args = parser.parse_args(args)
    if args:
        parser.error('Unused arguments: "%s"' % '" "'.join(args))
    if not metrics.collector.config.is_googler:
        print("You're not a Googler. Metrics collection is disabled for you.")
        return 0

    if options.enable_metrics is not None:
        metrics.collector.config.opted_in = options.enable_metrics

    if metrics.collector.config.opted_in is None:
        print("You haven't opted in or out of metrics collection.")
    elif metrics.collector.config.opted_in:
        print("You have opted in. Thanks!")
    else:
        print("You have opted out. Please consider opting in.")
    return 0


class OptionParser(optparse.OptionParser):
    gclientfile_default = os.environ.get('GCLIENT_FILE', '.gclient')

    def __init__(self, **kwargs):
        optparse.OptionParser.__init__(self,
                                       version='%prog ' + __version__,
                                       **kwargs)

        # Some arm boards have issues with parallel sync.
        if platform.machine().startswith('arm'):
            jobs = 1
        else:
            jobs = max(8, gclient_utils.NumLocalCpus())

        self.add_option(
            '-j',
            '--jobs',
            default=jobs,
            type='int',
            help='Specify how many SCM commands can run in parallel; defaults to '
            '%default on this machine')
        self.add_option(
            '-v',
            '--verbose',
            action='count',
            default=0,
            help='Produces additional output for diagnostics. Can be used up to '
            'three times for more logging info.')
        self.add_option('--gclientfile',
                        dest='config_filename',
                        help='Specify an alternate %s file' %
                        self.gclientfile_default)
        self.add_option(
            '--spec',
            help='create a gclient file containing the provided string. Due to '
            'Cygwin/Python brokenness, it can\'t contain any newlines.')
        self.add_option('--no-nag-max',
                        default=False,
                        action='store_true',
                        help='Ignored for backwards compatibility.')

    def parse_args(self, args=None, _values=None):
        """Integrates standard options processing."""
        # Create an optparse.Values object that will store only the actual
        # passed options, without the defaults.
        actual_options = optparse.Values()
        _, args = optparse.OptionParser.parse_args(self, args, actual_options)
        # Create an optparse.Values object with the default options.
        options = optparse.Values(self.get_default_values().__dict__)
        # Update it with the options passed by the user.
        options._update_careful(actual_options.__dict__)
        # Store the options passed by the user in an _actual_options attribute.
        # We store only the keys, and not the values, since the values can
        # contain arbitrary information, which might be PII.
        metrics.collector.add('arguments', list(actual_options.__dict__))

        levels = [logging.ERROR, logging.WARNING, logging.INFO, logging.DEBUG]
        logging.basicConfig(
            level=levels[min(options.verbose,
                             len(levels) - 1)],
            format='%(module)s(%(lineno)d) %(funcName)s:%(message)s')
        if options.config_filename and options.spec:
            self.error('Cannot specify both --gclientfile and --spec')
        if (options.config_filename and options.config_filename !=
                os.path.basename(options.config_filename)):
            self.error('--gclientfile target must be a filename, not a path')
        if not options.config_filename:
            options.config_filename = self.gclientfile_default
        options.entries_filename = options.config_filename + '_entries'
        if options.jobs < 1:
            self.error('--jobs must be 1 or higher')

        # These hacks need to die.
        if not hasattr(options, 'revisions'):
            # GClient.RunOnDeps expects it even if not applicable.
            options.revisions = []
        if not hasattr(options, 'experiments'):
            options.experiments = []
        if not hasattr(options, 'head'):
            options.head = None
        if not hasattr(options, 'nohooks'):
            options.nohooks = True
        if not hasattr(options, 'noprehooks'):
            options.noprehooks = True
        if not hasattr(options, 'deps_os'):
            options.deps_os = None
        if not hasattr(options, 'force'):
            options.force = None
        return (options, args)


def disable_buffering():
    # Make stdout auto-flush so buildbot doesn't kill us during lengthy
    # operations. Python as a strong tendency to buffer sys.stdout.
    sys.stdout = gclient_utils.MakeFileAutoFlush(sys.stdout)
    # Make stdout annotated with the thread ids.
    sys.stdout = gclient_utils.MakeFileAnnotated(sys.stdout)


def path_contains_tilde():
    for element in os.environ['PATH'].split(os.pathsep):
        if element.startswith('~') and os.path.abspath(
                os.path.realpath(
                    os.path.expanduser(element))) == DEPOT_TOOLS_DIR:
            return True
    return False


def can_run_gclient_and_helpers():
    if not sys.executable:
        print('\nPython cannot find the location of it\'s own executable.\n',
              file=sys.stderr)
        return False
    if path_contains_tilde():
        print(
            '\nYour PATH contains a literal "~", which works in some shells ' +
            'but will break when python tries to run subprocesses. ' +
            'Replace the "~" with $HOME.\n' + 'See https://crbug.com/952865.\n',
            file=sys.stderr)
        return False
    return True


def main(argv):
    """Doesn't parse the arguments here, just find the right subcommand to
    execute."""
    if not can_run_gclient_and_helpers():
        return 2
    disable_buffering()
    setup_color.init()
    dispatcher = subcommand.CommandDispatcher(__name__)
    try:
        return dispatcher.execute(OptionParser(), argv)
    except KeyboardInterrupt:
        gclient_utils.GClientChildren.KillAllRemainingChildren()
        raise
    except (gclient_utils.Error, subprocess2.CalledProcessError) as e:
        print('Error: %s' % str(e), file=sys.stderr)
        return 1
    finally:
        gclient_utils.PrintWarnings()
    return 0


if '__main__' == __name__:
    with metrics.collector.print_notice_and_exit():
        sys.exit(main(sys.argv[1:]))

# vim: ts=2:sw=2:tw=80:et:
