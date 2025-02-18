# Copyright (c) Microsoft Corporation. All rights reserved.
# Licensed under the MIT License.

import hamcrest


def test_import():
    """Ensure that LSP types are importable."""
    import lsprotocol.types as lsp

    hamcrest.assert_that(lsp.MarkupKind.Markdown.value, hamcrest.is_("markdown"))
