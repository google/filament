#!/usr/bin/python3
#
# Copyright 2024 The ANGLE Project Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
#
# winappsdk_setup.py:
#   Downloads and processes a specific version of Windows App SDK
#   for use when building ANGLE for WinUI 3 apps.
#
# Usage:
#   python3 winappsdk_setup.py [--version <package-version>] [--output /path/to/dest] [--force]

import argparse
import fnmatch
import io
import os
import subprocess
import urllib.request
import zipfile

package_id = "Microsoft.WindowsAppSDK"
uap_version = "10.0.18362"

default_version = "1.3.230724000"
default_output = "third_party\WindowsAppSDK"


def download_and_extract_nuget_package(force, version, output_path):
    # first check if the last download was successful
    stamp = os.path.join(output_path, f"{version}.stamp")
    if os.path.exists(stamp) and not force:
        return

    # download the package
    nuget_url = f"https://www.nuget.org/api/v2/package/{package_id}/{version}"
    with urllib.request.urlopen(nuget_url) as response:
        if response.status == 200:
            package_bytes = io.BytesIO(response.read())
            # extract
            os.makedirs(output_path, exist_ok=True)
            with zipfile.ZipFile(package_bytes, 'r') as zip_ref:
                zip_ref.extractall(output_path)
            # make a stamp to avoid re-downloading
            with open(stamp, 'w') as file:
                pass
        else:
            print(
                f"Failed to download the {package_id} NuGet package. Status code: {response.status}"
            )
            sys.exit(1)


def get_win_sdk():
    p = os.getenv("ProgramFiles(x86)", "C:\\Program Files (x86)")
    win_sdk = os.getenv("WindowsSdkDir")
    if not win_sdk:
        win_sdk = os.path.join(p, "Windows Kits", "10")
        print("%WindowsSdkDir% not set. Defaulting to", win_sdk)
        print("You might want to run this from a Visual Studio cmd prompt.")
    return win_sdk


def get_latest_sdk_version(win_sdk):
    files = os.listdir(os.path.join(win_sdk, "bin"))
    filtered_files = fnmatch.filter(files, "10.0.*")
    sorted_files = sorted(filtered_files, reverse=True)
    return sorted_files[0]


def run_winmdidl(force, win_sdk_bin, output_path, winmd, stamp_filename=None):
    include = os.path.join(output_path, "include")
    lib = os.path.join(output_path, "lib")

    if stamp_filename:
        stamp = os.path.join(include, stamp_filename)
    else:
        no_ext = os.path.splitext(os.path.basename(winmd))[0]
        stamp = os.path.join(include, no_ext + ".idl")
    if os.path.exists(stamp) and not force:
        return

    winmdidl = os.path.join(win_sdk_bin, "winmdidl.exe")
    command = [
        winmdidl,
        os.path.join(lib, winmd), "/metadata_dir:C:\\Windows\\System32\\WinMetadata",
        "/metadata_dir:" + os.path.join(lib, f"uap{uap_version}"),
        "/metadata_dir:" + os.path.join(lib, "uap10.0"), "/outdir:" + include, "/nologo"
    ]
    subprocess.run(command, check=True, cwd=include)


def run_midlrt(force, win_sdk_bin, output_path, idl):
    include = os.path.join(output_path, "include")
    lib = os.path.join(output_path, "lib")

    no_ext = os.path.splitext(os.path.basename(idl))[0]
    stamp = os.path.join(include, no_ext + ".h")
    if os.path.exists(stamp) and not force:
        return

    midlrt = os.path.join(win_sdk_bin, "midlrt.exe")
    command = [
        midlrt,
        os.path.join(include, idl), "/metadata_dir", "C:\\Windows\\System32\\WinMetadata",
        "/ns_prefix", "/nomidl", "/nologo"
    ]
    subprocess.run(command, check=True, cwd=include)


if __name__ == "__main__":

    parser = argparse.ArgumentParser(description="Setup the Windows App SDK.")
    parser.add_argument(
        "--version", default=default_version, help="the package version to download")
    parser.add_argument(
        "--output", default=default_output, help="the destination path for extracted contents")
    parser.add_argument(
        "--force", action="store_true", help="ignore existing files and re-download")
    args = parser.parse_args()

    if os.path.isabs(args.output):
        output_path = args.output
    else:
        output_path = os.path.join(os.getcwd(), args.output)

    win_sdk = get_win_sdk()
    latest_sdk = get_latest_sdk_version(win_sdk)
    arch = "x64"
    win_sdk_bin = os.path.join(win_sdk, "bin", latest_sdk, arch)

    winmd_files = {
        f"uap{uap_version}\\Microsoft.Foundation.winmd": None,
        f"uap{uap_version}\\Microsoft.Graphics.winmd": "Microsoft.Graphics.DirectX.idl",
        f"uap{uap_version}\\Microsoft.UI.winmd": None,
        "uap10.0\\Microsoft.UI.Text.winmd": None,
        "uap10.0\\Microsoft.UI.Xaml.winmd": None,
        "uap10.0\\Microsoft.Web.WebView2.Core.winmd": None,
    }

    idl_files = [
        "Microsoft.Foundation.idl",
        "Microsoft.Graphics.DirectX.idl",
        "Microsoft.UI.Composition.idl",
        "Microsoft.UI.Composition.SystemBackdrops.idl",
        "Microsoft.UI.Dispatching.idl",
        "Microsoft.UI.idl",
        "Microsoft.UI.Input.idl",
        "Microsoft.UI.Text.idl",
        "Microsoft.UI.Windowing.idl",
        "Microsoft.UI.Xaml.Automation.idl",
        "Microsoft.UI.Xaml.Automation.Peers.idl",
        "Microsoft.UI.Xaml.Automation.Provider.idl",
        "Microsoft.UI.Xaml.Automation.Text.idl",
        "Microsoft.UI.Xaml.Controls.idl",
        "Microsoft.UI.Xaml.Controls.Primitives.idl",
        "Microsoft.UI.Xaml.Data.idl",
        "Microsoft.UI.Xaml.Documents.idl",
        "Microsoft.UI.Xaml.idl",
        "Microsoft.UI.Xaml.Input.idl",
        "Microsoft.UI.Xaml.Interop.idl",
        "Microsoft.UI.Xaml.Media.Animation.idl",
        "Microsoft.UI.Xaml.Media.idl",
        "Microsoft.UI.Xaml.Media.Imaging.idl",
        "Microsoft.UI.Xaml.Media.Media3D.idl",
        "Microsoft.UI.Xaml.Navigation.idl",
        "Microsoft.Web.WebView2.Core.idl",
    ]

    progress = 1
    total = len(winmd_files) + len(idl_files) + 1

    # Download the NuGet package that contains all the files we need
    print(
        f"[{progress}/{total}] Downloading {package_id} NuGet package version {args.version} to {output_path}..."
    )
    download_and_extract_nuget_package(args.force, args.version, output_path)

    # Generate .idl files from the .winmd files
    for winmd, header in winmd_files.items():
        progress += 1
        print(f"[{progress}/{total}] Processing WINMD {winmd}...")
        run_winmdidl(args.force, win_sdk_bin, output_path, winmd, header)

    # Generate all the C++ headers and related files from the .idl files
    for idl in idl_files:
        progress += 1
        print(f"[{progress}/{total}] Processing IDL {idl}...")
        run_midlrt(args.force, win_sdk_bin, output_path, idl)

    print("Setup is complete.")
    print("")
    print(f"Pass winappsdk_dir=\"{output_path}\" to gn gen or ninja.")
