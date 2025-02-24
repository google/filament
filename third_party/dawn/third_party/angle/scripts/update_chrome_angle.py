#!/usr/bin/python3
#
# Copyright 2016 The ANGLE Project Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
#
# update_chrome_angle.py:
#   Helper script that copies ANGLE libraries into the Chromium Canary (on Windows and macOS)
#   or Dev (on Linux) installed directory. Testing ANGLE this way is much faster than compiling
#   Chromium from source. The script checks for the most recent build in a set of search paths,
#   and copies that into:
#
#    - /opt/google/chrome-unstable on Linux
#    - the most recent Canary installation folder on Windows.
#    - /Applications/Google\ Chrome\ Canary.app on macOS
#

import glob, sys, os, shutil

# Set of search paths.
script_dir = os.path.dirname(sys.argv[0])
os.chdir(os.path.join(script_dir, ".."))

source_paths = glob.glob('out/*')

is_windows = sys.platform == 'cygwin' or sys.platform.startswith('win')
is_macos = sys.platform == 'darwin'

if is_windows:
    # Default Canary installation path.
    chrome_folder = os.path.join(os.environ['LOCALAPPDATA'], 'Google', 'Chrome SxS', 'Application')
    libs_to_copy = ['libGLESv2.dll', 'libEGL.dll']
    optional_libs_to_copy = []

elif is_macos:
    chrome_folder = '/Applications/Google Chrome Canary.app/Contents/Frameworks/Google Chrome Framework.framework/Libraries'
    libs_to_copy = ['libGLESv2.dylib', 'libEGL.dylib']
    optional_libs_to_copy = [
        'libc++_chrome.dylib',
        'libchrome_zlib.dylib',
        'libthird_party_abseil-cpp_absl.dylib',
        'libvk_swiftshader.dylib',
    ]

else:
    # Must be Linux
    chrome_folder = '/opt/google/chrome-unstable'
    libs_to_copy = ['libGLESv2.so', 'libEGL.so']
    optional_libs_to_copy = ['libchrome_zlib.so', 'libabsl.so', 'libc++.so']

# Find the most recent ANGLE DLLs
binary_name = libs_to_copy[0]
newest_folder = None
newest_mtime = None
for path in source_paths:
    binary_path = os.path.join(path, binary_name)
    if os.path.exists(binary_path):
        binary_mtime = os.path.getmtime(binary_path)
        if (newest_folder is None) or (binary_mtime > newest_mtime):
            newest_folder = path
            newest_mtime = binary_mtime

if newest_folder is None:
    sys.exit("Could not find ANGLE binaries!")

source_folder = newest_folder

if is_windows:
    # Is a folder a chrome binary directory?
    def is_chrome_bin(str):
        chrome_file = os.path.join(chrome_folder, str)
        return os.path.isdir(chrome_file) and all([char.isdigit() or char == '.' for char in str])

    sorted_chrome_bins = sorted(
        [folder for folder in os.listdir(chrome_folder) if is_chrome_bin(folder)], reverse=True)

    dest_folder = os.path.join(chrome_folder, sorted_chrome_bins[0])
else:
    dest_folder = chrome_folder

print('Copying binaries from ' + source_folder + ' to ' + dest_folder + '.')


def copy_file(src, dst):
    print(' - ' + src + '   -->   ' + dst)
    if is_macos and os.path.isfile(dst):
        # For the codesign to work, the original file must be removed
        os.remove(dst)
    shutil.copyfile(src, dst)


def do_copy(filename, is_optional):
    src = os.path.join(source_folder, filename)
    if os.path.exists(src):
        # No backup is made.  Any backup becomes stale on the next update and could be a cause for
        # confusion.  Reintall Chromium if it needs to be recovered.
        dst = os.path.join(dest_folder, filename)
        copy_file(src, dst)

        if is_windows:
            copy_file(src + '.pdb', dst + '.pdb')

    elif not is_optional:
        print(' - COULD NOT FIND "' + src + '"')


for filename in libs_to_copy:
    do_copy(filename, False)
# Optionally copy the following, which are needed for a component build
# (i.e. is_component_build = true, which is the default)
for filename in optional_libs_to_copy:
    do_copy(filename, True)

if is_macos:
    # Clear all attributes, codesign doesn't work otherwise
    os.system('xattr -cr /Applications/Google\ Chrome\ Canary.app')
    # Re-sign the bundle
    os.system('codesign --force --sign - --deep /Applications/Google\ Chrome\ Canary.app')
