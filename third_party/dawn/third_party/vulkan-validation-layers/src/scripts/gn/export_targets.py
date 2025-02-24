#! /usr/bin/env python3
assert __name__ == '__main__'

'''
To update ANGLE in Gecko, use Windows with git-bash, and setup depot_tools, python2, and
python3. Because depot_tools expects `python` to be `python2` (shame!), python2 must come
before python3 in your path.

Upstream: https://chromium.googlesource.com/angle/angle

Our repo: https://github.com/mozilla/angle
It has branches like 'firefox-60' which is the branch we use for pulling into
Gecko with this script.

This script leaves a record of the merge-base and cherry-picks that we pull into
Gecko. (gfx/angle/cherries.log)

ANGLE<->Chrome version mappings are here: https://omahaproxy.appspot.com/
An easy choice is to grab Chrome's Beta's ANGLE branch.

## Usage

Prepare your env:

~~~
export PATH="$PATH:/path/to/depot_tools"
~~~

If this is a new repo, don't forget:

~~~
# In the angle repo:
./scripts/bootstrap.py
gclient sync
~~~

Update: (in the angle repo)

~~~
# In the angle repo:
/path/to/gecko/gfx/angle/update-angle.py origin/chromium/XXXX
git push moz # Push the firefox-XX branch to github.com/mozilla/angle
~~~~

'''

import json
import os
import pathlib
import re
import shutil
import subprocess
import sys
from typing import * # mypy annotations

SCRIPT_DIR = os.path.dirname(__file__)

GN_ENV = dict(os.environ)
# We need to set DEPOT_TOOLS_WIN_TOOLCHAIN to 0 for non-Googlers, but otherwise
# leave it unset since vs_toolchain.py assumes that the user is a Googler with
# the Visual Studio files in depot_tools if DEPOT_TOOLS_WIN_TOOLCHAIN is not
# explicitly set to 0.
vs_found = False
vs_dir = os.path.join(SCRIPT_DIR, '..', 'third_party', 'depot_tools', 'win_toolchain', 'vs_files')
if not os.path.isdir(vs_dir):
    GN_ENV['DEPOT_TOOLS_WIN_TOOLCHAIN'] = '0'

if len(sys.argv) < 3:
    sys.exit('Usage: export_targets.py OUT_DIR ROOTS...')

(OUT_DIR, *ROOTS) = sys.argv[1:]
for x in ROOTS:
    assert x.startswith('//:')

# ------------------------------------------------------------------------------

def run_checked(*args, **kwargs):
    print(' ', args, file=sys.stderr)
    sys.stderr.flush()
    return subprocess.run(args, check=True, **kwargs)


def sortedi(x):
    return sorted(x, key=str.lower)


def dag_traverse(root_keys: Sequence[str], pre_recurse_func: Callable[[str], list]):
    visited_keys: Set[str] = set()

    def recurse(key):
        if key in visited_keys:
            return
        visited_keys.add(key)

        t = pre_recurse_func(key)
        try:
            (next_keys, post_recurse_func) = t
        except ValueError:
            (next_keys,) = t
            post_recurse_func = None

        for x in next_keys:
            recurse(x)

        if post_recurse_func:
            post_recurse_func(key)
        return

    for x in root_keys:
        recurse(x)
    return

# ------------------------------------------------------------------------------

print('Importing graph', file=sys.stderr)

try:
    p = run_checked('gn', 'desc', '--format=json', str(OUT_DIR), '*', stdout=subprocess.PIPE,
                env=GN_ENV, shell=(True if sys.platform == 'win32' else False))
except subprocess.CalledProcessError:
    sys.stderr.buffer.write(b'"gn desc" failed. Is depot_tools in your PATH?\n')
    exit(1)

# -

print('\nProcessing graph', file=sys.stderr)
descs = json.loads(p.stdout.decode())

# Ready to traverse
# ------------------------------------------------------------------------------

LIBRARY_TYPES = ('shared_library', 'static_library')

def flattened_target(target_name: str, descs: dict, stop_at_lib: bool =True) -> dict:
    flattened = dict(descs[target_name])

    EXPECTED_TYPES = LIBRARY_TYPES + ('source_set', 'group', 'action')

    def pre(k):
        dep = descs[k]

        dep_type = dep['type']
        deps = dep['deps']
        if stop_at_lib and dep_type in LIBRARY_TYPES:
            return ((),)

        if dep_type == 'copy':
            assert not deps, (target_name, dep['deps'])
        else:
            assert dep_type in EXPECTED_TYPES, (k, dep_type)
            for (k,v) in dep.items():
                if type(v) in (list, tuple, set):
                    # This is a workaround for
                    # https://bugs.chromium.org/p/gn/issues/detail?id=196, where
                    # the value of "public" can be a string instead of a list.
                    existing = flattened.get(k, [])
                    if isinstance(existing, str):
                      existing = [existing]
                    # Use temporary sets then sort them to avoid a bottleneck here
                    if not isinstance(existing, set):
                        flattened[k] = set(existing)
                    flattened[k].update(v)
                else:
                    #flattened.setdefault(k, v)
                    pass
        return (deps,)

    dag_traverse(descs[target_name]['deps'], pre)

    for k, v in flattened.items():
        if isinstance(v, set):
            flattened[k] = sortedi(v)
    return flattened

# ------------------------------------------------------------------------------
# Check that includes are valid. (gn's version of this check doesn't seem to work!)

INCLUDE_REGEX = re.compile(b'^ *# *include +([<"])([^>"]+)[>"].*$', re.MULTILINE)
assert INCLUDE_REGEX.findall(b' #  include <foo>  //comment\n#include "bar"') == [(b'<', b'foo'), (b'"', b'bar')]

# Most of these are ignored because this script does not currently handle
# #includes in #ifdefs properly, so they will erroneously be marked as being
# included, but not part of the source list.
IGNORED_INCLUDES = {
    b'absl/container/flat_hash_map.h',
    b'absl/container/flat_hash_set.h',
    b'compiler/translator/glsl/TranslatorESSL.h',
    b'compiler/translator/glsl/TranslatorGLSL.h',
    b'compiler/translator/hlsl/TranslatorHLSL.h',
    b'compiler/translator/msl/TranslatorMSL.h',
    b'compiler/translator/null/TranslatorNULL.h',
    b'compiler/translator/spirv/TranslatorSPIRV.h',
    b'compiler/translator/wgsl/TranslatorWGSL.h',
    b'contrib/optimizations/slide_hash_neon.h',
    b'dirent_on_windows.h',
    b'dlopen_fuchsia.h',
    b'kernel/image.h',
    b'libANGLE/renderer/d3d/d3d11/Device11.h',
    b'libANGLE/renderer/d3d/d3d11/winrt/NativeWindow11WinRT.h',
    b'libANGLE/renderer/d3d/DisplayD3D.h',
    b'libANGLE/renderer/d3d/RenderTargetD3D.h',
    b'libANGLE/renderer/gl/cgl/DisplayCGL.h',
    b'libANGLE/renderer/gl/egl/android/DisplayAndroid.h',
    b'libANGLE/renderer/gl/egl/DisplayEGL.h',
    b'libANGLE/renderer/gl/egl/gbm/DisplayGbm.h',
    b'libANGLE/renderer/gl/glx/DisplayGLX.h',
    b'libANGLE/renderer/gl/glx/DisplayGLX_api.h',
    b'libANGLE/renderer/gl/wgl/DisplayWGL.h',
    b'libANGLE/renderer/metal/DisplayMtl_api.h',
    b'libANGLE/renderer/null/DisplayNULL.h',
    b'libANGLE/renderer/vulkan/android/AHBFunctions.h',
    b'libANGLE/renderer/vulkan/android/DisplayVkAndroid.h',
    b'libANGLE/renderer/vulkan/DisplayVk_api.h',
    b'libANGLE/renderer/vulkan/fuchsia/DisplayVkFuchsia.h',
    b'libANGLE/renderer/vulkan/ggp/DisplayVkGGP.h',
    b'libANGLE/renderer/vulkan/mac/DisplayVkMac.h',
    b'libANGLE/renderer/vulkan/win32/DisplayVkWin32.h',
    b'libANGLE/renderer/vulkan/xcb/DisplayVkXcb.h',
    b'libANGLE/renderer/vulkan/wayland/DisplayVkWayland.h',
    b'loader_cmake_config.h',
    b'loader_linux.h',
    b'loader_windows.h',
    b'optick.h',
    b'spirv-tools/libspirv.h',
    b'third_party/volk/volk.h',
    b'vk_loader_extensions.c',
    b'vk_snippets.h',
    b'vulkan_android.h',
    b'vulkan_beta.h',
    b'vulkan_directfb.h',
    b'vulkan_fuchsia.h',
    b'vulkan_ggp.h',
    b'vulkan_ios.h',
    b'vulkan_macos.h',
    b'vulkan_metal.h',
    b'vulkan_sci.h',
    b'vulkan_vi.h',
    b'vulkan_wayland.h',
    b'vulkan_win32.h',
    b'vulkan_xcb.h',
    b'vulkan_xlib.h',
    b'vulkan_xlib_xrandr.h',
    # rapidjson adds these include stubs into their documentation
    # comments. Since the script doesn't skip comments they are
    # erroneously marked as valid includes
    b'rapidjson/...',
    # Validation layers support building with robin hood hashing, but we are not enabling that
    # See http://anglebug.com/42264327
    b'robin_hood.h',
    # Validation layers optionally use mimalloc
    b'mimalloc-new-delete.h',
    # From the Vulkan-Loader
    b'winres.h',
    # From a comment in vulkan-validation-layers/src/layers/vk_mem_alloc.h
    b'my_custom_assert.h',
    b'my_custom_min.h',
    # https://bugs.chromium.org/p/gn/issues/detail?id=311
    b'spirv/unified1/spirv.hpp11',
    # Behind #if defined(QAT_COMPRESSION_ENABLED) in third_party/zlib/deflate.c
    b'contrib/qat/deflate_qat.h',
    # Behind #if defined(TRACY_ENABLE) in third_party/vulkan-validation-layers/src/layers/vulkan/generated/chassis.cpp
    b'profiling/profiling.h',
}

IGNORED_INCLUDE_PREFIXES = {
    b'android',
    b'Carbon',
    b'CoreFoundation',
    b'CoreServices',
    b'IOSurface',
    b'mach',
    b'mach-o',
    b'OpenGL',
    b'pci',
    b'sys',
    b'wrl',
    b'X11',
}

IGNORED_DIRECTORIES = {
    '//buildtools/third_party/libc++',
    '//third_party/libc++/src',
    '//third_party/abseil-cpp',
    '//third_party/SwiftShader',
    '//third_party/dawn',
}

def has_all_includes(target_name: str, descs: dict) -> bool:
    for ignored_directory in IGNORED_DIRECTORIES:
        if target_name.startswith(ignored_directory):
            return True

    flat = flattened_target(target_name, descs, stop_at_lib=False)
    acceptable_sources = flat.get('sources', []) + flat.get('outputs', [])
    acceptable_sources = {x.rsplit('/', 1)[-1].encode() for x in acceptable_sources}

    ret = True
    desc = descs[target_name]
    for cur_file in desc.get('sources', []):
        assert cur_file.startswith('/'), cur_file
        if not cur_file.startswith('//'):
            continue
        cur_file = pathlib.Path(cur_file[2:])
        text = cur_file.read_bytes()
        for m in INCLUDE_REGEX.finditer(text):
            if m.group(1) == b'<':
                continue
            include = m.group(2)
            if include in IGNORED_INCLUDES:
                continue
            try:
                (prefix, _) = include.split(b'/', 1)
                if prefix in IGNORED_INCLUDE_PREFIXES:
                    continue
            except ValueError:
                pass

            include_file = include.rsplit(b'/', 1)[-1]
            if include_file not in acceptable_sources:
                #print('  acceptable_sources:')
                #for x in sorted(acceptable_sources):
                #    print('   ', x)
                print('Warning in {}: {}: Included file must be listed in the GN target or its public dependency: {}'.format(target_name, cur_file, include), file=sys.stderr)
                ret = False
            #print('Looks valid:', m.group())
            continue

    return ret

# -
# Gather real targets:

def gather_libraries(roots: Sequence[str], descs: dict) -> Set[str]:
    libraries = set()
    def fn(target_name):
        cur = descs[target_name]
        print('  ' + cur['type'], target_name, file=sys.stderr)
        assert has_all_includes(target_name, descs), target_name

        if cur['type'] in ('shared_library', 'static_library'):
            libraries.add(target_name)
        return (cur['deps'], )

    dag_traverse(roots, fn)
    return libraries

# -

libraries = gather_libraries(ROOTS, descs)
print(f'\n{len(libraries)} libraries:', file=sys.stderr)
for k in libraries:
    print(f'  {k}', file=sys.stderr)
print('\nstdout begins:', file=sys.stderr)
sys.stderr.flush()

# ------------------------------------------------------------------------------
# Output

out = {k: flattened_target(k, descs) for k in libraries}

for (k,desc) in out.items():
    dep_libs: Set[str] = set()
    for dep_name in set(desc['deps']):
        dep = descs[dep_name]
        if dep['type'] in LIBRARY_TYPES:
            dep_libs.add(dep_name)
    desc['dep_libs'] = sortedi(dep_libs)

json.dump(out, sys.stdout, indent='  ')
exit(0)
