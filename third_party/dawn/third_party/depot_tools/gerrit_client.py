#!/usr/bin/env vpython3
# Copyright 2017 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
"""Simple client for the Gerrit REST API.

Example usage:
    ./gerrit_client.py [command] [args]
"""

import json
import logging
import optparse
import subcommand
import sys
import urllib.parse

import gerrit_util
import setup_color

__version__ = '0.1'


def write_result(result, opt):
    if opt.json_file:
        with open(opt.json_file, 'w') as json_file:
            json_file.write(json.dumps(result))


@subcommand.usage('[args ...]')
def CMDmovechanges(parser, args):
    """Move changes to a different destination branch."""
    parser.add_option('-p',
                      '--param',
                      dest='params',
                      action='append',
                      help='repeatable query parameter, format: -p key=value')
    parser.add_option('--destination_branch',
                      dest='destination_branch',
                      help='where to move changes to')

    (opt, args) = parser.parse_args(args)
    if not opt.destination_branch:
        parser.error('--destination_branch is required')
    for p in opt.params:
        if '=' not in p:
            parser.error('--param is key=value, not "%s"' % p)
    host = urllib.parse.urlparse(opt.host).netloc

    limit = 100
    while True:
        result = gerrit_util.QueryChanges(
            host,
            list(tuple(p.split('=', 1)) for p in opt.params),
            limit=limit,
        )
        for change in result:
            gerrit_util.MoveChange(host, change['id'], opt.destination_branch)

        if len(result) < limit:
            break
    logging.info("Done")


@subcommand.usage('[args ...]')
def CMDbranchinfo(parser, args):
    """Get information on a gerrit branch."""
    parser.add_option('--branch', dest='branch', help='branch name')

    (opt, args) = parser.parse_args(args)
    host = urllib.parse.urlparse(opt.host).netloc
    project = urllib.parse.quote_plus(opt.project)
    branch = urllib.parse.quote_plus(opt.branch)
    result = gerrit_util.GetGerritBranch(host, project, branch)
    logging.info(result)
    write_result(result, opt)


@subcommand.usage('[args ...]')
def CMDrawapi(parser, args):
    """Call an arbitrary Gerrit REST API endpoint."""
    parser.add_option('--path',
                      dest='path',
                      help='HTTP path of the API endpoint')
    parser.add_option('--method',
                      dest='method',
                      help='HTTP method for the API (default: GET)')
    parser.add_option('--body', dest='body', help='API JSON body contents')
    parser.add_option('--accept_status',
                      dest='accept_status',
                      help='Comma-delimited list of status codes for success.')

    (opt, args) = parser.parse_args(args)
    if not opt.path:
        parser.error('--path is required')

    host = urllib.parse.urlparse(opt.host).netloc
    kwargs = {}
    if opt.method:
        kwargs['reqtype'] = opt.method.upper()
    if opt.body:
        kwargs['body'] = json.loads(opt.body)
    if opt.accept_status:
        kwargs['accept_statuses'] = [
            int(x) for x in opt.accept_status.split(',')
        ]
    result = gerrit_util.CallGerritApi(host, opt.path, **kwargs)
    logging.info(result)
    write_result(result, opt)


@subcommand.usage('[args ...]')
def CMDbranch(parser, args):
    """Create a branch in a gerrit project."""
    parser.add_option('--branch', dest='branch', help='branch name')
    parser.add_option('--commit', dest='commit', help='commit hash')
    parser.add_option(
        '--allow-existent-branch',
        action='store_true',
        help=('Accept that the branch alread exists as long as the'
              ' branch head points the given commit'))

    (opt, args) = parser.parse_args(args)
    if not opt.project:
        parser.error('--project is required')
    if not opt.branch:
        parser.error('--branch is required')
    if not opt.commit:
        parser.error('--commit is required')

    project = urllib.parse.quote_plus(opt.project)
    host = urllib.parse.urlparse(opt.host).netloc
    branch = urllib.parse.quote_plus(opt.branch)
    result = gerrit_util.GetGerritBranch(host, project, branch)
    if result:
        if not opt.allow_existent_branch:
            raise gerrit_util.GerritError(200, 'Branch already exists')
        if result.get('revision') != opt.commit:
            raise gerrit_util.GerritError(
                200, ('Branch already exists but '
                      'the branch head is not at the given commit'))
    else:
        try:
            result = gerrit_util.CreateGerritBranch(host, project, branch,
                                                    opt.commit)
        except gerrit_util.GerritError as e:
            result = gerrit_util.GetGerritBranch(host, project, branch)
            if not result:
                raise e
            # If reached here, we hit a real conflict error, because the
            # branch just created is pointing a different commit.
            if result.get('revision') != opt.commit:
                raise gerrit_util.GerritError(
                    200, ('Conflict: branch was created but '
                          'the branch head is not at the given commit'))
    logging.info(result)
    write_result(result, opt)


@subcommand.usage('[args ...]')
def CMDtag(parser, args):
    """Create a tag in a gerrit project."""
    parser.add_option('--tag', dest='tag', help='tag name')
    parser.add_option('--commit', dest='commit', help='commit hash')

    (opt, args) = parser.parse_args(args)
    if not opt.project:
        parser.error('--project is required')
    if not opt.tag:
        parser.error('--tag is required')
    if not opt.commit:
        parser.error('--commit is required')

    project = urllib.parse.quote_plus(opt.project)
    host = urllib.parse.urlparse(opt.host).netloc
    tag = urllib.parse.quote_plus(opt.tag)
    result = gerrit_util.CreateGerritTag(host, project, tag, opt.commit)
    logging.info(result)
    write_result(result, opt)


@subcommand.usage('[args ...]')
def CMDhead(parser, args):
    """Update which branch the project HEAD points to."""
    parser.add_option('--branch', dest='branch', help='branch name')

    (opt, args) = parser.parse_args(args)
    if not opt.project:
        parser.error('--project is required')
    if not opt.branch:
        parser.error('--branch is required')

    project = urllib.parse.quote_plus(opt.project)
    host = urllib.parse.urlparse(opt.host).netloc
    branch = urllib.parse.quote_plus(opt.branch)
    result = gerrit_util.UpdateHead(host, project, branch)
    logging.info(result)
    write_result(result, opt)


@subcommand.usage('[args ...]')
def CMDheadinfo(parser, args):
    """Retrieves the current HEAD of the project."""

    (opt, args) = parser.parse_args(args)
    if not opt.project:
        parser.error('--project is required')

    project = urllib.parse.quote_plus(opt.project)
    host = urllib.parse.urlparse(opt.host).netloc
    result = gerrit_util.GetHead(host, project)
    logging.info(result)
    write_result(result, opt)


@subcommand.usage('[args ...]')
def CMDchanges(parser, args):
    """Queries gerrit for matching changes."""
    parser.add_option('-p',
                      '--param',
                      dest='params',
                      action='append',
                      default=[],
                      help='repeatable query parameter, format: -p key=value')
    parser.add_option('--query', help='raw gerrit search query string')
    parser.add_option('-o',
                      '--o-param',
                      dest='o_params',
                      action='append',
                      help='gerrit output parameters, e.g. ALL_REVISIONS')
    parser.add_option('--limit',
                      dest='limit',
                      type=int,
                      help='maximum number of results to return')
    parser.add_option('--start',
                      dest='start',
                      type=int,
                      help='how many changes to skip '
                      '(starting with the most recent)')

    (opt, args) = parser.parse_args(args)
    if not (opt.params or opt.query):
        parser.error('--param or --query required')
    for p in opt.params:
        if '=' not in p:
            parser.error('--param is key=value, not "%s"' % p)

    result = gerrit_util.QueryChanges(
        urllib.parse.urlparse(opt.host).netloc,
        list(tuple(p.split('=', 1)) for p in opt.params),
        first_param=opt.query,
        start=opt.start,  # Default: None
        limit=opt.limit,  # Default: None
        o_params=opt.o_params,  # Default: None
    )
    logging.info('Change query returned %d changes.', len(result))
    write_result(result, opt)


@subcommand.usage('[args ...]')
def CMDrelatedchanges(parser, args):
    """Gets related changes for a given change and revision."""
    parser.add_option('-c', '--change', type=str, help='change id')
    parser.add_option('-r', '--revision', type=str, help='revision id')

    (opt, args) = parser.parse_args(args)

    result = gerrit_util.GetRelatedChanges(
        urllib.parse.urlparse(opt.host).netloc,
        change=opt.change,
        revision=opt.revision,
    )
    logging.info(result)
    write_result(result, opt)


@subcommand.usage('[args ...]')
def CMDcreatechange(parser, args):
    """Create a new change in gerrit."""
    parser.add_option('-s', '--subject', help='subject for change')
    parser.add_option('-b',
                      '--branch',
                      default='main',
                      help='target branch for change')
    parser.add_option(
        '-p',
        '--param',
        dest='params',
        action='append',
        help='repeatable field value parameter, format: -p key=value')

    parser.add_option('--cc',
                      dest='cc_list',
                      action='append',
                      help='CC address to notify, format: --cc foo@example.com')

    (opt, args) = parser.parse_args(args)
    for p in opt.params:
        if '=' not in p:
            parser.error('--param is key=value, not "%s"' % p)

    params = list(tuple(p.split('=', 1)) for p in opt.params)

    if opt.cc_list:
        params.append(('notify_details', {'CC': {'accounts': opt.cc_list}}))

    result = gerrit_util.CreateChange(
        urllib.parse.urlparse(opt.host).netloc,
        opt.project,
        branch=opt.branch,
        subject=opt.subject,
        params=params,
    )
    logging.info(result)
    write_result(result, opt)


@subcommand.usage('[args ...]')
def CMDchangeedit(parser, args):
    """Puts content of a file into a change edit."""
    parser.add_option('-c', '--change', type=int, help='change number')
    parser.add_option('--path', help='path for file')
    parser.add_option('--file', help='file to place at |path|')

    (opt, args) = parser.parse_args(args)

    with open(opt.file) as f:
        data = f.read()
    result = gerrit_util.ChangeEdit(
        urllib.parse.urlparse(opt.host).netloc, opt.change, opt.path, data)
    logging.info(result)
    write_result(result, opt)


@subcommand.usage('[args ...]')
def CMDpublishchangeedit(parser, args):
    """Publish a Gerrit change edit."""
    parser.add_option('-c', '--change', type=int, help='change number')
    parser.add_option('--notify', help='whether to notify')

    (opt, args) = parser.parse_args(args)

    result = gerrit_util.PublishChangeEdit(
        urllib.parse.urlparse(opt.host).netloc, opt.change, opt.notify)
    logging.info(result)
    write_result(result, opt)


@subcommand.usage('[args ...]')
def CMDsubmitchange(parser, args):
    """Submit a Gerrit change."""
    parser.add_option('-c', '--change', type=int, help='change number')
    (opt, args) = parser.parse_args(args)
    result = gerrit_util.SubmitChange(
        urllib.parse.urlparse(opt.host).netloc, opt.change)
    logging.info(result)
    write_result(result, opt)


@subcommand.usage('[args ...]')
def CMDchangesubmittedtogether(parser, args):
    """Get all changes submitted with the given one."""
    parser.add_option('-c', '--change', type=int, help='change number')
    (opt, args) = parser.parse_args(args)
    result = gerrit_util.GetChangesSubmittedTogether(
        urllib.parse.urlparse(opt.host).netloc, opt.change)
    logging.info(result)
    write_result(result, opt)


@subcommand.usage('[args ...]')
def CMDgetcommitincludedin(parser, args):
    """Retrieves the branches and tags for a given commit."""
    parser.add_option('--commit', dest='commit', help='commit hash')
    (opt, args) = parser.parse_args(args)
    result = gerrit_util.GetCommitIncludedIn(
        urllib.parse.urlparse(opt.host).netloc, opt.project, opt.commit)
    logging.info(result)
    write_result(result, opt)


@subcommand.usage('[args ...]')
def CMDsetbotcommit(parser, args):
    """Sets bot-commit+1 to a bot generated change."""
    parser.add_option('-c', '--change', type=int, help='change number')
    (opt, args) = parser.parse_args(args)
    result = gerrit_util.SetReview(urllib.parse.urlparse(opt.host).netloc,
                                   opt.change,
                                   labels={'Bot-Commit': 1},
                                   ready=True)
    logging.info(result)
    write_result(result, opt)


@subcommand.usage('[args ...]')
def CMDsetlabel(parser, args):
    """Sets a label to a specific value on a given change."""
    parser.add_option('-c', '--change', type=int, help='change number')
    parser.add_option('-l',
                      '--label',
                      nargs=2,
                      metavar=('label_name', 'label_value'))
    (opt, args) = parser.parse_args(args)
    result = gerrit_util.SetReview(urllib.parse.urlparse(opt.host).netloc,
                                   opt.change,
                                   labels={opt.label[0]: opt.label[1]})
    logging.info(result)
    write_result(result, opt)


@subcommand.usage('[args ...]')
def CMDaddMessage(parser, args):
    """Adds a message to a given change at given revision."""
    parser.add_option('-c', '--change', type=int, help='change number')
    parser.add_option(
        '-r',
        '--revision',
        type=str,
        default='current',
        help='revision ID. See '
        'https://gerrit-review.googlesource.com/Documentation/rest-api-changes.html#revision-id '  # pylint: disable=line-too-long
        'for acceptable format')
    parser.add_option('-m', '--message', type=str, help='message to add')
    # This default matches the Gerrit REST API & web UI defaults.
    parser.add_option('--automatic-attention-set-update',
                      action='store_true',
                      help='Update the attention set (default)')
    parser.add_option('--no-automatic-attention-set-update',
                      dest='automatic_attention_set_update',
                      action='store_false',
                      help='Do not update the attention set')
    (opt, args) = parser.parse_args(args)
    if not opt.change:
        parser.error('--change is required')
    if not opt.message:
        parser.error('--message is required')
    result = gerrit_util.SetReview(
        urllib.parse.urlparse(opt.host).netloc,
        opt.change,
        revision=opt.revision,
        msg=opt.message,
        automatic_attention_set_update=opt.automatic_attention_set_update)
    logging.info(result)
    write_result(result, opt)


@subcommand.usage('')
def CMDrestore(parser, args):
    """Restores a Gerrit change."""
    parser.add_option('-c', '--change', type=str, help='change number')
    parser.add_option('-m',
                      '--message',
                      default='',
                      help='reason for restoring')

    (opt, args) = parser.parse_args(args)
    if not opt.change:
        parser.error('--change is required')
    result = gerrit_util.RestoreChange(
        urllib.parse.urlparse(opt.host).netloc, opt.change, opt.message)
    logging.info(result)
    write_result(result, opt)


@subcommand.usage('')
def CMDabandon(parser, args):
    """Abandons a Gerrit change."""
    parser.add_option('-c', '--change', type=int, help='change number')
    parser.add_option('-m',
                      '--message',
                      default='',
                      help='reason for abandoning')

    (opt, args) = parser.parse_args(args)
    if not opt.change:
        parser.error('--change is required')
    result = gerrit_util.AbandonChange(
        urllib.parse.urlparse(opt.host).netloc, opt.change, opt.message)
    logging.info(result)
    write_result(result, opt)


@subcommand.usage('')
def CMDmass_abandon(parser, args):
    """Mass abandon changes

    Abandons CLs that match search criteria provided by user. Before any change
    is actually abandoned, user is presented with a list of CLs that will be
    affected if user confirms. User can skip confirmation by passing --force
    parameter.

    The script can abandon up to 100 CLs per invocation.

    Examples:
    gerrit_client.py mass-abandon --host https://HOST -p 'project=repo2'
    gerrit_client.py mass-abandon --host https://HOST -p 'message=testing'
    gerrit_client.py mass-abandon --host https://HOST -p 'is=wip' -p 'age=1y'
    """
    parser.add_option('-p',
                      '--param',
                      dest='params',
                      action='append',
                      default=[],
                      help='repeatable query parameter, format: -p key=value')
    parser.add_option('-m',
                      '--message',
                      default='',
                      help='reason for abandoning')
    parser.add_option('-f',
                      '--force',
                      action='store_true',
                      help='Don\'t prompt for confirmation')

    opt, args = parser.parse_args(args)

    for p in opt.params:
        if '=' not in p:
            parser.error('--param is key=value, not "%s"' % p)
    search_query = list(tuple(p.split('=', 1)) for p in opt.params)
    if not any(t for t in search_query if t[0] == 'owner'):
        # owner should always be present when abandoning changes
        search_query.append(('owner', 'me'))
    search_query.append(('status', 'open'))
    logging.info("Searching for: %s" % search_query)

    host = urllib.parse.urlparse(opt.host).netloc

    result = gerrit_util.QueryChanges(
        host,
        search_query,
        # abandon at most 100 changes as not all Gerrit instances support
        # unlimited results.
        limit=100,
    )
    if len(result) == 0:
        logging.warning("Nothing to abandon")
        return

    logging.warning("%s CLs match search query: " % len(result))
    for change in result:
        logging.warning("[ID: %d] %s" % (change['_number'], change['subject']))

    if not opt.force:
        q = input('Do you want to move forward with abandoning? [y to confirm] '
                  ).strip()
        if q not in ['y', 'Y']:
            logging.warning("Aborting...")
            return

    for change in result:
        logging.warning("Abandoning: %s" % change['subject'])
        gerrit_util.AbandonChange(host, change['id'], opt.message)

    logging.warning("Done")


class OptionParser(optparse.OptionParser):
    """Creates the option parse and add --verbose support."""
    def __init__(self, *args, **kwargs):
        optparse.OptionParser.__init__(self,
                                       *args,
                                       version=__version__,
                                       **kwargs)
        self.add_option('--verbose',
                        action='count',
                        default=0,
                        help='Use 2 times for more debugging info')
        self.add_option('--host', dest='host', help='Url of host.')
        self.add_option('--project', dest='project', help='project name')
        self.add_option('--json_file',
                        dest='json_file',
                        help='output json filepath')

    def parse_args(self, args=None, values=None):
        options, args = optparse.OptionParser.parse_args(self, args, values)
        # Host is always required
        if not options.host:
            self.error('--host is required')
        levels = [logging.WARNING, logging.INFO, logging.DEBUG]
        logging.basicConfig(level=levels[min(options.verbose, len(levels) - 1)])
        return options, args


def main(argv):
    dispatcher = subcommand.CommandDispatcher(__name__)
    return dispatcher.execute(OptionParser(), argv)


if __name__ == '__main__':
    # These affect sys.stdout so do it outside of main() to simplify mocks in
    # unit testing.
    setup_color.init()
    try:
        sys.exit(main(sys.argv[1:]))
    except KeyboardInterrupt:
        sys.stderr.write('interrupted\n')
        sys.exit(1)
