# Copyright (c) Microsoft Corporation. All rights reserved.
# Licensed under the MIT License.

import logging
import pathlib
from typing import Dict

import generator.model as model

from .dotnet_classes import generate_all_classes
from .dotnet_commons import TypeData
from .dotnet_constants import NAMESPACE, PACKAGE_DIR_NAME
from .dotnet_enums import generate_enums
from .dotnet_helpers import namespace_wrapper
from .dotnet_special_classes import generate_special_classes

LOGGER = logging.getLogger("dotnet")


def generate_from_spec(spec: model.LSPModel, output_dir: str, _test_dir: str) -> None:
    """Generate the code for the given spec."""
    output_path = pathlib.Path(output_dir, PACKAGE_DIR_NAME)
    if not output_path.exists():
        output_path.mkdir(parents=True, exist_ok=True)

    cleanup(output_path)
    copy_custom_classes(output_path)

    LOGGER.info("Generating code in C#")
    types = TypeData()
    generate_package_code(spec, types)

    for name, lines in types.get_all():
        file_name = f"{name}.cs"
        (output_path / file_name).write_text("\n".join(lines), encoding="utf-8")


def generate_package_code(spec: model.LSPModel, types: TypeData) -> Dict[str, str]:
    generate_enums(spec, types)
    generate_special_classes(spec, types)
    generate_all_classes(spec, types)


def cleanup(output_path: pathlib.Path) -> None:
    """Cleanup the generated C# files."""
    for file in output_path.glob("*.cs"):
        file.unlink()


def copy_custom_classes(output_path: pathlib.Path) -> None:
    """Copy the custom classes to the output directory."""
    custom = pathlib.Path(__file__).parent / "custom"
    for file in custom.glob("*.cs"):
        lines = file.read_text(encoding="utf-8").splitlines()
        lines = namespace_wrapper(NAMESPACE, [], lines)
        (output_path / file.name).write_text("\n".join(lines), encoding="utf-8")
