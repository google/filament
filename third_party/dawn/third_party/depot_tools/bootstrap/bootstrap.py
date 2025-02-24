# Copyright 2016 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import argparse
import collections
import contextlib
import fnmatch
import hashlib
import logging
import os
import platform
import posixpath
import shutil
import string
import subprocess
import sys
import tempfile

THIS_DIR = os.path.abspath(os.path.dirname(__file__))
ROOT_DIR = os.path.abspath(os.path.join(THIS_DIR, '..'))

DEVNULL = open(os.devnull, 'w')

IS_WIN = sys.platform.startswith('win')
BAT_EXT = '.bat' if IS_WIN else ''

# Top-level stubs to generate that fall through to executables within the Git
# directory.
WIN_GIT_STUBS = {
    'git.bat': 'cmd\\git.exe',
    'gitk.bat': 'cmd\\gitk.exe',
    'ssh.bat': 'usr\\bin\\ssh.exe',
    'ssh-keygen.bat': 'usr\\bin\\ssh-keygen.exe',
}

# The global git config which should be applied by |git_postprocess|.
GIT_GLOBAL_CONFIG = {
    'core.autocrlf': 'false',
    'core.filemode': 'false',
    'core.preloadindex': 'true',
    'core.fscache': 'true',
}


# Accumulated template parameters for generated stubs.
class Template(
        collections.namedtuple('Template', (
            'PYTHON3_BIN_RELDIR',
            'PYTHON3_BIN_RELDIR_UNIX',
            'GIT_BIN_ABSDIR',
            'GIT_PROGRAM',
        ))):
    @classmethod
    def empty(cls):
        return cls(**{k: None for k in cls._fields})

    def maybe_install(self, name, dst_path):
        """Installs template |name| to |dst_path| if it has changed.

        This loads the template |name| from THIS_DIR, resolves template parameters,
        and installs it to |dst_path|. See `maybe_update` for more information.

        Args:
            name (str): The name of the template to install.
            dst_path (str): The destination filesystem path.

        Returns (bool): True if |dst_path| was updated, False otherwise.
        """
        template_path = os.path.join(THIS_DIR, name)
        with open(template_path, 'r', encoding='utf8') as fd:
            t = string.Template(fd.read())
        return maybe_update(t.safe_substitute(self._asdict()), dst_path)


def maybe_update(content, dst_path):
    """Writes |content| to |dst_path| if |dst_path| does not already match.

    This function will ensure that there is a file at |dst_path| containing
    |content|. If |dst_path| already exists and contains |content|, no operation
    will be performed, preserving filesystem modification times and avoiding
    potential write contention.

    Args:
        content (str): The file content.
        dst_path (str): The destination filesystem path.

    Returns (bool): True if |dst_path| was updated, False otherwise.
    """
    # If the path already exists and matches the new content, refrain from
    # writing a new one.
    if os.path.exists(dst_path):
        with open(dst_path, 'r', encoding='utf-8') as fd:
            if fd.read() == content:
                return False

    logging.debug('Updating %r', dst_path)
    with open(dst_path, 'w', encoding='utf-8') as fd:
        fd.write(content)
    os.chmod(dst_path, 0o755)
    return True


def maybe_copy(src_path, dst_path):
    """Writes the content of |src_path| to |dst_path| if needed.

    See `maybe_update` for more information.

    Args:
        src_path (str): The content source filesystem path.
        dst_path (str): The destination filesystem path.

    Returns (bool): True if |dst_path| was updated, False otherwise.
    """
    with open(src_path, 'r', encoding='utf-8') as fd:
        content = fd.read()
    return maybe_update(content, dst_path)


def call_if_outdated(stamp_path, stamp_version, fn):
    """Invokes |fn| if the stamp at |stamp_path| doesn't match |stamp_version|.

    This can be used to keep a filesystem record of whether an operation has been
    performed. The record is stored at |stamp_path|. To invalidate a record,
    change the value of |stamp_version|.

    After |fn| completes successfully, |stamp_path| will be updated to match
    |stamp_version|, preventing the same update from happening in the future.

    Args:
        stamp_path (str): The filesystem path of the stamp file.
        stamp_version (str): The desired stamp version.
        fn (callable): A callable to invoke if the current stamp version doesn't
            match |stamp_version|.

    Returns (bool): True if an update occurred.
    """

    stamp_version = stamp_version.strip()
    if os.path.isfile(stamp_path):
        with open(stamp_path, 'r', encoding='utf-8') as fd:
            current_version = fd.read().strip()
        if current_version == stamp_version:
            return False

    fn()

    with open(stamp_path, 'w', encoding='utf-8') as fd:
        fd.write(stamp_version)
    return True


def _in_use(path):
    """Checks if a Windows file is in use.

    When Windows is using an executable, it prevents other writers from
    modifying or deleting that executable. We can safely test for an in-use
    file by opening it in write mode and checking whether or not there was
    an error.

    Returns (bool): True if the file was in use, False if not.
    """
    try:
        with open(path, 'r+'):
            return False
    except IOError:
        return True


def _toolchain_in_use(toolchain_path):
    """Returns (bool): True if a toolchain rooted at |path| is in use.
    """
    # Look for Python files that may be in use.
    for python_dir in (
            os.path.join(toolchain_path, 'python', 'bin'),  # CIPD
            toolchain_path,  # Legacy ZIP distributions.
    ):
        for component in (
                os.path.join(python_dir, 'python.exe'),
                os.path.join(python_dir, 'DLLs', 'unicodedata.pyd'),
        ):
            if os.path.isfile(component) and _in_use(component):
                return True
    # Look for Pytho:n 3 files that may be in use.
    python_dir = os.path.join(toolchain_path, 'python3', 'bin')
    for component in (
            os.path.join(python_dir, 'python3.exe'),
            os.path.join(python_dir, 'DLLs', 'unicodedata.pyd'),
    ):
        if os.path.isfile(component) and _in_use(component):
            return True
    return False


def _check_call(argv, stdin_input=None, **kwargs):
    """Wrapper for subprocess.Popen that adds logging."""
    logging.info('running %r', argv)
    if stdin_input is not None:
        kwargs['stdin'] = subprocess.PIPE
    proc = subprocess.Popen(argv, **kwargs)
    stdout, stderr = proc.communicate(input=stdin_input)
    if proc.returncode:
        raise subprocess.CalledProcessError(proc.returncode, argv, None)
    return stdout, stderr


def _safe_rmtree(path):
    if not os.path.exists(path):
        return

    def _make_writable_and_remove(path):
        st = os.stat(path)
        new_mode = st.st_mode | 0o200
        if st.st_mode == new_mode:
            return False
        try:
            os.chmod(path, new_mode)
            os.remove(path)
            return True
        except Exception:
            return False

    def _on_error(function, path, excinfo):
        if not _make_writable_and_remove(path):
            logging.warning('Failed to %s: %s (%s)', function, path, excinfo)

    shutil.rmtree(path, onerror=_on_error)


def clean_up_old_installations(skip_dir):
    """Removes Python installations other than |skip_dir|.

    This includes an "in-use" check against the "python.exe" in a given
    directory to avoid removing Python executables that are currently running.
    We need this because our Python bootstrap may be run after (and by) other
    software that is using the bootstrapped Python!
    """
    root_contents = os.listdir(ROOT_DIR)
    for f in ('win_tools-*_bin', 'python27*_bin', 'git-*_bin',
              'bootstrap-*_bin'):
        for entry in fnmatch.filter(root_contents, f):
            full_entry = os.path.join(ROOT_DIR, entry)
            if full_entry == skip_dir or not os.path.isdir(full_entry):
                continue

            logging.info('Cleaning up old installation %r', entry)
            if not _toolchain_in_use(full_entry):
                _safe_rmtree(full_entry)
            else:
                logging.info('Toolchain at %r is in-use; skipping', full_entry)


def _within_depot_tools(path):
    """Returns whether the given path is within depot_tools."""
    try:
        return os.path.commonpath([os.path.abspath(path), ROOT_DIR]) == ROOT_DIR
    except ValueError:
        return False


def _traverse_to_git_root(abspath):
    """Traverses up the path to the closest "git" directory (case-insensitive).

    Returns:
        The path to the directory with name "git" (case-insensitive), if it
        exists as an ancestor; otherwise, None.

    Examples:
      * "C:\Program Files\Git\cmd" -> "C:\Program Files\Git"
      * "C:\Program Files\Git\mingw64\bin" -> "C:\Program Files\Git"
    """
    head, tail = os.path.split(abspath)
    while tail:
        if tail.lower() == 'git':
            return os.path.join(head, tail)
        head, tail = os.path.split(head)
    return None


def search_win_git_directory():
    """Searches for a git directory outside of depot_tools.

    As depot_tools will soon stop bundling Git for Windows, this function logs
    a warning if git has not yet been directly installed.
    """
    # Look for the git command in PATH outside of depot_tools.
    for p in os.environ.get('PATH', '').split(os.pathsep):
        if _within_depot_tools(p):
            continue

        for cmd in ('git.exe', 'git.bat'):
            if os.path.isfile(os.path.join(p, cmd)):
                git_root = _traverse_to_git_root(p)
                if git_root:
                    return git_root

    # Log deprecation warning.
    logging.warning(
        'depot_tools will stop bundling Git for Windows on 2025-01-27.\n'
        'To prepare for this change, please install Git directly. See\n'
        'https://chromium.googlesource.com/chromium/src/+/main/docs/windows_build_instructions.md#Install-git\n'
        '\n'
        'Having issues and not ready for depot_tools to stop bundling\n'
        'Git for Windows? File a bug at:\n'
        'https://issues.chromium.org/issues/new?component=1456702&template=2045785\n'
    )
    return None


def git_get_mingw_dir(git_directory):
    """Returns (str) The "mingw" directory in a Git installation, or None."""
    for candidate in ('mingw64', 'mingw32'):
        mingw_dir = os.path.join(git_directory, candidate)
        if os.path.isdir(mingw_dir):
            return mingw_dir
    return None


class GitConfigDict(collections.UserDict):
    """Custom dict to support mixed case sensitivity for Git config options.

    See the docs at: https://git-scm.com/docs/git-config#_syntax
    """

    @staticmethod
    def _to_case_compliant_key(config_key):
        parts = config_key.split('.')
        if len(parts) < 2:
            # The config key does not conform to the expected format.
            # Leave as-is.
            return config_key

        # Section headers are case-insensitive; set to lowercase for lookup
        # consistency.
        section = parts[0].lower()
        # Subsection headers are case-sensitive and allow '.'.
        subsection_parts = parts[1:-1]
        # Variable names are case-insensitive; again, set to lowercase for
        # lookup consistency.
        name = parts[-1].lower()

        return '.'.join([section] + subsection_parts + [name])

    def __setitem__(self, key, value):
        self.data[self._to_case_compliant_key(key)] = value

    def __getitem__(self, key):
        return self.data[self._to_case_compliant_key(key)]


def get_git_global_config(git_path):
    """Helper to get all values from the global git config.

    Note: multivalued config variables (multivars) are not supported; only the
    last value for each multivar will be in the returned config.

    Returns:
      - GitConfigDict of the current global git config.
      - If there was an error reading the global git config (e.g. file doesn't
        exist, or is an invalid config), returns an empty GitConfigDict.
    """
    try:
        # List all values in the global git config. Using the `-z` option allows
        # us to securely parse the output even if a value contains line breaks.
        # See docs at:
        # https://git-scm.com/docs/git-config#Documentation/git-config.txt--z
        stdout, _ = _check_call(
            [git_path, 'config', '--list', '--global', '-z'],
            stdout=subprocess.PIPE,
            encoding='utf-8')
    except subprocess.CalledProcessError as e:
        logging.warning(f'Failed to read your global Git config:\n{e}\n')
        return GitConfigDict({})

    # Process all entries in the config.
    config = {}
    for line in stdout.split('\0'):
        entry = line.split('\n', 1)
        if len(entry) != 2:
            continue
        config[entry[0]] = entry[1]

    return GitConfigDict(config)


def _win_git_bootstrap_config():
    """Bootstraps the global git config, if enabled by the user.

    To allow depot_tools to update your global git config, run:
        git config --global depot-tools.allowGlobalGitConfig true

    To prevent depot_tools updating your global git config and silence the
    warning, run:
        git config --global depot-tools.allowGlobalGitConfig false
    """
    git_bat_path = os.path.join(ROOT_DIR, 'git.bat')

    # Read the current global git config in its entirety.
    current_config = get_git_global_config(git_bat_path)

    # Get the current values for the settings which have defined values for
    # optimal Chromium development.
    config_keys = sorted(GIT_GLOBAL_CONFIG.keys())
    mismatching_keys = []
    for k in config_keys:
        if current_config.get(k) != GIT_GLOBAL_CONFIG.get(k):
            mismatching_keys.append(k)
    if not mismatching_keys:
        # Global git config already has the desired values. Nothing to do.
        return

    # Check whether the user has authorized depot_tools to update their global
    # git config.
    allow_global_key = 'depot-tools.allowGlobalGitConfig'
    allow_global = current_config.get(allow_global_key, '').lower()

    if allow_global in ('false', '0', 'no', 'off'):
        # The user has explicitly disabled this.
        return

    if allow_global not in ('true', '1', 'yes', 'on'):
        lines = [
            'depot_tools recommends setting the following for',
            'optimal Chromium development:',
            '',
        ] + [
            f'$ git config --global {k} {GIT_GLOBAL_CONFIG.get(k)}'
            for k in mismatching_keys
        ] + [
            '',
            'You can silence this message by setting these recommended values.',
            '',
            'You can allow depot_tools to automatically update your global',
            'Git config to recommended settings by running:',
            f'$ git config --global {allow_global_key} true',
            '',
            'To suppress this warning and silence future recommendations, run:',
            f'$ git config --global {allow_global_key} false',
        ]

        logging.warning('\n'.join(lines))
        return

    # Global git config changes have been authorized - do the necessary updates.
    for k in mismatching_keys:
        desired = GIT_GLOBAL_CONFIG.get(k)
        _check_call([git_bat_path, 'config', '--global', k, desired])

    # Clean up deprecated setting depot-tools.gitPostprocessVersion.
    postprocess_key = 'depot-tools.gitPostprocessVersion'
    if current_config.get(postprocess_key) != None:
        _check_call(
            [git_bat_path, 'config', '--unset', '--global', postprocess_key])


def git_postprocess(template):
    # Create Git templates and configure its base layout.
    for stub_name, relpath in WIN_GIT_STUBS.items():
        stub_template = template._replace(GIT_PROGRAM=relpath)
        stub_template.maybe_install('git.template.bat',
                                    os.path.join(ROOT_DIR, stub_name))

    # Bootstrap the git global config.
    _win_git_bootstrap_config()


def main(argv):
    parser = argparse.ArgumentParser()
    parser.add_argument('--verbose', action='store_true')
    parser.add_argument('--bootstrap-name',
                        required=True,
                        help='The directory of the Python installation.')
    args = parser.parse_args(argv)

    logging.basicConfig(level=logging.DEBUG if args.verbose else logging.WARN)

    template = Template.empty()._replace(
        PYTHON3_BIN_RELDIR=os.path.join(args.bootstrap_name, 'python3', 'bin'),
        PYTHON3_BIN_RELDIR_UNIX=posixpath.join(args.bootstrap_name, 'python3',
                                               'bin'))

    bootstrap_dir = os.path.join(ROOT_DIR, args.bootstrap_name)

    # Clean up any old Python and Git installations.
    clean_up_old_installations(bootstrap_dir)

    if IS_WIN:
        # Search for a Git installation.
        git_dir = search_win_git_directory()
        if not git_dir:
            logging.error('Failed to bootstrap depot_tools.\n'
                          'Git was not found in PATH. Have you installed it?')
            return 1

        template = template._replace(GIT_BIN_ABSDIR=git_dir)
        git_postprocess(template)
        templates = [
            ('git-bash.template.sh', 'git-bash', ROOT_DIR),
            ('python3.bat', 'python3.bat', ROOT_DIR),
        ]
        for src_name, dst_name, dst_dir in templates:
            # Re-evaluate and regenerate our root templated files.
            template.maybe_install(src_name, os.path.join(dst_dir, dst_name))

    # Emit our Python bin depot-tools-relative directory. This is read by
    # python3.bat to identify the path of the current Python installation.
    #
    # We use this indirection so that upgrades can change this pointer to
    # redirect "python.bat" to a new Python installation. We can't just update
    # "python.bat" because batch file executions reload the batch file and seek
    # to the previous cursor in between every command, so changing the batch
    # file contents could invalidate any existing executions.
    #
    # The intention is that the batch file itself never needs to change when
    # switching Python versions.

    maybe_update(template.PYTHON3_BIN_RELDIR,
                 os.path.join(ROOT_DIR, 'python3_bin_reldir.txt'))

    return 0


if __name__ == '__main__':
    sys.exit(main(sys.argv[1:]))
