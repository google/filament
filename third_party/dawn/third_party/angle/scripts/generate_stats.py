#!/usr/bin/env vpython
#
# [VPYTHON:BEGIN]
# wheel: <
#   name: "infra/python/wheels/google-auth-py2_py3"
#   version: "version:1.2.1"
# >
#
# wheel: <
#   name: "infra/python/wheels/pyasn1-py2_py3"
#   version: "version:0.4.5"
# >
#
# wheel: <
#   name: "infra/python/wheels/pyasn1_modules-py2_py3"
#   version: "version:0.2.4"
# >
#
# wheel: <
#   name: "infra/python/wheels/six"
#   version: "version:1.10.0"
# >
#
# wheel: <
#   name: "infra/python/wheels/cachetools-py2_py3"
#   version: "version:2.0.1"
# >
# wheel: <
#   name: "infra/python/wheels/rsa-py2_py3"
#   version: "version:4.0"
# >
#
# wheel: <
#   name: "infra/python/wheels/requests"
#   version: "version:2.13.0"
# >
#
# wheel: <
#   name: "infra/python/wheels/google-api-python-client-py2_py3"
#   version: "version:1.6.2"
# >
#
# wheel: <
#   name: "infra/python/wheels/httplib2-py2_py3"
#   version: "version:0.12.1"
# >
#
# wheel: <
#   name: "infra/python/wheels/oauth2client-py2_py3"
#   version: "version:3.0.0"
# >
#
# wheel: <
#   name: "infra/python/wheels/uritemplate-py2_py3"
#   version: "version:3.0.0"
# >
#
# wheel: <
#   name: "infra/python/wheels/google-auth-oauthlib-py2_py3"
#   version: "version:0.3.0"
# >
#
# wheel: <
#   name: "infra/python/wheels/requests-oauthlib-py2_py3"
#   version: "version:1.2.0"
# >
#
# wheel: <
#   name: "infra/python/wheels/oauthlib-py2_py3"
#   version: "version:3.0.1"
# >
#
# wheel: <
#   name: "infra/python/wheels/google-auth-httplib2-py2_py3"
#   version: "version:0.0.3"
# >
# [VPYTHON:END]
#
# Copyright 2019 The ANGLE Project Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
#
# generate_deqp_stats.py:
#   Checks output of deqp testers and generates stats using the GDocs API
#
# prerequirements:
#   https://devsite.googleplex.com/sheets/api/quickstart/python
#   Follow the quickstart guide.
#
# usage: generate_deqp_stats.py [-h] [--auth_path [AUTH_PATH]] [--spreadsheet [SPREADSHEET]]
#                               [--verbosity [VERBOSITY]]
#
# optional arguments:
#   -h, --help            show this help message and exit
#   --auth_path [AUTH_PATH]
#                         path to directory containing authorization data (credentials.json and
#                         token.pickle). [default=<home>/.auth]
#   --spreadsheet [SPREADSHEET]
#                         ID of the spreadsheet to write stats to. [default
#                         ='1D6Yh7dAPP-aYLbX3HHQD8WubJV9XPuxvkKowmn2qhIw']
#   --verbosity [VERBOSITY]
#                         Verbosity of output. Valid options are [DEBUG, INFO, WARNING, ERROR].
#                         [default=INFO]

import argparse
import datetime
import logging
import os
import pickle
import re
import subprocess
import sys
import urllib
from google.auth.transport.requests import Request
from googleapiclient.discovery import build
from google_auth_oauthlib.flow import InstalledAppFlow

####################
# Global Constants #
####################

HOME_DIR = os.path.expanduser('~')
SCRIPT_DIR = sys.path[0]
ROOT_DIR = os.path.abspath(os.path.join(SCRIPT_DIR, '..'))

LOGGER = logging.getLogger('generate_stats')

SCOPES = ['https://www.googleapis.com/auth/spreadsheets']

BOT_NAMES = [
    'mac-angle-amd',
    'mac-angle-intel',
    'win10-angle-x64-nvidia',
    'win10-angle-x64-intel',
    'win7-angle-x64-nvidia',
    'win7-angle-x86-amd',
    'Linux FYI dEQP Release (Intel HD 630)',
    'Linux FYI dEQP Release (NVIDIA)',
    'Android FYI dEQP Release (Nexus 5X)',
    'Android FYI 32 dEQP Vk Release (Pixel 2)',
    'Android FYI 64 dEQP Vk Release (Pixel 2)',
]
BOT_NAME_PREFIX = 'chromium/ci/'
BUILD_LINK_PREFIX = 'https://ci.chromium.org/p/chromium/builders/ci/'

REQUIRED_COLUMNS = ['build_link', 'time', 'date', 'revision', 'angle_revision', 'duplicate']
MAIN_RESULT_COLUMNS = ['Passed', 'Failed', 'Skipped', 'Not Supported', 'Exception', 'Crashed']

INFO_TAG = '*RESULT'

WORKAROUND_FORMATTING_ERROR_STRING = "Still waiting for the following processes to finish:"

######################
# Build Info Parsing #
######################


# Returns a struct with info about the latest successful build given a bot name. Info contains the
# build_name, time, date, angle_revision, and chrome revision.
# Uses: bb ls '<botname>' -n 1 -status success -p
def get_latest_success_build_info(bot_name):
    bb = subprocess.Popen(['bb', 'ls', bot_name, '-n', '1', '-status', 'success', '-p'],
                          stdout=subprocess.PIPE,
                          stderr=subprocess.PIPE)
    LOGGER.debug("Ran [bb ls '" + bot_name + "' -n 1 -status success -p]")
    out, err = bb.communicate()
    if err:
        raise ValueError("Unexpected error from bb ls: '" + err + "'")
    if not out:
        raise ValueError("Unexpected empty result from bb ls of bot '" + bot_name + "'")
    # Example output (line 1):
    # ci.chromium.org/b/8915280275579996928 SUCCESS   'chromium/ci/Win10 FYI dEQP Release (NVIDIA)/26877'
    # ...
    if 'SUCCESS' not in out:
        raise ValueError("Unexpected result from bb ls: '" + out + "'")
    info = {}
    for line in out.splitlines():
        # The first line holds the build name
        if 'build_name' not in info:
            info['build_name'] = line.strip().split("'")[1]
            # Remove the bot name and prepend the build link
            info['build_link'] = BUILD_LINK_PREFIX + urllib.quote(
                info['build_name'].split(BOT_NAME_PREFIX)[1])
        if 'Created' in line:
            # Example output of line with 'Created':
            # ...
            # Created today at 12:26:39, waited 2.056319s, started at 12:26:41, ran for 1h16m48.14963s, ended at 13:43:30
            # ...
            info['time'] = re.findall(r'[0-9]{1,2}:[0-9]{2}:[0-9]{2}', line.split(',', 1)[0])[0]
            # Format today's date in US format so Sheets can read it properly
            info['date'] = datetime.datetime.now().strftime('%m/%d/%y')
        if 'got_angle_revision' in line:
            # Example output of line with angle revision:
            # ...
            #   "parent_got_angle_revision": "8cbd321cafa92ffbf0495e6d0aeb9e1a97940fee",
            # ...
            info['angle_revision'] = filter(str.isalnum, line.split(':')[1])
        if '"revision"' in line:
            # Example output of line with chromium revision:
            # ...
            #   "revision": "3b68405a27f1f9590f83ae07757589dba862f141",
            # ...
            info['revision'] = filter(str.isalnum, line.split(':')[1])
    if 'build_name' not in info:
        raise ValueError("Could not find build_name from bot '" + bot_name + "'")
    return info


# Returns a list of step names that we're interested in given a build name. We are interested in
# step names starting with 'angle_'. May raise an exception.
# Uses: bb get '<build_name>' -steps
def get_step_names(build_name):
    bb = subprocess.Popen(['bb', 'get', build_name, '-steps'],
                          stdout=subprocess.PIPE,
                          stderr=subprocess.PIPE)
    LOGGER.debug("Ran [bb get '" + build_name + "' -steps]")
    out, err = bb.communicate()
    if err:
        raise ValueError("Unexpected error from bb get: '" + err + "'")
    step_names = []
    # Example output (relevant lines to a single step):
    # ...
    # Step "angle_deqp_egl_vulkan_tests on (nvidia-quadro-p400-win10-stable) GPU on Windows on Windows-10"                                      SUCCESS   4m12s     Logs: "stdout", "chromium_swarming.summary", "Merge script log", "Flaky failure: dEQP.EGL&#x2f;info_version (status CRASH,SUCCESS)", "step_metadata"
    # Run on OS: 'Windows-10'<br>Max shard duration: 0:04:07.309848 (shard \#1)<br>Min shard duration: 0:02:26.402128 (shard \#0)<br/>flaky failures [ignored]:<br/>dEQP.EGL/info\_version<br/>
    #  * [shard #0 isolated out](https://isolateserver.appspot.com/browse?namespace=default-gzip&hash=9a5999a59d332e55f54f495948d0c9f959e60ed2)
    #  * [shard #0 (128.3 sec)](https://chromium-swarm.appspot.com/user/task/446903ae365b8110)
    #  * [shard #1 isolated out](https://isolateserver.appspot.com/browse?namespace=default-gzip&hash=d71e1bdd91dee61b536b4057a9222e642bd3809f)
    #  * [shard #1 (229.3 sec)](https://chromium-swarm.appspot.com/user/task/446903b7b0d90210)
    #  * [shard #2 isolated out](https://isolateserver.appspot.com/browse?namespace=default-gzip&hash=ac9ba85b1cca77774061b87335c077980e1eef85)
    #  * [shard #2 (144.5 sec)](https://chromium-swarm.appspot.com/user/task/446903c18e15a010)
    #  * [shard #3 isolated out](https://isolateserver.appspot.com/browse?namespace=default-gzip&hash=976d586386864abecf53915fbac3e085f672e30f)
    #  * [shard #3 (138.4 sec)](https://chromium-swarm.appspot.com/user/task/446903cc8da0ad10)
    # ...
    for line in out.splitlines():
        if 'Step "angle_' not in line:
            continue
        step_names.append(line.split('"')[1])
    return step_names


# Performs some heuristic validation of the step_info struct returned from a single step log.
# Returns True if valid, False if invalid. May write to stderr
def validate_step_info(step_info, build_name, step_name):
    print_name = "'" + build_name + "': '" + step_name + "'"
    if not step_info:
        LOGGER.warning('Step info empty for ' + print_name + '\n')
        return False

    if 'Total' in step_info:
        partial_sum_keys = MAIN_RESULT_COLUMNS
        partial_sum_values = [int(step_info[key]) for key in partial_sum_keys if key in step_info]
        computed_total = sum(partial_sum_values)
        if step_info['Total'] != computed_total:
            LOGGER.warning('Step info does not sum to total for ' + print_name + ' | Total: ' +
                           str(step_info['Total']) + ' - Computed total: ' + str(computed_total) +
                           '\n')
    return True


# Returns a struct containing parsed info from a given step log. The info is parsed by looking for
# lines with the following format in stdout:
# '[TESTSTATS]: <key>: <value>''
# May write to stderr
# Uses: bb log '<build_name>' '<step_name>'
def get_step_info(build_name, step_name):
    bb = subprocess.Popen(['bb', 'log', build_name, step_name],
                          stdout=subprocess.PIPE,
                          stderr=subprocess.PIPE)
    LOGGER.debug("Ran [bb log '" + build_name + "' '" + step_name + "']")
    out, err = bb.communicate()
    if err:
        LOGGER.warning("Unexpected error from bb log '" + build_name + "' '" + step_name + "': '" +
                       err + "'")
        return None
    step_info = {}
    # Example output (relevant lines of stdout):
    # ...
    # *RESULT: Total: 155
    # *RESULT: Passed: 11
    # *RESULT: Failed: 0
    # *RESULT: Skipped: 12
    # *RESULT: Not Supported: 132
    # *RESULT: Exception: 0
    # *RESULT: Crashed: 0
    # *RESULT: Unexpected Passed: 12
    # ...
    append_errors = []
    # Hacky workaround to fix issue where messages are dropped into the middle of lines by another
    # process:
    # eg.
    # *RESULT: <start_of_result>Still waiting for the following processes to finish:
    # "c:\b\s\w\ir\out\Release\angle_deqp_gles3_tests.exe" --deqp-egl-display-type=angle-vulkan --gtest_flagfile="c:\b\s\w\itlcgdrz\scoped_dir7104_364984996\8ad93729-f679-406d-973b-06b9d1bf32de.tmp" --single-process-tests --test-launcher-batch-limit=400 --test-launcher-output="c:\b\s\w\itlcgdrz\7104_437216092\test_results.xml" --test-launcher-summary-output="c:\b\s\w\iosuk8ai\output.json"
    # <end_of_result>
    #
    # Removes the message and skips the line following it, and then appends the <start_of_result>
    # and <end_of_result> back together
    workaround_prev_line = ""
    workaround_prev_line_count = 0
    for line in out.splitlines():
        # Skip lines if the workaround still has lines to skip
        if workaround_prev_line_count > 0:
            workaround_prev_line_count -= 1
            continue
        # If there are no more lines to skip and there is a previous <start_of_result> to append,
        # append it and finish the workaround
        elif workaround_prev_line != "":
            line = workaround_prev_line + line
            workaround_prev_line = ""
            workaround_prev_line_count = 0
            LOGGER.debug("Formatting error workaround rebuilt line as: '" + line + "'\n")

        if INFO_TAG not in line:
            continue

        # When the workaround string is detected, start the workaround with 1 line to skip and save
        # the <start_of_result>, but continue the loop until the workaround is finished
        if WORKAROUND_FORMATTING_ERROR_STRING in line:
            workaround_prev_line = line.split(WORKAROUND_FORMATTING_ERROR_STRING)[0]
            workaround_prev_line_count = 1
            continue

        found_stat = True
        line_columns = line.split(INFO_TAG, 1)[1].split(':')
        if len(line_columns) is not 3:
            LOGGER.warning("Line improperly formatted: '" + line + "'\n")
            continue
        key = line_columns[1].strip()
        # If the value is clearly an int, sum it. Otherwise, concatenate it as a string
        isInt = False
        intVal = 0
        try:
            intVal = int(line_columns[2])
            if intVal is not None:
                isInt = True
        except Exception as error:
            isInt = False

        if isInt:
            if key not in step_info:
                step_info[key] = 0
            step_info[key] += intVal
        else:
            if key not in step_info:
                step_info[key] = line_columns[2].strip()
            else:
                append_string = '\n' + line_columns[2].strip()
                # Sheets has a limit of 50000 characters per cell, so make sure to stop appending
                # below this limit
                if len(step_info[key]) + len(append_string) < 50000:
                    step_info[key] += append_string
                else:
                    if key not in append_errors:
                        append_errors.append(key)
                        LOGGER.warning("Too many characters in column '" + key +
                                       "'. Output capped.")
    return step_info


# Returns the info for each step run on a given bot_name.
def get_bot_info(bot_name):
    info = get_latest_success_build_info(bot_name)
    info['step_names'] = get_step_names(info['build_name'])
    broken_step_names = []
    for step_name in info['step_names']:
        LOGGER.info("Parsing step '" + step_name + "'...")
        step_info = get_step_info(info['build_name'], step_name)
        if validate_step_info(step_info, info['build_name'], step_name):
            info[step_name] = step_info
        else:
            broken_step_names += step_name
    for step_name in broken_step_names:
        info['step_names'].remove(step_name)
    return info


#####################
# Sheets Formatting #
#####################


# Get an individual spreadsheet based on the spreadsheet id. Returns the result of
# spreadsheets.get(), or throws an exception if the sheet could not open.
def get_spreadsheet(service, spreadsheet_id):
    LOGGER.debug("Called [spreadsheets.get(spreadsheetId='" + spreadsheet_id + "')]")
    request = service.get(spreadsheetId=spreadsheet_id)
    spreadsheet = request.execute()
    if not spreadsheet:
        raise Exception("Did not open spreadsheet '" + spreadsheet_id + "'")
    return spreadsheet


# Returns a nicely formatted string based on the bot_name and step_name
def format_sheet_name(bot_name, step_name):
    # Some tokens should be ignored for readability in the name
    unneccesary_tokens = ['FYI', 'Release', 'Vk', 'dEQP', '(', ')']
    for token in unneccesary_tokens:
        bot_name = bot_name.replace(token, '')
    bot_name = ' '.join(bot_name.strip().split())  # Remove extra spaces
    step_name = re.findall(r'angle\w*', step_name)[0]  # Separate test name
    # Test names are formatted as 'angle_deqp_<frontend>_<backend>_tests'
    new_step_name = ''
    # Put the frontend first
    if '_egl_' in step_name:
        step_name = step_name.replace('_egl_', '_')
        new_step_name += ' EGL'
    if '_gles2_' in step_name:
        step_name = step_name.replace('_gles2_', '_')
        new_step_name += ' GLES 2.0 '
    if '_gles3_' in step_name:
        step_name = step_name.replace('_gles3_', '_')
        new_step_name += ' GLES 3.0 '
    if '_gles31_' in step_name:
        step_name = step_name.replace('_gles31_', '_')
        new_step_name += ' GLES 3.1 '
    # Put the backend second
    if '_d3d9_' in step_name:
        step_name = step_name.replace('_d3d9_', '_')
        new_step_name += ' D3D9 '
    if '_d3d11' in step_name:
        step_name = step_name.replace('_d3d11_', '_')
        new_step_name += ' D3D11 '
    if '_gl_' in step_name:
        step_name = step_name.replace('_gl_', '_')
        new_step_name += ' Desktop OpenGL '
    if '_gles_' in step_name:
        step_name = step_name.replace('_gles_', '_')
        new_step_name += ' OpenGLES '
    if '_vulkan_' in step_name:
        step_name = step_name.replace('_vulkan_', '_')
        new_step_name += ' Vulkan '
    # Add any remaining keywords from the step name into the formatted name (formatted nicely)
    step_name = step_name.replace('angle_', '_')
    step_name = step_name.replace('_deqp_', '_')
    step_name = step_name.replace('_tests', '_')
    step_name = step_name.replace('_', ' ').strip()
    new_step_name += ' ' + step_name
    new_step_name = ' '.join(new_step_name.strip().split())  # Remove extra spaces
    return new_step_name + ' ' + bot_name


# Returns the full list of sheet names that should be populated based on the info struct
def get_sheet_names(info):
    sheet_names = []
    for bot_name in info:
        for step_name in info[bot_name]['step_names']:
            sheet_name = format_sheet_name(bot_name, step_name)
            sheet_names.append(sheet_name)
    return sheet_names


# Returns True if the sheet is found in the spreadsheets object
def sheet_exists(spreadsheet, step_name):
    for sheet in spreadsheet['sheets']:
        if sheet['properties']['title'] == step_name:
            return True
    return False


# Validates the spreadsheets object against the list of sheet names which should appear. Returns a
# list of sheets that need creation.
def validate_sheets(spreadsheet, sheet_names):
    create_sheets = []
    for sheet_name in sheet_names:
        if not sheet_exists(spreadsheet, sheet_name):
            create_sheets.append(sheet_name)
    return create_sheets


# Performs a batch update with a given service, spreadsheet id, and list <object(Request)> of
# updates to do.
def batch_update(service, spreadsheet_id, updates):
    batch_update_request_body = {
        'requests': updates,
    }
    LOGGER.debug("Called [spreadsheets.batchUpdate(spreadsheetId='" + spreadsheet_id + "', body=" +
                 str(batch_update_request_body) + ')]')
    request = service.batchUpdate(spreadsheetId=spreadsheet_id, body=batch_update_request_body)
    request.execute()


# Creates sheets given a service and spreadsheed id based on a list of sheet names input
def create_sheets(service, spreadsheet_id, sheet_names):
    updates = [{'addSheet': {'properties': {'title': sheet_name,}}} for sheet_name in sheet_names]
    batch_update(service, spreadsheet_id, updates)


# Calls a values().batchGet() on the service to find the list of column names from each sheet in
# sheet_names. Returns a dictionary with one list per sheet_name.
def get_headers(service, spreadsheet_id, sheet_names):
    header_ranges = [sheet_name + '!A1:Z' for sheet_name in sheet_names]
    LOGGER.debug("Called [spreadsheets.values().batchGet(spreadsheetId='" + spreadsheet_id +
                 ', ranges=' + str(header_ranges) + "')]")
    request = service.values().batchGet(spreadsheetId=spreadsheet_id, ranges=header_ranges)
    response = request.execute()
    headers = {}
    for k, sheet_name in enumerate(sheet_names):
        if 'values' in response['valueRanges'][k]:
            # Headers are in the first row of values
            headers[sheet_name] = response['valueRanges'][k]['values'][0]
        else:
            headers[sheet_name] = []
    return headers


# Calls values().batchUpdate() with supplied list of data <object(ValueRange)> to update on the
# service.
def batch_update_values(service, spreadsheet_id, data):
    batch_update_values_request_body = {
        'valueInputOption': 'USER_ENTERED',  # Helps with formatting of dates
        'data': data,
    }
    LOGGER.debug("Called [spreadsheets.values().batchUpdate(spreadsheetId='" + spreadsheet_id +
                 "', body=" + str(batch_update_values_request_body) + ')]')
    request = service.values().batchUpdate(
        spreadsheetId=spreadsheet_id, body=batch_update_values_request_body)
    request.execute()


# Get the sheetId of a sheet based on its name
def get_sheet_id(spreadsheet, sheet_name):
    for sheet in spreadsheet['sheets']:
        if sheet['properties']['title'] == sheet_name:
            return sheet['properties']['sheetId']
    return -1


# Update the filters on sheets with a 'duplicate' column. Filter out any duplicate rows
def update_filters(service, spreadsheet_id, headers, info, spreadsheet):
    updates = []
    for bot_name in info:
        for step_name in info[bot_name]['step_names']:
            sheet_name = format_sheet_name(bot_name, step_name)
            duplicate_found = 'duplicate' in headers[sheet_name]
            if duplicate_found:
                sheet_id = get_sheet_id(spreadsheet, sheet_name)
                if sheet_id > -1:
                    updates.append({
                        "setBasicFilter": {
                            "filter": {
                                "range": {
                                    "sheetId": sheet_id,
                                    "startColumnIndex": 0,
                                    "endColumnIndex": len(headers[sheet_name])
                                },
                                "sortSpecs": [{
                                    "dimensionIndex": headers[sheet_name].index('date'),
                                    "sortOrder": "ASCENDING"
                                }],
                                "criteria": {
                                    str(headers[sheet_name].index('duplicate')): {
                                        "hiddenValues":
                                            ["1"]  # Hide rows when duplicate is 1 (true)
                                    }
                                }
                            }
                        }
                    })
    if updates:
        LOGGER.info('Updating sheet filters...')
        batch_update(service, spreadsheet_id, updates)

# Populates the headers with any missing/desired rows based on the info struct, and calls
# batch update to update the corresponding sheets if necessary.
def update_headers(service, spreadsheet_id, headers, info):
    data = []
    sheet_names = []
    for bot_name in info:
        for step_name in info[bot_name]['step_names']:
            if not step_name in info[bot_name]:
                LOGGER.error("Missing info for step name: '" + step_name + "'")
            sheet_name = format_sheet_name(bot_name, step_name)
            headers_stale = False
            # Headers should always contain the following columns
            for req in REQUIRED_COLUMNS:
                if req not in headers[sheet_name]:
                    headers_stale = True
                    headers[sheet_name].append(req)
            # Headers also must contain all the keys seen in this step
            for key in info[bot_name][step_name].keys():
                if key not in headers[sheet_name]:
                    headers_stale = True
                    headers[sheet_name].append(key)
            # Update the Gdoc headers if necessary
            if headers_stale:
                sheet_names.append(sheet_name)
                header_range = sheet_name + '!A1:Z'
                data.append({
                    'range': header_range,
                    'majorDimension': 'ROWS',
                    'values': [headers[sheet_name]]
                })
    if data:
        LOGGER.info('Updating sheet headers...')
        batch_update_values(service, spreadsheet_id, data)

# Calls values().append() to append a list of values to a given sheet.
def append_values(service, spreadsheet_id, sheet_name, values):
    header_range = sheet_name + '!A1:Z'
    insert_data_option = 'INSERT_ROWS'
    value_input_option = 'USER_ENTERED'  # Helps with formatting of dates
    append_values_request_body = {
        'range': header_range,
        'majorDimension': 'ROWS',
        'values': [values],
    }
    LOGGER.debug("Called [spreadsheets.values().append(spreadsheetId='" + spreadsheet_id +
                 "', body=" + str(append_values_request_body) + ", range='" + header_range +
                 "', insertDataOption='" + insert_data_option + "', valueInputOption='" +
                 value_input_option + "')]")
    request = service.values().append(
        spreadsheetId=spreadsheet_id,
        body=append_values_request_body,
        range=header_range,
        insertDataOption=insert_data_option,
        valueInputOption=value_input_option)
    request.execute()


# Formula to determine whether a row is a duplicate of the previous row based on checking the
# columns listed in filter_columns.
# Eg.
# date | pass | fail
# Jan 1  100    50
# Jan 2  100    50
# Jan 3  99     51
#
# If we want to filter based on only the "pass" and "fail" columns, we generate the following
# formula in the 'duplicate' column: 'IF(B1=B0, IF(C1=C0,1,0) ,0);
# This formula is recursively generated for each column in filter_columns, using the column
# position as determined by headers. The formula uses a more generalized form with
# 'INDIRECT(ADDRESS(<row>, <col>))'' instead of 'B1', where <row> is Row() and Row()-1, and col is
# determined by the column's position in headers
def generate_duplicate_formula(headers, filter_columns):
    # No more columns, put a 1 in the IF statement true branch
    if len(filter_columns) == 0:
        return '1'
    # Next column is found, generate the formula for duplicate checking, and remove from the list
    # for recursion
    for i in range(len(headers)):
        if headers[i] == filter_columns[0]:
            col = str(i + 1)
            formula = "IF(INDIRECT(ADDRESS(ROW(), " + col + "))=INDIRECT(ADDRESS(ROW() - 1, " + \
                col + "))," + generate_duplicate_formula(headers, filter_columns[1:]) + ",0)"
            return formula
    # Next column not found, remove from recursion but just return whatever the next one is
    return generate_duplicate_formula(headers, filter_columns[1:])


# Helper function to start the recursive call to generate_duplicate_formula
def generate_duplicate_formula_helper(headers):
    filter_columns = MAIN_RESULT_COLUMNS
    formula = generate_duplicate_formula(headers, filter_columns)
    if (formula == "1"):
        return ""
    else:
        # Final result needs to be prepended with =
        return "=" + formula

# Uses the list of headers and the info struct to come up with a list of values for each step
# from the latest builds.
def update_values(service, spreadsheet_id, headers, info):
    data = []
    for bot_name in info:
        for step_name in info[bot_name]['step_names']:
            sheet_name = format_sheet_name(bot_name, step_name)
            values = []
            # For each key in the list of headers, either add the corresponding value or add a blank
            # value. It's necessary for the values to match the order of the headers
            for key in headers[sheet_name]:
                if key in info[bot_name] and key in REQUIRED_COLUMNS:
                    values.append(info[bot_name][key])
                elif key in info[bot_name][step_name]:
                    values.append(info[bot_name][step_name][key])
                elif key == "duplicate" and key in REQUIRED_COLUMNS:
                    values.append(generate_duplicate_formula_helper(headers[sheet_name]))
                else:
                    values.append('')
            LOGGER.info("Appending new rows to sheet '" + sheet_name + "'...")
            try:
                append_values(service, spreadsheet_id, sheet_name, values)
            except Exception as error:
                LOGGER.warning('%s\n' % str(error))


# Updates the given spreadsheed_id with the info struct passed in.
def update_spreadsheet(service, spreadsheet_id, info):
    LOGGER.info('Opening spreadsheet...')
    spreadsheet = get_spreadsheet(service, spreadsheet_id)
    LOGGER.info('Parsing sheet names...')
    sheet_names = get_sheet_names(info)
    new_sheets = validate_sheets(spreadsheet, sheet_names)
    if new_sheets:
        LOGGER.info('Creating new sheets...')
        create_sheets(service, spreadsheet_id, new_sheets)
    LOGGER.info('Parsing sheet headers...')
    headers = get_headers(service, spreadsheet_id, sheet_names)
    update_headers(service, spreadsheet_id, headers, info)
    update_filters(service, spreadsheet_id, headers, info, spreadsheet)
    update_values(service, spreadsheet_id, headers, info)


#####################
# Main/helpers      #
#####################


# Loads or creates credentials and connects to the Sheets API. Returns a Spreadsheets object with
# an open connection.
def get_sheets_service(auth_path):
    credentials_path = auth_path + '/credentials.json'
    token_path = auth_path + '/token.pickle'
    creds = None
    if not os.path.exists(auth_path):
        LOGGER.info("Creating auth dir '" + auth_path + "'")
        os.makedirs(auth_path)
    if not os.path.exists(credentials_path):
        raise Exception('Missing credentials.json.\n'
                        'Go to: https://developers.google.com/sheets/api/quickstart/python\n'
                        "Under Step 1, click 'ENABLE THE GOOGLE SHEETS API'\n"
                        "Click 'DOWNLOAD CLIENT CONFIGURATION'\n"
                        'Save to your auth_path (' + auth_path + ') as credentials.json')
    if os.path.exists(token_path):
        with open(token_path, 'rb') as token:
            creds = pickle.load(token)
            LOGGER.info('Loaded credentials from ' + token_path)
    if not creds or not creds.valid:
        if creds and creds.expired and creds.refresh_token:
            LOGGER.info('Refreshing credentials...')
            creds.refresh(Request())
        else:
            LOGGER.info('Could not find credentials. Requesting new credentials.')
            flow = InstalledAppFlow.from_client_secrets_file(credentials_path, SCOPES)
            creds = flow.run_local_server()
        with open(token_path, 'wb') as token:
            pickle.dump(creds, token)
    service = build('sheets', 'v4', credentials=creds)
    sheets = service.spreadsheets()
    return sheets


# Parse the input to the script
def parse_args():
    parser = argparse.ArgumentParser(os.path.basename(sys.argv[0]))
    parser.add_argument(
        '--auth_path',
        default=HOME_DIR + '/.auth',
        nargs='?',
        help='path to directory containing authorization data '
        '(credentials.json and token.pickle). '
        '[default=<home>/.auth]')
    parser.add_argument(
        '--spreadsheet',
        default='1uttk1z8lJ4ZsUY7wMdFauMzUxb048nh5l52zdrAznek',
        nargs='?',
        help='ID of the spreadsheet to write stats to. '
        "[default='1uttk1z8lJ4ZsUY7wMdFauMzUxb048nh5l52zdrAznek']")
    parser.add_argument(
        '--verbosity',
        default='INFO',
        nargs='?',
        help='Verbosity of output. Valid options are '
        '[DEBUG, INFO, WARNING, ERROR]. '
        '[default=INFO]')
    return parser.parse_args()


# Set up the logging with the right verbosity and output.
def initialize_logging(verbosity):
    handler = logging.StreamHandler()
    formatter = logging.Formatter(fmt='%(levelname)s: %(message)s')
    handler.setFormatter(formatter)
    LOGGER.addHandler(handler)
    if 'DEBUG' in verbosity:
        LOGGER.setLevel(level=logging.DEBUG)
    elif 'INFO' in verbosity:
        LOGGER.setLevel(level=logging.INFO)
    elif 'WARNING' in verbosity:
        LOGGER.setLevel(level=logging.WARNING)
    elif 'ERROR' in verbosity:
        LOGGER.setLevel(level=logging.ERROR)
    else:
        LOGGER.setLevel(level=logging.INFO)


def main():
    os.chdir(ROOT_DIR)
    args = parse_args()
    verbosity = args.verbosity.strip().upper()
    initialize_logging(verbosity)
    auth_path = args.auth_path.replace('\\', '/')
    try:
        service = get_sheets_service(auth_path)
    except Exception as error:
        LOGGER.error('%s\n' % str(error))
        exit(1)

    info = {}
    LOGGER.info('Building info struct...')
    for bot_name in BOT_NAMES:
        LOGGER.info("Parsing bot '" + bot_name + "'...")
        try:
            info[bot_name] = get_bot_info(BOT_NAME_PREFIX + bot_name)
        except Exception as error:
            LOGGER.error('%s\n' % str(error))

    LOGGER.info('Updating sheets...')
    try:
        update_spreadsheet(service, args.spreadsheet, info)
    except Exception as error:
        LOGGER.error('%s\n' % str(error))
        quit(1)

    LOGGER.info('Info was successfully parsed to sheet: https://docs.google.com/spreadsheets/d/' +
                args.spreadsheet)


if __name__ == '__main__':
    sys.exit(main())
