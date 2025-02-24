# Licensed under the Apache License: http://www.apache.org/licenses/LICENSE-2.0
# For details: https://bitbucket.org/ned/coveragepy/src/default/NOTICE.txt

"""Code coverage measurement for Python"""

# Distutils setup for coverage.py
# This file is used unchanged under all versions of Python, 2.x and 3.x.

import os
import sys

from setuptools import setup
from distutils.core import Extension                # pylint: disable=no-name-in-module, import-error
from distutils.command.build_ext import build_ext   # pylint: disable=no-name-in-module, import-error
from distutils import errors                        # pylint: disable=no-name-in-module

# Get or massage our metadata.  We exec coverage/version.py so we can avoid
# importing the product code into setup.py.

classifiers = """\
Environment :: Console
Intended Audience :: Developers
License :: OSI Approved :: Apache Software License
Operating System :: OS Independent
Programming Language :: Python :: 2.6
Programming Language :: Python :: 2.7
Programming Language :: Python :: 3.3
Programming Language :: Python :: 3.4
Programming Language :: Python :: 3.5
Programming Language :: Python :: Implementation :: CPython
Programming Language :: Python :: Implementation :: PyPy
Topic :: Software Development :: Quality Assurance
Topic :: Software Development :: Testing
"""

cov_ver_py = os.path.join(os.path.split(__file__)[0], "coverage/version.py")
with open(cov_ver_py) as version_file:
    # __doc__ will be overwritten by version.py.
    doc = __doc__
    # Keep pylint happy.
    __version__ = __url__ = version_info = ""
    # Execute the code in version.py.
    exec(compile(version_file.read(), cov_ver_py, 'exec'))

with open("README.rst") as readme:
    long_description = readme.read().replace("http://coverage.readthedocs.org", __url__)

classifier_list = classifiers.splitlines()

if version_info[3] == 'alpha':
    devstat = "3 - Alpha"
elif version_info[3] in ['beta', 'candidate']:
    devstat = "4 - Beta"
else:
    assert version_info[3] == 'final'
    devstat = "5 - Production/Stable"
classifier_list.append("Development Status :: " + devstat)

# Create the keyword arguments for setup()

setup_args = dict(
    name='coverage',
    version=__version__,

    packages=[
        'coverage',
    ],

    package_data={
        'coverage': [
            'htmlfiles/*.*',
        ]
    },

    entry_points={
        # Install a script as "coverage", and as "coverage[23]", and as
        # "coverage-2.7" (or whatever).
        'console_scripts': [
            'coverage = coverage.cmdline:main',
            'coverage%d = coverage.cmdline:main' % sys.version_info[:1],
            'coverage-%d.%d = coverage.cmdline:main' % sys.version_info[:2],
        ],
    },

    # We need to get HTML assets from our htmlfiles directory.
    zip_safe=False,

    author='Ned Batchelder and others',
    author_email='ned@nedbatchelder.com',
    description=doc,
    long_description=long_description,
    keywords='code coverage testing',
    license='Apache 2.0',
    classifiers=classifier_list,
    url=__url__,
)

# A replacement for the build_ext command which raises a single exception
# if the build fails, so we can fallback nicely.

ext_errors = (
    errors.CCompilerError,
    errors.DistutilsExecError,
    errors.DistutilsPlatformError,
)
if sys.platform == 'win32':
    # distutils.msvc9compiler can raise an IOError when failing to
    # find the compiler
    ext_errors += (IOError,)


class BuildFailed(Exception):
    """Raise this to indicate the C extension wouldn't build."""
    def __init__(self):
        Exception.__init__(self)
        self.cause = sys.exc_info()[1]      # work around py 2/3 different syntax


class ve_build_ext(build_ext):
    """Build C extensions, but fail with a straightforward exception."""

    def run(self):
        """Wrap `run` with `BuildFailed`."""
        try:
            build_ext.run(self)
        except errors.DistutilsPlatformError:
            raise BuildFailed()

    def build_extension(self, ext):
        """Wrap `build_extension` with `BuildFailed`."""
        try:
            # Uncomment to test compile failure handling:
            #   raise errors.CCompilerError("OOPS")
            build_ext.build_extension(self, ext)
        except ext_errors:
            raise BuildFailed()
        except ValueError as err:
            # this can happen on Windows 64 bit, see Python issue 7511
            if "'path'" in str(err):    # works with both py 2/3
                raise BuildFailed()
            raise

# There are a few reasons we might not be able to compile the C extension.
# Figure out if we should attempt the C extension or not.

compile_extension = True

if sys.platform.startswith('java'):
    # Jython can't compile C extensions
    compile_extension = False

if '__pypy__' in sys.builtin_module_names:
    # Pypy can't compile C extensions
    compile_extension = False

if compile_extension:
    setup_args.update(dict(
        ext_modules=[
            Extension(
                "coverage.tracer",
                sources=[
                    "coverage/ctracer/datastack.c",
                    "coverage/ctracer/filedisp.c",
                    "coverage/ctracer/module.c",
                    "coverage/ctracer/tracer.c",
                ],
            ),
        ],
        cmdclass={
            'build_ext': ve_build_ext,
        },
    ))

# Py3.x-specific details.

if sys.version_info >= (3, 0):
    setup_args.update(dict(
        use_2to3=False,
    ))


def main():
    """Actually invoke setup() with the arguments we built above."""
    # For a variety of reasons, it might not be possible to install the C
    # extension.  Try it with, and if it fails, try it without.
    try:
        setup(**setup_args)
    except BuildFailed as exc:
        msg = "Couldn't install with extension module, trying without it..."
        exc_msg = "%s: %s" % (exc.__class__.__name__, exc.cause)
        print("**\n** %s\n** %s\n**" % (msg, exc_msg))

        del setup_args['ext_modules']
        setup(**setup_args)

if __name__ == '__main__':
    main()
