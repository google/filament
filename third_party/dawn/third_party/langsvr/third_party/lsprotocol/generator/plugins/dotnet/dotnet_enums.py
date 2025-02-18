# Copyright (c) Microsoft Corporation. All rights reserved.
# Licensed under the MIT License.

from typing import List, Union

from generator import model

from .dotnet_commons import TypeData
from .dotnet_constants import NAMESPACE
from .dotnet_helpers import (
    indent_lines,
    lines_to_doc_comments,
    namespace_wrapper,
    to_upper_camel_case,
)


def generate_enums(spec: model.LSPModel, types: TypeData) -> None:
    """Generate the code for the given spec."""
    for enum_def in spec.enumerations:
        types.add_type_info(enum_def, enum_def.name, generate_enum(enum_def))


def _get_enum_doc(enum: Union[model.Enum, model.EnumItem]) -> List[str]:
    doc = enum.documentation.splitlines(keepends=False) if enum.documentation else []
    return lines_to_doc_comments(doc)


def generate_enum(enum: model.Enum) -> List[str]:
    use_enum_member = all(isinstance(item.value, str) for item in enum.values)
    imports = ["using System.Runtime.Serialization;"]
    if use_enum_member:
        imports += ["using Newtonsoft.Json;", "using Newtonsoft.Json.Converters;"]

    lines = _get_enum_doc(enum)
    if use_enum_member:
        lines += ["[JsonConverter(typeof(StringEnumConverter))]"]
    lines += [f"public enum {enum.name}", "{"]

    for item in enum.values:
        name = to_upper_camel_case(item.name)
        inner = _get_enum_doc(item)
        if use_enum_member:
            inner += [f'[EnumMember(Value = "{item.value}")]{name},']
        else:
            inner += [f"{name} = {item.value},"]
        lines += indent_lines(inner) + [""]

    lines += ["}"]

    return namespace_wrapper(NAMESPACE, (imports if use_enum_member else []), lines)
