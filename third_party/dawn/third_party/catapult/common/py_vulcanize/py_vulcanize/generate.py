# Copyright (c) 2014 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import absolute_import
from __future__ import division
from __future__ import print_function

import os
import subprocess
import sys
import tempfile

from py_vulcanize import html_generation_controller

try:
  from six import StringIO
except ImportError:
  from io import StringIO



html_warning_message = """


<!--
WARNING: This file is auto generated.

         Do not edit directly.
-->
"""

js_warning_message = """
// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

/* WARNING: This file is auto generated.
 *
 * Do not edit directly.
 */
"""

css_warning_message = """
/* Copyright 2015 The Chromium Authors. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file. */

/* WARNING: This file is auto-generated.
 *
 * Do not edit directly.
 */
"""

origin_trial_tokens = [
  # WebComponent V0 origin trial token for googleusercontent.com + subdomains.
  # This is the domain from which traces in cloud storage are served.
  # Expires Nov 5, 2020. See https://crbug.com/1021137
  "AnYuQDtUf6OrWCmR9Okd67JhWVTbmnRedvPi1TEvAxac8+1p6o9q08FoDO6oCbLD0xEqev+SkZFiIhFSzlY9HgUAAABxeyJvcmlnaW4iOiJodHRwczovL2dvb2dsZXVzZXJjb250ZW50LmNvbTo0NDMiLCJmZWF0dXJlIjoiV2ViQ29tcG9uZW50c1YwIiwiZXhwaXJ5IjoxNjA0NjE0NTM4LCJpc1N1YmRvbWFpbiI6dHJ1ZX0=",
  # This is for chromium-build-stats.appspot.com (ukai@)
  # Expires Feb 2, 2021. see https://crbug.com/1050215
  "AkFXw3wHnOs/XXYqFXpc3diDLrRFd9PTgGs/gs43haZmngI/u1g8L4bDnSKLZkB6fecjmjTwcAMQFCpWMAoHSQEAAAB8eyJvcmlnaW4iOiJodHRwczovL2Nocm9taXVtLWJ1aWxkLXN0YXRzLmFwcHNwb3QuY29tOjQ0MyIsImZlYXR1cmUiOiJXZWJDb21wb25lbnRzVjAiLCJleHBpcnkiOjE2MTIyMjM5OTksImlzU3ViZG9tYWluIjp0cnVlfQ==",
  # This is for chromium-build-stats-staging.appspot.com (ukai@)
  # Expires Feb 2, 2021, see https://crbug.com/1050215
  "AtQY4wpX9+nj+Vn27cTgygzIPbtB2WoAoMQR5jK9mCm/H2gRIDH6MmGVAaziv9XnYTDKjhBnQYtecbTiIHCQiAIAAACEeyJvcmlnaW4iOiJodHRwczovL2Nocm9taXVtLWJ1aWxkLXN0YXRzLXN0YWdpbmcuYXBwc3BvdC5jb206NDQzIiwiZmVhdHVyZSI6IldlYkNvbXBvbmVudHNWMCIsImV4cGlyeSI6MTYxMjIyMzk5OSwiaXNTdWJkb21haW4iOnRydWV9"
  #
  # Add more tokens here if traces are served from other domains.
  # WebComponent V0 origin tiral token is generated on
  # https://developers.chrome.com/origintrials/#/trials/active
]

def _AssertIsUTF8(f):
  if isinstance(f, StringIO):
    return
  assert f.encoding == 'utf-8'


def _MinifyJS(input_js):
  py_vulcanize_path = os.path.abspath(os.path.join(
      os.path.dirname(__file__), '..'))
  rjsmin_path = os.path.abspath(
      os.path.join(py_vulcanize_path, 'third_party', 'rjsmin', 'rjsmin.py'))

  with tempfile.NamedTemporaryFile() as _:
    args = [
        sys.executable,
        rjsmin_path
    ]
    p = subprocess.Popen(args,
                         stdin=subprocess.PIPE,
                         stdout=subprocess.PIPE,
                         stderr=subprocess.PIPE)
    res = p.communicate(input=input_js.encode('utf-8'))
    errorcode = p.wait()
    if errorcode != 0:
      sys.stderr.write('rJSmin exited with error code %d' % errorcode)
      sys.stderr.write(res[1].decode('utf-8'))
      raise Exception('Failed to minify, omgah')
    return res[0].decode('utf-8')


def GenerateJS(load_sequence,
               use_include_tags_for_scripts=False,
               dir_for_include_tag_root=None,
               minify=False,
               report_sizes=False):
  f = StringIO()
  GenerateJSToFile(f,
                   load_sequence,
                   use_include_tags_for_scripts,
                   dir_for_include_tag_root,
                   minify=minify,
                   report_sizes=report_sizes)

  return f.getvalue()


def GenerateJSToFile(f,
                     load_sequence,
                     use_include_tags_for_scripts=False,
                     dir_for_include_tag_root=None,
                     minify=False,
                     report_sizes=False):
  _AssertIsUTF8(f)
  if use_include_tags_for_scripts and dir_for_include_tag_root is None:
    raise Exception('Must provide dir_for_include_tag_root')

  f.write(js_warning_message)
  f.write('\n')

  if not minify:
    flatten_to_file = f
  else:
    flatten_to_file = StringIO()

  for module in load_sequence:
    module.AppendJSContentsToFile(flatten_to_file,
                                  use_include_tags_for_scripts,
                                  dir_for_include_tag_root)
  if minify:
    js = flatten_to_file.getvalue()
    minified_js = _MinifyJS(js)
    f.write(minified_js)
    f.write('\n')

  if report_sizes:
    for module in load_sequence:
      s = StringIO()
      module.AppendJSContentsToFile(s,
                                    use_include_tags_for_scripts,
                                    dir_for_include_tag_root)

      # Add minified size info.
      js = s.getvalue()
      min_js_size = str(len(_MinifyJS(js)))

      # Print names for this module. Some domain-specific simplifications
      # are included to make pivoting more obvious.
      parts = module.name.split('.')
      if parts[:2] == ['base', 'ui']:
        parts = ['base_ui'] + parts[2:]
      if parts[:2] == ['tracing', 'importer']:
        parts = ['importer'] + parts[2:]
      tln = parts[0]
      sln = '.'.join(parts[:2])

      # Output
      print(('%i\t%s\t%s\t%s\t%s' %
             (len(js), min_js_size, module.name, tln, sln)))
      sys.stdout.flush()


class ExtraScript(object):

  def __init__(self, script_id=None, text_content=None, content_type=None):
    if script_id is not None:
      assert script_id[0] != '#'
    self.script_id = script_id
    self.text_content = text_content
    self.content_type = content_type

  def WriteToFile(self, output_file):
    _AssertIsUTF8(output_file)
    attrs = []
    if self.script_id:
      attrs.append('id="%s"' % self.script_id)
    if self.content_type:
      attrs.append('content-type="%s"' % self.content_type)

    if len(attrs) > 0:
      output_file.write('<script %s>\n' % ' '.join(attrs))
    else:
      output_file.write('<script>\n')
    if self.text_content:
      output_file.write(self.text_content)
    output_file.write('</script>\n')


def _MinifyCSS(css_text):
  py_vulcanize_path = os.path.abspath(os.path.join(
      os.path.dirname(__file__), '..'))
  rcssmin_path = os.path.abspath(
      os.path.join(py_vulcanize_path, 'third_party', 'rcssmin', 'rcssmin.py'))

  with tempfile.NamedTemporaryFile() as _:
    rcssmin_args = [sys.executable, rcssmin_path]
    p = subprocess.Popen(rcssmin_args,
                         stdin=subprocess.PIPE,
                         stdout=subprocess.PIPE,
                         stderr=subprocess.PIPE)
    res = p.communicate(input=css_text.encode('utf-8'))
    errorcode = p.wait()
    if errorcode != 0:
      sys.stderr.write('rCSSmin exited with error code %d' % errorcode)
      sys.stderr.write(res[1])
      raise Exception('Failed to generate css for %s.' % css_text)
    return res[0].decode('utf-8')


def GenerateStandaloneHTMLAsString(*args, **kwargs):
  f = StringIO()
  GenerateStandaloneHTMLToFile(f, *args, **kwargs)
  return f.getvalue()

def _WriteOriginTrialTokens(output_file):
  for token in origin_trial_tokens:
    output_file.write('  <meta http-equiv="origin-trial" content="')
    output_file.write(token)
    output_file.write('">\n')

def GenerateStandaloneHTMLToFile(output_file,
                                 load_sequence,
                                 title=None,
                                 flattened_js_url=None,
                                 extra_scripts=None,
                                 minify=False,
                                 report_sizes=False,
                                 output_html_head_and_body=True):
  """Writes a HTML file with the content of all modules in a load sequence.

  The load_sequence is a list of (HTML or JS) Module objects; the order that
  they're inserted into the file depends on their type and position in the load
  sequence.
  """
  _AssertIsUTF8(output_file)
  extra_scripts = extra_scripts or []

  if output_html_head_and_body:
    output_file.write(
        '<!DOCTYPE html>\n'
        '<html>\n'
        '  <head i18n-values="dir:textdirection;">\n'
        '  <meta http-equiv="Content-Type" content="text/html;'
        'charset=utf-8">\n')
    _WriteOriginTrialTokens(output_file)
    if title:
      output_file.write('  <title>%s</title>\n  ' % title)
  else:
    assert title is None

  loader = load_sequence[0].loader

  written_style_sheets = set()

  class HTMLGenerationController(
      html_generation_controller.HTMLGenerationController):

    def __init__(self, module):
      self.module = module

    def GetHTMLForStylesheetHRef(self, href):
      resource = self.module.HRefToResource(
          href, '<link rel="stylesheet" href="%s">' % href)
      style_sheet = loader.LoadStyleSheet(resource.name)

      if style_sheet in written_style_sheets:
        return None
      written_style_sheets.add(style_sheet)

      text = style_sheet.contents_with_inlined_images
      if minify:
        text = _MinifyCSS(text)
      return '<style>\n%s\n</style>' % text

  for module in load_sequence:
    controller = HTMLGenerationController(module)
    module.AppendHTMLContentsToFile(output_file, controller, minify=minify)

  if flattened_js_url:
    output_file.write('<script src="%s"></script>\n' % flattened_js_url)
  else:
    output_file.write('<script>\n')
    js = GenerateJS(load_sequence, minify=minify, report_sizes=report_sizes)
    output_file.write(js)
    output_file.write('</script>\n')

  for extra_script in extra_scripts:
    extra_script.WriteToFile(output_file)

  if output_html_head_and_body:
    output_file.write('</head>\n  <body>\n  </body>\n</html>\n')
