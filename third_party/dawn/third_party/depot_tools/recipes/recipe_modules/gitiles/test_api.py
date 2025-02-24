# Copyright 2014 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import base64
import hashlib

from recipe_engine import recipe_test_api


class GitilesTestApi(recipe_test_api.RecipeTestApi):

  def _make_gitiles_response_json(self, data):
    return self.m.json.output(data)

  def make_refs_test_data(self, *refs):
    return self._make_gitiles_response_json({ref: None for ref in refs})

  def make_log_test_data(self, s, n=3, cursor=None):
    result = {
        'log': [
            self.make_commit_gitiles_dict(
                commit='fake %s hash %d' % (s, i),
                msg='fake %s msg %d' % (s, i),
                new_files=['%s.py' % (chr(i + ord('a')))],
                email='fake_%s@fake_%i.email.com' % (s, i),
            )
            for i in range(n)
        ],
    }
    if cursor:
      result['next'] = cursor
    return self._make_gitiles_response_json(result)

  def make_commit_test_data(self, commit, msg, new_files=None, email=None):
    """Constructs fake Gitiles commit JSON test output.

    This data structure conforms to the JSON response that Gitiles provides when
    a commit is queried. For example:
    https://chromium.googlesource.com/chromium/src/+/875b896a3256c5b86c8725e81489e99ea6c2b4c9?format=json

    Args:
      commit (str): The fake commit hash.
      msg (str): The commit message.
      new_files (list): If not None, a list of filenames (str) to simulate being
          added in this commit.
      email: if not None, a proper email with '@' in it to be used for
          committer's and author's emails.
    Returns: (raw_io.Output) A simulated Gitiles fetch 'json' output.
    """
    commit = self.make_commit_gitiles_dict(commit, msg, new_files, email)
    return self._make_gitiles_response_json(commit)

  def make_commit_gitiles_dict(self, commit, msg, new_files, email=None):
    if email is None:
      name = 'Test Author'
      email = 'testauthor@fake.chromium.org'
    else:
      assert '@' in email
      name = email.split('@')[0]
    d = {
        'commit': self.make_hash(commit),
        'tree': self.make_hash('tree', commit),
        'parents': [self.make_hash('parent', commit)],
        'author': {
            'name': name,
            'email': email,
            'time': 'Mon Jan 01 00:00:00 2015',
        },
        'committer': {
            'name': name,
            'email': email,
            'time': 'Mon Jan 01 00:00:00 2015',
        },
        'message': msg,
        'tree_diff': [],
    }
    if new_files:
      d['tree_diff'].extend({
          'type': 'add',
          'old_id': 40 * '0',
          'old_mode': 0,
          'new_id': self.make_hash('file', f, commit),
          'new_mode': 33188,
          'new_path': f,
      } for f in new_files)
    return d

  def make_hash(self, *bases):
    return hashlib.sha1(':'.join(bases).encode('utf-8')).hexdigest()

  def make_encoded_file(self, data):
    """Encodes data into base64.

    Args:
      data (str): unicode-encodable string.
    Returns: (str) base64-encoded data string.
    """
    return self.m.json.output({
        'value': base64.b64encode(data.encode('utf-8')).decode('utf-8'),
    })

  def make_encoded_file_from_bytes(self, data):
    """Encodes data into base64.

    Args:
      data (bytes): byte string to encode.
    Returns: (str) base64-encoded data string.
    """
    return self.m.json.output({
        'value': base64.b64encode(data).decode('utf-8'),
    })