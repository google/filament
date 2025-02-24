#!/usr/bin/env python

import datetime
import re
import subprocess

RELEASE = re.compile(r'[0-9]+\.[0-9]+\.[0-9]+')
ISSUE = re.compile(r'#([0-9]+)')
REVLIST = 'git rev-list develop --abbrev-commit --format="parents %p%n%B%n~~~" --max-count=200 develop'
TEMPLATE = """
boto v{version}
===========

:date: {date}

Description goes here.


Changes
-------
{changes}
"""

revisions = subprocess.check_output(REVLIST, shell=True, stderr=subprocess.STDOUT)

commit_list = []
for hunk in revisions.split('~~~')[:-1]:
    lines = hunk.strip().splitlines()
    commit = lines[0].split(' ', 1)[1]
    parents = lines[1].split(' ', 1)[1].split(' ')
    message = ' '.join(lines[2:])

    # print(commit, parents)

    if RELEASE.search(message):
        print('Found release commit, stopping:')
        print(message)
        break

    if len(parents) > 1:
        commit_list.append([commit, message])

removals = [
    re.compile(r'merge pull request #[0-9]+ from [a-z0-9/_-]+', re.I),
    re.compile(r"merge branch '[a-z0-9/_-]+' into [a-z0-9/_-]+", re.I),
    re.compile(r'fix(es)? [#0-9, ]+.?', re.I)
]

changes = ''
for commit, message in commit_list:
    append = []
    issues = set()
    for issue in ISSUE.findall(message):
        if issue not in issues:
            append.append(':issue:`{issue}`'.format(issue=issue))
            issues.add(issue)
    append.append(':sha:`{commit}`'.format(commit=commit))
    append = ' (' + ', '.join(append) + ')'

    original = message
    for removal in removals:
        message = removal.sub('', message)

    message = message.strip()

    if not message:
        message = original.strip()

    changes += '* ' + message + append + '\n'

print(TEMPLATE.format(
    version='?.?.?',
    date=datetime.datetime.now().strftime('%Y/%m/%d'),
    changes=changes
))
