# Copyright 2023 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
"""This helper provides a build context that handles
the reclient lifecycle safely. It will automatically start
reproxy before running ninja and stop reproxy when build stops
for any reason e.g. build completion, keyboard interrupt etc."""

import atexit
import contextlib
import datetime
import hashlib
import os
import shutil
import socket
import subprocess
import sys
import time
import uuid

import gclient_paths
import ninja
import siso

THIS_DIR = os.path.dirname(__file__)
RECLIENT_LOG_CLEANUP = os.path.join(THIS_DIR, 'reclient_log_cleanup.py')


def find_reclient_bin_dir():
    tools_path = gclient_paths.GetBuildtoolsPath()
    if not tools_path:
        return None

    reclient_bin_dir = os.path.join(tools_path, 'reclient')
    if os.path.isdir(reclient_bin_dir):
        return reclient_bin_dir
    return None


def find_reclient_cfg():
    tools_path = gclient_paths.GetBuildtoolsPath()
    if not tools_path:
        return None

    reclient_cfg = os.path.join(tools_path, 'reclient_cfgs', 'reproxy.cfg')
    if os.path.isfile(reclient_cfg):
        return reclient_cfg
    return None


def run(cmd_args):
    if os.environ.get('NINJA_SUMMARIZE_BUILD') == '1':
        print(' '.join(cmd_args))
    return subprocess.call(cmd_args)


def start_reproxy(reclient_cfg, reclient_bin_dir):
    return run([
        os.path.join(reclient_bin_dir,
                     'bootstrap' + gclient_paths.GetExeSuffix()),
        '--re_proxy=' + os.path.join(reclient_bin_dir,
                                     'reproxy' + gclient_paths.GetExeSuffix()),
        '--cfg=' + reclient_cfg
    ])


def stop_reproxy(reclient_cfg, reclient_bin_dir):
    return run([
        os.path.join(reclient_bin_dir,
                     'bootstrap' + gclient_paths.GetExeSuffix()), '--shutdown',
        '--cfg=' + reclient_cfg
    ])


def find_ninja_out_dir(args):
    # Ninja uses getopt_long, which allows to intermix non-option arguments.
    # To leave non supported parameters untouched, we do not use getopt.
    for index, arg in enumerate(args[1:]):
        if arg == '-C':
            # + 1 to get the next argument and +1 because we trimmed off args[0]
            return args[index + 2]
        if arg.startswith('-C'):
            # Support -Cout/Default
            return arg[2:]
    return '.'


def find_cache_dir(tmp_dir):
    """Helper to find the correct cache directory for a build.

    tmp_dir should be a build specific temp directory within the out directory.

    If this is called from within a gclient checkout, the cache dir will be:
    <gclient_root>/.reproxy_cache/md5(tmp_dir)/
    If this is not called from within a gclient checkout, the cache dir will be:
    tmp_dir/cache
    """
    gclient_root = gclient_paths.FindGclientRoot(os.getcwd())
    if gclient_root:
        return os.path.join(gclient_root, '.reproxy_cache',
                            hashlib.md5(tmp_dir.encode()).hexdigest())
    return os.path.join(tmp_dir, 'cache')


def auth_cache_status():
    cred_file = os.path.join(os.environ["RBE_cache_dir"], "reproxy.creds")
    if not os.path.isfile(cred_file):
        return "missing", "UNSPECIFIED"
    try:
        with open(cred_file) as f:
            status = "valid"
            mechanism = "UNSPECIFIED"
            for line in f.readlines():
                if "seconds:" in line:
                    exp = int(line.strip()[len("seconds:"):].strip())
                    if exp < (time.time() + 5 * 60):
                        status = "expired"
                elif "mechanism:" in line:
                    mechanism = line.strip()[len("mechanism:"):].strip()
            return status, mechanism
    except OSError:
        return "missing", "UNSPECIFIED"


def get_hostname():
    hostname = socket.gethostname()
    # In case that returned an address, make a best effort attempt to get
    # the hostname and ignore any errors.
    try:
        return socket.gethostbyaddr(hostname)[0]
    except Exception:
        return hostname


def set_reproxy_metrics_flags(tool):
    """Helper to setup metrics collection flags for reproxy.

    The following env vars are set if not already set:
        RBE_metrics_project=chromium-reclient-metrics
        RBE_invocation_id=$AUTONINJA_BUILD_ID
        RBE_metrics_table=rbe_metrics.builds
        RBE_metrics_labels=\
                source=developer,\
                tool={tool},\
                creds_cache_status={auth_status},\
                creds_cache_mechanism={auth_mechanism},\
                host={host}
        RBE_metrics_prefix=go.chromium.org
    """
    autoninja_id = os.environ.get("AUTONINJA_BUILD_ID")
    if autoninja_id is not None:
        os.environ.setdefault("RBE_invocation_id", autoninja_id)
    os.environ.setdefault("RBE_metrics_project", "chromium-reclient-metrics")
    os.environ.setdefault("RBE_metrics_table", "rbe_metrics.builds")
    labels = "source=developer,tool=" + tool
    auth_status, auth_mechanism = auth_cache_status()
    labels += ",creds_cache_status=" + auth_status
    labels += ",creds_cache_mechanism=" + auth_mechanism
    labels += ",host=" + get_hostname()
    os.environ.setdefault("RBE_metrics_labels", labels)
    os.environ.setdefault("RBE_metrics_prefix", "go.chromium.org")


def remove_mdproxy_from_path():
    os.environ["PATH"] = os.pathsep.join(
        d for d in os.environ.get("PATH", "").split(os.pathsep)
        if "mdproxy" not in d)


# Mockable datetime.datetime.utcnow for testing.
def datetime_now():
    return datetime.datetime.utcnow()


# Deletes the tree at dir if it exists.
def rmtree_if_exists(rm_dir):
    if os.path.exists(rm_dir) and os.path.isdir(rm_dir):
        shutil.rmtree(rm_dir, ignore_errors=True)


def set_reproxy_path_flags(out_dir, make_dirs=True):
    """Helper to setup the logs and cache directories for reclient.

    Creates the following directory structure if make_dirs is true:
    If in a gclient checkout
    out_dir/
        .reproxy_tmp/
        logs/
    <gclient_root>
        .reproxy_cache/
        md5(out_dir/.reproxy_tmp)/

    If not in a gclient checkout
    out_dir/
        .reproxy_tmp/
        logs/
        cache/

    The following env vars are set if not already set:
        RBE_output_dir=out_dir/.reproxy_tmp/logs
        RBE_proxy_log_dir=out_dir/.reproxy_tmp/logs
        RBE_log_dir=out_dir/.reproxy_tmp/logs
        RBE_cache_dir=out_dir/.reproxy_tmp/cache
    *Nix Only:
        RBE_server_address=unix://out_dir/.reproxy_tmp/reproxy.sock
    Windows Only:
        RBE_server_address=pipe://md5(out_dir/.reproxy_tmp)/reproxy.pipe
    """
    os.environ.setdefault("AUTONINJA_BUILD_ID", str(uuid.uuid4()))
    run_sub_dir = datetime_now().strftime(
        '%Y%m%dT%H%M%S.%f') + "_" + os.environ["AUTONINJA_BUILD_ID"]
    tmp_dir = os.path.abspath(os.path.join(out_dir, '.reproxy_tmp'))
    log_dir = os.path.join(tmp_dir, 'logs')
    run_log_dir = os.path.join(log_dir, run_sub_dir)
    racing_dir = os.path.join(tmp_dir, 'racing')
    run_racing_dir = os.path.join(racing_dir, run_sub_dir)
    cache_dir = find_cache_dir(tmp_dir)

    atexit.register(rmtree_if_exists, run_racing_dir)

    if make_dirs:
        if os.path.isfile(os.path.join(log_dir, "rbe_metrics.txt")):
            try:
                # Delete entire log dir if it is in the old format
                # which had no subdirectories for each build.
                shutil.rmtree(log_dir)
            except OSError:
                print(
                    "Couldn't clear logs because reproxy did "
                    "not shutdown after the last build",
                    file=sys.stderr)
        os.makedirs(tmp_dir, exist_ok=True)
        os.makedirs(log_dir, exist_ok=True)
        os.makedirs(run_log_dir, exist_ok=True)
        os.makedirs(cache_dir, exist_ok=True)
        os.makedirs(racing_dir, exist_ok=True)
        os.makedirs(run_racing_dir, exist_ok=True)

    old_log_dirs = [
        d for d in os.listdir(log_dir)
        if os.path.isdir(os.path.join(log_dir, d))
    ]

    if len(old_log_dirs) > 5:
        old_log_dirs.sort(key=lambda dir: dir.split("_"), reverse=True)
        for d in old_log_dirs[5:]:
            shutil.rmtree(os.path.join(log_dir, d))

    os.environ.setdefault("RBE_output_dir", run_log_dir)
    os.environ.setdefault("RBE_proxy_log_dir", run_log_dir)
    os.environ.setdefault("RBE_log_dir", run_log_dir)
    os.environ.setdefault("RBE_cache_dir", cache_dir)
    os.environ.setdefault("RBE_racing_tmp_dir", run_racing_dir)
    if sys.platform.startswith('win'):
        pipe_dir = hashlib.sha256(run_log_dir.encode()).hexdigest()
        os.environ.setdefault("RBE_server_address",
                              "pipe://%s/reproxy.pipe" % pipe_dir)
    else:
        # unix domain socket has path length limit, so use fixed size path here.
        # ref: https://www.man7.org/linux/man-pages/man7/unix.7.html
        os.environ.setdefault(
            "RBE_server_address", "unix:///tmp/reproxy_%s.sock" %
            hashlib.sha256(run_log_dir.encode()).hexdigest())


def set_racing_defaults():
    os.environ.setdefault("RBE_local_resource_fraction", "0.2")
    os.environ.setdefault("RBE_racing_bias", "0.95")


def set_mac_defaults():
    # Reduce the cas concurrency on macs.  Lower value doesn't impact
    # performance when on high-speed connection, but does show improvements
    # on easily congested networks.
    os.environ.setdefault("RBE_cas_concurrency", "100")
    # Enable the deps cache on macs.  Mac needs a larger deps cache as it
    # seems to have larger dependency sets per action.
    os.environ.setdefault("RBE_enable_deps_cache", "true")
    os.environ.setdefault("RBE_deps_cache_max_mb", "1024")


def set_win_defaults():
    # Enable the deps cache on windows.  This makes a notable improvement
    # in performance at the cost of a ~200MB cache file.
    os.environ.setdefault("RBE_enable_deps_cache", "true")
    # Reduce local resource fraction used to do local compile actions on
    # windows, to try and prevent machine saturation.
    os.environ.setdefault("RBE_local_resource_fraction", "0.05")
    # Set execution strategy to remote_local_fallback while racing performance
    # on windows is addressed.
    os.environ.setdefault("RBE_exec_strategy", "remote_local_fallback")
    # Turn off creds caching for windows, as luci-auth as credshelper shouldn't
    # use it.
    os.environ.setdefault("RBE_enable_creds_cache", "false")
    # Extend timeouts on windows
    os.environ.setdefault("RBE_exec_timeout","4m")
    os.environ.setdefault("RBE_reclient_timeout","8m")


def workspace_is_cog():
    return sys.platform == "linux" and os.path.realpath(
        os.getcwd()).startswith("/google/cog")


# pylint: disable=line-too-long
def reclient_setup_docs_url():
    if sys.platform == "darwin":
        return "https://chromium.googlesource.com/chromium/src/+/main/docs/mac_build_instructions.md#use-reclient"
    if sys.platform.startswith("win"):
        return "https://chromium.googlesource.com/chromium/src/+/main/docs/windows_build_instructions.md#use-reclient"
    return "https://chromium.googlesource.com/chromium/src/+/main/docs/linux/build_instructions.md#use-reclient"


@contextlib.contextmanager
def build_context(argv, tool, should_collect_logs):
    # If use_remoteexec is set, but the reclient binaries or configs don't
    # exist, display an error message and stop.  Otherwise, the build will
    # attempt to run with rewrapper wrapping actions, but will fail with
    # possible non-obvious problems.
    reclient_bin_dir = find_reclient_bin_dir()
    reclient_cfg = find_reclient_cfg()
    if reclient_bin_dir is None or reclient_cfg is None:
        print(
            'Build is configured to use reclient but necessary binaries '
            "or config files can't be found.\n"
            'Please check if `"download_remoteexec_cfg": True` custom var is '
            'set in `.gclient`, and run `gclient sync`.',
            file=sys.stderr)
        yield 1
        return

    ninja_out = find_ninja_out_dir(argv)

    try:
        set_reproxy_path_flags(ninja_out)
    except OSError as e:
        print(f"Error creating reproxy_tmp in output dir: {e}", file=sys.stderr)
        yield 1
        return

    if should_collect_logs:
        set_reproxy_metrics_flags(tool)

    if os.environ.get('RBE_instance', None):
        print('WARNING: Using RBE_instance=%s\n' %
              os.environ.get('RBE_instance', ''))

    remote_disabled = os.environ.get('RBE_remote_disabled')
    if remote_disabled not in ('1', 't', 'T', 'true', 'TRUE', 'True'):
        # If we are building inside a Cog workspace, racing is likely not a
        # performance improvement, so we disable it by default.
        if workspace_is_cog():
            os.environ.setdefault("RBE_exec_strategy", "remote_local_fallback")
        set_racing_defaults()
        if sys.platform == "darwin":
            set_mac_defaults()
        if sys.platform.startswith("win"):
            set_win_defaults()

    # TODO(b/292523514) remove this once a fix is landed in reproxy
    remove_mdproxy_from_path()

    start = time.time()
    reproxy_ret_code = start_reproxy(reclient_cfg, reclient_bin_dir)
    if os.environ.get('NINJA_SUMMARIZE_BUILD') == '1':
        elapsed = time.time() - start
        print('%1.3fs to start reproxy' % elapsed)
    if reproxy_ret_code != 0:
        print(f'''Failed to start reproxy!
See above error message for details.
Ensure you have completed the reproxy setup instructions:
{reclient_setup_docs_url()}''',
              file=sys.stderr)
        yield reproxy_ret_code
        return
    try:
        yield
    finally:
        start = time.time()
        stop_reproxy(reclient_cfg, reclient_bin_dir)
        if os.environ.get('NINJA_SUMMARIZE_BUILD') == '1':
            elapsed = time.time() - start
            print('%1.3fs to stop reproxy' % elapsed)


def run_ninja(ninja_cmd, should_collect_logs=False):
    """Runs Ninja in build_context()."""
    # TODO: crbug.com/345113094 - rename the `tool` label to `ninja`.
    with build_context(ninja_cmd, "ninja_reclient",
                       should_collect_logs) as ret_code:
        if ret_code:
            return ret_code
        try:
            return ninja.main(ninja_cmd)
        except KeyboardInterrupt:
            print("Shutting down reproxy...", file=sys.stderr)
            return 1


def run_siso(siso_cmd, should_collect_logs=False):
    """Runs Siso in build_context()."""
    # TODO: crbug.com/345113094 - rename the `autosiso` label to `siso`.
    with build_context(siso_cmd, "autosiso", should_collect_logs) as ret_code:
        if ret_code:
            return ret_code
        try:
            return siso.main(siso_cmd)
        except KeyboardInterrupt:
            print("Shutting down reproxy...", file=sys.stderr)
            return 1
