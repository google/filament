# Copyright 2017 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
"""An interface to the Google Spreadsheets API.

API documentation: https://developers.google.com/sheets/api/reference/rest/

This service uses the default application credentials, so it can only access
public spreadsheets.
"""
from __future__ import print_function
from __future__ import division
from __future__ import absolute_import

import logging

from apiclient import discovery
from dashboard.common import oauth2_utils

DISCOVERY_URL = 'https://sheets.googleapis.com/$discovery/rest?version=v4'


def GetRange(spreadsheet_id, sheet_name, range_in_sheet):
  """Gets the given range in the given spreadsheet.

  Args:
    spreadsheet_id: The id from Google Sheets, like
      https://docs.google.com/spreadsheets/d/<THIS PART>/
    sheet_name: The name of the sheet to get, from the bottom tab.
    range_in_sheet: The range, such as "A1:F14"
  """
  credentials = oauth2_utils.GetAppDefaultCredentials()
  service = discovery.build(
      'sheets',
      'v4',
      credentials=credentials,
      discoveryServiceUrl=DISCOVERY_URL)
  sheet_range = '%s!%s' % (sheet_name, range_in_sheet)
  result = list(service.spreadsheets().values()).get(
      spreadsheetId=spreadsheet_id, range=sheet_range).execute()
  values = result.get('values', [])
  if not values:
    # Error reporting is not spectacular. Looks like values will just be None.
    # But they could be None if there wasn't any data, either. So log it
    # and still return the None value.
    logging.error('Could not get values for %s of %s', sheet_range, sheet_name)
  return values
