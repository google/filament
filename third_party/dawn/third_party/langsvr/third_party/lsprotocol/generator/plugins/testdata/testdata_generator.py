# Copyright (c) Microsoft Corporation. All rights reserved.
# Licensed under the MIT License.

import hashlib
import itertools
import json
import logging
import re
from copy import deepcopy
from typing import Any, List, Optional

import generator.model as model

LSP_MAX_INT = 2**31 - 1
LSP_MIN_INT = -(2**31)

LSP_MAX_UINT = 2**31 - 1
LSP_MIN_UINT = 0

LSP_OVER_MAX_INT = 2**31
LSP_UNDER_MIN_INT = -(2**31) - 1

LSP_OVER_MAX_UINT = 2**32 + 1
LSP_UNDER_MIN_UINT = -1


def get_hash_from(text: str) -> str:
    return hashlib.sha256(text.encode("utf-8")).hexdigest()


def request_variants(method: str):
    for id_value in [1, LSP_MAX_INT, LSP_MIN_INT, "string-id-1"]:
        yield True, {"jsonrpc": "2.0", "id": id_value, "method": method}
    for id_value in [LSP_OVER_MAX_INT, LSP_UNDER_MIN_INT, 1.0, True, None]:
        yield False, {"jsonrpc": "2.0", "id": id_value, "method": method}
    yield False, {"jsonrpc": "2.0", "method": method}
    yield False, {"jsonrpc": "2.0", "id": 1}
    yield False, {"id": 1, "method": method}


def response_variants():
    for id_value in [1, LSP_MAX_INT, LSP_MIN_INT, "string-id-1"]:
        yield True, {"jsonrpc": "2.0", "id": id_value}
    for id_value in [LSP_OVER_MAX_INT, LSP_UNDER_MIN_INT, 1.0, True, None]:
        yield False, {"jsonrpc": "2.0", "id": id_value}
    yield False, {"jsonrpc": "2.0"}
    yield False, {"id": 1}


def notify_variants(method: str):
    yield True, {"jsonrpc": "2.0", "method": method}
    yield False, {"jsonrpc": "2.0", "id": 1, "method": method}


def _get_struct(name: str, spec: model.LSPModel) -> Optional[model.Structure]:
    for struct in spec.structures:
        if struct.name == name:
            return struct
    return None


def _get_type_alias(name: str, spec: model.LSPModel) -> Optional[model.TypeAlias]:
    for alias in spec.typeAliases:
        if alias.name == name:
            return alias
    return None


def _get_enum(name: str, spec: model.LSPModel) -> Optional[model.Enum]:
    for enum in spec.enumerations:
        if enum.name == name:
            return enum
    return None


class Ignore:
    pass


def extend(arr: List[Any], length: int) -> List[Any]:
    result = arr * (length // len(arr)) + arr[: length % len(arr)]
    return result


def extend_all(lists: List[List[Any]]) -> List[List[Any]]:
    max_len = max(len(part) for part in lists)
    max_len = min(1000, max_len)
    return [extend(part, max_len) for part in lists]


def has_null_base_type(items: List[model.LSP_TYPE_SPEC]) -> bool:
    return any(item.kind == "base" and item.name == "null" for item in items)


def filter_null_base_type(
    items: List[model.LSP_TYPE_SPEC],
) -> List[model.LSP_TYPE_SPEC]:
    return [item for item in items if not (item.kind == "base" and item.name == "null")]


def generate_for_base(name: str):
    if name == "string":
        yield (True, "some string 🐍🐜")
        yield (True, "")
    elif name == "integer":
        yield (True, 1)
        yield (True, LSP_MAX_INT)
        yield (True, LSP_MIN_INT)
        yield (False, LSP_OVER_MAX_INT)
        yield (False, LSP_UNDER_MIN_INT)
    elif name == "decimal":
        yield (True, 1.0)
    elif name == "boolean":
        yield (True, True)
        yield (True, False)
    elif name == "null":
        yield (True, None)
    elif name == "uinteger":
        yield (True, 1)
        yield (True, LSP_MAX_UINT)
        yield (True, LSP_MIN_UINT)
        yield (False, LSP_OVER_MAX_UINT)
        yield (False, LSP_UNDER_MIN_UINT)
    elif name in ["URI", "DocumentUri"]:
        yield (True, "file:///some/path")
    elif name == "RegExp":
        yield (True, ".*")


def generate_for_array(
    type_def: model.LSP_TYPE_SPEC, spec: model.LSPModel, visited: List[str]
):
    generated = list(generate_for_type(type_def, spec, visited))
    yield (True, [])

    # array with 1 item
    for valid, value in generated:
        if not isinstance(value, Ignore):
            yield (valid, [value])

    # array with 2 items
    generated = generated[:100] if len(generated) > 100 else generated
    values = itertools.product(generated, repeat=2)
    for (valid1, value1), (valid2, value2) in values:
        if not isinstance(value1, Ignore) and not isinstance(value2, Ignore):
            yield (valid1 and valid2, [value1, value2])


def generate_for_tuple(
    type_defs: List[model.LSP_TYPE_SPEC], spec: model.LSPModel, visited: List[str]
):
    generated = [
        list(generate_for_type(type_def, spec, visited)) for type_def in type_defs
    ]
    products = zip(*extend_all(generated))

    for product in products:
        is_valid = all(valid for valid, _ in product)
        values = [value for _, value in product if not isinstance(value, Ignore)]
        yield (is_valid, tuple(values))


def generate_for_map(
    key_def: model.LSP_TYPE_SPEC,
    value_def: model.LSP_TYPE_SPEC,
    spec: model.LSPModel,
    visited: List[str],
):
    key_values = list(generate_for_type(key_def, spec, visited))
    value_values = list(generate_for_type(value_def, spec, visited))

    for key_valid, key_value in key_values:
        for value_valid, value_value in value_values:
            if not (isinstance(key_value, Ignore) or isinstance(value_value, Ignore)):
                yield (key_valid and value_valid, {key_value: value_value})


def get_all_extends(struct_def: model.Structure, spec) -> List[model.Structure]:
    extends = []
    for extend in struct_def.extends:
        extends.append(_get_struct(extend.name, spec))
        for struct in get_all_extends(_get_struct(extend.name, spec), spec):
            if not any(struct.name == e.name for e in extends):
                extends.append(struct)
    return extends


def get_all_properties(struct: model.Structure, spec) -> List[model.Property]:
    properties = []
    for prop in struct.properties:
        properties.append(prop)

    for extend in get_all_extends(struct, spec):
        for prop in get_all_properties(extend, spec):
            if not any(prop.name == p.name for p in properties):
                properties.append(prop)

    if not all(mixin.kind == "reference" for mixin in struct.mixins):
        raise ValueError(f"Struct {struct.name} has non-reference mixins")
    for mixin in [_get_struct(mixin.name, spec) for mixin in struct.mixins]:
        for prop in get_all_properties(mixin, spec):
            if not any(prop.name == p.name for p in properties):
                properties.append(prop)

    return properties


def generate_for_property(
    prop: model.Property, spec: model.LSPModel, visited: List[str]
):
    if prop.optional:
        yield (True, Ignore())
    yield from generate_for_type(prop.type, spec, visited)


def generate_for_reference(
    refname: str,
    spec: model.LSPModel,
    visited: List[str],
):
    if len([name for name in visited if name == refname]) > 2:
        return

    ref = _get_struct(refname, spec)
    alias = _get_type_alias(refname, spec)
    enum = _get_enum(refname, spec)
    if ref:
        properties = get_all_properties(ref, spec)
        if properties:
            names = [prop.name for prop in properties]
            value_variants = [
                list(generate_for_property(prop, spec, visited)) for prop in properties
            ]

            products = zip(*extend_all(value_variants))
            for variants in products:
                is_valid = all(valid for valid, _ in variants)
                values = [value for _, value in variants]
                variant = {
                    name: value
                    for name, value in zip(names, values)
                    if not isinstance(value, Ignore)
                }
                yield (is_valid, variant)
        else:
            yield (True, {"lspExtension": "some value"})
            yield (True, dict())
    elif alias:
        if refname in ["LSPObject", "LSPAny", "LSPArray"]:
            yield from (
                (True, value)
                for _, value in generate_for_type(alias.type, spec, visited)
            )
        else:
            yield from generate_for_type(alias.type, spec, visited)
    elif enum:
        value = enum.values[0].value
        yield (True, value)
        if isinstance(value, int):
            yield (bool(enum.supportsCustomValues), 12345)
        elif isinstance(value, str):
            yield (bool(enum.supportsCustomValues), "testCustomValue")
    else:
        raise ValueError(f"Unknown reference {refname}")


def generate_for_or(
    type_defs: List[model.LSP_TYPE_SPEC],
    spec: model.LSPModel,
    visited: List[str],
):
    if has_null_base_type(type_defs):
        yield (True, None)

    subset = filter_null_base_type(type_defs)
    generated = [
        list(generate_for_type(type_def, spec, visited)) for type_def in subset
    ]
    for valid, value in itertools.chain(*generated):
        yield (valid, value)


def generate_for_and(
    type_defs: List[model.LSP_TYPE_SPEC],
    spec: model.LSPModel,
    visited: List[str],
):
    generated = [
        list(generate_for_type(type_def, spec, visited)) for type_def in type_defs
    ]
    products = zip(*extend_all(generated))
    for variants in products:
        is_valid = all(valid for valid, _ in variants)
        values = [value for _, value in variants]
        variant = {}
        for value in values:
            variant.update(value)
        yield (is_valid, variant)


def generate_for_literal(
    type_def: model.LiteralType,
    spec: model.LSPModel,
    visited: List[str],
):
    if type_def.value.properties:
        names = [prop.name for prop in type_def.value.properties]
        value_variants = [
            list(generate_for_type(prop.type, spec, visited))
            for prop in type_def.value.properties
        ]

        products = zip(*extend_all(value_variants))
        for variants in products:
            is_valid = all(valid for valid, _ in variants)
            values = [value for _, value in variants]
            variant = {name: value for name, value in zip(names, values)}
            yield (is_valid, variant)
    else:
        # Literal with no properties are a way to extend LSP spec
        # see: https://github.com/microsoft/vscode-languageserver-node/issues/997
        yield (True, {"lspExtension": "some value"})
        yield (True, dict())


def generate_for_type(
    type_def: model.LSP_TYPE_SPEC,
    spec: model.LSPModel,
    visited: List[str],
):
    if type_def is None:
        yield (True, None)
    elif type_def.kind == "base":
        yield from generate_for_base(type_def.name)
    elif type_def.kind == "array":
        yield from generate_for_array(type_def.element, spec, visited)
    elif type_def.kind == "reference":
        yield from generate_for_reference(
            type_def.name, spec, visited + [type_def.name]
        )
    elif type_def.kind == "stringLiteral":
        yield (True, type_def.value)
        # yield (False, f"invalid@{type_def.value}")
    elif type_def.kind == "tuple":
        yield from generate_for_tuple(type_def.items, spec, visited)
    elif type_def.kind == "or":
        yield from generate_for_or(type_def.items, spec, visited)
    elif type_def.kind == "and":
        yield from generate_for_and(type_def.items, spec, visited)
    elif type_def.kind == "literal":
        yield from generate_for_literal(type_def, spec, visited)
    elif type_def.kind == "map":
        yield from generate_for_map(type_def.key, type_def.value, spec, visited)


def generate_requests(request: model.Request, spec: model.LSPModel):
    variants = zip(
        *extend_all(
            [
                list(request_variants(request.method)),
                list(generate_for_type(request.params, spec, [])),
            ]
        )
    )

    for (valid1, base), (valid2, params) in variants:
        valid = valid1 and valid2
        if isinstance(params, Ignore):
            yield (valid, base)
        else:
            message = deepcopy(base)
            message.update({"params": params})
            yield (valid1 and valid2, message)


def generate_notifications(notify: model.Notification, spec: model.LSPModel) -> None:
    variants = zip(
        *extend_all(
            [
                list(notify_variants(notify.method)),
                list(generate_for_type(notify.params, spec, [])),
            ]
        )
    )

    for (valid1, base), (valid2, params) in variants:
        valid = valid1 and valid2
        if isinstance(params, Ignore):
            yield (valid, base)
        else:
            message = deepcopy(base)
            message.update({"params": params})
            yield (valid1 and valid2, message)


RESPONSE_ERROR = model.Structure(
    **{
        "name": "ResponseError",
        "properties": [
            {
                "name": "code",
                "type": {"kind": "base", "name": "integer"},
            },
            {
                "name": "message",
                "type": {"kind": "base", "name": "string"},
            },
            {
                "name": "data",
                "type": {"kind": "reference", "name": "LSPObject"},
                "optional": True,
            },
        ],
    }
)


def generate_responses(request: model.Request, spec: model.LSPModel) -> None:
    variants = zip(
        *extend_all(
            [
                list(response_variants()),
                list(generate_for_type(request.result, spec, [])),
                list(
                    generate_for_type(
                        model.ReferenceType("reference", "ResponseError"), spec, []
                    )
                ),
            ]
        )
    )

    for (valid1, base), (valid2, result), (valid3, error) in variants:
        valid = valid1 and valid2 and valid3
        if isinstance(result, Ignore):
            yield (valid, base)
        else:
            message = deepcopy(base)
            message.update({"result": result})
            message.update({"error": error})
            yield (valid, message)


PARTS_RE = re.compile(r"(([a-z0-9])([A-Z]))")


def get_parts(name: str) -> List[str]:
    name = name.replace("_", " ")
    return PARTS_RE.sub(r"\2 \3", name).split()


def to_upper_camel_case(name: str) -> str:
    return "".join([c.capitalize() for c in get_parts(name)])


def lsp_method_to_name(method: str) -> str:
    if method.startswith("$"):
        method = method[1:]
    method = method.replace("/", "_")
    return to_upper_camel_case(method)


def generate(spec: model.LSPModel, logger: logging.Logger):
    spec.structures.append(RESPONSE_ERROR)
    testdata = {}
    for request in spec.requests:
        counter = 0
        for valid, value in generate_requests(request, spec):
            content = json.dumps(value, indent=4, ensure_ascii=False)
            name = f"{lsp_method_to_name(request.method)}Request-{valid}-{get_hash_from(content)}.json"
            if name in testdata:
                continue
            testdata[name] = content
            counter += 1
        logger.info(f"Generated {counter} variants for Request: {request.method}")

        for valid, value in generate_responses(request, spec):
            content = json.dumps(value, indent=4, ensure_ascii=False)
            name = f"{lsp_method_to_name(request.method)}Response-{valid}-{get_hash_from(content)}.json"
            if name in testdata:
                continue
            testdata[name] = content
            counter += 1
        logger.info(f"Generated {counter} variants for Response: {request.method}")

    for notify in spec.notifications:
        counter = 0
        for valid, value in generate_notifications(notify, spec):
            content = json.dumps(value, indent=4, ensure_ascii=False)
            name = f"{lsp_method_to_name(notify.method)}Notification-{valid}-{get_hash_from(content)}.json"
            if name in testdata:
                continue
            testdata[name] = content
            counter += 1
        logger.info(f"Generated {counter} variants for Notification: {notify.method}")

    logger.info(f"Generated {len(testdata)} test variants")
    return testdata
