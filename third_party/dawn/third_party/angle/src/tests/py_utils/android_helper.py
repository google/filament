# Copyright 2022 The ANGLE Project Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import contextlib
import functools
import glob
import hashlib
import json
import logging
import os
import pathlib
import platform
import posixpath
import random
import re
import subprocess
import sys
import tarfile
import tempfile
import threading
import time
import zipfile

import angle_path_util


ANGLE_TRACE_TEST_SUITE = 'angle_trace_tests'


class _Global(object):
    initialized = False
    is_android = False
    current_suite = None
    current_user = None
    external_storage = None
    has_adb_root = False
    traces_outside_of_apk = False
    base_dir = None
    temp_dir = None
    use_run_as = True

    @classmethod
    def IsMultiUser(cls):
        assert cls.current_user != None, "Call _GetCurrentUser before using IsMultiUser"
        return cls.current_user != '0'

def _ApkPath(suite_name):
    return os.path.join('%s_apk' % suite_name, '%s-debug.apk' % suite_name)


def _RemovePrefix(str, prefix):
    assert str.startswith(prefix), 'Expected prefix %s, got: %s' % (prefix, str)
    return str[len(prefix):]


def _InitializeAndroid(apk_path):
    # Pull a few pieces of data with a single adb trip
    shell_id, su_path, current_user, data_permissions = _AdbShell(
        'id -u; which su || echo noroot; am get-current-user; stat --format %a /data').decode(
        ).strip().split('\n')

    # Populate globals with those results
    _Global.has_adb_root = _GetAdbRoot(shell_id, su_path)
    _Global.current_user = _GetCurrentUser(current_user)
    _Global.use_run_as = _GetRunAs(data_permissions)

    # Storage location varies by user
    _Global.external_storage = '/storage/emulated/' + _Global.current_user + '/chromium_tests_root/'

    # We use the app's home directory for storing several things
    _Global.base_dir = '/data/user/' + _Global.current_user + '/com.android.angle.test/'

    if _Global.has_adb_root:
        # /data/local/tmp/ is not writable by apps.. So use the app path
        _Global.temp_dir = _Global.base_dir + 'tmp/'
        # Additionally, if we're not the default user, we need to use the app's dir for external storage
        if _Global.IsMultiUser():
            # TODO(b/361388557): Switch to a content provider for this, i.e. `content write`
            logging.warning(
                'Using app dir for external storage, may not work with chromium scripts, may require `setenforce 0`'
            )
            _Global.external_storage = _Global.base_dir + 'chromium_tests_root/'
    else:
        # /sdcard/ is slow (see https://crrev.com/c/3615081 for details)
        # logging will be fully-buffered, can be truncated on crashes
        _Global.temp_dir = '/storage/emulated/' + _Global.current_user + '/'

    logging.debug('Temp dir: %s', _Global.temp_dir)
    logging.debug('External storage: %s', _Global.external_storage)

    with zipfile.ZipFile(apk_path) as zf:
        apk_so_libs = [posixpath.basename(f) for f in zf.namelist() if f.endswith('.so')]

    # When traces are outside of the apk this lib is also outside
    interpreter_so_lib = 'libangle_trace_interpreter.so'
    _Global.traces_outside_of_apk = interpreter_so_lib not in apk_so_libs

    if logging.getLogger().isEnabledFor(logging.DEBUG):
        logging.debug(_AdbShell('df -h').decode())


def Initialize(suite_name):
    if _Global.initialized:
        return

    apk_path = _ApkPath(suite_name)
    if os.path.exists(apk_path):
        _Global.is_android = True
        _InitializeAndroid(apk_path)

    _Global.initialized = True


def IsAndroid():
    assert _Global.initialized, 'Initialize not called'
    return _Global.is_android


def _EnsureTestSuite(suite_name):
    assert IsAndroid()

    if _Global.current_suite != suite_name:
        PrepareTestSuite(suite_name)
        _Global.current_suite = suite_name


def _Run(cmd):
    logging.debug('Executing command: %s', cmd)
    startupinfo = None
    if hasattr(subprocess, 'STARTUPINFO'):
        # Prevent console window popping up on Windows
        startupinfo = subprocess.STARTUPINFO()
        startupinfo.dwFlags |= subprocess.STARTF_USESHOWWINDOW
        startupinfo.wShowWindow = subprocess.SW_HIDE
    output = subprocess.check_output(cmd, startupinfo=startupinfo)
    return output


@functools.lru_cache()
def FindAdb():
    if platform.system() == 'Windows':
        adb = 'adb.exe'  # from PATH
    else:
        platform_tools = (
            pathlib.Path(angle_path_util.ANGLE_ROOT_DIR) / 'third_party' / 'android_sdk' /
            'public' / 'platform-tools')
        adb = str(platform_tools / 'adb') if platform_tools.exists() else 'adb'

    adb_info = ', '.join(subprocess.check_output([adb, '--version']).decode().strip().split('\n'))
    logging.info('adb --version: %s', adb_info)
    return adb


def _AdbRun(args):
    return _Run([FindAdb()] + args)


def _AdbShell(cmd):
    output = _Run([FindAdb(), 'shell', cmd])
    if platform.system() == 'Windows':
        return output.replace(b'\r\n', b'\n')
    return output


def _GetAdbRoot(shell_id, su_path):
    if int(shell_id) == 0:
        logging.info('adb already got root')
        return True

    if su_path == 'noroot':
        logging.warning('adb root not available on this device')
        return False

    logging.info('Getting adb root (may take a few seconds)')
    _AdbRun(['root'])
    for _ in range(20):  # `adb root` restarts adbd which can take quite a few seconds
        time.sleep(0.5)
        id_out = _AdbShell('id -u').decode('ascii').strip()
        if id_out == '0':
            logging.info('adb root succeeded')
            return True

    # Device has "su" but we couldn't get adb root. Something is wrong.
    raise Exception('Failed to get adb root')


def _GetRunAs(data_permissions):
    # Determine run-as usage
    if data_permissions.endswith('7'):
        # run-as broken due to "/data readable or writable by others"
        logging.warning('run-as not available due to /data permissions')
        return False

    if _Global.IsMultiUser():
        # run-as is failing is the presence of multiple users
        logging.warning('Disabling run-as for non-default user')
        return False

    return True


def _GetCurrentUser(current_user):
    # Ensure current user is clean
    assert current_user.isnumeric(), current_user
    logging.debug('Current user: %s', current_user)
    return current_user


def _ReadDeviceFile(device_path):
    with _TempLocalFile() as tempfile_path:
        _AdbRun(['pull', device_path, tempfile_path])
        with open(tempfile_path, 'rb') as f:
            return f.read()


def _RemoveDeviceFile(device_path):
    _AdbShell('rm -f ' + device_path + ' || true')  # ignore errors


def _MakeTar(path, patterns):
    with _TempLocalFile() as tempfile_path:
        with tarfile.open(tempfile_path, 'w', format=tarfile.GNU_FORMAT) as tar:
            for p in patterns:
                for f in glob.glob(p, recursive=True):
                    tar.add(f, arcname=f.replace('../../', ''))
        _AdbRun(['push', tempfile_path, path])


def _AddRestrictedTracesJson():
    _MakeTar(_Global.external_storage + 't.tar', [
        '../../src/tests/restricted_traces/*/*.json',
        'gen/trace_list.json',
    ])
    _AdbShell('r=' + _Global.external_storage + '; tar -xf $r/t.tar -C $r/ && rm $r/t.tar')


def _AddDeqpFiles(suite_name):
    patterns = [
        '../../third_party/VK-GL-CTS/src/external/openglcts/data/gl_cts/data/mustpass/*/*/main/*.txt',
        '../../src/tests/deqp_support/*.txt'
    ]
    if '_gles2_' in suite_name:
        patterns.append('gen/vk_gl_cts_data/data/gles2/**')
    elif '_gles3_' in suite_name:
        patterns.append('gen/vk_gl_cts_data/data/gles3/**')
        patterns.append('gen/vk_gl_cts_data/data/gl_cts/data/gles3/**')
    elif '_gles31_' in suite_name:
        patterns.append('gen/vk_gl_cts_data/data/gles31/**')
        patterns.append('gen/vk_gl_cts_data/data/gl_cts/data/gles31/**')
    elif '_gles32_' in suite_name:
        patterns.append('gen/vk_gl_cts_data/data/gl_cts/data/gles32/**')
    else:
        # Harness crashes if vk_gl_cts_data/data dir doesn't exist, so add a file
        patterns.append('gen/vk_gl_cts_data/data/gles2/data/brick.png')

    _MakeTar(_Global.external_storage + 'deqp.tar', patterns)
    _AdbShell('r=' + _Global.external_storage + '; tar -xf $r/deqp.tar -C $r/ && rm $r/deqp.tar')


def _GetDeviceApkPath():
    pm_path = _AdbShell('pm path com.android.angle.test || true').decode().strip()
    if not pm_path:
        logging.debug('No installed path found for com.android.angle.test')
        return None
    device_apk_path = _RemovePrefix(pm_path, 'package:')
    logging.debug('Device APK path is %s' % device_apk_path)
    return device_apk_path


def _LocalFileHash(local_path, gz_tail_size):
    h = hashlib.sha256()
    with open(local_path, 'rb') as f:
        if local_path.endswith('.gz'):
            # equivalent of tail -c {gz_tail_size}
            offset = os.path.getsize(local_path) - gz_tail_size
            if offset > 0:
                f.seek(offset)
        for data in iter(lambda: f.read(65536), b''):
            h.update(data)
    return h.hexdigest()


def _CompareHashes(local_path, device_path):
    # The last 8 bytes of gzip contain CRC-32 and the initial file size and the preceding
    # bytes should be affected by changes in the middle if we happen to run into a collision
    gz_tail_size = 4096

    if local_path.endswith('.gz'):
        cmd = 'test -f {path} && tail -c {gz_tail_size} {path} | sha256sum -b || true'.format(
            path=device_path, gz_tail_size=gz_tail_size)
    else:
        cmd = 'test -f {path} && sha256sum -b {path} || true'.format(path=device_path)

    if _Global.use_run_as and device_path.startswith('/data'):
        # Use run-as for files that reside on /data, which aren't accessible without root
        cmd = "run-as com.android.angle.test sh -c '{cmd}'".format(cmd=cmd)

    device_hash = _AdbShell(cmd).decode().strip()
    if not device_hash:
        logging.debug('_CompareHashes: File not found on device')
        return False  # file not on device

    return _LocalFileHash(local_path, gz_tail_size) == device_hash


def _CheckSameApkInstalled(apk_path):
    device_apk_path = _GetDeviceApkPath()

    try:
        if device_apk_path and _CompareHashes(apk_path, device_apk_path):
            return True
    except subprocess.CalledProcessError as e:
        # non-debuggable test apk installed on device breaks run-as
        logging.warning('_CompareHashes of apk failed: %s' % e)

    return False


def PrepareTestSuite(suite_name):
    apk_path = _ApkPath(suite_name)

    if _CheckSameApkInstalled(apk_path):
        logging.info('Skipping APK install because host and device hashes match')
    else:
        logging.info('Installing apk path=%s size=%s' % (apk_path, os.path.getsize(apk_path)))
        _AdbRun(['install', '-r', '-d', apk_path])

    permissions = [
        'android.permission.CAMERA', 'android.permission.CHANGE_CONFIGURATION',
        'android.permission.READ_EXTERNAL_STORAGE', 'android.permission.RECORD_AUDIO',
        'android.permission.WRITE_EXTERNAL_STORAGE'
    ]
    _AdbShell('for q in %s;do pm grant com.android.angle.test "$q";done;' %
              (' '.join(permissions)))

    _AdbShell('appops set com.android.angle.test MANAGE_EXTERNAL_STORAGE allow || true')

    _AdbShell('mkdir -p ' + _Global.external_storage)
    _AdbShell('mkdir -p %s' % _Global.temp_dir)

    if suite_name == ANGLE_TRACE_TEST_SUITE:
        _AddRestrictedTracesJson()

    if '_deqp_' in suite_name:
        _AddDeqpFiles(suite_name)

    if suite_name == 'angle_end2end_tests':
        _AdbRun([
            'push', '../../src/tests/angle_end2end_tests_expectations.txt',
            _Global.external_storage + 'src/tests/angle_end2end_tests_expectations.txt'
        ])


def PrepareRestrictedTraces(traces):
    start = time.time()
    total_size = 0
    skipped = 0

    # In order to get files to the app's home directory and loadable as libraries, we must first
    # push them to tmp on the device.  We then use `run-as` which allows copying files from tmp.
    # Note that `mv` is not allowed with `run-as`.  This means there will briefly be two copies
    # of the trace on the device, so keep that in mind as space becomes a problem in the future.
    app_tmp_path = '/data/local/tmp/angle_traces/'

    if _Global.use_run_as:
        _AdbShell('mkdir -p ' + app_tmp_path +
                  ' && run-as com.android.angle.test mkdir -p angle_traces')
    else:
        _AdbShell('mkdir -p ' + app_tmp_path + ' ' + _Global.base_dir + 'angle_traces/')

    def _HashesMatch(local_path, device_path):
        nonlocal total_size, skipped
        if _CompareHashes(local_path, device_path):
            skipped += 1
            return True
        else:
            total_size += os.path.getsize(local_path)
            return False

    def _Push(local_path, path_from_root):
        device_path = _Global.external_storage + path_from_root
        if not _HashesMatch(local_path, device_path):
            _AdbRun(['push', local_path, device_path])

    def _PushLibToAppDir(lib_name):
        local_path = lib_name
        if not os.path.exists(local_path):
            print('Error: missing library: ' + local_path)
            print('Is angle_restricted_traces set in gn args?')  # b/294861737
            sys.exit(1)

        device_path = _Global.base_dir + 'angle_traces/' + lib_name
        if _HashesMatch(local_path, device_path):
            return

        if _Global.use_run_as:
            tmp_path = posixpath.join(app_tmp_path, lib_name)
            logging.debug('_PushToAppDir: Pushing %s to %s' % (local_path, tmp_path))
            try:
                _AdbRun(['push', local_path, tmp_path])
                _AdbShell('run-as com.android.angle.test cp ' + tmp_path + ' ./angle_traces/')
                _AdbShell('rm ' + tmp_path)
            finally:
                _RemoveDeviceFile(tmp_path)
        else:
            _AdbRun(['push', local_path, _Global.base_dir + 'angle_traces/'])

    # Set up each trace
    for idx, trace in enumerate(sorted(traces)):
        logging.info('Syncing %s trace (%d/%d)', trace, idx + 1, len(traces))

        path_from_root = 'src/tests/restricted_traces/' + trace + '/' + trace + '.angledata.gz'
        _Push('../../' + path_from_root, path_from_root)

        if _Global.traces_outside_of_apk:
            lib_name = 'libangle_restricted_traces_' + trace + '.so'
            _PushLibToAppDir(lib_name)

        tracegz = 'gen/tracegz_' + trace + '.gz'
        if os.path.exists(tracegz):  # Requires angle_enable_tracegz
            _Push(tracegz, tracegz)

    # Push one additional file when running outside the APK
    if _Global.traces_outside_of_apk:
        _PushLibToAppDir('libangle_trace_interpreter.so')

    logging.info('Synced files for %d traces (%.1fMB, %d files already ok) in %.1fs', len(traces),
                 total_size / 1e6, skipped,
                 time.time() - start)


def _RandomHex():
    return hex(random.randint(0, 2**64))[2:]


@contextlib.contextmanager
def _TempDeviceDir():
    path = posixpath.join(_Global.temp_dir, 'temp_dir-%s' % _RandomHex())
    _AdbShell('mkdir -p ' + path)
    try:
        yield path
    finally:
        _AdbShell('rm -rf ' + path)


@contextlib.contextmanager
def _TempDeviceFile():
    path = posixpath.join(_Global.temp_dir, 'temp_file-%s' % _RandomHex())
    try:
        yield path
    finally:
        _AdbShell('rm -f ' + path)


@contextlib.contextmanager
def _TempLocalFile():
    fd, path = tempfile.mkstemp()
    os.close(fd)
    try:
        yield path
    finally:
        os.remove(path)


def _SetCaptureProps(env, device_out_dir):
    capture_var_map = {  # src/libANGLE/capture/FrameCapture.cpp
        'ANGLE_CAPTURE_ENABLED': 'debug.angle.capture.enabled',
        'ANGLE_CAPTURE_FRAME_START': 'debug.angle.capture.frame_start',
        'ANGLE_CAPTURE_FRAME_END': 'debug.angle.capture.frame_end',
        'ANGLE_CAPTURE_TRIGGER': 'debug.angle.capture.trigger',
        'ANGLE_CAPTURE_LABEL': 'debug.angle.capture.label',
        'ANGLE_CAPTURE_COMPRESSION': 'debug.angle.capture.compression',
        'ANGLE_CAPTURE_VALIDATION': 'debug.angle.capture.validation',
        'ANGLE_CAPTURE_VALIDATION_EXPR': 'debug.angle.capture.validation_expr',
        'ANGLE_CAPTURE_SOURCE_EXT': 'debug.angle.capture.source_ext',
        'ANGLE_CAPTURE_SOURCE_SIZE': 'debug.angle.capture.source_size',
        'ANGLE_CAPTURE_FORCE_SHADOW': 'debug.angle.capture.force_shadow',
    }
    empty_value = '""'
    shell_cmds = [
        # out_dir is special because the corresponding env var is a host path not a device path
        'setprop debug.angle.capture.out_dir ' + (device_out_dir or empty_value),
    ] + [
        'setprop %s %s' % (v, env.get(k, empty_value)) for k, v in sorted(capture_var_map.items())
    ]

    _AdbShell('\n'.join(shell_cmds))


def _RunInstrumentation(flags):
    with _TempDeviceFile() as temp_device_file:
        cmd = r'''
am instrument --user {user} -w \
    -e org.chromium.native_test.NativeTestInstrumentationTestRunner.StdoutFile {out} \
    -e org.chromium.native_test.NativeTest.CommandLineFlags "{flags}" \
    -e org.chromium.native_test.NativeTestInstrumentationTestRunner.ShardNanoTimeout "1000000000000000000" \
    -e org.chromium.native_test.NativeTestInstrumentationTestRunner.NativeTestActivity \
    com.android.angle.test.AngleUnitTestActivity \
    com.android.angle.test/org.chromium.build.gtest_apk.NativeTestInstrumentationTestRunner
        '''.format(
            user=_Global.current_user, out=temp_device_file, flags=r' '.join(flags)).strip()

        capture_out_dir = os.environ.get('ANGLE_CAPTURE_OUT_DIR')
        if capture_out_dir:
            assert os.path.isdir(capture_out_dir)
            with _TempDeviceDir() as device_out_dir:
                _SetCaptureProps(os.environ, device_out_dir)
                try:
                    _AdbShell(cmd)
                finally:
                    _SetCaptureProps({}, None)  # reset
                _PullDir(device_out_dir, capture_out_dir)
        else:
            _AdbShell(cmd)
        return _ReadDeviceFile(temp_device_file)


def AngleSystemInfo(args):
    _EnsureTestSuite('angle_system_info_test')

    with _TempDeviceDir() as temp_dir:
        _RunInstrumentation(args + ['--render-test-output-dir=' + temp_dir])
        output_file = posixpath.join(temp_dir, 'angle_system_info.json')
        return json.loads(_ReadDeviceFile(output_file))


def GetBuildFingerprint():
    return _AdbShell('getprop ro.build.fingerprint').decode('ascii').strip()


def _PullDir(device_dir, local_dir):
    files = _AdbShell('ls -1 %s' % device_dir).decode('ascii').split('\n')
    for f in files:
        f = f.strip()
        if f:
            _AdbRun(['pull', posixpath.join(device_dir, f), posixpath.join(local_dir, f)])


def _RemoveFlag(args, f):
    matches = [a for a in args if a.startswith(f + '=')]
    assert len(matches) <= 1
    if matches:
        original_value = matches[0].split('=')[1]
        args.remove(matches[0])
    else:
        original_value = None

    return original_value


def RunTests(test_suite, args, stdoutfile=None, log_output=True):
    _EnsureTestSuite(test_suite)

    args = args[:]
    test_output_path = _RemoveFlag(args, '--isolated-script-test-output')
    perf_output_path = _RemoveFlag(args, '--isolated-script-test-perf-output')
    test_output_dir = _RemoveFlag(args, '--render-test-output-dir')

    result = 0
    output = b''
    output_json = {}
    try:
        with contextlib.ExitStack() as stack:
            device_test_output_path = stack.enter_context(_TempDeviceFile())
            args.append('--isolated-script-test-output=' + device_test_output_path)

            if perf_output_path:
                device_perf_path = stack.enter_context(_TempDeviceFile())
                args.append('--isolated-script-test-perf-output=%s' % device_perf_path)

            if test_output_dir:
                assert os.path.isdir(test_output_dir), 'Dir does not exist: %s' % test_output_dir
                device_output_dir = stack.enter_context(_TempDeviceDir())
                args.append('--render-test-output-dir=' + device_output_dir)

            output = _RunInstrumentation(args)

            if '--list-tests' in args:
                # When listing tests, there may be no output file. We parse stdout anyways.
                test_output = b'{"interrupted": false}'
            else:
                try:
                    test_output = _ReadDeviceFile(device_test_output_path)
                except subprocess.CalledProcessError:
                    logging.error('Unable to read test json output. Stdout:\n%s', output.decode())
                    result = 1
                    return result, output.decode(), None

            if test_output_path:
                with open(test_output_path, 'wb') as f:
                    f.write(test_output)

            output_json = json.loads(test_output)

            num_failures = output_json.get('num_failures_by_type', {}).get('FAIL', 0)
            interrupted = output_json.get('interrupted', True)  # Normally set to False
            if num_failures != 0 or interrupted or output_json.get('is_unexpected', False):
                logging.error('Tests failed: %s', test_output.decode())
                result = 1

            if test_output_dir:
                _PullDir(device_output_dir, test_output_dir)

            if perf_output_path:
                _AdbRun(['pull', device_perf_path, perf_output_path])

        if log_output:
            logging.info(output.decode())

        if stdoutfile:
            with open(stdoutfile, 'wb') as f:
                f.write(output)
    except Exception as e:
        logging.exception(e)
        result = 1

    return result, output.decode(), output_json


def GetTraceFromTestName(test_name):
    if test_name.startswith('TraceTest.'):
        return test_name[len('TraceTest.'):]
    return None


def GetTemps():
    temps = _AdbShell(
        'cat /dev/thermal/tz-by-name/*_therm/temp 2>/dev/null || true').decode().split()
    logging.debug('tz-by-name temps: %s' % ','.join(temps))

    temps_celsius = []
    for t in temps:
        try:
            temps_celsius.append(float(t) / 1e3)
        except ValueError:
            pass

    return temps_celsius
