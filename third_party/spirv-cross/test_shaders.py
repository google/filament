#!/usr/bin/env python3

# Copyright 2015-2021 Arm Limited
# SPDX-License-Identifier: Apache-2.0
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

import sys
import os
import os.path
import subprocess
import tempfile
import re
import itertools
import hashlib
import shutil
import argparse
import codecs
import json
import multiprocessing
import errno
import platform
from functools import partial

class Paths():
    def __init__(self, spirv_cross, glslang, spirv_as, spirv_val, spirv_opt):
        self.spirv_cross = spirv_cross
        self.glslang = glslang
        self.spirv_as = spirv_as
        self.spirv_val = spirv_val
        self.spirv_opt = spirv_opt

def remove_file(path):
    #print('Removing file:', path)
    os.remove(path)

def create_temporary(suff = ''):
    f, path = tempfile.mkstemp(suffix = suff)
    os.close(f)
    #print('Creating temporary:', path)
    return path

def parse_stats(stats):
    m = re.search('([0-9]+) work registers', stats)
    registers = int(m.group(1)) if m else 0

    m = re.search('([0-9]+) uniform registers', stats)
    uniform_regs = int(m.group(1)) if m else 0

    m_list = re.findall('(-?[0-9]+)\s+(-?[0-9]+)\s+(-?[0-9]+)', stats)
    alu_short = float(m_list[1][0]) if m_list else 0
    ls_short = float(m_list[1][1]) if m_list else 0
    tex_short = float(m_list[1][2]) if m_list else 0
    alu_long = float(m_list[2][0]) if m_list else 0
    ls_long = float(m_list[2][1]) if m_list else 0
    tex_long = float(m_list[2][2]) if m_list else 0

    return (registers, uniform_regs, alu_short, ls_short, tex_short, alu_long, ls_long, tex_long)

def get_shader_type(shader):
    _, ext = os.path.splitext(shader)
    if ext == '.vert':
        return '--vertex'
    elif ext == '.frag':
        return '--fragment'
    elif ext == '.comp':
        return '--compute'
    elif ext == '.tesc':
        return '--tessellation_control'
    elif ext == '.tese':
        return '--tessellation_evaluation'
    elif ext == '.geom':
        return '--geometry'
    else:
        return ''

def get_shader_stats(shader):
    path = create_temporary()

    p = subprocess.Popen(['malisc', get_shader_type(shader), '--core', 'Mali-T760', '-V', shader], stdout = subprocess.PIPE, stderr = subprocess.PIPE)
    stdout, stderr = p.communicate()
    remove_file(path)

    if p.returncode != 0:
        print(stderr.decode('utf-8'))
        raise OSError('malisc failed')
    p.wait()

    returned = stdout.decode('utf-8')
    return parse_stats(returned)

def print_msl_compiler_version():
    try:
        if platform.system() == 'Darwin':
            subprocess.check_call(['xcrun', '--sdk', 'iphoneos', 'metal', '--version'])
            print('... are the Metal compiler characteristics.\n')   # display after so xcrun FNF is silent
        else:
            # Use Metal Windows toolkit to test on Linux (Wine) and Windows.
            print('Running on non-macOS system.')
            subprocess.check_call(['metal', '-x', 'metal', '--version'])

    except OSError as e:
        if (e.errno != errno.ENOENT):    # Ignore xcrun not found error
            raise
        print('Metal SDK is not present.\n')
    except subprocess.CalledProcessError:
        pass

def msl_compiler_supports_version(version):
    try:
        if platform.system() == 'Darwin':
            subprocess.check_call(['xcrun', '--sdk', 'macosx', 'metal', '-x', 'metal', version, '-'],
                stdin = subprocess.DEVNULL, stdout = subprocess.DEVNULL, stderr = subprocess.DEVNULL)
            print('Current SDK supports MSL {0}. Enabling validation for MSL {0} shaders.'.format(version))
        else:
            print('Running on {}, assuming {} is supported.'.format(platform.system(), version))
        # If we're running on non-macOS system, assume it's supported.
        return True
    except OSError as e:
        print('Failed to check if MSL {} is not supported. It probably is not.'.format(version))
        return False
    except subprocess.CalledProcessError:
        print('Current SDK does NOT support MSL {0}. Disabling validation for MSL {0} shaders.'.format(version))
        return False

def path_to_msl_standard(shader):
    if '.msl31.' in shader:
        return '-std=metal3.1'
    elif '.msl3.' in shader:
        return '-std=metal3.0'
    elif '.ios.' in shader:
        if '.msl2.' in shader:
            return '-std=ios-metal2.0'
        elif '.msl21.' in shader:
            return '-std=ios-metal2.1'
        elif '.msl22.' in shader:
            return '-std=ios-metal2.2'
        elif '.msl23.' in shader:
            return '-std=ios-metal2.3'
        elif '.msl24.' in shader:
            return '-std=ios-metal2.4'
        elif '.msl11.' in shader:
            return '-std=ios-metal1.1'
        elif '.msl10.' in shader:
            return '-std=ios-metal1.0'
        else:
            return '-std=ios-metal1.2'
    else:
        if '.msl2.' in shader:
            return '-std=macos-metal2.0'
        elif '.msl21.' in shader:
            return '-std=macos-metal2.1'
        elif '.msl22.' in shader:
            return '-std=macos-metal2.2'
        elif '.msl23.' in shader:
            return '-std=macos-metal2.3'
        elif '.msl24.' in shader:
            return '-std=macos-metal2.4'
        elif '.msl11.' in shader:
            return '-std=macos-metal1.1'
        else:
            return '-std=macos-metal1.2'

def path_to_msl_standard_cli(shader):
    if '.msl31.' in shader:
        return '30100'
    elif '.msl3.' in shader:
        return '30000'
    elif '.msl2.' in shader:
        return '20000'
    elif '.msl21.' in shader:
        return '20100'
    elif '.msl22.' in shader:
        return '20200'
    elif '.msl23.' in shader:
        return '20300'
    elif '.msl24.' in shader:
        return '20400'
    elif '.msl11.' in shader:
        return '10100'
    else:
        return '10200'

ignore_win_metal_tool = False
def validate_shader_msl(shader, opt):
    msl_path = reference_path(shader[0], shader[1], opt)
    global ignore_win_metal_tool
    try:
        if '.ios.' in msl_path:
            msl_os = 'iphoneos'
        else:
            msl_os = 'macosx'

        if platform.system() == 'Darwin':
            subprocess.check_call(['xcrun', '--sdk', msl_os, 'metal', '-x', 'metal', path_to_msl_standard(msl_path), '-Werror', '-Wno-unused-variable', msl_path])
            print('Compiled Metal shader: ' + msl_path)   # display after so xcrun FNF is silent
        elif not ignore_win_metal_tool:
            # Use Metal Windows toolkit to test on Linux (Wine) and Windows. Running offline tool on Linux gets weird.
            # Normal winepath doesn't work, it must be Z:/abspath *exactly* for some bizarre reason.
            target_path = msl_path if platform.system == 'Windows' else ('Z:' + os.path.abspath(msl_path))
            subprocess.check_call(['metal', '-x', 'metal', path_to_msl_standard(msl_path), '-Werror', '-Wno-unused-variable', target_path])

    except OSError as oe:
        if (oe.errno != errno.ENOENT):   # Ignore xcrun or metal not found error
            raise
        print('metal toolkit does not exist, ignoring further attempts to use it.')
        ignore_win_metal_tool = True
    except subprocess.CalledProcessError:
        print('Error compiling Metal shader: ' + msl_path)
        raise RuntimeError('Failed to compile Metal shader')

def cross_compile_msl(shader, spirv, opt, iterations, paths):
    spirv_path = create_temporary()
    msl_path = create_temporary(os.path.basename(shader))

    spirv_16 = '.spv16.' in shader
    spirv_14 = '.spv14.' in shader

    if spirv_16:
        spirv_env = 'spv1.6'
        glslang_env = 'spirv1.6'
    elif spirv_14:
        spirv_env = 'vulkan1.1spv1.4'
        glslang_env = 'spirv1.4'
    else:
        spirv_env = 'vulkan1.1'
        glslang_env = 'vulkan1.1'

    spirv_cmd = [paths.spirv_as, '--preserve-numeric-ids', '--target-env', spirv_env, '-o', spirv_path, shader]

    if spirv:
        subprocess.check_call(spirv_cmd)
    else:
        subprocess.check_call([paths.glslang, '--amb' ,'--target-env', glslang_env, '-V', '-o', spirv_path, shader])

    if opt and (not shader_is_invalid_spirv(shader)):
        if '.graphics-robust-access.' in shader:
            subprocess.check_call([paths.spirv_opt, '--skip-validation', '-O', '--graphics-robust-access', '-o', spirv_path, spirv_path])
        else:
            subprocess.check_call([paths.spirv_opt, '--skip-validation', '-O', '-o', spirv_path, spirv_path])

    spirv_cross_path = paths.spirv_cross

    msl_args = [spirv_cross_path, '--output', msl_path, spirv_path, '--msl', '--iterations', str(iterations)]
    msl_args.append('--msl-version')
    msl_args.append(path_to_msl_standard_cli(shader))
    if not '.nomain.' in shader:
        msl_args.append('--entry')
        msl_args.append('main')
    if '.swizzle.' in shader:
        msl_args.append('--msl-swizzle-texture-samples')
    if '.ios.' in shader:
        msl_args.append('--msl-ios')
    if '.pad-fragment.' in shader:
        msl_args.append('--msl-pad-fragment-output')
    if '.capture.' in shader:
        msl_args.append('--msl-capture-output')
    if '.domain.' in shader:
        msl_args.append('--msl-domain-lower-left')
    if '.argument.' in shader:
        msl_args.append('--msl-argument-buffers')
    if '.argument-tier-1.' in shader:
        msl_args.append('--msl-argument-buffer-tier')
        msl_args.append('1')
    if '.texture-buffer-native.' in shader:
        msl_args.append('--msl-texture-buffer-native')
    if '.framebuffer-fetch.' in shader:
        msl_args.append('--msl-framebuffer-fetch')
    if '.invariant-float-math.' in shader:
        msl_args.append('--msl-invariant-float-math')
    if '.emulate-cube-array.' in shader:
        msl_args.append('--msl-emulate-cube-array')
    if '.discrete.' in shader:
        # Arbitrary for testing purposes.
        msl_args.append('--msl-discrete-descriptor-set')
        msl_args.append('2')
        msl_args.append('--msl-discrete-descriptor-set')
        msl_args.append('3')
    if '.force-active.' in shader:
        msl_args.append('--msl-force-active-argument-buffer-resources')
    if '.line.' in shader:
        msl_args.append('--emit-line-directives')
    if '.multiview.' in shader:
        msl_args.append('--msl-multiview')
    if '.no-layered.' in shader:
        msl_args.append('--msl-multiview-no-layered-rendering')
    if '.viewfromdev.' in shader:
        msl_args.append('--msl-view-index-from-device-index')
    if '.dispatchbase.' in shader:
        msl_args.append('--msl-dispatch-base')
    if '.dynamic-buffer.' in shader:
        # Arbitrary for testing purposes.
        msl_args.append('--msl-dynamic-buffer')
        msl_args.append('0')
        msl_args.append('0')
        msl_args.append('--msl-dynamic-buffer')
        msl_args.append('1')
        msl_args.append('2')
    if '.inline-block.' in shader:
        # Arbitrary for testing purposes.
        msl_args.append('--msl-inline-uniform-block')
        msl_args.append('0')
        msl_args.append('0')
    if '.device-argument-buffer.' in shader:
        msl_args.append('--msl-device-argument-buffer')
        msl_args.append('0')
        msl_args.append('--msl-device-argument-buffer')
        msl_args.append('1')
    if '.force-native-array.' in shader:
        msl_args.append('--msl-force-native-arrays')
    if '.zero-initialize.' in shader:
        msl_args.append('--force-zero-initialized-variables')
    if '.frag-output.' in shader:
        # Arbitrary for testing purposes.
        msl_args.append('--msl-disable-frag-depth-builtin')
        msl_args.append('--msl-disable-frag-stencil-ref-builtin')
        msl_args.append('--msl-enable-frag-output-mask')
        msl_args.append('0x000000ca')
    if '.no-user-varying.' in shader:
        msl_args.append('--msl-no-clip-distance-user-varying')
    if '.shader-inputs.' in shader:
        # Arbitrary for testing purposes.
        msl_args.append('--msl-shader-input')
        msl_args.append('0')
        msl_args.append('u8')
        msl_args.append('2')
        msl_args.append('--msl-shader-input')
        msl_args.append('1')
        msl_args.append('u16')
        msl_args.append('3')
        msl_args.append('--msl-shader-input')
        msl_args.append('6')
        msl_args.append('other')
        msl_args.append('4')
    if '.multi-patch.' in shader:
        msl_args.append('--msl-multi-patch-workgroup')
        # Arbitrary for testing purposes.
        msl_args.append('--msl-shader-input')
        msl_args.append('0')
        msl_args.append('any32')
        msl_args.append('3')
        msl_args.append('--msl-shader-input')
        msl_args.append('1')
        msl_args.append('any16')
        msl_args.append('2')
    if '.raw-tess-in.' in shader:
        msl_args.append('--msl-raw-buffer-tese-input')
    if '.for-tess.' in shader:
        msl_args.append('--msl-vertex-for-tessellation')
    if '.fixed-sample-mask.' in shader:
        msl_args.append('--msl-additional-fixed-sample-mask')
        msl_args.append('0x00000022')
    if '.arrayed-subpass.' in shader:
        msl_args.append('--msl-arrayed-subpass-input')
    if '.1d-as-2d.' in shader:
        msl_args.append('--msl-texture-1d-as-2d')
    if '.simd.' in shader:
        msl_args.append('--msl-ios-use-simdgroup-functions')
    if '.emulate-subgroup.' in shader:
        msl_args.append('--msl-emulate-subgroups')
    if '.fixed-subgroup.' in shader:
        # Arbitrary for testing purposes.
        msl_args.append('--msl-fixed-subgroup-size')
        msl_args.append('32')
    if '.force-sample.' in shader:
        msl_args.append('--msl-force-sample-rate-shading')
    if '.discard-checks.' in shader:
        msl_args.append('--msl-check-discarded-frag-stores')
    if '.lod-as-grad.' in shader:
        msl_args.append('--msl-sample-dref-lod-array-as-grad')
    if '.agx-cube-grad.' in shader:
        msl_args.append('--msl-agx-manual-cube-grad-fixup')
    if '.decoration-binding.' in shader:
        msl_args.append('--msl-decoration-binding')
    if '.rich-descriptor.' in shader:
        msl_args.append('--msl-runtime-array-rich-descriptor')
    if '.replace-recursive-inputs.' in shader:
        msl_args.append('--msl-replace-recursive-inputs')
    if '.mask-location-0.' in shader:
        msl_args.append('--mask-stage-output-location')
        msl_args.append('0')
        msl_args.append('0')
    if '.mask-location-1.' in shader:
        msl_args.append('--mask-stage-output-location')
        msl_args.append('1')
        msl_args.append('0')
    if '.mask-position.' in shader:
        msl_args.append('--mask-stage-output-builtin')
        msl_args.append('Position')
    if '.mask-point-size.' in shader:
        msl_args.append('--mask-stage-output-builtin')
        msl_args.append('PointSize')
    if '.mask-clip-distance.' in shader:
        msl_args.append('--mask-stage-output-builtin')
        msl_args.append('ClipDistance')
    if '.relax-nan.' in shader:
        msl_args.append('--relax-nan-checks')

    subprocess.check_call(msl_args)

    if not shader_is_invalid_spirv(msl_path):
        subprocess.check_call([paths.spirv_val, '--allow-localsizeid', '--scalar-block-layout', '--target-env', spirv_env, spirv_path])

    return (spirv_path, msl_path)

def shader_model_hlsl(shader):
    if '.vert' in shader:
        if '.sm30.' in shader:
            return '-Tvs_3_0'
        else:
            return '-Tvs_5_1'
    elif '.frag' in shader:
        if '.sm30.' in shader:
            return '-Tps_3_0'
        else:
            return '-Tps_5_1'
    elif '.comp' in shader:
        return '-Tcs_5_1'
    elif '.mesh' in shader:
        return '-Tms_6_5'
    elif '.task' in shader:
        return '-Tas_6_5'
    else:
        return None

def shader_to_win_path(shader):
    # It's (very) convenient to be able to run HLSL testing in wine on Unix-likes, so support that.
    try:
        with subprocess.Popen(['winepath', '-w', shader], stdout = subprocess.PIPE, stderr = subprocess.PIPE) as f:
            stdout_data, stderr_data = f.communicate()
            return stdout_data.decode('utf-8')
    except OSError as oe:
        if (oe.errno != errno.ENOENT): # Ignore not found errors
            return shader
    except subprocess.CalledProcessError:
        raise

    return shader

ignore_fxc = False
def validate_shader_hlsl(shader, force_no_external_validation, paths):
    test_glslang = True
    if '.nonuniformresource.' in shader:
        test_glslang = False
    if '.fxconly.' in shader:
        test_glslang = False
    if '.task' in shader or '.mesh' in shader:
        test_glslang = False

    hlsl_args = [paths.glslang, '--amb', '-e', 'main', '-D', '--target-env', 'vulkan1.1', '-V', shader]
    if '.sm30.' in shader:
        hlsl_args.append('--hlsl-dx9-compatible')

    if test_glslang:
        subprocess.check_call(hlsl_args)

    is_no_fxc = '.nofxc.' in shader
    global ignore_fxc
    if (not ignore_fxc) and (not force_no_external_validation) and (not is_no_fxc):
        try:
            win_path = shader_to_win_path(shader)
            args = ['fxc', '-nologo', shader_model_hlsl(shader), win_path]
            if '.nonuniformresource.' in shader:
                args.append('/enable_unbounded_descriptor_tables')
            subprocess.check_call(args)
        except OSError as oe:
            if (oe.errno != errno.ENOENT): # Ignore not found errors
                print('Failed to run FXC.')
                ignore_fxc = True
                raise
            else:
                print('Could not find FXC.')
                ignore_fxc = True
        except subprocess.CalledProcessError:
            print('Failed compiling HLSL shader:', shader, 'with FXC.')
            raise RuntimeError('Failed compiling HLSL shader')

def shader_to_sm(shader):
    if '.sm62.' in shader:
        return '62'
    elif '.sm60.' in shader:
        return '60'
    elif '.sm51.' in shader:
        return '51'
    elif '.sm30.' in shader:
        return '30'
    else:
        return '50'

def cross_compile_hlsl(shader, spirv, opt, force_no_external_validation, iterations, paths):
    spirv_path = create_temporary()
    hlsl_path = create_temporary(os.path.basename(shader))

    spirv_16 = '.spv16.' in shader
    spirv_14 = '.spv14.' in shader

    if spirv_16:
        spirv_env = 'spv1.6'
        glslang_env = 'spirv1.6'
    elif spirv_14:
        spirv_env = 'vulkan1.1spv1.4'
        glslang_env = 'spirv1.4'
    else:
        spirv_env = 'vulkan1.1'
        glslang_env = 'vulkan1.1'

    spirv_cmd = [paths.spirv_as, '--preserve-numeric-ids', '--target-env', spirv_env, '-o', spirv_path, shader]

    if spirv:
        subprocess.check_call(spirv_cmd)
    else:
        subprocess.check_call([paths.glslang, '--amb', '--target-env', glslang_env, '-V', '-o', spirv_path, shader])

    if opt and (not shader_is_invalid_spirv(hlsl_path)):
        subprocess.check_call([paths.spirv_opt, '--skip-validation', '-O', '-o', spirv_path, spirv_path])

    spirv_cross_path = paths.spirv_cross

    sm = shader_to_sm(shader)

    hlsl_args = [spirv_cross_path, '--entry', 'main', '--output', hlsl_path, spirv_path, '--hlsl-enable-compat', '--hlsl', '--shader-model', sm, '--iterations', str(iterations)]
    if '.line.' in shader:
        hlsl_args.append('--emit-line-directives')
    if '.flatten.' in shader:
        hlsl_args.append('--flatten-ubo')
    if '.force-uav.' in shader:
        hlsl_args.append('--hlsl-force-storage-buffer-as-uav')
    if '.zero-initialize.' in shader:
        hlsl_args.append('--force-zero-initialized-variables')
    if '.nonwritable-uav-texture.' in shader:
        hlsl_args.append('--hlsl-nonwritable-uav-texture-as-srv')
    if '.native-16bit.' in shader:
        hlsl_args.append('--hlsl-enable-16bit-types')
    if '.flatten-matrix-vertex-input.' in shader:
        hlsl_args.append('--hlsl-flatten-matrix-vertex-input-semantics')
    if '.relax-nan.' in shader:
        hlsl_args.append('--relax-nan-checks')
    if '.structured.' in shader:
        hlsl_args.append('--hlsl-preserve-structured-buffers')
    if '.flip-vert-y.' in shader:
        hlsl_args.append('--flip-vert-y')

    subprocess.check_call(hlsl_args)

    if not shader_is_invalid_spirv(hlsl_path):
        subprocess.check_call([paths.spirv_val, '--allow-localsizeid', '--scalar-block-layout', '--target-env', spirv_env, spirv_path])

    validate_shader_hlsl(hlsl_path, force_no_external_validation, paths)

    return (spirv_path, hlsl_path)

def cross_compile_reflect(shader, spirv, opt, iterations, paths):
    spirv_path = create_temporary()
    reflect_path = create_temporary(os.path.basename(shader))

    spirv_cmd = [paths.spirv_as, '--preserve-numeric-ids', '--target-env', 'vulkan1.1', '-o', spirv_path, shader]

    if spirv:
        subprocess.check_call(spirv_cmd)
    else:
        subprocess.check_call([paths.glslang, '--amb', '--target-env', 'vulkan1.1', '-V', '-o', spirv_path, shader])

    if opt and (not shader_is_invalid_spirv(reflect_path)):
        subprocess.check_call([paths.spirv_opt, '--skip-validation', '-O', '-o', spirv_path, spirv_path])

    spirv_cross_path = paths.spirv_cross

    sm = shader_to_sm(shader)
    subprocess.check_call([spirv_cross_path, '--entry', 'main', '--output', reflect_path, spirv_path, '--reflect', '--iterations', str(iterations)])
    return (spirv_path, reflect_path)

def validate_shader(shader, vulkan, paths):
    if vulkan:
        spirv_14 = '.spv14.' in shader
        glslang_env = 'spirv1.4' if spirv_14 else 'vulkan1.1'
        subprocess.check_call([paths.glslang, '--amb', '--target-env', glslang_env, '-V', shader])
    else:
        subprocess.check_call([paths.glslang, shader])

def cross_compile(shader, vulkan, spirv, invalid_spirv, eliminate, is_legacy, force_es, flatten_ubo, sso, flatten_dim, opt, push_ubo, iterations, paths):
    spirv_path = create_temporary()
    glsl_path = create_temporary(os.path.basename(shader))

    spirv_16 = '.spv16.' in shader
    spirv_14 = '.spv14.' in shader
    if spirv_16:
        spirv_env = 'spv1.6'
        glslang_env = 'spirv1.6'
    elif spirv_14:
        spirv_env = 'vulkan1.1spv1.4'
        glslang_env = 'spirv1.4'
    else:
        spirv_env = 'vulkan1.1'
        glslang_env = 'vulkan1.1'

    if vulkan or spirv:
        vulkan_glsl_path = create_temporary('vk' + os.path.basename(shader))

    spirv_cmd = [paths.spirv_as, '--preserve-numeric-ids', '--target-env', spirv_env, '-o', spirv_path, shader]

    if spirv:
        subprocess.check_call(spirv_cmd)
    else:
        glslang_cmd = [paths.glslang, '--amb', '--target-env', glslang_env, '-V', '-o', spirv_path, shader]
        if '.g.' in shader:
            glslang_cmd.append('-g')
        if '.gV.' in shader:
            glslang_cmd.append('-gV')
        subprocess.check_call(glslang_cmd)

    if opt and (not invalid_spirv):
        subprocess.check_call([paths.spirv_opt, '--skip-validation', '-O', '-o', spirv_path, spirv_path])

    if not invalid_spirv:
        subprocess.check_call([paths.spirv_val, '--allow-localsizeid', '--scalar-block-layout', '--target-env', spirv_env, spirv_path])

    extra_args = ['--iterations', str(iterations)]
    if eliminate:
        extra_args += ['--remove-unused-variables']
    if is_legacy:
        extra_args += ['--version', '100', '--es']
    if force_es:
        extra_args += ['--version', '310', '--es']
    if flatten_ubo:
        extra_args += ['--flatten-ubo']
    if sso:
        extra_args += ['--separate-shader-objects']
    if flatten_dim:
        extra_args += ['--flatten-multidimensional-arrays']
    if push_ubo:
        extra_args += ['--glsl-emit-push-constant-as-ubo']
    if '.line.' in shader:
        extra_args += ['--emit-line-directives']
    if '.no-samplerless.' in shader:
        extra_args += ['--vulkan-glsl-disable-ext-samplerless-texture-functions']
    if '.no-qualifier-deduction.' in shader:
        extra_args += ['--disable-storage-image-qualifier-deduction']
    if '.framebuffer-fetch.' in shader:
        extra_args += ['--glsl-remap-ext-framebuffer-fetch', '0', '0']
        extra_args += ['--glsl-remap-ext-framebuffer-fetch', '1', '1']
        extra_args += ['--glsl-remap-ext-framebuffer-fetch', '2', '2']
        extra_args += ['--glsl-remap-ext-framebuffer-fetch', '3', '3']
    if '.framebuffer-fetch-noncoherent.' in shader:
        extra_args += ['--glsl-ext-framebuffer-fetch-noncoherent']
    if '.zero-initialize.' in shader:
        extra_args += ['--force-zero-initialized-variables']
    if '.force-flattened-io.' in shader:
        extra_args += ['--glsl-force-flattened-io-blocks']
    if '.relax-nan.' in shader:
        extra_args.append('--relax-nan-checks')

    spirv_cross_path = paths.spirv_cross

    # A shader might not be possible to make valid GLSL from, skip validation for this case.
    if (not ('nocompat' in glsl_path)) or (not vulkan):
        subprocess.check_call([spirv_cross_path, '--entry', 'main', '--output', glsl_path, spirv_path] + extra_args)
        if not 'nocompat' in glsl_path:
            validate_shader(glsl_path, False, paths)
    else:
        remove_file(glsl_path)
        glsl_path = None

    if (vulkan or spirv) and (not is_legacy):
        subprocess.check_call([spirv_cross_path, '--entry', 'main', '-V', '--output', vulkan_glsl_path, spirv_path] + extra_args)
        validate_shader(vulkan_glsl_path, True, paths)
        # SPIR-V shaders might just want to validate Vulkan GLSL output, we don't always care about the output.
        if not vulkan:
            remove_file(vulkan_glsl_path)

    return (spirv_path, glsl_path, vulkan_glsl_path if vulkan else None)

def make_unix_newline(buf):
    decoded = codecs.decode(buf, 'utf-8')
    decoded = decoded.replace('\r', '')
    return codecs.encode(decoded, 'utf-8')

def md5_for_file(path):
    md5 = hashlib.md5()
    with open(path, 'rb') as f:
        for chunk in iter(lambda: make_unix_newline(f.read(8192)), b''):
            md5.update(chunk)
    return md5.digest()

def make_reference_dir(path):
    base = os.path.dirname(path)
    if not os.path.exists(base):
        os.makedirs(base)

def reference_path(directory, relpath, opt):
    split_paths = os.path.split(directory)
    reference_dir = os.path.join(split_paths[0], 'reference/' + ('opt/' if opt else ''))
    reference_dir = os.path.join(reference_dir, split_paths[1])
    return os.path.join(reference_dir, relpath)

def regression_check_reflect(shader, json_file, args):
    reference = reference_path(shader[0], shader[1], args.opt) + '.json'
    joined_path = os.path.join(shader[0], shader[1])
    print('Reference shader reflection path:', reference)
    if os.path.exists(reference):
        actual = md5_for_file(json_file)
        expected = md5_for_file(reference)
        if actual != expected:
            if args.update:
                print('Generated reflection json has changed for {}!'.format(reference))
                # If we expect changes, update the reference file.
                if os.path.exists(reference):
                    remove_file(reference)
                make_reference_dir(reference)
                shutil.move(json_file, reference)
            else:
                print('Generated reflection json in {} does not match reference {}!'.format(json_file, reference))
                if args.diff:
                    diff_path = generate_diff_file(reference, glsl)
                    with open(diff_path, 'r') as f:
                        print('')
                        print('Diff:')
                        print(f.read())
                        print('')
                    remove_file(diff_path)
                else:
                    with open(json_file, 'r') as f:
                        print('')
                        print('Generated:')
                        print('======================')
                        print(f.read())
                        print('======================')
                        print('')

                # Otherwise, fail the test. Keep the shader file around so we can inspect.
                if not args.keep:
                    remove_file(json_file)

                raise RuntimeError('Does not match reference')
        else:
            remove_file(json_file)
    else:
        print('Found new shader {}. Placing generated source code in {}'.format(joined_path, reference))
        make_reference_dir(reference)
        shutil.move(json_file, reference)

def generate_diff_file(origin, generated):
    diff_destination = create_temporary()
    with open(diff_destination, "w") as f:
        try:
            subprocess.check_call(["diff", origin, generated], stdout=f)
        except subprocess.CalledProcessError as e:
            # diff returns 1 when the files are different so we can safely
            # ignore this case.
            if e.returncode != 1:
                raise e

    return diff_destination

def regression_check(shader, glsl, args):
    reference = reference_path(shader[0], shader[1], args.opt)
    joined_path = os.path.join(shader[0], shader[1])
    print('Reference shader path:', reference)

    if os.path.exists(reference):
        if md5_for_file(glsl) != md5_for_file(reference):
            if args.update:
                print('Generated source code has changed for {}!'.format(reference))
                # If we expect changes, update the reference file.
                if os.path.exists(reference):
                    remove_file(reference)
                make_reference_dir(reference)
                shutil.move(glsl, reference)
            else:
                print('Generated source code in {} does not match reference {}!'.format(glsl, reference))
                if args.diff:
                    diff_path = generate_diff_file(reference, glsl)
                    with open(diff_path, 'r') as f:
                        print('')
                        print('Diff:')
                        print(f.read())
                        print('')
                    remove_file(diff_path)
                else:
                    with open(glsl, 'r') as f:
                        print('')
                        print('Generated:')
                        print('======================')
                        print(f.read())
                        print('======================')
                        print('')

                # Otherwise, fail the test. Keep the shader file around so we can inspect.
                if not args.keep:
                    remove_file(glsl)
                raise RuntimeError('Does not match reference')
        else:
            remove_file(glsl)
    else:
        print('Found new shader {}. Placing generated source code in {}'.format(joined_path, reference))
        make_reference_dir(reference)
        shutil.move(glsl, reference)

def shader_is_vulkan(shader):
    return '.vk.' in shader

def shader_is_desktop(shader):
    return '.desktop.' in shader

def shader_is_eliminate_dead_variables(shader):
    return '.noeliminate.' not in shader

def shader_is_spirv(shader):
    return '.asm.' in shader

def shader_is_invalid_spirv(shader):
    return '.invalid.' in shader

def shader_is_legacy(shader):
    return '.legacy.' in shader

def shader_is_force_es(shader):
    return '.es.' in shader

def shader_is_flatten_ubo(shader):
    return '.flatten.' in shader

def shader_is_sso(shader):
    return '.sso.' in shader

def shader_is_flatten_dimensions(shader):
    return '.flatten_dim.' in shader

def shader_is_noopt(shader):
    return '.noopt.' in shader

def shader_is_push_ubo(shader):
    return '.push-ubo.' in shader

def test_shader(stats, shader, args, paths):
    joined_path = os.path.join(shader[0], shader[1])
    vulkan = shader_is_vulkan(shader[1])
    desktop = shader_is_desktop(shader[1])
    eliminate = shader_is_eliminate_dead_variables(shader[1])
    is_spirv = shader_is_spirv(shader[1])
    invalid_spirv = shader_is_invalid_spirv(shader[1])
    is_legacy = shader_is_legacy(shader[1])
    force_es = shader_is_force_es(shader[1])
    flatten_ubo = shader_is_flatten_ubo(shader[1])
    sso = shader_is_sso(shader[1])
    flatten_dim = shader_is_flatten_dimensions(shader[1])
    noopt = shader_is_noopt(shader[1])
    push_ubo = shader_is_push_ubo(shader[1])

    print('Testing shader:', joined_path)
    spirv, glsl, vulkan_glsl = cross_compile(joined_path, vulkan, is_spirv, invalid_spirv, eliminate, is_legacy, force_es, flatten_ubo, sso, flatten_dim, args.opt and (not noopt), push_ubo, args.iterations, paths)

    # Only test GLSL stats if we have a shader following GL semantics.
    if stats and (not vulkan) and (not is_spirv) and (not desktop):
        cross_stats = get_shader_stats(glsl)

    if glsl:
        regression_check(shader, glsl, args)
    if vulkan_glsl:
        regression_check((shader[0], shader[1] + '.vk'), vulkan_glsl, args)

    remove_file(spirv)

    if stats and (not vulkan) and (not is_spirv) and (not desktop):
        pristine_stats = get_shader_stats(joined_path)

        a = []
        a.append(shader[1])
        for i in pristine_stats:
            a.append(str(i))
        for i in cross_stats:
            a.append(str(i))
        print(','.join(a), file = stats)

def test_shader_msl(stats, shader, args, paths):
    joined_path = os.path.join(shader[0], shader[1])
    print('\nTesting MSL shader:', joined_path)
    is_spirv = shader_is_spirv(shader[1])
    noopt = shader_is_noopt(shader[1])
    spirv, msl = cross_compile_msl(joined_path, is_spirv, args.opt and (not noopt), args.iterations, paths)
    regression_check(shader, msl, args)

    # Uncomment the following line to print the temp SPIR-V file path.
    # This temp SPIR-V file is not deleted until after the Metal validation step below.
    # If Metal validation fails, the temp SPIR-V file can be copied out and
    # used as input to an invocation of spirv-cross to debug from Xcode directly.
    # To do so, build spriv-cross using `make DEBUG=1`, then run the spriv-cross
    # executable from Xcode using args: `--msl --entry main --output msl_path spirv_path`.
#    print('SPRIV shader: ' + spirv)

    shader_is_msl22 = '.msl22.' in joined_path
    shader_is_msl23 = '.msl23.' in joined_path
    shader_is_msl24 = '.msl24.' in joined_path
    shader_is_msl30 = '.msl3.' in joined_path
    shader_is_msl31 = '.msl31.' in joined_path
    skip_validation = (shader_is_msl22 and (not args.msl22)) or \
        (shader_is_msl23 and (not args.msl23)) or \
        (shader_is_msl24 and (not args.msl24)) or \
        (shader_is_msl30 and (not args.msl30)) or \
        (shader_is_msl31 and (not args.msl31))

    if skip_validation:
        print('Skipping validation for {} due to lack of toolchain support.'.format(joined_path))

    if '.invalid.' in joined_path:
        skip_validation = True

    if (not args.force_no_external_validation) and (not skip_validation):
        validate_shader_msl(shader, args.opt)

    remove_file(spirv)

def test_shader_hlsl(stats, shader, args, paths):
    joined_path = os.path.join(shader[0], shader[1])
    print('Testing HLSL shader:', joined_path)
    is_spirv = shader_is_spirv(shader[1])
    noopt = shader_is_noopt(shader[1])
    spirv, hlsl = cross_compile_hlsl(joined_path, is_spirv, args.opt and (not noopt), args.force_no_external_validation, args.iterations, paths)
    regression_check(shader, hlsl, args)
    remove_file(spirv)

def test_shader_reflect(stats, shader, args, paths):
    joined_path = os.path.join(shader[0], shader[1])
    print('Testing shader reflection:', joined_path)
    is_spirv = shader_is_spirv(shader[1])
    noopt = shader_is_noopt(shader[1])
    spirv, reflect = cross_compile_reflect(joined_path, is_spirv, args.opt and (not noopt), args.iterations, paths)
    regression_check_reflect(shader, reflect, args)
    remove_file(spirv)

def test_shader_file(relpath, stats, args, backend):
    paths = Paths(args.spirv_cross, args.glslang, args.spirv_as, args.spirv_val, args.spirv_opt)
    try:
        if backend == 'msl':
            test_shader_msl(stats, (args.folder, relpath), args, paths)
        elif backend == 'hlsl':
            test_shader_hlsl(stats, (args.folder, relpath), args, paths)
        elif backend == 'reflect':
            test_shader_reflect(stats, (args.folder, relpath), args, paths)
        else:
            test_shader(stats, (args.folder, relpath), args, paths)
        return None
    except Exception as e:
        return e

def test_shaders_helper(stats, backend, args):
    all_files = []
    for root, dirs, files in os.walk(os.path.join(args.folder)):
        files = [ f for f in files if not f.startswith(".") ]   #ignore system files (esp OSX)
        for i in files:
            path = os.path.join(root, i)
            relpath = os.path.relpath(path, args.folder)
            all_files.append(relpath)

    # The child processes in parallel execution mode don't have the proper state for the global args variable, so
    # at this point we need to switch to explicit arguments
    if args.parallel:
        with multiprocessing.Pool(multiprocessing.cpu_count()) as pool:
            results = []
            for f in all_files:
                results.append(pool.apply_async(test_shader_file,
                    args = (f, stats, args, backend)))

            pool.close()
            pool.join()
            results_completed = [res.get() for res in results]

            for error in results_completed:
                if error is not None:
                    print('Error:', error)
                    sys.exit(1)

    else:
        for i in all_files:
            e = test_shader_file(i, stats, args, backend)
            if e is not None:
                print('Error:', e)
                sys.exit(1)

def test_shaders(backend, args):
    if args.malisc:
        with open('stats.csv', 'w') as stats:
            print('Shader,OrigRegs,OrigUniRegs,OrigALUShort,OrigLSShort,OrigTEXShort,OrigALULong,OrigLSLong,OrigTEXLong,CrossRegs,CrossUniRegs,CrossALUShort,CrossLSShort,CrossTEXShort,CrossALULong,CrossLSLong,CrossTEXLong', file = stats)
            test_shaders_helper(stats, backend, args)
    else:
        test_shaders_helper(None, backend, args)

def main():
    parser = argparse.ArgumentParser(description = 'Script for regression testing.')
    parser.add_argument('folder',
            help = 'Folder containing shader files to test.')
    parser.add_argument('--update',
            action = 'store_true',
            help = 'Updates reference files if there is a mismatch. Use when legitimate changes in output is found.')
    parser.add_argument('--keep',
            action = 'store_true',
            help = 'Leave failed GLSL shaders on disk if they fail regression. Useful for debugging.')
    parser.add_argument('--diff',
            action = 'store_true',
            help = 'Displays a diff instead of the generated output on failure. Useful for debugging.')
    parser.add_argument('--malisc',
            action = 'store_true',
            help = 'Use malisc offline compiler to determine static cycle counts before and after spirv-cross.')
    parser.add_argument('--msl',
            action = 'store_true',
            help = 'Test Metal backend.')
    parser.add_argument('--metal',
            action = 'store_true',
            help = 'Deprecated Metal option. Use --msl instead.')
    parser.add_argument('--hlsl',
            action = 'store_true',
            help = 'Test HLSL backend.')
    parser.add_argument('--force-no-external-validation',
            action = 'store_true',
            help = 'Disable all external validation.')
    parser.add_argument('--opt',
            action = 'store_true',
            help = 'Run SPIRV-Tools optimization passes as well.')
    parser.add_argument('--reflect',
            action = 'store_true',
            help = 'Test reflection backend.')
    parser.add_argument('--parallel',
            action = 'store_true',
            help = 'Execute tests in parallel.  Useful for doing regression quickly, but bad for debugging and stat output.')
    parser.add_argument('--spirv-cross',
            default = './spirv-cross',
            help = 'Explicit path to spirv-cross')
    parser.add_argument('--glslang',
            default = 'glslangValidator',
            help = 'Explicit path to glslangValidator')
    parser.add_argument('--spirv-as',
            default = 'spirv-as',
            help = 'Explicit path to spirv-as')
    parser.add_argument('--spirv-val',
            default = 'spirv-val',
            help = 'Explicit path to spirv-val')
    parser.add_argument('--spirv-opt',
            default = 'spirv-opt',
            help = 'Explicit path to spirv-opt')
    parser.add_argument('--iterations',
            default = 1,
            type = int,
            help = 'Number of iterations to run SPIRV-Cross (benchmarking)')

    args = parser.parse_args()
    if not args.folder:
        sys.stderr.write('Need shader folder.\n')
        sys.exit(1)

    if (args.parallel and (args.malisc or args.force_no_external_validation or args.update)):
        sys.stderr.write('Parallel execution is disabled when using the flags --update, --malisc or --force-no-external-validation\n')
        args.parallel = False

    args.msl22 = False
    args.msl23 = False
    args.msl24 = False
    args.msl30 = False
    args.msl31 = False
    if args.msl:
        print_msl_compiler_version()
        args.msl22 = msl_compiler_supports_version('-std=macos-metal2.2')
        args.msl23 = msl_compiler_supports_version('-std=macos-metal2.3')
        args.msl24 = msl_compiler_supports_version('-std=macos-metal2.4')
        args.msl30 = msl_compiler_supports_version('-std=metal3.0')
        args.msl31 = msl_compiler_supports_version('-std=metal3.1')

    backend = 'glsl'
    if (args.msl or args.metal):
        backend = 'msl'
    elif args.hlsl:
        backend = 'hlsl'
    elif args.reflect:
        backend = 'reflect'

    test_shaders(backend, args)
    if args.malisc:
        print('Stats in stats.csv!')
    print('Tests completed!')

if __name__ == '__main__':
    main()
