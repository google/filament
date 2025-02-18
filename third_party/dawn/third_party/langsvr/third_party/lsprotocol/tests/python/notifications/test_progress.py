# Copyright (c) Microsoft Corporation. All rights reserved.
# Licensed under the MIT License.

import json

import hamcrest
import jsonrpc
import pytest
from lsprotocol.types import (
    ProgressNotification,
    ProgressParams,
    WorkDoneProgressBegin,
    WorkDoneProgressEnd,
    WorkDoneProgressReport,
)


@pytest.mark.parametrize(
    "obj, expected",
    [
        (
            ProgressNotification(
                params=ProgressParams(
                    token="id1",
                    value=WorkDoneProgressBegin(title="Begin Progress", percentage=0),
                )
            ),
            json.dumps(
                {
                    "params": {
                        "token": "id1",
                        "value": {
                            "title": "Begin Progress",
                            "kind": "begin",
                            "percentage": 0,
                        },
                    },
                    "method": "$/progress",
                    "jsonrpc": "2.0",
                }
            ),
        ),
        (
            ProgressNotification(
                params=ProgressParams(
                    token="id1",
                    value=WorkDoneProgressReport(message="Still going", percentage=50),
                )
            ),
            json.dumps(
                {
                    "params": {
                        "token": "id1",
                        "value": {
                            "kind": "report",
                            "message": "Still going",
                            "percentage": 50,
                        },
                    },
                    "method": "$/progress",
                    "jsonrpc": "2.0",
                }
            ),
        ),
        (
            ProgressNotification(
                params=ProgressParams(
                    token="id1",
                    value=WorkDoneProgressEnd(message="Finished"),
                )
            ),
            json.dumps(
                {
                    "params": {
                        "token": "id1",
                        "value": {
                            "kind": "end",
                            "message": "Finished",
                        },
                    },
                    "method": "$/progress",
                    "jsonrpc": "2.0",
                }
            ),
        ),
    ],
)
def test_exit_serialization(obj, expected):
    actual_str = jsonrpc.to_json(obj)
    hamcrest.assert_that(actual_str, hamcrest.is_(expected))
