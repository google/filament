# Copyright (c) Microsoft Corporation. All rights reserved.
# Licensed under the MIT License.

import re
from typing import Any, Dict, List, Optional, Tuple, Union

import cattrs

from generator import model

from .dotnet_commons import TypeData
from .dotnet_constants import NAMESPACE
from .dotnet_helpers import (
    class_wrapper,
    generate_extras,
    get_doc,
    get_special_case_class_name,
    get_special_case_property_name,
    get_usings,
    indent_lines,
    lsp_method_to_name,
    namespace_wrapper,
    to_camel_case,
    to_upper_camel_case,
)

ORTYPE_CONVERTER_RE = re.compile(r"OrType<(?P<parts>.*)>")
IMMUTABLE_ARRAY_CONVERTER_RE = re.compile(r"ImmutableArray<(?P<elements>.*)>")


def _get_enum(name: str, spec: model.LSPModel) -> Optional[model.Enum]:
    for enum in spec.enumerations:
        if enum.name == name:
            return enum
    return None


def _get_struct(name: str, spec: model.LSPModel) -> Optional[model.Structure]:
    for struct in spec.structures:
        if struct.name == name:
            return struct
    return None


def _is_str_enum(enum_def: model.Enum) -> bool:
    return all(isinstance(item.value, str) for item in enum_def.values)


def _is_int_enum(enum_def: model.Enum) -> bool:
    return all(isinstance(item.value, int) for item in enum_def.values)


def lsp_to_base_types(lsp_type: model.BaseType):
    if lsp_type.name in ["string", "RegExp"]:
        return "string"
    elif lsp_type.name in ["DocumentUri", "URI"]:
        return "Uri"
    elif lsp_type.name in ["decimal"]:
        return "float"
    elif lsp_type.name in ["integer"]:
        return "int"
    elif lsp_type.name in ["uinteger"]:
        return "long"
    elif lsp_type.name in ["boolean"]:
        return "bool"
    elif lsp_type.name in ["null"]:
        return "object"

    # null should be handled by the caller as an Option<> type
    raise ValueError(f"Unknown base type: {lsp_type.name}")


def get_types_for_usings(code: List[str]) -> List[str]:
    immutable = []
    for line in code:
        if "ImmutableArray<" in line:
            immutable.append("ImmutableArray")
        if "ImmutableDictionary<" in line:
            immutable.append("ImmutableDictionary")
    return list(set(immutable))


def has_null_base_type(items: List[model.LSP_TYPE_SPEC]) -> bool:
    return any(item.kind == "base" and item.name == "null" for item in items)


def filter_null_base_type(
    items: List[model.LSP_TYPE_SPEC],
) -> List[model.LSP_TYPE_SPEC]:
    return [item for item in items if not (item.kind == "base" and item.name == "null")]


def get_type_name(
    type_def: model.LSP_TYPE_SPEC,
    types: TypeData,
    spec: model.LSPModel,
    name_context: Optional[str] = None,
) -> str:
    name = None
    if type_def.kind == "reference":
        enum_def = _get_enum(type_def.name, spec)
        if enum_def and enum_def.supportsCustomValues:
            if _is_str_enum(enum_def):
                name = "string"
            elif _is_int_enum(enum_def):
                name = "int"
        else:
            name = get_special_case_class_name(type_def.name)
    elif type_def.kind == "array":
        name = f"ImmutableArray<{get_type_name(type_def.element, types, spec, name_context)}>"
    elif type_def.kind == "map":
        name = generate_map_type(type_def, types, spec, name_context)
    elif type_def.kind == "base":
        name = lsp_to_base_types(type_def)
    elif type_def.kind == "literal":
        name = generate_literal_type(type_def, types, spec, name_context)
    elif type_def.kind == "stringLiteral":
        name = "string"
    elif type_def.kind == "tuple":
        subset = filter_null_base_type(type_def.items)
        subset_types = [
            get_type_name(item, types, spec, name_context) for item in subset
        ]
        name = f"({', '.join(subset_types)})"
    elif type_def.kind == "or":
        subset = filter_null_base_type(type_def.items)
        if len(subset) == 1:
            name = get_type_name(subset[0], types, spec, name_context)
        elif len(subset) >= 2:
            if are_variant_literals(subset):
                name = generate_class_from_variant_literals(
                    subset, spec, types, name_context
                )
            else:
                subset_types = [
                    get_type_name(item, types, spec, name_context) for item in subset
                ]
                name = f"OrType<{', '.join(subset_types)}>"
        else:
            raise ValueError(f"Unknown type kind: {type_def.kind}")
    else:
        raise ValueError(f"Unknown type kind: {type_def.kind}")
    return name


def generate_map_type(
    type_def: model.LSP_TYPE_SPEC,
    types: TypeData,
    spec: model.LSPModel,
    name_context: Optional[str] = None,
) -> str:
    key_type = get_type_name(type_def.key, types, spec, name_context)

    if type_def.value.kind == "or":
        subset = filter_null_base_type(type_def.value.items)
        if len(subset) == 1:
            value_type = get_type_name(type_def.value, types, spec, name_context)
        else:
            value_type = to_upper_camel_case(f"{name_context}Value")
            type_alias = model.TypeAlias(
                **{
                    "name": value_type,
                    "type": type_def.value,
                }
            )
            generate_class_from_type_alias(type_alias, spec, types)

    else:
        value_type = get_type_name(type_def.value, types, spec, name_context)
    return f"ImmutableDictionary<{key_type}, {value_type}>"


def get_converter(type_def: model.LSP_TYPE_SPEC, type_name: str) -> Optional[str]:
    if type_def.kind == "base" and type_def.name in ["DocumentUri", "URI"]:
        return "[JsonConverter(typeof(CustomStringConverter<Uri>))]"
    elif type_def.kind == "reference" and type_def.name in [
        "Pattern",
        "ChangeAnnotationIdentifier",
    ]:
        return f"[JsonConverter(typeof(CustomStringConverter<{type_def.name}>))]"
    elif type_def.kind == "reference" and type_def.name == "DocumentSelector":
        return "[JsonConverter(typeof(DocumentSelectorConverter))]"
    elif type_def.kind == "or":
        subset = filter_null_base_type(type_def.items)
        if len(subset) == 1:
            return get_converter(subset[0], type_name)
        elif len(subset) >= 2:
            converter = type_name.replace("OrType<", "OrTypeConverter<")
            return f"[JsonConverter(typeof({converter}))]"
    elif type_def.kind == "array" and type_name.startswith("OrType<"):
        matches = ORTYPE_CONVERTER_RE.match(type_name).groupdict()
        if "parts" in matches:
            converter = f"OrTypeArrayConverter<{matches['parts']}>"
            return f"[JsonConverter(typeof({converter}))]"
    elif type_def.kind == "array":
        matches = IMMUTABLE_ARRAY_CONVERTER_RE.match(type_name).groupdict()
        elements = matches["elements"]
        if elements.startswith("OrType<"):
            matches = ORTYPE_CONVERTER_RE.match(elements).groupdict()
            converter = f"OrTypeArrayConverter<{matches['parts']}>"
            return f"[JsonConverter(typeof({converter}))]"
        else:
            converter = f"CustomArrayConverter<{elements}>"
            return f"[JsonConverter(typeof({converter}))]"
    return None


def generate_property(
    prop_def: model.Property,
    spec: model.LSPModel,
    types: TypeData,
    usings: List[str],
    class_name: str = "",
) -> Tuple[List[str], str]:
    if prop_def.name == "jsonrpc":
        name = "JsonRPC"
    else:
        name = to_upper_camel_case(prop_def.name)
    type_name = get_type_name(
        prop_def.type, types, spec, f"{class_name}_{prop_def.name}"
    )
    converter = get_converter(prop_def.type, type_name)
    special_optional = prop_def.type.kind == "or" and has_null_base_type(
        prop_def.type.items
    )
    optional = (
        "?"
        if (prop_def.optional or special_optional)
        and not (
            type_name.startswith("ImmutableArray<")
            or type_name.startswith("ImmutableDictionary<")
        )
        else ""
    )
    lines = (
        get_doc(prop_def.documentation)
        + generate_extras(prop_def)
        + ([converter] if converter else [])
        + (
            ["[JsonProperty(NullValueHandling = NullValueHandling.Ignore)]"]
            if optional and not special_optional
            else []
        )
        + [
            f'[DataMember(Name = "{prop_def.name}")]',
        ]
    )

    if prop_def.type.kind == "stringLiteral":
        lines.append(
            f'public {type_name}{optional} {name} {{ get; init; }} = "{prop_def.type.value}";'
        )
    elif prop_def.type.kind == "base" and prop_def.type.name == "uinteger":
        private_name = f"_{prop_def.name}" if prop_def.name == name else prop_def.name
        lines.append(
            f"public {type_name}{optional} {name} {{ get => {private_name}; set => {private_name} = Validators.validUInteger(value); }}"
        )
        lines.append(f"private {type_name}{optional} {private_name};")
    else:
        lines.append(f"public {type_name}{optional} {name} {{ get; init; }}")

    usings.append("DataMember")
    if converter:
        usings.append("JsonConverter")
    if optional and not special_optional:
        usings.append("JsonProperty")

    return lines, type_name


def generate_name(name_context: str, types: TypeData) -> str:
    # If name context has a '_' it is likely a property.
    # Try name generation using just the property name
    parts = [to_upper_camel_case(p) for p in name_context.split("_") if len(p) > 3]

    # Try the last part of the name context
    name = parts[-1]
    if not types.get_by_name(name) and "info" in name_context.lower():
        return name

    # Combine all parts and try again
    name = "".join(parts)
    if not types.get_by_name(name):
        return name

    raise ValueError(f"Unable to generate name for {name_context}")


def generate_literal_type(
    literal: model.LiteralType,
    types: TypeData,
    spec: model.LSPModel,
    name_context: Optional[str] = None,
) -> str:
    if len(literal.value.properties) == 0:
        return "LSPObject"

    if types.get_by_name(literal.name) and not _get_struct(literal.name, spec):
        return literal.name

    if name_context is None:
        raise ValueError("name_context must be provided for literal types")

    if name_context.startswith("I") and name_context[1].isupper():
        # This is a interface name ISomething => Something
        name_context = name_context[1:]

    if "_" not in name_context:
        name_context = f"{name_context}_{get_context_from_literal(literal)}"

    literal.name = generate_name(name_context, types)

    usings = ["DataContract"]
    inner = []
    for prop in literal.value.properties:
        prop_code, _ = generate_property(prop, spec, types, usings, literal.name)
        inner += prop_code

    lines = namespace_wrapper(
        NAMESPACE,
        get_usings(usings + get_types_for_usings(inner)),
        class_wrapper(literal, inner),
    )
    types.add_type_info(literal, literal.name, lines)
    return literal.name


def generate_constructor(
    struct: model.Structure,
    types: TypeData,
    properties: List[Tuple[model.Property, str]],
) -> List[str]:
    class_name = get_special_case_class_name(struct.name)
    constructor = [
        "[JsonConstructor]",
        f"public {class_name}(",
    ]

    arguments = []
    optional_args = []
    assignments = []
    ctor_data = []
    for prop, prop_type in properties:
        name = get_special_case_property_name(to_camel_case(prop.name))
        special_optional = prop.type.kind == "or" and has_null_base_type(
            prop.type.items
        )
        if prop.optional or special_optional:
            if prop_type.startswith("ImmutableArray<") or prop_type.startswith(
                "ImmutableDictionary<"
            ):
                optional_args += [f"{prop_type} {name} = default!"]
            else:
                optional_args += [f"{prop_type}? {name} = null"]
            ctor_data += [(prop_type, name, True)]
        elif prop.name == "jsonrpc":
            optional_args += [f'{prop_type} {name} = "2.0"']
            ctor_data += [(prop_type, name, True)]
        else:
            arguments += [f"{prop_type} {name}"]
            ctor_data += [(prop_type, name, False)]

        if prop.name == "jsonrpc":
            assignments += [f"JsonRPC = {name};"]
        else:
            assignments += [f"{to_upper_camel_case(prop.name)} = {name};"]

    # combine args with a '\n' to get comma with indent
    all_args = (",\n".join(indent_lines(arguments + optional_args))).splitlines()
    types.add_ctor(struct.name, ctor_data)

    # re-split args to get the right coma placement and indent
    constructor += all_args
    constructor += [")", "{"]
    constructor += indent_lines(assignments)
    constructor += ["}"]
    return constructor


def generate_class_from_struct(
    struct: model.Structure,
    spec: model.LSPModel,
    types: TypeData,
    derived: Optional[str] = None,
    attributes: Optional[List[str]] = None,
):
    if types.get_by_name(struct.name) or struct.name.startswith("_"):
        return

    if attributes is None:
        attributes = []

    inner = []
    usings = ["DataContract", "JsonConstructor"]

    properties = get_all_properties(struct, spec)
    prop_types = []
    for prop in properties:
        prop_code, prop_type = generate_property(prop, spec, types, usings, struct.name)
        inner += prop_code
        prop_types += [prop_type]

    ctor = generate_constructor(struct, types, zip(properties, prop_types))
    inner = ctor + inner

    lines = namespace_wrapper(
        NAMESPACE,
        get_usings(usings + get_types_for_usings(inner + attributes)),
        class_wrapper(struct, inner, derived, attributes),
    )
    types.add_type_info(struct, struct.name, lines)


def get_context_from_literal(literal: model.LiteralType) -> str:
    if len(literal.value.properties) == 0:
        return "LSPObject"

    skipped = 0
    skip = [
        "range",
        "rangeLength",
        "position",
        "position",
        "location",
        "locationLink",
        "text",
    ]
    for prop in literal.value.properties:
        if prop.name in skip:
            skipped += 1
            continue
        return prop.name

    if skipped == len(literal.value.properties):
        # pick property with longest name
        names = sorted([p.name for p in literal.value.properties])
        return sorted(names, key=lambda n: len(n))[-1]

    return ""


def generate_type_alias_constructor(
    type_def: model.TypeAlias, spec: model.LSPModel, types: TypeData
) -> List[str]:
    constructor = []

    if type_def.type.kind == "or":
        subset = filter_null_base_type(type_def.type.items)
        if len(subset) == 1:
            raise ValueError("Unable to generate constructor for single item union")
        elif len(subset) >= 2:
            type_name = to_upper_camel_case(type_def.name)
            for t in subset:
                sub_type = get_type_name(t, types, spec, type_def.name)
                arg = get_special_case_property_name(to_camel_case(sub_type))
                matches = re.match(r"ImmutableArray<(?P<arg>\w+)>", arg)
                if matches:
                    arg = f"{matches['arg']}s"

                constructor += [
                    f"public {type_name}({sub_type} {arg}): base({arg}) {{}}",
                ]
        else:
            raise ValueError("Unable to generate constructor for empty union")
    elif type_def.type.kind == "reference":
        type_name = to_upper_camel_case(type_def.name)
        ctor_data = types.get_ctor(type_def.type.name)
        required = [
            (prop_type, prop_name)
            for prop_type, prop_name, optional in ctor_data
            if not optional
        ]
        optional = [
            (prop_type, prop_name)
            for prop_type, prop_name, optional in ctor_data
            if optional
        ]

        ctor_args = [f"{prop_type} {prop_name}" for prop_type, prop_name in required]
        ctor_args += [
            f"{prop_type}? {prop_name} = null" for prop_type, prop_name in optional
        ]

        base_args = [f"{prop_name}" for _, prop_name in required + optional]
        constructor += [
            f"public {type_name}({','.join(ctor_args)}): base({','.join(base_args)}) {{}}",
        ]

    return constructor


def generate_type_alias_converter(
    type_def: model.TypeAlias, spec: model.LSPModel, types: TypeData
) -> None:
    assert type_def.type.kind == "or"
    subset_types = [
        get_type_name(i, types, spec, type_def.name)
        for i in filter_null_base_type(type_def.type.items)
    ]
    converter = f"{type_def.name}Converter"
    or_type_converter = f"OrTypeConverter<{','.join(subset_types)}>"
    or_type = f"OrType<{','.join(subset_types)}>"
    code = [
        f"public class {converter} : JsonConverter<{type_def.name}>",
        "{",
        f"private {or_type_converter} _orType;",
        f"public {converter}()",
        "{",
        f"_orType = new {or_type_converter}();",
        "}",
        f"public override {type_def.name}? ReadJson(JsonReader reader, Type objectType, {type_def.name}? existingValue, bool hasExistingValue, JsonSerializer serializer)",
        "{",
        "reader = reader ?? throw new ArgumentNullException(nameof(reader));",
        "if (reader.TokenType == JsonToken.Null) { return null; }",
        "var o = _orType.ReadJson(reader, objectType, existingValue, serializer);",
        f"if (o is {or_type} orType)",
        "{",
    ]
    for t in subset_types:
        code += [
            f"if (orType.Value?.GetType() == typeof({t}))",
            "{",
            f"return new {type_def.name}(({t})orType.Value);",
            "}",
        ]
    code += [
        "}",
        'throw new JsonSerializationException($"Unexpected token type.");',
        "}",
        f"public override void WriteJson(JsonWriter writer, {type_def.name}? value, JsonSerializer serializer)",
        "{",
        "_orType.WriteJson(writer, value, serializer);",
        "}",
        "}",
    ]

    code = namespace_wrapper(
        NAMESPACE, get_usings(["JsonConverter"] + get_types_for_usings(code)), code
    )

    ref = model.Structure(**{"name": converter, "properties": []})
    types.add_type_info(ref, converter, code)
    return converter


def generate_class_from_type_alias(
    type_def: model.TypeAlias, spec: model.LSPModel, types: TypeData
) -> None:
    if types.get_by_name(type_def.name):
        return

    usings = ["DataContract"]
    type_name = get_type_name(type_def.type, types, spec, type_def.name)
    class_attributes = []
    if type_def.type.kind == "or":
        converter = generate_type_alias_converter(type_def, spec, types)
        class_attributes += [f"[JsonConverter(typeof({converter}))]"]
        usings.append("JsonConverter")

    inner = generate_type_alias_constructor(type_def, spec, types)
    lines = namespace_wrapper(
        NAMESPACE,
        get_usings(usings + get_types_for_usings(inner)),
        class_wrapper(type_def, inner, type_name, class_attributes),
    )
    types.add_type_info(type_def, type_def.name, lines)


def generate_class_from_variant_literals(
    literals: List[model.LiteralType],
    spec: model.LSPModel,
    types: TypeData,
    name_context: Optional[str] = None,
) -> str:
    name = generate_name(name_context, types)
    if types.get_by_name(name):
        raise ValueError(f"Name {name} already exists")

    struct = model.Structure(
        **{
            "name": name,
            "properties": get_properties_from_literals(literals),
        }
    )

    lines = generate_code_for_variant_struct(struct, spec, types)
    types.add_type_info(struct, struct.name, lines)
    return struct.name


def get_properties_from_literals(literals: List[model.LiteralType]) -> Dict[str, Any]:
    properties = []
    for literal in literals:
        assert literal.kind == "literal"
        for prop in literal.value.properties:
            if prop.name not in [p["name"] for p in properties]:
                properties.append(
                    {
                        "name": prop.name,
                        "type": cattrs.unstructure(prop.type),
                        "optional": has_optional_variant(literals, prop.name),  #
                    }
                )
    return properties


def generate_code_for_variant_struct(
    struct: model.Structure,
    spec: model.LSPModel,
    types: TypeData,
) -> None:
    prop_types = []
    inner = []
    usings = ["DataContract", "JsonConstructor"]
    for prop in struct.properties:
        prop_code, prop_type = generate_property(prop, spec, types, usings, struct.name)
        inner += prop_code
        prop_types += [prop_type]

    ctor_data = []
    constructor_args = []
    conditions = []
    for prop, prop_type in zip(struct.properties, prop_types):
        name = get_special_case_property_name(to_camel_case(prop.name))
        immutable = prop_type.startswith("ImmutableArray<") or prop_type.startswith(
            "ImmutableDictionary<"
        )
        constructor_args += [
            f"{prop_type} {name}" if immutable else f"{prop_type}? {name}"
        ]
        ctor_data = [(prop_type)]
        if immutable:
            conditions += [f"({name}.IsDefault)"]
        else:
            conditions += [f"({name} is null)"]

    sig = ", ".join(constructor_args)
    types.add_ctor(struct.name, ctor_data)
    ctor = [
        "[JsonConstructor]",
        f"public {struct.name}({sig})",
        "{",
        *indent_lines(
            [
                f"if ({'&&'.join(conditions)})",
                "{",
                *indent_lines(
                    [
                        'throw new ArgumentException("At least one of the arguments must be non-null");'
                    ]
                ),
                "}",
            ]
        ),
        *indent_lines(
            [
                f"{to_upper_camel_case(prop.name)} = {get_special_case_property_name(to_camel_case(prop.name))};"
                for prop in struct.properties
            ]
        ),
        "}",
    ]

    inner = ctor + inner

    return namespace_wrapper(
        NAMESPACE,
        get_usings(usings + get_types_for_usings(inner)),
        class_wrapper(struct, inner, None),
    )


def generate_class_from_variant_type_alias(
    type_def: model.TypeAlias,
    spec: model.LSPModel,
    types: TypeData,
    name_context: Optional[str] = None,
) -> None:
    struct = model.Structure(
        **{
            "name": type_def.name,
            "properties": get_properties_from_literals(type_def.type.items),
            "documentation": type_def.documentation,
            "since": type_def.since,
            "deprecated": type_def.deprecated,
            "proposed": type_def.proposed,
        }
    )

    lines = generate_code_for_variant_struct(struct, spec, types)
    types.add_type_info(type_def, type_def.name, lines)


def has_optional_variant(literals: List[model.LiteralType], property_name: str) -> bool:
    count = 0
    optional = False
    for literal in literals:
        for prop in literal.value.properties:
            if prop.name == property_name:
                count += 1
                optional = optional or prop.optional
    return optional and count == len(literals)


def are_variant_literals(literals: List[model.LiteralType]) -> bool:
    if all(i.kind == "literal" for i in literals):
        return all(
            has_optional_variant(literals, prop.name)
            for prop in literals[0].value.properties
        )
    return False


def is_variant_type_alias(type_def: model.TypeAlias) -> bool:
    if type_def.type.kind == "or" and all(
        i.kind == "literal" for i in type_def.type.items
    ):
        literals = type_def.type.items
        return all(
            has_optional_variant(literals, prop.name)
            for prop in literals[0].value.properties
        )
    return False


def copy_struct(struct_def: model.Structure, new_name: str):
    converter = cattrs.GenConverter()
    obj = converter.unstructure(struct_def, model.Structure)
    obj["name"] = new_name
    return model.Structure(**obj)


def copy_property(prop_def: model.Property):
    converter = cattrs.GenConverter()
    obj = converter.unstructure(prop_def, model.Property)
    return model.Property(**obj)


def get_all_extends(struct_def: model.Structure, spec) -> List[model.Structure]:
    extends = []
    for extend in struct_def.extends:
        extends.append(_get_struct(extend.name, spec))
        for struct in get_all_extends(_get_struct(extend.name, spec), spec):
            if not any(struct.name == e.name for e in extends):
                extends.append(struct)
    return extends


def get_all_properties(struct: model.Structure, spec) -> List[model.Structure]:
    properties = []
    for prop in struct.properties:
        properties.append(copy_property(prop))

    for extend in get_all_extends(struct, spec):
        for prop in get_all_properties(extend, spec):
            if not any(prop.name == p.name for p in properties):
                properties.append(copy_property(prop))

    if not all(mixin.kind == "reference" for mixin in struct.mixins):
        raise ValueError(f"Struct {struct.name} has non-reference mixins")
    for mixin in [_get_struct(mixin.name, spec) for mixin in struct.mixins]:
        for prop in get_all_properties(mixin, spec):
            if not any(prop.name == p.name for p in properties):
                properties.append(copy_property(prop))

    return properties


def generate_code_for_request(request: model.Request):
    lines = get_doc(request.documentation) + generate_extras(request)
    lines.append(
        f'public static string {lsp_method_to_name(request.method)} {{ get; }} = "{request.method}";'
    )
    return lines


def generate_code_for_notification(notify: model.Notification):
    lines = get_doc(notify.documentation) + generate_extras(notify)
    lines.append(
        f'public static string {lsp_method_to_name(notify.method)} {{ get; }} = "{notify.method}";'
    )
    return lines


def generate_request_notification_methods(spec: model.LSPModel, types: TypeData):
    inner_lines = []
    for request in spec.requests:
        inner_lines += generate_code_for_request(request)

    for notification in spec.notifications:
        inner_lines += generate_code_for_notification(notification)

    lines = namespace_wrapper(
        NAMESPACE,
        get_usings(["System"] + get_types_for_usings(inner_lines)),
        ["public static class LSPMethods", "{", *indent_lines(inner_lines), "}"],
    )
    enum_type = model.Enum(
        **{
            "name": "LSPMethods",
            "type": {"kind": "base", "name": "string"},
            "values": [],
            "documentation": "LSP methods as defined in the LSP spec",
        }
    )
    types.add_type_info(enum_type, "LSPMethods", lines)


def get_message_template(
    obj: Union[model.Request, model.Notification],
    is_request: bool,
) -> model.Structure:
    text = "Request" if is_request else "Notification"
    properties = [
        {
            "name": "jsonrpc",
            "type": {"kind": "stringLiteral", "value": "2.0"},
            "documentation": "The jsonrpc version.",
        }
    ]
    if is_request:
        properties += [
            {
                "name": "id",
                "type": {
                    "kind": "or",
                    "items": [
                        {"kind": "base", "name": "string"},
                        {"kind": "base", "name": "integer"},
                    ],
                },
                "documentation": f"The {text} id.",
            }
        ]
    properties += [
        {
            "name": "method",
            "type": {"kind": "base", "name": "string"},
            "documentation": f"The {text} method.",
        },
    ]
    if obj.params:
        properties.append(
            {
                "name": "params",
                "type": cattrs.unstructure(obj.params),
                "documentation": f"The {text} parameters.",
            }
        )
    else:
        properties.append(
            {
                "name": "params",
                "type": {"kind": "reference", "name": "LSPAny"},
                "documentation": f"The {text} parameters.",
                "optional": True,
            }
        )

    class_template = {
        "name": f"{lsp_method_to_name(obj.method)}{text}",
        "properties": properties,
        "documentation": obj.documentation,
        "since": obj.since,
        "deprecated": obj.deprecated,
        "proposed": obj.proposed,
    }
    return model.Structure(**class_template)


def get_response_template(
    obj: model.Request, spec: model.LSPModel, types: TypeData
) -> model.Structure:
    properties = [
        {
            "name": "jsonrpc",
            "type": {"kind": "stringLiteral", "value": "2.0"},
            "documentation": "The jsonrpc version.",
        },
        {
            "name": "id",
            "type": {
                "kind": "or",
                "items": [
                    {"kind": "base", "name": "string"},
                    {"kind": "base", "name": "integer"},
                ],
            },
            "documentation": "The Request id.",
        },
    ]
    if obj.result:
        properties.append(
            {
                "name": "result",
                "type": cattrs.unstructure(obj.result),
                "documentation": "Results for the request.",
                "optional": True,
            }
        )
    else:
        properties.append(
            {
                "name": "result",
                "type": {"kind": "base", "name": "null"},
                "documentation": "Results for the request.",
                "optional": True,
            }
        )
    properties.append(
        {
            "name": "error",
            "type": {"kind": "reference", "name": "ResponseError"},
            "documentation": "Error while handling the request.",
            "optional": True,
        }
    )
    class_template = {
        "name": f"{lsp_method_to_name(obj.method)}Response",
        "properties": properties,
        "documentation": obj.documentation,
        "since": obj.since,
        "deprecated": obj.deprecated,
        "proposed": obj.proposed,
    }
    return model.Structure(**class_template)


def get_registration_options_template(
    obj: Union[model.Request, model.Notification],
    spec: model.LSPModel,
    types: TypeData,
) -> model.Structure:
    if obj.registrationOptions and obj.registrationOptions.kind != "reference":
        if obj.registrationOptions.kind == "and":
            structs = [_get_struct(s.name, spec) for s in obj.registrationOptions.items]
            properties = []
            for struct in structs:
                properties += get_all_properties(struct, spec)

            class_template = {
                "name": f"{lsp_method_to_name(obj.method)}RegistrationOptions",
                "properties": [
                    cattrs.unstructure(p, model.Property) for p in properties
                ],
            }
            return model.Structure(**class_template)
        else:
            raise ValueError(
                f"Unexpected registrationOptions type: {obj.registrationOptions.type.kind}"
            )
    return None


def generate_all_classes(spec: model.LSPModel, types: TypeData):
    for struct in spec.structures:
        generate_class_from_struct(struct, spec, types)

    for type_alias in spec.typeAliases:
        if is_variant_type_alias(type_alias):
            generate_class_from_variant_type_alias(type_alias, spec, types)
        else:
            generate_class_from_type_alias(type_alias, spec, types)

    generate_request_notification_methods(spec, types)

    for request in spec.requests:
        partial_result_name = None
        if request.partialResult:
            partial_result_name = get_type_name(request.partialResult, types, spec)

        struct = get_message_template(request, is_request=True)
        generate_class_from_struct(
            struct,
            spec,
            types,
            (
                f"IRequest<{get_type_name(request.params, types, spec)}>"
                if request.params
                else "IRequest<LSPAny?>"
            ),
            [
                f"[Direction(MessageDirection.{to_upper_camel_case(request.messageDirection)})]",
                f'[LSPRequest("{request.method}", typeof({lsp_method_to_name(request.method)}Response), typeof({partial_result_name}))]'
                if partial_result_name
                else f'[LSPRequest("{request.method}", typeof({lsp_method_to_name(request.method)}Response))]',
            ],
        )
        response = get_response_template(request, spec, types)
        generate_class_from_struct(
            response,
            spec,
            types,
            f"IResponse<{get_type_name(request.result, types, spec)}>",
            [
                f"[LSPResponse(typeof({lsp_method_to_name(request.method)}Request))]",
            ],
        )
        registration_options = get_registration_options_template(request, spec, types)
        if registration_options:
            generate_class_from_struct(
                registration_options,
                spec,
                types,
            )

    for notification in spec.notifications:
        struct = get_message_template(notification, is_request=False)
        generate_class_from_struct(
            struct,
            spec,
            types,
            (
                f"INotification<{get_type_name(notification.params, types, spec)}>"
                if notification.params
                else "INotification<LSPAny>"
            ),
            [
                f"[Direction(MessageDirection.{to_upper_camel_case(request.messageDirection)})]",
            ],
        )
        registration_options = get_registration_options_template(
            notification, spec, types
        )
        if registration_options:
            generate_class_from_struct(
                registration_options,
                spec,
                types,
            )
