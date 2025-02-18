# Copyright (c) Microsoft Corporation. All rights reserved.
# Licensed under the MIT License.

from typing import List


def license_header() -> List[str]:
    return [
        "Copyright (c) Microsoft Corporation. All rights reserved.",
        "Licensed under the MIT License.",
    ]


def package_description() -> List[str]:
    return ["Language Server Protocol types for Rust generated from LSP specification."]
