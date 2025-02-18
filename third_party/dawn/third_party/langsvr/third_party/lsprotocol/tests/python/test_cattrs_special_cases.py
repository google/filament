# Copyright (c) Microsoft Corporation. All rights reserved.
# Licensed under the MIT License.

from typing import Optional, Union

import attrs
import hamcrest
import pytest
from cattrs.errors import ClassValidationError
from lsprotocol import converters as cv
from lsprotocol import types as lsp


def test_simple():
    """Ensure that simple LSP types are serializable."""
    data = {
        "range": {
            "start": {"line": 0, "character": 0},
            "end": {"line": 0, "character": 0},
        },
        "message": "Missing module docstring",
        "severity": 3,
        "code": "C0114:missing-module-docstring",
        "source": "my_lint",
    }
    converter = cv.get_converter()
    obj = converter.structure(data, lsp.Diagnostic)
    hamcrest.assert_that(obj, hamcrest.instance_of(lsp.Diagnostic))


def test_numeric_validation():
    """Ensure that out of range numbers raise exception."""
    data = {"line": -1, "character": 0}
    converter = cv.get_converter()
    with pytest.raises((ClassValidationError, ValueError)):
        converter.structure(data, lsp.Position)


def test_forward_refs():
    """Test that forward references are handled correctly by cattrs converter."""
    data = {
        "uri": "something.py",
        "diagnostics": [
            {
                "range": {
                    "start": {"line": 0, "character": 0},
                    "end": {"line": 0, "character": 0},
                },
                "message": "Missing module docstring",
                "severity": 3,
                "code": "C0114:missing-module-docstring",
                "source": "my_lint",
            },
            {
                "range": {
                    "start": {"line": 2, "character": 6},
                    "end": {
                        "line": 2,
                        "character": 7,
                    },
                },
                "message": "Undefined variable 'x'",
                "severity": 1,
                "code": "E0602:undefined-variable",
                "source": "my_lint",
            },
            {
                "range": {
                    "start": {"line": 0, "character": 0},
                    "end": {
                        "line": 0,
                        "character": 10,
                    },
                },
                "message": "Unused import sys",
                "severity": 2,
                "code": "W0611:unused-import",
                "source": "my_lint",
            },
        ],
    }
    converter = cv.get_converter()
    obj = converter.structure(data, lsp.PublishDiagnosticsParams)
    hamcrest.assert_that(obj, hamcrest.instance_of(lsp.PublishDiagnosticsParams))


@pytest.mark.parametrize(
    "data",
    [
        {},  # No properties provided
        {"documentSelector": None},
        {"documentSelector": []},
        {"documentSelector": [{"pattern": "something/**"}]},
        {"documentSelector": [{"language": "python"}]},
        {"documentSelector": [{"scheme": "file"}]},
        {"documentSelector": [{"notebook": "jupyter"}]},
        {"documentSelector": [{"language": "python"}]},
        {"documentSelector": [{"notebook": {"notebookType": "jupyter-notebook"}}]},
        {"documentSelector": [{"notebook": {"scheme": "file"}}]},
        {"documentSelector": [{"notebook": {"pattern": "something/**"}}]},
        {
            "documentSelector": [
                {"pattern": "something/**"},
                {"language": "python"},
                {"scheme": "file"},
                {"scheme": "untitled", "language": "python"},
                {"notebook": {"pattern": "something/**"}},
                {"notebook": {"scheme": "untitled"}},
                {"notebook": {"notebookType": "jupyter-notebook"}},
                {
                    "notebook": {"notebookType": "jupyter-notebook"},
                    "language": "jupyter",
                },
            ]
        },
    ],
)
def test_union_with_complex_type(data):
    """Ensure types with multiple possible resolutions are handled correctly."""
    converter = cv.get_converter()
    obj = converter.structure(data, lsp.TextDocumentRegistrationOptions)
    hamcrest.assert_that(obj, hamcrest.instance_of(lsp.TextDocumentRegistrationOptions))


def test_keyword_field():
    """Ensure that fields same names as keywords are handled correctly."""
    data = {
        "from": {
            "name": "something",
            "kind": 5,
            "uri": "something.py",
            "range": {
                "start": {"line": 0, "character": 0},
                "end": {
                    "line": 0,
                    "character": 10,
                },
            },
            "selectionRange": {
                "start": {"line": 0, "character": 2},
                "end": {
                    "line": 0,
                    "character": 8,
                },
            },
            "data": {"something": "some other"},
        },
        "fromRanges": [
            {
                "start": {"line": 0, "character": 0},
                "end": {
                    "line": 0,
                    "character": 10,
                },
            },
            {
                "start": {"line": 12, "character": 0},
                "end": {
                    "line": 13,
                    "character": 0,
                },
            },
        ],
    }

    converter = cv.get_converter()
    obj = converter.structure(data, lsp.CallHierarchyIncomingCall)
    hamcrest.assert_that(obj, hamcrest.instance_of(lsp.CallHierarchyIncomingCall))
    rev = converter.unstructure(obj, lsp.CallHierarchyIncomingCall)
    hamcrest.assert_that(rev, hamcrest.is_(data))


@pytest.mark.parametrize(
    "data",
    [
        {"settings": None},
        {"settings": 100000},
        {"settings": 1.23456},
        {"settings": True},
        {"settings": "something"},
        {"settings": {"something": "something"}},
        {"settings": []},
        {"settings": [None, None]},
        {"settings": [None, 1, 1.23, True]},
    ],
)
def test_LSPAny(data):
    """Ensure that broad primitive and custom type alias is handled correctly."""
    converter = cv.get_converter()
    obj = converter.structure(data, lsp.DidChangeConfigurationParams)
    hamcrest.assert_that(obj, hamcrest.instance_of(lsp.DidChangeConfigurationParams))
    hamcrest.assert_that(
        converter.unstructure(obj, lsp.DidChangeConfigurationParams),
        hamcrest.is_(data),
    )


@pytest.mark.parametrize(
    "data",
    [
        {"label": "hi"},
        {"label": [0, 42]},
    ],
)
def test_ParameterInformation(data):
    converter = cv.get_converter()
    obj = converter.structure(data, lsp.ParameterInformation)
    hamcrest.assert_that(obj, hamcrest.instance_of(lsp.ParameterInformation))
    hamcrest.assert_that(
        converter.unstructure(obj, lsp.ParameterInformation),
        hamcrest.is_(data),
    )


def test_completion_item():
    data = dict(label="example", documentation="This is documented")
    converter = cv.get_converter()
    obj = converter.structure(data, lsp.CompletionItem)
    hamcrest.assert_that(obj, hamcrest.instance_of(lsp.CompletionItem))
    hamcrest.assert_that(
        converter.unstructure(obj, lsp.CompletionItem),
        hamcrest.is_(data),
    )


def test_notebook_change_event():
    data = {
        "notebookDocument": {
            "uri": "untitled:Untitled-1.ipynb?jupyter-notebook",
            "notebookType": "jupyter-notebook",
            "version": 0,
            "cells": [
                {
                    "kind": 2,
                    "document": "vscode-notebook-cell:Untitled-1.ipynb?jupyter-notebook#W0sdW50aXRsZWQ%3D",
                    "metadata": {"custom": {"metadata": {}}},
                }
            ],
            "metadata": {
                "custom": {
                    "cells": [],
                    "metadata": {
                        "orig_nbformat": 4,
                        "language_info": {"name": "python"},
                    },
                },
                "indentAmount": " ",
            },
        },
        "cellTextDocuments": [
            {
                "uri": "vscode-notebook-cell:Untitled-1.ipynb?jupyter-notebook#W0sdW50aXRsZWQ%3D",
                "languageId": "python",
                "version": 1,
                "text": "",
            }
        ],
    }

    converter = cv.get_converter()
    obj = converter.structure(data, lsp.DidOpenNotebookDocumentParams)
    hamcrest.assert_that(obj, hamcrest.instance_of(lsp.DidOpenNotebookDocumentParams))
    hamcrest.assert_that(
        converter.unstructure(obj, lsp.DidOpenNotebookDocumentParams),
        hamcrest.is_(data),
    )


def test_notebook_sync_options():
    data = {"notebookSelector": [{"cells": [{"language": "python"}]}]}

    converter = cv.get_converter()
    obj = converter.structure(data, lsp.NotebookDocumentSyncOptions)
    hamcrest.assert_that(obj, hamcrest.instance_of(lsp.NotebookDocumentSyncOptions))
    hamcrest.assert_that(
        converter.unstructure(obj, lsp.NotebookDocumentSyncOptions),
        hamcrest.is_(data),
    )


@attrs.define
class TestPosEncoding:
    """Defines the capabilities provided by a language
    server."""

    position_encoding: Optional[Union[lsp.PositionEncodingKind, str]] = attrs.field(
        default=None
    )


@pytest.mark.parametrize("e", [None, "utf-8", "utf-16", "utf-32", "something"])
def test_position_encoding_kind(e):
    data = {"positionEncoding": e}
    converter = cv.get_converter()
    obj = converter.structure(data, TestPosEncoding)
    hamcrest.assert_that(obj, hamcrest.instance_of(TestPosEncoding))

    if e is None:
        hamcrest.assert_that(
            converter.unstructure(obj, TestPosEncoding), hamcrest.is_({})
        )
    else:
        hamcrest.assert_that(
            converter.unstructure(obj, TestPosEncoding), hamcrest.is_(data)
        )
