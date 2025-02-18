# Copyright (c) Microsoft Corporation. All rights reserved.
# Licensed under the MIT License.

import hamcrest
import pytest
from lsprotocol import types as lsp


@pytest.mark.parametrize(
    ("a", "b", "expected"),
    [
        (
            lsp.Location(
                "some_path", lsp.Range(lsp.Position(1, 23), lsp.Position(4, 56))
            ),
            lsp.Location(
                "some_path", lsp.Range(lsp.Position(1, 23), lsp.Position(4, 56))
            ),
            True,
        ),
        (
            lsp.Location(
                "some_path", lsp.Range(lsp.Position(1, 23), lsp.Position(4, 56))
            ),
            lsp.Location(
                "some_path2", lsp.Range(lsp.Position(1, 23), lsp.Position(4, 56))
            ),
            False,
        ),
        (
            lsp.Location(
                "some_path", lsp.Range(lsp.Position(1, 23), lsp.Position(4, 56))
            ),
            lsp.Location(
                "some_path", lsp.Range(lsp.Position(1, 23), lsp.Position(8, 91))
            ),
            False,
        ),
    ],
)
def test_location_equality(a, b, expected):
    hamcrest.assert_that(a == b, hamcrest.is_(expected))


def test_location_repr():
    a = lsp.Location("some_path", lsp.Range(lsp.Position(1, 23), lsp.Position(4, 56)))
    hamcrest.assert_that(f"{a!r}", hamcrest.is_("some_path:1:23-4:56"))
