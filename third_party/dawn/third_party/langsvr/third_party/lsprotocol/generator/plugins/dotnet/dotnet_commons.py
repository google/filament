# Copyright (c) Microsoft Corporation. All rights reserved.
# Licensed under the MIT License.

from typing import Dict, List, Tuple, Union

from generator import model

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
        self._id_data: Dict[str, Tuple[str, TypesWithId, List[str]]] = {}
        self._ctor_data: Dict[str, Tuple[str, str]] = {}

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
        return [
            type_def
            for name, type_def, _ in self._id_data.values()
            if name == type_name
        ]

    def get_all(self) -> List[Tuple[str, List[str]]]:
        return [(name, lines) for name, _, lines in self._id_data.values()]

    def add_ctor(self, type_name: str, ctor: Tuple[str, str, bool]) -> None:
        self._ctor_data[type_name] = ctor

    def get_ctor(self, type_name: str) -> Tuple[str, str, bool]:
        return self._ctor_data[type_name]
