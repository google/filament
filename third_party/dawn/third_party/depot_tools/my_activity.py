#!/usr/bin/env vpython3
# Copyright (c) 2012 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
"""Get stats about your activity.

Example:
  - my_activity.py  for stats for the current week (last week on mondays).
  - my_activity.py -Q  for stats for last quarter.
  - my_activity.py -Y  for stats for this year.
  - my_activity.py -b 4/24/19  for stats since April 24th 2019.
  - my_activity.py -b 4/24/19 -e 6/16/19 stats between April 24th and June 16th.
  - my_activity.py -jd to output stats for the week to json with deltas data.

To add additional gerrit instances, one can pass a JSON file as parameter:
  - my_activity.py -F config.json
{
  "gerrit_instances": {
    "team-internal-review.googlesource.com": {
      "shorturl": "go/teamcl",
      "short_url_protocol": "http"
    },
    "team-external-review.googlesource.com": {}
  }
}
"""

# These services typically only provide a created time and a last modified time
# for each item for general queries. This is not enough to determine if there
# was activity in a given time period. So, we first query for all things created
# before end and modified after begin. Then, we get the details of each item and
# check those details to determine if there was activity in the given period.
# This means that query time scales mostly with (today() - begin).

import collections
import contextlib
from datetime import datetime
from datetime import timedelta
import httplib2
import itertools
import json
import logging
from multiprocessing.pool import ThreadPool
import optparse
import os
from string import Formatter
import sys
import urllib
import re

import auth
import gclient_utils
import gerrit_util

try:
    import dateutil  # pylint: disable=import-error
    import dateutil.parser
    from dateutil.relativedelta import relativedelta
except ImportError:
    logging.error('python-dateutil package required')
    sys.exit(1)


class DefaultFormatter(Formatter):
    def __init__(self, default=''):
        super(DefaultFormatter, self).__init__()
        self.default = default

    def get_value(self, key, args, kwargs):
        if isinstance(key, str) and key not in kwargs:
            return self.default
        return Formatter.get_value(self, key, args, kwargs)


gerrit_instances = [
    {
        'url': 'android-review.googlesource.com',
        'shorturl': 'r.android.com',
        'short_url_protocol': 'https',
    },
    {
        'url': 'gerrit-review.googlesource.com',
    },
    {
        'url': 'chrome-internal-review.googlesource.com',
        'shorturl': 'crrev.com/i',
        'short_url_protocol': 'https',
    },
    {
        'url': 'chromium-review.googlesource.com',
        'shorturl': 'crrev.com/c',
        'short_url_protocol': 'https',
    },
    {
        'url': 'dawn-review.googlesource.com',
    },
    {
        'url': 'pdfium-review.googlesource.com',
    },
    {
        'url': 'skia-review.googlesource.com',
    },
    {
        'url': 'review.coreboot.org',
    },
]

monorail_projects = {
    'angleproject': {
        'shorturl': 'anglebug.com',
        'short_url_protocol': 'http',
    },
    'chromium': {
        'shorturl': 'crbug.com',
        'short_url_protocol': 'https',
    },
    'dawn': {},
    'google-breakpad': {},
    'gyp': {},
    'pdfium': {
        'shorturl': 'crbug.com/pdfium',
        'short_url_protocol': 'https',
    },
    'skia': {},
    'tint': {},
    'v8': {
        'shorturl': 'crbug.com/v8',
        'short_url_protocol': 'https',
    },
}


def username(email):
    """Keeps the username of an email address."""
    return email and email.split('@', 1)[0]


def datetime_to_midnight(date):
    return date - timedelta(hours=date.hour,
                            minutes=date.minute,
                            seconds=date.second,
                            microseconds=date.microsecond)


def get_quarter_of(date):
    begin = (datetime_to_midnight(date) -
             relativedelta(months=(date.month - 1) % 3, days=(date.day - 1)))
    return begin, begin + relativedelta(months=3)


def get_year_of(date):
    begin = (datetime_to_midnight(date) -
             relativedelta(months=(date.month - 1), days=(date.day - 1)))
    return begin, begin + relativedelta(years=1)


def get_week_of(date):
    begin = (datetime_to_midnight(date) - timedelta(days=date.weekday()))
    return begin, begin + timedelta(days=7)


def get_yes_or_no(msg):
    while True:
        response = gclient_utils.AskForData(msg + ' yes/no [no] ')
        if response in ('y', 'yes'):
            return True

        if not response or response in ('n', 'no'):
            return False


def datetime_from_gerrit(date_string):
    return datetime.strptime(date_string, '%Y-%m-%d %H:%M:%S.%f000')


def datetime_from_monorail(date_string):
    return datetime.strptime(date_string, '%Y-%m-%dT%H:%M:%S')


def extract_bug_numbers_from_description(issue):
    # Getting the description for REST Gerrit
    revision = issue['revisions'][issue['current_revision']]
    description = revision['commit']['message']

    bugs = []
    # Handle both "Bug: 99999" and "BUG=99999" bug notations
    # Multiple bugs can be noted on a single line or in multiple ones.
    matches = re.findall(
        r'^(BUG=|(Bug|Fixed):\s*)((((?:[a-zA-Z0-9-]+:)?\d+)(,\s?)?)+)',
        description,
        flags=re.IGNORECASE | re.MULTILINE)
    if matches:
        for match in matches:
            bugs.extend(match[2].replace(' ', '').split(','))
        # Add default chromium: prefix if none specified.
        bugs = [bug if ':' in bug else 'chromium:%s' % bug for bug in bugs]

    return sorted(set(bugs))


class MyActivity(object):
    def __init__(self, options):
        self.options = options
        self.modified_after = options.begin
        self.modified_before = options.end
        self.user = options.user
        self.changes = []
        self.reviews = []
        self.issues = []
        self.referenced_issues = []
        self.google_code_auth_token = None
        self.access_errors = set()
        self.skip_servers = (options.skip_servers.split(','))

    def show_progress(self, how='.'):
        if sys.stdout.isatty():
            sys.stdout.write(how)
            sys.stdout.flush()

    def gerrit_changes_over_rest(self, instance, filters):
        # Convert the "key:value" filter to a list of (key, value) pairs.
        req = list(f.split(':', 1) for f in filters)
        try:
            # Instantiate the generator to force all the requests now and catch
            # the errors here.
            return list(
                gerrit_util.GenerateAllChanges(instance['url'],
                                               req,
                                               o_params=[
                                                   'MESSAGES', 'LABELS',
                                                   'DETAILED_ACCOUNTS',
                                                   'CURRENT_REVISION',
                                                   'CURRENT_COMMIT'
                                               ]))
        except gerrit_util.GerritError as e:
            error_message = 'Looking up %r: %s' % (instance['url'], e)
            if error_message not in self.access_errors:
                self.access_errors.add(error_message)
            return []

    def gerrit_search(self, instance, owner=None, reviewer=None):
        if instance['url'] in self.skip_servers:
            return []
        max_age = datetime.today() - self.modified_after
        filters = ['-age:%ss' % (max_age.days * 24 * 3600 + max_age.seconds)]
        if owner:
            assert not reviewer
            filters.append('owner:%s' % owner)
        else:
            filters.extend(('-owner:%s' % reviewer, 'reviewer:%s' % reviewer))
        # TODO(cjhopman): Should abandoned changes be filtered out when
        # merged_only is not enabled?
        if self.options.merged_only:
            filters.append('status:merged')

        issues = self.gerrit_changes_over_rest(instance, filters)
        self.show_progress()
        issues = [
            self.process_gerrit_issue(instance, issue) for issue in issues
        ]

        issues = filter(self.filter_issue, issues)
        issues = sorted(issues, key=lambda i: i['modified'], reverse=True)

        return issues

    def process_gerrit_issue(self, instance, issue):
        ret = {}
        if self.options.deltas:
            ret['delta'] = DefaultFormatter().format(
                '+{insertions},-{deletions}', **issue)
        ret['status'] = issue['status']
        if 'shorturl' in instance:
            protocol = instance.get('short_url_protocol', 'http')
            url = instance['shorturl']
        else:
            protocol = 'https'
            url = instance['url']
        ret['review_url'] = '%s://%s/%s' % (protocol, url, issue['_number'])

        ret['header'] = issue['subject']
        ret['owner'] = issue['owner'].get('email', '')
        ret['author'] = ret['owner']
        ret['created'] = datetime_from_gerrit(issue['created'])
        ret['modified'] = datetime_from_gerrit(issue['updated'])
        if 'messages' in issue:
            ret['replies'] = self.process_gerrit_issue_replies(
                issue['messages'])
        else:
            ret['replies'] = []
        ret['reviewers'] = set(r['author'] for r in ret['replies'])
        ret['reviewers'].discard(ret['author'])
        ret['bugs'] = extract_bug_numbers_from_description(issue)
        return ret

    @staticmethod
    def process_gerrit_issue_replies(replies):
        ret = []
        replies = filter(lambda r: 'author' in r and 'email' in r['author'],
                         replies)
        for reply in replies:
            ret.append({
                'author': reply['author']['email'],
                'created': datetime_from_gerrit(reply['date']),
                'content': reply['message'],
            })
        return ret

    def monorail_get_auth_http(self):
        # Manually use a long timeout (10m); for some users who have a
        # long history on the issue tracker, whatever the default timeout
        # is is reached.
        return auth.Authenticator().authorize(httplib2.Http(timeout=600))

    def filter_modified_monorail_issue(self, issue):
        """Precisely checks if an issue has been modified in the time range.

        This fetches all issue comments to check if the issue has been modified
        in the time range specified by user. This is needed because monorail
        only allows filtering by last updated and published dates, which is not
        sufficient to tell whether a given issue has been modified at some
        specific time range. Any update to the issue is a reported as comment
        on Monorail.

        Args:
            issue: Issue dict as returned by monorail_query_issues method. In
                particular, must have a key 'uid' formatted as
                'project:issue_id'.

        Returns:
            Passed issue if modified, None otherwise.
        """
        http = self.monorail_get_auth_http()
        project, issue_id = issue['uid'].split(':')
        url = ('https://monorail-prod.appspot.com/_ah/api/monorail/v1/projects'
               '/%s/issues/%s/comments?maxResults=10000') % (project, issue_id)
        _, body = http.request(url)
        self.show_progress()
        content = json.loads(body)
        if not content:
            logging.error('Unable to parse %s response from monorail.', project)
            return issue

        for item in content.get('items', []):
            comment_published = datetime_from_monorail(item['published'])
            if self.filter_modified(comment_published):
                return issue

        return None

    def monorail_query_issues(self, project, query):
        http = self.monorail_get_auth_http()
        url = ('https://monorail-prod.appspot.com/_ah/api/monorail/v1/projects'
               '/%s/issues') % project
        query_data = urllib.parse.urlencode(query)
        url = url + '?' + query_data
        _, body = http.request(url)
        self.show_progress()
        try:
          content = json.loads(body)
        except json.JSONDecodeError:
          content = None
        if not content:
            logging.error('Unable to parse %s response from monorail.', project)
            return []

        issues = []
        project_config = monorail_projects.get(project, {})
        for item in content.get('items', []):
            if project_config.get('shorturl'):
                protocol = project_config.get('short_url_protocol', 'http')
                item_url = '%s://%s/%d' % (protocol, project_config['shorturl'],
                                           item['id'])
            else:
                item_url = (
                    'https://bugs.chromium.org/p/%s/issues/detail?id=%d' %
                    (project, item['id']))
            issue = {
                'uid': '%s:%s' % (project, item['id']),
                'header': item['title'],
                'created': datetime_from_monorail(item['published']),
                'modified': datetime_from_monorail(item['updated']),
                'author': item['author']['name'],
                'url': item_url,
                'comments': [],
                'status': item['status'],
                'labels': [],
                'components': []
            }
            if 'owner' in item:
                issue['owner'] = item['owner']['name']
            else:
                issue['owner'] = 'None'
            if 'labels' in item:
                issue['labels'] = item['labels']
            if 'components' in item:
                issue['components'] = item['components']
            issues.append(issue)

        return issues

    def monorail_issue_search(self, project):
        epoch = datetime.utcfromtimestamp(0)
        # Defaults to @chromium.org email if one wasn't provided on -u option.
        user_str = (self.options.email if self.options.email.find('@') >= 0 else
                    '%s@chromium.org' % self.user)

        issues = self.monorail_query_issues(
            project, {
                'maxResults':
                10000,
                'q':
                user_str,
                'publishedMax':
                '%d' % (self.modified_before - epoch).total_seconds(),
                'updatedMin':
                '%d' % (self.modified_after - epoch).total_seconds(),
            })

        if self.options.completed_issues:
            return [
                issue for issue in issues
                if (self.match(issue['owner']) and issue['status'].lower() in (
                    'verified', 'fixed'))
            ]

        return [
            issue for issue in issues
            if user_str in (issue['author'], issue['owner'])
        ]

    def monorail_get_issues(self, project, issue_ids):
        return self.monorail_query_issues(project, {
            'maxResults': 10000,
            'q': 'id:%s' % ','.join(issue_ids)
        })

    def print_heading(self, heading):
        print()
        print(self.options.output_format_heading.format(heading=heading))

    def match(self, author):
        if '@' in self.user:
            return author == self.user
        return author.startswith(self.user + '@')

    def print_change(self, change):
        activity = len([
            reply for reply in change['replies'] if self.match(reply['author'])
        ])
        optional_values = {
            'created': change['created'].date().isoformat(),
            'modified': change['modified'].date().isoformat(),
            'reviewers': ', '.join(change['reviewers']),
            'status': change['status'],
            'activity': activity,
        }
        if self.options.deltas:
            optional_values['delta'] = change['delta']

        self.print_generic(self.options.output_format,
                           self.options.output_format_changes, change['header'],
                           change['review_url'], change['author'],
                           change['created'], change['modified'],
                           optional_values)

    def print_issue(self, issue):
        optional_values = {
            'created': issue['created'].date().isoformat(),
            'modified': issue['modified'].date().isoformat(),
            'owner': issue['owner'],
            'status': issue['status'],
        }
        self.print_generic(self.options.output_format,
                           self.options.output_format_issues, issue['header'],
                           issue['url'], issue['author'], issue['created'],
                           issue['modified'], optional_values)

    def print_review(self, review):
        activity = len([
            reply for reply in review['replies'] if self.match(reply['author'])
        ])
        optional_values = {
            'created': review['created'].date().isoformat(),
            'modified': review['modified'].date().isoformat(),
            'status': review['status'],
            'activity': activity,
        }
        if self.options.deltas:
            optional_values['delta'] = review['delta']

        self.print_generic(self.options.output_format,
                           self.options.output_format_reviews, review['header'],
                           review['review_url'], review['author'],
                           review['created'], review['modified'],
                           optional_values)

    @staticmethod
    def print_generic(default_fmt,
                      specific_fmt,
                      title,
                      url,
                      author,
                      created,
                      modified,
                      optional_values=None):
        output_format = (specific_fmt
                         if specific_fmt is not None else default_fmt)
        values = {
            'title': title,
            'url': url,
            'author': author,
            'created': created,
            'modified': modified,
        }
        if optional_values is not None:
            values.update(optional_values)
        print(DefaultFormatter().format(output_format, **values))

    def filter_issue(self, issue, should_filter_by_user=True):
        def maybe_filter_username(email):
            return not should_filter_by_user or username(email) == self.user

        if (maybe_filter_username(issue['author'])
                and self.filter_modified(issue['created'])):
            return True
        if (maybe_filter_username(issue['owner'])
                and (self.filter_modified(issue['created'])
                     or self.filter_modified(issue['modified']))):
            return True
        for reply in issue['replies']:
            if self.filter_modified(reply['created']):
                if not should_filter_by_user:
                    break
                if (username(reply['author']) == self.user
                        or (self.user + '@') in reply['content']):
                    break
        else:
            return False
        return True

    def filter_modified(self, modified):
        return self.modified_after < modified < self.modified_before

    def auth_for_changes(self):
        #TODO(cjhopman): Move authentication check for getting changes here.
        pass

    def auth_for_reviews(self):
        # Reviews use all the same instances as changes so no authentication is
        # required.
        pass

    def get_changes(self):
        num_instances = len(gerrit_instances)
        with contextlib.closing(ThreadPool(num_instances)) as pool:
            gerrit_changes = pool.map_async(
                lambda instance: self.gerrit_search(instance, owner=self.user),
                gerrit_instances)
            gerrit_changes = itertools.chain.from_iterable(gerrit_changes.get())
            self.changes = list(gerrit_changes)

    def print_changes(self):
        if self.changes:
            self.print_heading('Changes')
            for change in self.changes:
                self.print_change(change)

    def print_access_errors(self):
        if self.access_errors:
            logging.error('Access Errors:')
            for error in self.access_errors:
                logging.error(error.rstrip())

    def get_reviews(self):
        num_instances = len(gerrit_instances)
        with contextlib.closing(ThreadPool(num_instances)) as pool:
            gerrit_reviews = pool.map_async(
                lambda instance: self.gerrit_search(instance,
                                                    reviewer=self.user),
                gerrit_instances)
            gerrit_reviews = itertools.chain.from_iterable(gerrit_reviews.get())
            self.reviews = list(gerrit_reviews)

    def print_reviews(self):
        if self.reviews:
            self.print_heading('Reviews')
            for review in self.reviews:
                self.print_review(review)

    def get_issues(self):
        with contextlib.closing(ThreadPool(len(monorail_projects))) as pool:
            monorail_issues = pool.map(self.monorail_issue_search,
                                       monorail_projects.keys())
            monorail_issues = list(
                itertools.chain.from_iterable(monorail_issues))

        if not monorail_issues:
            return

        with contextlib.closing(ThreadPool(len(monorail_issues))) as pool:
            filtered_issues = pool.map(self.filter_modified_monorail_issue,
                                       monorail_issues)
            self.issues = [issue for issue in filtered_issues if issue]

    def get_referenced_issues(self):
        if not self.issues:
            self.get_issues()

        if not self.changes:
            self.get_changes()

        referenced_issue_uids = set(
            itertools.chain.from_iterable(change['bugs']
                                          for change in self.changes))
        fetched_issue_uids = set(issue['uid'] for issue in self.issues)
        missing_issue_uids = referenced_issue_uids - fetched_issue_uids

        missing_issues_by_project = collections.defaultdict(list)
        for issue_uid in missing_issue_uids:
            project, issue_id = issue_uid.split(':')
            missing_issues_by_project[project].append(issue_id)

        for project, issue_ids in missing_issues_by_project.items():
            self.referenced_issues += self.monorail_get_issues(
                project, issue_ids)

    def print_issues(self):
        if self.issues:
            self.print_heading('Issues')
            for issue in self.issues:
                self.print_issue(issue)

    def print_changes_by_issue(self, skip_empty_own):
        if not self.issues or not self.changes:
            return

        self.print_heading('Changes by referenced issue(s)')
        issues = {issue['uid']: issue for issue in self.issues}
        ref_issues = {issue['uid']: issue for issue in self.referenced_issues}
        changes_by_issue_uid = collections.defaultdict(list)
        changes_by_ref_issue_uid = collections.defaultdict(list)
        changes_without_issue = []
        for change in self.changes:
            added = False
            for issue_uid in change['bugs']:
                if issue_uid in issues:
                    changes_by_issue_uid[issue_uid].append(change)
                    added = True
                if issue_uid in ref_issues:
                    changes_by_ref_issue_uid[issue_uid].append(change)
                    added = True
            if not added:
                changes_without_issue.append(change)

        # Changes referencing own issues.
        for issue_uid in issues:
            if changes_by_issue_uid[issue_uid] or not skip_empty_own:
                self.print_issue(issues[issue_uid])
            if changes_by_issue_uid[issue_uid]:
                print()
                for change in changes_by_issue_uid[issue_uid]:
                    print('    ', end='')  # this prints no newline
                    self.print_change(change)
                print()

        # Changes referencing others' issues.
        for issue_uid in ref_issues:
            assert changes_by_ref_issue_uid[issue_uid]
            self.print_issue(ref_issues[issue_uid])
            for change in changes_by_ref_issue_uid[issue_uid]:
                print('', end=' '
                      )  # this prints one space due to comma, but no newline
                self.print_change(change)

        # Changes referencing no issues.
        if changes_without_issue:
            print(
                self.options.output_format_no_url.format(title='Other changes'))
            for change in changes_without_issue:
                print('', end=' '
                      )  # this prints one space due to comma, but no newline
                self.print_change(change)

    def print_activity(self):
        self.print_changes()
        self.print_reviews()
        self.print_issues()

    def dump_json(self, ignore_keys=None):
        if ignore_keys is None:
            ignore_keys = ['replies']

        def format_for_json_dump(in_array):
            output = {}
            for item in in_array:
                url = item.get('url') or item.get('review_url')
                if not url:
                    raise Exception('Dumped item %s does not specify url' %
                                    item)
                output[url] = dict(
                    (k, v) for k, v in item.items() if k not in ignore_keys)
            return output

        class PythonObjectEncoder(json.JSONEncoder):
            def default(self, o):  # pylint: disable=method-hidden
                if isinstance(o, datetime):
                    return o.isoformat()
                if isinstance(o, set):
                    return list(o)
                return json.JSONEncoder.default(self, o)

        output = {
            'reviews': format_for_json_dump(self.reviews),
            'changes': format_for_json_dump(self.changes),
            'issues': format_for_json_dump(self.issues)
        }
        print(json.dumps(output, indent=2, cls=PythonObjectEncoder))


def main():
    parser = optparse.OptionParser(description=sys.modules[__name__].__doc__)
    parser.add_option(
        '-u',
        '--user',
        metavar='<email>',
        # Look for USER and USERNAME (Windows) environment variables.
        default=os.environ.get('USER', os.environ.get('USERNAME')),
        help='Filter on user, default=%default')
    parser.add_option('-b',
                      '--begin',
                      metavar='<date>',
                      help='Filter issues created after the date (mm/dd/yy)')
    parser.add_option('-e',
                      '--end',
                      metavar='<date>',
                      help='Filter issues created before the date (mm/dd/yy)')
    quarter_begin, quarter_end = get_quarter_of(datetime.today() -
                                                relativedelta(months=2))
    parser.add_option(
        '-Q',
        '--last_quarter',
        action='store_true',
        help='Use last quarter\'s dates, i.e. %s to %s' %
        (quarter_begin.strftime('%Y-%m-%d'), quarter_end.strftime('%Y-%m-%d')))
    parser.add_option('-Y',
                      '--this_year',
                      action='store_true',
                      help='Use this year\'s dates')
    parser.add_option('-w',
                      '--week_of',
                      metavar='<date>',
                      help='Show issues for week of the date (mm/dd/yy)')
    parser.add_option(
        '-W',
        '--last_week',
        action='count',
        help='Show last week\'s issues. Use more times for more weeks.')
    parser.add_option(
        '-a',
        '--auth',
        action='store_true',
        help='Ask to authenticate for instances with no auth cookie')
    parser.add_option('-d',
                      '--deltas',
                      action='store_true',
                      help='Fetch deltas for changes.')
    parser.add_option(
        '--no-referenced-issues',
        action='store_true',
        help='Do not fetch issues referenced by owned changes. Useful in '
        'combination with --changes-by-issue when you only want to list '
        'issues that have also been modified in the same time period.')
    parser.add_option(
        '--skip_servers',
        action='store',
        default='',
        help='A comma separated list of gerrit and rietveld servers to ignore')
    parser.add_option(
        '--skip-own-issues-without-changes',
        action='store_true',
        help='Skips listing own issues without changes when showing changes '
        'grouped by referenced issue(s). See --changes-by-issue for more '
        'details.')
    parser.add_option(
        '-F',
        '--config_file',
        metavar='<config_file>',
        help='Configuration file in JSON format, used to add additional gerrit '
        'instances (see source code for an example).')

    activity_types_group = optparse.OptionGroup(
        parser, 'Activity Types',
        'By default, all activity will be looked up and '
        'printed. If any of these are specified, only '
        'those specified will be searched.')
    activity_types_group.add_option('-c',
                                    '--changes',
                                    action='store_true',
                                    help='Show changes.')
    activity_types_group.add_option('-i',
                                    '--issues',
                                    action='store_true',
                                    help='Show issues.')
    activity_types_group.add_option('-r',
                                    '--reviews',
                                    action='store_true',
                                    help='Show reviews.')
    activity_types_group.add_option(
        '--changes-by-issue',
        action='store_true',
        help='Show changes grouped by referenced issue(s).')
    parser.add_option_group(activity_types_group)

    output_format_group = optparse.OptionGroup(
        parser, 'Output Format',
        'By default, all activity will be printed in the '
        'following format: {url} {title}. This can be '
        'changed for either all activity types or '
        'individually for each activity type. The format '
        'is defined as documented for '
        'string.format(...). The variables available for '
        'all activity types are url, title, author, '
        'created and modified. Format options for '
        'specific activity types will override the '
        'generic format.')
    output_format_group.add_option(
        '-f',
        '--output-format',
        metavar='<format>',
        default=u'{url} {title}',
        help='Specifies the format to use when printing all your activity.')
    output_format_group.add_option(
        '--output-format-changes',
        metavar='<format>',
        default=None,
        help='Specifies the format to use when printing changes. Supports the '
        'additional variable {reviewers}')
    output_format_group.add_option(
        '--output-format-issues',
        metavar='<format>',
        default=None,
        help='Specifies the format to use when printing issues. Supports the '
        'additional variable {owner}.')
    output_format_group.add_option(
        '--output-format-reviews',
        metavar='<format>',
        default=None,
        help='Specifies the format to use when printing reviews.')
    output_format_group.add_option(
        '--output-format-heading',
        metavar='<format>',
        default=u'{heading}:',
        help='Specifies the format to use when printing headings. '
        'Supports the variable {heading}.')
    output_format_group.add_option(
        '--output-format-no-url',
        default='{title}',
        help='Specifies the format to use when printing activity without url.')
    output_format_group.add_option(
        '-m',
        '--markdown',
        action='store_true',
        help='Use markdown-friendly output (overrides --output-format '
        'and --output-format-heading)')
    output_format_group.add_option(
        '-j',
        '--json',
        action='store_true',
        help='Output json data (overrides other format options)')
    parser.add_option_group(output_format_group)

    parser.add_option('-v',
                      '--verbose',
                      action='store_const',
                      dest='verbosity',
                      default=logging.WARN,
                      const=logging.INFO,
                      help='Output extra informational messages.')
    parser.add_option('-q',
                      '--quiet',
                      action='store_const',
                      dest='verbosity',
                      const=logging.ERROR,
                      help='Suppress non-error messages.')
    parser.add_option('-M',
                      '--merged-only',
                      action='store_true',
                      dest='merged_only',
                      default=False,
                      help='Shows only changes that have been merged.')
    parser.add_option(
        '-C',
        '--completed-issues',
        action='store_true',
        dest='completed_issues',
        default=False,
        help='Shows only monorail issues that have completed (Fixed|Verified) '
        'by the user.')
    parser.add_option(
        '-o',
        '--output',
        metavar='<file>',
        help='Where to output the results. By default prints to stdout.')

    # Remove description formatting
    parser.format_description = (lambda _: parser.description)  # pylint: disable=no-member

    options, args = parser.parse_args()
    options.local_user = os.environ.get('USER')
    if args:
        parser.error('Args unsupported')
    if not options.user:
        parser.error('USER/USERNAME is not set, please use -u')
    # Retains the original -u option as the email address.
    options.email = options.user
    options.user = username(options.email)

    logging.basicConfig(level=options.verbosity)

    # python-keyring provides easy access to the system keyring.
    try:
        import keyring  # pylint: disable=unused-import,unused-variable,F0401
    except ImportError:
        logging.warning('Consider installing python-keyring')

    if not options.begin:
        if options.last_quarter:
            begin, end = quarter_begin, quarter_end
        elif options.this_year:
            begin, end = get_year_of(datetime.today())
        elif options.week_of:
            begin, end = (get_week_of(
                datetime.strptime(options.week_of, '%m/%d/%y')))
        elif options.last_week:
            begin, end = (
                get_week_of(datetime.today() -
                            timedelta(days=1 + 7 * options.last_week)))
        else:
            begin, end = (get_week_of(datetime.today() - timedelta(days=1)))
    else:
        begin = dateutil.parser.parse(options.begin)
        if options.end:
            end = dateutil.parser.parse(options.end)
        else:
            end = datetime.today()
    options.begin, options.end = begin, end
    if begin >= end:
        # The queries fail in peculiar ways when the begin date is in the
        # future. Give a descriptive error message instead.
        logging.error(
            'Start date (%s) is the same or later than end date (%s)' %
            (begin, end))
        return 1

    if options.markdown:
        options.output_format_heading = '### {heading}\n'
        options.output_format = '  * [{title}]({url})'
        options.output_format_no_url = '  * {title}'
    logging.info('Searching for activity by %s', options.user)
    logging.info('Using range %s to %s', options.begin, options.end)

    if options.config_file:
        with open(options.config_file) as f:
            config = json.load(f)

            for item, entries in config.items():
                if item == 'gerrit_instances':
                    for repo, dic in entries.items():
                        # Use property name as URL
                        dic['url'] = repo
                        gerrit_instances.append(dic)
                elif item == 'monorail_projects':
                    monorail_projects.append(entries)
                else:
                    logging.error('Invalid entry in config file.')
                    return 1

    my_activity = MyActivity(options)
    my_activity.show_progress('Loading data')

    if not (options.changes or options.reviews or options.issues
            or options.changes_by_issue):
        options.changes = True
        options.issues = True
        options.reviews = True

    # First do any required authentication so none of the user interaction has
    # to wait for actual work.
    if options.changes or options.changes_by_issue:
        my_activity.auth_for_changes()
    if options.reviews:
        my_activity.auth_for_reviews()

    logging.info('Looking up activity.....')

    try:
        if options.changes or options.changes_by_issue:
            my_activity.get_changes()
        if options.reviews:
            my_activity.get_reviews()
        if options.issues or options.changes_by_issue:
            my_activity.get_issues()
        if not options.no_referenced_issues:
            my_activity.get_referenced_issues()
    except auth.LoginRequiredError as e:
        logging.error('auth.LoginRequiredError: %s', e)

    my_activity.show_progress('\n')

    my_activity.print_access_errors()

    output_file = None
    try:
        if options.output:
            output_file = open(options.output, 'w')
            logging.info('Printing output to "%s"', options.output)
            sys.stdout = output_file
    except (IOError, OSError) as e:
        logging.error('Unable to write output: %s', e)
    else:
        if options.json:
            my_activity.dump_json()
        else:
            if options.changes:
                my_activity.print_changes()
            if options.reviews:
                my_activity.print_reviews()
            if options.issues:
                my_activity.print_issues()
            if options.changes_by_issue:
                my_activity.print_changes_by_issue(
                    options.skip_own_issues_without_changes)
    finally:
        if output_file:
            logging.info('Done printing to file.')
            sys.stdout = sys.__stdout__
            output_file.close()

    return 0


if __name__ == '__main__':
    # Fix encoding to support non-ascii issue titles.

    try:
        sys.exit(main())
    except KeyboardInterrupt:
        sys.stderr.write('interrupted\n')
        sys.exit(1)
