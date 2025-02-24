# Copyright (c) 2012 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
"""Gclient-specific SCM-specific operations."""

import collections
import contextlib
import errno
import glob
import json
import logging
import os
import platform
import posixpath
import re
import shutil
import sys
import tempfile
import threading
import traceback

import gclient_utils
import gerrit_util
import git_auth
import git_cache
import git_common
import newauth
import scm
import subprocess2

# TODO: Should fix these warnings.
# pylint: disable=line-too-long


class NoUsableRevError(gclient_utils.Error):
    """Raised if requested revision isn't found in checkout."""


class DiffFiltererWrapper(object):
    """Simple base class which tracks which file is being diffed and
  replaces instances of its file name in the original and
  working copy lines of the git diff output."""
    index_string = None
    original_prefix = "--- "
    working_prefix = "+++ "

    def __init__(self, relpath, print_func):
        # Note that we always use '/' as the path separator to be
        # consistent with cygwin-style output on Windows
        self._relpath = relpath.replace("\\", "/")
        self._current_file = None
        self._print_func = print_func

    def SetCurrentFile(self, current_file):
        self._current_file = current_file

    @property
    def _replacement_file(self):
        return posixpath.join(self._relpath, self._current_file)

    def _Replace(self, line):
        return line.replace(self._current_file, self._replacement_file)

    def Filter(self, line):
        if (line.startswith(self.index_string)):
            self.SetCurrentFile(line[len(self.index_string):])
            line = self._Replace(line)
        else:
            if (line.startswith(self.original_prefix)
                    or line.startswith(self.working_prefix)):
                line = self._Replace(line)
        self._print_func(line)


class GitDiffFilterer(DiffFiltererWrapper):
    index_string = "diff --git "

    def SetCurrentFile(self, current_file):
        # Get filename by parsing "a/<filename> b/<filename>"
        self._current_file = current_file[:(len(current_file) / 2)][2:]

    def _Replace(self, line):
        return re.sub("[a|b]/" + self._current_file, self._replacement_file,
                      line)


# SCMWrapper base class


class SCMWrapper(object):
    """Add necessary glue between all the supported SCM.

  This is the abstraction layer to bind to different SCM.
  """
    def __init__(self,
                 url=None,
                 root_dir=None,
                 relpath=None,
                 out_fh=None,
                 out_cb=None,
                 print_outbuf=False):
        self.url = url
        self._root_dir = root_dir
        if self._root_dir:
            self._root_dir = self._root_dir.replace('/', os.sep)
        self.relpath = relpath
        if self.relpath:
            self.relpath = self.relpath.replace('/', os.sep)
        if self.relpath and self._root_dir:
            self.checkout_path = os.path.join(self._root_dir, self.relpath)
        if out_fh is None:
            out_fh = sys.stdout
        self.out_fh = out_fh
        self.out_cb = out_cb
        self.print_outbuf = print_outbuf

    def Print(self, *args, **kwargs):
        kwargs.setdefault('file', self.out_fh)
        if kwargs.pop('timestamp', True):
            self.out_fh.write('[%s] ' % gclient_utils.Elapsed())
        print(*args, **kwargs)

    def RunCommand(self, command, options, args, file_list=None):
        commands = [
            'update', 'updatesingle', 'revert', 'revinfo', 'status', 'diff',
            'pack', 'runhooks'
        ]

        if not command in commands:
            raise gclient_utils.Error('Unknown command %s' % command)

        if not command in dir(self):
            raise gclient_utils.Error(
                'Command %s not implemented in %s wrapper' %
                (command, self.__class__.__name__))

        return getattr(self, command)(options, args, file_list)

    @staticmethod
    def _get_first_remote_url(checkout_path):
        log = scm.GIT.YieldConfigRegexp(checkout_path, r'remote.*.url')
        return next(log)[1]

    def GetCacheMirror(self):
        if getattr(self, 'cache_dir', None):
            url, _ = gclient_utils.SplitUrlRevision(self.url)
            return git_cache.Mirror(url)
        return None

    def GetActualRemoteURL(self, options):
        """Attempt to determine the remote URL for this SCMWrapper."""
        # Git
        if os.path.exists(os.path.join(self.checkout_path, '.git')):
            actual_remote_url = self._get_first_remote_url(self.checkout_path)

            mirror = self.GetCacheMirror()
            # If the cache is used, obtain the actual remote URL from there.
            if (mirror and mirror.exists() and mirror.mirror_path.replace(
                    '\\', '/') == actual_remote_url.replace('\\', '/')):
                actual_remote_url = self._get_first_remote_url(
                    mirror.mirror_path)
            return actual_remote_url
        return None

    def DoesRemoteURLMatch(self, options):
        """Determine whether the remote URL of this checkout is the expected URL."""
        if not os.path.exists(self.checkout_path):
            # A checkout which doesn't exist can't be broken.
            return True

        actual_remote_url = self.GetActualRemoteURL(options)
        if actual_remote_url:
            return (gclient_utils.SplitUrlRevision(actual_remote_url)[0].rstrip(
                '/') == gclient_utils.SplitUrlRevision(self.url)[0].rstrip('/'))

        # This may occur if the self.checkout_path exists but does not contain a
        # valid git checkout.
        return False

    def _DeleteOrMove(self, force):
        """Delete the checkout directory or move it out of the way.

    Args:
        force: bool; if True, delete the directory. Otherwise, just move it.
    """
        if force and os.environ.get('CHROME_HEADLESS') == '1':
            self.Print('_____ Conflicting directory found in %s. Removing.' %
                       self.checkout_path)
            gclient_utils.AddWarning('Conflicting directory %s deleted.' %
                                     self.checkout_path)
            gclient_utils.rmtree(self.checkout_path)
        else:
            bad_scm_dir = os.path.join(self._root_dir, '_bad_scm',
                                       os.path.dirname(self.relpath))

            try:
                os.makedirs(bad_scm_dir)
            except OSError as e:
                if e.errno != errno.EEXIST:
                    raise

            dest_path = tempfile.mkdtemp(prefix=os.path.basename(self.relpath),
                                         dir=bad_scm_dir)
            self.Print(
                '_____ Conflicting directory found in %s. Moving to %s.' %
                (self.checkout_path, dest_path))
            gclient_utils.AddWarning('Conflicting directory %s moved to %s.' %
                                     (self.checkout_path, dest_path))
            shutil.move(self.checkout_path, dest_path)


class GitWrapper(SCMWrapper):
    """Wrapper for Git"""
    name = 'git'
    remote = 'origin'

    @property
    def cache_dir(self):
        try:
            return git_cache.Mirror.GetCachePath()
        except RuntimeError:
            return None

    def __init__(self, url=None, *args, **kwargs):
        """Removes 'git+' fake prefix from git URL."""
        if url and (url.startswith('git+http://')
                    or url.startswith('git+https://')):
            url = url[4:]
        SCMWrapper.__init__(self, url, *args, **kwargs)
        filter_kwargs = {'time_throttle': 1, 'out_fh': self.out_fh}
        if self.out_cb:
            filter_kwargs['predicate'] = self.out_cb
        self.filter = gclient_utils.GitFilter(**filter_kwargs)
        self._running_under_rosetta = None
        self.current_revision = None

    def GetCheckoutRoot(self):
        return scm.GIT.GetCheckoutRoot(self.checkout_path)

    def GetRevisionDate(self, _revision):
        """Returns the given revision's date in ISO-8601 format (which contains the
    time zone)."""
        # TODO(floitsch): get the time-stamp of the given revision and not just
        # the time-stamp of the currently checked out revision.
        return self._Capture(['log', '-n', '1', '--format=%ai'])

    def _GetDiffFilenames(self, base):
        """Returns the names of files modified since base."""
        return self._Capture(
            # Filter to remove base if it is None.
            list(
                filter(
                    bool,
                    ['-c', 'core.quotePath=false', 'diff', '--name-only', base])
            )).split()

    def GetSubmoduleStateFromIndex(self):
        """Returns a map where keys are submodule names and values are commit
        hashes. It reads data from the Git index, so only committed values are
        present."""
        out = self._Capture(['ls-files', '-s'])
        result = {}
        for l in out.split('\n'):
            if not l.startswith('160000'):
                # Not a submodule
                continue
            (_, commit, _, filepath) = l.split(maxsplit=3)
            result[filepath] = commit
        return result

    def GetSubmoduleDiff(self):
        """Returns a map where keys are submodule names and values are tuples of
        (old_commit_hash, new_commit_hash). old_commit_hash matches the Git
        index, whereas new_commit_hash matches currently checked out commit
        hash."""
        out = self._Capture([
            'diff',
            '--no-prefix',
            '--no-ext-diff',
            '--no-color',
            '--ignore-submodules=dirty',
            '--submodule=short',
            '-G',
            'Subproject commit',
        ])
        NO_COMMIT = 40 * '0'
        committed_submodule = None
        checked_submodule = None
        filepath = None
        state = 0
        diff = {}
        # Parsing git diff uses simple state machine. States:
        # 0 - start state
        # 1 - diff file/line detected, ready to process content
        # 2 - gitlink detected, ready to process gitlink past and current
        # content.
        # 3 - past gitlink content detected. It contains a commit hash that's in
        # git index.
        # 4 - new gitlink content detected. It contains currently checked
        # commit. At this point, we have all information needed, and we can
        # reset state to 0.
        for l in out.split('\n'):
            if l.startswith('diff --git'):
                # New file detected, reset state.
                state = 1
            elif state == 1 and l.startswith('index') and l.endswith('160000'):
                # We detected gitlink
                state = 2
            elif state == 2 and l.startswith('+++ '):
                # This line contains filename
                filepath = l[4:]
                state = 3
            elif state == 3 and l.startswith('-Subproject commit '):
                # This line contains what commit hash Git index expects
                # (ls-files).
                committed_submodule = l.split(' ')[-1]
                state = 4
            elif state == 4 and l.startswith('+Subproject commit '):
                # This line contains currently checked out commit for this submodule.
                checked_submodule = l.split(' ')[-1]
                if NO_COMMIT not in (committed_submodule, checked_submodule):
                    diff[filepath] = (committed_submodule, checked_submodule)
                state = 0
        return diff

    def diff(self, options, _args, _file_list):
        _, revision = gclient_utils.SplitUrlRevision(self.url)
        if not revision:
            revision = 'refs/remotes/%s/main' % self.remote
        self._Run(['-c', 'core.quotePath=false', 'diff', revision], options)

    def pack(self, _options, _args, _file_list):
        """Generates a patch file which can be applied to the root of the
    repository.

    The patch file is generated from a diff of the merge base of HEAD and
    its upstream branch.
    """
        try:
            merge_base = [self._Capture(['merge-base', 'HEAD', self.remote])]
        except subprocess2.CalledProcessError:
            merge_base = []
        gclient_utils.CheckCallAndFilter(['git', 'diff'] + merge_base,
                                         cwd=self.checkout_path,
                                         filter_fn=GitDiffFilterer(
                                             self.relpath,
                                             print_func=self.Print).Filter)

    def _Scrub(self, target, options):
        """Scrubs out all changes in the local repo, back to the state of target."""
        quiet = []
        if not options.verbose:
            quiet = ['--quiet']
        self._Run(['reset', '--hard', target] + quiet, options)
        if options.force and options.delete_unversioned_trees:
            # where `target` is a commit that contains both upper and lower case
            # versions of the same file on a case insensitive filesystem, we are
            # actually in a broken state here. The index will have both 'a' and
            # 'A', but only one of them will exist on the disk. To progress, we
            # delete everything that status thinks is modified.
            output = self._Capture(
                ['-c', 'core.quotePath=false', 'status', '--porcelain'],
                strip=False)
            for line in output.splitlines():
                # --porcelain (v1) looks like:
                # XY filename
                try:
                    filename = line[3:]
                    self.Print('_____ Deleting residual after reset: %r.' %
                               filename)
                    gclient_utils.rm_file_or_tree(
                        os.path.join(self.checkout_path, filename))
                except OSError:
                    pass

    def _FetchAndReset(self, revision, file_list, options):
        """Equivalent to git fetch; git reset."""
        self._SetFetchConfig(options)

        self._Fetch(options, prune=True, quiet=options.verbose)
        revision = self._AutoFetchRef(options, revision)
        self._Scrub(revision, options)
        if file_list is not None:
            files = self._Capture(['-c', 'core.quotePath=false',
                                   'ls-files']).splitlines()
            file_list.extend(
                [os.path.join(self.checkout_path, f) for f in files])

    def _DisableHooks(self):
        hook_dir = os.path.join(self.checkout_path, '.git', 'hooks')
        if not os.path.isdir(hook_dir):
            return
        for f in os.listdir(hook_dir):
            if not f.endswith('.sample') and not f.endswith('.disabled'):
                disabled_hook_path = os.path.join(hook_dir, f + '.disabled')
                if os.path.exists(disabled_hook_path):
                    os.remove(disabled_hook_path)
                os.rename(os.path.join(hook_dir, f), disabled_hook_path)

    def _maybe_break_locks(self, options):
        """This removes all .lock files from this repo's .git directory, if the
    user passed the --break_repo_locks command line flag.

    In particular, this will cleanup index.lock files, as well as ref lock
    files.
    """
        if options.break_repo_locks:
            git_dir = os.path.join(self.checkout_path, '.git')
            for path, _, filenames in os.walk(git_dir):
                for filename in filenames:
                    if filename.endswith('.lock'):
                        to_break = os.path.join(path, filename)
                        self.Print('breaking lock: %s' % (to_break, ))
                        try:
                            os.remove(to_break)
                        except OSError as ex:
                            self.Print('FAILED to break lock: %s: %s' %
                                       (to_break, ex))
                            raise

    def _download_topics(self, patch_rev, googlesource_url):
        """This method returns new patch_revs to process that have the same topic.

    It does the following:
    1. Finds the topic of the Gerrit change specified in the patch_rev.
    2. Find all changes with that topic.
    3. Append patch_rev of the changes with the same topic to the patch_revs
       to process.
    4. Returns the new patch_revs to process.
    """
        patch_revs_to_process = []
        # Parse the patch_rev to extract the CL and patchset.
        patch_rev_tokens = patch_rev.split('/')
        change = patch_rev_tokens[-2]
        # Parse the googlesource_url.
        tokens = re.search('//(.+).googlesource.com/(.+?)(?:\.git)?$',
                           googlesource_url)
        if not tokens or len(tokens.groups()) != 2:
            # googlesource_url is not in the expected format.
            return patch_revs_to_process

        # parse the gerrit host and repo out of googlesource_url.
        host, repo = tokens.groups()[:2]
        gerrit_host_url = '%s-review.googlesource.com' % host

        # 1. Find the topic of the Gerrit change specified in the patch_rev.
        change_object = gerrit_util.GetChange(gerrit_host_url, change)
        topic = change_object.get('topic')
        if not topic:
            # This change has no topic set.
            return patch_revs_to_process

        # 2. Find all changes with that topic.
        changes_with_same_topic = gerrit_util.QueryChanges(
            gerrit_host_url, [('topic', topic), ('status', 'open'),
                              ('repo', repo)],
            o_params=['ALL_REVISIONS'])
        for c in changes_with_same_topic:
            if str(c['_number']) == change:
                # This change is already in the patch_rev.
                continue
            self.Print('Found CL %d with the topic name %s' %
                       (c['_number'], topic))
            # 3. Append patch_rev of the changes with the same topic to the
            #    patch_revs to process.
            curr_rev = c['current_revision']
            new_patch_rev = c['revisions'][curr_rev]['ref']
            patch_revs_to_process.append(new_patch_rev)

        # 4. Return the new patch_revs to process.
        return patch_revs_to_process

    def _ref_to_remote_ref(self, target_rev):
        """Helper function for scm.GIT.RefToRemoteRef with error checking.

    Joins the results of scm.GIT.RefToRemoteRef into a string, but raises a
    comprehensible error if RefToRemoteRef fails.

    Args:
      target_rev: a ref somewhere under refs/.
    """
        tmp_ref = scm.GIT.RefToRemoteRef(target_rev, self.remote)
        if not tmp_ref:
            raise gclient_utils.Error(
                'Failed to turn target revision %r in repo %r into remote ref' %
                (target_rev, self.checkout_path))
        return ''.join(tmp_ref)

    def apply_patch_ref(self, patch_repo, patch_rev, target_rev, options,
                        file_list):
        # type: (str, str, str, optparse.Values, Collection[str]) -> str
        """Apply a patch on top of the revision we're synced at.

    The patch ref is given by |patch_repo|@|patch_rev|.
    |target_rev| is usually the branch that the |patch_rev| was uploaded against
    (e.g. 'refs/heads/main'), but this is not required.

    We cherry-pick all commits reachable from |patch_rev| on top of the curret
    HEAD, excluding those reachable from |target_rev|
    (i.e. git cherry-pick target_rev..patch_rev).

    Graphically, it looks like this:

     ... -> o -> [possibly already landed commits] -> target_rev
             \
              -> [possibly not yet landed dependent CLs] -> patch_rev

    The final checkout state is then:

     ... -> HEAD -> [possibly not yet landed dependent CLs] -> patch_rev

    After application, if |options.reset_patch_ref| is specified, we soft reset
    the cherry-picked changes, keeping them in git index only.

    Args:
      patch_repo: The patch origin.
          e.g. 'https://foo.googlesource.com/bar'
      patch_rev: The revision to patch.
          e.g. 'refs/changes/1234/34/1'.
      target_rev: The revision to use when finding the merge base.
          Typically, the branch that the patch was uploaded against.
          e.g. 'refs/heads/main' or 'refs/heads/infra/config'.
      options: The options passed to gclient.
      file_list: A list where modified files will be appended.
    """

        # Abort any cherry-picks in progress.
        try:
            self._Capture(['cherry-pick', '--abort'])
        except subprocess2.CalledProcessError:
            pass

        base_rev = self.revinfo(None, None, None)

        if not target_rev:
            raise gclient_utils.Error(
                'A target revision for the patch must be given')

        if target_rev.startswith(('refs/heads/', 'refs/branch-heads')):
            # If |target_rev| is in refs/heads/** or refs/branch-heads/**, try
            # first to find the corresponding remote ref for it, since
            # |target_rev| might point to a local ref which is not up to date
            # with the corresponding remote ref.
            remote_ref = self._ref_to_remote_ref(target_rev)
            self.Print('Trying the corresponding remote ref for %r: %r\n' %
                       (target_rev, remote_ref))
            if scm.GIT.IsValidRevision(self.checkout_path, remote_ref):
                # refs/remotes may need to be updated to cleanly cherry-pick
                # changes. See https://crbug.com/1255178.
                self._Capture(['fetch', '--no-tags', self.remote, target_rev])
                target_rev = remote_ref
        elif not scm.GIT.IsValidRevision(self.checkout_path, target_rev):
            # Fetch |target_rev| if it's not already available.
            url, _ = gclient_utils.SplitUrlRevision(self.url)
            mirror = self._GetMirror(url, options, target_rev, target_rev)
            if mirror:
                rev_type = 'branch' if target_rev.startswith(
                    'refs/') else 'hash'
                self._UpdateMirrorIfNotContains(mirror, options, rev_type,
                                                target_rev)
            self._Fetch(options, refspec=target_rev)

        patch_revs_to_process = [patch_rev]

        if hasattr(options, 'download_topics') and options.download_topics:
            patch_revs_to_process_from_topics = self._download_topics(
                patch_rev, self.url)
            patch_revs_to_process.extend(patch_revs_to_process_from_topics)

        self._Capture(['reset', '--hard'])
        for pr in patch_revs_to_process:
            self.Print('===Applying patch===')
            self.Print('Revision to patch is %r @ %r.' % (patch_repo, pr))
            self.Print('Current dir is %r' % self.checkout_path)
            self._Capture(['fetch', '--no-tags', patch_repo, pr])
            pr = self._Capture(['rev-parse', 'FETCH_HEAD'])

            if not options.rebase_patch_ref:
                self._Capture(['checkout', pr])
                # Adjust base_rev to be the first parent of our checked out
                # patch ref; This will allow us to correctly extend `file_list`,
                # and will show the correct file-list to programs which do `git
                # diff --cached` expecting to see the patch diff.
                base_rev = self._Capture(['rev-parse', pr + '~'])
            else:
                self.Print('Will cherrypick %r .. %r on top of %r.' %
                           (target_rev, pr, base_rev))
                try:
                    if scm.GIT.IsAncestor(pr,
                                          target_rev,
                                          cwd=self.checkout_path):
                        if len(patch_revs_to_process) > 1:
                            # If there are multiple patch_revs_to_process then
                            # we do not want want to invalidate a previous patch
                            # so throw an error.
                            raise gclient_utils.Error(
                                'patch_rev %s is an ancestor of target_rev %s. This '
                                'situation is unsupported when we need to apply multiple '
                                'patch_revs: %s' %
                                (pr, target_rev, patch_revs_to_process))
                        # If |patch_rev| is an ancestor of |target_rev|, check
                        # it out.
                        self._Capture(['checkout', pr])
                    else:
                        # If a change was uploaded on top of another change,
                        # which has already landed, one of the commits in the
                        # cherry-pick range will be redundant, since it has
                        # already landed and its changes incorporated in the
                        # tree. We pass '--keep-redundant-commits' to ignore
                        # those changes.
                        self._Capture([
                            'cherry-pick', target_rev + '..' + pr,
                            '--keep-redundant-commits'
                        ])

                except subprocess2.CalledProcessError as e:
                    self.Print('Failed to apply patch.')
                    self.Print('Revision to patch was %r @ %r.' %
                               (patch_repo, pr))
                    self.Print('Tried to cherrypick %r .. %r on top of %r.' %
                               (target_rev, pr, base_rev))
                    self.Print('Current dir is %r' % self.checkout_path)
                    self.Print('git returned non-zero exit status %s:\n%s' %
                               (e.returncode, e.stderr.decode('utf-8')))
                    # Print the current status so that developers know what
                    # changes caused the patch failure, since git cherry-pick
                    # doesn't show that information.
                    self.Print(self._Capture(['status']))
                    try:
                        self._Capture(['cherry-pick', '--abort'])
                    except subprocess2.CalledProcessError:
                        pass
                    raise

            if file_list is not None:
                file_list.extend(self._GetDiffFilenames(base_rev))

        latest_commit = self.revinfo(None, None, None)
        if options.reset_patch_ref:
            self._Capture(['reset', '--soft', base_rev])
        return latest_commit

    def check_diff(self, previous_commit, files=None):
        # type: (str, Optional[List[str]]) -> bool
        """Check if a diff exists between the current commit and `previous_commit`.

      Returns True if there were diffs or if an error was encountered.
    """
        cmd = ['diff', previous_commit, '--quiet']
        if files:
            cmd += ['--'] + files
        try:
            self._Capture(cmd)
            return False
        except subprocess2.CalledProcessError as e:
            # git diff --quiet exits with 1 if there were diffs.
            if e.returncode != 1:
                self.Print('git returned non-zero exit status %s:\n%s' %
                           (e.returncode, e.stderr.decode('utf-8')))
            return True

    def set_config(f):
        def wrapper(*args):
            return_val = f(*args)
            checkout_path = args[0].checkout_path
            if os.path.exists(os.path.join(checkout_path, '.git')):
                # The config updates to the project are stored in this list
                # and updated consecutively after the reads. The updates
                # are done this way because `scm.GIT.GetConfig` caches
                # the config file and `scm.GIT.SetConfig` evicts the cache.
                # This ensures we don't interleave reads and writes causing
                # the cache to set and unset consecutively.
                config_updates = []

                blame_ignore_revs_cfg = scm.GIT.GetConfig(
                    checkout_path, 'blame.ignorerevsfile')

                blame_ignore_revs_cfg_set = \
                    blame_ignore_revs_cfg == \
                    git_common.GIT_BLAME_IGNORE_REV_FILE

                blame_ignore_revs_exists = os.path.isfile(
                    os.path.join(checkout_path,
                                 git_common.GIT_BLAME_IGNORE_REV_FILE))

                if not blame_ignore_revs_cfg_set and blame_ignore_revs_exists:
                    config_updates.append(
                        ('blame.ignoreRevsFile',
                         git_common.GIT_BLAME_IGNORE_REV_FILE))
                elif blame_ignore_revs_cfg_set and not blame_ignore_revs_exists:
                    # Some repos may have incorrect config set, unset this
                    # value. Moreover, some repositories may decide to remove
                    # git_common.GIT_BLAME_IGNORE_REV_FILE, which would break
                    # blame without this check.
                    # See https://crbug.com/368562244 for more details.
                    config_updates.append(('blame.ignoreRevsFile', None))

                ignore_submodules = scm.GIT.GetConfig(checkout_path,
                                                      'diff.ignoresubmodules',
                                                      None, 'local')

                if not ignore_submodules:
                    config_updates.append(('diff.ignoreSubmodules', 'dirty'))
                elif ignore_submodules != 'dirty':
                    warning_message = (
                        "diff.ignoreSubmodules is not set to 'dirty' "
                        "for this repository.\n"
                        "This may cause unexpected behavior with submodules; "
                        "see //docs/git_submodules.md\n"
                        "Consider setting the config:\n"
                        "\tgit config diff.ignoreSubmodules dirty\n"
                        "or disable this warning by setting the "
                        "GCLIENT_SUPPRESS_SUBMODULE_WARNING\n"
                        "environment variable to 1.")
                    if os.environ.get(
                            'GCLIENT_SUPPRESS_SUBMODULE_WARNING') != '1':
                        gclient_utils.AddWarning(warning_message)


                if scm.GIT.GetConfig(checkout_path,
                                     'fetch.recursesubmodules') != 'off':
                    config_updates.append(('fetch.recurseSubmodules', 'off'))

                if scm.GIT.GetConfig(checkout_path,
                                     'push.recursesubmodules') != 'off':
                    # The default is off, but if user sets submodules.recurse to
                    # on, this becomes on too. We never want to push submodules
                    # for gclient managed repositories. Push, even if a no-op,
                    # will increase `git cl upload` latency.
                    config_updates.append(('push.recurseSubmodules', 'off'))

                for update in config_updates:
                    scm.GIT.SetConfig(checkout_path,
                                      update[0],
                                      update[1],
                                      modify_all=True)

            return return_val

        return wrapper

    @set_config
    def update(self, options, args, file_list):
        """Runs git to update or transparently checkout the working copy.

    All updated files will be appended to file_list.

    Raises:
      Error: if can't get URL for relative path.
    """
        if args:
            raise gclient_utils.Error("Unsupported argument(s): %s" %
                                      ",".join(args))

        url, deps_revision = gclient_utils.SplitUrlRevision(self.url)
        revision = deps_revision
        managed = True
        if options.revision:
            # Override the revision number.
            revision = str(options.revision)
        if revision == 'unmanaged':
            # Check again for a revision in case an initial ref was specified
            # in the url, for example bla.git@refs/heads/custombranch
            revision = deps_revision
            managed = False
        if not revision:
            # If a dependency is not pinned, track the default remote branch.
            revision = scm.GIT.GetRemoteHeadRef(self.checkout_path, self.url,
                                                self.remote)
        if revision.startswith('origin/'):
            revision = 'refs/remotes/' + revision

        if managed and platform.system() == 'Windows':
            self._DisableHooks()

        printed_path = False
        verbose = []
        if options.verbose:
            self.Print('_____ %s at %s' % (self.relpath, revision),
                       timestamp=False)
            verbose = ['--verbose']
            printed_path = True

        revision_ref = revision
        if ':' in revision:
            revision_ref, _, revision = revision.partition(':')

        if revision_ref.startswith('refs/branch-heads'):
            options.with_branch_heads = True

        mirror = self._GetMirror(url, options, revision, revision_ref)
        if mirror:
            url = mirror.mirror_path

        remote_ref = scm.GIT.RefToRemoteRef(revision, self.remote)
        if remote_ref:
            # Rewrite remote refs to their local equivalents.
            revision = ''.join(remote_ref)
            rev_type = "branch"
        elif revision.startswith('refs/heads/'):
            # Local branch? We probably don't want to support, since DEPS should
            # always specify branches as they are in the upstream repo.
            rev_type = "branch"
        else:
            # hash is also a tag, only make a distinction at checkout.
            # Any ref (e.g. /refs/changes/*) not a branch has no difference from
            # a hash.
            rev_type = "hash"

        # If we are going to introduce a new project, there is a possibility
        # that we are syncing back to a state where the project was originally a
        # sub-project rolled by DEPS (realistic case: crossing the Blink merge
        # point syncing backwards, when Blink was a DEPS entry and not part of
        # src.git). In such case, we might have a backup of the former .git
        # folder, which can be used to avoid re-fetching the entire repo again
        # (useful for bisects).
        backup_dir = self.GetGitBackupDirPath()
        target_dir = os.path.join(self.checkout_path, '.git')
        if os.path.exists(backup_dir) and not os.path.exists(target_dir):
            gclient_utils.safe_makedirs(self.checkout_path)
            os.rename(backup_dir, target_dir)
            # Reset to a clean state
            self._Scrub('HEAD', options)

        if (not os.path.exists(self.checkout_path) or
            (os.path.isdir(self.checkout_path)
             and not os.path.exists(os.path.join(self.checkout_path, '.git')))):
            if mirror:
                self._UpdateMirrorIfNotContains(mirror, options, rev_type,
                                                revision)
            try:
                self.current_revision = self._Clone(revision, url, options)
            except subprocess2.CalledProcessError as e:
                logging.warning('Clone failed due to: %s', e)
                self._DeleteOrMove(options.force)
                self.current_revision = self._Clone(revision, url, options)
            if file_list is not None:
                files = self._Capture(
                    ['-c', 'core.quotePath=false', 'ls-files']).splitlines()
                file_list.extend(
                    [os.path.join(self.checkout_path, f) for f in files])
            if mirror:
                self._Capture(
                    ['remote', 'set-url', '--push', 'origin', mirror.url])
            if not verbose:
                # Make the output a little prettier. It's nice to have some
                # whitespace between projects when cloning.
                self.Print('')
            return self._Capture(['rev-parse', '--verify', 'HEAD'])

        if mirror:
            self._Capture(['remote', 'set-url', '--push', 'origin', mirror.url])

        if not managed:
            self._SetFetchConfig(options)
            self.Print('________ unmanaged solution; skipping %s' %
                       self.relpath)
            return self._Capture(['rev-parse', '--verify', 'HEAD'])

        # Special case for rev_type = hash. If we use submodules, we can check
        # information already.
        if rev_type == 'hash':
            if self.current_revision == revision:
                if verbose:
                    self.Print('Using submodule information to skip check')
                if options.reset or options.force:
                    self._Scrub('HEAD', options)

                return revision

        self._maybe_break_locks(options)

        if mirror:
            self._UpdateMirrorIfNotContains(mirror, options, rev_type, revision)

        # See if the url has changed (the unittests use git://foo for the url,
        # let that through).
        current_url = scm.GIT.GetConfig(self.checkout_path,
                                        f'remote.{self.remote}.url',
                                        default='')
        return_early = False
        # TODO(maruel): Delete url != 'git://foo' since it's just to make the
        # unit test pass. (and update the comment above)
        strp_url = url[:-4] if url.endswith('.git') else url
        strp_current_url = current_url[:-4] if current_url.endswith(
            '.git') else current_url
        if (strp_current_url.rstrip('/') != strp_url.rstrip('/')
                and url != 'git://foo'):
            self.Print('_____ switching %s from %s to new upstream %s' %
                       (self.relpath, current_url, url))
            if not (options.force or options.reset):
                # Make sure it's clean
                self._CheckClean(revision)
            # Switch over to the new upstream
            self._Run(['remote', 'set-url', self.remote, url], options)
            if mirror:
                if git_cache.Mirror.CacheDirToUrl(current_url.rstrip(
                        '/')) == git_cache.Mirror.CacheDirToUrl(
                            url.rstrip('/')):
                    # Reset alternates when the cache dir is updated.
                    with open(
                            os.path.join(self.checkout_path, '.git', 'objects',
                                         'info', 'alternates'), 'w') as fh:
                        fh.write(os.path.join(url, 'objects'))
                else:
                    # Because we use Git alternatives, our existing repository
                    # is not self-contained. It's possible that new git
                    # alternative doesn't have all necessary objects that the
                    # current repository needs. Instead of blindly hoping that
                    # new alternative contains all necessary objects, keep the
                    # old alternative and just append a new one on top of it.
                    with open(
                            os.path.join(self.checkout_path, '.git', 'objects',
                                         'info', 'alternates'), 'a') as fh:
                        fh.write("\n" + os.path.join(url, 'objects'))
            current_revision = self._EnsureValidHeadObjectOrCheckout(
                revision, options, url)
            self._FetchAndReset(revision, file_list, options)

            return_early = True
        else:
            current_revision = self._EnsureValidHeadObjectOrCheckout(
                revision, options, url)

        if return_early:
            return current_revision or self._Capture(
                ['rev-parse', '--verify', 'HEAD'])

        cur_branch = self._GetCurrentBranch()

        # Cases:
        # 0) HEAD is detached. Probably from our initial clone.
        #   - make sure HEAD is contained by a named ref, then update.
        # Cases 1-4. HEAD is a branch.
        # 1) current branch is not tracking a remote branch
        #   - try to rebase onto the new hash or branch
        # 2) current branch is tracking a remote branch with local committed
        #    changes, but the DEPS file switched to point to a hash
        #   - rebase those changes on top of the hash
        # 3) current branch is tracking a remote branch w/or w/out changes, and
        #    no DEPS switch
        # - see if we can FF, if not, prompt the user for rebase, merge, or stop
        # 4) current branch is tracking a remote branch, but DEPS switches to a
        # different remote branch, and a) current branch has no local changes,
        # and --force: - checkout new branch b) current branch has local
        # changes, and --force and --reset: - checkout new branch c) otherwise
        # exit

        # GetUpstreamBranch returns something like 'refs/remotes/origin/main'
        # for a tracking branch or 'main' if not a tracking branch (it's based
        # on a specific rev/hash) or it returns None if it couldn't find an
        # upstream
        if cur_branch is None:
            upstream_branch = None
            current_type = "detached"
            logging.debug("Detached HEAD")
        else:
            upstream_branch = scm.GIT.GetUpstreamBranch(self.checkout_path)
            if not upstream_branch or not upstream_branch.startswith(
                    'refs/remotes'):
                current_type = "hash"
                logging.debug(
                    "Current branch is not tracking an upstream (remote)"
                    " branch.")
            elif upstream_branch.startswith('refs/remotes'):
                current_type = "branch"
            else:
                raise gclient_utils.Error('Invalid Upstream: %s' %
                                          upstream_branch)

        self._SetFetchConfig(options)

        # Fetch upstream if we don't already have |revision|.
        if not scm.GIT.IsValidRevision(
                self.checkout_path, revision, sha_only=True):
            self._Fetch(options, prune=options.force)

            if not scm.GIT.IsValidRevision(
                    self.checkout_path, revision, sha_only=True):
                # Update the remotes first so we have all the refs.
                remote_output = scm.GIT.Capture(['remote'] + verbose +
                                                ['update'],
                                                cwd=self.checkout_path)
                if verbose:
                    self.Print(remote_output)

        revision = self._AutoFetchRef(options, revision)

        # This is a big hammer, debatable if it should even be here...
        if options.force or options.reset:
            target = 'HEAD'
            if options.upstream and upstream_branch:
                target = upstream_branch
            self._Scrub(target, options)

        if current_type == 'detached':
            # case 0
            # We just did a Scrub, this is as clean as it's going to get. In
            # particular if HEAD is a commit that contains two versions of the
            # same file on a case-insensitive filesystem (e.g. 'a' and 'A'),
            # there's no way to actually "Clean" the checkout; that commit is
            # uncheckoutable on this system. The best we can do is carry forward
            # to the checkout step.
            if not (options.force or options.reset):
                self._CheckClean(revision)
            self._CheckDetachedHead(revision, options)

            if not current_revision:
                current_revision = self._Capture(
                    ['rev-list', '-n', '1', 'HEAD'])
            if current_revision == revision:
                self.Print('Up-to-date; skipping checkout.')
            else:
                # 'git checkout' may need to overwrite existing untracked files.
                # Allow it only when nuclear options are enabled.
                self._Checkout(
                    options,
                    revision,
                    force=(options.force and options.delete_unversioned_trees),
                    quiet=True,
                )
            if not printed_path:
                self.Print('_____ %s at %s' % (self.relpath, revision),
                           timestamp=False)
        elif current_type == 'hash':
            # case 1
            # Can't find a merge-base since we don't know our upstream. That
            # makes this command VERY likely to produce a rebase failure. For
            # now we assume origin is our upstream since that's what the old
            # behavior was.
            upstream_branch = self.remote
            if options.revision or deps_revision:
                upstream_branch = revision
            self._AttemptRebase(upstream_branch,
                                file_list,
                                options,
                                printed_path=printed_path,
                                merge=options.merge)
            printed_path = True
        elif rev_type == 'hash':
            # case 2
            self._AttemptRebase(upstream_branch,
                                file_list,
                                options,
                                newbase=revision,
                                printed_path=printed_path,
                                merge=options.merge)
            printed_path = True
        elif remote_ref and ''.join(remote_ref) != upstream_branch:
            # case 4
            new_base = ''.join(remote_ref)
            if not printed_path:
                self.Print('_____ %s at %s' % (self.relpath, revision),
                           timestamp=False)
            switch_error = (
                "Could not switch upstream branch from %s to %s\n" %
                (upstream_branch, new_base) +
                "Please use --force or merge or rebase manually:\n" +
                "cd %s; git rebase %s\n" % (self.checkout_path, new_base) +
                "OR git checkout -b <some new branch> %s" % new_base)
            force_switch = False
            if options.force:
                try:
                    self._CheckClean(revision)
                    # case 4a
                    force_switch = True
                except gclient_utils.Error as e:
                    if options.reset:
                        # case 4b
                        force_switch = True
                    else:
                        switch_error = '%s\n%s' % (str(e), switch_error)
            if force_switch:
                self.Print("Switching upstream branch from %s to %s" %
                           (upstream_branch, new_base))
                switch_branch = 'gclient_' + remote_ref[1]
                self._Capture(['branch', '-f', switch_branch, new_base])
                self._Checkout(options, switch_branch, force=True, quiet=True)
            else:
                # case 4c
                raise gclient_utils.Error(switch_error)
        else:
            # case 3 - the default case
            rebase_files = self._GetDiffFilenames(upstream_branch)
            if verbose:
                self.Print('Trying fast-forward merge to branch : %s' %
                           upstream_branch)
            try:
                merge_args = ['merge']
                if options.merge:
                    merge_args.append('--ff')
                else:
                    merge_args.append('--ff-only')
                merge_args.append(upstream_branch)
                merge_output = self._Capture(merge_args)
            except subprocess2.CalledProcessError as e:
                rebase_files = []
                if re.search(b'fatal: Not possible to fast-forward, aborting.',
                             e.stderr):
                    if not printed_path:
                        self.Print('_____ %s at %s' % (self.relpath, revision),
                                   timestamp=False)
                        printed_path = True
                    while True:
                        if not options.auto_rebase:
                            try:
                                action = self._AskForData(
                                    'Cannot %s, attempt to rebase? '
                                    '(y)es / (q)uit / (s)kip : ' %
                                    ('merge' if options.merge else
                                     'fast-forward merge'), options)
                            except ValueError:
                                raise gclient_utils.Error('Invalid Character')
                        if options.auto_rebase or re.match(
                                r'yes|y', action, re.I):
                            self._AttemptRebase(upstream_branch,
                                                rebase_files,
                                                options,
                                                printed_path=printed_path,
                                                merge=False)
                            printed_path = True
                            break

                        if re.match(r'quit|q', action, re.I):
                            raise gclient_utils.Error(
                                "Can't fast-forward, please merge or "
                                "rebase manually.\n"
                                "cd %s && git " % self.checkout_path +
                                "rebase %s" % upstream_branch)

                        if re.match(r'skip|s', action, re.I):
                            self.Print('Skipping %s' % self.relpath)
                            return

                        self.Print('Input not recognized')
                elif re.match(
                        b"error: Your local changes to '.*' would be "
                        b"overwritten by merge.  Aborting.\nPlease, commit your "
                        b"changes or stash them before you can merge.\n",
                        e.stderr):
                    if not printed_path:
                        self.Print('_____ %s at %s' % (self.relpath, revision),
                                   timestamp=False)
                        printed_path = True
                    raise gclient_utils.Error(e.stderr.decode('utf-8'))
                else:
                    # Some other problem happened with the merge
                    logging.error("Error during fast-forward merge in %s!" %
                                  self.relpath)
                    self.Print(e.stderr.decode('utf-8'))
                    raise
            else:
                # Fast-forward merge was successful
                if not re.match('Already up-to-date.', merge_output) or verbose:
                    if not printed_path:
                        self.Print('_____ %s at %s' % (self.relpath, revision),
                                   timestamp=False)
                        printed_path = True
                    self.Print(merge_output.strip())
                    if not verbose:
                        # Make the output a little prettier. It's nice to have
                        # some whitespace between projects when syncing.
                        self.Print('')

            if file_list is not None:
                file_list.extend(
                    [os.path.join(self.checkout_path, f) for f in rebase_files])

        # If the rebase generated a conflict, abort and ask user to fix
        if self._IsRebasing():
            raise gclient_utils.Error(
                '\n____ %s at %s\n'
                '\nConflict while rebasing this branch.\n'
                'Fix the conflict and run gclient again.\n'
                'See man git-rebase for details.\n' % (self.relpath, revision))

        # If --reset and --delete_unversioned_trees are specified, remove any
        # untracked directories.
        if options.reset and options.delete_unversioned_trees:
            # GIT.CaptureStatus() uses 'dit diff' to compare to a specific SHA1
            # (the merge-base by default), so doesn't include untracked files.
            # So we use 'git ls-files --directory --others --exclude-standard'
            # here directly.
            paths = scm.GIT.Capture([
                '-c', 'core.quotePath=false', 'ls-files', '--directory',
                '--others', '--exclude-standard'
            ], self.checkout_path)
            for path in (p for p in paths.splitlines() if p.endswith('/')):
                full_path = os.path.join(self.checkout_path, path)
                if not os.path.islink(full_path):
                    self.Print('_____ removing unversioned directory %s' % path)
                    gclient_utils.rmtree(full_path)

        if not current_revision:
            current_revision = self._Capture(['rev-parse', '--verify', 'HEAD'])
        if verbose:
            self.Print(f'Checked out revision {current_revision}',
                       timestamp=False)
        return current_revision

    def revert(self, options, _args, file_list):
        """Reverts local modifications.

    All reverted files will be appended to file_list.
    """
        if not os.path.isdir(self.checkout_path):
            # revert won't work if the directory doesn't exist. It needs to
            # checkout instead.
            self.Print('_____ %s is missing, syncing instead' % self.relpath)
            # Don't reuse the args.
            return self.update(options, [], file_list)

        default_rev = "refs/heads/main"
        if options.upstream:
            if self._GetCurrentBranch():
                upstream_branch = scm.GIT.GetUpstreamBranch(self.checkout_path)
                default_rev = upstream_branch or default_rev
        _, deps_revision = gclient_utils.SplitUrlRevision(self.url)
        if not deps_revision:
            deps_revision = default_rev
        if deps_revision.startswith('refs/heads/'):
            deps_revision = deps_revision.replace('refs/heads/',
                                                  self.remote + '/')
        try:
            deps_revision = self.GetUsableRev(deps_revision, options)
        except NoUsableRevError as e:
            # If the DEPS entry's url and hash changed, try to update the
            # origin. See also http://crbug.com/520067.
            logging.warning(
                "Couldn't find usable revision, will retrying to update instead: %s",
                str(e))
            return self.update(options, [], file_list)

        if file_list is not None:
            files = self._GetDiffFilenames(deps_revision)

        self._Scrub(deps_revision, options)
        self._Run(['clean', '-f', '-d'], options)

        if file_list is not None:
            file_list.extend(
                [os.path.join(self.checkout_path, f) for f in files])

    def revinfo(self, _options, _args, _file_list):
        """Returns revision"""
        return self._Capture(['rev-parse', 'HEAD'])

    def runhooks(self, options, args, file_list):
        self.status(options, args, file_list)

    def status(self, options, _args, file_list):
        """Display status information."""
        if not os.path.isdir(self.checkout_path):
            self.Print('________ couldn\'t run status in %s:\n'
                       'The directory does not exist.' % self.checkout_path)
        else:
            merge_base = []
            if self.url:
                _, base_rev = gclient_utils.SplitUrlRevision(self.url)
                if base_rev:
                    if base_rev.startswith('refs/'):
                        base_rev = self._ref_to_remote_ref(base_rev)
                    merge_base = [base_rev]
            self._Run(['-c', 'core.quotePath=false', 'diff', '--name-status'] +
                      merge_base,
                      options,
                      always_show_header=options.verbose)
            if file_list is not None:
                files = self._GetDiffFilenames(
                    merge_base[0] if merge_base else None)
                file_list.extend(
                    [os.path.join(self.checkout_path, f) for f in files])

    def GetUsableRev(self, rev, options):
        """Finds a useful revision for this repository."""
        sha1 = None
        if not os.path.isdir(self.checkout_path):
            raise NoUsableRevError(
                'This is not a git repo, so we cannot get a usable rev.')

        if scm.GIT.IsValidRevision(cwd=self.checkout_path, rev=rev):
            sha1 = rev
        else:
            # May exist in origin, but we don't have it yet, so fetch and look
            # again.
            self._Fetch(options)
            if scm.GIT.IsValidRevision(cwd=self.checkout_path, rev=rev):
                sha1 = rev

        if not sha1:
            raise NoUsableRevError(
                'Hash %s does not appear to be a valid hash in this repo.' %
                rev)

        return sha1

    def GetGitBackupDirPath(self):
        """Returns the path where the .git folder for the current project can be
    staged/restored. Use case: subproject moved from DEPS <-> outer project."""
        return os.path.join(self._root_dir,
                            'old_' + self.relpath.replace(os.sep, '_')) + '.git'

    def _GetMirror(self, url, options, revision=None, revision_ref=None):
        """Get a git_cache.Mirror object for the argument url."""
        if not self.cache_dir:
            return None
        mirror_kwargs = {
            'print_func': self.filter,
            'refs': [],
            'commits': [],
        }
        if hasattr(options, 'with_branch_heads') and options.with_branch_heads:
            mirror_kwargs['refs'].append('refs/branch-heads/*')
        elif revision_ref and revision_ref.startswith('refs/branch-heads/'):
            mirror_kwargs['refs'].append(revision_ref)
        if hasattr(options, 'with_tags') and options.with_tags:
            mirror_kwargs['refs'].append('refs/tags/*')
        elif revision_ref and revision_ref.startswith('refs/tags/'):
            mirror_kwargs['refs'].append(revision_ref)
        if revision and not revision.startswith('refs/'):
            mirror_kwargs['commits'].append(revision)
        return git_cache.Mirror(url, **mirror_kwargs)

    def _UpdateMirrorIfNotContains(self, mirror, options, rev_type, revision):
        """Update a git mirror by fetching the latest commits from the remote,
    unless mirror already contains revision whose type is sha1 hash.
    """
        if rev_type == 'hash' and mirror.contains_revision(revision):
            if options.verbose:
                self.Print('skipping mirror update, it has rev=%s already' %
                           revision,
                           timestamp=False)
            return

        if getattr(options, 'shallow', False):
            depth = 10000
        else:
            depth = None
        mirror.populate(verbose=options.verbose,
                        bootstrap=not getattr(options, 'no_bootstrap', False),
                        depth=depth,
                        lock_timeout=getattr(options, 'lock_timeout', 0))

    def _Clone(self, revision, url, options):
        """Clone a git repository from the given URL.

        Once we've cloned the repo, we checkout a working branch if the
        specified revision is a branch head. If it is a tag or a specific
        commit, then we leave HEAD detached as it makes future updates simpler
        -- in this case the user should first create a new branch or switch to
        an existing branch before making changes in the repo."""

        if self.print_outbuf:
            print_stdout = True
            filter_fn = None
        else:
            print_stdout = False
            filter_fn = self.filter

        if not options.verbose:
            # git clone doesn't seem to insert a newline properly before
            # printing to stdout
            self.Print('')

        # If the parent directory does not exist, Git clone on Windows will not
        # create it, so we need to do it manually.
        parent_dir = os.path.dirname(self.checkout_path)
        gclient_utils.safe_makedirs(parent_dir)

        # Set up Git authentication configuration that is needed to clone/fetch the repo.
        if newauth.Enabled():
            # We need the host from the URL to determine auth settings.
            # The url parameter might have been re-written to a local
            # cache directory, so we need self.url, which contains the
            # original remote URL.
            git_auth.ConfigureGlobal('/', self.url)

        if hasattr(options, 'no_history') and options.no_history:
            self._Run(['init', self.checkout_path], options, cwd=self._root_dir)
            self._Run(['remote', 'add', 'origin', url], options)
            revision = self._AutoFetchRef(options, revision, depth=1)
            remote_ref = scm.GIT.RefToRemoteRef(revision, self.remote)
            self._Checkout(options, ''.join(remote_ref or revision), quiet=True)
        else:
            cfg = gclient_utils.DefaultIndexPackConfig(url)
            clone_cmd = cfg + ['clone', '--no-checkout', '--progress']
            if self.cache_dir:
                clone_cmd.append('--shared')
            if options.verbose:
                clone_cmd.append('--verbose')
            clone_cmd.append(url)
            tmp_dir = tempfile.mkdtemp(prefix='_gclient_%s_' %
                                       os.path.basename(self.checkout_path),
                                       dir=parent_dir)
            clone_cmd.append(tmp_dir)

            try:
                self._Run(clone_cmd,
                          options,
                          cwd=self._root_dir,
                          retry=True,
                          print_stdout=print_stdout,
                          filter_fn=filter_fn)
                logging.debug(
                    'Cloned into temporary dir, moving to checkout_path')
                gclient_utils.safe_makedirs(self.checkout_path)
                gclient_utils.safe_rename(
                    os.path.join(tmp_dir, '.git'),
                    os.path.join(self.checkout_path, '.git'))
            except:
                traceback.print_exc(file=self.out_fh)
                raise
            finally:
                if os.listdir(tmp_dir):
                    self.Print('_____ removing non-empty tmp dir %s' % tmp_dir)
                gclient_utils.rmtree(tmp_dir)

            self._SetFetchConfig(options)
            self._Fetch(options, prune=options.force)
            revision = self._AutoFetchRef(options, revision)
            remote_ref = scm.GIT.RefToRemoteRef(revision, self.remote)
            self._Checkout(options, ''.join(remote_ref or revision), quiet=True)

        if self._GetCurrentBranch() is None:
            # Squelch git's very verbose detached HEAD warning and use our own
            self.Print((
                'Checked out %s to a detached HEAD. Before making any commits\n'
                'in this repo, you should use \'git checkout <branch>\' to switch \n'
                'to an existing branch or use \'git checkout %s -b <branch>\' to\n'
                'create a new branch for your work.') % (revision, self.remote))
        return revision

    def _AskForData(self, prompt, options):
        if options.jobs > 1:
            self.Print(prompt)
            raise gclient_utils.Error("Background task requires input. Rerun "
                                      "gclient with --jobs=1 so that\n"
                                      "interaction is possible.")
        return gclient_utils.AskForData(prompt)

    def _AttemptRebase(self,
                       upstream,
                       files,
                       options,
                       newbase=None,
                       branch=None,
                       printed_path=False,
                       merge=False):
        """Attempt to rebase onto either upstream or, if specified, newbase."""
        if files is not None:
            files.extend(self._GetDiffFilenames(upstream))
        revision = upstream
        if newbase:
            revision = newbase
        action = 'merge' if merge else 'rebase'
        if not printed_path:
            self.Print('_____ %s : Attempting %s onto %s...' %
                       (self.relpath, action, revision))
            printed_path = True
        else:
            self.Print('Attempting %s onto %s...' % (action, revision))

        if merge:
            merge_output = self._Capture(['merge', revision])
            if options.verbose:
                self.Print(merge_output)
            return

        # Build the rebase command here using the args
        # git rebase [options] [--onto <newbase>] <upstream> [<branch>]
        rebase_cmd = ['rebase']
        if options.verbose:
            rebase_cmd.append('--verbose')
        if newbase:
            rebase_cmd.extend(['--onto', newbase])
        rebase_cmd.append(upstream)
        if branch:
            rebase_cmd.append(branch)

        try:
            rebase_output = scm.GIT.Capture(rebase_cmd, cwd=self.checkout_path)
        except subprocess2.CalledProcessError as e:
            if (re.match(
                    br'cannot rebase: you have unstaged changes', e.stderr
            ) or re.match(
                    br'cannot rebase: your index contains uncommitted changes',
                    e.stderr)):
                while True:
                    rebase_action = self._AskForData(
                        'Cannot rebase because of unstaged changes.\n'
                        '\'git reset --hard HEAD\' ?\n'
                        'WARNING: destroys any uncommitted work in your current branch!'
                        ' (y)es / (q)uit / (s)how : ', options)
                    if re.match(r'yes|y', rebase_action, re.I):
                        self._Scrub('HEAD', options)
                        # Should this be recursive?
                        rebase_output = scm.GIT.Capture(rebase_cmd,
                                                        cwd=self.checkout_path)
                        break

                    if re.match(r'quit|q', rebase_action, re.I):
                        raise gclient_utils.Error(
                            "Please merge or rebase manually\n"
                            "cd %s && git " % self.checkout_path +
                            "%s" % ' '.join(rebase_cmd))

                    if re.match(r'show|s', rebase_action, re.I):
                        self.Print('%s' % e.stderr.decode('utf-8').strip())
                        continue

                    gclient_utils.Error("Input not recognized")
                    continue
            elif re.search(br'^CONFLICT', e.stdout, re.M):
                raise gclient_utils.Error(
                    "Conflict while rebasing this branch.\n"
                    "Fix the conflict and run gclient again.\n"
                    "See 'man git-rebase' for details.\n")
            else:
                self.Print(e.stdout.decode('utf-8').strip())
                self.Print('Rebase produced error output:\n%s' %
                           e.stderr.decode('utf-8').strip())
                raise gclient_utils.Error(
                    "Unrecognized error, please merge or rebase "
                    "manually.\ncd %s && git " % self.checkout_path +
                    "%s" % ' '.join(rebase_cmd))

        self.Print(rebase_output.strip())
        if not options.verbose:
            # Make the output a little prettier. It's nice to have some
            # whitespace between projects when syncing.
            self.Print('')

    def _EnsureValidHeadObjectOrCheckout(self, revision, options, url):
        # Special case handling if all 3 conditions are met:
        # * the mirros have recently changed, but deps destination remains same,
        # * the git histories of mirrors are conflicting. * git cache is used
        # This manifests itself in current checkout having invalid HEAD commit
        # on most git operations. Since git cache is used, just deleted the .git
        # folder, and re-create it by cloning.
        try:
            return self._Capture(['rev-list', '-n', '1', 'HEAD'])
        except subprocess2.CalledProcessError as e:
            if (b'fatal: bad object HEAD' in e.stderr and self.cache_dir
                    and self.cache_dir in url):
                self.Print(
                    ('Likely due to DEPS change with git cache_dir, '
                     'the current commit points to no longer existing object.\n'
                     '%s' % e))
                self._DeleteOrMove(options.force)
                return self._Clone(revision, url, options)
            raise

    def _IsRebasing(self):
        # Check for any of REBASE-i/REBASE-m/REBASE/AM. Unfortunately git
        # doesn't have a plumbing command to determine whether a rebase is in
        # progress, so for now emualate (more-or-less) git-rebase.sh /
        # git-completion.bash
        g = os.path.join(self.checkout_path, '.git')
        return (os.path.isdir(os.path.join(g, "rebase-merge"))
                or os.path.isdir(os.path.join(g, "rebase-apply")))

    def _CheckClean(self, revision):
        lockfile = os.path.join(self.checkout_path, ".git", "index.lock")
        if os.path.exists(lockfile):
            raise gclient_utils.Error(
                '\n____ %s at %s\n'
                '\tYour repo is locked, possibly due to a concurrent git process.\n'
                '\tIf no git executable is running, then clean up %r and try again.\n'
                % (self.relpath, revision, lockfile))

        # Ensure that the tree is clean.
        if scm.GIT.Capture([
                'status', '--porcelain', '--untracked-files=no',
                '--ignore-submodules'
        ],
                           cwd=self.checkout_path):
            raise gclient_utils.Error(
                '\n____ %s at %s\n'
                '\tYou have uncommitted changes.\n'
                '\tcd into %s, run git status to see changes,\n'
                '\tand commit, stash, or reset.\n' %
                (self.relpath, revision, self.relpath))

    def _CheckDetachedHead(self, revision, _options):
        # HEAD is detached. Make sure it is safe to move away from (i.e., it is
        # reference by a commit). If not, error out -- most likely a rebase is
        # in progress, try to detect so we can give a better error.
        try:
            scm.GIT.Capture(['name-rev', '--no-undefined', 'HEAD'],
                            cwd=self.checkout_path)
        except subprocess2.CalledProcessError:
            # Commit is not contained by any rev. See if the user is rebasing:
            if self._IsRebasing():
                # Punt to the user
                raise gclient_utils.Error(
                    '\n____ %s at %s\n'
                    '\tAlready in a conflict, i.e. (no branch).\n'
                    '\tFix the conflict and run gclient again.\n'
                    '\tOr to abort run:\n\t\tgit-rebase --abort\n'
                    '\tSee man git-rebase for details.\n' %
                    (self.relpath, revision))
            # Let's just save off the commit so we can proceed.
            name = ('saved-by-gclient-' +
                    self._Capture(['rev-parse', '--short', 'HEAD']))
            self._Capture(['branch', '-f', name])
            self.Print(
                '_____ found an unreferenced commit and saved it as \'%s\'' %
                name)

    def _GetCurrentBranch(self):
        # Returns name of current branch or None for detached HEAD
        branch = self._Capture(['rev-parse', '--abbrev-ref=strict', 'HEAD'])
        if branch == 'HEAD':
            return None
        return branch

    def _Capture(self, args, **kwargs):
        set_git_dir = 'cwd' not in kwargs
        kwargs.setdefault('cwd', self.checkout_path)
        kwargs.setdefault('stderr', subprocess2.PIPE)
        strip = kwargs.pop('strip', True)
        env = scm.GIT.ApplyEnvVars(kwargs)
        # If an explicit cwd isn't set, then default to the .git/ subdir so we
        # get stricter behavior.  This can be useful in cases of slight
        # corruption -- we don't accidentally go corrupting parent git checks
        # too.  See https://crbug.com/1000825 for an example.
        if set_git_dir:
            env.setdefault(
                'GIT_DIR',
                os.path.abspath(os.path.join(self.checkout_path, '.git')))
        kwargs.setdefault('env', env)
        ret = git_common.run(*args, **kwargs)
        if strip:
            ret = ret.strip()
        self.Print('Finished running: %s %s' % ('git', ' '.join(args)))
        return ret

    def _Checkout(self, options, ref, force=False, quiet=None):
        """Performs a 'git-checkout' operation.

    Args:
      options: The configured option set
      ref: (str) The branch/commit to checkout
      quiet: (bool/None) Whether or not the checkout should pass '--quiet'; if
          'None', the behavior is inferred from 'options.verbose'.
    Returns: (str) The output of the checkout operation
    """
        if quiet is None:
            quiet = (not options.verbose)
        checkout_args = ['checkout']
        if force:
            checkout_args.append('--force')
        if quiet:
            checkout_args.append('--quiet')
        checkout_args.append(ref)
        return self._Capture(checkout_args)

    def _Fetch(self,
               options,
               remote=None,
               prune=False,
               quiet=False,
               refspec=None,
               depth=None):
        cfg = gclient_utils.DefaultIndexPackConfig(self.url)
        # When updating, the ref is modified to be a remote ref .
        # (e.g. refs/heads/NAME becomes refs/remotes/REMOTE/NAME).
        # Try to reverse that mapping.
        original_ref = scm.GIT.RemoteRefToRef(refspec, self.remote)
        if original_ref:
            refspec = original_ref + ':' + refspec
            # When a mirror is configured, it only fetches
            # refs/{heads,branch-heads,tags}/*.
            # If asked to fetch other refs, we must fetch those directly from
            # the repository, and not from the mirror.
            if not original_ref.startswith(
                ('refs/heads/', 'refs/branch-heads/', 'refs/tags/')):
                remote, _ = gclient_utils.SplitUrlRevision(self.url)
        fetch_cmd = cfg + [
            'fetch',
            remote or self.remote,
        ]
        if refspec:
            fetch_cmd.append(refspec)

        if prune:
            fetch_cmd.append('--prune')
        if options.verbose:
            fetch_cmd.append('--verbose')
        if not hasattr(options, 'with_tags') or not options.with_tags:
            fetch_cmd.append('--no-tags')
        elif quiet:
            fetch_cmd.append('--quiet')
        if depth:
            fetch_cmd.append('--depth=' + str(depth))
        self._Run(fetch_cmd, options, show_header=options.verbose, retry=True)

    def _SetFetchConfig(self, options):
        """Adds, and optionally fetches, "branch-heads" and "tags" refspecs
    if requested."""
        if options.force or options.reset:
            try:
                scm.GIT.SetConfig(self.checkout_path,
                                  f'remote.{self.remote}.fetch',
                                  modify_all=True)
                scm.GIT.SetConfig(
                    self.checkout_path, f'remote.{self.remote}.fetch',
                    f'+refs/heads/*:refs/remotes/{self.remote}/*')
            except subprocess2.CalledProcessError as e:
                # If exit code was 5, it means we attempted to unset a config
                # that didn't exist. Ignore it.
                if e.returncode != 5:
                    raise
        if hasattr(options, 'with_branch_heads') and options.with_branch_heads:
            scm.GIT.SetConfig(
                self.checkout_path,
                f'remote.{self.remote}.fetch',
                '+refs/branch-heads/*:refs/remotes/branch-heads/*',
                value_pattern='^\\+refs/branch-heads/\\*:.*$',
                modify_all=True)
        if hasattr(options, 'with_tags') and options.with_tags:
            scm.GIT.SetConfig(self.checkout_path,
                              f'remote.{self.remote}.fetch',
                              '+refs/tags/*:refs/tags/*',
                              value_pattern='^\\+refs/tags/\\*:.*$',
                              modify_all=True)

    def _AutoFetchRef(self, options, revision, depth=None):
        """Attempts to fetch |revision| if not available in local repo.

        Returns possibly updated revision."""
        if not scm.GIT.IsValidRevision(self.checkout_path, revision):
            self._Fetch(options, refspec=revision, depth=depth)
            revision = self._Capture(['rev-parse', 'FETCH_HEAD'])
        return revision

    def _Run(self, args, options, **kwargs):
        # Disable 'unused options' warning | pylint: disable=unused-argument
        kwargs.setdefault('cwd', self.checkout_path)
        kwargs.setdefault('filter_fn', self.filter)
        kwargs.setdefault('show_header', True)
        env = scm.GIT.ApplyEnvVars(kwargs)

        cmd = ['git'] + args
        gclient_utils.CheckCallAndFilter(cmd, env=env, **kwargs)


class CipdPackage(object):
    """A representation of a single CIPD package."""
    def __init__(self, name, version, authority_for_subdir):
        self._authority_for_subdir = authority_for_subdir
        self._name = name
        self._version = version

    @property
    def authority_for_subdir(self):
        """Whether this package has authority to act on behalf of its subdir.

    Some operations should only be performed once per subdirectory. A package
    that has authority for its subdirectory is the only package that should
    perform such operations.

    Returns:
      bool; whether this package has subdir authority.
    """
        return self._authority_for_subdir

    @property
    def name(self):
        return self._name

    @property
    def version(self):
        return self._version


class CipdRoot(object):
    """A representation of a single CIPD root."""
    def __init__(self, root_dir, service_url, log_level=None):
        self._all_packages = set()
        self._mutator_lock = threading.Lock()
        self._packages_by_subdir = collections.defaultdict(list)
        self._root_dir = root_dir
        self._service_url = service_url
        self._resolved_packages = None
        self._log_level = log_level or 'error'

    def add_package(self, subdir, package, version):
        """Adds a package to this CIPD root.

    As far as clients are concerned, this grants both root and subdir authority
    to packages arbitrarily. (The implementation grants root authority to the
    first package added and subdir authority to the first package added for that
    subdir, but clients should not depend on or expect that behavior.)

    Args:
      subdir: str; relative path to where the package should be installed from
        the cipd root directory.
      package: str; the cipd package name.
      version: str; the cipd package version.
    Returns:
      CipdPackage; the package that was created and added to this root.
    """
        with self._mutator_lock:
            cipd_package = CipdPackage(package, version,
                                       not self._packages_by_subdir[subdir])
            self._all_packages.add(cipd_package)
            self._packages_by_subdir[subdir].append(cipd_package)
            return cipd_package

    def packages(self, subdir):
        """Get the list of configured packages for the given subdir."""
        return list(self._packages_by_subdir[subdir])

    def resolved_packages(self):
        if not self._resolved_packages:
            self._resolved_packages = self.ensure_file_resolve()
        return self._resolved_packages

    def clobber(self):
        """Remove the .cipd directory.

    This is useful for forcing ensure to redownload and reinitialize all
    packages.
    """
        with self._mutator_lock:
            cipd_cache_dir = os.path.join(self.root_dir, '.cipd')
            try:
                gclient_utils.rmtree(os.path.join(cipd_cache_dir))
            except OSError:
                if os.path.exists(cipd_cache_dir):
                    raise

    def expand_package_name(self, package_name_string, **kwargs):
        """Run `cipd expand-package-name`.

    CIPD package names can be declared with placeholder variables
    such as '${platform}', this cmd will return the package name
    with the variables resolved. The resolution is based on the host
    the command is executing on.
    """

        kwargs.setdefault('stderr', subprocess2.PIPE)
        cmd = ['cipd', 'expand-package-name', package_name_string]
        ret = subprocess2.check_output(cmd, **kwargs).decode('utf-8')
        return ret.strip()

    @contextlib.contextmanager
    def _create_ensure_file(self):
        try:
            contents = '$ParanoidMode CheckPresence\n'
            # TODO(crbug/1329641): Remove once cipd packages have been updated
            # to always be created in copy mode.
            contents += '$OverrideInstallMode copy\n\n'
            for subdir, packages in sorted(self._packages_by_subdir.items()):
                contents += '@Subdir %s\n' % subdir
                for package in sorted(packages, key=lambda p: p.name):
                    contents += '%s %s\n' % (package.name, package.version)
                contents += '\n'
            ensure_file = None
            with tempfile.NamedTemporaryFile(suffix='.ensure',
                                             delete=False,
                                             mode='wb') as ensure_file:
                ensure_file.write(contents.encode('utf-8', 'replace'))
            yield ensure_file.name
        finally:
            if ensure_file is not None and os.path.exists(ensure_file.name):
                os.remove(ensure_file.name)

    def ensure(self):
        """Run `cipd ensure`."""
        with self._mutator_lock:
            with self._create_ensure_file() as ensure_file:
                cmd = [
                    'cipd',
                    'ensure',
                    '-log-level',
                    self._log_level,
                    '-root',
                    self.root_dir,
                    '-ensure-file',
                    ensure_file,
                ]
                gclient_utils.CheckCallAndFilter(cmd,
                                                 print_stdout=True,
                                                 show_header=True)

    @contextlib.contextmanager
    def _create_ensure_file_for_resolve(self):
        try:
            contents = '$ResolvedVersions %s\n' % os.devnull
            for subdir, packages in sorted(self._packages_by_subdir.items()):
                contents += '@Subdir %s\n' % subdir
                for package in sorted(packages, key=lambda p: p.name):
                    contents += '%s %s\n' % (package.name, package.version)
                contents += '\n'
            ensure_file = None
            with tempfile.NamedTemporaryFile(suffix='.ensure',
                                             delete=False,
                                             mode='wb') as ensure_file:
                ensure_file.write(contents.encode('utf-8', 'replace'))
            yield ensure_file.name
        finally:
            if ensure_file is not None and os.path.exists(ensure_file.name):
                os.remove(ensure_file.name)

    def _create_resolved_file(self):
        return tempfile.NamedTemporaryFile(suffix='.resolved',
                                           delete=False,
                                           mode='wb')

    def ensure_file_resolve(self):
        """Run `cipd ensure-file-resolve`."""
        with self._mutator_lock:
            with self._create_resolved_file() as output_file:
                with self._create_ensure_file_for_resolve() as ensure_file:
                    cmd = [
                        'cipd',
                        'ensure-file-resolve',
                        '-log-level',
                        self._log_level,
                        '-ensure-file',
                        ensure_file,
                        '-json-output',
                        output_file.name,
                    ]
                    gclient_utils.CheckCallAndFilter(cmd,
                                                     print_stdout=False,
                                                     show_header=False)
                    with open(output_file.name) as f:
                        output_json = json.load(f)
                        return output_json.get('result', {})

    def run(self, command):
        if command == 'update':
            self.ensure()
        elif command == 'revert':
            self.clobber()
            self.ensure()

    def created_package(self, package):
        """Checks whether this root created the given package.

    Args:
      package: CipdPackage; the package to check.
    Returns:
      bool; whether this root created the given package.
    """
        return package in self._all_packages

    @property
    def root_dir(self):
        return self._root_dir

    @property
    def service_url(self):
        return self._service_url


class CipdWrapper(SCMWrapper):
    """Wrapper for CIPD.

  Currently only supports chrome-infra-packages.appspot.com.
  """
    name = 'cipd'

    def __init__(self,
                 url=None,
                 root_dir=None,
                 relpath=None,
                 out_fh=None,
                 out_cb=None,
                 root=None,
                 package=None):
        super(CipdWrapper, self).__init__(url=url,
                                          root_dir=root_dir,
                                          relpath=relpath,
                                          out_fh=out_fh,
                                          out_cb=out_cb)
        assert root.created_package(package)
        self._package = package
        self._root = root

    #override
    def GetCacheMirror(self):
        return None

    #override
    def GetActualRemoteURL(self, options):
        return self._root.service_url

    #override
    def DoesRemoteURLMatch(self, options):
        del options
        return True

    def revert(self, options, args, file_list):
        """Does nothing.

    CIPD packages should be reverted at the root by running
    `CipdRoot.run('revert')`.
    """

    def diff(self, options, args, file_list):
        """CIPD has no notion of diffing."""

    def pack(self, options, args, file_list):
        """CIPD has no notion of diffing."""

    def revinfo(self, options, args, file_list):
        """Grab the instance ID."""
        try:
            tmpdir = tempfile.mkdtemp()
            # Attempt to get instance_id from the root resolved cache.
            # Resolved cache will not match on any CIPD packages with
            # variables such as ${platform}, they will fall back to
            # the slower method below.
            resolved = self._root.resolved_packages()
            if resolved:
                # CIPD uses POSIX separators across all platforms, so
                # replace any Windows separators.
                path_split = self.relpath.replace(os.sep, "/").split(":")
                if len(path_split) > 1:
                    src_path, package = path_split
                    if src_path in resolved:
                        for resolved_package in resolved[src_path]:
                            if package == resolved_package.get(
                                    'pin', {}).get('package'):
                                return resolved_package.get(
                                    'pin', {}).get('instance_id')

            describe_json_path = os.path.join(tmpdir, 'describe.json')
            cmd = [
                'cipd', 'describe', self._package.name, '-log-level', 'error',
                '-version', self._package.version, '-json-output',
                describe_json_path
            ]
            gclient_utils.CheckCallAndFilter(cmd)
            with open(describe_json_path) as f:
                describe_json = json.load(f)
            return describe_json.get('result', {}).get('pin',
                                                       {}).get('instance_id')
        finally:
            gclient_utils.rmtree(tmpdir)

    def status(self, options, args, file_list):
        pass

    def update(self, options, args, file_list):
        """Does nothing.

    CIPD packages should be updated at the root by running
    `CipdRoot.run('update')`.
    """


class GcsRoot(object):
    """Root to keep track of all GCS objects, per checkout"""

    def __init__(self, root_dir):
        self._mutator_lock = threading.Lock()
        self._root_dir = root_dir
        # Populated when the DEPS file is parsed
        # The objects here have not yet been downloaded and written into
        # the .gcs_entries file
        self._parsed_objects = {}
        # .gcs_entries keeps track of which GCS deps have already been installed
        # Maps checkout_name -> {GCS dep path -> [object_name]}
        # This file is in the same directory as .gclient
        self._gcs_entries_file = os.path.join(self._root_dir, '.gcs_entries')
        # Contents of the .gcs_entries file
        self._gcs_entries = self.read_gcs_entries()

    @property
    def root_dir(self):
        return self._root_dir

    def add_object(self, checkout_name, dep_path, object_name):
        """Records the object in the _parsed_objects variable

        This does not actually download the object"""
        with self._mutator_lock:
            if checkout_name not in self._parsed_objects:
                self._parsed_objects[checkout_name] = {}
            if dep_path not in self._parsed_objects[checkout_name]:
                self._parsed_objects[checkout_name][dep_path] = [object_name]
            else:
                self._parsed_objects[checkout_name][dep_path].append(
                    object_name)

    def read_gcs_entries(self):
        """Reads .gcs_entries file and loads the content into _gcs_entries"""
        if not os.path.exists(self._gcs_entries_file):
            return {}

        with open(self._gcs_entries_file, 'r') as f:
            content = f.read().rstrip()
            if content:
                return json.loads(content)
            return {}

    def resolve_objects(self, checkout_name):
        """Updates .gcs_entries with objects in _parsed_objects

        This should only be called after the objects have been downloaded
        and extracted."""
        with self._mutator_lock:
            object_dict = self._parsed_objects.get(checkout_name)
            if not object_dict:
                return
            self._gcs_entries[checkout_name] = object_dict
            with open(self._gcs_entries_file, 'w') as f:
                f.write(json.dumps(self._gcs_entries, indent=2))
            self._parsed_objects[checkout_name] = {}

    def clobber_deps_with_updated_objects(self, checkout_name):
        """Clobber the path if an object or GCS dependency is removed/added

        This must be called before the GCS dependencies are
        downloaded and extracted."""
        with self._mutator_lock:
            parsed_object_dict = self._parsed_objects.get(checkout_name, {})
            parsed_paths = set(parsed_object_dict.keys())

            resolved_object_dict = self._gcs_entries.get(checkout_name, {})
            resolved_paths = set(resolved_object_dict.keys())

            # If any GCS deps are added or removed entirely, clobber that path
            intersected_paths = parsed_paths.intersection(resolved_paths)

            # If any objects within a GCS dep are added/removed, clobber its
            # extracted contents and relevant gcs dotfiles
            for path in intersected_paths:
                resolved_objects = resolved_object_dict[path]
                parsed_objects = parsed_object_dict[path]

                full_path = os.path.join(self.root_dir, path)
                if (len(resolved_objects) != len(parsed_objects)
                        and os.path.exists(full_path)):
                    self.clobber_tar_content_names(full_path)
                    self.clobber_hash_files(full_path)
                    self.clobber_migration_files(full_path)

    def clobber_tar_content_names(self, entry_directory):
        """Delete paths written in .*_content_names files"""
        content_names_files = glob.glob(
            os.path.join(entry_directory, '.*_content_names'))
        for file in content_names_files:
            with open(file, 'r') as f:
                names = json.loads(f.read().strip())
                for name in names:
                    name_path = os.path.join(entry_directory, name)
                    if os.path.isdir(
                            name_path) or not os.path.exists(name_path):
                        continue
                    os.remove(os.path.join(entry_directory, name))
            os.remove(file)

    def clobber_hash_files(self, entry_directory):
        files = glob.glob(os.path.join(entry_directory, '.*_hash'))
        for f in files:
            os.remove(f)

    def clobber_migration_files(self, entry_directory):
        files = glob.glob(os.path.join(entry_directory,
                                       '.*_is_first_class_gcs'))
        for f in files:
            os.remove(f)

    def clobber(self):
        """Remove all dep path gcs items and clear .gcs_entries"""
        for _, objects_dict in self._gcs_entries.items():
            for dep_path, _ in objects_dict.items():
                full_path = os.path.join(self.root_dir, dep_path)
                self.clobber_tar_content_names(full_path)
                self.clobber_hash_files(full_path)
                self.clobber_migration_files(full_path)

        if os.path.exists(self._gcs_entries_file):
            os.remove(self._gcs_entries_file)
        with self._mutator_lock:
            self._gcs_entries = {}


class GcsWrapper(SCMWrapper):
    """Wrapper for GCS.

  Currently only supports content from Google Cloud Storage.
  """
    name = 'gcs'

    def __init__(self,
                 url=None,
                 root_dir=None,
                 relpath=None,
                 out_fh=None,
                 out_cb=None):
        super(GcsWrapper, self).__init__(url=url,
                                         root_dir=root_dir,
                                         relpath=relpath,
                                         out_fh=out_fh,
                                         out_cb=out_cb)

    #override
    def GetCacheMirror(self):
        return None

    #override
    def GetActualRemoteURL(self, options):
        return None

    #override
    def DoesRemoteURLMatch(self, options):
        del options
        return True

    def revert(self, options, args, file_list):
        """Does nothing."""

    def diff(self, options, args, file_list):
        """GCS has no notion of diffing."""

    def pack(self, options, args, file_list):
        """GCS has no notion of diffing."""

    def revinfo(self, options, args, file_list):
        """Does nothing"""

    def status(self, options, args, file_list):
        pass

    def update(self, options, args, file_list):
        """Does nothing."""


class CogWrapper(SCMWrapper):
    """Wrapper for Cog, all no-op."""
    name = 'cog'

    def __init__(self):
        super(CogWrapper, self).__init__()

    #override
    def GetCacheMirror(self):
        return None

    #override
    def GetActualRemoteURL(self, options):
        return None

    #override
    def GetSubmoduleDiff(self):
        return None

    #override
    def GetSubmoduleStateFromIndex(self):
        return None

    #override
    def DoesRemoteURLMatch(self, options):
        del options
        return True

    def revert(self, options, args, file_list):
        pass

    def diff(self, options, args, file_list):
        pass

    def pack(self, options, args, file_list):
        pass

    def revinfo(self, options, args, file_list):
        pass

    def status(self, options, args, file_list):
        pass

    def update(self, options, args, file_list):
        pass
