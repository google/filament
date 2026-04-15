#!/usr/bin/env vpython3
# Copyright 2025 The Dawn & Tint Authors
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are met:
#
# 1. Redistributions of source code must retain the above copyright notice, this
#    list of conditions and the following disclaimer.
#
# 2. Redistributions in binary form must reproduce the above copyright notice,
#    this list of conditions and the following disclaimer in the documentation
#    and/or other materials provided with the distribution.
#
# 3. Neither the name of the copyright holder nor the names of its
#    contributors may be used to endorse or promote products derived from
#    this software without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
# AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
# DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
# FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
# DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
# SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
# CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
# OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
# OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
"""Rolls dependencies shared with Chromium.

This is mostly DEPS entries, but some additional dependencies elsewhere in the
repo are also synced.
"""

import abc
import argparse
import base64
import dataclasses
import datetime
import functools
import logging
import pathlib
import posixpath
import re
import shlex
import subprocess
import sys
from typing import Any, Self, Type

import requests

DAWN_ROOT = pathlib.Path(__file__).resolve().parents[1]
DEPS_FILE = DAWN_ROOT / 'DEPS'
INFRA_PATH = DAWN_ROOT / 'infra' / 'config' / 'global'
PACKAGE_STAR = INFRA_PATH / 'PACKAGE.star'

CHROMIUM_GOB_URL = 'https://chromium.googlesource.com'
CHROMIUM_SRC_URL = posixpath.join(CHROMIUM_GOB_URL, 'chromium', 'src')
CHROMIUM_REVISION_VAR = 'chromium_revision'

DEFAULT_REVISION_CHARACTERS = 10

# GN variables that need to be synced. A map from Dawn variable name to
# Chromium variable name.
SYNCED_VARIABLES = {}

# DEPS entries which have dep_type = cipd. In the Chromium DEPS file, these
# will be prefixed with src/.
SYNCED_CIPD_DEPS = {
    'buildtools/linux64',
    'buildtools/mac',
    'buildtools/reclient',
    'buildtools/win',
    'third_party/ninja',
    'third_party/siso/cipd',
}

# DEPS entries which have dep_type = gcs. In the Chromium DEPS file, these will
# be prefixed with src/.
SYNCED_GCS_DEPS = {
    'build/linux/debian_bullseye_arm64-sysroot',
    'build/linux/debian_bullseye_armhf-sysroot',
    'build/linux/debian_bullseye_i386-sysroot',
    'build/linux/debian_bullseye_mipsel-sysroot',
    'build/linux/debian_bullseye_mips64el-sysroot',
    'build/linux/debian_bullseye_amd64-sysroot',
}

# Repos that are independently synced by Chromium and Dawn. A map from Dawn
# names to Chromium names. None means that the names are identical. In the
# Chromium DEPS file, these will be prefixed with src/.
# The following repos are already synced by dedicated autorollers:
# * https://autoroll.skia.org/r/angle-dawn-autoroll
#   * third_party/angle
# * https://autoroll.skia.org/r/swiftshader-dawn-autoroll
#   * third_party/swiftshader
# * https://autoroll.skia.org/r/vulkan-deps-dawn-autoroll
#   * third_party/glslang/src
#   * third_party/spirv-headers/src
#   * third_party/spirv-tools/src
#   * third_party/vulkan-deps
#   * third_party/vulkan-headers/src
#   * third_party/vulkan-loader/src
#   * third_party/vulkan-tools/src
#   * third_party/vulkan-utility-libraries/src
#   * third_party/vulkan-validation-layers/src
SYNCED_REPOS = {
    'third_party/catapult': None,
    'third_party/clang-format/script': None,
    'third_party/depot_tools': None,
    # third_party/directx-headers/src is technically used by both Chromium and
    # Dawn, but for different purposes and on non-overlapping platforms. Thus,
    # there is no need to sync their revisions.
    'third_party/google_benchmark/src': None,
    'third_party/googletest': 'third_party/googletest/src',
    'third_party/jsoncpp': 'third_party/jsoncpp/source',
    'third_party/libc++/src': None,
    'third_party/libc++abi/src': None,
    'third_party/libprotobuf-mutator/src': None,
    'third_party/llvm-libc/src': None,
    'third_party/libdrm/src': None,
    'third_party/libFuzzer/src': None,
    # third_party/vulkan_memory_allocator is shared with Chromium, but is
    # manually rolled since it typically requires additional code changes in
    # the repo.
    # third_party/webgpu-cts is technically used by both Chromium and Dawn, but
    # they are used for different purposes and the CTS roller needs to roll
    # Dawn's copy in order to update expectations.
}

# Chromium directories that are exported as pseudo-repos in
# chromium.googlesource.com under chromium/src/. Mapping of Dawn path to
# Chromium src-relative path. None means that the names are identical.
EXPORTED_CHROMIUM_REPOS = {
    'build': None,
    'buildtools': None,
    'testing': None,
    'third_party/abseil-cpp': None,
    'third_party/jinja2': None,
    'third_party/markupsafe': None,
    'third_party/partition_alloc': 'base/allocator/partition_allocator',
    'third_party/protobuf': None,
    'third_party/zlib': None,
    'tools/clang': None,
    'tools/mb': None,
    'tools/memory': None,
    'tools/protoc_wrapper': None,
    'tools/valgrind': None,
    'tools/win': None,
}


@dataclasses.dataclass
class ChangedDepsEntry(abc.ABC):
    """Base class for all changed DEPS entries."""
    # The name of the dependency in Dawn's DEPS file.
    name: str

    @abc.abstractmethod
    def setdep_args(self) -> list[str]:
        """Returns 'gclient setdep'-compatible arguments.

        The returned arguments will cause 'gclient setdep' to update the DEPS
        file content to the new version.
        """

    @abc.abstractmethod
    def commit_message_lines(self) -> list[str]:
        """Returns lines to add to the commit message."""


@dataclasses.dataclass
class ChangedVariable(ChangedDepsEntry):
    """Represents a single changed DEPS variable."""
    # The old version in Dawn's DEPS file.
    old_version: str
    # The new version that Dawn's DEPS file will contain.
    new_version: str

    def setdep_args(self) -> list[str]:
        return [
            '--var',
            f'{self.name}={self.new_version}',
        ]

    def commit_message_lines(self) -> list[str]:
        return [
            f'  {self.name}: {self.old_version} -> {self.new_version}',
        ]


@dataclasses.dataclass
class ChangedRepo(ChangedDepsEntry):
    """Represents a single changed DEPS repo entry."""
    # The URL of the repo that Dawn depends on.
    url: str
    # The old revision in Dawn's DEPS file.
    old_revision: str
    # The new revision that Dawn's DEPS file will contain.
    new_revision: str

    def setdep_args(self) -> list[str]:
        return [
            '--revision',
            f'{self.name}@{self.new_revision}',
        ]

    def commit_message_lines(self) -> list[str]:
        return [f'  {self.name}: {self.log_link()}']

    def revision_range(self, num_characters: int | None = None) -> str:
        num_characters = num_characters or DEFAULT_REVISION_CHARACTERS
        return (f'{self.old_revision[:num_characters]}'
                f'..'
                f'{self.new_revision[:num_characters]}')

    def log_link(self, num_characters: int | None = None) -> str:
        return f'{self.url}/+log/{self.revision_range(num_characters)}'

    def commit_link(self, num_characters: int | None = None) -> str:
        return f'{self.url}/+/{self.revision_range(num_characters)}'


@dataclasses.dataclass
class CipdPackage:
    """Represents a single element of a CIPD DEPS entry's 'packages' list."""
    package: str
    version: str

    @classmethod
    def from_dict(cls, dict_repr: dict[str, str]) -> Self:
        """Creates an instance from a DEPS entry dict.

        Args:
            dict_repr: A dictionary from a GCS DEPS entry's 'packages' list.

        Returns:
            A CipdPackage instance filled with the information contained within
            |dict_repr|.
        """
        return cls(
            package=dict_repr['package'],
            version=dict_repr['version'],
        )

    def setdep_str(self) -> str:
        """Gets a 'gclient setdep'-compatible string representation."""
        # Remove any double curly braces, e.g. ${{arch}}
        package = self.package.format()
        return f'{package}@{self.version}'


@dataclasses.dataclass
class ChangedCipd(ChangedDepsEntry):
    """Represents a single changed DEPS CIPD entry."""
    # The old packages in Dawn's DEPS file
    old_packages: list[CipdPackage]
    # The new packages that Dawn's DEPS file will contain.
    new_packages: list[CipdPackage]

    def setdep_args(self) -> list[str]:
        revisions = [
            f'{self.name}:{p.setdep_str()}' for p in self.new_packages
        ]
        return ['--revision'] + revisions

    def commit_message_lines(self) -> list[str]:
        return [
            f'  {self.name}',
        ]


@dataclasses.dataclass
class GcsObject:
    """Represents a single element of a GCS DEPS entry's 'objects' list."""
    object_name: str
    sha256sum: str
    size_bytes: int
    generation: int

    @classmethod
    def from_dict(cls, dict_repr: dict[str, str | int]) -> Self:
        """Creates an instance from a DEPS entry dict.

        Args:
            dict_repr: A dictionary from a GCS DEPS entry's 'objects' list.

        Returns:
            A GcsObject instance filled with the information contained within
            |dict_repr|.
        """
        return cls(
            object_name=dict_repr['object_name'],
            sha256sum=dict_repr['sha256sum'],
            size_bytes=dict_repr['size_bytes'],
            generation=dict_repr['generation'],
        )

    def as_comma_separated_str(self) -> str:
        return (f'{self.object_name},{self.sha256sum},{self.size_bytes},'
                f'{self.generation}')


@dataclasses.dataclass
class ChangedGcs(ChangedDepsEntry):
    """Represents a single changed DEPS GCS entry."""
    # The old objects in Dawn's DEPS file.
    old_objects: list[GcsObject]
    # The new objects that Dawn's DEPS file will contain.
    new_objects: list[GcsObject]

    def setdep_args(self) -> list[str]:
        comma_separated_objects = [
            o.as_comma_separated_str() for o in self.new_objects
        ]
        object_string = '?'.join(comma_separated_objects)
        return ['--revision', f'{self.name}@{object_string}']

    def commit_message_lines(self) -> list[str]:
        return [
            f'  {self.name}',
        ]


def _parse_deps_file(deps_content: str) -> dict[str, Any]:
    """Parses DEPS file content into a Python dict.

    Args:
        deps_content: The content of a DEPS file.

    Returns:
        A dict containing all content of the DEPS file. For example, the top
        level `vars` mapping in a DEPS file can be accessed via the 'vars' item
        in the returned dictionary.
    """
    local_scope = {}
    global_scope = {
        'Str': lambda str_value: str_value,
        'Var': lambda var_name: local_scope['vars'][var_name],
    }
    exec(deps_content, global_scope, local_scope)
    return local_scope


def _add_depot_tools_to_path() -> None:
    sys.path.append(str(DAWN_ROOT / 'build'))
    import find_depot_tools
    find_depot_tools.add_depot_tools_to_path()


def _is_tree_clean() -> bool:
    """Checks for untracked/uncommitted files.

    Returns:
        True iff there are no untracked or uncommitted files.
    """
    proc = subprocess.run(['git', 'status', '--porcelain'],
                          capture_output=True,
                          text=True,
                          check=True)
    if not proc.stdout:
        return True

    logging.error('Dirty or untracked files:\n%s', proc.stdout)
    return False


def _ensure_updated_main_branch() -> None:
    """Ensures that the main branch is checked out and up to date."""
    proc = subprocess.run(['git', 'rev-parse', '--abbrev-ref', 'HEAD'],
                          capture_output=True,
                          text=True,
                          check=True)
    current_branch = proc.stdout.splitlines()[0]
    if current_branch != 'main':
        raise RuntimeError('Please run this from the main branch')

    logging.info('Updating main branch...')
    subprocess.run(['git', 'pull'], check=True)


def _create_roll_branch() -> None:
    """Creates a unique branch for a roll."""
    # YYYY-MM-DD-HH-MM-SS
    now = datetime.datetime.now().strftime('%Y-%m-%d-%H-%M-%S')
    branch_name = f'roll-chromium-deps-{now}'
    logging.info('Creating branch %s', branch_name)
    _ = subprocess.run(['git', 'checkout', '-b', branch_name], check=True)


def _amend_commit(commit_message: str) -> None:
    """Amends the most recent commit.

    Args:
        commit_message: The commit message to add to the existing commit
            message. This will be inserted towards the end of the existing
            message, just before "Bug:".
    """
    logging.info('Amending changes to local commit')

    proc = subprocess.run(['git', 'log', '-1', '--pretty=%B'],
                          capture_output=True,
                          text=True,
                          check=True)
    old_commit_message = proc.stdout.strip()
    logging.debug('Existing commit message:\n%s', old_commit_message)

    bug_index = old_commit_message.rfind('Bug:')
    if bug_index == -1:
        raise RuntimeError('"Bug:" not found in existing commit message.')

    new_commit_message = (f'{old_commit_message[:bug_index]}'
                          f'{commit_message}'
                          f'\n'
                          f'{old_commit_message[bug_index:]}')

    _ = subprocess.run(
        ['git', 'commit', '-a', '--amend', '-m', new_commit_message],
        capture_output=True,
        check=True)


def _commit(commit_message: str) -> None:
    """Commits all changed files.

    Args:
        commit_message: The commit message to use for the new commit.
    """
    # `gclient setdep` should have already staged changes and running
    # `git add -u` actually undoes the submodule changes, so commit as-is.
    _ = subprocess.run(['git', 'commit', '-m', commit_message],
                       capture_output=True,
                       check=True)


def _get_remote_head_revision(remote_url: str) -> str:
    """Retrieves the HEAD revision for a remote git URL.

    Args:
        remote_url: The remote git URL to get HEAD from.

    Returns:
        The revision currently corresponding to HEAD.
    """
    cmd = [
        'git',
        'ls-remote',
        '--branches',
        remote_url,
        # main and HEAD should be equivalent in this case and main allows usage
        # of --branches, which significantly speeds this up.
        'main',
    ]
    proc = subprocess.run(cmd, check=True, capture_output=True, text=True)
    head_revision = proc.stdout.strip().split()[0]
    return head_revision


def _get_roll_revision_range(target_revision: str | None,
                             dawn_deps: dict) -> ChangedRepo:
    """Determines the range being rolled.

    Args:
        target_revision: The revision being targeted for the roll. If not
            specified, the HEAD revision will be determined and used.
        dawn_deps: The parsed contents of the Dawn DEPS file.

    Returns:
        A ChangedRepo with the determined revision range.
    """
    old_revision = dawn_deps['vars'][CHROMIUM_REVISION_VAR]
    new_revision = target_revision
    if not new_revision:
        new_revision = _get_remote_head_revision(CHROMIUM_SRC_URL)
        logging.info('Using %s as the HEAD revision.', new_revision)
    return ChangedRepo(name='chromium/src',
                       url=CHROMIUM_SRC_URL,
                       old_revision=old_revision,
                       new_revision=new_revision)


def _read_gitiles_content(file_url: str) -> str:
    """Reads the contents of a file from Gitiles.

    Args:
        file_url: A URL pointing to a file to read from Gitiles.

    Returns:
        The string content of the specified file.
    """
    file_url = file_url + '?format=TEXT'
    r = requests.get(file_url)
    r.raise_for_status()
    return base64.b64decode(r.text).decode('utf-8')


def _read_remote_chromium_file(src_relative_path: str, revision: str) -> str:
    """Reads the contents of a Chromium file from Gitiles.

    Args:
        src_relative_path: A POSIX path to the file to read relative to
            chromium/src.
        revision: The Chromium revision to read the file contents at.
    """
    file_url = posixpath.join(CHROMIUM_SRC_URL, '+', revision,
                              src_relative_path)
    return _read_gitiles_content(file_url)


def _get_changed_deps_entries(dawn_deps: dict,
                              chromium_deps: dict) -> list[ChangedDepsEntry]:
    """Gets all entries that have changed between the two provided DEPS.

    Args:
        dawn_deps: The parsed content of the Dawn DEPS file.
        chromium_deps: The parsed content of the Chromium DEPS file.

    Returns:
        A list ChangedDepsEntry objects, each one corresponding to a change
        between the two DEPS files.
    """
    changed_entries = []
    changed_entries.extend(_get_changed_variables(dawn_deps, chromium_deps))
    changed_entries.extend(_get_changed_cipd(dawn_deps, chromium_deps))
    changed_entries.extend(_get_changed_gcs(dawn_deps, chromium_deps))
    changed_entries.extend(
        _get_changed_non_exported_repos(dawn_deps, chromium_deps))
    changed_entries.extend(_get_changed_exported_repos(dawn_deps))
    return changed_entries


def _get_changed_variables(dawn_deps: dict,
                           chromium_deps: dict) -> list[ChangedVariable]:
    """Gets all GN variables that have changed between the two provided DEPS.

    Args:
        dawn_deps: The parsed content of the Dawn DEPS file.
        chromium_deps: The parsed content of the Chromium DEPS file.

    Returns:
        A list of all variable entries that have changed between the two DEPS
        files.
    """
    changed_variables = []
    for dawn_var, chromium_var in SYNCED_VARIABLES.items():
        dawn_value = dawn_deps['vars'].get(dawn_var)
        chromium_value = chromium_deps['vars'].get(chromium_var)
        if not dawn_value:
            raise RuntimeError(
                f'Could not find Dawn GN variable {dawn_var}. Was it removed?')
        if not chromium_value:
            raise RuntimeError(
                f'Could not find Chromium GN variable {chromium_var}. Was it '
                f'removed?')
        changed_variables.append(
            ChangedVariable(
                name=dawn_var,
                old_version=dawn_value,
                new_version=chromium_value,
            ))
    return changed_variables


def _get_changed_cipd(dawn_deps: dict,
                      chromium_deps: dict) -> list[ChangedCipd]:
    """Gets all CIPD entries that have changed between the two provided DEPS.

    Args:
        dawn_deps: The parsed content of the Dawn DEPS file.
        chromium_deps: The parsed content of the Chromium DEPS file.

    Returns:
        A list of all CIPD DEPS entries that have changed between the two
        DEPS files.
    """
    changed_cipd = []
    for dawn_name in SYNCED_CIPD_DEPS:
        chromium_name = 'src/' + dawn_name
        if dawn_name not in dawn_deps['deps']:
            raise RuntimeError(
                f'Unable to find Dawn CIPD entry {dawn_name}. Was it removed?')
        if chromium_name not in chromium_deps['deps']:
            raise RuntimeError(
                f'Unable to find Chromium CIPD entry {chromium_name}. Was it '
                f'removed?')

        dawn_packages = [
            CipdPackage.from_dict(p)
            for p in dawn_deps['deps'][dawn_name]['packages']
        ]
        chromium_packages = [
            CipdPackage.from_dict(p)
            for p in chromium_deps['deps'][chromium_name]['packages']
        ]
        # Unlike GCS entries which provide all object content with a single
        # --revision, CIPD entries provide one package to update per
        # --revision. The behavior when a package within an entry disappears is
        # not clearly defined, so just fail if we ever see that. This should
        # rarely happen, though.
        dawn_package_names = set(p.package for p in dawn_packages)
        chromium_package_names = set(p.package for p in chromium_packages)
        if not dawn_package_names.issubset(chromium_package_names):
            raise RuntimeError(
                f'Packages for CIPD entry {dawn_name} appear to have changed. '
                f'Please manually sync the package list.')
        if dawn_packages != chromium_packages:
            changed_cipd.append(
                ChangedCipd(
                    name=dawn_name,
                    old_packages=dawn_packages,
                    new_packages=chromium_packages,
                ))
    return changed_cipd


def _get_changed_gcs(dawn_deps: dict, chromium_deps: dict) -> list[ChangedGcs]:
    """Gets all GCS entries that have changed between the two provided DEPS.

    Args:
        dawn_deps: The parsed content of the Dawn DEPS file.
        chromium_deps: The parsed content of the Chromium DEPS file.

    Returns:
        A list of all GCS DEPS entries that have changed between the two DEPS
        files.
    """
    changed_gcs = []
    for dawn_name in SYNCED_GCS_DEPS:
        chromium_name = 'src/' + dawn_name
        if dawn_name not in dawn_deps['deps']:
            raise RuntimeError(
                f'Unable to find Dawn GCS entry {dawn_name}. Was it removed?')
        if chromium_name not in chromium_deps['deps']:
            raise RuntimeError(
                f'Unable to find Chromium GCS entry {chromium_name}. Was it '
                f'removed?')

        dawn_objects = [
            GcsObject.from_dict(o)
            for o in dawn_deps['deps'][dawn_name]['objects']
        ]
        chromium_objects = [
            GcsObject.from_dict(o)
            for o in chromium_deps['deps'][chromium_name]['objects']
        ]
        if dawn_objects != chromium_objects:
            changed_gcs.append(
                ChangedGcs(
                    name=dawn_name,
                    old_objects=dawn_objects,
                    new_objects=chromium_objects,
                ))
    return changed_gcs


def _get_changed_non_exported_repos(dawn_deps: dict,
                                    chromium_deps: dict) -> list[ChangedRepo]:
    """Gets all non-exported repos that have changed between the DEPS files.

    Args:
        dawn_deps: The parsed content of the Dawn DEPS file.
        chromium_deps: The parsed content of the Chromium DEPS file.

    Returns:
        A list of all repo entries that have changed between the two DEPS files
        that are not exported pseudo-repos from Chromium.
    """
    changed_repos = []
    for dawn_name, chromium_name in SYNCED_REPOS.items():
        chromium_name = chromium_name or dawn_name
        chromium_name = 'src/' + chromium_name
        if dawn_name not in dawn_deps['deps']:
            raise RuntimeError(
                f'Unable to find Dawn repo {dawn_name}. Was it removed?')
        if chromium_name not in chromium_deps['deps']:
            raise RuntimeError(
                f'Unable to find Chromium repo {chromium_name}. Was it '
                f'removed?')

        url, dawn_revision = _get_url_and_revision(
            _get_raw_url_for_dep_entry(dawn_name, dawn_deps),
            dawn_deps['vars'])
        _, chromium_revision = _get_url_and_revision(
            _get_raw_url_for_dep_entry(chromium_name, chromium_deps),
            chromium_deps['vars'])
        if dawn_revision != chromium_revision:
            changed_repos.append(
                ChangedRepo(
                    name=dawn_name,
                    url=url,
                    old_revision=dawn_revision,
                    new_revision=chromium_revision,
                ))
    return changed_repos


def _get_changed_exported_repos(dawn_deps: dict) -> list[ChangedRepo]:
    """Gets all exported repos that have changed since the last roll.

    Args:
        dawn_deps: The parsed content of the Dawn DEPS file.

    Returns:
        A list of all repo entries for exported pseudo-repos from Chromium
        whose HEAD revision is different from the revision currently used by
        Dawn.
    """
    changed_repos = []
    for dawn_name, chromium_path in EXPORTED_CHROMIUM_REPOS.items():
        chromium_path = chromium_path or dawn_name
        if dawn_name not in dawn_deps['deps']:
            raise RuntimeError(
                f'Unable to find Dawn repo {dawn_name}. Was it removed?')
        url, dawn_revision = _get_url_and_revision(
            _get_raw_url_for_dep_entry(dawn_name, dawn_deps),
            dawn_deps['vars'])
        head_revision = _get_remote_head_revision(url)
        if dawn_revision != head_revision:
            changed_repos.append(
                ChangedRepo(
                    name=dawn_name,
                    url=url,
                    old_revision=dawn_revision,
                    new_revision=head_revision,
                ))
    return changed_repos


def _get_raw_url_for_dep_entry(dep_name: str, deps: dict) -> str:
    """Gets the URL associated with the specified DEPS entry.

    Args:
        dep_name: The name of the DEPS entry to retrieve the URL for.
        deps: The parsed DEPS content to extract information from.

    Returns:
        A string containing the URL associated with |dep_name|.
    """
    dep_entry = deps['deps'][dep_name]
    # Most entries are dicts, but it's also valid for an entry to just be a
    # git URL.
    if isinstance(dep_entry, str):
        return dep_entry
    return dep_entry['url']


def _get_url_and_revision(deps_url: str, vars: dict) -> tuple[str, str]:
    """Extracts the repo URL and revision from a DEPS url value.

    Known substitutions are also performed on the URL, e.g. replacing
    {chromium_git} with the actual Chromium git URL.

    Args:
        deps_url: The URL to operate on.
        vars: The 'vars' mapping from the parsed DEPS content that |deps_url|
            came from.

    Returns:
        A tuple (url, revision).
    """
    url, revision = deps_url.rsplit('@', 1)
    for var, value in vars.items():
        if not isinstance(value, str):
            continue
        search_str = f'{{{var}}}'
        url = url.replace(search_str, value)
    return url, revision


def _generate_commit_message(changed_entries: list[ChangedDepsEntry],
                             chromium_revision_range: ChangedRepo,
                             autoroll: bool) -> str:
    """Generates a commit message for a roll.

    Args:
        changed_entries: A list of all ChangedDepsEntry for this roll.
        chromium_revision_range: The ChangedRepo entry for Chromium for this
            roll.
        autoroll: Whether this roll is being done by the autoroller.

    Returns:
        A string containing the commit message to use.
    """
    commit_message_lines = []
    commit_message_lines.extend(
        _generate_chromium_section(chromium_revision_range, autoroll))
    commit_message_lines.append('')
    commit_message_lines.extend(_generate_command_section())
    commit_message_lines.append('')
    commit_message_lines.extend(_generate_repo_section(changed_entries))
    commit_message_lines.append('')
    commit_message_lines.extend(_generate_cipd_section(changed_entries))
    commit_message_lines.append('')
    commit_message_lines.extend(_generate_gcs_section(changed_entries))
    commit_message_lines.append('')
    commit_message_lines.extend(_generate_variables_section(changed_entries))
    commit_message_lines.append('')
    commit_message_lines.extend(_generate_footers_section(autoroll))
    return '\n'.join(commit_message_lines)


def _generate_chromium_section(chromium_revision_range: ChangedRepo,
                               autoroll: bool) -> list[str]:
    """Generates the commit message section for Chromium.

    Args:
        chromium_revision_range: The ChangedRepo entry for Chromium for this
            roll.
        autoroll: Whether this roll is being done by the autoroller.

    Returns:
        A list of strings containing lines to append to the commit message.
    """
    chromium_lines = []
    # Autorolls already add information about chromium_revision to the commit
    # message.
    if autoroll:
        return chromium_lines

    chromium_lines.extend([
        f'Roll chromium_revision {chromium_revision_range.revision_range()}',
        f'',
        f'Change log: {chromium_revision_range.log_link()}',
        f'Full diff: {chromium_revision_range.commit_link()}',
    ])
    return chromium_lines


def _generate_command_section() -> list[str]:
    """Generates the commit message section for the script command.

    Returns:
        A list of strings containing lines to append to the commit message.
    """
    command_lines = [
        'DEPS, submodule, and //infra/config changes generated by running:',
    ]
    script = pathlib.Path(__file__).resolve()
    script = script.relative_to(DAWN_ROOT)
    relative_command = [str(script)] + sys.argv[1:]
    command_lines.append(f'  {shlex.join(relative_command)}')
    return command_lines


def _generate_section_for_entry_type(entry_type: Type[ChangedDepsEntry],
                                     changed_entries: list[ChangedDepsEntry],
                                     empty_message: str,
                                     header: str) -> list[str]:
    """Generates the commit message section for a given ChangedDepsEntry type.

    Args:
        entry_type: The type of DEPS entry that should be included in this
            section.
        changed_entries: A list of all ChangedDepsEntry for this roll.
        empty_message: The message to use if no matching entries are found.
        header: The message to use at the beginning of the section if at least
            one matching entry is found.

    Returns:
        A list of strings containing lines to append to the commit message.
    """
    matching_entries = []
    for ce in changed_entries:
        if isinstance(ce, entry_type):
            matching_entries.append(ce)

    if not matching_entries:
        return [empty_message]

    matching_entries.sort(key=lambda e: e.name)
    lines = [header]
    for me in matching_entries:
        lines.extend(me.commit_message_lines())
    return lines


def _generate_variables_section(
        changed_entries: list[ChangedDepsEntry]) -> list[str]:
    """Generates the commit message section for changed variables.

    Args:
        changed_entries: A list of all ChangedDepsEntry for this roll.

    Returns:
        A list of strings containing lines to append to the commit message.
    """
    return _generate_section_for_entry_type(
        ChangedVariable, changed_entries,
        'No explicitly synced GN variables changed in this roll',
        'Explicitly synced GN variables:')


def _generate_cipd_section(
        changed_entries: list[ChangedDepsEntry]) -> list[str]:
    """Generates the commit message section for changed CIPD entries.

    Args:
        changed_entries: A list of all ChangedDepsEntry for this roll.

    Returns:
        A list of strings containing lines to append to the commit message.
    """
    return _generate_section_for_entry_type(
        ChangedCipd, changed_entries, 'No CIPD entries changed in this roll',
        'CIPD entries:')


def _generate_gcs_section(
        changed_entries: list[ChangedDepsEntry]) -> list[str]:
    """Generates the commit message section for changed GCS entries.

    Args:
        changed_entries: A list of all ChangedDepsEntry for this roll.

    Returns:
        A list of strings containing lines to append to the commit message.
    """
    return _generate_section_for_entry_type(
        ChangedGcs, changed_entries, 'No GCS entries changed in this roll',
        'GCS entries:')


def _generate_repo_section(
        changed_entries: list[ChangedDepsEntry]) -> list[str]:
    """Generates the commit message section for change repo entries.

    Args:
        changed_entries: A list of all ChangedDepsEntry for this roll.

    Returns:
        A list of strings containing lines to append to the commit message.
    """
    return _generate_section_for_entry_type(
        ChangedRepo, changed_entries, 'No repo entries changed in this roll',
        'Repo entries:')


def _generate_footers_section(autoroll: bool) -> list[str]:
    """Generates the commit message section for CL footers.

    Args:
        autoroll: Whether this roll is being done by the autoroller.

    Returns:
        A list of strings containing lines to append to the commit message.
    """
    lines = []
    if not autoroll:
        lines.append('Bug: None')
    return lines


def _apply_changed_deps(changed_entries: list[ChangedDepsEntry]) -> None:
    """Applies all changed DEPS entries to the Dawn DEPS file.

    Args:
        changed_entries: All calculated ChangedDepsEntry objects.
    """
    cmd = [
        'gclient',
        'setdep',
    ]
    for ce in changed_entries:
        cmd.extend(ce.setdep_args())
    subprocess.run(cmd, check=True)


def _sync_starlark_packages(chromium_revision: str) -> list[ChangedRepo]:
    """Syncs Starlark packages shared with Chromium.

    These are used by //infra/config and stored in
    //infra/config/global/PACKAGE.star.

    Note: The returned entries are technically not DEPS entries (ChangedRepo
    inherits from ChangedDepsEntry), but they similar enough that they can
    treated as such as long as the returned ChangedRepos are not actually
    applied to the DEPS file.

    Args:
        chromium_revision: The Chromium revision to sync Starlark packages to.

    Returns:
        A list of ChangedRepo specifying the old and new revisions for Starlark
        packages.
    """
    chromium_package_contents = _read_remote_chromium_file(
        'infra/config/PACKAGE.star', chromium_revision)
    chromium_luci_package = '@chromium-luci'
    new_chromium_luci_revision = _extract_starlark_package_revision(
        chromium_luci_package, chromium_package_contents)

    with open(PACKAGE_STAR, encoding='utf-8') as infile:
        dawn_package_contents = infile.read()
    dawn_package_contents, old_chromium_luci_revision = (
        _exchange_starlark_package_revision(chromium_luci_package,
                                            dawn_package_contents,
                                            new_chromium_luci_revision))
    dawn_package_contents, old_chromium_targets_revision = (
        _exchange_starlark_package_revision('@chromium-targets',
                                            dawn_package_contents,
                                            chromium_revision))
    with open(PACKAGE_STAR, 'w', encoding='utf-8') as outfile:
        outfile.write(dawn_package_contents)

    # Automatically stage any changes to be consistent with DEPS modifications.
    _run_lucicfg_and_stage_changes()

    changed_packages = [
        ChangedRepo(name='chromium-luci (Starlark)',
                    url=posixpath.join(CHROMIUM_GOB_URL, 'infra', 'chromium'),
                    old_revision=old_chromium_luci_revision,
                    new_revision=new_chromium_luci_revision),
        # The chromium-targets package is not reported since the revision is
        # identical to Chromium's revision, which is already reported.
    ]

    # Only report actual changes so that unchanged packages are omitted from the
    # CL description.
    return [p for p in changed_packages if p.old_revision != p.new_revision]


def _run_lucicfg_and_stage_changes() -> None:
    """Runs lucicfg on Dawn's Starlark files and stages any changes."""
    subprocess.check_call(
        ['lucicfg', 'generate',
         str(INFRA_PATH / 'main.star')],
        stdout=subprocess.DEVNULL,
        stderr=subprocess.DEVNULL)

    subprocess.check_call(['git', 'add', str(INFRA_PATH)],
                          stdout=subprocess.DEVNULL,
                          stderr=subprocess.DEVNULL)


def _exchange_starlark_package_revision(package_name: str,
                                        package_star_contents: str,
                                        new_revision: str) -> tuple[str, str]:
    """Exchanges the revision for a Starlark package.

    Args:
        package_name: The Starlark package name to exchange the revision for,
            including the leading @.
        package_star_contents: The contents of a PACKAGE.star file to modify.
        new_revision: The new revision to put in the file contents.

    Returns:
        A tuple (exchanged_contents, old_revision). |exchanged_contents| is a
        copy of |package_star_contents| with the revision for |package_name|
        replaced with |new_revision|. |old_revision| is the revision for
        |package_name| that was present before the exchange.
    """
    old_revision = _extract_starlark_package_revision(package_name,
                                                      package_star_contents)
    package_star_contents = _replace_starlark_package_revision(
        package_name, new_revision, package_star_contents)
    return package_star_contents, old_revision


@functools.cache
def _get_starlark_package_regex_for(package_name: str) -> re.Pattern:
    """Get a Pattern to match the given Starlark package definition.

    This Pattern is suitable for either extracting a revision or replacing it
    with a new one.

    Args:
        package_name: The Starlark package name to search for, including the
            leading @.

    Returns:
        A Pattern that matches |package_name| in PACKAGE.star.
    """
    revision_pattern = re.compile(
        #   Group 1: Context prefix to ensure that we're matching the correct
        #   package entry.
        rf'(name\s*=\s*"{package_name}".*?revision\s*=\s*")'
        #   Group 2: The hexadecimal revision to extract/replace.
        r'([a-fA-F0-9]+)'
        #   Positive lookahead: Asserts that a " follows, but does not
        #   match/consume it.
        r'(?=")',
        re.DOTALL)

    return revision_pattern


def _extract_starlark_package_revision(package_name: str,
                                       package_star_contents: str) -> str:
    """Extract a Starlark package revision from PACKAGE.star content.

    Args:
        package_name: The Starlark package name to search for, including the
            leading @.
        package_star_contents: The contents of a PACKAGE.star file to search.

    Returns:
        The git revision of the requested package.
    """
    revision_pattern = _get_starlark_package_regex_for(package_name)
    match = revision_pattern.search(package_star_contents)
    if not match:
        raise RuntimeError(
            f'Unable to extract {package_name} revision from PACKAGE.star '
            f'contents')
    return match.group(2)


def _replace_starlark_package_revision(package_name: str, new_revision: str,
                                       package_star_contents: str) -> str:
    """Replace a Starlark package revision in PACKAGE.star content.

    package_name: The Starlark package name to search for, including the
            leading @.
    new_revision: The new revision to update the package to.
    package_star_contents: The contents of a PACKAGE.star file to
        find/replace in.

    Returns:
        A copy of |package_star_contents| with the revision for |package_name|
        updated to |new_revision|.
    """
    revision_pattern = _get_starlark_package_regex_for(package_name)
    # Replace the match with Group 1 (matched content before the revision) and
    # the new revision itself.
    updated_contents = revision_pattern.sub(rf'\g<1>{new_revision}',
                                            package_star_contents)
    return updated_contents


def _parse_args() -> argparse.Namespace:
    """Parses and returns command line arguments."""
    parser = argparse.ArgumentParser('Roll DEPS entries shared with Chromium.')
    parser.add_argument('--verbose',
                        '-v',
                        action='store_true',
                        help='Increase logging verbosity')
    parser.add_argument('--autoroll',
                        action='store_true',
                        help='Run the script in autoroll mode')
    parser.add_argument('--ignore-unclean-workdir',
                        action='store_true',
                        help=('Ignore uncommitted changes and being on a '
                              'non-main branch'))
    parser.add_argument('--revision',
                        help=('A Chromium revision to roll to. If '
                              'unspecified, HEAD is used.'))
    return parser.parse_args()


def main() -> None:
    args = _parse_args()
    if args.verbose:
        logging.basicConfig(level=logging.DEBUG)
    else:
        logging.basicConfig(level=logging.INFO)

    # The autoroller does not have locally synced dependencies, so we cannot use
    # the copy under //third_party. Instead, we have to assume that it has
    # depot_tools in PATH already.
    if not args.autoroll:
        _add_depot_tools_to_path()

    if not args.ignore_unclean_workdir:
        if not _is_tree_clean():
            raise RuntimeError(
                'Uncommitted or untracked files found. Please commit them or '
                'pass --ignore-unclean-workdir')
        _ensure_updated_main_branch()

    with open(DEPS_FILE, encoding='utf-8') as infile:
        dawn_deps = _parse_deps_file(infile.read())
    revision_range = _get_roll_revision_range(args.revision, dawn_deps)
    chromium_deps = _parse_deps_file(
        _read_remote_chromium_file('DEPS', revision_range.new_revision))
    changed_entries = _get_changed_deps_entries(dawn_deps, chromium_deps)

    # We want these entries to be in the commit message, but we do not want them
    # to be present for _apply_changed_deps() since they are not actually DEPS
    # entries.
    changed_packages = _sync_starlark_packages(revision_range.new_revision)
    entries_for_commit_message = changed_entries + changed_packages

    # Create the commit message before adding the entry for the Chromium
    # revision since Chromium information is explicitly added to the message.
    commit_message = _generate_commit_message(entries_for_commit_message,
                                              revision_range, args.autoroll)
    # We change the variable directly instead of using ChangedRepo since
    # 'gclient setdep --revision' does not work for repos if there is no entry
    # in .gitmodules.
    changed_entries.append(
        ChangedVariable(
            name=CHROMIUM_REVISION_VAR,
            old_version=revision_range.old_revision,
            new_version=revision_range.new_revision,
        ))

    # When running as part of the autoroller, we update the existing commit
    # on the current branch.
    if not args.autoroll:
        _create_roll_branch()

    _apply_changed_deps(changed_entries)

    if args.autoroll:
        _amend_commit(commit_message)
    else:
        if _is_tree_clean():
            logging.info('No changes detected, skipping commit')
        else:
            _commit(commit_message)
            logging.info('Changes committed locally and are ready for upload')


if __name__ == '__main__':
    main()
