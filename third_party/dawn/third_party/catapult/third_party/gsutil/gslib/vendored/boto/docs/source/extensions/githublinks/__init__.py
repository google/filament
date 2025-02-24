"""Add github roles to sphinx docs.

Based entirely on Doug Hellmann's bitbucket version, but
adapted for Github.
(https://bitbucket.org/dhellmann/sphinxcontrib-bitbucket/)

"""
from urlparse import urljoin

from docutils import nodes, utils
from docutils.parsers.rst.roles import set_classes


def make_node(rawtext, app, type_, slug, options):
    base_url = app.config.github_project_url
    if base_url is None:
        raise ValueError(
            "Configuration value for 'github_project_url' is not set.")
    relative = '%s/%s' % (type_, slug)
    full_ref = urljoin(base_url, relative)
    set_classes(options)
    if type_ == 'issues':
        type_ = 'issue'
    node = nodes.reference(rawtext, type_ + ' ' + utils.unescape(slug),
                           refuri=full_ref, **options)
    return node


def github_sha(name, rawtext, text, lineno, inliner,
                 options={}, content=[]):
    app = inliner.document.settings.env.app
    node = make_node(rawtext, app, 'commit', text, options)
    return [node], []


def github_issue(name, rawtext, text, lineno, inliner,
                 options={}, content=[]):
    try:
        issue = int(text)
    except ValueError:
        msg = inliner.reporter.error(
            "Invalid Github Issue '%s', must be an integer" % text,
            line=lineno)
        problem = inliner.problematic(rawtext, rawtext, msg)
        return [problem], [msg]
    app = inliner.document.settings.env.app
    node = make_node(rawtext, app, 'issues', str(issue), options)
    return [node], []


def setup(app):
    app.info('Adding github link roles')
    app.add_role('sha', github_sha)
    app.add_role('issue', github_issue)
    app.add_config_value('github_project_url', None, 'env')
