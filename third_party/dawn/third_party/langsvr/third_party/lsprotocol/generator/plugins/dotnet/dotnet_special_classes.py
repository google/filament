# Copyright (c) Microsoft Corporation. All rights reserved.
# Licensed under the MIT License.

from typing import Dict, List, Union

from generator import model

from .dotnet_commons import TypeData
from .dotnet_constants import NAMESPACE
from .dotnet_helpers import class_wrapper, get_usings, namespace_wrapper

SPECIAL_CLASSES = [
    "LSPObject",
    "LSPAny",
    "LSPArray",
    "ChangeAnnotationIdentifier",
    "Pattern",
    "DocumentSelector",
    "InitializedParams",
]


def generate_special_classes(spec: model.LSPModel, types: TypeData) -> None:
    """Generate code for special classes in LSP."""
    for special_class in SPECIAL_CLASSES:
        for class_def in spec.structures + spec.typeAliases:
            if class_def.name == special_class:
                generate_special_class(class_def, spec, types)


def generate_special_class(
    type_def: Union[model.Structure, model.TypeAlias],
    spec: model.LSPModel,
    types: TypeData,
) -> Dict[str, str]:
    """Generate code for a special class."""
    lines: List[str] = []

    if type_def.name == "LSPObject":
        lines = namespace_wrapper(
            NAMESPACE,
            get_usings(["Dictionary", "DataContract", "JsonConverter"]),
            class_wrapper(
                type_def,
                ["public LSPObject(Dictionary<string, object?> value):base(value){}"],
                "Dictionary<string, object?>",
                ["[JsonConverter(typeof(CustomObjectConverter<LSPObject>))]"],
                is_record=False,
            ),
        )
    if type_def.name == "InitializedParams":
        lines = namespace_wrapper(
            NAMESPACE,
            get_usings(["Dictionary", "DataContract", "JsonConverter"]),
            class_wrapper(
                type_def,
                [
                    "public InitializedParams(Dictionary<string, object?> value):base(value){}"
                ],
                "Dictionary<string, object?>",
                ["[JsonConverter(typeof(CustomObjectConverter<InitializedParams>))]"],
                is_record=False,
            ),
        )
    if type_def.name == "LSPAny":
        lines = namespace_wrapper(
            NAMESPACE,
            get_usings(["DataContract", "JsonConverter"]),
            class_wrapper(
                type_def,
                [
                    "public LSPAny(object? value){this.Value = value;}",
                    "public object? Value { get; set; }",
                ],
                "object",
                ["[JsonConverter(typeof(LSPAnyConverter))]"],
            ),
        )
    if type_def.name == "LSPArray":
        lines = namespace_wrapper(
            NAMESPACE,
            get_usings(["DataContract", "List"]),
            class_wrapper(
                type_def,
                ["public LSPArray(List<object> value):base(value){}"],
                "List<object>",
                is_record=False,
            ),
        )

    if type_def.name == "Pattern":
        inner = [
            "private string pattern;",
            "public Pattern(string value){pattern = value;}",
            "public static implicit operator Pattern(string value) => new Pattern(value);",
            "public static implicit operator string(Pattern pattern) => pattern.pattern;",
            "public override string ToString() => pattern;",
        ]
        lines = namespace_wrapper(
            NAMESPACE,
            get_usings(["JsonConverter", "DataContract"]),
            class_wrapper(
                type_def,
                inner,
                None,
                [f"[JsonConverter(typeof(CustomStringConverter<{type_def.name}>))]"],
            ),
        )

    if type_def.name == "ChangeAnnotationIdentifier":
        inner = [
            "private string identifier;",
            "public ChangeAnnotationIdentifier(string value){identifier = value;}",
            "public static implicit operator ChangeAnnotationIdentifier(string value) => new ChangeAnnotationIdentifier(value);",
            "public static implicit operator string(ChangeAnnotationIdentifier identifier) => identifier.identifier;",
            "public override string ToString() => identifier;",
        ]
        lines = namespace_wrapper(
            NAMESPACE,
            get_usings(["JsonConverter", "DataContract"]),
            class_wrapper(
                type_def,
                inner,
                None,
                [f"[JsonConverter(typeof(CustomStringConverter<{type_def.name}>))]"],
            ),
        )

    if type_def.name == "DocumentSelector":
        inner = [
            "private DocumentFilter[] Filters { get; set; }",
            "public DocumentSelector(params DocumentFilter[] filters)",
            "{",
            "    Filters = filters ?? Array.Empty<DocumentFilter>();",
            "}",
            "public DocumentFilter this[int index]",
            "{",
            "    get { return Filters[index]; }",
            "    set { Filters[index] = value; }",
            "}",
            "public int Length => Filters.Length;",
            "public static implicit operator DocumentSelector(DocumentFilter[] filters) => new(filters);",
            "public static implicit operator DocumentFilter[](DocumentSelector selector) => selector.Filters;",
            "public IEnumerator<DocumentFilter> GetEnumerator() => ((IEnumerable<DocumentFilter>)Filters).GetEnumerator();",
            "System.Collections.IEnumerator System.Collections.IEnumerable.GetEnumerator() => Filters.GetEnumerator();",
        ]
        lines = namespace_wrapper(
            NAMESPACE,
            get_usings(["JsonConverter", "DataContract"]),
            class_wrapper(
                type_def,
                inner,
                "IEnumerable<DocumentFilter>",
                ["[JsonConverter(typeof(DocumentSelectorConverter))]"],
            ),
        )

    types.add_type_info(type_def, type_def.name, lines)
