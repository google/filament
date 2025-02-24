#!/usr/bin/env python3
# -*- coding: utf-8 -*-
# Copyright (c) 2011 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
"""Generate fake repositories for testing."""

import atexit
import datetime
import errno
import io
import logging
import os
import pprint
import random
import re
import socket
import sys
import tarfile
import tempfile
import textwrap
import time

# trial_dir must be first for non-system libraries.
from testing_support import trial_dir
import gclient_utils
import scm
import subprocess2

DEFAULT_BRANCH = 'main'


def write(path, content):
    f = open(path, 'wb')
    f.write(content.encode())
    f.close()


join = os.path.join


def read_tree(tree_root):
    """Returns a dict of all the files in a tree. Defaults to self.root_dir."""
    tree = {}
    for root, dirs, files in os.walk(tree_root):
        for d in filter(lambda x: x.startswith('.'), dirs):
            dirs.remove(d)
        for f in [join(root, f) for f in files if not f.startswith('.')]:
            filepath = f[len(tree_root) + 1:].replace(os.sep, '/')
            assert len(filepath) > 0, f
            if tarfile.is_tarfile(join(root, f)):
                tree[filepath] = 'tarfile'
                continue
            with io.open(join(root, f), encoding='utf-8') as f:
                tree[filepath] = f.read()
    return tree


def dict_diff(dict1, dict2):
    diff = {}
    for k, v in dict1.items():
        if k not in dict2:
            diff[k] = v
        elif v != dict2[k]:
            diff[k] = (v, dict2[k])
    for k, v in dict2.items():
        if k not in dict1:
            diff[k] = v
    return diff


def commit_git(repo):
    """Commits the changes and returns the new hash."""
    subprocess2.check_call(['git', 'add', '-A', '-f'], cwd=repo)
    subprocess2.check_call(['git', 'commit', '-q', '--message', 'foo'],
                           cwd=repo)
    rev = subprocess2.check_output(['git', 'show-ref', '--head', 'HEAD'],
                                   cwd=repo).split(b' ', 1)[0]
    rev = rev.decode('utf-8')
    logging.debug('At revision %s' % rev)
    return rev


class FakeReposBase(object):
    """Generate git repositories to test gclient functionality.

    Many DEPS functionalities need to be tested: Var, deps_os, hooks,
    use_relative_paths.

    And types of dependencies: Relative urls, Full urls, git.

    populateGit() needs to be implemented by the subclass.
    """
    # Hostname
    NB_GIT_REPOS = 1
    USERS = [
        ('user1@example.com', 'foo Fuß'),
        ('user2@example.com', 'bar'),
    ]

    def __init__(self, host=None):
        self.trial = trial_dir.TrialDir('repos')
        self.host = host or '127.0.0.1'
        # Format is { repo: [ None, (hash, tree), (hash, tree), ... ], ... }
        # so reference looks like self.git_hashes[repo][rev][0] for hash and
        # self.git_hashes[repo][rev][1] for it's tree snapshot.
        # It is 1-based too.
        self.git_hashes = {}
        self.git_pid_file_name = None
        self.git_base = None
        self.initialized = False

    @property
    def root_dir(self):
        return self.trial.root_dir

    def set_up(self):
        """All late initialization comes here."""
        if not self.root_dir:
            try:
                # self.root_dir is not set before this call.
                self.trial.set_up()
                self.git_base = join(self.root_dir, 'git') + os.sep
            finally:
                # Registers cleanup.
                atexit.register(self.tear_down)

    def tear_down(self):
        """Kills the servers and delete the directories."""
        self.tear_down_git()
        # This deletes the directories.
        self.trial.tear_down()
        self.trial = None

    def tear_down_git(self):
        if self.trial.SHOULD_LEAK:
            return False
        logging.debug('Removing %s' % self.git_base)
        gclient_utils.rmtree(self.git_base)
        return True

    @staticmethod
    def _genTree(root, tree_dict):
        """For a dictionary of file contents, generate a filesystem."""
        if not os.path.isdir(root):
            os.makedirs(root)
        for (k, v) in tree_dict.items():
            k_os = k.replace('/', os.sep)
            k_arr = k_os.split(os.sep)
            if len(k_arr) > 1:
                p = os.sep.join([root] + k_arr[:-1])
                if not os.path.isdir(p):
                    os.makedirs(p)
            if v is None:
                os.remove(join(root, k))
            else:
                write(join(root, k), v)

    def set_up_git(self):
        """Creates git repositories and start the servers."""
        self.set_up()
        if self.initialized:
            return True
        try:
            subprocess2.check_output(['git', '--version'])
        except (OSError, subprocess2.CalledProcessError):
            return False
        for repo in ['repo_%d' % r for r in range(1, self.NB_GIT_REPOS + 1)]:
            subprocess2.check_call([
                'git', 'init', '-b', DEFAULT_BRANCH, '-q',
                join(self.git_base, repo)
            ])
            subprocess2.check_call([
                'git',
                '-C',
                join(self.git_base, repo),
                'config',
                'user.name',
                'Hina Hoshino',
            ])
            subprocess2.check_call([
                'git',
                '-C',
                join(self.git_base, repo),
                'config',
                'user.email',
                'testing@example.com',
            ])
            self.git_hashes[repo] = [(None, None)]
        self.populateGit()
        self.initialized = True
        return True

    def _git_rev_parse(self, path):
        return subprocess2.check_output(['git', 'rev-parse', 'HEAD'],
                                        cwd=path).strip()

    def _commit_git(self, repo, tree, base=None):
        repo_root = join(self.git_base, repo)
        if base:
            base_commit = self.git_hashes[repo][base][0]
            subprocess2.check_call(['git', 'checkout', base_commit],
                                   cwd=repo_root)
        self._genTree(repo_root, tree)
        commit_hash = commit_git(repo_root)
        base = base or -1
        if self.git_hashes[repo][base][1]:
            new_tree = self.git_hashes[repo][base][1].copy()
            new_tree.update(tree)
        else:
            new_tree = tree.copy()
        self.git_hashes[repo].append((commit_hash, new_tree))

    def _create_ref(self, repo, ref, revision):
        repo_root = join(self.git_base, repo)
        subprocess2.check_call(
            ['git', 'update-ref', ref, self.git_hashes[repo][revision][0]],
            cwd=repo_root)

    def _fast_import_git(self, repo, data):
        repo_root = join(self.git_base, repo)
        logging.debug('%s: fast-import %s', repo, data)
        subprocess2.check_call(['git', 'fast-import', '--quiet'],
                               cwd=repo_root,
                               stdin=data.encode())

    def populateGit(self):
        raise NotImplementedError()


class FakeRepos(FakeReposBase):
    """Implements populateGit()."""
    NB_GIT_REPOS = 24

    def populateGit(self):
        # Testing:
        # - dependency disappear
        # - dependency renamed
        # - versioned and unversioned reference
        # - relative and full reference
        # - deps_os
        # - var
        # - hooks
        # TODO(maruel):
        # - use_relative_paths
        self._commit_git('repo_3', {
            'origin': 'git/repo_3@1\n',
        })

        self._commit_git('repo_3', {
            'origin': 'git/repo_3@2\n',
        })

        self._commit_git(
            'repo_1',
            {
                'DEPS': """
vars = {
  'DummyVariable': 'repo',
  'false_var': False,
  'false_str_var': 'False',
  'true_var': True,
  'true_str_var': 'True',
  'str_var': 'abc',
  'cond_var': 'false_str_var and true_var',
}
# Nest the args file in a sub-repo, to make sure we don't try to
# write it before we've cloned everything.
gclient_gn_args_file = 'src/repo2/gclient.args'
gclient_gn_args = [
  'false_var',
  'false_str_var',
  'true_var',
  'true_str_var',
  'str_var',
  'cond_var',
]
deps = {
  'src/repo2': {
    'url': %(git_base)r + 'repo_2',
    'condition': 'True',
  },
  'src/repo2/repo3': '/' + Var('DummyVariable') + '_3@%(hash3)s',
  # Test that deps where condition evaluates to False are skipped.
  'src/repo5': {
    'url': '/repo_5',
    'condition': 'False',
  },
}
deps_os = {
  'mac': {
    'src/repo4': '/repo_4',
  },
}""" % {
                    'git_base': self.git_base,
                    # See self.__init__() for the format. Grab's the hash of the
                    # first commit in repo_2. Only keep the first 7 character
                    # because of: TODO(maruel): http://crosbug.com/3591 We need
                    # to strip the hash..  duh.
                    'hash3': self.git_hashes['repo_3'][1][0][:7]
                },
                'origin': 'git/repo_1@1\n',
                'foo bar': 'some file with a space',
            })

        self._commit_git(
            'repo_2', {
                'origin':
                'git/repo_2@1\n',
                'DEPS':
                """
vars = {
  'repo2_false_var': 'False',
}

deps = {
  'foo/bar': {
    'url': '/repo_3',
    'condition': 'repo2_false_var',
  }
}
""",
            })

        self._commit_git('repo_2', {
            'origin': 'git/repo_2@2\n',
        })

        self._commit_git('repo_4', {
            'origin': 'git/repo_4@1\n',
        })

        self._commit_git('repo_4', {
            'origin': 'git/repo_4@2\n',
        })

        self._commit_git(
            'repo_1',
            {
                'DEPS': """
deps = {
  'src/repo2': %(git_base)r + 'repo_2@%(hash)s',
  'src/repo2/repo_renamed': '/repo_3',
  'src/should_not_process': {
    'url': '/repo_4',
    'condition': 'False',
  }
}
# I think this is wrong to have the hooks run from the base of the gclient
# checkout. It's maybe a bit too late to change that behavior.
hooks = [
  {
    'pattern': '.',
    'action': ['python3', '-c',
               'open(\\'src/git_hooked1\\', \\'w\\').write(\\'git_hooked1\\')'],
  },
  {
    # Should not be run.
    'pattern': 'nonexistent',
    'action': ['python3', '-c',
               'open(\\'src/git_hooked2\\', \\'w\\').write(\\'git_hooked2\\')'],
  },
]
""" % {
                    'git_base': self.git_base,
                    # See self.__init__() for the format. Grab's the hash of the
                    # first commit in repo_2. Only keep the first 7 character
                    # because of: TODO(maruel): http://crosbug.com/3591 We need
                    # to strip the hash.. duh.
                    'hash': self.git_hashes['repo_2'][1][0][:7]
                },
                'origin': 'git/repo_1@2\n',
            })

        self._commit_git('repo_5', {'origin': 'git/repo_5@1\n'})
        self._commit_git(
            'repo_5', {
                'DEPS': """
deps = {
  'src/repo1': %(git_base)r + 'repo_1@%(hash1)s',
  'src/repo2': %(git_base)r + 'repo_2@%(hash2)s',
}

# Hooks to run after a project is processed but before its dependencies are
# processed.
pre_deps_hooks = [
  {
    'action': ['python3', '-c',
               'print("pre-deps hook"); open(\\'src/git_pre_deps_hooked\\', \\'w\\').write(\\'git_pre_deps_hooked\\')'],
  }
]
""" % {
                    'git_base': self.git_base,
                    'hash1': self.git_hashes['repo_1'][2][0][:7],
                    'hash2': self.git_hashes['repo_2'][1][0][:7],
                },
                'origin': 'git/repo_5@2\n',
            })
        self._commit_git(
            'repo_5', {
                'DEPS': """
deps = {
  'src/repo1': %(git_base)r + 'repo_1@%(hash1)s',
  'src/repo2': %(git_base)r + 'repo_2@%(hash2)s',
}

# Hooks to run after a project is processed but before its dependencies are
# processed.
pre_deps_hooks = [
  {
    'action': ['python3', '-c',
               'print("pre-deps hook"); open(\\'src/git_pre_deps_hooked\\', \\'w\\').write(\\'git_pre_deps_hooked\\')'],
  },
  {
    'action': ['python3', '-c', 'import sys; sys.exit(1)'],
  }
]
""" % {
                    'git_base': self.git_base,
                    'hash1': self.git_hashes['repo_1'][2][0][:7],
                    'hash2': self.git_hashes['repo_2'][1][0][:7],
                },
                'origin': 'git/repo_5@3\n',
            })

        self._commit_git(
            'repo_6', {
                'DEPS': """
vars = {
  'DummyVariable': 'repo',
  'git_base': %(git_base)r,
  'hook1_contents': 'git_hooked1',
  'repo5_var': '/repo_5',

  'false_var': False,
  'false_str_var': 'False',
  'true_var': True,
  'true_str_var': 'True',
  'str_var': 'abc',
  'cond_var': 'false_str_var and true_var',
}

gclient_gn_args_file = 'src/repo2/gclient.args'
gclient_gn_args = [
  'false_var',
  'false_str_var',
  'true_var',
  'true_str_var',
  'str_var',
  'cond_var',
]

allowed_hosts = [
  %(git_base)r,
]
deps = {
  'src/repo2': {
    'url': Var('git_base') + 'repo_2@%(hash)s',
    'condition': 'true_str_var',
  },
  'src/repo4': {
    'url': '/repo_4',
    'condition': 'False',
  },
  # Entries can have a None repository, which has the effect of either:
  # - disabling a dep checkout (e.g. in a .gclient solution to prevent checking
  # out optional large repos, or in deps_os where some repos aren't used on some
  # platforms)
  # - allowing a completely local directory to be processed by gclient (handy
  # for dealing with "custom" DEPS, like buildspecs).
  '/repoLocal': {
    'url': None,
  },
  'src/repo8': '/repo_8',
  'src/repo15': '/repo_15',
  'src/repo16': '/repo_16',
}
deps_os ={
  'mac': {
    # This entry should not appear in flattened DEPS' |deps|.
    'src/mac_repo': '{repo5_var}',
  },
  'unix': {
    # This entry should not appear in flattened DEPS' |deps|.
    'src/unix_repo': '{repo5_var}',
  },
  'win': {
    # This entry should not appear in flattened DEPS' |deps|.
    'src/win_repo': '{repo5_var}',
  },
}
hooks = [
  {
    'pattern': '.',
    'condition': 'True',
    'action': ['python3', '-c',
               'open(\\'src/git_hooked1\\', \\'w\\').write(\\'{hook1_contents}\\')'],
  },
  {
    # Should not be run.
    'pattern': 'nonexistent',
    'action': ['python3', '-c',
               'open(\\'src/git_hooked2\\', \\'w\\').write(\\'git_hooked2\\')'],
  },
]
hooks_os = {
  'mac': [
    {
      'pattern': '.',
      'action': ['python3', '-c',
                 'open(\\'src/git_hooked_mac\\', \\'w\\').write('
                     '\\'git_hooked_mac\\')'],
    },
  ],
}
recursedeps = [
  'src/repo2',
  'src/repo8',
  'src/repo15',
  'src/repo16',
]""" % {
                    'git_base': self.git_base,
                    'hash': self.git_hashes['repo_2'][1][0][:7]
                },
                'origin': 'git/repo_6@1\n',
            })

        self._commit_git(
            'repo_7', {
                'DEPS': """
vars = {
  'true_var': 'True',
  'false_var': 'true_var and False',
}
hooks = [
  {
    'action': ['python3', '-c',
               'open(\\'src/should_run\\', \\'w\\').write(\\'should_run\\')'],
    'condition': 'true_var or True',
  },
  {
    'action': ['python3', '-c',
               'open(\\'src/should_not_run\\', \\'w\\').write(\\'should_not_run\\')'],
    'condition': 'false_var',
  },
]""",
                'origin': 'git/repo_7@1\n',
            })

        self._commit_git(
            'repo_8', {
                'DEPS': """
deps_os ={
  'mac': {
    'src/recursed_os_repo': '/repo_5',
  },
  'unix': {
    'src/recursed_os_repo': '/repo_5',
  },
}""",
                'origin': 'git/repo_8@1\n',
            })

        self._commit_git(
            'repo_9', {
                'DEPS': """
vars = {
  'str_var': 'xyz',
}
gclient_gn_args_file = 'src/repo8/gclient.args'
gclient_gn_args = [
  'str_var',
]
deps = {
  'src/repo8': '/repo_8',

  # This entry should appear in flattened file,
  # but not recursed into, since it's not
  # in recursedeps.
  'src/repo7': '/repo_7',
}
deps_os = {
  'android': {
    # This entry should only appear in flattened |deps_os|,
    # not |deps|, even when used with |recursedeps|.
    'src/repo4': '/repo_4',
  }
}
recursedeps = [
  'src/repo4',
  'src/repo8',
]""",
                'origin': 'git/repo_9@1\n',
            })

        self._commit_git(
            'repo_10', {
                'DEPS': """
gclient_gn_args_from = 'src/repo9'
deps = {
  'src/repo9': '/repo_9',

  # This entry should appear in flattened file,
  # but not recursed into, since it's not
  # in recursedeps.
  'src/repo6': '/repo_6',
}
deps_os = {
  'mac': {
    'src/repo11': '/repo_11',
  },
  'ios': {
    'src/repo11': '/repo_11',
  }
}
recursedeps = [
  'src/repo9',
  'src/repo11',
]""",
                'origin': 'git/repo_10@1\n',
            })

        self._commit_git(
            'repo_11', {
                'DEPS': """
deps = {
  'src/repo12': '/repo_12',
}""",
                'origin': 'git/repo_11@1\n',
            })

        self._commit_git('repo_12', {
            'origin': 'git/repo_12@1\n',
        })

        self._fast_import_git(
            'repo_12', """blob
mark :1
data 6
Hello

blob
mark :2
data 4
Bye

reset refs/changes/1212
commit refs/changes/1212
mark :3
author Bob <bob@example.com> 1253744361 -0700
committer Bob <bob@example.com> 1253744361 -0700
data 8
A and B
M 100644 :1 a
M 100644 :2 b
""")

        self._commit_git(
            'repo_13', {
                'DEPS': """
deps = {
  'src/repo12': '/repo_12',
}""",
                'origin': 'git/repo_13@1\n',
            })

        self._commit_git(
            'repo_13', {
                'DEPS': """
deps = {
  'src/repo12': '/repo_12@refs/changes/1212',
}""",
                'origin': 'git/repo_13@2\n',
            })

        # src/repo12 is now a CIPD dependency.
        self._commit_git(
            'repo_13', {
                'DEPS': """
deps = {
  'src/repo12': {
    'packages': [
      {
        'package': 'foo',
        'version': '1.3',
      },
    ],
    'dep_type': 'cipd',
  },
}
hooks = [{
  # make sure src/repo12 exists and is a CIPD dir.
  'action': ['python3', '-c', 'with open("src/repo12/_cipd"): pass'],
}]
""",
                'origin': 'git/repo_13@3\n'
            })

        self._commit_git(
            'repo_14', {
                'DEPS':
                textwrap.dedent("""\
        vars = {}
        deps = {
          'src/cipd_dep': {
            'packages': [
              {
                'package': 'package0',
                'version': '0.1',
              },
            ],
            'dep_type': 'cipd',
          },
          'src/another_cipd_dep': {
            'packages': [
              {
                'package': 'package1',
                'version': '1.1-cr0',
              },
              {
                'package': 'package2',
                'version': '1.13',
              },
            ],
            'dep_type': 'cipd',
          },
          'src/cipd_dep_with_cipd_variable': {
            'packages': [
              {
                'package': 'package3/${{platform}}',
                'version': '1.2',
              },
            ],
            'dep_type': 'cipd',
          },
        }"""),
                'origin':
                'git/repo_14@2\n'
            })

        # A repo with a hook to be recursed in, without use_relative_paths
        self._commit_git(
            'repo_15', {
                'DEPS':
                textwrap.dedent("""\
        hooks = [{
          "name": "absolute_cwd",
          "pattern": ".",
          "action": ["python3", "-c", "pass"]
        }]"""),
                'origin':
                'git/repo_15@2\n'
            })
        # A repo with a hook to be recursed in, with use_relative_paths
        self._commit_git(
            'repo_16', {
                'DEPS':
                textwrap.dedent("""\
        use_relative_paths=True
        hooks = [{
          "name": "relative_cwd",
          "pattern": ".",
          "action": ["python3", "relative.py"]
        }]"""),
                'relative.py':
                'pass',
                'origin':
                'git/repo_16@2\n'
            })
        # A repo with a gclient_gn_args_file and use_relative_paths
        self._commit_git(
            'repo_17', {
                'DEPS':
                textwrap.dedent("""\
        use_relative_paths=True
        vars = {
          'toto': 'tata',
        }
        gclient_gn_args_file = 'repo17_gclient.args'
        gclient_gn_args = [
          'toto',
        ]"""),
                'origin':
                'git/repo_17@2\n'
            })

        self._commit_git(
            'repo_18', {
                'DEPS':
                textwrap.dedent("""\
        deps = {
          'src/cipd_dep': {
            'packages': [
              {
                'package': 'package0',
                'version': 'package0-fake-tag:1.0',
              },
              {
                'package': 'package0/${{platform}}',
                'version': 'package0/${{platform}}-fake-tag:1.0',
              },
              {
                'package': 'package1/another',
                'version': 'package1/another-fake-tag:1.0',
              },
            ],
            'dep_type': 'cipd',
          },
        }"""),
                'origin':
                'git/repo_18@2\n'
            })

        # a relative path repo
        self._commit_git(
            'repo_19', {
                'DEPS': """

git_dependencies = "SUBMODULES"
use_relative_paths = True
vars = {
  'foo_checkout': True,
}
deps = {
  "some_repo": {
    "url": '/repo_2@%(hash_2)s',
    "condition": "not foo_checkout",
  },
  "chicken/dickens": {
    "url": '/repo_3@%(hash_3)s',
  },
  "weird/deps": {
    "url": '/repo_1'
  },
  "bar": {
    "packages": [{
      "package": "lemur",
      "version": "version:1234",
     }],
     "dep_type": "cipd",
  },
}

recursedeps = [
  'chicken/dickens',
]""" % {
                    'hash_2': self.git_hashes['repo_2'][1][0],
                    'hash_3': self.git_hashes['repo_3'][1][0],
                },
            })

        # a non-relative_path repo
        self._commit_git(
            'repo_20', {
                'DEPS': """

git_dependencies = "SUBMODULES"
vars = {
  'foo_checkout': True,
}
deps = {
  "foo/some_repo": {
    "url": '/repo_2@%(hash_2)s',
    "condition": "not foo_checkout",
  },
  "foo/chicken/dickens": {
    "url": '/repo_3@%(hash_3)s',
  },
  "foo/weird/deps": {
    "url": '/repo_1'
  },
  "foo/bar": {
    "packages": [{
      "package": "lemur",
      "version": "version:1234",
     }],
     "dep_type": "cipd",
  },
}

recursedeps = [
  'foo/chicken/dickens',
]
""" % {
                    'hash_2': self.git_hashes['repo_2'][1][0],
                    'hash_3': self.git_hashes['repo_3'][1][0],
                },
            })

        # gitmodules already present, test migration, .git suffix
        self._commit_git(
            'repo_21',
            {
                'DEPS':
                """
use_relative_paths = True
git_dependencies = "SYNC"
deps = {
  "bar": {
    "url": 'https://example.googlesource.com/repo.git@%(hash)s',
  },
}""" % {
                    'hash': self.git_hashes['repo_2'][1][0],
                },
                '.gitmodules':
                """
[submodule "bar"]
	path = bar
	url = invalid/repo_url.git"""
            },
        )

        self._commit_git(
            'repo_22', {
                'DEPS':
                textwrap.dedent("""\
        vars = {}
        deps = {
          'src/gcs_dep': {
            'bucket': '123bucket',
            'dep_type': 'gcs',
            'objects': [{
                'object_name': 'deadbeef',
                'sha256sum': 'abcd123',
                'size_bytes': 10000,
                'generation': 1542380408102454,
            }]
          },
          'src/another_gcs_dep': {
            'bucket': '456bucket',
            'dep_type': 'gcs',
            'objects': [{
                'object_name': 'Linux/llvmfile.tar.gz',
                'sha256sum': 'abcd123',
                'size_bytes': 10000,
                'generation': 1542380408102455,
            }]
          },
          'src/gcs_dep_with_output_file': {
            'bucket': '789bucket',
            'dep_type': 'gcs',
            'objects': [{
                'object_name': 'clang-format-version123',
                'sha256sum': 'abcd123',
                'output_file': 'clang-format-no-extract',
                'size_bytes': 10000,
                'generation': 1542380408102456,
            }]
          },
          'src/gcs_dep_ignored': {
            'bucket': '789bucket',
            'dep_type': 'gcs',
            'objects': [{
                'object_name': 'foobar',
                'sha256sum': 'abcd124',
                'size_bytes': 10000,
                'generation': 1542380408102457,
                'condition': 'False',
            }]
          },
        }"""),
                'origin':
                'git/repo_22@1\n'
            })

        self._commit_git(
            'repo_23', {
                'DEPS': """
deps = {
  'src/repo12': '/repo_12',
}""",
                'origin': 'git/repo_23@1\n',
            })

        self._commit_git(
            'repo_23', {
                'DEPS': """
deps = {
  'src/repo12': '/repo_12@refs/changes/1212',
}""",
                'origin': 'git/repo_23@2\n',
            })

        # src/repo12 is now a GCS dependency.
        self._commit_git(
            'repo_23', {
                'DEPS': """
deps = {
  'src/repo12': {
    'bucket': 'bucket123',
    'dep_type': 'gcs',
    'objects': [{
        'object_name': 'path_to_file.tar.gz',
        'sha256sum': 'abcd123',
        'size_bytes': 10000,
        'generation': 1542380408102454,
    }]
  },
}
""",
                'origin': 'git/repo_23@3\n'
            })

        # gitmodules already present, test migration, gclient-recursedeps
        self._commit_git(
            'repo_24',
            {
                'DEPS':
                """
use_relative_paths = True
git_dependencies = "SYNC"
deps = {
  "bar": {
    "url": 'https://example.googlesource.com/repo@%(hash)s',
  },
}

recursedeps = [
  'bar',
]""" % {
                    'hash': self.git_hashes['repo_2'][1][0],
                },
                '.gitmodules':
                """
[submodule "bar"]
	path = bar
	url = https://example.googlesource.com/repo"""
            },
        )  # Update `NB_GIT_REPOS` if you add more repos.


class FakeRepoSkiaDEPS(FakeReposBase):
    """Simulates the Skia DEPS transition in Chrome."""

    NB_GIT_REPOS = 5

    DEPS_git_pre = """deps = {
  'src/third_party/skia/gyp': %(git_base)r + 'repo_3',
  'src/third_party/skia/include': %(git_base)r + 'repo_4',
  'src/third_party/skia/src': %(git_base)r + 'repo_5',
}"""

    DEPS_post = """deps = {
  'src/third_party/skia': %(git_base)r + 'repo_1',
}"""

    def populateGit(self):
        # Skia repo.
        self._commit_git(
            'repo_1', {
                'skia_base_file': 'root-level file.',
                'gyp/gyp_file': 'file in the gyp directory',
                'include/include_file': 'file in the include directory',
                'src/src_file': 'file in the src directory',
            })
        self._commit_git(
            'repo_3',
            {  # skia/gyp
                'gyp_file': 'file in the gyp directory',
            })
        self._commit_git('repo_4', { # skia/include
            'include_file': 'file in the include directory',
        })
        self._commit_git(
            'repo_5',
            {  # skia/src
                'src_file': 'file in the src directory',
            })

        # Chrome repo.
        self._commit_git(
            'repo_2', {
                'DEPS': self.DEPS_git_pre % {
                    'git_base': self.git_base
                },
                'myfile': 'src/trunk/src@1'
            })
        self._commit_git(
            'repo_2', {
                'DEPS': self.DEPS_post % {
                    'git_base': self.git_base
                },
                'myfile': 'src/trunk/src@2'
            })


class FakeRepoBlinkDEPS(FakeReposBase):
    """Simulates the Blink DEPS transition in Chrome."""

    NB_GIT_REPOS = 2
    DEPS_pre = 'deps = {"src/third_party/WebKit": "%(git_base)srepo_2",}'
    DEPS_post = 'deps = {}'

    def populateGit(self):
        # Blink repo.
        self._commit_git(
            'repo_2', {
                'OWNERS': 'OWNERS-pre',
                'Source/exists_always': '_ignored_',
                'Source/exists_before_but_not_after': '_ignored_',
            })

        # Chrome repo.
        self._commit_git(
            'repo_1', {
                'DEPS': self.DEPS_pre % {
                    'git_base': self.git_base
                },
                'myfile': 'myfile@1',
                '.gitignore': '/third_party/WebKit',
            })
        self._commit_git(
            'repo_1', {
                'DEPS': self.DEPS_post % {
                    'git_base': self.git_base
                },
                'myfile': 'myfile@2',
                '.gitignore': '',
                'third_party/WebKit/OWNERS': 'OWNERS-post',
                'third_party/WebKit/Source/exists_always': '_ignored_',
                'third_party/WebKit/Source/exists_after_but_not_before':
                '_ignored',
            })

    def populateSvn(self):
        raise NotImplementedError()


class FakeRepoNoSyncDEPS(FakeReposBase):
    """Simulates a repo with some DEPS changes."""

    NB_GIT_REPOS = 2

    def populateGit(self):
        self._commit_git('repo_2', {'myfile': 'then egg'})
        self._commit_git('repo_2', {'myfile': 'before egg!'})

        self._commit_git(
            'repo_1', {
                'DEPS':
                textwrap.dedent(
                    """\
          deps = {
            'src/repo2': {
              'url': %(git_base)r + 'repo_2@%(repo2hash)s',
            },
          }""" % {
                        'git_base': self.git_base,
                        'repo2hash': self.git_hashes['repo_2'][1][0][:7]
                    })
            })
        self._commit_git(
            'repo_1', {
                'DEPS':
                textwrap.dedent(
                    """\
          deps = {
            'src/repo2': {
              'url': %(git_base)r + 'repo_2@%(repo2hash)s',
            },
          }""" % {
                        'git_base': self.git_base,
                        'repo2hash': self.git_hashes['repo_2'][2][0][:7]
                    })
            })
        self._commit_git(
            'repo_1', {
                'foo_file':
                'chicken content',
                'DEPS':
                textwrap.dedent(
                    """\
          deps = {
            'src/repo2': {
              'url': %(git_base)r + 'repo_2@%(repo2hash)s',
            },
          }""" % {
                        'git_base': self.git_base,
                        'repo2hash': self.git_hashes['repo_2'][1][0][:7]
                    })
            })

        self._commit_git('repo_1', {'foo_file': 'chicken content@4'})


class FakeReposTestBase(trial_dir.TestCase):
    """This is vaguely inspired by twisted."""
    # Static FakeRepos instances. Lazy loaded.
    CACHED_FAKE_REPOS = {}
    # Override if necessary.
    FAKE_REPOS_CLASS = FakeRepos

    def setUp(self):
        super(FakeReposTestBase, self).setUp()
        if not self.FAKE_REPOS_CLASS in self.CACHED_FAKE_REPOS:
            self.CACHED_FAKE_REPOS[
                self.FAKE_REPOS_CLASS] = self.FAKE_REPOS_CLASS()
        self.FAKE_REPOS = self.CACHED_FAKE_REPOS[self.FAKE_REPOS_CLASS]
        # No need to call self.FAKE_REPOS.setUp(), it will be called by the
        # child class. Do not define tearDown(), since super's version does the
        # right thing and self.FAKE_REPOS is kept across tests.

    @property
    def git_base(self):
        """Shortcut."""
        return self.FAKE_REPOS.git_base

    def checkString(self, expected, result, msg=None):
        """Prints the diffs to ease debugging."""
        self.assertEqual(expected.splitlines(), result.splitlines(), msg)
        if expected != result:
            # Strip the beginning
            while expected and result and expected[0] == result[0]:
                expected = expected[1:]
                result = result[1:]
            # The exception trace makes it hard to read so dump it too.
            if '\n' in result:
                print(result)
        self.assertEqual(expected, result, msg)

    def check(self, expected, results):
        """Checks stdout, stderr, returncode."""
        self.checkString(expected[0], results[0])
        self.checkString(expected[1], results[1])
        self.assertEqual(expected[2], results[2])

    def assertTree(self, tree, tree_root=None):
        """Diff the checkout tree with a dict."""
        if not tree_root:
            tree_root = self.root_dir
        actual = read_tree(tree_root)
        self.assertEqual(sorted(tree.keys()), sorted(actual.keys()))
        self.assertEqual(tree, actual)

    def mangle_git_tree(self, *args):
        """Creates a 'virtual directory snapshot' to compare with the actual
        result on disk."""
        result = {}
        for item, new_root in args:
            repo, rev = item.split('@', 1)
            tree = self.gittree(repo, rev)
            for k, v in tree.items():
                path = join(new_root, k).replace(os.sep, '/')
                result[path] = v
        return result

    def githash(self, repo, rev):
        """Sort-hand: Returns the hash for a git 'revision'."""
        return self.FAKE_REPOS.git_hashes[repo][int(rev)][0]

    def gittree(self, repo, rev):
        """Sort-hand: returns the directory tree for a git 'revision'."""
        return self.FAKE_REPOS.git_hashes[repo][int(rev)][1]

    def gitrevparse(self, repo):
        """Returns the actual revision for a given repo."""
        return self.FAKE_REPOS._git_rev_parse(repo).decode('utf-8')


def main(argv):
    fake = FakeRepos()
    print('Using %s' % fake.root_dir)
    try:
        fake.set_up_git()
        print(
            'Fake setup, press enter to quit or Ctrl-C to keep the checkouts.')
        sys.stdin.readline()
    except KeyboardInterrupt:
        trial_dir.TrialDir.SHOULD_LEAK.leak = True
    return 0


if __name__ == '__main__':
    sys.exit(main(sys.argv))
