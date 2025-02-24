# -*- coding: utf-8 -*-

import os
import boto
import sys

sys.path.append(os.path.join(os.path.dirname(__file__), 'extensions'))

extensions = ['sphinx.ext.autodoc', 'sphinx.ext.intersphinx', 'sphinx.ext.todo',
              'githublinks']
autoclass_content = "both"
templates_path = ['_templates']
source_suffix = '.rst'
master_doc = 'index'
project = u'boto'
copyright = u'2009,2010, Mitch Garnaat'
version = boto.__version__
exclude_trees = []
pygments_style = 'sphinx'
html_theme = 'boto_theme'
html_theme_path = ["."]
html_static_path = ['_static']
htmlhelp_basename = 'botodoc'
latex_documents = [
  ('index', 'boto.tex', u'boto Documentation',
   u'Mitch Garnaat', 'manual'),
]
intersphinx_mapping = {'http://docs.python.org/': None}
github_project_url = 'https://github.com/boto/boto/'

try:
    release = os.environ.get('SVN_REVISION', 'HEAD')
    print release
except Exception as e:
    print e

html_title = "boto v%s" % version
