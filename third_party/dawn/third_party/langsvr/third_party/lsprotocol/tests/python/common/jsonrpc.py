# Copyright (c) Microsoft Corporation. All rights reserved.
# Licensed under the MIT License.

import json

from lsprotocol import converters, types


def to_json(
    obj: types.MESSAGE_TYPES,
    method: str = None,
    converter=None,
) -> str:
    """Converts a given LSP message object to JSON string using the provided
    converter."""
    if not converter:
        converter = converters.get_converter()

    if method is None:
        method = obj.method if hasattr(obj, "method") else None

    if hasattr(obj, "result"):
        if method is None:
            raise ValueError("`method` must not be None for response type objects.")
        obj_type = types.METHOD_TO_TYPES[method][1]
    elif hasattr(obj, "error"):
        obj_type = types.ResponseErrorMessage
    else:
        obj_type = types.METHOD_TO_TYPES[method][0]
    return json.dumps(converter.unstructure(obj, unstructure_as=obj_type))


def from_json(json_str: str, method: str = None, converter=None) -> types.MESSAGE_TYPES:
    """Parses and given JSON string and returns LSP message object using the provided
    converter."""
    if not converter:
        converter = converters.get_converter()

    obj = json.loads(json_str)

    if method is None:
        method = obj.get("method", None)

    if "result" in obj:
        if method is None:
            raise ValueError("`method` must not be None for response type objects.")
        obj_type = types.METHOD_TO_TYPES[method][1]
    elif "error" in obj:
        obj_type = types.ResponseErrorMessage
    else:
        obj_type = types.METHOD_TO_TYPES[method][0]
    return converter.structure(obj, obj_type)
