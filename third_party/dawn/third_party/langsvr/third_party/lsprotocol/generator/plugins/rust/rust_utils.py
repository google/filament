# Copyright (c) Microsoft Corporation. All rights reserved.
# Licensed under the MIT License.

import pathlib
from typing import List

from generator import model

from .rust_commons import TypeData, generate_commons
from .rust_enum import generate_enums
from .rust_file_header import license_header
from .rust_lang_utils import lines_to_comments
from .rust_structs import (
    generate_notifications,
    generate_requests,
    generate_structures,
    generate_type_aliases,
)
from .rust_tests import generate_test_code

PACKAGE_DIR_NAME = "lsprotocol"


def generate_from_spec(spec: model.LSPModel, output_dir: str, test_dir: str) -> None:
    code = generate_package_code(spec)

    output_path = pathlib.Path(output_dir, PACKAGE_DIR_NAME)
    if not output_path.exists():
        output_path.mkdir(parents=True, exist_ok=True)
        (output_path / "src").mkdir(parents=True, exist_ok=True)

    for file_name in code:
        (output_path / file_name).write_text(code[file_name], encoding="utf-8")

    # update tests if exists
    test_path = pathlib.Path(test_dir) / "src" / "main.rs"
    if test_path.exists():
        generate_test_code(spec, test_path)


def generate_package_code(spec: model.LSPModel) -> List[str]:
    return {
        "src/lib.rs": generate_lib_rs(spec),
    }


def generate_lib_rs(spec: model.LSPModel) -> List[str]:
    lines = lines_to_comments(license_header())
    lines += [
        "",
        "// ****** THIS IS A GENERATED FILE, DO NOT EDIT. ******",
        "// Steps to generate:",
        "// 1. Checkout https://github.com/microsoft/lsprotocol",
        "// 2. Install nox: `python -m pip install nox`",
        "// 3. Run command: `python -m nox --session build_lsp`",
        "",
    ]
    lines += [
        "use serde::{Serialize, Deserialize};",
        "use std::collections::HashMap;",
        "use rust_decimal::Decimal;" "",
    ]

    type_data = TypeData()
    generate_commons(spec, type_data)
    generate_enums(spec.enumerations, type_data)

    generate_type_aliases(spec, type_data)
    generate_structures(spec, type_data)
    generate_notifications(spec, type_data)
    generate_requests(spec, type_data)

    lines += type_data.get_lines()
    return "\n".join(lines)
