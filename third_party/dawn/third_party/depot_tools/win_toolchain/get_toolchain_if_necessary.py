#!/usr/bin/env python3
# Copyright 2013 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
"""Downloads and unpacks a toolchain for building on Windows. The contents are
matched by sha1 which will be updated when the toolchain is updated.

Having a toolchain script in depot_tools means that it's not versioned
directly with the source code. That is, if the toolchain is upgraded, but
you're trying to build an historical version of Chromium from before the
toolchain upgrade, this will cause you to build with a newer toolchain than
was available when that code was committed. This is done for a two main
reasons: 1) it would likely be annoying to have the up-to-date toolchain
removed and replaced by one without a service pack applied); 2) it would
require maintaining scripts that can build older not-up-to-date revisions of
the toolchain. This is likely to be a poorly tested code path that probably
won't be properly maintained. See http://crbug.com/323300.
"""

import argparse
from contextlib import closing
import hashlib
import filecmp
import json
import os
import shutil
import subprocess
import sys
import tempfile
import time
from urllib.request import urlopen
from urllib.parse import urljoin
from urllib.error import URLError
import zipfile

# Environment variable that, if set, specifies the default Visual Studio
# toolchain root directory to use.
ENV_TOOLCHAIN_ROOT = 'DEPOT_TOOLS_WIN_TOOLCHAIN_ROOT'

# winreg isn't natively available under CygWin
if sys.platform == "win32":
    try:
        import winreg
    except ImportError:
        import _winreg as winreg
elif sys.platform == "cygwin":
    try:
        import cygwinreg as winreg
    except ImportError:
        print('')
        print(
            'CygWin does not natively support winreg but a replacement exists.')
        print('https://pypi.python.org/pypi/cygwinreg/')
        print('')
        print('Try: easy_install cygwinreg')
        print('')
        raise

BASEDIR = os.path.dirname(os.path.abspath(__file__))
DEPOT_TOOLS_PATH = os.path.join(BASEDIR, '..')
sys.path.append(DEPOT_TOOLS_PATH)
try:
    import download_from_google_storage
except ImportError:
    # Allow use of utility functions in this script from package_from_installed
    # on bare VM that doesn't have a full depot_tools.
    pass


def GetFileList(root):
    """Gets a normalized list of files under |root|."""
    assert not os.path.isabs(root)
    assert os.path.normpath(root) == root
    file_list = []
    # Ignore WER ReportQueue entries that vctip/cl leave in the bin dir if/when
    # they crash. Also ignores the content of the
    # Windows Kits/10/debuggers/x(86|64)/(sym|src)/ directories as this is just
    # the temporarily location that Windbg might use to store the symbol files
    # and downloaded sources.
    #
    # Note: These files are only created on a Windows host, so the
    # ignored_directories list isn't relevant on non-Windows hosts.

    # The Windows SDK is either in `win_sdk` or in `Windows Kits\10`. This
    # script must work with both layouts, so check which one it is.
    # This can be different in each |root|.
    if os.path.isdir(os.path.join(root, 'Windows Kits', '10')):
        win_sdk = 'Windows Kits\\10'
    else:
        win_sdk = 'win_sdk'

    ignored_directories = [
        'wer\\reportqueue', win_sdk + '\\debuggers\\x86\\sym\\',
        win_sdk + '\\debuggers\\x64\\sym\\',
        win_sdk + '\\debuggers\\x86\\src\\', win_sdk + '\\debuggers\\x64\\src\\'
    ]
    ignored_directories = [d.lower() for d in ignored_directories]

    for base, _, files in os.walk(root):
        paths = [os.path.join(base, f) for f in files]
        for p in paths:
            if any(ignored_dir in p.lower()
                   for ignored_dir in ignored_directories):
                continue
            file_list.append(p)
    return sorted(file_list, key=lambda s: s.replace('/', '\\').lower())


def MakeTimestampsFileName(root, sha1):
    return os.path.join(root, os.pardir, '%s.timestamps' % sha1)


def CalculateHash(root, expected_hash):
    """Calculates the sha1 of the paths to all files in the given |root| and the
    contents of those files, and returns as a hex string.

    |expected_hash| is the expected hash value for this toolchain if it has
    already been installed.
    """
    if expected_hash:
        full_root_path = os.path.join(root, expected_hash)
    else:
        full_root_path = root
    file_list = GetFileList(full_root_path)
    # Check whether we previously saved timestamps in
    # $root/../{sha1}.timestamps. If we didn't, or they don't match, then do the
    # full calculation, otherwise return the saved value.
    timestamps_file = MakeTimestampsFileName(root, expected_hash)
    timestamps_data = {'files': [], 'sha1': ''}
    if os.path.exists(timestamps_file):
        with open(timestamps_file, 'rb') as f:
            try:
                timestamps_data = json.load(f)
            except ValueError:
                # json couldn't be loaded, empty data will force a re-hash.
                pass

    matches = len(file_list) == len(timestamps_data['files'])
    # Don't check the timestamp of the version file as we touch this file to
    # indicates which versions of the toolchain are still being used.
    vc_dir = os.path.join(full_root_path, 'VC').lower()
    if matches:
        for disk, cached in zip(file_list, timestamps_data['files']):
            if disk != cached[0] or (disk != vc_dir
                                     and os.path.getmtime(disk) != cached[1]):
                matches = False
                break
    elif os.path.exists(timestamps_file):
        # Print some information about the extra/missing files. Don't do this if
        # we don't have a timestamp file, as all the files will be considered as
        # missing.
        timestamps_data_files = []
        for f in timestamps_data['files']:
            timestamps_data_files.append(f[0])
        missing_files = [f for f in timestamps_data_files if f not in file_list]
        if len(missing_files):
            print('%d files missing from the %s version of the toolchain:' %
                  (len(missing_files), expected_hash))
            for f in missing_files[:10]:
                print('\t%s' % f)
            if len(missing_files) > 10:
                print('\t...')
        extra_files = [f for f in file_list if f not in timestamps_data_files]
        if len(extra_files):
            print('%d extra files in the %s version of the toolchain:' %
                  (len(extra_files), expected_hash))
            for f in extra_files[:10]:
                print('\t%s' % f)
            if len(extra_files) > 10:
                print('\t...')
    if matches:
        return timestamps_data['sha1']

    # Make long hangs when updating the toolchain less mysterious.
    print('Calculating hash of toolchain in %s. Please wait...' %
          full_root_path)
    sys.stdout.flush()
    digest = hashlib.sha1()
    for path in file_list:
        path_without_hash = str(path).replace('/', '\\')
        if expected_hash:
            path_without_hash = path_without_hash.replace(
                os.path.join(root, expected_hash).replace('/', '\\'), root)
        digest.update(bytes(path_without_hash.lower(), 'utf-8'))
        with open(path, 'rb') as f:
            digest.update(f.read())

    # Save the timestamp file if the calculated hash is the expected one.
    # The expected hash may be shorter, to reduce path lengths, in which case
    # just compare that many characters.
    if expected_hash and digest.hexdigest().startswith(expected_hash):
        SaveTimestampsAndHash(root, digest.hexdigest())
        # Return the (potentially truncated) expected_hash.
        return expected_hash
    return digest.hexdigest()


def CalculateToolchainHashes(root, remove_corrupt_toolchains):
    """Calculate the hash of the different toolchains installed in the |root|
    directory."""
    hashes = []
    dir_list = [
        d for d in os.listdir(root) if os.path.isdir(os.path.join(root, d))
    ]
    for d in dir_list:
        toolchain_hash = CalculateHash(root, d)
        if toolchain_hash != d:
            print(
                'The hash of a version of the toolchain has an unexpected value ('
                '%s instead of %s)%s.' %
                (toolchain_hash, d,
                 ', removing it' if remove_corrupt_toolchains else ''))
            if remove_corrupt_toolchains:
                RemoveToolchain(root, d, True)
        else:
            hashes.append(toolchain_hash)
    return hashes


def SaveTimestampsAndHash(root, sha1):
    """Saves timestamps and the final hash to be able to early-out more quickly
    next time."""
    file_list = GetFileList(os.path.join(root, sha1))
    timestamps_data = {
        'files': [[f, os.path.getmtime(f)] for f in file_list],
        'sha1': sha1,
    }
    with open(MakeTimestampsFileName(root, sha1), 'wb') as f:
        f.write(json.dumps(timestamps_data).encode('utf-8'))


def HaveSrcInternalAccess():
    """Checks whether access to src-internal is available."""
    with open(os.devnull, 'w') as nul:
        # This is required to avoid modal dialog boxes after Git 2.14.1 and Git
        # Credential Manager for Windows 1.12. See https://crbug.com/755694 and
        # https://github.com/Microsoft/Git-Credential-Manager-for-Windows/issues/482.
        child_env = dict(os.environ, GCM_INTERACTIVE='NEVER')
        # If this script is run from an embedded terminal in VSCode, VSCode may
        # intercept the git call and show an easily-missable username/passsword
        # prompt. To ensure we can run without user input, just return false if
        # we don't get a response quickly. See crbug.com/376067358.
        try:
            return subprocess.call(
                [
                    'git', '-c', 'core.askpass=true', 'remote', 'show',
                    'https://chrome-internal.googlesource.com/chrome/src-internal/'
                ],
                shell=True,
                stdin=nul,
                stdout=nul,
                stderr=nul,
                timeout=10,  # seconds
                env=child_env) == 0
        except subprocess.TimeoutExpired:
            return False


def LooksLikeGoogler():
    """Checks for a USERDOMAIN environment variable of 'GOOGLE', which
    probably implies the current user is a Googler."""
    return os.environ.get('USERDOMAIN', '').upper() == 'GOOGLE'


def CanAccessToolchainBucket():
    """Checks whether the user has access to gs://chrome-wintoolchain/."""
    gsutil = download_from_google_storage.Gsutil(
        download_from_google_storage.GSUTIL_DEFAULT_PATH, boto_path=None)
    code, stdout, stderr = gsutil.check_call('ls', 'gs://chrome-wintoolchain/')
    if code != 0:
        # Make sure any error messages are made visible to the user.
        print(stderr, file=sys.stderr, end='')
        print(stdout, end='')
    return code == 0


def ToolchainBaseURL():
    base_url = os.environ.get('DEPOT_TOOLS_WIN_TOOLCHAIN_BASE_URL', '')
    if base_url.startswith('file://'):
        base_url = base_url[len('file://'):]
    return base_url


def UsesToolchainFromFile():
    return os.path.isdir(ToolchainBaseURL())


def UsesToolchainFromHttp():
    url = ToolchainBaseURL()
    return url.startswith('http://') or url.startswith('https://')


def RequestGsAuthentication():
    """Requests that the user authenticate to be able to access gs:// as a
    Googler. This allows much faster downloads, and pulling (old) toolchains
    that match src/ revisions.
    """
    print('Access to gs://chrome-wintoolchain/ not configured.')
    print('-----------------------------------------------------------------')
    print()
    print('You appear to be a Googler.')
    print()
    print('I\'m sorry for the hassle, but you need to do a one-time manual')
    print('authentication. Please run:')
    print()
    print('    download_from_google_storage --config')
    print()
    print('and follow the instructions.')
    print()
    print('NOTE 1: Use your google.com credentials, not chromium.org.')
    print('NOTE 2: Enter 0 when asked for a "project-id".')
    print()
    print('-----------------------------------------------------------------')
    print()
    sys.stdout.flush()
    sys.exit(1)


def DelayBeforeRemoving(target_dir):
    """A grace period before deleting the out of date toolchain directory."""
    if (os.path.isdir(target_dir)
            and not bool(int(os.environ.get('CHROME_HEADLESS', '0')))):
        for i in range(9, 0, -1):
            sys.stdout.write(
                '\rRemoving old toolchain in %ds... (Ctrl-C to cancel)' % i)
            sys.stdout.flush()
            time.sleep(1)
        print()


def DownloadUsingHttp(filename):
    """Downloads the given file from a url defined in
     DEPOT_TOOLS_WIN_TOOLCHAIN_BASE_URL environment variable."""
    temp_dir = tempfile.mkdtemp()
    assert os.path.basename(filename) == filename
    target_path = os.path.join(temp_dir, filename)
    base_url = ToolchainBaseURL()
    src_url = urljoin(base_url, filename)
    try:
        with closing(urlopen(src_url)) as fsrc, \
             open(target_path, 'wb') as fdst:
            shutil.copyfileobj(fsrc, fdst)
    except URLError as e:
        RmDir(temp_dir)
        sys.exit('Failed to retrieve file: %s' % e)
    return temp_dir, target_path


def DownloadUsingGsutil(filename):
    """Downloads the given file from Google Storage chrome-wintoolchain bucket."""
    temp_dir = tempfile.mkdtemp()
    assert os.path.basename(filename) == filename
    target_path = os.path.join(temp_dir, filename)
    gsutil = download_from_google_storage.Gsutil(
        download_from_google_storage.GSUTIL_DEFAULT_PATH, boto_path=None)
    code = gsutil.call('cp', 'gs://chrome-wintoolchain/' + filename,
                       target_path)
    if code != 0:
        sys.exit('gsutil failed')
    return temp_dir, target_path


def RmDir(path):
    """Deletes path and all the files it contains."""
    if sys.platform != 'win32':
        shutil.rmtree(path, ignore_errors=True)
    else:
        # shutil.rmtree() doesn't delete read-only files on Windows.
        subprocess.check_call('rmdir /s/q "%s"' % path, shell=True)


def DoTreeMirror(target_dir, tree_sha1):
    """In order to save temporary space on bots that do not have enough space to
    download ISOs, unpack them, and copy to the target location, the whole tree
    is uploaded as a zip to internal storage, and then mirrored here."""
    if UsesToolchainFromFile():
        temp_dir = None
        local_zip = os.path.join(ToolchainBaseURL(), tree_sha1 + '.zip')
        if not os.path.isfile(local_zip):
            sys.exit('%s is not a valid file.' % local_zip)
    elif UsesToolchainFromHttp():
        temp_dir, local_zip = DownloadUsingHttp(tree_sha1 + '.zip')
    else:
        temp_dir, local_zip = DownloadUsingGsutil(tree_sha1 + '.zip')
    sys.stdout.write('Extracting %s...\n' % local_zip)
    sys.stdout.flush()
    with zipfile.ZipFile(local_zip, 'r', zipfile.ZIP_DEFLATED, True) as zf:
        zf.extractall(target_dir)
    if temp_dir:
        RmDir(temp_dir)


def RemoveToolchain(root, sha1, delay_before_removing):
    """Remove the |sha1| version of the toolchain from |root|."""
    toolchain_target_dir = os.path.join(root, sha1)
    if delay_before_removing:
        DelayBeforeRemoving(toolchain_target_dir)
    if sys.platform == 'win32':
        # These stay resident and will make the rmdir below fail.
        kill_list = [
            'mspdbsrv.exe',
            'vctip.exe',  # Compiler and tools experience improvement data uploader.
        ]
        for process_name in kill_list:
            with open(os.devnull, 'wb') as nul:
                subprocess.call(['taskkill', '/f', '/im', process_name],
                                stdin=nul,
                                stdout=nul,
                                stderr=nul)
    if os.path.isdir(toolchain_target_dir):
        RmDir(toolchain_target_dir)

    timestamp_file = MakeTimestampsFileName(root, sha1)
    if os.path.exists(timestamp_file):
        os.remove(timestamp_file)


def RemoveUnusedToolchains(root):
    """Remove the versions of the toolchain that haven't been used recently."""
    valid_toolchains = []
    dirs_to_remove = []

    for d in os.listdir(root):
        full_path = os.path.join(root, d)
        if os.path.isdir(full_path):
            if not os.path.exists(MakeTimestampsFileName(root, d)):
                dirs_to_remove.append(d)
            else:
                vc_dir = os.path.join(full_path, 'VC')
                valid_toolchains.append((os.path.getmtime(vc_dir), d))
        elif os.path.isfile(full_path):
            os.remove(full_path)

    for d in dirs_to_remove:
        print('Removing %s as it doesn\'t correspond to any known toolchain.' %
              os.path.join(root, d))
        # Use the RemoveToolchain function to remove these directories as they
        # might contain an older version of the toolchain.
        RemoveToolchain(root, d, False)

    # Remove the versions of the toolchains that haven't been used in the past
    # 30 days.
    toolchain_expiration_time = 60 * 60 * 24 * 30
    for toolchain in valid_toolchains:
        toolchain_age_in_sec = time.time() - toolchain[0]
        if toolchain_age_in_sec > toolchain_expiration_time:
            print(
                'Removing version %s of the Win toolchain as it hasn\'t been used'
                ' in the past %d days.' %
                (toolchain[1], toolchain_age_in_sec / 60 / 60 / 24))
            RemoveToolchain(root, toolchain[1], True)


def EnableCrashDumpCollection():
    """Tell Windows Error Reporting to record crash dumps so that we can diagnose
    linker crashes and other toolchain failures. Documented at:
    https://msdn.microsoft.com/en-us/library/windows/desktop/bb787181.aspx
    """
    if sys.platform == 'win32' and os.environ.get('CHROME_HEADLESS') == '1':
        key_name = r'SOFTWARE\Microsoft\Windows\Windows Error Reporting'
        try:
            key = winreg.CreateKeyEx(
                winreg.HKEY_LOCAL_MACHINE, key_name, 0,
                winreg.KEY_WOW64_64KEY | winreg.KEY_ALL_ACCESS)
            # Merely creating LocalDumps is sufficient to enable the defaults.
            winreg.CreateKey(key, "LocalDumps")
            # Disable the WER UI, as documented here:
            # https://msdn.microsoft.com/en-us/library/windows/desktop/bb513638.aspx
            winreg.SetValueEx(key, "DontShowUI", 0, winreg.REG_DWORD, 1)
        # Trap OSError instead of WindowsError so pylint will succeed on Linux.
        # Catching errors is important because some build machines are not
        # elevated and writing to HKLM requires elevation.
        except OSError:
            pass


def main():
    parser = argparse.ArgumentParser(
        description=__doc__,
        formatter_class=argparse.RawDescriptionHelpFormatter,
    )
    parser.add_argument('--output-json',
                        metavar='FILE',
                        help='write information about toolchain to FILE')
    parser.add_argument('--force',
                        action='store_true',
                        help='force script to run on non-Windows hosts')
    parser.add_argument('--no-download',
                        action='store_true',
                        help='configure if present but don\'t download')
    parser.add_argument('--toolchain-dir',
                        default=os.getenv(ENV_TOOLCHAIN_ROOT, BASEDIR),
                        help='directory to install toolchain into')
    parser.add_argument('desired_hash',
                        metavar='desired-hash',
                        help='toolchain hash to download')
    args = parser.parse_args()

    if not (sys.platform.startswith(('cygwin', 'win32')) or args.force):
        return 0

    if sys.platform == 'cygwin':
        # This script requires Windows Python, so invoke with depot_tools'
        # Python.
        def winpath(path):
            return subprocess.check_output(['cygpath', '-w', path]).strip()

        python = os.path.join(DEPOT_TOOLS_PATH, 'python3.bat')
        cmd = [python, winpath(__file__)]
        if args.output_json:
            cmd.extend(['--output-json', winpath(args.output_json)])
        cmd.append(args.desired_hash)
        sys.exit(subprocess.call(cmd))
    assert sys.platform != 'cygwin'

    # Create our toolchain destination and "chdir" to it.
    toolchain_dir = os.path.abspath(args.toolchain_dir)
    if not os.path.isdir(toolchain_dir):
        os.makedirs(toolchain_dir)
    os.chdir(toolchain_dir)

    # Move to depot_tools\win_toolchain where we'll store our files, and where
    # the downloader script is.
    target_dir = 'vs_files'
    if not os.path.isdir(target_dir):
        os.mkdir(target_dir)
    toolchain_target_dir = os.path.join(target_dir, args.desired_hash)

    abs_toolchain_target_dir = os.path.abspath(toolchain_target_dir)

    got_new_toolchain = False

    # If the current hash doesn't match what we want in the file, nuke and pave.
    # Typically this script is only run when the .sha1 one file is updated, but
    # directly calling "gclient runhooks" will also run it, so we cache
    # based on timestamps to make that case fast.
    current_hashes = CalculateToolchainHashes(target_dir, True)
    if args.desired_hash not in current_hashes:
        if args.no_download:
            raise SystemExit(
                'Toolchain is out of date. Run "gclient runhooks" to '
                'update the toolchain, or set '
                'DEPOT_TOOLS_WIN_TOOLCHAIN=0 to use the locally '
                'installed toolchain.')
        should_use_file = False
        should_use_http = False
        should_use_gs = False
        if UsesToolchainFromFile():
            should_use_file = True
        elif UsesToolchainFromHttp():
            should_use_http = True
        elif (HaveSrcInternalAccess() or LooksLikeGoogler()
              or CanAccessToolchainBucket()):
            should_use_gs = True
            if not CanAccessToolchainBucket():
                RequestGsAuthentication()
        if not should_use_file and not should_use_gs and not should_use_http:
            if sys.platform not in ('win32', 'cygwin'):
                doc = 'https://chromium.googlesource.com/chromium/src/+/HEAD/docs/' \
                      'win_cross.md'
                print('\n\n\nPlease follow the instructions at %s\n\n' % doc)
            else:
                doc = 'https://chromium.googlesource.com/chromium/src/+/HEAD/docs/' \
                      'windows_build_instructions.md'
                print(
                    '\n\n\nNo downloadable toolchain found. In order to use your '
                    'locally installed version of Visual Studio to build Chrome '
                    'please set DEPOT_TOOLS_WIN_TOOLCHAIN=0.\n'
                    'For details search for DEPOT_TOOLS_WIN_TOOLCHAIN in the '
                    'instructions at %s\n\n' % doc)
            return 1
        print(
            'Windows toolchain out of date or doesn\'t exist, updating (Pro)...'
        )
        print('  current_hashes: %s' % ', '.join(current_hashes))
        print('  desired_hash: %s' % args.desired_hash)
        sys.stdout.flush()

        DoTreeMirror(toolchain_target_dir, args.desired_hash)

        got_new_toolchain = True

    # The Windows SDK is either in `win_sdk` or in `Windows Kits\10`. This
    # script must work with both layouts, so check which one it is.
    win_sdk_in_windows_kits = os.path.isdir(
        os.path.join(abs_toolchain_target_dir, 'Windows Kits', '10'))
    if win_sdk_in_windows_kits:
        win_sdk = os.path.join(abs_toolchain_target_dir, 'Windows Kits', '10')
    else:
        win_sdk = os.path.join(abs_toolchain_target_dir, 'win_sdk')

    version_file = os.path.join(toolchain_target_dir, 'VS_VERSION')
    vc_dir = os.path.join(toolchain_target_dir, 'VC')
    with open(version_file, 'rb') as f:
        vs_version = f.read().decode('utf-8').strip()
        # Touch the VC directory so we can use its timestamp to know when this
        # version of the toolchain has been used for the last time.
    os.utime(vc_dir, None)

    data = {
        'path':
        abs_toolchain_target_dir,
        'version':
        vs_version,
        'win_sdk':
        win_sdk,
        'wdk':
        os.path.join(abs_toolchain_target_dir, 'wdk'),
        'runtime_dirs': [
            os.path.join(abs_toolchain_target_dir, 'sys64'),
            os.path.join(abs_toolchain_target_dir, 'sys32'),
            os.path.join(abs_toolchain_target_dir, 'sysarm64'),
        ],
    }
    data_json = json.dumps(data, indent=2)
    data_path = os.path.join(target_dir, '..', 'data.json')
    if not os.path.exists(data_path) or open(data_path).read() != data_json:
        with open(data_path, 'w') as f:
            f.write(data_json)

    if got_new_toolchain:
        current_hashes = CalculateToolchainHashes(target_dir, False)
        if args.desired_hash not in current_hashes:
            print('Got wrong hash after pulling a new toolchain. '
                  'Wanted \'%s\', got one of \'%s\'.' %
                  (args.desired_hash, ', '.join(current_hashes)),
                  file=sys.stderr)
            return 1
        SaveTimestampsAndHash(target_dir, args.desired_hash)

    if args.output_json:
        if (not os.path.exists(args.output_json)
                or not filecmp.cmp(data_path, args.output_json)):
            shutil.copyfile(data_path, args.output_json)

    EnableCrashDumpCollection()

    RemoveUnusedToolchains(target_dir)

    return 0


if __name__ == '__main__':
    sys.exit(main())
