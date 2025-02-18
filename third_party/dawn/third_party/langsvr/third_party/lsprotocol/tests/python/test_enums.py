# Copyright (c) Microsoft Corporation. All rights reserved.
# Licensed under the MIT License.

import hamcrest
import pytest
from lsprotocol import types as lsp


@pytest.mark.parametrize(
    ("value", "expected"),
    [
        ("refactor", lsp.CodeActionKind.Refactor),
        (lsp.CodeActionKind.Refactor, lsp.CodeActionKind.Refactor),
        ("namespace", lsp.SemanticTokenTypes.Namespace),
        (lsp.SemanticTokenTypes.Namespace, lsp.SemanticTokenTypes.Namespace),
        ("declaration", lsp.SemanticTokenModifiers.Declaration),
        (
            lsp.SemanticTokenModifiers.Declaration,
            lsp.SemanticTokenModifiers.Declaration,
        ),
        ("comment", lsp.FoldingRangeKind.Comment),
        (lsp.FoldingRangeKind.Comment, lsp.FoldingRangeKind.Comment),
        ("utf-8", lsp.PositionEncodingKind.Utf8),
        (lsp.PositionEncodingKind.Utf8, lsp.PositionEncodingKind.Utf8),
        (1, lsp.WatchKind.Create),
        (lsp.WatchKind.Create, lsp.WatchKind.Create),
        (-32700, lsp.ErrorCodes.ParseError),
        (lsp.ErrorCodes.ParseError, lsp.ErrorCodes.ParseError),
        (-32803, lsp.LSPErrorCodes.RequestFailed),
        (lsp.LSPErrorCodes.RequestFailed, lsp.LSPErrorCodes.RequestFailed),
    ],
)
def test_custom_enum_types(value, expected):
    hamcrest.assert_that(value, hamcrest.is_(expected))
