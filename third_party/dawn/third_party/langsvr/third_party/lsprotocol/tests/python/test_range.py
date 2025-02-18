# Copyright (c) Microsoft Corporation. All rights reserved.
# Licensed under the MIT License.

import hamcrest
import pytest
from lsprotocol import types as lsp


@pytest.mark.parametrize(
    ("a", "b", "expected"),
    [
        (
            lsp.Range(lsp.Position(1, 23), lsp.Position(4, 56)),
            lsp.Range(lsp.Position(1, 23), lsp.Position(4, 56)),
            True,
        ),
        (
            lsp.Range(lsp.Position(1, 23), lsp.Position(4, 56)),
            lsp.Range(lsp.Position(1, 23), lsp.Position(4, 57)),
            False,
        ),
        (
            lsp.Range(lsp.Position(1, 23), lsp.Position(4, 56)),
            lsp.Range(lsp.Position(1, 23), lsp.Position(7, 56)),
            False,
        ),
    ],
)
def test_range_equality(a, b, expected):
    hamcrest.assert_that(a == b, hamcrest.is_(expected))


def test_range_repr():
    a = lsp.Range(lsp.Position(1, 23), lsp.Position(4, 56))
    hamcrest.assert_that(f"{a!r}", hamcrest.is_("1:23-4:56"))
