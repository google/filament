# Copyright 2017 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

"""Functions to use the drive APIs to manipulate spreadsheets and folders."""

# Pylint warnings occur if you install a different version than in third_party
# pylint: disable=no-member

from __future__ import absolute_import
from __future__ import print_function
import time

# pylint: disable=import-error
import httplib2
from apiclient import discovery  # pylint: disable=import-error
from oauth2client import service_account  # pylint: disable=no-name-in-module
# pylint: enable=import-error

# Update this to the location you downloaded the keyfile to.
# See https://developers.google.com/sheets/api/quickstart/python
_PATH_TO_JSON_KEYFILE = 'PATH_TO/keyfile.json'


def ReadSpreadsheet(spreadsheet_id, range_name):
  """Returns the values in the given range.

  Args:
    spreadsheet_id: The spreadsheet id, which can be found in the url.
    range_name: A string range, such as 'A2:D9'

  Returns: A 2D list of values in the given range.
  """
  sheets_service = _GetSheetsService()
  result = list(sheets_service.spreadsheets().values()).get(
      spreadsheetId=spreadsheet_id, range=range_name).execute()
  return result.get('values', [])


def CreateSpreadsheet(title, sheets, folder_id):
  """Creates the given spreadsheet in the given folder.

  Args:
    title: The title of the file to be created.
    sheets: A list of dicts containing:
      'name': The name of the tab for the sheet
      'values': A 2D list of data to fill in
    folder_id: The id of the folder to put the spreadsheet in, from CreateFolder

  Returns: The url of the created spreadsheet.
  """
  file_info = CreateEmptyFile(
      title, 'application/vnd.google-apps.spreadsheet', folder_id)
  spreadsheet_id = file_info['id']
  spreadsheet_url = _AddSheetsToSpreadsheet(sheets, spreadsheet_id)
  print('Created Spreadsheet %s' % spreadsheet_url)
  # Limit of 100 actions in 100 seconds; sleep 1s between sheets.
  time.sleep(10)
  return spreadsheet_url

def _AddSheetsToSpreadsheet(sheets, spreadsheet_id):
  # Batch and execute requests to add empty sheets with the correct names.
  requests = []
  for sheet in sheets:
    requests.append({
        'addSheet': {'properties': {'title': sheet['name']}}
    })
  sheets_service = _GetSheetsService()
  sheets_service.spreadsheets().batchUpdate(
      spreadsheetId=spreadsheet_id, body={'requests': requests}).execute()
  sheet_metadata = sheets_service.spreadsheets().get(
      spreadsheetId=spreadsheet_id).execute()

  sheet_ids = dict((
      s['properties']['title'],
      s['properties']['sheetId']) for s in sheet_metadata['sheets'])

  # Delete placeholder 'Sheet1'
  requests = [{
      'deleteSheet': {
          'sheetId': sheet_ids['Sheet1']
      }
  }]
  sheets_service.spreadsheets().batchUpdate(
      spreadsheetId=spreadsheet_id, body={'requests': requests}).execute()

  # Add the correct data to the new sheets.
  for sheet in sheets:
    # Update values
    r = '%s!A1' % sheet['name']
    list(sheets_service.spreadsheets().values()).update(
        spreadsheetId=spreadsheet_id,
        body={'values': sheet['values']},
        range=r,
        valueInputOption='USER_ENTERED').execute()

    # Batch and execute requests to set the formatting on the new sheets
    # (one frozen bolded row at the top for a header).
    requests = [{
        'repeatCell': {
            'range': {
                'sheetId': sheet_ids[sheet['name']],
                'startRowIndex': 0,
                'endRowIndex': 1
            },
            'cell': {
                'userEnteredFormat': {
                    'textFormat': {
                        'bold': True
                    }
                }
            },
            'fields': 'userEnteredFormat(textFormat)'
        }
    }, {
        'updateSheetProperties': {
            'properties': {
                'sheetId': sheet_ids[sheet['name']],
                'gridProperties': {
                    'frozenRowCount': 1
                }
            },
            'fields': 'gridProperties.frozenRowCount'
        }
    }]
    sheets_service.spreadsheets().batchUpdate(
        spreadsheetId=spreadsheet_id, body={'requests': requests}).execute()
  return sheet_metadata['spreadsheetUrl']


def CreateEmptyFile(name, mime_type, folder_id):
  """Creates an empty drive file and returns the result."""
  drive_service = _GetDriveService()
  file_metadata = {
      'name': name,
      'mimeType': mime_type,
      'parents': [folder_id],
  }
  print('Creating file: %s' % file_metadata)
  return drive_service.files().create(body=file_metadata).execute()


def CreateFolder(title):
  """Creates a drive folder with the given title."""
  drive_service = _GetDriveService()
  folder_metadata = {
      'name': title,
      'mimeType': 'application/vnd.google-apps.folder'
  }
  folder = drive_service.files().create(
      body=folder_metadata, fields='id').execute()
  folder_id = folder.get('id')
  # TODO(sullivan): What should permissions look like?
  user_permission = {
      'type': 'user',
      'role': 'writer',
      'emailAddress': 'sullivan@google.com'
  }
  drive_service.permissions().create(
      fileId=folder_id, body=user_permission, fields='id').execute()
  return folder_id

def _GetSheetsService():
  scopes = ['https://www.googleapis.com/auth/spreadsheets']
  return _BuildDiscoveryService(
      scopes, 'sheets', 'v4',
      'https://sheets.googleapis.com/$discovery/rest?version=v4')

def _GetDriveService():
  scopes = ['https://www.googleapis.com/auth/drive']
  return _BuildDiscoveryService(scopes, 'drive', 'v3', None)


def _BuildDiscoveryService(scopes, api, version, url):
  """Helper functions to build discovery services for sheets and drive."""
  creds = service_account.ServiceAccountCredentials.from_json_keyfile_name(
      _PATH_TO_JSON_KEYFILE, scopes)
  http = creds.authorize(httplib2.Http())
  if url:
    return discovery.build(api, version, http=http, discoveryServiceUrl=url)
  return discovery.build(api, version, http=http)
