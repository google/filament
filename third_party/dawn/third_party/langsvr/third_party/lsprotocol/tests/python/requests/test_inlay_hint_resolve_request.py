# Copyright (c) Microsoft Corporation. All rights reserved.
# Licensed under the MIT License.

import json
import uuid

import hamcrest
import jsonrpc
import pytest

ID = str(uuid.uuid4())

TEST_DATA = [
    (
        {
            "id": ID,
            "method": "inlayHint/resolve",
            "params": {
                "position": {"line": 6, "character": 5},
                "label": "a label",
                "kind": 1,
                "paddingLeft": False,
                "paddingRight": True,
            },
            "jsonrpc": "2.0",
        },
        json.dumps(
            {
                "id": ID,
                "params": {
                    "position": {"line": 6, "character": 5},
                    "label": "a label",
                    "kind": 1,
                    "paddingLeft": False,
                    "paddingRight": True,
                },
                "method": "inlayHint/resolve",
                "jsonrpc": "2.0",
            }
        ),
    ),
    (
        {
            "id": ID,
            "method": "inlayHint/resolve",
            "params": {
                "position": {"line": 6, "character": 5},
                "label": [
                    {"value": "part 1"},
                    {"value": "part 2", "tooltip": "a tooltip"},
                ],
                "kind": 1,
                "paddingLeft": False,
                "paddingRight": True,
            },
            "jsonrpc": "2.0",
        },
        json.dumps(
            {
                "id": ID,
                "params": {
                    "position": {"line": 6, "character": 5},
                    "label": [
                        {"value": "part 1"},
                        {"value": "part 2", "tooltip": "a tooltip"},
                    ],
                    "kind": 1,
                    "paddingLeft": False,
                    "paddingRight": True,
                },
                "method": "inlayHint/resolve",
                "jsonrpc": "2.0",
            }
        ),
    ),
]


@pytest.mark.parametrize("index", list(range(0, len(TEST_DATA))))
def test_inlay_hint_resolve_request_serialization(index):
    data, expected = TEST_DATA[index]
    data_str = json.dumps(data)
    parsed = jsonrpc.from_json(data_str)
    actual_str = jsonrpc.to_json(parsed)
    hamcrest.assert_that(actual_str, hamcrest.is_(expected))
