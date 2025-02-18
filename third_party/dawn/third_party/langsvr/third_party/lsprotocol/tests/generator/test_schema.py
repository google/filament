# Copyright (c) Microsoft Corporation. All rights reserved.
# Licensed under the MIT License.

import json
import pathlib

import jsonschema

lsp_json_path = pathlib.Path(__file__).parent.parent.parent / "generator" / "lsp.json"
lsp_schema_path = lsp_json_path.parent / "lsp.schema.json"


def test_validate_with_schema():
    model = json.loads((lsp_json_path).read_text(encoding="utf-8"))
    schema = json.loads((lsp_schema_path).read_text(encoding="utf-8"))

    jsonschema.validate(model, schema)
