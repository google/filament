#!/usr/bin/env python3
# Copyright 2020 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
"""
angle_presubmit_utils: Mock depot_tools class for ANGLE presubmit checks's unittests
"""


class Change_mock():

    def __init__(self, description_text):
        self.description_text = description_text

    def DescriptionText(self):
        return self.description_text


class AffectedFile_mock():

    def __init__(self, diff):
        self.diff = diff

    def GenerateScmDiff(self):
        return self.diff


class InputAPI_mock():

    def __init__(self, description_text, source_files=[]):
        self.change = Change_mock(description_text)
        self.source_files = source_files

    def PresubmitLocalPath(self):
        return self.cwd

    def AffectedSourceFiles(self, source_filter):
        return self.source_files


class _PresubmitResult(object):
    """Base class for result objects."""
    fatal = False
    should_prompt = False

    def __init__(self, message, long_text=''):
        self._message = message

    def __eq__(self, other):
        return self.fatal == other.fatal and self.should_prompt == other.should_prompt \
            and self._message == other._message


# Top level object so multiprocessing can pickle
# Public access through OutputApi object.
class _PresubmitError(_PresubmitResult):
    """A hard presubmit error."""
    fatal = True


# Top level object so multiprocessing can pickle
# Public access through OutputApi object.
class _PresubmitPromptWarning(_PresubmitResult):
    """An warning that prompts the user if they want to continue."""
    should_prompt = True


# Top level object so multiprocessing can pickle
# Public access through OutputApi object.
class _PresubmitNotifyResult(_PresubmitResult):
    """Just print something to the screen -- but it's not even a warning."""
    pass


class OutputAPI_mock():
    PresubmitResult = _PresubmitResult
    PresubmitError = _PresubmitError
    PresubmitPromptWarning = _PresubmitPromptWarning
    PresubmitNotifyResult = _PresubmitNotifyResult
