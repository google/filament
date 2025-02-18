# Copyright (c) Microsoft Corporation. All rights reserved.
# Licensed under the MIT License.

import lsprotocol.types as lsp
import lsprotocol.validators as v
import pytest


@pytest.mark.parametrize(
    "number", [v.INTEGER_MIN_VALUE, v.INTEGER_MAX_VALUE, 0, 1, -1, 1000, -1000]
)
def test_integer_validator_basic(number):
    lsp.VersionedTextDocumentIdentifier(version=number, uri="")


@pytest.mark.parametrize("number", [v.INTEGER_MIN_VALUE - 1, v.INTEGER_MAX_VALUE + 1])
def test_integer_validator_out_of_range(number):
    with pytest.raises(Exception):
        lsp.VersionedTextDocumentIdentifier(version=number, uri="")


@pytest.mark.parametrize(
    "number", [v.UINTEGER_MIN_VALUE, v.UINTEGER_MAX_VALUE, 0, 1, 1000, 10000]
)
def test_uinteger_validator_basic(number):
    lsp.Position(line=number, character=0)


@pytest.mark.parametrize("number", [v.UINTEGER_MIN_VALUE - 1, v.UINTEGER_MAX_VALUE + 1])
def test_uinteger_validator_out_of_range(number):
    with pytest.raises(Exception):
        lsp.Position(line=number, character=0)
