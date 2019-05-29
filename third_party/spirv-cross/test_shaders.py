#!/usr/bin/env python3

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
        subprocess.check_call(['xcrun', '--sdk', 'iphoneos', 'metal', '--version'])
        print('...are the Metal compiler characteristics.\n')   # display after so xcrun FNF is silent
    except OSError as e:
        if (e.errno != errno.ENOENT):    # Ignore xcrun not found error
            raise

def path_to_msl_standard(shader):
    if '.ios.' in shader:
        if '.msl2.' in shader:
            return '-std=ios-metal2.0'
        elif '.msl21.' in shader:
            return '-std=ios-metal2.1'
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
        elif '.msl11.' in shader:
            return '-std=macos-metal1.1'
        else:
            return '-std=macos-metal1.2'

def path_to_msl_standard_cli(shader):
    if '.msl2.' in shader:
        return '20000'
    elif '.msl21.' in shader:
        return '20100'
    elif '.msl11.' in shader:
        return '10100'
    else:
        return '10200'

def validate_shader_msl(shader, opt):
    msl_path = reference_path(shader[0], shader[1], opt)
    try:
        if '.ios.' in msl_path:
            msl_os = 'iphoneos'
        else:
            msl_os = 'macosx'
        subprocess.check_call(['xcrun', '--sdk', msl_os, 'metal', '-x', 'metal', path_to_msl_standard(msl_path), '-Werror', '-Wno-unused-variable', msl_path])
        print('Compiled Metal shader: ' + msl_path)   # display after so xcrun FNF is silent
    except OSError as oe:
        if (oe.errno != errno.ENOENT):   # Ignore xcrun not found error
            raise
    except subprocess.CalledProcessError:
        print('Error compiling Metal shader: ' + msl_path)
        raise RuntimeError('Failed to compile Metal shader')

def cross_compile_msl(shader, spirv, opt, iterations, paths):
    spirv_path = create_temporary()
    msl_path = create_temporary(os.path.basename(shader))

    spirv_cmd = [paths.spirv_as, '--target-env', 'vulkan1.1', '-o', spirv_path, shader]
    if '.preserve.' in shader:
        spirv_cmd.append('--preserve-numeric-ids')

    if spirv:
        subprocess.check_call(spirv_cmd)
    else:
        subprocess.check_call([paths.glslang, '--target-env', 'vulkan1.1', '-V', '-o', spirv_path, shader])

    if opt:
        subprocess.check_call([paths.spirv_opt, '--skip-validation', '-O', '-o', spirv_path, spirv_path])

    spirv_cross_path = paths.spirv_cross

    msl_args = [spirv_cross_path, '--entry', 'main', '--output', msl_path, spirv_path, '--msl', '--iterations', str(iterations)]
    msl_args.append('--msl-version')
    msl_args.append(path_to_msl_standard_cli(shader))
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
    if '.texture-buffer-native.' in shader:
        msl_args.append('--msl-texture-buffer-native')
    if '.discrete.' in shader:
        # Arbitrary for testing purposes.
        msl_args.append('--msl-discrete-descriptor-set')
        msl_args.append('2')
        msl_args.append('--msl-discrete-descriptor-set')
        msl_args.append('3')
    if '.line.' in shader:
        msl_args.append('--emit-line-directives')

    subprocess.check_call(msl_args)

    if not shader_is_invalid_spirv(msl_path):
        subprocess.check_call([paths.spirv_val, '--target-env', 'vulkan1.1', spirv_path])

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
    if not '.nonuniformresource' in shader:
        # glslang HLSL does not support this, so rely on fxc to test it.
        subprocess.check_call([paths.glslang, '-e', 'main', '-D', '--target-env', 'vulkan1.1', '-V', shader])
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
    if '.sm60.' in shader:
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

    spirv_cmd = [paths.spirv_as, '--target-env', 'vulkan1.1', '-o', spirv_path, shader]
    if '.preserve.' in shader:
        spirv_cmd.append('--preserve-numeric-ids')

    if spirv:
        subprocess.check_call(spirv_cmd)
    else:
        subprocess.check_call([paths.glslang, '--target-env', 'vulkan1.1', '-V', '-o', spirv_path, shader])

    if opt:
        subprocess.check_call([paths.spirv_opt, '--skip-validation', '-O', '-o', spirv_path, spirv_path])

    spirv_cross_path = paths.spirv_cross

    sm = shader_to_sm(shader)

    hlsl_args = [spirv_cross_path, '--entry', 'main', '--output', hlsl_path, spirv_path, '--hlsl-enable-compat', '--hlsl', '--shader-model', sm, '--iterations', str(iterations)]
    if '.line.' in shader:
        hlsl_args.append('--emit-line-directives')
    subprocess.check_call(hlsl_args)

    if not shader_is_invalid_spirv(hlsl_path):
        subprocess.check_call([paths.spirv_val, '--target-env', 'vulkan1.1', spirv_path])

    validate_shader_hlsl(hlsl_path, force_no_external_validation, paths)
    
    return (spirv_path, hlsl_path)

def cross_compile_reflect(shader, spirv, opt, iterations, paths):
    spirv_path = create_temporary()
    reflect_path = create_temporary(os.path.basename(shader))

    spirv_cmd = [paths.spirv_as, '--target-env', 'vulkan1.1', '-o', spirv_path, shader]
    if '.preserve.' in shader:
        spirv_cmd.append('--preserve-numeric-ids')

    if spirv:
        subprocess.check_call(spirv_cmd)
    else:
        subprocess.check_call([paths.glslang, '--target-env', 'vulkan1.1', '-V', '-o', spirv_path, shader])

    if opt:
        subprocess.check_call([paths.spirv_opt, '--skip-validation', '-O', '-o', spirv_path, spirv_path])

    spirv_cross_path = paths.spirv_cross

    sm = shader_to_sm(shader)
    subprocess.check_call([spirv_cross_path, '--entry', 'main', '--output', reflect_path, spirv_path, '--reflect', '--iterations', str(iterations)])
    return (spirv_path, reflect_path)

def validate_shader(shader, vulkan, paths):
    if vulkan:
        subprocess.check_call([paths.glslang, '--target-env', 'vulkan1.1', '-V', shader])
    else:
        subprocess.check_call([paths.glslang, shader])

def cross_compile(shader, vulkan, spirv, invalid_spirv, eliminate, is_legacy, flatten_ubo, sso, flatten_dim, opt, push_ubo, iterations, paths):
    spirv_path = create_temporary()
    glsl_path = create_temporary(os.path.basename(shader))

    if vulkan or spirv:
        vulkan_glsl_path = create_temporary('vk' + os.path.basename(shader))

    spirv_cmd = [paths.spirv_as, '--target-env', 'vulkan1.1', '-o', spirv_path, shader]
    if '.preserve.' in shader:
        spirv_cmd.append('--preserve-numeric-ids')

    if spirv:
        subprocess.check_call(spirv_cmd)
    else:
        subprocess.check_call([paths.glslang, '--target-env', 'vulkan1.1', '-V', '-o', spirv_path, shader])

    if opt and (not invalid_spirv):
        subprocess.check_call([paths.spirv_opt, '--skip-validation', '-O', '-o', spirv_path, spirv_path])

    if not invalid_spirv:
        subprocess.check_call([paths.spirv_val, '--target-env', 'vulkan1.1', spirv_path])

    extra_args = ['--iterations', str(iterations)]
    if eliminate:
        extra_args += ['--remove-unused-variables']
    if is_legacy:
        extra_args += ['--version', '100', '--es']
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

    spirv_cross_path = paths.spirv_cross

    # A shader might not be possible to make valid GLSL from, skip validation for this case.
    if not ('nocompat' in glsl_path):
        subprocess.check_call([spirv_cross_path, '--entry', 'main', '--output', glsl_path, spirv_path] + extra_args)
        validate_shader(glsl_path, False, paths)
    else:
        remove_file(glsl_path)
        glsl_path = None

    if vulkan or spirv:
        subprocess.check_call([spirv_cross_path, '--entry', 'main', '--vulkan-semantics', '--output', vulkan_glsl_path, spirv_path] + extra_args)
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

def json_ordered(obj):
    if isinstance(obj, dict):
        return sorted((k, json_ordered(v)) for k, v in obj.items())
    if isinstance(obj, list):
        return sorted(json_ordered(x) for x in obj)
    else:
        return obj
    
def json_compare(json_a, json_b):
    return json_ordered(json_a) == json_ordered(json_b)

def regression_check_reflect(shader, json_file, args):
    reference = reference_path(shader[0], shader[1], args.opt) + '.json'
    joined_path = os.path.join(shader[0], shader[1])
    print('Reference shader reflection path:', reference)
    if os.path.exists(reference):
        actual = ''
        expected = ''
        with open(json_file) as f:
            actual_json = f.read();
            actual = json.loads(actual_json)
        with open(reference) as f:
            expected = json.load(f)
        if (json_compare(actual, expected) != True):
            if args.update:
                print('Generated reflection json has changed for {}!'.format(reference))
                # If we expect changes, update the reference file.
                if os.path.exists(reference):
                    remove_file(reference)
                make_reference_dir(reference)
                shutil.move(json_file, reference)
            else:
                print('Generated reflection json in {} does not match reference {}!'.format(json_file, reference))
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
    flatten_ubo = shader_is_flatten_ubo(shader[1])
    sso = shader_is_sso(shader[1])
    flatten_dim = shader_is_flatten_dimensions(shader[1])
    noopt = shader_is_noopt(shader[1])
    push_ubo = shader_is_push_ubo(shader[1])

    print('Testing shader:', joined_path)
    spirv, glsl, vulkan_glsl = cross_compile(joined_path, vulkan, is_spirv, invalid_spirv, eliminate, is_legacy, flatten_ubo, sso, flatten_dim, args.opt and (not noopt), push_ubo, args.iterations, paths)

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

    if not args.force_no_external_validation:
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
        pool = multiprocessing.Pool(multiprocessing.cpu_count())

        results = []
        for f in all_files:
            results.append(pool.apply_async(test_shader_file,
                args = (f, stats, args, backend)))

        for res in results:
            error = res.get()
            if error is not None:
                pool.close()
                pool.join()
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
        
    if args.msl:
        print_msl_compiler_version()

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
