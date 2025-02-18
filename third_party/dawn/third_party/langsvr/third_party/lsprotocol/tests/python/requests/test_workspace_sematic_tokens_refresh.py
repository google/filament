# Copyright (c) Microsoft Corporation. All rights reserved.
# Licensed under the MIT License.

import json
import uuid

import hamcrest
import jsonrpc
import pytest
from cattrs.errors import ClassValidationError

ID = str(uuid.uuid4())

TEST_DATA = [
    (
        {"id": ID, "method": "workspace/semanticTokens/refresh", "jsonrpc": "2.0"},
        json.dumps(
            {"id": ID, "method": "workspace/semanticTokens/refresh", "jsonrpc": "2.0"}
        ),
    ),
    (
        {
            "id": ID,
            "method": "workspace/semanticTokens/refresh",
            "params": None,
            "jsonrpc": "2.0",
        },
        json.dumps(
            {"id": ID, "method": "workspace/semanticTokens/refresh", "jsonrpc": "2.0"}
        ),
    ),
]


@pytest.mark.parametrize("index", list(range(0, len(TEST_DATA))))
def test_workspace_sematic_tokens_refresh_request_serialization(index):
    data, expected = TEST_DATA[index]
    data_str = json.dumps(data)
    parsed = jsonrpc.from_json(data_str)
    actual_str = jsonrpc.to_json(parsed)
    hamcrest.assert_that(actual_str, hamcrest.is_(expected))


@pytest.mark.parametrize(
    "data",
    [
        json.dumps({}),  # missing method and jsonrpc
        json.dumps({"method": "invalid"}),  # invalid method type
    ],
)
def test_workspace_sematic_tokens_refresh_request_invalid(data):
    with pytest.raises((ClassValidationError, KeyError)):
        jsonrpc.from_json(data)
