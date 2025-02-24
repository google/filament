#!/usr/bin/env vpython3
# Copyright (c) 2013 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
"""
Tool to perform checkouts in one easy command line!

Usage:
    fetch <config> [--property=value [--property2=value2 ...]]

This script is a wrapper around various version control and repository
checkout commands. It requires a |config| name, fetches data from that
config in depot_tools/fetch_configs, and then performs all necessary inits,
checkouts, pulls, fetches, etc.

Optional arguments may be passed on the command line in key-value pairs.
These parameters will be passed through to the config's main method.
"""

import json
import argparse
import os
import shlex
import subprocess
import sys

import gclient_utils
import git_common

from distutils import spawn

SCRIPT_PATH = os.path.dirname(os.path.abspath(__file__))


#################################################
# Checkout class definitions.
#################################################
class Checkout(object):
    """Base class for implementing different types of checkouts.

    Attributes:
        |base|: the absolute path of the directory in which this script is run.
        |spec|: the spec for this checkout as returned by the config. Different
            subclasses will expect different keys in this dictionary.
        |root|: the directory into which the checkout will be performed, as
            returnedby the config. This is a relative path from |base|.
    """
    def __init__(self, options, spec, root):
        self.base = os.getcwd()
        self.options = options
        self.spec = spec
        self.root = root

    def exists(self):
        """Check does this checkout already exist on desired location."""

    def init(self):
        pass

    def run(self, cmd, return_stdout=False, **kwargs):
        print('Running: %s' % (' '.join(shlex.quote(x) for x in cmd)))
        if self.options.dry_run:
            return ''
        if return_stdout:
            return subprocess.check_output(cmd, **kwargs).decode()

        try:
            subprocess.check_call(cmd, **kwargs)
        except subprocess.CalledProcessError as e:
            # If the subprocess failed, it likely emitted its own distress
            # message already - don't scroll that message off the screen with a
            # stack trace from this program as well. Emit a terse message and
            # bail out here; otherwise a later step will try doing more work and
            # may hide the subprocess message.
            print('Subprocess failed with return code %d.' % e.returncode)
            sys.exit(e.returncode)
        return ''


class GclientCheckout(Checkout):
    def run_gclient(self, *cmd, **kwargs):
        if not spawn.find_executable('gclient'):
            cmd_prefix = (sys.executable, os.path.join(SCRIPT_PATH,
                                                       'gclient.py'))
        else:
            cmd_prefix = ('gclient', )
        return self.run(cmd_prefix + cmd, **kwargs)

    def exists(self):
        try:
            gclient_root = self.run_gclient('root', return_stdout=True).strip()
            return (os.path.exists(os.path.join(gclient_root, '.gclient'))
                    or os.path.exists(
                        os.path.join(os.getcwd(), self.root, '.git')))
        except subprocess.CalledProcessError:
            pass
        return os.path.exists(os.path.join(os.getcwd(), self.root))


class GitCheckout(Checkout):
    def run_git(self, *cmd, **kwargs):
        print('Running: git %s' % (' '.join(shlex.quote(x) for x in cmd)))
        if self.options.dry_run:
            return ''
        return git_common.run(*cmd, **kwargs)


class GclientGitCheckout(GclientCheckout, GitCheckout):
    def __init__(self, options, spec, root):
        super(GclientGitCheckout, self).__init__(options, spec, root)
        assert 'solutions' in self.spec

    def _format_spec(self):
        def _format_literal(lit):
            if isinstance(lit, str):
                return '"%s"' % lit
            if isinstance(lit, list):
                return '[%s]' % ', '.join(_format_literal(i) for i in lit)
            return '%r' % lit

        soln_strings = []
        for soln in self.spec['solutions']:
            soln_string = '\n'.join('    "%s": %s,' %
                                    (key, _format_literal(value))
                                    for key, value in soln.items())
            soln_strings.append('  {\n%s\n  },' % soln_string)
        gclient_spec = 'solutions = [\n%s\n]\n' % '\n'.join(soln_strings)
        extra_keys = ['target_os', 'target_os_only', 'cache_dir']
        gclient_spec += ''.join('%s = %s\n' %
                                (key, _format_literal(self.spec[key]))
                                for key in extra_keys if key in self.spec)
        return gclient_spec

    def init(self):
        # Configure and do the gclient checkout.
        self.run_gclient('config', '--spec', self._format_spec())
        sync_cmd = ['sync']
        if self.options.nohooks:
            sync_cmd.append('--nohooks')
        if self.options.nohistory:
            sync_cmd.append('--no-history')
        if self.spec.get('with_branch_heads', False):
            sync_cmd.append('--with_branch_heads')
        self.run_gclient(*sync_cmd)

        # Configure git.
        wd = os.path.join(self.base, self.root)
        if self.options.dry_run:
            print('cd %s' % wd)
        if not self.options.nohistory:
            self.run_git('config',
                         '--add',
                         'remote.origin.fetch',
                         '+refs/tags/*:refs/tags/*',
                         cwd=wd)
        self.run_git('config', 'diff.ignoreSubmodules', 'dirty', cwd=wd)


CHECKOUT_TYPE_MAP = {
    'gclient': GclientCheckout,
    'gclient_git': GclientGitCheckout,
    'git': GitCheckout,
}


def CheckoutFactory(type_name, options, spec, root):
    """Factory to build Checkout class instances."""
    class_ = CHECKOUT_TYPE_MAP.get(type_name)
    if not class_:
        raise KeyError('unrecognized checkout type: %s' % type_name)
    return class_(options, spec, root)


def handle_args(argv):
    """Gets the config name from the command line arguments."""

    configs_dir = os.path.join(SCRIPT_PATH, 'fetch_configs')
    configs = [f[:-3] for f in os.listdir(configs_dir) if f.endswith('.py')]
    configs.sort()

    parser = argparse.ArgumentParser(
      formatter_class=argparse.RawDescriptionHelpFormatter,
      description='''
    This script can be used to download the Chromium sources. See
    http://www.chromium.org/developers/how-tos/get-the-code
    for full usage instructions.''',
      epilog='Valid fetch configs:\n' + \
        '\n'.join(map(lambda s: '  ' + s, configs))
      )

    parser.add_argument('-n',
                        '--dry-run',
                        action='store_true',
                        default=False,
                        help='Don\'t run commands, only print them.')
    parser.add_argument('--nohooks',
                        '--no-hooks',
                        action='store_true',
                        default=False,
                        help='Don\'t run hooks after checkout.')
    parser.add_argument(
        '--nohistory',
        '--no-history',
        action='store_true',
        default=False,
        help='Perform shallow clones, don\'t fetch the full git history.')
    parser.add_argument(
        '--force',
        action='store_true',
        default=False,
        help='(dangerous) Don\'t look for existing .gclient file.')
    parser.add_argument(
        '-p',
        '--protocol-override',
        type=str,
        default=None,
        help='Protocol to use to fetch dependencies, defaults to https.')

    parser.add_argument('config',
                        type=str,
                        help="Project to fetch, e.g. chromium.")
    parser.add_argument('props',
                        metavar='props',
                        type=str,
                        nargs=argparse.REMAINDER,
                        default=[])

    args = parser.parse_args(argv[1:])

    # props passed to config must be of the format --<name>=<value>
    looks_like_arg = lambda arg: arg.startswith('--') and arg.count('=') == 1
    bad_param = [x for x in args.props if not looks_like_arg(x)]
    if bad_param:
        print('Error: Got bad arguments %s' % bad_param)
        parser.print_help()
        sys.exit(1)

    return args


def run_config_fetch(config, props, aliased=False):
    """Invoke a config's fetch method with the passed-through args
    and return its json output as a python object."""
    config_path = os.path.abspath(
        os.path.join(SCRIPT_PATH, 'fetch_configs', config))
    if not os.path.exists(config_path + '.py'):
        print("Could not find a config for %s" % config)
        sys.exit(1)

    cmd = [sys.executable, config_path + '.py', 'fetch'] + props
    result = subprocess.Popen(cmd, stdout=subprocess.PIPE).communicate()[0]

    spec = json.loads(result.decode("utf-8"))
    if 'alias' in spec:
        assert not aliased
        return run_config_fetch(spec['alias']['config'],
                                spec['alias']['props'] + props,
                                aliased=True)
    cmd = [sys.executable, config_path + '.py', 'root']
    result = subprocess.Popen(cmd, stdout=subprocess.PIPE).communicate()[0]
    root = json.loads(result.decode("utf-8"))
    return spec, root


def run(options, spec, root):
    """Perform a checkout with the given type and configuration.

        Args:
        options: Options instance.
        spec: Checkout configuration returned by the the config's fetch_spec
            method (checkout type, repository url, etc.).
        root: The directory into which the repo expects to be checkout out.
    """
    if gclient_utils.IsEnvCog():
        print(
            'Your current directory appears to be in a Cog workspace.'
            '"fetch" command is not supported in this environment.',
            file=sys.stderr)
        return 1
    assert 'type' in spec
    checkout_type = spec['type']
    checkout_spec = spec['%s_spec' % checkout_type]

    # Update solutions with protocol_override field
    if options.protocol_override is not None:
        for solution in checkout_spec['solutions']:
            solution['protocol_override'] = options.protocol_override

    try:
        checkout = CheckoutFactory(checkout_type, options, checkout_spec, root)
    except KeyError:
        return 1
    if not options.force and checkout.exists():
        print(
            'Your current directory appears to already contain, or be part of, '
        )
        print('a checkout. "fetch" is used only to get new checkouts. Use ')
        print('"gclient sync" to update existing checkouts.')
        print()
        print(
            'Fetch also does not yet deal with partial checkouts, so if fetch')
        print('failed, delete the checkout and start over (crbug.com/230691).')
        return 1
    return checkout.init()


def main():
    args = handle_args(sys.argv)
    spec, root = run_config_fetch(args.config, args.props)
    return run(args, spec, root)


if __name__ == '__main__':
    try:
        sys.exit(main())
    except KeyboardInterrupt:
        sys.stderr.write('interrupted\n')
        sys.exit(1)
