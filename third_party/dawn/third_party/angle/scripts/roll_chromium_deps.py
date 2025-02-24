#!/usr/bin/env python3
# Copyright 2019 The ANGLE project authors. All Rights Reserved.
#
# Use of this source code is governed by a BSD-style license
# that can be found in the LICENSE file in the root of the source
# tree. An additional intellectual property rights grant can be found
# in the file PATENTS.  All contributing project authors may
# be found in the AUTHORS file in the root of the source tree.

# This is a modified copy of the script in
# https://webrtc.googlesource.com/src/+/main/tools_webrtc/autoroller/roll_deps.py
# customized for ANGLE.
"""Script to automatically roll Chromium dependencies in the ANGLE DEPS file."""

import argparse
import base64
import collections
import logging
import os
import platform
import re
import subprocess
import sys
import urllib.request


def FindSrcDirPath():
    """Returns the abs path to the root dir of the project."""
    # Special cased for ANGLE.
    return os.path.dirname(os.path.abspath(os.path.join(__file__, '..')))

ANGLE_CHROMIUM_DEPS = [
    'build',
    'buildtools',
    'buildtools/linux64',
    'buildtools/mac',
    'buildtools/reclient',
    'buildtools/win',
    'testing',
    'third_party/abseil-cpp',
    'third_party/android_build_tools',
    'third_party/android_build_tools/aapt2/cipd',
    'third_party/android_build_tools/art',
    'third_party/android_build_tools/bundletool',
    'third_party/android_build_tools/error_prone/cipd',
    'third_party/android_build_tools/error_prone_javac/cipd',
    'third_party/android_build_tools/lint/cipd',
    'third_party/android_build_tools/manifest_merger/cipd',
    'third_party/android_build_tools/nullaway/cipd',
    'third_party/android_deps',
    'third_party/android_platform',
    'third_party/android_sdk',
    'third_party/android_sdk/public',
    'third_party/android_system_sdk/cipd',
    'third_party/android_toolchain/ndk',
    'third_party/bazel',
    'third_party/catapult',
    'third_party/clang-format/script',
    'third_party/colorama/src',
    'third_party/cpu_features/src',
    'third_party/depot_tools',
    'third_party/flatbuffers/src',
    'third_party/fuchsia-sdk/sdk',
    'third_party/ijar',
    'third_party/jdk',
    'third_party/jdk/extras',
    'third_party/jinja2',
    'third_party/kotlin_stdlib',
    'third_party/libc++/src',
    'third_party/libc++abi/src',
    'third_party/libdrm/src',
    'third_party/libjpeg_turbo',
    'third_party/libunwind/src',
    'third_party/llvm-libc/src',
    'third_party/markupsafe',
    'third_party/nasm',
    'third_party/ninja',
    'third_party/proguard',
    'third_party/protobuf',
    'third_party/Python-Markdown',
    'third_party/qemu-linux-x64',
    'third_party/qemu-mac-x64',
    'third_party/r8/cipd',
    'third_party/r8/d8/cipd',
    'third_party/requests/src',
    'third_party/rust',
    'third_party/siso/cipd',
    'third_party/six',
    'third_party/turbine/cipd',
    'third_party/zlib',
    'tools/android',
    'tools/clang',
    'tools/clang/dsymutil',
    'tools/luci-go',
    'tools/mb',
    'tools/md_browser',
    'tools/memory',
    'tools/perf',
    'tools/protoc_wrapper',
    'tools/python',
    'tools/rust',
    'tools/skia_goldctl/linux',
    'tools/skia_goldctl/mac_amd64',
    'tools/skia_goldctl/mac_arm64',
    'tools/skia_goldctl/win',
    'tools/valgrind',
]

ANGLE_URL = 'https://chromium.googlesource.com/angle/angle'
CHROMIUM_SRC_URL = 'https://chromium.googlesource.com/chromium/src'
CHROMIUM_COMMIT_TEMPLATE = CHROMIUM_SRC_URL + '/+/%s'
CHROMIUM_LOG_TEMPLATE = CHROMIUM_SRC_URL + '/+log/%s'
CHROMIUM_FILE_TEMPLATE = CHROMIUM_SRC_URL + '/+/%s/%s'

COMMIT_POSITION_RE = re.compile('^Cr-Commit-Position: .*#([0-9]+).*$')
CLANG_REVISION_RE = re.compile(r'^CLANG_REVISION = \'([-0-9a-z]+)\'')
ROLL_BRANCH_NAME = 'roll_chromium_revision'

SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))
CHECKOUT_SRC_DIR = FindSrcDirPath()
CHECKOUT_ROOT_DIR = CHECKOUT_SRC_DIR

# Copied from tools/android/roll/android_deps/.../BuildConfigGenerator.groovy.
ANDROID_DEPS_START = r'=== ANDROID_DEPS Generated Code Start ==='
ANDROID_DEPS_END = r'=== ANDROID_DEPS Generated Code End ==='
# Location of automically gathered android deps.
ANDROID_DEPS_PATH = 'src/third_party/android_deps/'

NOTIFY_EMAIL = 'angle-wrangler@grotations.appspotmail.com'

CLANG_TOOLS_URL = 'https://chromium.googlesource.com/chromium/src/tools/clang'
CLANG_FILE_TEMPLATE = CLANG_TOOLS_URL + '/+/%s/%s'

CLANG_TOOLS_PATH = 'tools/clang'
CLANG_UPDATE_SCRIPT_URL_PATH = 'scripts/update.py'
CLANG_UPDATE_SCRIPT_LOCAL_PATH = os.path.join(CHECKOUT_SRC_DIR, 'tools', 'clang', 'scripts',
                                              'update.py')

DepsEntry = collections.namedtuple('DepsEntry', 'path url revision')
ChangedDep = collections.namedtuple('ChangedDep', 'path url current_rev new_rev')
ClangChange = collections.namedtuple('ClangChange', 'mirror_change clang_change')
CipdDepsEntry = collections.namedtuple('CipdDepsEntry', 'path packages')
ChangedCipdPackage = collections.namedtuple('ChangedCipdPackage',
                                            'path package current_version new_version')

ChromiumRevisionUpdate = collections.namedtuple('ChromiumRevisionUpdate', ('current_chromium_rev '
                                                                           'new_chromium_rev '))


def AddDepotToolsToPath():
    sys.path.append(os.path.join(CHECKOUT_SRC_DIR, 'build'))
    import find_depot_tools
    find_depot_tools.add_depot_tools_to_path()


class RollError(Exception):
    pass


def StrExpansion():
    return lambda str_value: str_value


def VarLookup(local_scope):
    return lambda var_name: local_scope['vars'][var_name]


def ParseDepsDict(deps_content):
    local_scope = {}
    global_scope = {
        'Str': StrExpansion(),
        'Var': VarLookup(local_scope),
        'deps_os': {},
    }
    exec (deps_content, global_scope, local_scope)
    return local_scope


def ParseLocalDepsFile(filename):
    with open(filename, 'rb') as f:
        deps_content = f.read()
    return ParseDepsDict(deps_content)


def ParseCommitPosition(commit_message):
    for line in reversed(commit_message.splitlines()):
        m = COMMIT_POSITION_RE.match(line.strip())
        if m:
            return int(m.group(1))
    logging.error('Failed to parse commit position id from:\n%s\n', commit_message)
    sys.exit(-1)


def _RunCommand(command, working_dir=None, ignore_exit_code=False, extra_env=None,
                input_data=None):
    """Runs a command and returns the output from that command.

  If the command fails (exit code != 0), the function will exit the process.

  Returns:
    A tuple containing the stdout and stderr outputs as strings.
  """
    working_dir = working_dir or CHECKOUT_SRC_DIR
    logging.debug('CMD: %s CWD: %s', ' '.join(command), working_dir)
    env = os.environ.copy()
    if extra_env:
        assert all(isinstance(value, str) for value in extra_env.values())
        logging.debug('extra env: %s', extra_env)
        env.update(extra_env)
    p = subprocess.Popen(
        command,
        stdin=subprocess.PIPE,
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
        env=env,
        cwd=working_dir,
        universal_newlines=True)
    std_output, err_output = p.communicate(input_data)
    p.stdout.close()
    p.stderr.close()
    if not ignore_exit_code and p.returncode != 0:
        logging.error('Command failed: %s\n'
                      'stdout:\n%s\n'
                      'stderr:\n%s\n', ' '.join(command), std_output, err_output)
        sys.exit(p.returncode)
    return std_output, err_output


def _GetBranches():
    """Returns a tuple of active,branches.

  The 'active' is the name of the currently active branch and 'branches' is a
  list of all branches.
  """
    lines = _RunCommand(['git', 'branch'])[0].split('\n')
    branches = []
    active = ''
    for line in lines:
        if '*' in line:
            # The assumption is that the first char will always be the '*'.
            active = line[1:].strip()
            branches.append(active)
        else:
            branch = line.strip()
            if branch:
                branches.append(branch)
    return active, branches


def _ReadGitilesContent(url):
    # Download and decode BASE64 content until
    # https://code.google.com/p/gitiles/issues/detail?id=7 is fixed.
    logging.debug('Reading gitiles URL %s' % url)
    base64_content = ReadUrlContent(url + '?format=TEXT')
    return base64.b64decode(base64_content[0]).decode('utf-8')


def ReadRemoteCrFile(path_below_src, revision):
    """Reads a remote Chromium file of a specific revision. Returns a string."""
    return _ReadGitilesContent(CHROMIUM_FILE_TEMPLATE % (revision, path_below_src))


def ReadRemoteCrCommit(revision):
    """Reads a remote Chromium commit message. Returns a string."""
    return _ReadGitilesContent(CHROMIUM_COMMIT_TEMPLATE % revision)


def ReadRemoteClangFile(path_below_src, revision):
    """Reads a remote Clang file of a specific revision. Returns a string."""
    return _ReadGitilesContent(CLANG_FILE_TEMPLATE % (revision, path_below_src))


def ReadUrlContent(url):
    """Connect to a remote host and read the contents. Returns a list of lines."""
    conn = urllib.request.urlopen(url)
    try:
        return conn.readlines()
    except IOError as e:
        logging.exception('Error connecting to %s. Error: %s', url, e)
        raise
    finally:
        conn.close()


def GetMatchingDepsEntries(depsentry_dict, dir_path):
    """Gets all deps entries matching the provided path.

  This list may contain more than one DepsEntry object.
  Example: dir_path='src/testing' would give results containing both
  'src/testing/gtest' and 'src/testing/gmock' deps entries for Chromium's DEPS.
  Example 2: dir_path='src/build' should return 'src/build' but not
  'src/buildtools'.

  Returns:
    A list of DepsEntry objects.
  """
    result = []
    for path, depsentry in depsentry_dict.items():
        if path == dir_path:
            result.append(depsentry)
        else:
            parts = path.split('/')
            if all(part == parts[i] for i, part in enumerate(dir_path.split('/'))):
                result.append(depsentry)
    return result


def BuildDepsentryDict(deps_dict):
    """Builds a dict of paths to DepsEntry objects from a raw parsed deps dict."""
    result = {}

    def AddDepsEntries(deps_subdict):
        for path, dep in deps_subdict.items():
            if path in result:
                continue
            if not isinstance(dep, dict):
                dep = {'url': dep}
            if dep.get('dep_type') == 'cipd':
                result[path] = CipdDepsEntry(path, dep['packages'])
            elif dep.get('dep_type') == 'gcs':
                # Ignore GCS deps - there aren't any that we want to sync yet
                continue
            else:
                if '@' not in dep['url']:
                    continue
                url, revision = dep['url'].split('@')
                result[path] = DepsEntry(path, url, revision)

    AddDepsEntries(deps_dict['deps'])
    for deps_os in ['win', 'mac', 'unix', 'android', 'ios', 'unix']:
        AddDepsEntries(deps_dict.get('deps_os', {}).get(deps_os, {}))
    return result


def _FindChangedCipdPackages(path, old_pkgs, new_pkgs):
    pkgs_equal = ({p['package'] for p in old_pkgs} == {p['package'] for p in new_pkgs})
    assert pkgs_equal, ('Old: %s\n New: %s.\nYou need to do a manual roll '
                        'and remove/add entries in DEPS so the old and new '
                        'list match.' % (old_pkgs, new_pkgs))
    for old_pkg in old_pkgs:
        for new_pkg in new_pkgs:
            old_version = old_pkg['version']
            new_version = new_pkg['version']
            if (old_pkg['package'] == new_pkg['package'] and old_version != new_version):
                logging.debug('Roll dependency %s to %s', path, new_version)
                yield ChangedCipdPackage(path, old_pkg['package'], old_version, new_version)


def _FindNewDeps(old, new):
    """ Gather dependencies only in |new| and return corresponding paths. """
    old_entries = set(BuildDepsentryDict(old))
    new_entries = set(BuildDepsentryDict(new))
    return [path for path in new_entries - old_entries if path in ANGLE_CHROMIUM_DEPS]


def CalculateChangedDeps(angle_deps, new_cr_deps):
    """
  Calculate changed deps entries based on entries defined in the ANGLE DEPS
  file:
     - If a shared dependency with the Chromium DEPS file: roll it to the same
       revision as Chromium (i.e. entry in the new_cr_deps dict)
     - If it's a Chromium sub-directory, roll it to the HEAD revision (notice
       this means it may be ahead of the chromium_revision, but generally these
       should be close).
     - If it's another DEPS entry (not shared with Chromium), roll it to HEAD
       unless it's configured to be skipped.

  Returns:
    A list of ChangedDep objects representing the changed deps.
  """

    def ChromeURL(angle_deps_entry):
        # Perform variable substitutions.
        # This is a hack to get around the unsupported way this script parses DEPS.
        # A better fix would be to use the gclient APIs to query and update DEPS.
        # However this is complicated by how this script downloads DEPS remotely.
        return angle_deps_entry.url.replace('{chromium_git}', 'https://chromium.googlesource.com')

    result = []
    angle_entries = BuildDepsentryDict(angle_deps)
    new_cr_entries = BuildDepsentryDict(new_cr_deps)
    for path, angle_deps_entry in angle_entries.items():
        if path not in ANGLE_CHROMIUM_DEPS:
            continue

        # All ANGLE Chromium dependencies are located in src/.
        chrome_path = 'src/%s' % path
        cr_deps_entry = new_cr_entries.get(chrome_path)

        if cr_deps_entry:
            assert type(cr_deps_entry) is type(angle_deps_entry)

            if isinstance(cr_deps_entry, CipdDepsEntry):
                result.extend(
                    _FindChangedCipdPackages(path, angle_deps_entry.packages,
                                             cr_deps_entry.packages))
                continue

            # Use the revision from Chromium's DEPS file.
            new_rev = cr_deps_entry.revision
            assert ChromeURL(angle_deps_entry) == cr_deps_entry.url, (
                'ANGLE DEPS entry %s has a different URL (%s) than Chromium (%s).' %
                (path, ChromeURL(angle_deps_entry), cr_deps_entry.url))
        else:
            if isinstance(angle_deps_entry, DepsEntry):
                # Use the HEAD of the deps repo.
                stdout, _ = _RunCommand(['git', 'ls-remote', ChromeURL(angle_deps_entry), 'HEAD'])
                new_rev = stdout.strip().split('\t')[0]
            else:
                # The dependency has been removed from chromium.
                # This is handled by FindRemovedDeps.
                continue

        # Check if an update is necessary.
        if angle_deps_entry.revision != new_rev:
            logging.debug('Roll dependency %s to %s', path, new_rev)
            result.append(
                ChangedDep(path, ChromeURL(angle_deps_entry), angle_deps_entry.revision, new_rev))
    return sorted(result)


def CalculateChangedClang(changed_deps, autoroll):
    mirror_change = [change for change in changed_deps if change.path == CLANG_TOOLS_PATH]
    if not mirror_change:
        return None

    mirror_change = mirror_change[0]

    def GetClangRev(lines):
        for line in lines:
            match = CLANG_REVISION_RE.match(line)
            if match:
                return match.group(1)
        raise RollError('Could not parse Clang revision!')

    old_clang_update_py = ReadRemoteClangFile(CLANG_UPDATE_SCRIPT_URL_PATH,
                                              mirror_change.current_rev).splitlines()
    old_clang_rev = GetClangRev(old_clang_update_py)
    logging.debug('Found old clang rev: %s' % old_clang_rev)

    new_clang_update_py = ReadRemoteClangFile(CLANG_UPDATE_SCRIPT_URL_PATH,
                                              mirror_change.new_rev).splitlines()
    new_clang_rev = GetClangRev(new_clang_update_py)
    logging.debug('Found new clang rev: %s' % new_clang_rev)
    clang_change = ChangedDep(CLANG_UPDATE_SCRIPT_LOCAL_PATH, None, old_clang_rev, new_clang_rev)
    return ClangChange(mirror_change, clang_change)


def GenerateCommitMessage(
        rev_update,
        current_commit_pos,
        new_commit_pos,
        changed_deps_list,
        autoroll,
        clang_change,
):
    current_cr_rev = rev_update.current_chromium_rev[0:10]
    new_cr_rev = rev_update.new_chromium_rev[0:10]
    rev_interval = '%s..%s' % (current_cr_rev, new_cr_rev)
    git_number_interval = '%s:%s' % (current_commit_pos, new_commit_pos)

    commit_msg = []
    # Autoroll already adds chromium_revision changes to commit message
    if not autoroll:
        commit_msg.extend([
            'Roll chromium_revision %s (%s)\n' % (rev_interval, git_number_interval),
            'Change log: %s' % (CHROMIUM_LOG_TEMPLATE % rev_interval),
            'Full diff: %s\n' % (CHROMIUM_COMMIT_TEMPLATE % rev_interval)
        ])

    def Section(adjective, deps):
        noun = 'dependency' if len(deps) == 1 else 'dependencies'
        commit_msg.append('%s %s' % (adjective, noun))

    tbr_authors = ''
    if changed_deps_list:
        Section('Changed', changed_deps_list)

        for c in changed_deps_list:
            if isinstance(c, ChangedCipdPackage):
                commit_msg.append('* %s: %s..%s' % (c.path, c.current_version, c.new_version))
            else:
                commit_msg.append('* %s: %s/+log/%s..%s' %
                                  (c.path, c.url, c.current_rev[0:10], c.new_rev[0:10]))

    if changed_deps_list:
        # rev_interval is empty for autoroll, since we are starting from a state
        # in which chromium_revision is already modified in DEPS
        if not autoroll:
            change_url = CHROMIUM_FILE_TEMPLATE % (rev_interval, 'DEPS')
            commit_msg.append('DEPS diff: %s\n' % change_url)
    else:
        commit_msg.append('No dependencies changed.')

    c = clang_change
    if (c and (c.clang_change.current_rev != c.clang_change.new_rev)):
        commit_msg.append('Clang version changed %s:%s' %
                          (c.clang_change.current_rev, c.clang_change.new_rev))

        rev_clang = rev_interval = '%s..%s' % (c.mirror_change.current_rev,
                                               c.mirror_change.new_rev)
        change_url = CLANG_FILE_TEMPLATE % (rev_clang, CLANG_UPDATE_SCRIPT_URL_PATH)
        commit_msg.append('Details: %s\n' % change_url)
    else:
        commit_msg.append('No update to Clang.\n')

    # Autoroll takes care of BUG and TBR in commit message
    if not autoroll:
        # TBR needs to be non-empty for Gerrit to process it.
        git_author = _RunCommand(['git', 'config', 'user.email'],
                                 working_dir=CHECKOUT_SRC_DIR)[0].splitlines()[0]
        tbr_authors = git_author + ',' + tbr_authors

        commit_msg.append('TBR=%s' % tbr_authors)
        commit_msg.append('BUG=None')

    return '\n'.join(commit_msg)


def UpdateDepsFile(deps_filename, rev_update, changed_deps, new_cr_content, autoroll):
    """Update the DEPS file with the new revision."""

    with open(deps_filename, 'rb') as deps_file:
        deps_content = deps_file.read().decode('utf-8')
        # Autoroll takes care of updating 'chromium_revision', thus we don't need to.
        if not autoroll:
            # Update the chromium_revision variable.
            deps_content = deps_content.replace(rev_update.current_chromium_rev,
                                                rev_update.new_chromium_rev)

        # Add and remove dependencies. For now: only generated android deps.
        # Since gclient cannot add or remove deps, we rely on the fact that
        # these android deps are located in one place to copy/paste.
        deps_re = re.compile(ANDROID_DEPS_START + '.*' + ANDROID_DEPS_END, re.DOTALL)
        new_deps = deps_re.search(new_cr_content)
        old_deps = deps_re.search(deps_content)
        if not new_deps or not old_deps:
            faulty = 'Chromium' if not new_deps else 'ANGLE'
            raise RollError('Was expecting to find "%s" and "%s"\n'
                            'in %s DEPS' % (ANDROID_DEPS_START, ANDROID_DEPS_END, faulty))

        replacement = new_deps.group(0).replace('src/third_party/android_deps',
                                                'third_party/android_deps')
        replacement = replacement.replace('checkout_android',
                                          'checkout_android and not build_with_chromium')

        deps_content = deps_re.sub(replacement, deps_content)

        with open(deps_filename, 'wb') as deps_file:
            deps_file.write(deps_content.encode('utf-8'))

    # Update each individual DEPS entry.
    for dep in changed_deps:
        # We don't sync deps on autoroller, so ignore missing local deps
        if not autoroll:
            local_dep_dir = os.path.join(CHECKOUT_ROOT_DIR, dep.path)
            if not os.path.isdir(local_dep_dir):
                raise RollError('Cannot find local directory %s. Either run\n'
                                'gclient sync --deps=all\n'
                                'or make sure the .gclient file for your solution contains all '
                                'platforms in the target_os list, i.e.\n'
                                'target_os = ["android", "unix", "mac", "ios", "win"];\n'
                                'Then run "gclient sync" again.' % local_dep_dir)
        if isinstance(dep, ChangedCipdPackage):
            package = dep.package.format()  # Eliminate double curly brackets
            update = '%s:%s@%s' % (dep.path, package, dep.new_version)
        else:
            update = '%s@%s' % (dep.path, dep.new_rev)
        gclient_cmd = 'gclient'
        if platform.system() == 'Windows':
            gclient_cmd += '.bat'
        _RunCommand([gclient_cmd, 'setdep', '--revision', update], working_dir=CHECKOUT_SRC_DIR)


def _IsTreeClean():
    stdout, _ = _RunCommand(['git', 'status', '--porcelain'])
    if len(stdout) == 0:
        return True

    logging.error('Dirty/unversioned files:\n%s', stdout)
    return False


def _EnsureUpdatedMainBranch(dry_run):
    current_branch = _RunCommand(['git', 'rev-parse', '--abbrev-ref', 'HEAD'])[0].splitlines()[0]
    if current_branch != 'main':
        logging.error('Please checkout the main branch and re-run this script.')
        if not dry_run:
            sys.exit(-1)

    logging.info('Updating main branch...')
    _RunCommand(['git', 'pull'])


def _CreateRollBranch(dry_run):
    logging.info('Creating roll branch: %s', ROLL_BRANCH_NAME)
    if not dry_run:
        _RunCommand(['git', 'checkout', '-b', ROLL_BRANCH_NAME])


def _RemovePreviousRollBranch(dry_run):
    active_branch, branches = _GetBranches()
    if active_branch == ROLL_BRANCH_NAME:
        active_branch = 'main'
    if ROLL_BRANCH_NAME in branches:
        logging.info('Removing previous roll branch (%s)', ROLL_BRANCH_NAME)
        if not dry_run:
            _RunCommand(['git', 'checkout', active_branch])
            _RunCommand(['git', 'branch', '-D', ROLL_BRANCH_NAME])


def _LocalCommit(commit_msg, dry_run):
    logging.info('Committing changes locally.')
    if not dry_run:
        _RunCommand(['git', 'add', '--update', '.'])
        _RunCommand(['git', 'commit', '-m', commit_msg])


def _LocalCommitAmend(commit_msg, dry_run):
    logging.info('Amending changes to local commit.')
    if not dry_run:
        old_commit_msg = _RunCommand(['git', 'log', '-1', '--pretty=%B'])[0].strip()
        logging.debug('Existing commit message:\n%s\n', old_commit_msg)

        bug_index = old_commit_msg.rfind('Bug:')
        if bug_index == -1:
            logging.error('"Bug:" not found in commit message.')
            if not dry_run:
                sys.exit(-1)
        new_commit_msg = old_commit_msg[:bug_index] + commit_msg + '\n' + old_commit_msg[bug_index:]

        _RunCommand(['git', 'commit', '-a', '--amend', '-m', new_commit_msg])


def ChooseCQMode(skip_cq, cq_over, current_commit_pos, new_commit_pos):
    if skip_cq:
        return 0
    if (new_commit_pos - current_commit_pos) < cq_over:
        return 1
    return 2


def _UploadCL(commit_queue_mode):
    """Upload the committed changes as a changelist to Gerrit.

  commit_queue_mode:
    - 2: Submit to commit queue.
    - 1: Run trybots but do not submit to CQ.
    - 0: Skip CQ, upload only.
  """
    cmd = ['git', 'cl', 'upload', '--force', '--bypass-hooks', '--send-mail']
    cmd.extend(['--cc', NOTIFY_EMAIL])
    if commit_queue_mode >= 2:
        logging.info('Sending the CL to the CQ...')
        cmd.extend(['--use-commit-queue'])
    elif commit_queue_mode >= 1:
        logging.info('Starting CQ dry run...')
        cmd.extend(['--cq-dry-run'])
    extra_env = {
        'EDITOR': 'true',
        'SKIP_GCE_AUTH_FOR_GIT': '1',
    }
    stdout, stderr = _RunCommand(cmd, extra_env=extra_env)
    logging.debug('Output from "git cl upload":\nstdout:\n%s\n\nstderr:\n%s', stdout, stderr)


def GetRollRevisionRanges(opts, angle_deps):
    current_cr_rev = angle_deps['vars']['chromium_revision']
    new_cr_rev = opts.revision
    if not new_cr_rev:
        stdout, _ = _RunCommand(['git', 'ls-remote', CHROMIUM_SRC_URL, 'HEAD'])
        head_rev = stdout.strip().split('\t')[0]
        logging.info('No revision specified. Using HEAD: %s', head_rev)
        new_cr_rev = head_rev

    return ChromiumRevisionUpdate(current_cr_rev, new_cr_rev)


def main():
    p = argparse.ArgumentParser()
    p.add_argument(
        '--clean',
        action='store_true',
        default=False,
        help='Removes any previous local roll branch.')
    p.add_argument(
        '-r',
        '--revision',
        help=('Chromium Git revision to roll to. Defaults to the '
              'Chromium HEAD revision if omitted.'))
    p.add_argument(
        '--dry-run',
        action='store_true',
        default=False,
        help=('Calculate changes and modify DEPS, but don\'t create '
              'any local branch, commit, upload CL or send any '
              'tryjobs.'))
    p.add_argument(
        '-i',
        '--ignore-unclean-workdir',
        action='store_true',
        default=False,
        help=('Ignore if the current branch is not main or if there '
              'are uncommitted changes (default: %(default)s).'))
    grp = p.add_mutually_exclusive_group()
    grp.add_argument(
        '--skip-cq',
        action='store_true',
        default=False,
        help='Skip sending the CL to the CQ (default: %(default)s)')
    grp.add_argument(
        '--cq-over',
        type=int,
        default=1,
        help=('Commit queue dry run if the revision difference '
              'is below this number (default: %(default)s)'))
    grp.add_argument(
        '--autoroll',
        action='store_true',
        default=False,
        help='Autoroller mode - amend existing commit, '
        'do not create nor upload a CL (default: %(default)s)')
    p.add_argument(
        '-v',
        '--verbose',
        action='store_true',
        default=False,
        help='Be extra verbose in printing of log messages.')
    opts = p.parse_args()

    if opts.verbose:
        logging.basicConfig(level=logging.DEBUG)
    else:
        logging.basicConfig(level=logging.INFO)

    # We don't have locally sync'ed deps on autoroller,
    # so trust it to have depot_tools in path
    if not opts.autoroll:
        AddDepotToolsToPath()

    if not opts.ignore_unclean_workdir and not _IsTreeClean():
        logging.error('Please clean your local checkout first.')
        return 1

    if opts.clean:
        _RemovePreviousRollBranch(opts.dry_run)

    if not opts.ignore_unclean_workdir:
        _EnsureUpdatedMainBranch(opts.dry_run)

    deps_filename = os.path.join(CHECKOUT_SRC_DIR, 'DEPS')
    angle_deps = ParseLocalDepsFile(deps_filename)

    rev_update = GetRollRevisionRanges(opts, angle_deps)

    current_commit_pos = ParseCommitPosition(ReadRemoteCrCommit(rev_update.current_chromium_rev))
    new_commit_pos = ParseCommitPosition(ReadRemoteCrCommit(rev_update.new_chromium_rev))

    new_cr_content = ReadRemoteCrFile('DEPS', rev_update.new_chromium_rev)
    new_cr_deps = ParseDepsDict(new_cr_content)
    changed_deps = CalculateChangedDeps(angle_deps, new_cr_deps)
    clang_change = CalculateChangedClang(changed_deps, opts.autoroll)
    commit_msg = GenerateCommitMessage(rev_update, current_commit_pos, new_commit_pos,
                                       changed_deps, opts.autoroll, clang_change)
    logging.debug('Commit message:\n%s', commit_msg)

    # We are updating a commit that autoroll has created, using existing branch
    if not opts.autoroll:
        _CreateRollBranch(opts.dry_run)

    if not opts.dry_run:
        UpdateDepsFile(deps_filename, rev_update, changed_deps, new_cr_content, opts.autoroll)

    if opts.autoroll:
        _LocalCommitAmend(commit_msg, opts.dry_run)
    else:
        if _IsTreeClean():
            logging.info("No DEPS changes detected, skipping CL creation.")
        else:
            _LocalCommit(commit_msg, opts.dry_run)
            commit_queue_mode = ChooseCQMode(opts.skip_cq, opts.cq_over, current_commit_pos,
                                             new_commit_pos)
            logging.info('Uploading CL...')
            if not opts.dry_run:
                _UploadCL(commit_queue_mode)
    return 0


if __name__ == '__main__':
    sys.exit(main())
