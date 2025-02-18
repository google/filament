# Copyright (c) Microsoft Corporation. All rights reserved.
# Licensed under the MIT License.

import json
import pathlib
from typing import List, Union

import lsprotocol.converters as cv
import lsprotocol.types as lsp
import pytest

TEST_DATA_ROOT = pathlib.Path(__file__).parent.parent.parent / "packages" / "testdata"


def get_all_json_files(root: Union[pathlib.Path, str]) -> List[pathlib.Path]:
    root_path = pathlib.Path(root)
    return list(root_path.glob("**/*.json"))


converter = cv.get_converter()


@pytest.mark.parametrize("json_file", get_all_json_files(TEST_DATA_ROOT))
def test_generated_data(json_file: str) -> None:
    type_name, result_type, _ = json_file.name.split("-", 2)
    lsp_type = getattr(lsp, type_name)
    data = json.loads(json_file.read_text(encoding="utf-8"))

    try:
        converter.structure(data, lsp_type)
        assert result_type == "True", "Expected error, but succeeded structuring"
    except Exception:
        assert result_type == "False", "Expected success, but failed structuring"
