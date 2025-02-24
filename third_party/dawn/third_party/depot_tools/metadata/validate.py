#!/usr/bin/env python3
# Copyright 2023 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import os
import sys
from typing import Callable, List, Tuple, Union

_THIS_DIR = os.path.abspath(os.path.dirname(__file__))
# The repo's root directory.
_ROOT_DIR = os.path.abspath(os.path.join(_THIS_DIR, ".."))

# Add the repo's root directory for clearer imports.
sys.path.insert(0, _ROOT_DIR)

import gclient_utils
import metadata.parse
import metadata.validation_result as vr


_TRANSITION_PRESCRIPT = (
    "The following issue should be addressed now, as it will become a "
    "presubmit error (instead of warning) once third party metadata "
    "validation is enforced.\nThird party metadata issue:")


def validate_content(content: str,
                     source_file_dir: str,
                     repo_root_dir: str,
                     is_open_source_project: bool = False) -> List[vr.ValidationResult]:
    """Validate the content as a metadata file.

    Args:
        content: the entire content of a file to be validated as a
                 metadata file.
        source_file_dir: the directory of the metadata file that the
                         license file value is from; this is needed to
                         construct file paths to license files.
        repo_root_dir: the repository's root directory; this is needed
                       to construct file paths to license files.
        is_open_source_project: whether the project is open source.

    Returns: the validation results for the given content, sorted based
             severity then message.
    """
    results = []
    dependencies = metadata.parse.parse_content(content)
    if not dependencies:
        result = vr.ValidationError(reason="No dependency metadata found.")
        return [result]

    for dependency in dependencies:
        dependency_results = dependency.validate(
            source_file_dir=source_file_dir,
            repo_root_dir=repo_root_dir,
            is_open_source_project=is_open_source_project,
        )
        results.extend(dependency_results)
    return sorted(results)


def _construct_file_read_error(filepath: str, cause: str) -> vr.ValidationError:
    """Helper function to create a validation error for a
    file reading issue.
    """
    result = vr.ValidationError(
        reason="Cannot read metadata file.",
        additional=[f"Attempted to read '{filepath}' but {cause}."])
    return result


def validate_file(
    filepath: str,
    repo_root_dir: str,
    reader: Callable[[str], Union[str, bytes]] = None,
    is_open_source_project: bool = False,
) -> List[vr.ValidationResult]:
    """Validate the item located at the given filepath is a valid
    dependency metadata file.

    Args:
        filepath: the path to validate, e.g.
                  "/chromium/src/third_party/libname/README.chromium"
        repo_root_dir: the repository's root directory; this is needed
                       to construct file paths to license files.
        reader (optional): callable function/method to read the content
                           of the file.
        is_open_source_project: whether to allow reciprocal licenses.
                            This should only be True for open source projects.

    Returns: the validation results for the given filepath and its
             contents, if it exists.
    """
    if reader is None:
        reader = gclient_utils.FileRead

    try:
        content = reader(filepath)
    except FileNotFoundError:
        return [_construct_file_read_error(filepath, "file not found")]
    except PermissionError:
        return [_construct_file_read_error(filepath, "access denied")]
    except Exception as e:
        return [
            _construct_file_read_error(filepath, f"unexpected error: '{e}'")
        ]
    else:
        if not isinstance(content, str):
            return [_construct_file_read_error(filepath, "decoding failed")]

        # Get the directory the metadata file is in.
        source_file_dir = os.path.dirname(filepath)
        return validate_content(content=content,
                                source_file_dir=source_file_dir,
                                repo_root_dir=repo_root_dir,
                                is_open_source_project=is_open_source_project)


def check_file(
    filepath: str,
    repo_root_dir: str,
    reader: Callable[[str], Union[str, bytes]] = None,
    is_open_source_project: bool = False,
) -> Tuple[List[str], List[str]]:
    """Run metadata validation on the given filepath, and return all
    validation errors and validation warnings.

    Args:
        filepath: the path to a metadata file, e.g.
                  "/chromium/src/third_party/libname/README.chromium"
        repo_root_dir: the repository's root directory; this is needed
                       to construct file paths to license files.
        reader (optional): callable function/method to read the content
                           of the file.
        is_open_source_project: whether to allow reciprocal licenses.
                            This should only be True for open source projects.

    Returns:
        error_messages: the fatal validation issues present in the file;
                        i.e. presubmit should fail.
        warning_messages: the non-fatal validation issues present in the
                          file; i.e. presubmit should still pass.
    """
    results = validate_file(filepath=filepath,
                            repo_root_dir=repo_root_dir,
                            reader=reader,
                            is_open_source_project=is_open_source_project)

    error_messages = []
    warning_messages = []
    for result in results:
        # TODO(aredulla): Actually distinguish between validation errors
        # and warnings. The quality of metadata is currently being
        # uplifted, but is not yet guaranteed to pass validation. So for
        # now, all validation results will be returned as warnings so
        # CLs are not blocked by invalid metadata in presubmits yet.
        # Bug: b/285453019.
        if result.is_fatal():
            message = result.get_message(prescript=_TRANSITION_PRESCRIPT,
                                         width=60)
        else:
            message = result.get_message(width=60)
        warning_messages.append(message)

    return error_messages, warning_messages
