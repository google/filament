# Copyright 2014 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from recipe_engine import recipe_test_api


class GerritTestApi(recipe_test_api.RecipeTestApi):

  @staticmethod
  def gerrit_change_data(change_number=91827, patchset=1, **kwargs):
    # Exemplary change. Note: This contains only a subset of the key/value pairs
    # present in production to limit recipe simulation output.
    data = {
        'id': 'fully~qualified~changeid',
        'status': 'NEW',
        'created': '2017-01-30 13:11:20.000000000',
        '_number': str(change_number),
        'change_id': 'Ideadbeef',
        'project': 'chromium/src',
        'has_review_started': False,
        'branch': 'main',
        'subject': 'Change title',
        'revisions': {
            '184ebe53805e102605d11f6b143486d15c23a09c': {
                '_number': str(patchset),
                'commit': {
                    'message': 'Change commit message',
                },
            },
        },
    }
    data.update(kwargs)
    return data

  @staticmethod
  def _related_changes_data(**kwargs):
    data = {
        "changes": [{
            "project": "gerrit",
            "change_id": "Ic62ae3103fca2214904dbf2faf4c861b5f0ae9b5",
            "commit": {
                "commit": "78847477532e386f5a2185a4e8c90b2509e354e3",
                "parents": [{
                    "commit": "bb499510bbcdbc9164d96b0dbabb4aa45f59a87e"
                }],
                "author": {
                    "name": "Example Name",
                    "email": "example@example.com",
                    "date": "2014-07-12 15:04:24.000000000",
                    "tz": 120
                },
                "subject": "Remove Solr"
            },
            "_change_number": 58478,
            "_revision_number": 2,
            "_current_revision_number": 2,
            "status": "NEW"
        }]
    }
    data.update(kwargs)
    return data

  def _make_gerrit_response_json(self, data):
    return self.m.json.output(data)

  def make_gerrit_create_branch_response_data(self):
    return self._make_gerrit_response_json({
      "ref": "refs/heads/test",
      "revision": "76016386a0d8ecc7b6be212424978bb45959d668",
      "can_delete": True
    })

  def make_gerrit_create_tag_response_data(self):
    return self._make_gerrit_response_json({
      "ref": "refs/tags/1.0",
      "revision": "67ebf73496383c6777035e374d2d664009e2aa5c",
      "can_delete": True
    })

  def make_gerrit_get_branch_response_data(self):
    return self._make_gerrit_response_json({
      "ref": "refs/heads/main",
      "revision": "67ebf73496383c6777035e374d2d664009e2aa5c"
    })

  def get_one_change_response_data(self, **kwargs):
    return self.get_multiple_changes_response_data([self.gerrit_change_data(**kwargs)])

  def get_multiple_changes_response_data(self, changes):
    return self._make_gerrit_response_json(changes)

  def update_files_response_data(self, **kwargs):
    return self._make_gerrit_response_json(self.gerrit_change_data(**kwargs))

  def get_empty_changes_response_data(self):
    return self._make_gerrit_response_json([])

  def get_move_change_response_data(self, **kwargs):
    return self._make_gerrit_response_json([self.gerrit_change_data(**kwargs)])

  def get_related_changes_response_data(self, **kwargs):
    return self._make_gerrit_response_json(self._related_changes_data(**kwargs))
