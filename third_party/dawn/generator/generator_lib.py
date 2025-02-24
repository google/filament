#!/usr/bin/env python3
# Copyright 2019 The Dawn & Tint Authors
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are met:
#
# 1. Redistributions of source code must retain the above copyright notice, this
#    list of conditions and the following disclaimer.
#
# 2. Redistributions in binary form must reproduce the above copyright notice,
#    this list of conditions and the following disclaimer in the documentation
#    and/or other materials provided with the distribution.
#
# 3. Neither the name of the copyright holder nor the names of its
#    contributors may be used to endorse or promote products derived from
#    this software without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
# AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
# DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
# FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
# DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
# SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
# CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
# OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
# OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
"""Module to create generators that render multiple Jinja2 templates for GN.

A helper module that can be used to create generator scripts (clients)
that expand one or more Jinja2 templates, without outputs usable from
GN and Ninja build-based systems. See generator_lib.gni as well.

Clients should create a Generator sub-class, then call run_generator()
with a proper derived class instance.

Clients specify a list of FileRender operations, each one of them will
output a file into a temporary output directory through Jinja2 expansion.
All temporary output files are then grouped and written to into a single JSON
file, that acts as a convenient single GN output target. Use extract_json.py
to extract the output files from the JSON tarball in another GN action.

--depfile can be used to specify an output Ninja dependency file for the
JSON tarball, to ensure it is regenerated any time one of its dependencies
changes.

Finally, --expected-output-files can be used to check the list of generated
output files.
"""

import argparse, json, os, re, sys
from collections import namedtuple

# A FileRender represents a single Jinja2 template render operation:
#
#   template: Jinja2 template name, relative to --template-dir path.
#
#   output: Output file path, relative to temporary output directory.
#
#   params_dicts: iterable of (name:string -> value:string) dictionaries.
#       All of them will be merged before being sent as Jinja2 template
#       expansion parameters.
#
# Example:
#   FileRender('api.c', 'src/project_api.c', [{'PROJECT_VERSION': '1.0.0'}])
#
FileRender = namedtuple('FileRender', ['template', 'output', 'params_dicts'])

# A GeneratorOutput represent everything an invocation of the generator will
# produce.
#
#   renders: an iterable of FileRenders.
#
#   imported_templates: paths to additional templates that will be imported.
#       Trying to import with {% from %} will enforce that the file is listed
#       to ensure the dependency information produced is correct.
GeneratorOutput = namedtuple('GeneratorOutput',
                             ['renders', 'imported_templates'])

# The interface that must be implemented by generators.
class Generator:
    def get_description(self):
        """Return generator description for --help."""
        return ""

    def add_commandline_arguments(self, parser):
        """Add generator-specific argparse arguments."""
        pass

    def get_outputs(self, args):
        """Return the list of FileRender objects to process."""
        return []

    def get_dependencies(self, args):
        """Return a list of extra input dependencies."""
        return []


# Allow custom Jinja2 installation path through an additional python
# path from the arguments if present. This isn't done through the regular
# argparse because PreprocessingLoader uses jinja2 in the global scope before
# "main" gets to run.
#
# NOTE: If this argument appears several times, this only uses the first
#       value, while argparse would typically keep the last one!
kJinja2Path = '--jinja2-path'
try:
    jinja2_path_argv_index = sys.argv.index(kJinja2Path)
    # Add parent path for the import to succeed.
    path = os.path.join(sys.argv[jinja2_path_argv_index + 1], os.pardir)
    sys.path.insert(1, path)
except ValueError:
    # --jinja2-path isn't passed, ignore the exception and just import Jinja2
    # assuming it already is in the Python PATH.
    pass
kMarkupSafePath = '--markupsafe-path'
try:
    markupsafe_path_argv_index = sys.argv.index(kMarkupSafePath)
    # Add parent path for the import to succeed.
    path = os.path.join(sys.argv[markupsafe_path_argv_index + 1], os.pardir)
    sys.path.insert(1, path)
except ValueError:
    # --markupsafe-path isn't passed, ignore the exception and just import
    # assuming it already is in the Python PATH.
    pass

import jinja2


# A custom Jinja2 template loader that removes the extra indentation
# of the template blocks so that the output is correctly indented
class _PreprocessingLoader(jinja2.BaseLoader):

    def __init__(self, path, allow_list):
        self.path = path
        self.allow_list = set(allow_list)

        # Check that all the listed templates exist.
        for template in self.allow_list:
            if not os.path.exists(os.path.join(self.path, template)):
                raise jinja2.TemplateNotFound(template)

    def get_source(self, environment, template):
        if not template in self.allow_list:
            raise jinja2.TemplateNotFound(template)

        path = os.path.join(self.path, template)
        mtime = os.path.getmtime(path)
        with open(path) as f:
            source = self.preprocess(f.read())
        return source, path, lambda: mtime == os.path.getmtime(path)

    blockstart = re.compile(r'{%-?\s*(if|elif|else|for|block|macro)[^}]*%}')
    blockend = re.compile(r'{%-?\s*(end(if|for|block|macro)|elif|else)[^}]*%}')

    def preprocess(self, source):
        lines = source.split('\n')

        # Compute the current indentation level of the template blocks and
        # remove their indentation
        result = []
        indentation_level = 0

        # Filter lines that are pure comments. line_comment_prefix is not
        # enough because it removes the comment but doesn't completely remove
        # the line, resulting in more verbose output.
        lines = filter(lambda line: not line.strip().startswith('//*'), lines)

        # Remove indentation templates have for the Jinja control flow.
        for line in lines:
            # The capture in the regex adds one element per block start or end,
            # so we divide by two. There is also an extra line chunk
            # corresponding to the line end, so we subtract it.
            numends = (len(self.blockend.split(line)) - 1) // 2
            indentation_level -= numends

            result.append(self.remove_indentation(line, indentation_level))

            numstarts = (len(self.blockstart.split(line)) - 1) // 2
            indentation_level += numstarts

        return '\n'.join(result) + '\n'

    def remove_indentation(self, line, n):
        for _ in range(n):
            if line.startswith(' '):
                line = line[4:]
            elif line.startswith('\t'):
                line = line[1:]
            else:
                assert line.strip() == ''
        return line


_FileOutput = namedtuple('FileOutput', ['name', 'content'])


def _do_renders(output, template_dir):
    template_allow_list = [render.template for render in output.renders
                           ] + list(output.imported_templates)
    loader = _PreprocessingLoader(template_dir, template_allow_list)

    env = jinja2.Environment(
        extensions=['jinja2.ext.do', 'jinja2.ext.loopcontrols'],
        loader=loader,
        lstrip_blocks=True,
        trim_blocks=True,
        line_comment_prefix='//*')

    def do_assert(expr, message=''):
        assert expr, message
        return ''

    def debug(text):
        print(text)

    base_params = {
        'enumerate': enumerate,
        'format': format,
        'len': len,
        'debug': debug,
        'assert': do_assert,
    }

    outputs = []
    for render in output.renders:
        params = {}
        params.update(base_params)
        for param_dict in render.params_dicts:
            params.update(param_dict)
        content = env.get_template(render.template).render(**params)
        outputs.append(_FileOutput(render.output, content))

    return outputs


# Compute the list of imported, non-system Python modules.
# It assumes that any path outside of the root directory is system.
def _compute_python_dependencies(root_dir=None):
    if not root_dir:
        # Assume this script is under generator/ by default.
        root_dir = os.path.join(os.path.dirname(__file__), os.pardir)
    root_dir = os.path.abspath(root_dir)

    module_paths = (module.__file__ for module in sys.modules.values()
                    if module and hasattr(module, '__file__'))

    paths = set()
    for path in module_paths:
        # Builtin/namespaced modules may return None for the file path.
        if not path:
            continue

        path = os.path.abspath(path)

        if not path.startswith(root_dir):
            continue

        if (path.endswith('.pyc')
                or (path.endswith('c') and not os.path.splitext(path)[1])):
            path = path[:-1]

        paths.add(path)

    return sorted(paths)


# Computes the string representing a cmake list of paths.
def _cmake_path_list(paths):
    if os.name == "nt":
        # On Windows CMake still expects paths to be separated by forward
        # slashes
        return (";".join(paths)).replace("\\", "/")
    else:
        return ";".join(paths)


def run_generator(generator):
    parser = argparse.ArgumentParser(
        description=generator.get_description(),
        formatter_class=argparse.ArgumentDefaultsHelpFormatter,
    )

    generator.add_commandline_arguments(parser)
    parser.add_argument('--template-dir',
                        default='templates',
                        type=str,
                        help='Directory with template files.')
    parser.add_argument(
        kJinja2Path,
        default=None,
        type=str,
        help='Additional python path to set before loading Jinja2')
    parser.add_argument(
        kMarkupSafePath,
        default=None,
        type=str,
        help='Additional python path to set before loading MarkupSafe')
    parser.add_argument(
        '--output-json-tarball',
        default=None,
        type=str,
        help=('Name of the "JSON tarball" to create (tar is too annoying '
              'to use in python).'))
    parser.add_argument(
        '--depfile',
        default=None,
        type=str,
        help='Name of the Ninja depfile to create for the JSON tarball')
    parser.add_argument(
        '--expected-outputs-file',
        default=None,
        type=str,
        help="File to compare outputs with and fail if it doesn't match")
    parser.add_argument(
        '--root-dir',
        default=None,
        type=str,
        help=('Optional source root directory for Python dependency '
              'computations'))
    parser.add_argument(
        '--print-cmake-dependencies',
        default=False,
        action="store_true",
        help=("Prints a semi-colon separated list of dependencies to "
              "stdout and exits."))
    parser.add_argument(
        '--print-cmake-outputs',
        default=False,
        action="store_true",
        help=("Prints a semi-colon separated list of outputs to "
              "stdout and exits."))
    parser.add_argument('--output-dir',
                        default=None,
                        type=str,
                        help='Directory where to output generate files.')

    args = parser.parse_args()

    output = generator.get_outputs(args)

    # Output a list of all dependencies for CMake or the tarball for GN/Ninja.
    if args.depfile != None or args.print_cmake_dependencies:
        dependencies = generator.get_dependencies(args)
        dependencies += [
            args.template_dir + os.path.sep + render.template
            for render in output.renders
        ]
        dependencies += [
            args.template_dir + os.path.sep + template
            for template in output.imported_templates
        ]
        dependencies += _compute_python_dependencies(args.root_dir)

        if args.depfile != None:
            with open(args.depfile, 'w') as f:
                f.write(args.output_json_tarball + ": " +
                        " ".join(dependencies))

        if args.print_cmake_dependencies:
            sys.stdout.write(_cmake_path_list(dependencies))
            return 0

    # The caller wants to assert that the outputs are what it expects.
    # Load the file and compare with our renders.
    if args.expected_outputs_file != None:
        with open(args.expected_outputs_file) as f:
            expected = set([line.strip() for line in f.readlines()])

        actual = {render.output for render in output.renders}

        if actual != expected:
            print("Wrong expected outputs, caller expected:\n    " +
                  repr(sorted(expected)))
            print("Actual output:\n    " + repr(sorted(actual)))
            return 1

    # Print the list of all the outputs for cmake.
    if args.print_cmake_outputs:
        sys.stdout.write(
            _cmake_path_list([
                os.path.join(args.output_dir, render.output)
                for render in output.renders
            ]))
        return 0

    render_outputs = _do_renders(output, args.template_dir)

    # Output the JSON tarball
    if args.output_json_tarball != None:
        json_root = {}
        for output in render_outputs:
            json_root[output.name] = output.content

        with open(args.output_json_tarball, 'w') as f:
            f.write(json.dumps(json_root))

    # Output the files directly.
    if args.output_dir != None:
        for output in render_outputs:
            output_path = os.path.join(args.output_dir, output.name)

            directory = os.path.dirname(output_path)
            os.makedirs(directory, exist_ok=True)

            with open(output_path, 'w') as outfile:
                outfile.write(output.content)
