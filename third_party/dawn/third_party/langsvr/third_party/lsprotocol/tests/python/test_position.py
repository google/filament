# Copyright (c) Microsoft Corporation. All rights reserved.
# Licensed under the MIT License.

import hamcrest
import pytest
from lsprotocol import types as lsp


@pytest.mark.parametrize(
    ("a", "b", "comp", "expected"),
    [
        (lsp.Position(1, 10), lsp.Position(1, 10), "==", True),
        (lsp.Position(1, 10), lsp.Position(1, 11), "==", False),
        (lsp.Position(1, 10), lsp.Position(1, 11), "!=", True),
        (lsp.Position(1, 10), lsp.Position(2, 20), "!=", True),
        (lsp.Position(2, 10), lsp.Position(1, 10), ">", True),
        (lsp.Position(2, 10), lsp.Position(1, 10), ">=", True),
        (lsp.Position(1, 11), lsp.Position(1, 10), ">", True),
        (lsp.Position(1, 11), lsp.Position(1, 10), ">=", True),
        (lsp.Position(1, 10), lsp.Position(1, 10), ">=", True),
        (lsp.Position(1, 10), lsp.Position(2, 10), "<", True),
        (lsp.Position(1, 10), lsp.Position(2, 10), "<=", True),
        (lsp.Position(1, 10), lsp.Position(1, 10), "<=", True),
        (lsp.Position(1, 10), lsp.Position(1, 11), "<", True),
        (lsp.Position(1, 10), lsp.Position(1, 11), "<=", True),
    ],
)
def test_position_comparison(
    a: lsp.Position, b: lsp.Position, comp: str, expected: bool
):
    if comp == "==":
        result = a == b
    elif comp == "!=":
        result = a != b
    elif comp == "<":
        result = a < b
    elif comp == "<=":
        result = a <= b
    elif comp == ">":
        result = a > b
    elif comp == ">=":
        result = a >= b
    hamcrest.assert_that(result, hamcrest.is_(expected))


def test_position_repr():
    p = lsp.Position(1, 23)
    hamcrest.assert_that(f"{p!r}", hamcrest.is_("1:23"))
