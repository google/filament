# Copyright 2013 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

"""Utility function for stripping comments out of JavaScript source code."""

from __future__ import absolute_import
import re


def _TokenizeJS(text):
  """Splits source code text into segments in preparation for comment stripping.

  Note that this doesn't tokenize for parsing. There is no notion of statements,
  variables, etc. The only tokens of interest are comment-related tokens.

  Args:
    text: The contents of a JavaScript file.

  Yields:
    A succession of strings in the file, including all comment-related symbols.
  """
  rest = text
  tokens = ['//', '/*', '*/', '\n']
  next_tok = re.compile('|'.join(re.escape(x) for x in tokens))
  while len(rest):
    m = next_tok.search(rest)
    if not m:
      # end of string
      yield rest
      return
    min_index = m.start()
    end_index = m.end()

    if min_index > 0:
      yield rest[:min_index]

    yield rest[min_index:end_index]
    rest = rest[end_index:]


def StripJSComments(text):
  """Strips comments out of JavaScript source code.

  Args:
    text: JavaScript source text.

  Returns:
    JavaScript source text with comments stripped out.
  """
  result_tokens = []
  token_stream = _TokenizeJS(text).__iter__()
  while True:
    try:
      t = next(token_stream)
    except StopIteration:
      break

    if t == '//':
      while True:
        try:
          t2 = next(token_stream)
          if t2 == '\n':
            break
        except StopIteration:
          break
    elif t == '/*':
      nesting = 1
      while True:
        try:
          t2 = next(token_stream)
          if t2 == '/*':
            nesting += 1
          elif t2 == '*/':
            nesting -= 1
            if nesting == 0:
              break
        except StopIteration:
          break
    else:
      result_tokens.append(t)
  return ''.join(result_tokens)
