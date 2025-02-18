# Copyright (c) Microsoft Corporation. All rights reserved.
# Licensed under the MIT License.

from __future__ import annotations

import uuid
from typing import Any, Dict, Iterable, List, Optional, Type, Union

import attrs

LSP_TYPE_SPEC = Union[
    "BaseType",
    "ReferenceType",
    "OrType",
    "AndType",
    "ArrayType",
    "LiteralType",
    "StringLiteralType",
    "MapType",
    "TupleType",
]


def partial_apply(callable):
    def apply(x):
        if isinstance(x, dict):
            return callable(**x)
        else:
            return x

    return apply


def list_converter(callable):
    def apply(x):
        if isinstance(x, dict):
            return callable(**x)
        else:
            return x

    def converter(x: List[Any]) -> List[Any]:
        return list(map(apply, x))

    return converter


def enum_validator(instance: Enum, attribute: Any, value: Any) -> None:
    test_type = str if instance.type.name == "string" else int
    for e in value:
        if not isinstance(e.value, test_type):
            raise ValueError(
                f"Value of {instance.name}.{e.name} is not of type {test_type}: {e.value}."
            )


def convert_to_lsp_type(**type_info) -> Optional[LSP_TYPE_SPEC]:
    if type_info is None:
        return None
    lut: Dict[str, LSP_TYPE_SPEC] = {
        "base": BaseType,
        "reference": ReferenceType,
        "array": ArrayType,
        "or": OrType,
        "and": AndType,
        "literal": LiteralType,
        "map": MapType,
        "stringLiteral": StringLiteralType,
        "tuple": TupleType,
    }
    callable = lut.get(type_info["kind"])
    if callable:
        return callable(**type_info)

    raise ValueError(f"Unknown LSP type: {type_info}")


def type_validator(instance: Any, attribute: str, value: Any) -> bool:
    return isinstance(
        value,
        (
            BaseType,
            ReferenceType,
            ArrayType,
            OrType,
            LiteralType,
            AndType,
            MapType,
            StringLiteralType,
            TupleType,
        ),
    )


@attrs.define
class EnumItem:
    name: str = attrs.field(validator=attrs.validators.instance_of(str))
    value: Union[str, int] = attrs.field(
        validator=attrs.validators.instance_of((str, int))
    )
    proposed: Optional[bool] = attrs.field(
        validator=attrs.validators.optional(attrs.validators.instance_of(bool)),
        default=None,
    )
    documentation: Optional[str] = attrs.field(
        validator=attrs.validators.optional(attrs.validators.instance_of(str)),
        default=None,
    )
    since: Optional[str] = attrs.field(
        validator=attrs.validators.optional(attrs.validators.instance_of(str)),
        default=None,
    )
    deprecated: Optional[str] = attrs.field(
        validator=attrs.validators.optional(attrs.validators.instance_of(str)),
        default=None,
    )
    id_: Optional[str] = attrs.field(
        converter=lambda x: str(uuid.uuid4()),
        validator=attrs.validators.optional(attrs.validators.instance_of(str)),
        default=None,
    )

    def __eq__(self, other: Any) -> bool:
        if isinstance(other, EnumItem):
            return self.name == other.name and self.value == other.value
        return False

    def get_inner_types(self) -> List[Type]:
        return []


@attrs.define
class EnumValueType:
    kind: str = attrs.field(validator=attrs.validators.in_(["base"]))
    name: str = attrs.field(
        validator=attrs.validators.in_(["string", "integer", "uinteger"])
    )
    id_: Optional[str] = attrs.field(
        converter=lambda x: str(uuid.uuid4()),
        validator=attrs.validators.optional(attrs.validators.instance_of(str)),
        default=None,
    )

    def __eq__(self, other: object) -> bool:
        if isinstance(other, EnumValueType):
            return self.name == other.name and self.kind == other.kind
        return False

    def get_inner_types(self) -> List[Type]:
        return []


@attrs.define
class Enum:
    name: str = attrs.field(validator=attrs.validators.instance_of(str))
    type: EnumValueType = attrs.field(converter=partial_apply(EnumValueType))
    values: Iterable[EnumItem] = attrs.field(
        validator=enum_validator,
        converter=list_converter(EnumItem),
    )
    proposed: Optional[bool] = attrs.field(
        validator=attrs.validators.optional(attrs.validators.instance_of(bool)),
        default=None,
    )
    documentation: Optional[str] = attrs.field(
        validator=attrs.validators.optional(attrs.validators.instance_of(str)),
        default=None,
    )
    since: Optional[str] = attrs.field(
        validator=attrs.validators.optional(attrs.validators.instance_of(str)),
        default=None,
    )
    supportsCustomValues: Optional[bool] = attrs.field(
        validator=attrs.validators.optional(attrs.validators.instance_of(bool)),
        default=None,
    )
    deprecated: Optional[str] = attrs.field(
        validator=attrs.validators.optional(attrs.validators.instance_of(str)),
        default=None,
    )
    id_: Optional[str] = attrs.field(
        converter=lambda x: str(uuid.uuid4()),
        validator=attrs.validators.optional(attrs.validators.instance_of(str)),
        default=None,
    )

    def __eq__(self, other: object) -> bool:
        if isinstance(other, Enum):
            return (
                self.name == other.name
                and self.type == other.type
                and self.values == other.values
            )
        return False

    def get_inner_types(self) -> List[Type]:
        types = [self.type]
        for value in self.values:
            types.append(value)
            types.extend(value.get_inner_types())
        return types


@attrs.define
class BaseType:
    kind: str = attrs.field(validator=attrs.validators.in_(["base"]))
    name: str = attrs.field(
        validator=attrs.validators.in_(
            [
                "URI",
                "DocumentUri",
                "integer",
                "uinteger",
                "decimal",
                "RegExp",
                "string",
                "boolean",
                "null",
            ]
        )
    )
    id_: Optional[str] = attrs.field(
        converter=lambda x: str(uuid.uuid4()),
        validator=attrs.validators.optional(attrs.validators.instance_of(str)),
        default=None,
    )

    def __eq__(self, other: object) -> bool:
        if isinstance(other, BaseType):
            return self.name == other.name and self.kind == other.kind
        return False

    def get_inner_types(self) -> List[Type]:
        return []


@attrs.define
class ReferenceType:
    kind: str = attrs.field(validator=attrs.validators.in_(["reference"]))
    name: str = attrs.field(validator=attrs.validators.instance_of(str))
    id_: Optional[str] = attrs.field(
        converter=lambda x: str(uuid.uuid4()),
        validator=attrs.validators.optional(attrs.validators.instance_of(str)),
        default=None,
    )

    def __eq__(self, other: object) -> bool:
        if isinstance(other, ReferenceType):
            return self.name == other.name and self.kind == other.kind
        return False

    def get_inner_types(self) -> List[Type]:
        return []


@attrs.define
class StringLiteralType:
    kind: str = attrs.field(validator=attrs.validators.in_(["stringLiteral"]))
    value: str = attrs.field(validator=attrs.validators.instance_of(str))
    id_: Optional[str] = attrs.field(
        converter=lambda x: str(uuid.uuid4()),
        validator=attrs.validators.optional(attrs.validators.instance_of(str)),
        default=None,
    )

    def __eq__(self, other: object) -> bool:
        if isinstance(other, StringLiteralType):
            return self.value == other.value and self.kind == other.kind
        return False

    def get_inner_types(self) -> List[Type]:
        return []


@attrs.define
class OrType:
    kind: str = attrs.field(validator=attrs.validators.in_(["or"]))
    items: Iterable[LSP_TYPE_SPEC] = attrs.field(
        converter=list_converter(convert_to_lsp_type)
    )
    id_: Optional[str] = attrs.field(
        converter=lambda x: str(uuid.uuid4()),
        validator=attrs.validators.optional(attrs.validators.instance_of(str)),
        default=None,
    )

    def __eq__(self, other: object) -> bool:
        if isinstance(other, OrType):
            return self.items == other.items and self.kind == other.kind
        return False

    def get_inner_types(self) -> List[Type]:
        types = []
        for item in self.items:
            types.append(item)
            types.extend(item.get_inner_types())
        return types


@attrs.define
class AndType:
    kind: str = attrs.field(validator=attrs.validators.in_(["and"]))
    items: Iterable[LSP_TYPE_SPEC] = attrs.field(
        converter=list_converter(convert_to_lsp_type),
    )
    id_: Optional[str] = attrs.field(
        converter=lambda x: str(uuid.uuid4()),
        validator=attrs.validators.optional(attrs.validators.instance_of(str)),
        default=None,
    )

    def __eq__(self, other: object) -> bool:
        if isinstance(other, AndType):
            return self.items == other.items and self.kind == other.kind
        return False

    def get_inner_types(self) -> List[Type]:
        types = []
        for item in self.items:
            types.append(item)
            types.extend(item.get_inner_types())
        return types


@attrs.define
class ArrayType:
    kind: str = attrs.field(validator=attrs.validators.in_(["array"]))
    element: LSP_TYPE_SPEC = attrs.field(
        validator=type_validator, converter=partial_apply(convert_to_lsp_type)
    )
    id_: Optional[str] = attrs.field(
        converter=lambda x: str(uuid.uuid4()),
        validator=attrs.validators.optional(attrs.validators.instance_of(str)),
        default=None,
    )

    def __eq__(self, other: object) -> bool:
        if isinstance(other, ArrayType):
            return self.element == other.element and self.kind == other.kind
        return False

    def get_inner_types(self) -> List[Type]:
        return [self.element] + self.element.get_inner_types()


@attrs.define
class TupleType:
    kind: str = attrs.field(validator=attrs.validators.in_(["tuple"]))
    items: Iterable[LSP_TYPE_SPEC] = attrs.field(
        converter=list_converter(convert_to_lsp_type)
    )
    id_: Optional[str] = attrs.field(
        converter=lambda x: str(uuid.uuid4()),
        validator=attrs.validators.optional(attrs.validators.instance_of(str)),
        default=None,
    )

    def __eq__(self, other: object) -> bool:
        if isinstance(other, TupleType):
            return self.items == other.items and self.kind == other.kind
        return False

    def get_inner_types(self) -> List[Type]:
        types = []
        for item in self.items:
            types.append(item)
            types.extend(item.get_inner_types())
        return types


@attrs.define
class BaseMapKeyType:
    kind: str = attrs.field(validator=attrs.validators.in_(["base"]))
    name: str = attrs.field(
        validator=attrs.validators.in_(
            [
                "URI",
                "DocumentUri",
                "integer",
                "string",
            ]
        )
    )
    id_: Optional[str] = attrs.field(
        converter=lambda x: str(uuid.uuid4()),
        validator=attrs.validators.optional(attrs.validators.instance_of(str)),
        default=None,
    )

    def __eq__(self, other: object) -> bool:
        if isinstance(other, BaseMapKeyType):
            return self.name == other.name and self.kind == other.kind
        return False

    def get_inner_types(self) -> List[Type]:
        return []


@attrs.define
class ReferenceMapKeyType:
    kind: str = attrs.field(validator=attrs.validators.in_(["reference"]))
    name: str = attrs.field(validator=attrs.validators.instance_of(str))
    id_: Optional[str] = attrs.field(
        converter=lambda x: str(uuid.uuid4()),
        validator=attrs.validators.optional(attrs.validators.instance_of(str)),
        default=None,
    )

    def __eq__(self, other: object) -> bool:
        if isinstance(other, ReferenceMapKeyType):
            return self.name == other.name and self.kind == other.kind
        return False

    def get_inner_types(self) -> List[Type]:
        return []


def convert_map_key(
    key_info: Dict[str, Any],
) -> Union[BaseMapKeyType, ReferenceMapKeyType]:
    if key_info["kind"] == "base":
        return BaseMapKeyType(**key_info)
    return ReferenceMapKeyType(**key_info)


@attrs.define
class MapType:
    kind: str = attrs.field(validator=attrs.validators.in_(["map"]))
    key: Union[BaseMapKeyType, ReferenceMapKeyType] = attrs.field(
        converter=convert_map_key
    )
    value: LSP_TYPE_SPEC = attrs.field(
        validator=type_validator, converter=partial_apply(convert_to_lsp_type)
    )
    id_: Optional[str] = attrs.field(
        converter=lambda x: str(uuid.uuid4()),
        validator=attrs.validators.optional(attrs.validators.instance_of(str)),
        default=None,
    )

    def __eq__(self, other: object) -> bool:
        if isinstance(other, MapType):
            return (
                self.key == other.key
                and self.value == other.value
                and self.kind == other.kind
            )
        return False

    def get_inner_types(self) -> List[Type]:
        return (
            [self.value, self.key]
            + self.value.get_inner_types()
            + self.key.get_inner_types()
        )


@attrs.define
class MetaData:
    version: str = attrs.field(validator=attrs.validators.instance_of(str))
    id_: Optional[str] = attrs.field(
        converter=lambda x: str(uuid.uuid4()),
        validator=attrs.validators.optional(attrs.validators.instance_of(str)),
        default=None,
    )

    def __eq__(self, other: object) -> bool:
        if isinstance(other, MetaData):
            return self.version == other.version
        return False

    def get_inner_types(self) -> List[Type]:
        return []


@attrs.define
class Property:
    name: str = attrs.field(validator=attrs.validators.instance_of(str))
    type: LSP_TYPE_SPEC = attrs.field(
        validator=type_validator,
        converter=partial_apply(convert_to_lsp_type),
    )
    optional: Optional[bool] = attrs.field(
        validator=attrs.validators.optional(attrs.validators.instance_of(bool)),
        default=None,
    )
    proposed: Optional[bool] = attrs.field(
        validator=attrs.validators.optional(attrs.validators.instance_of(bool)),
        default=None,
    )
    documentation: Optional[str] = attrs.field(
        validator=attrs.validators.optional(attrs.validators.instance_of(str)),
        default=None,
    )
    since: Optional[str] = attrs.field(
        validator=attrs.validators.optional(attrs.validators.instance_of(str)),
        default=None,
    )
    deprecated: Optional[str] = attrs.field(
        validator=attrs.validators.optional(attrs.validators.instance_of(str)),
        default=None,
    )
    id_: Optional[str] = attrs.field(
        converter=lambda x: str(uuid.uuid4()),
        validator=attrs.validators.optional(attrs.validators.instance_of(str)),
        default=None,
    )

    def __eq__(self, other: object) -> bool:
        if isinstance(other, Property):
            return (
                self.name == other.name
                and self.type == other.type
                and self.optional == other.optional
            )
        return False

    def get_inner_types(self) -> List[Type]:
        return [self.type] + self.type.get_inner_types()


@attrs.define
class LiteralValue:
    properties: Iterable[Property] = attrs.field(
        converter=list_converter(Property),
    )
    id_: Optional[str] = attrs.field(
        converter=lambda x: str(uuid.uuid4()),
        validator=attrs.validators.optional(attrs.validators.instance_of(str)),
        default=None,
    )

    def __eq__(self, other: object) -> bool:
        if isinstance(other, LiteralValue):
            return self.properties == other.properties
        return False

    def get_inner_types(self) -> List[Type]:
        types = []
        for prop in self.properties:
            types.append(prop)
            types.extend(prop.get_inner_types())
        return types


@attrs.define
class LiteralType:
    kind: str = attrs.field(validator=attrs.validators.in_(["literal"]))
    value: LiteralValue = attrs.field(
        validator=attrs.validators.instance_of(LiteralValue),
        converter=partial_apply(LiteralValue),
    )
    name: Optional[str] = attrs.field(
        validator=attrs.validators.optional(attrs.validators.instance_of(str)),
        default=None,
    )
    proposed: Optional[bool] = attrs.field(
        validator=attrs.validators.optional(attrs.validators.instance_of(bool)),
        default=None,
    )
    documentation: Optional[str] = attrs.field(
        validator=attrs.validators.optional(attrs.validators.instance_of(str)),
        default=None,
    )
    since: Optional[str] = attrs.field(
        validator=attrs.validators.optional(attrs.validators.instance_of(str)),
        default=None,
    )
    deprecated: Optional[str] = attrs.field(
        validator=attrs.validators.optional(attrs.validators.instance_of(str)),
        default=None,
    )
    id_: Optional[str] = attrs.field(
        converter=lambda x: str(uuid.uuid4()),
        validator=attrs.validators.optional(attrs.validators.instance_of(str)),
        default=None,
    )

    def __eq__(self, other: object) -> bool:
        if isinstance(other, LiteralType):
            return (
                self.value == other.value
                and self.kind == other.kind
                and self.name == other.name
            )
        return False

    def get_inner_types(self) -> List[Type]:
        return [self.value] + self.value.get_inner_types()


@attrs.define
class TypeAlias:
    name: str = attrs.field(validator=attrs.validators.instance_of(str))
    type: LSP_TYPE_SPEC = attrs.field(
        validator=type_validator,
        converter=partial_apply(convert_to_lsp_type),
    )
    proposed: Optional[bool] = attrs.field(
        validator=attrs.validators.optional(attrs.validators.instance_of(bool)),
        default=None,
    )
    documentation: Optional[str] = attrs.field(
        validator=attrs.validators.optional(attrs.validators.instance_of(str)),
        default=None,
    )
    since: Optional[str] = attrs.field(
        validator=attrs.validators.optional(attrs.validators.instance_of(str)),
        default=None,
    )
    deprecated: Optional[str] = attrs.field(
        validator=attrs.validators.optional(attrs.validators.instance_of(str)),
        default=None,
    )
    id_: Optional[str] = attrs.field(
        converter=lambda x: str(uuid.uuid4()),
        validator=attrs.validators.optional(attrs.validators.instance_of(str)),
        default=None,
    )

    def __eq__(self, other: object) -> bool:
        if isinstance(other, TypeAlias):
            return (
                self.name == other.name
                and self.type == other.type
                and self.kind == other.kind
            )
        return False

    def get_inner_types(self) -> List[Type]:
        return [self.type] + self.type.get_inner_types()


@attrs.define
class Structure:
    name: str = attrs.field(validator=attrs.validators.instance_of(str))
    properties: Iterable[Property] = attrs.field(
        converter=list_converter(Property),
    )
    extends: Optional[Iterable[LSP_TYPE_SPEC]] = attrs.field(
        converter=list_converter(convert_to_lsp_type), default=[]
    )
    mixins: Optional[Iterable[LSP_TYPE_SPEC]] = attrs.field(
        converter=list_converter(convert_to_lsp_type), default=[]
    )
    proposed: Optional[bool] = attrs.field(
        validator=attrs.validators.optional(attrs.validators.instance_of(bool)),
        default=None,
    )
    documentation: Optional[str] = attrs.field(
        validator=attrs.validators.optional(attrs.validators.instance_of(str)),
        default=None,
    )
    since: Optional[str] = attrs.field(
        validator=attrs.validators.optional(attrs.validators.instance_of(str)),
        default=None,
    )
    deprecated: Optional[str] = attrs.field(
        validator=attrs.validators.optional(attrs.validators.instance_of(str)),
        default=None,
    )
    id_: Optional[str] = attrs.field(
        converter=lambda x: str(uuid.uuid4()),
        validator=attrs.validators.optional(attrs.validators.instance_of(str)),
        default=None,
    )

    def __eq__(self, other: object) -> bool:
        if isinstance(other, Structure):
            return (
                self.name == other.name
                and self.properties == other.properties
                and self.extends == other.extends
                and self.mixins == other.mixins
            )
        return False

    def get_inner_types(self) -> List[Type]:
        types = []
        for prop in self.properties:
            types.append(prop.type)
            types.extend(prop.type.get_inner_types())
        for ext in self.extends:
            types.append(ext)
            types.extend(ext.get_inner_types())
        for mixin in self.mixins:
            types.append(mixin)
            types.extend(mixin.get_inner_types())
        return types


@attrs.define
class Notification:
    method: str = attrs.field(validator=attrs.validators.instance_of(str))
    messageDirection: str = attrs.field(
        validator=attrs.validators.in_(["clientToServer", "serverToClient", "both"])
    )
    params: Optional[LSP_TYPE_SPEC] = attrs.field(
        validator=type_validator,
        converter=partial_apply(convert_to_lsp_type),
        default=None,
    )
    registrationOptions: Optional[LSP_TYPE_SPEC] = attrs.field(
        validator=type_validator,
        converter=partial_apply(convert_to_lsp_type),
        default=None,
    )
    proposed: Optional[bool] = attrs.field(
        validator=attrs.validators.optional(attrs.validators.instance_of(bool)),
        default=None,
    )
    documentation: Optional[str] = attrs.field(
        validator=attrs.validators.optional(attrs.validators.instance_of(str)),
        default=None,
    )
    since: Optional[str] = attrs.field(
        validator=attrs.validators.optional(attrs.validators.instance_of(str)),
        default=None,
    )
    registrationMethod: Optional[str] = attrs.field(
        validator=attrs.validators.optional(attrs.validators.instance_of(str)),
        default=None,
    )
    deprecated: Optional[str] = attrs.field(
        validator=attrs.validators.optional(attrs.validators.instance_of(str)),
        default=None,
    )
    id_: Optional[str] = attrs.field(
        converter=lambda x: str(uuid.uuid4()),
        validator=attrs.validators.optional(attrs.validators.instance_of(str)),
        default=None,
    )

    def __eq__(self, other: object) -> bool:
        if isinstance(other, Notification):
            return (
                self.method == other.method
                and self.messageDirection == other.messageDirection
                and self.params == other.params
                and self.registrationOptions == other.registrationOptions
                and self.registrationMethod == other.registrationMethod
            )
        return False

    def get_inner_types(self) -> List[Type]:
        types = []
        if self.params:
            types.append(self.params)
            types.extend(self.params.get_inner_types())
        if self.registrationOptions:
            types.append(self.registrationOptions)
            types.extend(self.registrationOptions.get_inner_types())
        return types


@attrs.define
class Request:
    method: str = attrs.field(validator=attrs.validators.instance_of(str))
    messageDirection: str = attrs.field(
        validator=attrs.validators.in_(["clientToServer", "serverToClient", "both"])
    )
    params: Optional[LSP_TYPE_SPEC] = attrs.field(
        validator=type_validator,
        converter=partial_apply(convert_to_lsp_type),
        default=None,
    )
    result: Optional[LSP_TYPE_SPEC] = attrs.field(
        validator=type_validator,
        converter=partial_apply(convert_to_lsp_type),
        default=None,
    )
    partialResult: Optional[LSP_TYPE_SPEC] = attrs.field(
        validator=type_validator,
        converter=partial_apply(convert_to_lsp_type),
        default=None,
    )
    errorData: Optional[LSP_TYPE_SPEC] = attrs.field(
        validator=type_validator,
        converter=partial_apply(convert_to_lsp_type),
        default=None,
    )
    registrationOptions: Optional[LSP_TYPE_SPEC] = attrs.field(
        validator=type_validator,
        converter=partial_apply(convert_to_lsp_type),
        default=None,
    )
    proposed: Optional[bool] = attrs.field(
        validator=attrs.validators.optional(attrs.validators.instance_of(bool)),
        default=None,
    )
    documentation: Optional[str] = attrs.field(
        validator=attrs.validators.optional(attrs.validators.instance_of(str)),
        default=None,
    )
    since: Optional[str] = attrs.field(
        validator=attrs.validators.optional(attrs.validators.instance_of(str)),
        default=None,
    )
    registrationMethod: Optional[str] = attrs.field(
        validator=attrs.validators.optional(attrs.validators.instance_of(str)),
        default=None,
    )
    deprecated: Optional[str] = attrs.field(
        validator=attrs.validators.optional(attrs.validators.instance_of(str)),
        default=None,
    )
    id_: Optional[str] = attrs.field(
        converter=lambda x: str(uuid.uuid4()),
        validator=attrs.validators.optional(attrs.validators.instance_of(str)),
        default=None,
    )

    def __eq__(self, other: object) -> bool:
        if isinstance(other, Request):
            return (
                self.method == other.method
                and self.messageDirection == other.messageDirection
                and self.params == other.params
                and self.result == other.result
                and self.partialResult == other.partialResult
                and self.errorData == other.errorData
                and self.registrationOptions == other.registrationOptions
                and self.registrationMethod == other.registrationMethod
            )
        return False

    def get_inner_types(self) -> List[Type]:
        types = []
        if self.params:
            types.append(self.params)
            types.extend(self.params.get_inner_types())
        if self.result:
            types.append(self.result)
            types.extend(self.result.get_inner_types())
        if self.partialResult:
            types.append(self.partialResult)
            types.extend(self.partialResult.get_inner_types())
        if self.errorData:
            types.append(self.errorData)
            types.extend(self.errorData.get_inner_types())
        if self.registrationOptions:
            types.append(self.registrationOptions)
            types.extend(self.registrationOptions.get_inner_types())
        return types


@attrs.define
class LSPModel:
    requests: Iterable[Request] = attrs.field(converter=list_converter(Request))
    notifications: Iterable[Notification] = attrs.field(
        converter=list_converter(Notification)
    )
    structures: Iterable[Structure] = attrs.field(converter=list_converter(Structure))
    enumerations: Iterable[Enum] = attrs.field(converter=list_converter(Enum))
    typeAliases: Iterable[TypeAlias] = attrs.field(converter=list_converter(TypeAlias))
    metaData: MetaData = attrs.field(converter=lambda x: MetaData(**x))

    def __eq__(self, other: object) -> bool:
        if isinstance(other, LSPModel):
            return (
                self.requests == other.requests
                and self.notifications == other.notifications
                and self.structures == other.structures
                and self.enumerations == other.enumerations
                and self.typeAliases == other.typeAliases
                and self.metaData == other.metaData
            )
        return False

    def get_inner_types(self) -> List[Type]:
        types = self.metaData.get_inner_types()
        for r in self.requests:
            types.extend(r.get_inner_types())
        for n in self.notifications:
            types.extend(n.get_inner_types())
        for s in self.structures:
            types.extend(s.get_inner_types())
        for e in self.enumerations:
            types.extend(e.get_inner_types())
        for t in self.typeAliases:
            types.extend(t.get_inner_types())
        return types


def create_lsp_model(models: List[Dict[str, Any]]) -> LSPModel:
    if len(models) >= 1:
        spec = LSPModel(**models[0])

    if len(models) >= 2:
        for model in models[1:]:
            addition = LSPModel(**model)
            spec.requests.extend(addition.requests)
            spec.notifications.extend(addition.notifications)
            spec.structures.extend(addition.structures)
            spec.enumerations.extend(addition.enumerations)
            spec.typeAliases.extend(addition.typeAliases)

    return spec
