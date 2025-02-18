# Copyright (c) Microsoft Corporation. All rights reserved.
# Licensed under the MIT License.

import json

import hamcrest
import jsonrpc
import pytest
from cattrs.errors import ClassValidationError


@pytest.mark.parametrize(
    "data, expected",
    [
        (
            {"method": "exit", "jsonrpc": "2.0"},
            json.dumps({"method": "exit", "jsonrpc": "2.0"}),
        ),
        (
            {"method": "exit", "params": None, "jsonrpc": "2.0"},
            json.dumps({"method": "exit", "jsonrpc": "2.0"}),
        ),
    ],
)
def test_exit_serialization(data, expected):
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
def test_exit_invalid(data):
    with pytest.raises((ClassValidationError, KeyError)):
        jsonrpc.from_json(data)
