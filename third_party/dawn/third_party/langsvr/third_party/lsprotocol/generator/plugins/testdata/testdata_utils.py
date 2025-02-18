# Copyright (c) Microsoft Corporation. All rights reserved.
# Licensed under the MIT License.

import logging
import pathlib
from typing import Dict

import generator.model as model

from .testdata_generator import generate

logger = logging.getLogger("testdata")


def generate_from_spec(spec: model.LSPModel, output_dir: str, test_dir: str) -> None:
    """Generate the code for the given spec."""
    output = pathlib.Path(output_dir)

    if not output.exists():
        output.mkdir(parents=True, exist_ok=True)

    logger.info("Cleaning up existing data")
    cleanup(output)
    # key is the relative path to the file, value is the content
    code: Dict[str, str] = generate(spec, logger)
    for file_name in code:
        # print file size
        file = output / file_name
        file.write_text(code[file_name], encoding="utf-8")


def cleanup(output_path: pathlib.Path) -> None:
    """Cleanup the generated C# files."""
    for file in output_path.glob("*.json"):
        file.unlink()
