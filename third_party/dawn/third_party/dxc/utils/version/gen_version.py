# Added generating of new version for each DX Compiler build. 

# There are 3 kinds of version:
# 1. **Official build**
# Built by using `hctbuild -official`. The version is based on the current DXIL version, latest official 
# release and a number of commits since then. The format is `dxil_major.dxil_minor.release_no.commit_count`.
# For example a current official version would be something like `1.5.1905.42`. The latest release 
# information is read from `utils\version\latest-release.json`. The `1905` corresponds to `dxil-2019-05-16`
# release branch and `42` is the number of commits since that release branch was created. For main branch 
# the `commit_count` will be incremented by 10000 to distinguish it from stabilized official release branch 
# builds. So the current official version of main would be someting like `1.5.1905.10042`.

# 2. **Dev build**
# Build by using `hctbuild` with no other version-related option. 
# The format is `dxil_major.dxil_minor.0.commit_count` where commit_count is the number of total commits 
# since the beginning of the project.

# 3. **Fixed version build**
# Build by using `hctbuild -fv`. Enables overriding of the version information. The fixed version is 
# read from `utils\version\version.inc`. Location of the version file can be overriden by `-fvloc` option
# on `hctbuild`.

# In addition to the numbered version the product version string on the binaries will also include branch
# name and last commit sha - `"1.5.1905.10042 (main, 47e31c8a)"`. This product version string is included 
# in `dxc -?` output.

import argparse
import json
import os
import re
import subprocess

def get_output_of(cmd):
    enlistment_root=os.path.dirname(os.path.abspath( __file__ ))
    output = subprocess.check_output(cmd, cwd=enlistment_root)
    return output.decode('ASCII').strip()

def is_dirty():
    diff = get_output_of(["git", "diff", "HEAD", "--shortstat"])
    return len(diff.strip()) != 0

def get_last_commit_sha():
    try:
        sha = get_output_of(["git", "rev-parse", "--short", "HEAD"])
        if is_dirty():
            sha += "-dirty"
        return sha
    except subprocess.CalledProcessError:
        return "00000000"

def get_current_branch():
    try:
        return get_output_of(["git", "rev-parse", "--abbrev-ref", "HEAD"])
    except subprocess.CalledProcessError:
        return "private"

def get_commit_count(sha):
    try:
        return get_output_of(["git", "rev-list", "--count", sha])
    except subprocess.CalledProcessError:
        return 0
    
def read_latest_release_info():
    latest_release_file = os.path.join(os.path.dirname(os.path.abspath( __file__)), "latest-release.json")
    with open(latest_release_file, 'r') as f:
        return json.load(f)

class VersionGen():
    def __init__(self, options):
        self.options = options
        self.latest_release_info = read_latest_release_info()
        self.current_branch = get_current_branch()
        self.rc_version_field_4_cache = None

    def tool_name(self):
        return self.latest_release_info.get("toolname",
            "dxcoob" if self.options.official else "dxc(private)")

    def rc_version_field_1(self):
        return self.latest_release_info["version"]["major"]

    def rc_version_field_2(self):
        return self.latest_release_info["version"]["minor"]

    def rc_version_field_3(self):
        return self.latest_release_info["version"]["rev"] if self.options.official else "0"

    def rc_version_field_4(self):
        if self.rc_version_field_4_cache is None:
            base_commit_count = 0
            if self.options.official:
                base_commit_count = int(get_commit_count(self.latest_release_info["sha"]))
            current_commit_count = int(get_commit_count("HEAD"))
            distance_from_base = current_commit_count - base_commit_count
            if (self.current_branch == "main"):
                distance_from_base += 10000
            self.rc_version_field_4_cache = str(distance_from_base)
        return self.rc_version_field_4_cache

    def quoted_version_str(self):
        return '"{}.{}.{}.{}"'.format(
            self.rc_version_field_1(),
            self.rc_version_field_2(),
            self.rc_version_field_3(),
            self.rc_version_field_4())

    def product_version_str(self):
        if (self.options.no_commit_sha):
            return self.quoted_version_str()
        pv = '"{}.{}.{}.{} '.format(
                self.rc_version_field_1(),
                self.rc_version_field_2(),
                self.rc_version_field_3(),
                self.rc_version_field_4())
        if (self.current_branch != "HEAD"):
             pv += '({}, {})"'.format(self.current_branch, get_last_commit_sha())
        else:
             pv += '({})"'.format(get_last_commit_sha())
        return pv

    def print_define(self, name, value):
        print('#ifdef {}'.format(name))
        print('#undef {}'.format(name))
        print('#endif')
        print('#define {} {}'.format(name, value))
        print()

    def print_version(self):
        print('#pragma once')
        print()
        self.print_define('RC_COMPANY_NAME',      '"Microsoft(r) Corporation"')
        self.print_define('RC_VERSION_FIELD_1',   self.rc_version_field_1())
        self.print_define('RC_VERSION_FIELD_2',   self.rc_version_field_2())
        self.print_define('RC_VERSION_FIELD_3',   self.rc_version_field_3() if self.options.official else "0")
        self.print_define('RC_VERSION_FIELD_4',   self.rc_version_field_4())
        self.print_define('RC_FILE_VERSION',      self.quoted_version_str())
        self.print_define('RC_FILE_DESCRIPTION', '"DirectX Compiler - Out Of Band"')
        self.print_define('RC_COPYRIGHT',        '"(c) Microsoft Corporation. All rights reserved."')
        self.print_define('RC_PRODUCT_NAME',     '"Microsoft(r) DirectX for Windows(r) - Out Of Band"')
        self.print_define('RC_PRODUCT_VERSION',   self.product_version_str())
        self.print_define('HLSL_TOOL_NAME',       '"{}"'.format(self.tool_name()))
        self.print_define('HLSL_VERSION_MACRO',   'HLSL_TOOL_NAME " " RC_FILE_VERSION')
        self.print_define('HLSL_LLVM_IDENT',      'HLSL_TOOL_NAME " " RC_PRODUCT_VERSION')


def main():
    p = argparse.ArgumentParser("gen_version")
    p.add_argument("--no-commit-sha", action='store_true')
    p.add_argument("--official", action="store_true")
    args = p.parse_args()

    VersionGen(args).print_version() 


if __name__ == '__main__':
    main()

