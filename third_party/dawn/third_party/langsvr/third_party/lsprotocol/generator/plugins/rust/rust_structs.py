# Copyright (c) Microsoft Corporation. All rights reserved.
# Licensed under the MIT License.

from typing import Dict, Iterable, List, Optional

import generator.model as model

from .rust_commons import (
    TypeData,
    fix_lsp_method_name,
    generate_extras,
    generate_literal_struct_name,
    generate_property,
    get_extended_properties,
    get_from_name,
    get_message_type_name,
    get_type_name,
    struct_wrapper,
    type_alias_wrapper,
)
from .rust_lang_utils import get_parts, lines_to_doc_comments, to_upper_camel_case


def generate_type_aliases(spec: model.LSPModel, types: TypeData) -> None:
    for alias in spec.typeAliases:
        if not types.has_id(alias):
            generate_type_alias(alias, types, spec)


def _get_doc(doc: Optional[str]) -> str:
    if doc:
        return lines_to_doc_comments(doc.splitlines(keepends=False))
    return []


def _is_some_array_type(items: Iterable[model.LSP_TYPE_SPEC]) -> bool:
    items_list = list(items)
    assert len(items_list) == 2
    item1, item2 = items_list

    if item1.kind == "array" and item2.kind == "reference":
        return item1.element.kind == "reference" and item1.element.name == item2.name

    if item2.kind == "array" and item1.kind == "reference":
        return item2.element.kind == "reference" and item2.element.name == item1.name
    return False


def _get_some_array_code(
    items: Iterable[model.LSP_TYPE_SPEC],
    types: Dict[str, List[str]],
    spec: model.LSPModel,
) -> List[str]:
    assert _is_some_array_type(items)
    items_list = list(items)
    item1 = items_list[0]
    item2 = items_list[1]

    if item1.kind == "array" and item2.kind == "reference":
        return [
            f"    One({get_type_name(item2, types, spec)}),",
            f"    Many({get_type_name(item1, types, spec)}),",
        ]

    if item2.kind == "array" and item1.kind == "reference":
        return [
            f"    One({get_type_name(item1, types, spec)}),",
            f"    Many({get_type_name(item2, types, spec)}),",
        ]
    return []


def _get_common_name(items: Iterable[model.LSP_TYPE_SPEC], kind: str) -> List[str]:
    names = [get_parts(item.name) for item in list(items) if item.kind == kind]
    if len(names) < 2:
        return []

    smallest = min(names, key=len)
    common = []
    for i in range(len(smallest)):
        if all(name[i] == smallest[i] for name in names):
            common.append(smallest[i])
    return common


def _is_all_reference_similar_type(alias: model.TypeAlias) -> bool:
    items_list = list(alias.type.items)
    return all(item.kind in ["reference", "base", "literal"] for item in items_list)


def _get_all_reference_similar_code(
    alias: model.TypeAlias,
    types: TypeData,
    spec: model.LSPModel,
) -> List[str]:
    items = alias.type.items
    assert _is_all_reference_similar_type(alias)

    # Ensure all literal types have a name
    for item in list(items):
        if item.kind == "literal":
            get_type_name(item, types, spec, None, alias.name)

    common_name = [
        i.lower()
        for i in (
            _get_common_name(items, "reference")
            + _get_common_name(items, "literal")
            + ["struct"]
        )
    ]

    lines = []
    value = 0
    field_names = []
    for item in list(items):
        if item.kind == "base" and item.name == "null":
            lines += ["None,"]
            field_names += ["None"]
        elif item.kind == "base":
            name = _base_to_field_name(item.name)
            lines += [f"{name}({get_type_name(item, types, spec)}),"]
            field_names += [name]
        elif item.kind == "reference":
            name = [
                part for part in get_parts(item.name) if part.lower() not in common_name
            ]
            if len(name) == 0:
                name = [f"Value{value}"]
                value += 1
            common_name += [n.lower() for n in name]
            name = to_upper_camel_case("".join(name))
            field_names += [name]
            lines += [f"{name}({get_type_name(item, types, spec)}),"]
        elif item.kind == "literal":
            name = [
                part for part in get_parts(item.name) if part.lower() not in common_name
            ]
            optional_props = [p for p in item.value.properties if p.optional]
            required_props = [p for p in item.value.properties if not p.optional]

            # Try picking a name using required props first and then optional props
            if len(name) == 0:
                for p in required_props + optional_props:
                    name = [
                        part
                        for part in get_parts(p.name)
                        if part.lower() not in common_name
                    ]
                    if len(name) != 0:
                        break

            # If we still don't have a name, then try picking a name using required props
            # and then optional props without checking for common name list. But check
            # that the name is not already used.
            if len(name) == 0:
                for p in required_props + optional_props:
                    if to_upper_camel_case(p.name) not in field_names:
                        name = get_parts(p.name)
                        break

            # If we still don't have a name, then just use a generic "Value{int}" as name
            if len(name) == 0:
                name = [f"Value{value}"]
                value += 1
            common_name += [n.lower() for n in name]
            name = to_upper_camel_case("".join(name))
            field_names += [name]
            lines += [f"{name}({item.name}),"]
        else:
            raise ValueError(f"Unknown type {item}")
    return lines


def _base_to_field_name(base_name: str) -> str:
    if base_name == "boolean":
        return "Bool"
    if base_name == "integer":
        return "Int"
    if base_name == "decimal":
        return "Real"
    if base_name == "string":
        return "String"
    if base_name == "uinteger":
        return "UInt"
    if base_name == "null":
        return "None"
    raise ValueError(f"Unknown base type {base_name}")


def _get_literal_field_name(literal: model.LiteralType, types: TypeData) -> str:
    properties = list(literal.value.properties)

    if len(properties) == 1 and properties[0].kind == "base":
        return _base_to_field_name(properties[0].name)

    if len(properties) == 1 and properties[0].kind == "reference":
        return to_upper_camel_case(properties[0].name)

    return generate_literal_struct_name(literal, types)


def _generate_or_type_alias(
    alias_def: model.TypeAlias, types: Dict[str, List[str]], spec: model.LSPModel
) -> List[str]:
    inner = []

    if len(alias_def.type.items) == 2 and _is_some_array_type(alias_def.type.items):
        inner += _get_some_array_code(alias_def.type.items, types, spec)
    elif _is_all_reference_similar_type(alias_def):
        inner += _get_all_reference_similar_code(alias_def, types, spec)
    else:
        index = 0

        for sub_type in alias_def.type.items:
            if sub_type.kind == "base" and sub_type.name == "null":
                inner += ["None,"]
            else:
                inner += [f"ValueType{index}({get_type_name(sub_type, types, spec)}),"]
            index += 1
    return type_alias_wrapper(alias_def, inner)


def generate_type_alias(
    alias_def: model.TypeAlias, types: TypeData, spec: model.LSPModel
) -> List[str]:
    doc = _get_doc(alias_def.documentation)
    doc += generate_extras(alias_def)

    lines = []
    if alias_def.type.kind == "reference":
        lines += doc
        lines += [f"pub type {alias_def.name} = {alias_def.type.name};"]
    elif alias_def.type.kind == "array":
        lines += doc
        lines += [
            f"pub type {alias_def.name} = {get_type_name(alias_def.type, types, spec)};"
        ]
    elif alias_def.type.kind == "or":
        lines += _generate_or_type_alias(alias_def, types, spec)
    elif alias_def.type.kind == "and":
        raise ValueError("And type not supported")
    elif alias_def.type.kind == "literal":
        lines += doc
        lines += [
            f"pub type {alias_def.name} = {get_type_name(alias_def.type, types, spec)};"
        ]
    elif alias_def.type.kind == "base":
        lines += doc
        lines += [
            f"pub type {alias_def.name} = {get_type_name(alias_def.type, types, spec)};"
        ]
    else:
        pass

    types.add_type_info(alias_def, alias_def.name, lines)


def generate_structures(spec: model.LSPModel, types: TypeData) -> Dict[str, List[str]]:
    for struct in spec.structures:
        if not types.has_id(struct):
            generate_struct(struct, types, spec)
    return types


def generate_struct(
    struct_def: model.Structure, types: TypeData, spec: model.LSPModel
) -> None:
    inner = []
    for prop_def in get_extended_properties(struct_def, spec):
        inner += generate_property(prop_def, types, spec)

    lines = struct_wrapper(struct_def, inner)
    types.add_type_info(struct_def, struct_def.name, lines)


def generate_notifications(
    spec: model.LSPModel, types: TypeData
) -> Dict[str, List[str]]:
    for notification in spec.notifications:
        if not types.has_id(notification):
            generate_notification(notification, types, spec)
    return types


def required_rpc_properties(name: Optional[str] = None) -> List[model.Property]:
    props = [
        model.Property(
            name="jsonrpc",
            type=model.BaseType(kind="base", name="string"),
            optional=False,
            documentation="The version of the JSON RPC protocol.",
        ),
    ]
    if name:
        props += [
            model.Property(
                name="method",
                type=model.ReferenceType(kind="reference", name=name),
                optional=False,
                documentation="The method to be invoked.",
            ),
        ]
    return props


def generate_notification(
    notification_def: model.Notification, types: TypeData, spec: model.LSPModel
) -> None:
    properties = required_rpc_properties("LSPNotificationMethods")
    if notification_def.params:
        ptype = get_from_name(notification_def.params.name, spec)
        if hasattr(ptype, "properties") and get_extended_properties(ptype, spec):
            properties += [
                model.Property(
                    name="params",
                    type=notification_def.params,
                )
            ]
        else:
            properties += [
                model.Property(
                    name="params",
                    type=model.ReferenceType(kind="reference", name="LSPAny"),
                    optional=True,
                )
            ]
    else:
        properties += [
            model.Property(
                name="params",
                type=model.ReferenceType(kind="reference", name="LSPNull"),
                optional=True,
            )
        ]

    inner = []
    for prop_def in properties:
        inner += generate_property(prop_def, types, spec)

    lines = struct_wrapper(notification_def, inner)
    types.add_type_info(
        notification_def, get_message_type_name(notification_def), lines
    )


def generate_required_request_types(
    spec: model.LSPModel, types: TypeData
) -> Dict[str, List[str]]:
    lsp_id = model.TypeAlias(
        name="LSPId",
        documentation="An identifier to denote a specific request.",
        type=model.OrType(
            kind="or",
            items=[
                model.BaseType(kind="base", name="integer"),
                model.BaseType(kind="base", name="string"),
            ],
        ),
    )
    generate_type_alias(lsp_id, types, spec)

    lsp_id_optional = model.TypeAlias(
        name="LSPIdOptional",
        documentation="An identifier to denote a specific response.",
        type=model.OrType(
            kind="or",
            items=[
                model.BaseType(kind="base", name="integer"),
                model.BaseType(kind="base", name="string"),
                model.BaseType(kind="base", name="null"),
            ],
        ),
    )
    generate_type_alias(lsp_id_optional, types, spec)


def generate_requests(spec: model.LSPModel, types: TypeData) -> Dict[str, List[str]]:
    generate_required_request_types(spec, types)
    for request in spec.requests:
        if not types.has_id(request):
            generate_request(request, types, spec)
            generate_response(request, types, spec)
            generate_partial_result(request, types, spec)
            generate_registration_options(request, types, spec)
    return types


def generate_request(
    request_def: model.Request, types: TypeData, spec: model.LSPModel
) -> None:
    properties = required_rpc_properties("LSPRequestMethods")

    properties += [
        model.Property(
            name="id",
            type=model.ReferenceType(kind="reference", name="LSPId"),
            optional=False,
            documentation="The request id.",
        )
    ]
    if request_def.params:
        ptype = get_from_name(request_def.params.name, spec)
        if hasattr(ptype, "properties") and get_extended_properties(ptype, spec):
            properties += [
                model.Property(
                    name="params",
                    type=request_def.params,
                )
            ]
        else:
            properties += [
                model.Property(
                    name="params",
                    type=model.ReferenceType(kind="reference", name="LSPAny"),
                    optional=True,
                )
            ]
    else:
        properties += [
            model.Property(
                name="params",
                type=model.ReferenceType(kind="reference", name="LSPNull"),
                optional=True,
            )
        ]

    inner = []
    for prop_def in properties:
        inner += generate_property(prop_def, types, spec)

    lines = struct_wrapper(request_def, inner)
    types.add_type_info(request_def, get_message_type_name(request_def), lines)


def generate_response(
    request_def: model.Request, types: TypeData, spec: model.LSPModel
) -> None:
    properties = required_rpc_properties("LSPRequestMethods")
    properties += [
        model.Property(
            name="id",
            type=model.ReferenceType(kind="reference", name="LSPIdOptional"),
            optional=False,
            documentation="The request id.",
        )
    ]
    if request_def.result:
        if request_def.result.kind == "base" and request_def.result.name == "null":
            properties += [
                model.Property(
                    name="result",
                    type=model.ReferenceType(kind="reference", name="LSPNull"),
                )
            ]
        else:
            properties += [
                model.Property(
                    name="result",
                    type=request_def.result,
                )
            ]
    name = fix_lsp_method_name(request_def.method)
    response_def = model.Structure(
        name=f"{name}Response",
        documentation=f"Response to the [{name}Request].",
        properties=properties,
        since=request_def.since,
        deprecated=request_def.deprecated,
    )

    inner = []
    for prop_def in properties:
        inner += generate_property(prop_def, types, spec)

    lines = struct_wrapper(response_def, inner)
    types.add_type_info(response_def, response_def.name, lines)


def generate_partial_result(
    request_def: model.Request, types: TypeData, spec: model.LSPModel
) -> None:
    if not request_def.partialResult:
        return

    if request_def.partialResult.kind not in ["and", "or"]:
        return


def generate_registration_options(
    request_def: model.Request, types: TypeData, spec: model.LSPModel
) -> None:
    if not request_def.registrationOptions:
        return

    if request_def.registrationOptions.kind not in ["and", "or"]:
        return
