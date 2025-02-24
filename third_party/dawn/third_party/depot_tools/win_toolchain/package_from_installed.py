#!/usr/bin/env python3
# Copyright 2014 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
"""
From a system-installed copy of the toolchain, packages all the required bits
into a .zip file.

It assumes default install locations for tools, on the C: drive.

1. Start from a fresh Windows VM image.
2. Download the VS installer. Run the installer with these parameters:
    --add Microsoft.VisualStudio.Workload.NativeDesktop
    --add Microsoft.VisualStudio.Component.VC.ATLMFC
    --add Microsoft.VisualStudio.Component.VC.Tools.ARM64
    --add Microsoft.VisualStudio.Component.VC.MFC.ARM64
    --includeRecommended --passive
These are equivalent to selecting the Desktop development with C++ workload,
within that the Visual C++ MFC for x86 and x64 component, and then  Individual
Components-> Compilers, build tools, and runtimes-> Visual C++ compilers and
libraries for ARM64, and Individual Components-> SDKs, libraries, and
frameworks-> Visual C++ MFC for ARM64 (which also brings in ATL for ARM64).
3. Use Add or Remove Programs to find the Windows SDK installed with VS
and modify it to include the debuggers.
4. Run this script, which will build a <sha1>.zip, something like this:
  python package_from_installed.py 2022 -w 10.0.22621.0|<SDK version>

Express is not yet supported by this script, but patches welcome (it's not too
useful as the resulting zip can't be redistributed, and most will presumably
have a Pro license anyway).
"""

import collections
import glob
import json
import optparse
import os
import platform
import shutil
import subprocess
import sys
import tempfile
import zipfile

import get_toolchain_if_necessary

_vs_version = None
_win_version = None
_vc_tools = None
SUPPORTED_VS_VERSIONS = ['2022']
_allow_multiple_vs_installs = False


def GetVSPath():
    # Use vswhere to find the VS installation. This will find prerelease
    # versions because -prerelease is specified. This assumes that only one
    # version is installed.
    command = (r'C:\Program Files (x86)\Microsoft Visual Studio\Installer'
               r'\vswhere.exe -prerelease')
    vs_version_marker = 'catalog_productLineVersion: '
    vs_path_marker = 'installationPath: '
    output = subprocess.check_output(command, universal_newlines=True)
    vs_path = None
    vs_installs_count = 0
    matching_vs_path = ""
    for line in output.splitlines():
        if line.startswith(vs_path_marker):
            # The path information comes first
            vs_path = line[len(vs_path_marker):]
            vs_installs_count += 1
        if line.startswith(vs_version_marker):
            # The version for that path comes later
            if line[len(vs_version_marker):] == _vs_version:
                matching_vs_path = vs_path

    if vs_installs_count == 0:
        raise Exception('VS %s path not found in vswhere output' %
                        (_vs_version))
    if vs_installs_count > 1:
        if not _allow_multiple_vs_installs:
            raise Exception(
                'Multiple VS installs detected. This is unsupported. '
                'It is recommended that packaging be done on a clean VM '
                'with just one version installed. To proceed anyway add '
                'the --allow_multiple_vs_installs flag to this script')
        else:
            print('Multiple VS installs were detected. This is unsupported. '
                  'Proceeding anyway')
    return matching_vs_path


def ExpandWildcards(root, sub_dir):
    # normpath is needed to change '/' to '\\' characters.
    path = os.path.normpath(os.path.join(root, sub_dir))
    matches = glob.glob(path)
    if len(matches) != 1:
        raise Exception('%s had %d matches - should be one' %
                        (path, len(matches)))
    return matches[0]


def BuildRepackageFileList(src_dir):
    # Strip off a trailing separator if present
    if src_dir.endswith(os.path.sep):
        src_dir = src_dir[:-len(os.path.sep)]

    # Ensure .\Windows Kits\10\Debuggers exists and fail to repackage if it
    # doesn't.
    debuggers_path = os.path.join(src_dir, 'Windows Kits', '10', 'Debuggers')
    if not os.path.exists(debuggers_path):
        raise Exception('Repacking failed. Missing %s.' % (debuggers_path))

    result = []
    for root, _, files in os.walk(src_dir):
        for f in files:
            final_from = os.path.normpath(os.path.join(root, f))
            dest = final_from[len(src_dir) + 1:]
            result.append((final_from, dest))
    return result


def BuildFileList(override_dir, include_arm, vs_path):
    result = []

    # Subset of VS corresponding roughly to VC.
    paths = [
        'DIA SDK/bin',
        'DIA SDK/idl',
        'DIA SDK/include',
        'DIA SDK/lib',
        _vc_tools + '/atlmfc',
        _vc_tools + '/crt',
        'VC/redist',
    ]

    if override_dir:
        paths += [
            (os.path.join(override_dir, 'bin'), _vc_tools + '/bin'),
            (os.path.join(override_dir, 'include'), _vc_tools + '/include'),
            (os.path.join(override_dir, 'lib'), _vc_tools + '/lib'),
        ]
    else:
        paths += [
            _vc_tools + '/bin',
            _vc_tools + '/include',
            _vc_tools + '/lib',
        ]

    paths += [
        ('VC/redist/MSVC/14.*.*/x86/Microsoft.VC*.CRT', 'sys32'),
        ('VC/redist/MSVC/14.*.*/x86/Microsoft.VC*.CRT',
         'Windows Kits/10//bin/x86'),
        ('VC/redist/MSVC/14.*.*/debug_nonredist/x86/Microsoft.VC*.DebugCRT',
         'sys32'),
        ('VC/redist/MSVC/14.*.*/x64/Microsoft.VC*.CRT', 'sys64'),
        ('VC/redist/MSVC/14.*.*/x64/Microsoft.VC*.CRT', 'VC/bin/amd64_x86'),
        ('VC/redist/MSVC/14.*.*/x64/Microsoft.VC*.CRT', 'VC/bin/amd64'),
        ('VC/redist/MSVC/14.*.*/x64/Microsoft.VC*.CRT',
         'Windows Kits/10/bin/x64'),
        ('VC/redist/MSVC/14.*.*/debug_nonredist/x64/Microsoft.VC*.DebugCRT',
         'sys64'),
    ]
    if include_arm:
        paths += [
            ('VC/redist/MSVC/14.*.*/arm64/Microsoft.VC*.CRT', 'sysarm64'),
            ('VC/redist/MSVC/14.*.*/arm64/Microsoft.VC*.CRT',
             'VC/bin/amd64_arm64'),
            ('VC/redist/MSVC/14.*.*/arm64/Microsoft.VC*.CRT', 'VC/bin/arm64'),
            ('VC/redist/MSVC/14.*.*/arm64/Microsoft.VC*.CRT',
             'Windows Kits/10/bin/arm64'),
            ('VC/redist/MSVC/14.*.*/debug_nonredist/arm64/Microsoft.VC*.DebugCRT',
             'sysarm64'),
        ]

    for path in paths:
        src = path[0] if isinstance(path, tuple) else path
        # Note that vs_path is ignored if src is an absolute path.
        combined = ExpandWildcards(vs_path, src)
        if not os.path.exists(combined):
            raise Exception('%s missing.' % combined)
        if not os.path.isdir(combined):
            raise Exception('%s not a directory.' % combined)
        for root, _, files in os.walk(combined):
            for f in files:
                # vctip.exe doesn't shutdown, leaving locks on directories. It's
                # optional so let's avoid this problem by not packaging it.
                # https://crbug.com/735226
                if f.lower() == 'vctip.exe':
                    continue
                final_from = os.path.normpath(os.path.join(root, f))
                if isinstance(path, tuple):
                    assert final_from.startswith(combined)
                    dest = final_from[len(combined) + 1:]
                    result.append((final_from,
                                   os.path.normpath(os.path.join(path[1],
                                                                 dest))))
                else:
                    assert final_from.startswith(vs_path)
                    dest = final_from[len(vs_path) + 1:]
                    result.append((final_from, dest))

    command = (
        r'reg query "HKLM\SOFTWARE\Microsoft\Windows Kits\Installed Roots"'
        r' /v KitsRoot10')
    marker = "    KitsRoot10    REG_SZ    "
    sdk_path = None
    output = subprocess.check_output(command, universal_newlines=True)
    for line in output.splitlines():
        if line.startswith(marker):
            sdk_path = line[len(marker):]

    # Strip off a trailing slash if present
    if sdk_path.endswith(os.path.sep):
        sdk_path = sdk_path[:-len(os.path.sep)]

    debuggers_path = os.path.join(sdk_path, 'Debuggers')
    if not os.path.exists(debuggers_path):
        raise Exception('Packaging failed. Missing %s.' % (debuggers_path))

    for root, _, files in os.walk(sdk_path):
        for f in files:
            combined = os.path.normpath(os.path.join(root, f))
            # Some of the files in this directory are exceedingly long (and
            # exceed _MAX_PATH for any moderately long root), so exclude them.
            # We don't need them anyway. Exclude/filter/skip others just to save
            # space.
            tail = combined[len(sdk_path) + 1:]
            skip_dir = False
            for dir in [
                    'References\\', 'Windows Performance Toolkit\\',
                    'Testing\\', 'App Certification Kit\\', 'Extension SDKs\\',
                    'Assessment and Deployment Kit\\'
            ]:
                if tail.startswith(dir):
                    skip_dir = True
            if skip_dir:
                continue
            # There may be many Include\Lib\Source\bin directories for many
            # different versions of Windows and packaging them all wastes ~450
            # MB (uncompressed) per version and wastes time. Only copy the
            # specified version. Note that the SDK version number started being
            # part of the bin path with 10.0.15063.0.
            if (tail.startswith('Include\\') or tail.startswith('Lib\\')
                    or tail.startswith('Source\\') or tail.startswith('bin\\')):
                if tail.count(_win_version) == 0:
                    continue
            to = os.path.join('Windows Kits', '10', tail)
            result.append((combined, to))

    # Copy the x86 ucrt DLLs to all directories with x86 binaries that are
    # added to the path by SetEnv.cmd, and to sys32. Starting with the 17763
    # SDK the ucrt files are in _win_version\ucrt instead of just ucrt.
    ucrt_dir = os.path.join(sdk_path, 'redist', _win_version, r'ucrt\dlls\x86')
    if not os.path.exists(ucrt_dir):
        ucrt_dir = os.path.join(sdk_path, r'redist\ucrt\dlls\x86')
    ucrt_paths = glob.glob(ucrt_dir + r'\*')
    assert (len(ucrt_paths) > 0)
    for ucrt_path in ucrt_paths:
        ucrt_file = os.path.split(ucrt_path)[1]
        for dest_dir in [r'Windows Kits\10\bin\x86', 'sys32']:
            result.append((ucrt_path, os.path.join(dest_dir, ucrt_file)))

    # Copy the x64 ucrt DLLs to all directories with x64 binaries that are
    # added to the path by SetEnv.cmd, and to sys64.
    ucrt_dir = os.path.join(sdk_path, 'redist', _win_version, r'ucrt\dlls\x64')
    if not os.path.exists(ucrt_dir):
        ucrt_dir = os.path.join(sdk_path, r'redist\ucrt\dlls\x64')
    ucrt_paths = glob.glob(ucrt_dir + r'\*')
    assert (len(ucrt_paths) > 0)
    for ucrt_path in ucrt_paths:
        ucrt_file = os.path.split(ucrt_path)[1]
        for dest_dir in [
                r'VC\bin\amd64_x86', r'VC\bin\amd64',
                r'Windows Kits\10\bin\x64', 'sys64'
        ]:
            result.append((ucrt_path, os.path.join(dest_dir, ucrt_file)))

    system_crt_files = [
        # Needed to let debug binaries run.
        'ucrtbased.dll',
    ]
    cpu_pairs = [
        ('x86', 'sys32'),
        ('x64', 'sys64'),
    ]
    if include_arm:
        cpu_pairs += [
            ('arm64', 'sysarm64'),
        ]
    for system_crt_file in system_crt_files:
        for cpu_pair in cpu_pairs:
            target_cpu, dest_dir = cpu_pair
            src_path = os.path.join(sdk_path, 'bin', _win_version, target_cpu,
                                    'ucrt')
            result.append((os.path.join(src_path, system_crt_file),
                           os.path.join(dest_dir, system_crt_file)))

    # Generically drop all arm stuff that we don't need, and
    # drop .msi files because we don't need installers and drop
    # samples since those are not used by any tools.
    def is_skippable(f):
        return ('arm\\' in f.lower()
                or (not include_arm and 'arm64\\' in f.lower())
                or 'samples\\' in f.lower() or f.lower().endswith(
                    ('.msi', '.msm')))

    return [(f, t) for f, t in result if not is_skippable(f)]


def GenerateSetEnvCmd(target_dir):
    """Generate a batch file that gyp expects to exist to set up the compiler
    environment.

    This is normally generated by a full install of the SDK, but we
    do it here manually since we do not do a full install."""
    vc_tools_parts = _vc_tools.split('/')

    # All these paths are relative to the root of the toolchain package.
    include_dirs = [
        ['Windows Kits', '10', 'Include', _win_version, 'um'],
        ['Windows Kits', '10', 'Include', _win_version, 'shared'],
        ['Windows Kits', '10', 'Include', _win_version, 'winrt'],
    ]
    include_dirs.append(['Windows Kits', '10', 'Include', _win_version, 'ucrt'])
    include_dirs.extend([
        vc_tools_parts + ['include'],
        vc_tools_parts + ['atlmfc', 'include'],
    ])
    libpath_dirs = [
        vc_tools_parts + ['lib', 'x86', 'store', 'references'],
        ['Windows Kits', '10', 'UnionMetadata', _win_version],
    ]
    # Common to x86, x64, and arm64
    env = collections.OrderedDict([
        # Yuck: These have a trailing \ character. No good way to represent this
        # in an OS-independent way.
        ('VSINSTALLDIR', [['.\\']]),
        ('VCINSTALLDIR', [['VC\\']]),
        ('INCLUDE', include_dirs),
        ('LIBPATH', libpath_dirs),
    ])
    # x86. Always use amd64_x86 cross, not x86 on x86.
    env['VCToolsInstallDir'] = [vc_tools_parts[:]]
    # Yuck: This one ends in a path separator as well.
    env['VCToolsInstallDir'][0][-1] += os.path.sep
    env_x86 = collections.OrderedDict([
        (
            'PATH',
            [
                ['Windows Kits', '10', 'bin', _win_version, 'x64'],
                vc_tools_parts + ['bin', 'HostX64', 'x86'],
                vc_tools_parts +
                ['bin', 'HostX64', 'x64'],  # Needed for mspdb1x0.dll.
            ]),
        ('LIB', [
            vc_tools_parts + ['lib', 'x86'],
            ['Windows Kits', '10', 'Lib', _win_version, 'um', 'x86'],
            ['Windows Kits', '10', 'Lib', _win_version, 'ucrt', 'x86'],
            vc_tools_parts + ['atlmfc', 'lib', 'x86'],
        ]),
    ])

    # x64.
    env_x64 = collections.OrderedDict([
        ('PATH', [
            ['Windows Kits', '10', 'bin', _win_version, 'x64'],
            vc_tools_parts + ['bin', 'HostX64', 'x64'],
        ]),
        ('LIB', [
            vc_tools_parts + ['lib', 'x64'],
            ['Windows Kits', '10', 'Lib', _win_version, 'um', 'x64'],
            ['Windows Kits', '10', 'Lib', _win_version, 'ucrt', 'x64'],
            vc_tools_parts + ['atlmfc', 'lib', 'x64'],
        ]),
    ])

    # arm64.
    env_arm64 = collections.OrderedDict([
        ('PATH', [
            ['Windows Kits', '10', 'bin', _win_version, 'x64'],
            vc_tools_parts + ['bin', 'HostX64', 'arm64'],
            vc_tools_parts + ['bin', 'HostX64', 'x64'],
        ]),
        ('LIB', [
            vc_tools_parts + ['lib', 'arm64'],
            ['Windows Kits', '10', 'Lib', _win_version, 'um', 'arm64'],
            ['Windows Kits', '10', 'Lib', _win_version, 'ucrt', 'arm64'],
            vc_tools_parts + ['atlmfc', 'lib', 'arm64'],
        ]),
    ])

    def BatDirs(dirs):
        return ';'.join(['%cd%\\' + os.path.join(*d) for d in dirs])

    set_env_prefix = os.path.join(target_dir, r'Windows Kits\10\bin\SetEnv')
    with open(set_env_prefix + '.cmd', 'w') as f:
        # The prologue changes the current directory to the root of the
        # toolchain package, so that path entries can be set up without needing
        # ..\..\..\ components.
        f.write('@echo off\n'
                ':: Generated by win_toolchain\\package_from_installed.py.\n'
                'pushd %~dp0..\..\..\n')
        for var, dirs in env.items():
            f.write('set %s=%s\n' % (var, BatDirs(dirs)))
        f.write('if "%1"=="/x64" goto x64\n')
        f.write('if "%1"=="/arm64" goto arm64\n')

        for var, dirs in env_x86.items():
            f.write('set %s=%s%s\n' %
                    (var, BatDirs(dirs), ';%PATH%' if var == 'PATH' else ''))
        f.write('goto :END\n')

        f.write(':x64\n')
        for var, dirs in env_x64.items():
            f.write('set %s=%s%s\n' %
                    (var, BatDirs(dirs), ';%PATH%' if var == 'PATH' else ''))
        f.write('goto :END\n')

        f.write(':arm64\n')
        for var, dirs in env_arm64.items():
            f.write('set %s=%s%s\n' %
                    (var, BatDirs(dirs), ';%PATH%' if var == 'PATH' else ''))
        f.write('goto :END\n')
        f.write(':END\n')
        # Restore the original directory.
        f.write('popd\n')
    with open(set_env_prefix + '.x86.json', 'wt', newline='') as f:
        assert not set(env.keys()) & set(env_x86.keys()), 'dupe keys'
        json.dump(
            {
                'env':
                collections.OrderedDict(
                    list(env.items()) + list(env_x86.items()))
            }, f)
    with open(set_env_prefix + '.x64.json', 'wt', newline='') as f:
        assert not set(env.keys()) & set(env_x64.keys()), 'dupe keys'
        json.dump(
            {
                'env':
                collections.OrderedDict(
                    list(env.items()) + list(env_x64.items()))
            }, f)
    with open(set_env_prefix + '.arm64.json', 'wt', newline='') as f:
        assert not set(env.keys()) & set(env_arm64.keys()), 'dupe keys'
        json.dump(
            {
                'env':
                collections.OrderedDict(
                    list(env.items()) + list(env_arm64.items()))
            }, f)


def AddEnvSetup(files, include_arm):
    """We need to generate this file in the same way that the "from pieces"
    script does, so pull that in here."""
    tempdir = tempfile.mkdtemp()
    os.makedirs(os.path.join(tempdir, 'Windows Kits', '10', 'bin'))
    GenerateSetEnvCmd(tempdir)
    files.append(
        (os.path.join(tempdir, 'Windows Kits', '10', 'bin',
                      'SetEnv.cmd'), 'Windows Kits\\10\\bin\\SetEnv.cmd'))
    files.append((os.path.join(tempdir, 'Windows Kits', '10', 'bin',
                               'SetEnv.x86.json'),
                  'Windows Kits\\10\\bin\\SetEnv.x86.json'))
    files.append((os.path.join(tempdir, 'Windows Kits', '10', 'bin',
                               'SetEnv.x64.json'),
                  'Windows Kits\\10\\bin\\SetEnv.x64.json'))
    if include_arm:
        files.append((os.path.join(tempdir, 'Windows Kits', '10', 'bin',
                                   'SetEnv.arm64.json'),
                      'Windows Kits\\10\\bin\\SetEnv.arm64.json'))
    vs_version_file = os.path.join(tempdir, 'VS_VERSION')
    with open(vs_version_file, 'wt', newline='') as version:
        print(_vs_version, file=version)
    files.append((vs_version_file, 'VS_VERSION'))


def RenameToSha1(output):
    """Determine the hash in the same way that the unzipper does to rename the
    # .zip file."""
    print('Extracting to determine hash...')
    tempdir = tempfile.mkdtemp()
    old_dir = os.getcwd()
    os.chdir(tempdir)
    rel_dir = 'vs_files'
    with zipfile.ZipFile(os.path.join(old_dir, output), 'r',
                         zipfile.ZIP_DEFLATED, True) as zf:
        zf.extractall(rel_dir)
    print('Hashing...')
    sha1 = get_toolchain_if_necessary.CalculateHash(rel_dir, None)
    # Shorten from forty characters to ten. This is still enough to avoid
    # collisions, while being less unwieldy and reducing the risk of MAX_PATH
    # failures.
    sha1 = sha1[:10]
    os.chdir(old_dir)
    shutil.rmtree(tempdir)
    final_name = sha1 + '.zip'
    os.rename(output, final_name)
    print('Renamed %s to %s.' % (output, final_name))


def main():
    usage = 'usage: %prog [options] 2022'
    parser = optparse.OptionParser(usage)
    parser.add_option('-w',
                      '--winver',
                      action='store',
                      type='string',
                      dest='winver',
                      default='10.0.22621.0',
                      help='Windows SDK version, such as 10.0.22621.0')
    parser.add_option('-d',
                      '--dryrun',
                      action='store_true',
                      dest='dryrun',
                      default=False,
                      help='scan for file existence and prints statistics')
    parser.add_option('--noarm',
                      action='store_false',
                      dest='arm',
                      default=True,
                      help='Avoids arm parts of the SDK')
    parser.add_option('--override',
                      action='store',
                      type='string',
                      dest='override_dir',
                      default=None,
                      help='Specify alternate bin/include/lib directory')
    parser.add_option(
        '--repackage',
        action='store',
        type='string',
        dest='repackage_dir',
        default=None,
        help='Specify raw directory to be packaged, for hot fixes.')
    parser.add_option('--allow_multiple_vs_installs',
                      action='store_true',
                      default=False,
                      dest='allow_multiple_vs_installs',
                      help='Specify if multiple VS installs are allowed.')
    (options, args) = parser.parse_args()

    if options.repackage_dir:
        files = BuildRepackageFileList(options.repackage_dir)
    else:
        if len(args) != 1 or args[0] not in SUPPORTED_VS_VERSIONS:
            print('Must specify 2022')
            parser.print_help()
            return 1

        if options.override_dir:
            if (not os.path.exists(os.path.join(options.override_dir, 'bin'))
                    or not os.path.exists(
                        os.path.join(options.override_dir, 'include'))
                    or not os.path.exists(
                        os.path.join(options.override_dir, 'lib'))):
                print(
                    'Invalid override directory - must contain bin/include/lib dirs'
                )
                return 1

        global _vs_version
        _vs_version = args[0]
        global _win_version
        _win_version = options.winver
        global _vc_tools
        global _allow_multiple_vs_installs
        _allow_multiple_vs_installs = options.allow_multiple_vs_installs
        vs_path = GetVSPath()
        temp_tools_path = ExpandWildcards(vs_path, 'VC/Tools/MSVC/14.*.*')
        # Strip off the leading vs_path characters and switch back to /
        # separators.
        _vc_tools = temp_tools_path[len(vs_path) + 1:].replace('\\', '/')

        print('Building file list for VS %s Windows %s...' %
              (_vs_version, _win_version))
        files = BuildFileList(options.override_dir, options.arm, vs_path)

        AddEnvSetup(files, options.arm)

    if False:
        for f in files:
            print(f[0], '->', f[1])
        return 0

    output = 'out.zip'
    if os.path.exists(output):
        os.unlink(output)
    count = 0
    version_match_count = 0
    total_size = 0
    missing_files = False
    with zipfile.ZipFile(output, 'w', zipfile.ZIP_DEFLATED, True) as zf:
        for disk_name, archive_name in files:
            sys.stdout.write('\r%d/%d ...%s' %
                             (count, len(files), disk_name[-40:]))
            sys.stdout.flush()
            count += 1
            if not options.repackage_dir and disk_name.count(_win_version) > 0:
                version_match_count += 1
            if os.path.exists(disk_name):
                total_size += os.path.getsize(disk_name)
                if not options.dryrun:
                    zf.write(disk_name, archive_name)
            else:
                missing_files = True
                sys.stdout.write('\r%s does not exist.\n\n' % disk_name)
                sys.stdout.flush()
    sys.stdout.write(
        '\r%1.3f GB of data in %d files, %d files for %s.%s\n' %
        (total_size / 1e9, count, version_match_count, _win_version, ' ' * 50))
    if options.dryrun:
        return 0
    if missing_files:
        raise Exception('One or more files were missing - aborting')
    if not options.repackage_dir and version_match_count == 0:
        raise Exception('No files found that match the specified winversion')
    sys.stdout.write('\rWrote to %s.%s\n' % (output, ' ' * 50))
    sys.stdout.flush()

    RenameToSha1(output)

    return 0


if __name__ == '__main__':
    sys.exit(main())
