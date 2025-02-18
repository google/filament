# Copyright (c) Microsoft Corporation. All rights reserved.
# Licensed under the MIT License.


import pathlib

import generator.model as model

from .rust_commons import get_message_type_name


def generate_test_code(spec: model.LSPModel, test_path: pathlib.Path) -> str:
    """Generate the code for the given spec."""
    lines = []
    for request in spec.requests:
        request_name = get_message_type_name(request)
        lines += [
            f'"{request_name}" =>' "{",
            f"return validate_type::<{request_name}>(result_type, data)",
            "}",
        ]
    for notification in spec.notifications:
        notification_name = get_message_type_name(notification)
        lines += [
            f'"{notification_name}" =>' "{",
            f"return validate_type::<{notification_name}>(result_type, data)",
            "}",
        ]

    code = test_path.read_text(encoding="utf-8").splitlines()
    start_marker = "GENERATED_TEST_CODE:start"
    end_marker = "GENERATED_TEST_CODE:end"

    start_index = -1
    end_index = -1
    for i, line in enumerate(code):
        if line.endswith(start_marker):
            start_index = i + 1
        elif line.endswith(end_marker):
            end_index = i
    code[start_index:end_index] = lines
    test_path.write_text("\n".join(code), encoding="utf-8")
