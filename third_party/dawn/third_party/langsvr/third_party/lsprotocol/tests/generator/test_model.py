# Copyright (c) Microsoft Corporation. All rights reserved.
# Licensed under the MIT License.

import json
import pathlib

import pytest

import generator.model as model

lsp_json_path = pathlib.Path(model.__file__).parent / "lsp.json"


def test_model_loading():
    json_model = json.loads((lsp_json_path).read_text(encoding="utf-8"))
    model.LSPModel(**json_model)


def test_model_loading_failure():
    json_model = json.loads((lsp_json_path).read_text(encoding="utf-8"))

    del json_model["structures"][0]["name"]
    with pytest.raises(TypeError):
        model.LSPModel(**json_model)
