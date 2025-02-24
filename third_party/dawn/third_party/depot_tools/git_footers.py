#!/usr/bin/env python3
# Copyright 2014 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import argparse
import json
import re
import sys

from collections import defaultdict

import gclient_utils
import git_common as git

FOOTER_PATTERN = re.compile(r'^\s*([\w-]+): *(.*)$')
CHROME_COMMIT_POSITION_PATTERN = re.compile(r'^([\w/\-\.]+)@{#(\d+)}$')
FOOTER_KEY_BLOCKLIST = set(['http', 'https'])


def normalize_name(header):
    return '-'.join([word.title() for word in header.strip().split('-')])


def parse_footer(line):
    """Returns footer's (key, value) if footer is valid, else None."""
    match = FOOTER_PATTERN.match(line)
    if match and match.group(1) not in FOOTER_KEY_BLOCKLIST:
        return (match.group(1), match.group(2))
    return None


def parse_footers(message):
    """Parses a git commit message into a multimap of footers."""
    _, _, parsed_footers = split_footers(message)
    footer_map = defaultdict(list)
    if parsed_footers:
        # Read footers from bottom to top, because latter takes precedense,
        # and we want it to be first in the multimap value.
        for (k, v) in reversed(parsed_footers):
            footer_map[normalize_name(k)].append(v.strip())
    return footer_map


def normalize_key(line):
    """Returns the key of the footer line in normalized form."""
    r = parse_footer(line)
    if r is None:
        return ''
    return normalize_name(r[0])


def split_footers(message):
    """Returns (non_footer_lines, footer_lines, parsed footers).

    Guarantees that:
        (non_footer_lines + footer_lines) ~= message.splitlines(), with at
            most one new newline, if the last paragraph is text followed by
            footers.
        parsed_footers is parse_footer applied on each line of footer_lines.
            There could be fewer parsed_footers than footer lines if some lines
            in last paragraph are malformed.
    """
    message_lines = list(message.rstrip().splitlines())
    footer_lines = []
    maybe_footer_lines = []
    for line in reversed(message_lines):
        if line == '' or line.isspace():
            break

        if parse_footer(line):
            footer_lines.extend(maybe_footer_lines)
            maybe_footer_lines = []
            footer_lines.append(line)
        else:
            # We only want to include malformed lines if they are preceded by
            # well-formed lines. So keep them in holding until we see a
            # well-formed line (case above).
            maybe_footer_lines.append(line)
    else:
        # The whole description was consisting of footers,
        # which means those aren't footers.
        footer_lines = []

    footer_lines.reverse()
    footers = [footer for footer in map(parse_footer, footer_lines) if footer]
    if not footers:
        return message_lines, [], []
    if maybe_footer_lines:
        # If some malformed lines were left over, add a newline to split them
        # from the well-formed ones.
        return message_lines[:-len(footer_lines)] + [''], footer_lines, footers
    return message_lines[:-len(footer_lines)], footer_lines, footers


def get_footer_change_id(message):
    """Returns a list of Gerrit's ChangeId from given commit message."""
    return parse_footers(message).get(normalize_name('Change-Id'), [])


def add_footer_change_id(message, change_id):
    """Returns message with Change-ID footer in it.

    Assumes that Change-Id is not yet in footers, which is then inserted at
    earliest footer line which is after all of these footers:
        Bug|Issue|Test|Feature.
    """
    assert 'Change-Id' not in parse_footers(message)
    return add_footer(message,
                      'Change-Id',
                      change_id,
                      after_keys=['Bug', 'Issue', 'Test', 'Feature'])


def add_footer(message, key, value, after_keys=None):
    """Returns a message with given footer appended.

    If after_keys is None (default), appends footer last.
    If after_keys is provided and matches footers already present, inserts
    footer as *early* as possible while still appearing after all provided
    keys.

    For example, given
        message='Header.\n\nAdded: 2016\nBug: 123\nVerified-By: CQ'
        after_keys=['Bug', 'Issue']
    the new footer will be inserted between Bug and Verified-By existing
    footers.
    """
    assert key == normalize_name(key), 'Use normalized key'
    new_footer = '%s: %s' % (key, value)
    if not FOOTER_PATTERN.match(new_footer):
        raise ValueError('Invalid footer %r' % new_footer)

    top_lines, footer_lines, _ = split_footers(message)
    if not footer_lines:
        if not top_lines or top_lines[-1] != '':
            top_lines.append('')
        footer_lines = [new_footer]
    else:
        # find the index of the last footer with any of the after_keys.
        after_keys = set(map(normalize_name, after_keys or []))
        last_akey_idx = None
        for idx in range(len(footer_lines)):
            line = footer_lines[idx]

            # if the current footer is indented, it may be a continuation of
            # the previous line.
            if line and line[0] == ' ':
                if last_akey_idx is None:
                    continue
                if idx - last_akey_idx > 1:
                    continue
            elif normalize_key(line) not in after_keys:
                continue
            last_akey_idx = idx

        if last_akey_idx is None:
            insert_idx = len(footer_lines)
        else:
            insert_idx = last_akey_idx + 1
        footer_lines.insert(insert_idx, new_footer)
    return '\n'.join(top_lines + footer_lines)


def remove_footer(message, key):
    """Returns a message with all instances of given footer removed."""
    key = normalize_name(key)
    top_lines, footer_lines, _ = split_footers(message)
    if not footer_lines:
        return message
    new_footer_lines = []
    for line in footer_lines:
        try:
            f = normalize_name(parse_footer(line)[0])
            if f != key:
                new_footer_lines.append(line)
        except TypeError:
            # If the footer doesn't parse (i.e. is malformed), just let it carry
            # over.
            new_footer_lines.append(line)
    return '\n'.join(top_lines + new_footer_lines)


def get_unique(footers, key):
    key = normalize_name(key)
    values = footers[key]
    assert len(values) <= 1, 'Multiple %s footers' % key
    if values:
        return values[0]

    return None


def get_position(footers):
    """Get the commit position from the footers multimap using a heuristic.

    Returns:
        A tuple of the branch and the position on that branch. For example,

        Cr-Commit-Position: refs/heads/main@{#292272}

        would give the return value ('refs/heads/main', 292272).
    """

    position = get_unique(footers, 'Cr-Commit-Position')
    if position:
        match = CHROME_COMMIT_POSITION_PATTERN.match(position)
        assert match, 'Invalid Cr-Commit-Position value: %s' % position
        return (match.group(1), match.group(2))

    raise ValueError('Unable to infer commit position from footers')


def main(args):
    if gclient_utils.IsEnvCog():
        print('footers command is not supported in non-git environment',
              file=sys.stderr)
        return 1
    parser = argparse.ArgumentParser(
        formatter_class=argparse.ArgumentDefaultsHelpFormatter)
    parser.add_argument('ref',
                        nargs='?',
                        help='Git ref to retrieve footers from.'
                        ' Omit to parse stdin.')

    g = parser.add_mutually_exclusive_group()
    g.add_argument('--key',
                   metavar='KEY',
                   help='Get all values for the given footer name, one per '
                   'line (case insensitive)')
    g.add_argument('--position', action='store_true')
    g.add_argument('--position-ref', action='store_true')
    g.add_argument('--position-num', action='store_true')
    g.add_argument('--json',
                   help='filename to dump JSON serialized footers to.')

    opts = parser.parse_args(args)

    if opts.ref:
        message = git.run('log', '-1', '--format=%B', opts.ref)
    else:
        message = sys.stdin.read()

    footers = parse_footers(message)

    if opts.key:
        for v in footers.get(normalize_name(opts.key), []):
            print(v)
    elif opts.position:
        pos = get_position(footers)
        print('%s@{#%s}' % (pos[0], pos[1] or '?'))
    elif opts.position_ref:
        print(get_position(footers)[0])
    elif opts.position_num:
        pos = get_position(footers)
        assert pos[1], 'No valid position for commit'
        print(pos[1])
    elif opts.json:
        with open(opts.json, 'w') as f:
            json.dump(footers, f)
    else:
        for k in footers.keys():
            for v in footers[k]:
                print('%s: %s' % (k, v))
    return 0


if __name__ == '__main__':
    try:
        sys.exit(main(sys.argv[1:]))
    except KeyboardInterrupt:
        sys.stderr.write('interrupted\n')
        sys.exit(1)
