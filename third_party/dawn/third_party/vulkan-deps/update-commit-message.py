#!/usr/bin/env python3
# Copyright 2021 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
# based on an almost identical script by: jyrki@google.com (Jyrki Alakuijala)
"""Updates the commit message used in the auto-roller.

Merges several small commit logs into a single more useful commit message.

Usage:
  update_commit_message.py --old-revision=<sha1>
"""

import argparse
import logging
import os
import platform
import re
import shutil
import subprocess
import sys
import tempfile

GCLIENT_LINE = r'([^:]+): ([^@]+)@(.*)'
CHANGE_TEMPLATE = '* %s: %s.git/+log/%s..%s'
EXIT_SUCCESS = 0
EXIT_FAILURE = 1
SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))
GCLIENT = """\
solutions = [{
    'name': '.',
    'url': 'https://chromium.googlesource.com/vulkan-deps.git',
    'deps_file': 'DEPS',
    'managed': False,
}]
"""
INSERT_NEEDLE = 'If this roll has caused a breakage'


def run(cmd, args):
    exe = ('%s.bat' % cmd) if platform.system() == 'Windows' else cmd
    cmd_args = [exe] + list(args)
    return subprocess.check_output(cmd_args).decode('ascii').strip()


def git(*args):
    return run('git', args)


def gclient(*args):
    return run('gclient', args)


def parse_revinfo(output):
    expr = re.compile(GCLIENT_LINE)
    config = dict()
    urls = dict()
    for line in output.split('\n'):
        match = expr.match(line.strip())
        if match:
            dep = match.group(1)
            urls[dep] = match.group(2)
            config[dep] = match.group(3)
    return config, urls


def _local_commit_amend(commit_msg, dry_run):
    logging.info('Amending changes to local commit.')
    old_commit_msg = git('log', '-1', '--pretty=%B')
    logging.debug('Existing commit message:\n%s\n', old_commit_msg)
    insert_index = old_commit_msg.rfind(INSERT_NEEDLE)
    if insert_index == -1:
        logging.exception('"%s" not found in commit message.' % INSERT_NEEDLE)

    new_commit_msg = old_commit_msg[:insert_index] + commit_msg + '\n\n' + old_commit_msg[insert_index:]
    logging.debug('New commit message:\n%s\n', new_commit_msg)
    if not dry_run:
        with tempfile.NamedTemporaryFile(delete=False, mode="w") as ntf:
            ntf.write(new_commit_msg)
            ntf.close()
            git('commit', '--amend', '--no-edit', '--file=%s' % ntf.name)
            os.unlink(ntf.name)


def main(raw_args):
    parser = argparse.ArgumentParser()
    parser.add_argument('--old-revision', help='Old git revision in the roll.', required=True)
    parser.add_argument(
        '--dry-run',
        help='Test out functionality without making changes.',
        action='store_true',
        default=False)
    parser.add_argument(
        '-v', '--verbose', help='Verbose debug logging.', action='store_true', default=False)
    args = parser.parse_args(raw_args)

    if args.verbose:
        logging.basicConfig(level=logging.DEBUG)
    else:
        logging.basicConfig(level=logging.INFO)

    cwd = os.getcwd()

    os.chdir(SCRIPT_DIR)
    old_deps_content = git('show', '%s:DEPS' % args.old_revision)

    with tempfile.TemporaryDirectory() as tempdir:
        os.chdir(tempdir)

        # Add the gclientfile.
        with open(os.path.join(tempdir, '.gclient'), 'w') as gcfile:
            gcfile.write(GCLIENT)
            gcfile.close()

        # Get the current config.
        shutil.copyfile(os.path.join(SCRIPT_DIR, 'DEPS'), os.path.join(tempdir, 'DEPS'))
        gclient_head_output = gclient('revinfo')

        # Get the prior config.
        with open('DEPS', 'w') as deps:
            deps.write(old_deps_content)
            deps.close()
        gclient_old_output = gclient('revinfo')
        os.chdir(SCRIPT_DIR)

    head_config, urls = parse_revinfo(gclient_head_output)
    old_config, _ = parse_revinfo(gclient_old_output)

    changed_deps = []

    for dep, new_sha1 in head_config.items():
        if dep in old_config:
            old_sha1 = old_config[dep]
            if new_sha1 != old_sha1:
                dep_short = dep.replace('\\', '/').split('/')[0]
                repo = urls[dep]
                logging.debug('Found change: %s to %s' % (dep, new_sha1))
                changed_deps.append(CHANGE_TEMPLATE %
                                    (dep_short, repo, old_sha1[:10], new_sha1[:10]))

    if not changed_deps:
        print('No changed dependencies, early exit.')
        return EXIT_SUCCESS

    commit_msg = 'Changed dependencies:\n%s' % '\n'.join(sorted(changed_deps))

    os.chdir(cwd)
    _local_commit_amend(commit_msg, args.dry_run)

    return EXIT_SUCCESS


if __name__ == '__main__':
    sys.exit(main(sys.argv[1:]))
