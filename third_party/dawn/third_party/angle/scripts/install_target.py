#! /usr/bin/env python3
# Copyright 2024 Google Inc.  All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
"""Install script for ANGLE targets

1. Suppose this is your custom prefix:
   `export CUSTOM_PREFIX=/custom/prefix
2. Configure the install prefix with gn:
   `gn gen --args="install_prefix=\"$CUSTOM_PREFIX\"" out`
3. Then install ANGLE:
   `ninja -C out install_angle`

This will copy all needed include directories under $CUSTOM_PREFIX/include and the
libraries will be copied to $CUSTOM_PREFIX/lib. A package config file is generated for
each library under $CUSTOM_PREFIX/lib/pkgconfig, therefore ANGLE libraries can be
discovered by package config making sure this path is listed in the PKG_CONFIG_PATH
environment variable.
```
export PKG_CONFIG_PATH=$PKG_CONFIG_PATH:$CUSTOM_PREFIX/lib/pkgconfig
pkg-config --libs EGL
pkg-config --cflags EGL
```
"""

import argparse
import os
import shutil
import sys


def install2(src_list: list, dst_dir: str):
    """Installs a list of files or directories in `src_list` to `dst_dir`"""
    os.makedirs(dst_dir, exist_ok=True)
    for src in src_list:
        if not os.path.exists(src):
            raise FileNotFoundError("Failed to find {}".format(src))
        basename = os.path.basename(src)
        dst = os.path.join(dst_dir, basename)
        print("Installing {} to {}".format(src, dst))
        if os.path.isdir(src):
            shutil.copytree(src, dst, dirs_exist_ok=True)
        else:
            shutil.copy2(src, dst)


PC_TEMPLATE = """prefix={prefix}
libdir=${{prefix}}/lib
includedir=${{prefix}}/include

Name: {name}
Description: {description}
Version: {version}
Libs: -L${{libdir}} {link_libraries}
Cflags: -I${{includedir}}
"""


def gen_link_libraries(libs: list):
    """Generates a string that can be used for the `Libs:` entry of a pkgconfig file"""
    link_libraries = ""
    for lib in libs:
        # Absolute paths to file names only -> libEGL.dylib
        basename = os.path.basename(lib)
        # lib name only -> libEGL
        libname: str = os.path.splitext(basename)[0]
        # name only -> EGL
        name = libname.strip('lib')
        link_libraries += '-l{}'.format(name)
    return link_libraries


def gen_pkgconfig(name: str, version: str, prefix: os.path.abspath, libs: list):
    """Generates a pkgconfig file for the current target"""
    # Remove lib from name -> EGL
    no_lib_name = name.strip('lib')
    description = "ANGLE's {}".format(no_lib_name)
    name_lowercase = no_lib_name.lower()
    link_libraries = gen_link_libraries(libs)
    pc_content = PC_TEMPLATE.format(
        name=name_lowercase,
        prefix=prefix,
        description=description,
        version=version,
        link_libraries=link_libraries)

    lib_pkgconfig_path = os.path.join(prefix, 'lib/pkgconfig')
    if not os.path.exists(lib_pkgconfig_path):
        os.makedirs(lib_pkgconfig_path)

    pc_path = os.path.join(lib_pkgconfig_path, '{}.pc'.format(name_lowercase))
    print("Generating {}".format(pc_path))
    with open(pc_path, 'w+') as pc_file:
        pc_file.write(pc_content)


def install(name, version, prefix: os.path.abspath, libs: list, includes: list):
    """Installs under `prefix`
    - the libraries in the `libs` list
    - the include directories in the `includes` list
    - the pkgconfig file for current target if name is set"""
    install2(libs, os.path.join(prefix, "lib"))

    for include in includes:
        assert (os.path.isdir(include))
        incs = [inc.path for inc in os.scandir(include)]
        install2(incs, os.path.join(prefix, "include"))

    if name:
        gen_pkgconfig(name, version, prefix, libs)


def main():
    parser = argparse.ArgumentParser(description='Install script for ANGLE targets')
    parser.add_argument(
        '--name',
        help='Name of the target (e.g., EGL or GLESv2). Set it to generate a pkgconfig file',
    )
    parser.add_argument(
        '--version', help='SemVer of the target (e.g., 0.1.0 or 2.1)', default='0.0.0')
    parser.add_argument(
        '--prefix',
        help='Install prefix to use (e.g., out/install or /usr/local/)',
        default='',
        type=os.path.abspath)
    parser.add_argument(
        '--libs',
        help='List of libraries to install (e.g., libEGL.dylib or libGLESv2.so)',
        default=[],
        nargs='+',
        type=os.path.abspath)
    parser.add_argument(
        '-I',
        '--includes',
        help='List of include directories to install (e.g., include or ../include)',
        default=[],
        nargs='+',
        type=os.path.abspath)

    args = parser.parse_args()
    install(args.name, args.version, args.prefix, args.libs, args.includes)


if __name__ == '__main__':
    sys.exit(main())
