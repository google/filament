# Copyright (c) Microsoft Corporation. All rights reserved.
# Licensed under the MIT License.

import hamcrest
import pytest
from cattrs import ClassValidationError
from lsprotocol import converters as cv
from lsprotocol import types as lsp

TEST_DATA = [
    {
        "id": 1,
        "result": [{"name": "test", "kind": 1, "location": {"uri": "test"}}],
        "jsonrpc": "2.0",
    },
    {
        "id": 1,
        "result": [
            {
                "name": "test",
                "kind": 1,
                "location": {
                    "uri": "test",
                    "range": {
                        "start": {"line": 1, "character": 1},
                        "end": {"line": 1, "character": 1},
                    },
                },
            }
        ],
        "jsonrpc": "2.0",
    },
    {
        "id": 1,
        "result": [{"name": "test", "kind": 1, "location": {"uri": "test"}, "data": 1}],
        "jsonrpc": "2.0",
    },
    {
        "id": 1,
        "result": [
            {
                "name": "test",
                "kind": 1,
                "location": {
                    "uri": "test",
                    "range": {
                        "start": {"line": 1, "character": 1},
                        "end": {"line": 1, "character": 1},
                    },
                },
                "deprecated": True,
            }
        ],
        "jsonrpc": "2.0",
    },
]

BAD_TEST_DATA = [
    {
        "id": 1,
        "result": [
            {
                "name": "test",
                "kind": 1,
                "location": {"uri": "test"},
                "deprecated": True,
            }
        ],
        "jsonrpc": "2.0",
    },
]


@pytest.mark.parametrize("data", TEST_DATA)
def test_workspace_symbols(data):
    converter = cv.get_converter()
    obj = converter.structure(data, lsp.WorkspaceSymbolResponse)
    hamcrest.assert_that(obj, hamcrest.instance_of(lsp.WorkspaceSymbolResponse))
    hamcrest.assert_that(
        converter.unstructure(obj, lsp.WorkspaceSymbolResponse),
        hamcrest.is_(data),
    )


@pytest.mark.parametrize("data", BAD_TEST_DATA)
def test_workspace_symbols_bad(data):
    converter = cv.get_converter()
    with pytest.raises(ClassValidationError):
        obj = converter.structure(data, lsp.WorkspaceSymbolResponse)
        hamcrest.assert_that(obj, hamcrest.instance_of(lsp.WorkspaceSymbolResponse))
