from setuptools import setup, Extension
from setuptools.command.build_ext import build_ext
import sys
import re
import os
import setuptools

__version__ = '0.0.1'


class get_pybind_include(object):
    """Helper class to determine the pybind11 include path
    The purpose of this class is to postpone importing pybind11
    until it is actually installed, so that the ``get_include()``
    method can be invoked. """

    def __init__(self, user=False):
        self.user = user

    def __str__(self):
        import pybind11
        return pybind11.get_include(self.user)


filament_dir = '/home/nan/p/filament/'
filament_build_dir = os.path.join(filament_dir, 'out/cmake-release')

includes = [
    'filament',
    'third_party/libsdl2',
    'third_party/imgui',
    'third_party/getopt',
    'libs/utils',
    'libs/math',
    'libs/filabridge',
    'libs/filamat',
]

include_dirs = [
    get_pybind_include(),
    get_pybind_include(user=True),
    os.path.join(filament_dir, 'samples'),
    os.path.join(filament_build_dir, 'samples')
]

include_dirs += [os.path.join(filament_dir, i, 'include') for i in includes]

lib_paths = [
    'filament',
    'third_party/libsdl2/tnt',
    'libs/math',
    'libs/filamat',
    'libs/utils',
    'third_party/getopt',
    'third_party/imgui/tnt',
    'libs/filagui',
    'samples'
]
libraries = ['filament', 'sdl2', 'math', 'filamat', 'utils', 'getopt',
             'imgui', 'filagui', 'sample-app']
library_dirs = [os.path.join(filament_build_dir, i) for i in lib_paths]

ext_modules = [
    Extension(
        'filament',
        ['filament.cpp', 'material_sandbox_copy.cpp'],
        include_dirs=include_dirs,
        language='c++',
        extra_compile_args=[
            '-std=c++14',
            '-static',
            '-fstrict-aliasing',
            '-fomit-frame-pointer',
            '-ffunction-sections',
            '-fdata-sections',
            '-Wno-unknown-pragmas',
            '-Wno-unused-function',
            '-stdlib=libc++',
            '-Wl,--gc-sections',
        ],
        extra_link_args=[
            '-lc++abi',
            '-static-libgcc',
            '-static-libstdc++',
            '-Bsymbolic-functions',
            '-Bsymbolic',
            '-z defs'
        ],
        libraries=libraries,
        library_dirs=library_dirs,
        runtime_library_dirs=[library_dirs[1]]
    ),
]


# As of Python 3.6, CCompiler has a `has_flag` method.
# cf http://bugs.python.org/issue26689
def has_flag(compiler, flagname):
    """Return a boolean indicating whether a flag name is supported on
    the specified compiler.
    """
    import tempfile
    with tempfile.NamedTemporaryFile('w', suffix='.cpp') as f:
        f.write('int main (int argc, char **argv) { return 0; }')
        try:
            compiler.compile([f.name], extra_postargs=[flagname])
        except setuptools.distutils.errors.CompileError:
            return False
    return True


def cpp_flag(compiler):
    """Return the -std=c++[11/14] compiler flag.
    The c++14 is prefered over c++11 (when it is available).
    """
    if has_flag(compiler, '-std=c++14'):
        return '-std=c++14'
    elif has_flag(compiler, '-std=c++11'):
        return '-std=c++11'
    else:
        raise RuntimeError('Unsupported compiler -- at least C++11 support '
                           'is needed!')


class BuildExt(build_ext):
    """A custom build extension for adding compiler-specific options."""
    c_opts = {
        'msvc': ['/EHsc'],
        'unix': [],
    }

    if sys.platform == 'darwin':
        c_opts['unix'] += ['-stdlib=libc++', '-mmacosx-version-min=10.7']

    def build_extensions(self):
        ct = self.compiler.compiler_type
        opts = self.c_opts.get(ct, [])
        if ct == 'unix':
            opts.append('-DVERSION_INFO="%s"' % self.distribution.get_version())
            opts.append(cpp_flag(self.compiler))
            if has_flag(self.compiler, '-fvisibility=hidden'):
                opts.append('-fvisibility=hidden')
        elif ct == 'msvc':
            opts.append('/DVERSION_INFO=\\"%s\\"' % self.distribution.get_version())
        for ext in self.extensions:
            ext.extra_compile_args = opts
        build_ext.build_extensions(self)


setup(
    name='filament',
    version=__version__,
    author='Adnan Yunus',
    author_email='adnan@artometa.com',
    url='https://github.com/google/filament',
    description='Python bindings for Filament',
    long_description='',
    ext_modules=ext_modules,
    install_requires=['pybind11>=2.2'],
    cmdclass={'build_ext': BuildExt},
    zip_safe=False
)
