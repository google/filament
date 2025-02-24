# Copyright 2014 Google Inc. All rights reserved.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#    http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

import argparse
import optparse

from typ.host import Host


class _Bailout(Exception):
    pass


DEFAULT_COVERAGE_OMIT = ['*/typ/*', '*/site-packages/*']
DEFAULT_STATUS_FORMAT = '[%f/%t] '
DEFAULT_SUFFIXES = ['*_test.py', '*_unittest.py']


class ArgumentParser(argparse.ArgumentParser):

    @staticmethod
    def add_option_group(parser, title, discovery=False,
                         running=False, reporting=False, skip=None):
        # TODO: Get rid of this when telemetry upgrades to argparse.
        ap = ArgumentParser(add_help=False, version=False, discovery=discovery,
                            running=running, reporting=reporting)
        optlist = ap.optparse_options(skip=skip)
        group = optparse.OptionGroup(parser, title)
        group.add_options(optlist)
        parser.add_option_group(group)

    @staticmethod
    def add_arguments_to_parser(parser, discovery=False, running=False,
                                reporting=False, skip=None):
        # TODO(crbug.com/40807291): Remove or refactor this once Telemetry is off
        # optparse.
        ap = ArgumentParser(add_help=False, version=False, discovery=discovery,
                            running=running, reporting=reporting)
        # This logic is very similar to optparse_actions(), but not tied to
        # optparse.
        skip = skip or []
        for action in ap._actions:
            args = [flag for flag in action.option_strings if flag not in skip]
            if not args or action.help == argparse.SUPPRESS:
                # This is either a positional arg or an option we want to skip
                # anyways.
                continue
            kwargs = {
                'default': action.default,
                'dest': action.dest,
                'help': action.help,
                'metavar': action.metavar,
                'type': action.type,
                'action': _action_str(action),
            }
            # add_argument seems to complain if any kwargs with None values are
            # passed in, so remove those now.
            for k, v in list(kwargs.items()):
                if v is None:
                    del kwargs[k]
            parser.add_argument(*args, **kwargs)

    def __init__(self, host=None, add_help=True, version=True, discovery=True,
                 reporting=True, running=True):
        super(ArgumentParser, self).__init__(prog='typ', add_help=add_help)

        self._host = host or Host()
        self.exit_status = None

        self.usage = '%(prog)s [options] [tests...]'

        if version:
            self.add_argument('-V', '--version', action='store_true',
                              help='Print the typ version and exit.')

        if discovery:
            self.add_argument('-f', '--file-list', '--test-list',
                              metavar='FILENAME',
                              action='store',
                              help=('Takes the list of tests from the file '
                                    '(use "-" for stdin).'))
            self.add_argument('--all', action='store_true',
                              help=('Run all the tests, including the ones '
                                    'normally skipped.'))
            self.add_argument('--isolate', metavar='glob', default=[],
                              action='append',
                              help=('Globs of tests to run in isolation '
                                    '(serially).'))
            self.add_argument('--suffixes', metavar='glob', default=[],
                              action='append',
                              help=('Globs of test filenames to look for ('
                                    'can specify multiple times; defaults '
                                    'to %s).' % DEFAULT_SUFFIXES))

        if reporting:
            self.add_argument('--builder-name',
                              help=('Builder name to include in the '
                                    'uploaded data.'))
            self.add_argument('-c', '--coverage', action='store_true',
                              help=('Reports coverage information. This is '
                                    'disabled when a test filter is used.'))
            self.add_argument('--coverage-source', action='append',
                              default=[],
                              help=('Directories to include when running and '
                                    'reporting coverage (defaults to '
                                    '--top-level-dirs plus --path)'))
            self.add_argument('--coverage-omit', action='append',
                              default=[],
                              help=('Globs to omit when reporting coverage '
                                    '(defaults to %s).' %
                                    DEFAULT_COVERAGE_OMIT))
            self.add_argument('--coverage-annotate', action='store_true',
                              help=('Produce an annotate source report.'))
            self.add_argument('--coverage-show-missing', action='store_true',
                              help=('Show missing line ranges in coverage '
                                    'report.'))
            self.add_argument('--master-name',
                              help=('Buildbot master name to include in the '
                                    'uploaded data.'))
            self.add_argument('--metadata', action='append', default=[],
                              help=('Optional key=value metadata that will '
                                    'be included in the results.'))
            self.add_argument('--repository-absolute-path', default='', action='store',
                              help=('Specifies the absolute path of the repository.'))
            self.add_argument('--test-results-server',
                              help=('If specified, uploads the full results '
                                    'to this server.'))
            self.add_argument('--test-type',
                              help=('Name of test type to include in the '
                                    'uploaded data (e.g., '
                                    '"telemetry_unittests").'))
            self.add_argument('--write-full-results-to',
                              '--isolated-script-test-output',
                              type=str,
                              metavar='FILENAME',
                              action='store',
                              help=('If specified, writes the full results to '
                                    'that path.'))
            self.add_argument('--isolated-script-test-perf-output',
                              type=str,
                              metavar='FILENAME',
                              action='store',
                              help='(ignored/unsupported)')
            self.add_argument('--write-trace-to', metavar='FILENAME',
                              action='store',
                              help=('If specified, writes the trace to '
                                    'that path.'))
            self.add_argument('--disable-resultsink',
                              action='store_true',
                              default=False,
                              help=('Explicitly disable ResultSink integration '
                                    'instead of automatically determining '
                                    'based off LUCI_CONTEXT.'))
            self.add_argument('--rdb-content-output-file',
                              type=str,
                              metavar='FILENAME',
                              action='store',
                              help=('Write ResultSink POST content to the '
                                    'given file in jsonl instead of actually '
                                    'POSTing, where each test result is '
                                    'represented as a json string in a new line.'
                                    'This is only intended as a workaround for '
                                    'Skylab where native ResultSink '
                                    'integration is not currently possible.'))
            self.add_argument('tests', nargs='*', default=[],
                              help=argparse.SUPPRESS)

        if running:
            self.add_argument('-d', '--debugger', action='store_true',
                              help='Runs the tests under the debugger.')
            self.add_argument('-j', '--jobs', metavar='N', type=int,
                              default=self._host.cpu_count(),
                              help=('Runs N jobs in parallel '
                                    '(defaults to %(default)s).'))
            self.add_argument('--stable-jobs', action='store_true',
                              default=False,
                              help='When multiple jobs are used, round-robin '
                                   'assignment of test inputs so the job '
                                   'assignment is stable regardless of runtime.')
            self.add_argument('-l', '--list-only', action='store_true',
                              help='Lists all the test names found and exits.')
            self.add_argument('-n', '--dry-run', action='store_true',
                              help=argparse.SUPPRESS)
            self.add_argument('-q', '--quiet', action='store_true',
                              default=False,
                              help=('Runs as quietly as possible '
                                    '(only prints errors).'))
            self.add_argument('-r', '--repeat',
                              '--isolated-script-test-repeat',
                              default=1, type=int,
                              help='The number of times to repeat running each '
                                    'test. Note that if the tests are A, B, C '
                                    'and repeat is 2, the execution order would'
                                    ' be A B C [possible retries] A B C '
                                    '[possible retries].')
            self.add_argument('-s', '--status-format',
                              default=self._host.getenv('NINJA_STATUS',
                                                        DEFAULT_STATUS_FORMAT),
                              help=argparse.SUPPRESS)
            self.add_argument('-t', '--timing', action='store_true',
                              help='Prints timing info.')
            self.add_argument('-v', '--verbose', action='count', default=0,
                              help=('Prints more stuff (can specify multiple '
                                    'times for more output).'))
            self.add_argument('-x', '--tag',
                              dest='tags', default=[], action='append',
                              help=('test tags (conditions) that apply to '
                                    'this run (can specify multiple times'))
            self.add_argument('-i', '--ignore-tag',
                              dest='ignored_tags', default=[], action='append',
                              help=('test tags (conditions) to treat as '
                                    'ignored for the purposes of tag '
                                    'validation.'))
            self.add_argument('-X', '--expectations-file',
                              dest='expectations_files',
                              default=[], action='append',
                              help=('test expectations file (can specify '
                                    'multiple times'))
            self.add_argument('--passthrough', action='store_true',
                              default=False,
                              help='Prints all output while running.')
            self.add_argument('--total-shards', default=1, type=int,
                              help=('Total number of shards being used for '
                                    'this test run. (The user of '
                                    'this script is responsible for spawning '
                                    'all of the shards.)'))
            self.add_argument('--shard-index', default=0, type=int,
                              help=('Shard index (0..total_shards-1) of this '
                                    'test run.'))
            self.add_argument('--retry-limit',
                              '--isolated-script-test-launcher-retry-limit',
                              type=int, default=0,
                              help='Retries each failure up to N times.')
            self.add_argument('--retry-only-retry-on-failure-tests',
                              action='store_true',
                              help=('Retries are only for tests that have the'
                                    ' RetryOnFailure tag in the test'
                                    ' expectations file'))
            self.add_argument('--typ-max-failures',
                              type=int, default=None,
                              help=('Maximum number of failures that can occur '
                                    'before exiting the suite early.'))
            self.add_argument('--terminal-width', type=int,
                              default=self._host.terminal_width(),
                              help=argparse.SUPPRESS)
            self.add_argument('--overwrite', action='store_true',
                              default=None,
                              help=argparse.SUPPRESS)
            self.add_argument('--no-overwrite', action='store_false',
                              dest='overwrite', default=None,
                              help=argparse.SUPPRESS)
            self.add_argument('--test-name-prefix', default='', action='store',
                              help=('Specifies the prefix that will be removed'
                                    ' from test names'))
            self.add_argument('--isolated-outdir',
                              type=str,
                              metavar='PATH',
                              help='directory to write output to (ignored)')
            self.add_argument('--use-global-pool', action='store_true',
                              default=False,
                              help=('Use the older, single/global process pool '
                                    'approach instead of the scoped approach.'))

        if discovery or running:
            self.add_argument('-P', '--path', action='append', default=[],
                              help=('Adds dir to sys.path (can specify '
                                    'multiple times).'))
            self.add_argument('--top-level-dir', action='store', default=None,
                              help=argparse.SUPPRESS)
            self.add_argument('--top-level-dirs', action='append', default=[],
                              help=('Sets the top directory of project '
                                    '(used when running subdirs).'))
            self.add_argument('--skip', metavar='glob', default=[],
                              action='append',
                              help=('Globs of test names to skip ('
                                    'defaults to %(default)s).'))
            self.add_argument(
                '--test-filter',
                '--isolated-script-test-filter',
                type=str, default='', action='store',
                help='Pass a double-colon-separated ("::") list of exact test '
                'names or globs, to run just that subset of tests. fnmatch will '
                'be used to match globs to test names. Utilizing a filter will '
                'disable coverage.')
            self.add_argument(
                '--partial-match-filter', type=str, default=[], action='append',
                help='Pass a string and Typ will run tests whose names '
                     'partially match the passed string')


    def parse_args(self, args=None, namespace=None):
        try:
            rargs = super(ArgumentParser, self).parse_args(args=args,
                                                           namespace=namespace)
        except _Bailout:
            return None

        for val in rargs.metadata:
            if '=' not in val:
                self._print_message('Error: malformed --metadata "%s"' % val)
                self.exit_status = 2

        if rargs.test_results_server:
            if not rargs.builder_name:
                self._print_message('Error: --builder-name must be specified '
                                    'along with --test-result-server')
                self.exit_status = 2
            if not rargs.master_name:
                self._print_message('Error: --master-name must be specified '
                                    'along with --test-result-server')
                self.exit_status = 2
            if not rargs.test_type:
                self._print_message('Error: --test-type must be specified '
                                    'along with --test-result-server')
                self.exit_status = 2

        if rargs.total_shards < 1:
            self._print_message('Error: --total-shards must be at least 1')
            self.exit_status = 2

        if rargs.shard_index < 0:
            self._print_message('Error: --shard-index must be at least 0')
            self.exit_status = 2

        if rargs.shard_index >= rargs.total_shards:
            self._print_message('Error: --shard-index must be no more than '
                                'the number of shards (%i) minus 1' %
                                rargs.total_shards)
            self.exit_status = 2

        if not rargs.suffixes:
            rargs.suffixes = DEFAULT_SUFFIXES

        if not rargs.coverage_omit:
            rargs.coverage_omit = DEFAULT_COVERAGE_OMIT

        if rargs.debugger:  # pragma: no cover
            rargs.jobs = 1
            rargs.passthrough = True

        if rargs.overwrite is None:
            rargs.overwrite = self._host.stdout.isatty() and not rargs.verbose

        if (rargs.test_filter and rargs.coverage):
            # Running a subset of tests will fail coverage check, so explicitly
            # disable coverage when a filter is passed.
            rargs.coverage = False

        return rargs

    # Redefining built-in 'file' pylint: disable=W0622

    def _print_message(self, msg, file=None):
        self._host.print_(msg=msg, stream=file, end='\n')

    def print_help(self, file=None):
        self._print_message(msg=self.format_help(), file=file)

    def error(self, message, bailout=True):  # pylint: disable=W0221
        self.exit(2, '%s: error: %s\n' % (self.prog, message), bailout=bailout)

    def exit(self, status=0, message=None,  # pylint: disable=W0221
             bailout=True):
        self.exit_status = status
        if message:
            self._print_message(message, file=self._host.stderr)
        if bailout:
            raise _Bailout()

    def optparse_options(self, skip=None):
        skip = skip or []
        options = []
        for action in self._actions:
            args = [flag for flag in action.option_strings if flag not in skip]
            if not args or action.help == '==SUPPRESS==':
                # must either be a positional argument like 'tests'
                # or an option we want to skip altogether.
                continue

            kwargs = {
                'default': action.default,
                'dest': action.dest,
                'help': action.help,
                'metavar': action.metavar,
                'type': action.type,
                'action': _action_str(action)
            }
            options.append(optparse.make_option(*args, **kwargs))
        return options

    def argv_from_args(self, args):
        default_parser = ArgumentParser(host=self._host)
        default_args = default_parser.parse_args([])
        argv = []
        tests = []
        d = vars(args)
        for k in sorted(d.keys()):
            v = d[k]
            argname = _argname_from_key(k)
            action = self._action_for_key(k)
            if not action:
                continue
            action_str = _action_str(action)
            if k == 'tests':
                tests = v
                continue
            if getattr(default_args, k) == v:
                # this arg has the default value, so skip it.
                continue

            assert action_str in ['append', 'count', 'store', 'store_true']
            if action_str == 'append':
                for el in v:
                    argv.append(argname)
                    argv.append(el)
            elif action_str == 'count':
                for _ in range(v):
                    argv.append(argname)
            elif action_str == 'store':
                argv.append(argname)
                argv.append(str(v))
            else:
                # action_str == 'store_true'
                argv.append(argname)

        return argv + tests

    def _action_for_key(self, key):
        for action in self._actions:
            if action.dest == key:
                return action

        # Assume foreign argument: something used by the embedder of typ, for
        # example.
        return None


def _action_str(action):
    # Access to a protected member pylint: disable=W0212
    assert action.__class__ in (
        argparse._AppendAction,
        argparse._CountAction,
        argparse._StoreAction,
        argparse._StoreTrueAction
    )

    if isinstance(action, argparse._AppendAction):
        return 'append'
    if isinstance(action, argparse._CountAction):
        return 'count'
    if isinstance(action, argparse._StoreAction):
        return 'store'
    if isinstance(action, argparse._StoreTrueAction):
        return 'store_true'


def _argname_from_key(key):
    return '--' + key.replace('_', '-')
