# Copyright (c) Microsoft Corporation. All rights reserved.
# Licensed under the MIT License.


import re
from typing import List

BASIC_LINK_RE = re.compile(r"{@link +(\w+) ([\w ]+)}")
BASIC_LINK_RE2 = re.compile(r"{@link +(\w+)\.(\w+) ([\w \.`]+)}")
BASIC_LINK_RE3 = re.compile(r"{@link +(\w+)}")
BASIC_LINK_RE4 = re.compile(r"{@link +(\w+)\.(\w+)}")
PARTS_RE = re.compile(r"(([a-z0-9])([A-Z]))")
DEFAULT_INDENT = "    "


def lines_to_comments(lines: List[str]) -> List[str]:
    return ["// " + line for line in lines]


def lines_to_doc_comments(lines: List[str]) -> List[str]:
    doc = []
    for line in lines:
        line = BASIC_LINK_RE.sub(r"[\2][\1]", line)
        line = BASIC_LINK_RE2.sub(r"[\3][`\1::\2`]", line)
        line = BASIC_LINK_RE3.sub(r"[\1]", line)
        line = BASIC_LINK_RE4.sub(r"[`\1::\2`]", line)
        if line.startswith("///"):
            doc.append(line)
        else:
            doc.append("/// " + line)
    return doc


def lines_to_block_comment(lines: List[str]) -> List[str]:
    return ["/*"] + lines + ["*/"]


def get_parts(name: str) -> List[str]:
    name = name.replace("_", " ")
    return PARTS_RE.sub(r"\2 \3", name).split()


def to_snake_case(name: str) -> str:
    return "_".join([part.lower() for part in get_parts(name)])


def has_upper_case(name: str) -> bool:
    return any(c.isupper() for c in name)


def is_snake_case(name: str) -> bool:
    return (
        not name.startswith("_")
        and not name.endswith("_")
        and ("_" in name)
        and not has_upper_case(name)
    )


def to_upper_camel_case(name: str) -> str:
    return "".join([c.capitalize() for c in get_parts(name)])


def to_camel_case(name: str) -> str:
    parts = get_parts(name)
    if len(parts) > 1:
        return parts[0] + "".join([c.capitalize() for c in parts[1:]])
    else:
        return parts[0]


def indent_lines(lines: List[str], indent: str = DEFAULT_INDENT) -> List[str]:
    return [f"{indent}{line}" for line in lines]
