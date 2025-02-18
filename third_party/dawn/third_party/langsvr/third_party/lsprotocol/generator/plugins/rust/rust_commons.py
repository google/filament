# Copyright (c) Microsoft Corporation. All rights reserved.
# Licensed under the MIT License.

from typing import Dict, List, Optional, Tuple, Union

from generator import model

from .rust_lang_utils import (
    get_parts,
    indent_lines,
    lines_to_doc_comments,
    to_snake_case,
    to_upper_camel_case,
)

TypesWithId = Union[
    model.Request,
    model.TypeAlias,
    model.Enum,
    model.Structure,
    model.Notification,
    model.LiteralType,
    model.ReferenceType,
    model.ReferenceMapKeyType,
    model.Property,
    model.EnumItem,
]


class TypeData:
    def __init__(self) -> None:
        self._id_data: Dict[
            str,
            Tuple[
                str,
                TypesWithId,
                List[str],
            ],
        ] = {}

    def add_type_info(
        self,
        type_def: TypesWithId,
        type_name: str,
        impl: List[str],
    ) -> None:
        if type_def.id_ in self._id_data:
            raise Exception(f"Duplicate id {type_def.id_} for type {type_name}")
        self._id_data[type_def.id_] = (type_name, type_def, impl)

    def has_id(
        self,
        type_def: TypesWithId,
    ) -> bool:
        return type_def.id_ in self._id_data

    def has_name(self, type_name: str) -> bool:
        return any(type_name == name for name, _, _ in self._id_data.values())

    def get_by_name(self, type_name: str) -> List[TypesWithId]:
        return [type_name == name for name, _, _ in self._id_data.values()]

    def get_lines(self):
        lines = []
        for _, _, impl in self._id_data.values():
            lines += impl + ["", ""]
        return lines


def generate_custom_enum(type_data: TypeData) -> None:
    type_data.add_type_info(
        model.ReferenceType(kind="reference", name="CustomStringEnum"),
        "CustomStringEnum",
        [
            "/// This type allows extending any string enum to support custom values.",
            "#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]",
            "#[serde(untagged)]",
            "pub enum CustomStringEnum<T> {",
            "    /// The value is one of the known enum values.",
            "    Known(T),",
            "    /// The value is custom.",
            "    Custom(String),",
            "}",
            "",
        ],
    )
    type_data.add_type_info(
        model.ReferenceType(kind="reference", name="CustomIntEnum"),
        "CustomIntEnum",
        [
            "/// This type allows extending any integer enum to support custom values.",
            "#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]",
            "#[serde(untagged)]",
            "pub enum CustomIntEnum<T> {",
            "    /// The value is one of the known enum values.",
            "    Known(T),",
            "    /// The value is custom.",
            "    Custom(i32),",
            "}",
            "",
        ],
    )
    type_data.add_type_info(
        model.ReferenceType(kind="reference", name="OR2"),
        "OR2",
        [
            "/// This allows a field to have two types.",
            "#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]",
            "#[serde(untagged)]",
            "pub enum OR2<T, U> {",
            "    T(T),",
            "    U(U),",
            "}",
            "",
        ],
    )
    type_data.add_type_info(
        model.ReferenceType(kind="reference", name="OR3"),
        "OR3",
        [
            "/// This allows a field to have three types.",
            "#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]",
            "#[serde(untagged)]",
            "pub enum OR3<T, U, V> {",
            "    T(T),",
            "    U(U),",
            "    V(V),",
            "}",
            "",
        ],
    )
    type_data.add_type_info(
        model.ReferenceType(kind="reference", name="OR4"),
        "OR4",
        [
            "/// This allows a field to have four types.",
            "#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]",
            "#[serde(untagged)]",
            "pub enum OR4<T, U, V, W> {",
            "    T(T),",
            "    U(U),",
            "    V(V),",
            "    W(W),",
            "}",
            "",
        ],
    )
    type_data.add_type_info(
        model.ReferenceType(kind="reference", name="OR5"),
        "OR5",
        [
            "/// This allows a field to have five types.",
            "#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]",
            "#[serde(untagged)]",
            "pub enum OR5<T, U, V, W, X> {",
            "    T(T),",
            "    U(U),",
            "    V(V),",
            "    W(W),",
            "    X(X),",
            "}",
            "",
        ],
    )
    type_data.add_type_info(
        model.ReferenceType(kind="reference", name="OR6"),
        "OR6",
        [
            "/// This allows a field to have six types.",
            "#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]",
            "#[serde(untagged)]",
            "pub enum OR6<T, U, V, W, X, Y> {",
            "    T(T),",
            "    U(U),",
            "    V(V),",
            "    W(W),",
            "    X(X),",
            "    Y(Y),",
            "}",
            "",
        ],
    )
    type_data.add_type_info(
        model.ReferenceType(kind="reference", name="OR7"),
        "OR7",
        [
            "/// This allows a field to have seven types.",
            "#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]",
            "#[serde(untagged)]",
            "pub enum OR7<T, U, V, W, X, Y, Z> {",
            "    T(T),",
            "    U(U),",
            "    V(V),",
            "    W(W),",
            "    X(X),",
            "    Y(Y),",
            "    Z(Z),",
            "}",
            "",
        ],
    )
    type_data.add_type_info(
        model.ReferenceType(kind="reference", name="LSPNull"),
        "LSPNull",
        [
            "/// This allows a field to always have null or empty value.",
            "#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]",
            "#[serde(untagged)]",
            "pub enum LSPNull {",
            "    None,",
            "}",
            "",
        ],
    )


def get_definition(
    name: str, spec: model.LSPModel
) -> Optional[Union[model.TypeAlias, model.Structure]]:
    for type_def in spec.typeAliases + spec.structures:
        if type_def.name == name:
            return type_def
    return None


def generate_special_types(model: model.LSPModel, types: TypeData) -> None:
    special_types = [
        get_definition("LSPAny", model),
        get_definition("LSPObject", model),
        get_definition("LSPArray", model),
        get_definition("SelectionRange", model),
    ]

    for type_def in special_types:
        if type_def:
            doc = (
                type_def.documentation.splitlines(keepends=False)
                if type_def.documentation
                else []
            )
            lines = lines_to_doc_comments(doc)
            lines += generate_extras(type_def)

            if type_def.name == "LSPAny":
                lines += [
                    "#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]",
                    "#[serde(untagged)]",
                    "pub enum LSPAny {",
                    "    String(String),",
                    "    Integer(i32),",
                    "    UInteger(u32),",
                    "    Decimal(Decimal),",
                    "    Boolean(bool),",
                    "    Object(LSPObject),",
                    "    Array(LSPArray),",
                    "    Null,",
                    "}",
                ]
            elif type_def.name == "LSPObject":
                lines += ["type LSPObject = serde_json::Value;"]
            elif type_def.name == "LSPArray":
                lines += ["type LSPArray = Vec<LSPAny>;"]
            elif type_def.name == "SelectionRange":
                lines += [
                    "#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]",
                    "pub struct SelectionRange {",
                ]
                for property in type_def.properties:
                    doc = (
                        property.documentation.splitlines(keepends=False)
                        if property.documentation
                        else []
                    )
                    lines += lines_to_doc_comments(doc)
                    lines += generate_extras(property)
                    prop_name = to_snake_case(property.name)
                    prop_type = get_type_name(
                        property.type, types, model, property.optional
                    )
                    if "SelectionRange" in prop_type:
                        prop_type = prop_type.replace(
                            "SelectionRange", "Box<SelectionRange>"
                        )
                    lines += [f"pub {prop_name}: {prop_type},"]
                    lines += [""]
                lines += ["}"]
            lines += [""]
            types.add_type_info(type_def, type_def.name, lines)


def fix_lsp_method_name(name: str) -> str:
    if name.startswith("$/"):
        name = name[2:]
    return to_upper_camel_case(name.replace("/", "_"))


def generate_special_enum(enum_name: str, items: List[str]) -> Dict[str, List[str]]:
    lines = [
        "#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]",
        f"pub enum {enum_name}" "{",
    ]
    for item in items:
        lines += indent_lines(
            [
                f'#[serde(rename = "{item}")]',
                f"{fix_lsp_method_name(item)},",
            ]
        )
    lines += ["}"]
    return lines


def generate_extra_types(spec: model.LSPModel, type_data: TypeData) -> None:
    type_data.add_type_info(
        model.ReferenceType(kind="reference", name="LSPRequestMethods"),
        "LSPRequestMethods",
        generate_special_enum("LSPRequestMethods", [m.method for m in spec.requests]),
    )
    type_data.add_type_info(
        model.ReferenceType(kind="reference", name="LSPNotificationMethods"),
        "LSPNotificationMethods",
        generate_special_enum(
            "LSPNotificationMethods", [m.method for m in spec.notifications]
        ),
    )

    direction = sorted(
        set([m.messageDirection for m in (spec.requests + spec.notifications)])
    )
    type_data.add_type_info(
        model.ReferenceType(kind="reference", name="MessageDirection"),
        "MessageDirection",
        generate_special_enum("MessageDirection", direction),
    )


def generate_commons(
    model: model.LSPModel, type_data: TypeData
) -> Dict[str, List[str]]:
    generate_custom_enum(type_data)
    generate_special_types(model, type_data)
    generate_extra_types(model, type_data)


def lsp_to_base_types(lsp_type: model.BaseType):
    if lsp_type.name in ["string", "DocumentUri", "URI", "RegExp"]:
        return "String"
    elif lsp_type.name in ["decimal"]:
        return "Decimal"
    elif lsp_type.name in ["integer"]:
        return "i32"
    elif lsp_type.name in ["uinteger"]:
        return "u32"
    elif lsp_type.name in ["boolean"]:
        return "bool"

    # null should be handled by the caller as an Option<> type
    raise ValueError(f"Unknown base type: {lsp_type.name}")


def _get_enum(name: str, spec: model.LSPModel) -> Optional[model.Enum]:
    for enum in spec.enumerations:
        if enum.name == name:
            return enum
    return None


def get_from_name(
    name: str, spec: model.LSPModel
) -> Optional[Union[model.Structure, model.Enum, model.TypeAlias]]:
    for some in spec.enumerations + spec.structures + spec.typeAliases:
        if some.name == name:
            return some
    return None


def get_extended_properties(
    struct_def: model.Structure, spec: model.LSPModel
) -> List[model.Property]:
    properties = [p for p in struct_def.properties]
    for t in struct_def.extends + struct_def.mixins:
        if t.kind == "reference":
            s = get_from_name(t.name, spec)
            if s:
                s_props = get_extended_properties(s, spec)
                properties += [p for p in s_props]
        elif t.kind == "literal":
            properties += [p for p in t.value.properties]
        else:
            raise ValueError(f"Unhandled extension type or mixin type: {t.kind}")
    unique_props = []
    for p in properties:
        if not any((p.name == u.name) for u in unique_props):
            unique_props.append(p)
    return sorted(unique_props, key=lambda p: p.name)


def _is_str_enum(enum_def: model.Enum) -> bool:
    return all(isinstance(item.value, str) for item in enum_def.values)


def _is_int_enum(enum_def: model.Enum) -> bool:
    return all(isinstance(item.value, int) for item in enum_def.values)


def generate_or_type(
    type_def: model.LSP_TYPE_SPEC,
    types: TypeData,
    spec: model.LSPModel,
    optional: Optional[bool] = None,
    name_context: Optional[str] = None,
) -> str:
    pass


def generate_and_type(
    type_def: model.LSP_TYPE_SPEC,
    types: TypeData,
    spec: model.LSPModel,
    optional: Optional[bool] = None,
    name_context: Optional[str] = None,
) -> str:
    pass


def get_type_name(
    type_def: model.LSP_TYPE_SPEC,
    types: TypeData,
    spec: model.LSPModel,
    optional: Optional[bool] = None,
    name_context: Optional[str] = None,
) -> str:
    if type_def.kind == "reference":
        enum_def = _get_enum(type_def.name, spec)
        if enum_def and enum_def.supportsCustomValues:
            if _is_str_enum(enum_def):
                name = f"CustomStringEnum<{enum_def.name}>"
            elif _is_int_enum(enum_def):
                name = f"CustomIntEnum<{enum_def.name}>"
        else:
            name = type_def.name
    elif type_def.kind == "array":
        name = f"Vec<{get_type_name(type_def.element, types, spec)}>"
    elif type_def.kind == "map":
        key_type = get_type_name(type_def.key, types, spec)
        value_type = get_type_name(type_def.value, types, spec)
        name = f"HashMap<{key_type}, {value_type}>"
    elif type_def.kind == "base":
        name = lsp_to_base_types(type_def)
    elif type_def.kind == "or":
        sub_set_items = [
            sub_spec
            for sub_spec in type_def.items
            if not (sub_spec.kind == "base" and sub_spec.name == "null")
        ]
        sub_types = [get_type_name(sub_spec, types, spec) for sub_spec in sub_set_items]
        sub_types_str = ", ".join(sub_types)
        if len(sub_types) >= 2:
            name = f"OR{len(sub_types)}<{sub_types_str}>"
        elif len(sub_types) == 1:
            name = sub_types[0]
        else:
            raise ValueError(
                f"OR type with more than out of range count of subtypes: {type_def}"
            )
        optional = optional or is_special(type_def)
    elif type_def.kind == "literal":
        name = generate_literal_struct_type(type_def, types, spec, name_context)
    elif type_def.kind == "stringLiteral":
        name = "String"
        # This type in rust requires a custom deserializer that fails if the value is not
        # one of the allowed values. This should be handled by the caller. This cannot be
        # handled here because all this does is handle type names.
    elif type_def.kind == "tuple":
        optional = optional or is_special(type_def)
        sub_set_items = [
            sub_spec
            for sub_spec in type_def.items
            if not (sub_spec.kind == "base" and sub_spec.name == "null")
        ]
        sub_types = [get_type_name(sub_spec, types, spec) for sub_spec in sub_set_items]
        sub_types_str = ", ".join(sub_types)
        if len(sub_types) >= 2:
            name = f"({sub_types_str})"
        elif len(sub_types) == 1:
            name = sub_types[0]
        else:
            raise ValueError(f"Invalid number of items for tuple: {type_def}")
    else:
        raise ValueError(f"Unknown type kind: {type_def.kind}")

    return f"Option<{name}>" if optional else name


def is_special(type_def: model.LSP_TYPE_SPEC) -> bool:
    if type_def.kind in ["or", "tuple"]:
        for item in type_def.items:
            if item.kind == "base" and item.name == "null":
                return True
    return False


def is_special_property(prop_def: model.Property) -> bool:
    return is_special(prop_def.type)


def is_string_literal_property(prop_def: model.Property) -> bool:
    return prop_def.type.kind == "stringLiteral"


def generate_literal_struct_name(
    type_def: model.LiteralType,
    types: TypeData,
    spec: model.LSPModel,
    name_context: Optional[str] = None,
) -> str:
    ignore_list = ["Struct", "Type", "Kind", "Options", "Params", "Result", "Options"]

    initial_parts = ["Struct"]
    if name_context:
        initial_parts += get_parts(name_context)

    optional_props = [p for p in type_def.value.properties if p.optional]
    required_props = [p for p in type_def.value.properties if not p.optional]

    required_parts = []
    for property in required_props:
        for p in get_parts(property.name):
            if p not in (ignore_list + required_parts):
                required_parts.append(p)

    optional = (
        ["Options"] if len(optional_props) == len(type_def.value.properties) else []
    )

    name_parts = initial_parts
    name = to_upper_camel_case("_".join(name_parts))

    all_ignore = all(n in ignore_list for n in name_parts)
    if types.has_name(name) or all_ignore:
        parts = []

        for r in required_parts:
            parts.append(r)
            name = to_upper_camel_case("_".join(initial_parts + parts + optional))
            if not types.has_name(name):
                return name

        for i in range(1, 100):
            end = [f"{i}"] if i > 1 else []
            name = to_upper_camel_case(
                "_".join(initial_parts + required_parts + optional + end)
            )
            if not types.has_name(name):
                return name
    return name


def _get_doc(doc: Optional[str]) -> str:
    if doc:
        return lines_to_doc_comments(doc.splitlines(keepends=False))
    return []


def generate_property(
    prop_def: model.Property, types: TypeData, spec: model.LSPModel
) -> str:
    prop_name = to_snake_case(prop_def.name)
    prop_type = get_type_name(
        prop_def.type, types, spec, prop_def.optional, prop_def.name
    )
    optional = (
        ['#[serde(skip_serializing_if = "Option::is_none")]']
        if is_special_property(prop_def) and not prop_def.optional
        else []
    )

    if prop_name in ["type"]:
        prop_name = f"{prop_name}_"
        if optional:
            optional = [
                f'#[serde(rename = "{prop_def.name}", skip_serializing_if = "Option::is_none")]'
            ]
        else:
            optional = [f'#[serde(rename = "{prop_def.name}")]']

    return (
        _get_doc(prop_def.documentation)
        + generate_extras(prop_def)
        + optional
        + [f"pub {prop_name}: {prop_type},"]
        + [""]
    )


def get_message_type_name(type_def: Union[model.Notification, model.Request]) -> str:
    name = fix_lsp_method_name(type_def.method)
    if isinstance(type_def, model.Notification):
        return f"{name}Notification"
    return f"{name}Request"


def struct_wrapper(
    type_def: Union[model.Structure, model.Notification, model.Request],
    inner: List[str],
) -> List[str]:
    if hasattr(type_def, "name"):
        name = type_def.name
    else:
        name = get_message_type_name(type_def)
    lines = (
        _get_doc(type_def.documentation)
        + generate_extras(type_def)
        + [
            "#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]",
            '#[serde(rename_all = "camelCase", deny_unknown_fields)]',
            f"pub struct {name}",
            "{",
        ]
    )
    lines += indent_lines(inner)
    lines += ["}", ""]
    return lines


def type_alias_wrapper(type_def: model.TypeAlias, inner: List[str]) -> List[str]:
    lines = (
        _get_doc(type_def.documentation)
        + generate_extras(type_def)
        + [
            "#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]",
            "#[serde(untagged)]",
            f"pub enum {type_def.name}",
            "{",
        ]
    )
    lines += indent_lines(inner)
    lines += ["}", ""]
    return lines


def generate_literal_struct_type(
    type_def: model.LiteralType,
    types: TypeData,
    spec: model.LSPModel,
    name_context: Optional[str] = None,
) -> None:
    if len(type_def.value.properties) == 0:
        return "LSPObject"

    if types.has_id(type_def):
        return type_def.name

    type_def.name = generate_literal_struct_name(type_def, types, spec, name_context)

    inner = []
    for prop_def in type_def.value.properties:
        inner += generate_property(prop_def, types, spec)

    lines = struct_wrapper(type_def, inner)
    types.add_type_info(type_def, type_def.name, lines)
    return type_def.name


def generate_extras(
    type_def: Union[
        model.Enum, model.EnumItem, model.Property, model.TypeAlias, model.Structure
    ],
) -> List[str]:
    extras = []
    if type_def.deprecated:
        extras += ["#[deprecated]"]
    if type_def.proposed:
        extras += ['#[cfg(feature = "proposed")]']
    return extras
