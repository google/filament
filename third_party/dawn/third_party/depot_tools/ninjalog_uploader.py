#!/usr/bin/env python3
# Copyright 2018 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
"""
This is script to upload ninja_log from googler.

Server side implementation is in
https://cs.chromium.org/chromium/infra/go/src/infra/appengine/chromium_build_stats/

Uploaded ninjalog is stored in BigQuery table having following schema.
https://cs.chromium.org/chromium/infra/go/src/infra/appengine/chromium_build_stats/ninjaproto/ninjalog.proto

The log will be used to analyze user side build performance.

See also the privacy review. http://eldar/assessments/656778450
"""

import argparse
import getpass
import gzip
import http
import io
import json
import logging
import multiprocessing
import os
import pathlib
import platform
import re
import subprocess
import sys
import time
import urllib.request

import build_telemetry
import gclient_utils

# Configs that should not be uploaded as is.
SENSITIVE_CONFIGS = (
    "google_api_key",
    "google_default_client_id",
    "google_default_client_secret",
    "ios_credential_provider_extension_api_key",
    "ios_credential_provider_extension_client_id",
    "ios_encryption_export_compliance_code",
    "ios_google_test_oauth_client_id",
    "ios_google_test_oauth_client_secret",
)

def ParseGNArgs(gn_args):
    """Parse gn_args as json and return config dictionary."""
    configs = json.loads(gn_args)
    build_configs = {}
    explicit_keys = []
    user = getpass.getuser()

    for config in configs:
        key = config["name"]
        if "current" in config:
            value = config["current"]["value"]
            # Record configs specified in args.gn as explicit configs.
            if config["current"]["file"] != "//.gn":
                explicit_keys.append(key)
        else:
            value = config["default"]["value"]
        value = value.strip('"')
        if key in SENSITIVE_CONFIGS and value:
            value = '<omitted>'
        # Do not upload username.
        if os.path.isabs(value):
            value = os.path.join(*[
                p if p != user else "$USER" for p in pathlib.Path(value).parts
            ])
        build_configs[key] = value

    return build_configs, explicit_keys


def GetBuildTargetFromCommandLine(cmdline):
    """Get build targets from commandline."""

    # Skip argv0, argv1: ['/path/to/python3', '/path/to/depot_tools/ninja.py']
    idx = 2

    # Skipping all args that involve these flags, and taking all remaining args
    # as targets.
    onearg_flags = ("-C", "-d", "-f", "-j", "-k", "-l", "-p", "-t", "-w")
    zeroarg_flags = ("--version", "-n", "-v")

    targets = []

    while idx < len(cmdline):
        arg = cmdline[idx]
        if arg in onearg_flags:
            idx += 2
            continue

        if arg[:2] in onearg_flags or arg in zeroarg_flags:
            idx += 1
            continue

        # A target doesn't start with '-'.
        if arg.startswith("-"):
            idx += 1
            continue

        # Avoid uploading absolute paths accidentally. e.g. b/270907050
        if os.path.isabs(arg):
            idx += 1
            continue

        targets.append(arg)
        idx += 1

    return targets


def GetJflag(cmdline):
    """Parse cmdline to get flag value for -j"""

    for i in range(len(cmdline)):
        if (cmdline[i] == "-j" and i + 1 < len(cmdline)
                and cmdline[i + 1].isdigit()):
            return int(cmdline[i + 1])

        if cmdline[i].startswith("-j") and cmdline[i][len("-j"):].isdigit():
            return int(cmdline[i][len("-j"):])


def GetMetadata(cmdline, ninjalog, exit_code, build_duration, user):
    """Get metadata for uploaded ninjalog.

    Returned metadata has schema defined in
    https://cs.chromium.org?q="type+Metadata+struct+%7B"+file:%5Einfra/go/src/infra/appengine/chromium_build_stats/ninjalog/
    """

    build_dir = os.path.dirname(ninjalog)

    build_configs = {}
    explicit_keys = []

    try:
        args = ["gn", "args", build_dir, "--list", "--json"]
        if sys.platform == "win32":
            # gn in PATH is bat file in windows environment (except cygwin).
            args = ["cmd", "/c"] + args

        gn_args = subprocess.check_output(args)
        build_configs, explicit_keys = ParseGNArgs(gn_args)
    except subprocess.CalledProcessError as e:
        logging.error("Failed to call gn %s", e)
        build_configs = {}

    # Stringify config.
    for k in build_configs:
        build_configs[k] = str(build_configs[k])

    metadata = {
        "user": user,
        "exit_code": exit_code,
        "build_duration_sec": build_duration,
        "platform": platform.system(),
        "cpu_core": multiprocessing.cpu_count(),
        "is_cog": gclient_utils.IsEnvCog(),
        "is_cloudtop": False,
        "build_configs": build_configs,
        "explicit_build_config_keys": explicit_keys,
        "targets": GetBuildTargetFromCommandLine(cmdline),
    }

    metadata.update(GetGCEMetadata())

    invocation_id = os.environ.get("AUTONINJA_BUILD_ID")
    if invocation_id:
        metadata['invocation_id'] = invocation_id
    jflag = GetJflag(cmdline)
    if jflag is not None:
        metadata["jobs"] = jflag

    return metadata


def GetGCEMetadata():
    gce = _getGCEInfo()
    if not gce:
        return {}
    md = {}
    if "cloudtop" in gce.get("project", {}).get("projectId", ""):
        md["is_cloudtop"] = True
    match = re.search(r"machineTypes/([^/]+)",
                      gce.get("instance", {}).get("machineType", ""))
    if match:
        md["gce_machine_type"] = match.group(1)
    return md


def _getGCEInfo():
    url = "http://metadata.google.internal/computeMetadata/v1/?recursive=true"
    request = urllib.request.Request(url, headers={"Metadata-Flavor": "Google"})
    try:
        response = urllib.request.urlopen(request)
        meta = json.load(response)
    except Exception as e:
        # Only GCE machines can access to the metadata server.
        logging.warning(e)
        return
    return meta


def GetNinjalog(cmdline):
    """GetNinjalog returns the path to ninjalog from cmdline."""
    # ninjalog is in current working directory by default.
    ninjalog_dir = "."

    i = 0
    while i < len(cmdline):
        cmd = cmdline[i]
        i += 1
        if cmd == "-C" and i < len(cmdline):
            ninjalog_dir = cmdline[i]
            i += 1
            continue

        if cmd.startswith("-C") and len(cmd) > len("-C"):
            ninjalog_dir = cmd[len("-C"):]

    return os.path.join(ninjalog_dir, ".ninja_log")


def UploadNinjaLog(server, ninjalog, metadata):
    output = io.BytesIO()

    with open(ninjalog) as f:
        with gzip.GzipFile(fileobj=output, mode="wb") as g:
            g.write(f.read().encode())
            g.write(b"# end of ninja log\n")
            logging.info("send metadata: %s", json.dumps(metadata))
            g.write(json.dumps(metadata).encode())

    status = None
    err_msg = ""
    try:
        resp = urllib.request.urlopen(
            urllib.request.Request(
                "https://" + server + "/upload_ninja_log/",
                data=output.getvalue(),
                headers={"Content-Encoding": "gzip"},
            ))
        status = resp.status
        logging.info("response header: %s", resp.headers)
        logging.info("response content: %s", resp.read())
    except urllib.error.HTTPError as e:
        status = e.status
        err_msg = e.msg

    if status != http.HTTPStatus.OK:
        logging.warning(
            "unexpected status code for response: status: %s, msg: %s", status,
            err_msg)
        return 1

    return 0


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument(
        "--server",
        default="chromium-build-stats.appspot.com",
        help="server to upload ninjalog file.",
    )
    parser.add_argument("--ninjalog", help="ninjalog file to upload.")
    parser.add_argument("--verbose",
                        action="store_true",
                        help="Enable verbose logging.")
    parser.add_argument("--exit_code",
                        type=int,
                        help="exit code of the ninja command.")
    parser.add_argument("--build_duration",
                        type=int,
                        help="total duration spent on autoninja (secounds)")
    parser.add_argument(
        "--cmdline",
        required=True,
        nargs=argparse.REMAINDER,
        help="command line args passed to ninja.",
    )

    args = parser.parse_args()

    if args.verbose:
        logging.basicConfig(level=logging.INFO)
    else:
        # Disable logging.
        logging.disable(logging.CRITICAL)

    cfg = build_telemetry.load_config()
    if not cfg.is_googler:
        logging.warning("Not Googler. Only Googlers can upload ninjalog.")
        return 1

    ninjalog = args.ninjalog or GetNinjalog(args.cmdline)
    if not os.path.isfile(ninjalog):
        logging.warning("ninjalog is not found in %s", ninjalog)
        return 1

    # To avoid uploading duplicated ninjalog entries,
    # record the mtime of ninjalog that is uploaded.
    # If the recorded timestamp is older than the mtime of ninjalog,
    # itt needs to be uploaded.
    ninjalog_mtime = os.stat(ninjalog).st_mtime
    last_upload_file = pathlib.Path(ninjalog + '.last_upload')
    if last_upload_file.exists() and ninjalog_mtime <= last_upload_file.stat(
    ).st_mtime:
        logging.info("ninjalog is already uploaded.")
        return 0

    metadata = GetMetadata(args.cmdline, ninjalog, args.exit_code,
                           args.build_duration, cfg.user)
    exit_code = UploadNinjaLog(args.server, ninjalog, metadata)
    if exit_code == 0:
        last_upload_file.touch()
    return exit_code



if __name__ == "__main__":
    sys.exit(main())
