#!/usr/bin/env python3
#
# Copyright (C) 2026 The Android Open Source Project
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

"""Helper utilities, naming transforms, and AST attribute extractors for JavaGen.

Architectural Purpose:
    This module centralizes stateless helper routines for:
    1. AST Attribute Extraction:
       Clang AST attributes emitted by `extractor.py` may carry legacy or current
       namespace prefixes (e.g. `filament:apigen:size_param:count`, `apigen:size_param:count`,
       or raw `size_param:count`). Functions like `get_size_param_attr` and
       `get_effective_method_name` isolate the generator from prefix variation.
    2. Java Identifier Sanitization & Keyword Avoidance:
       Functions such as `sanitize_identifier` prevent compile errors caused by
       C++ identifiers colliding with Java reserved keywords (e.g., `default`, `final`).
    3. C++ Scoping & Traversal:
       `get_scoped_cpp_name` reconstructs fully qualified C++ type paths (e.g.
       `LightManager::ShadowOptions`) by walking nested parent class hierarchies.
    4. Tagged Scalar & Uniform Array Classification:
       Routines like `classify_scalar_family`, `get_element_entry_name`, and
       `get_tagged_array_info` categorize template specializations into uniform
       array families (`float`, `int`, `bool`) for multi-element JNI marshaling.
"""

from typing import Any, Dict, List, Optional, Union

from .config import (
    ENUM_ENTRY_RENAMES,
    JAVA_KEYWORDS,
    KNOWN_CLASSES,
)


def get_scoped_cpp_name(cls_dict: Optional[Dict[str, Any]]) -> str:
    """Resolve the scoped C++ class name walking up the nested parent hierarchy.

    Architectural Purpose:
        Nested aggregate structs (e.g. `ShadowOptions` declared inside `LightManager`)
        must be referenced in JNI C++ code via their fully qualified enclosing scope
        (`LightManager::ShadowOptions`) rather than bare simple names (`ShadowOptions`)
        to avoid C++ compiler symbol lookup errors.

    Input-to-Output Examples:
        - Input: `{"name": "ShadowOptions", "qualified_name": "filament::LightManager::ShadowOptions"}`
          Output: `"LightManager::ShadowOptions"` (engine root namespaces stripped)
        - Input: `{"name": "Vsm", "parent_class": "ShadowOptions"}` (where ShadowOptions parent is LightManager)
          Output: `"LightManager::ShadowOptions::Vsm"`

    Args:
        cls_dict: The class representation dictionary from IR, or None.

    Returns:
        The scoped C++ name (e.g. 'LightManager::ShadowOptions') with root
        engine namespaces ('filament::', 'utils::', 'math::') stripped.
        Returns an empty string if cls_dict is None.
    """
    # Step 1: Handle null or empty class dictionary inputs
    if not cls_dict:
        return ""

    # Step 2: Check if an explicit qualified_name was extracted by libclang
    qname = cls_dict.get("qualified_name")
    if qname:
        # Strip top-level library namespace prefixes that are already imported
        # via `using namespace filament;` and `using namespace utils;` in JNI files.
        return qname.replace("filament::", "").replace("utils::", "").replace("math::", "")

    # Step 3: Walk up the parent_class hierarchy to build scoped chain if qname is missing
    parts = [cls_dict["name"]]
    curr = cls_dict
    while curr.get("parent_class"):
        parent_name = curr["parent_class"]
        parts.insert(0, parent_name)
        # Look up parent in the known classes registry to continue ascending
        curr = KNOWN_CLASSES.get(parent_name, {})

    # Step 4: Join parts with C++ scope resolution operator
    return "::".join(parts)


def get_size_param_attr(arg: Dict[str, Any]) -> Optional[str]:
    """Extract the associated count or size parameter name from an argument.

    Architectural Purpose:
        In legacy C++ APIs or IR fixtures, raw pointer array arguments were paired
        with a length/count parameter (e.g. `void setBones(Bone const* transforms, size_t count)`).
        Modern APIs use `utils::Slice<T>` instead. For legacy IR compatibility,
        this function extracts the target parameter name (`count`) from either the
        argument or its underlying type dictionary.

    Search Strategy:
        1. Inspect `arg["attributes"]` for prefixes:
           `filament:apigen:size_param:<name>`, `filament:size_param:<name>`,
           `apigen:size_param:<name>`, or `size_param:<name>`.
        2. Inspect `arg["size_param"]` direct attribute key.
        3. Recurse into `arg["type"]["attributes"]` and `arg["type"]["size_param"]`.

    Input-to-Output Example:
        Given an argument with `attributes = ["filament:apigen:size_param:count"]`:
        Returns `"count"`.

    Args:
        arg: Argument dictionary from method IR.

    Returns:
        The name of the paired size parameter string if found, or None.
    """
    prefixes = (
        "filament:apigen:size_param:",
        "filament:size_param:",
        "apigen:size_param:",
        "size_param:",
    )

    # Step 1: Scan direct argument-level attributes
    for attr in arg.get("attributes", []):
        for prefix in prefixes:
            if attr.startswith(prefix):
                return attr[len(prefix):]

    # Step 2: Check direct size_param metadata key on argument
    if arg.get("size_param"):
        return arg["size_param"]

    # Step 3: Scan nested type-level attributes (where clang often attaches type attributes)
    t = arg.get("type")
    if isinstance(t, dict):
        for attr in t.get("attributes", []):
            for prefix in prefixes:
                if attr.startswith(prefix):
                    return attr[len(prefix):]
        if t.get("size_param"):
            return t["size_param"]

    return None


def get_tagged_array_attr(arg: Dict[str, Any]) -> bool:
    """Check if an argument is annotated with a tagged_array attribute.

    Architectural Purpose:
        Identifies arguments marked with `UTILS_APIGEN_TAGGED_ARRAY` in C++.
        Tagged arrays collapse multiple SFINAE template specializations into a
        unified typed array method family taking an element enum in Java.

    Args:
        arg: Argument dictionary from method IR.

    Returns:
        True if the argument possesses any tagged_array attribute, False otherwise.
    """
    tagged_markers = (
        "filament:apigen:tagged_array",
        "filament:tagged_array",
        "apigen:tagged_array",
        "tagged_array",
    )

    # Step 1: Inspect attributes for any recognized tagged_array marker
    for attr in arg.get("attributes", []):
        if attr in tagged_markers:
            return True
    return False


def format_enum_entry_name(enum_name: str, entry_name: str) -> str:
    """Format an enum entry name, applying casing normalization and explicit renames.

    Architectural Purpose:
        In C++, enum constants often follow mixed casing styles (e.g. `sRGB`,
        `sRGB_A_DXT1`). In Java, standard conventions require all-caps identifiers
        (`SRGB`, `SRGB_A_DXT1`). Furthermore, specific enums have explicit renames
        registered in `ENUM_ENTRY_RENAMES` to resolve naming conflicts or enhance
        Java clarity.

    Input-to-Output Examples:
        - `format_enum_entry_name("TextureFormat", "sRGB")` -> `"SRGB"`
        - `format_enum_entry_name("CompressedFormat", "sRGB_A_DXT1")` -> `"SRGB_A_DXT1"`
        - `format_enum_entry_name("IndexType", "USHORT")` -> registered rename if present

    Args:
        enum_name: Name of the enclosing C++ enum class.
        entry_name: Raw entry identifier string from C++ AST.

    Returns:
        The normalized Java enum constant identifier.
    """
    # Step 1: Normalize lowercase 'sRGB' prefixes to uppercase 'SRGB'
    name = entry_name.replace("sRGB", "SRGB")

    # Step 2: Apply table-driven renames if registered for this enum
    if enum_name in ENUM_ENTRY_RENAMES and name in ENUM_ENTRY_RENAMES[enum_name]:
        return ENUM_ENTRY_RENAMES[enum_name][name]

    return name


def classify_scalar_family(cpp_type: Union[str, Dict[str, Any]]) -> Optional[str]:
    """Classify a C++ type into a tagged array scalar family ('float', 'int', or 'bool').

    Architectural Purpose:
        When unrolling tagged arrays (e.g. `MaterialInstance::setParameter`),
        method variants are grouped by their underlying primitive representation:
        - Floating-point scalars, vectors, and matrices (`float`, `float2..4`, `mat3f..4f`)
          collapse to Java `float[]` with `FloatElement`.
        - Integer scalars and vectors (`int32_t`, `int2..4`, `uint32_t`)
          collapse to Java `int[]` with `IntElement`.
        - Boolean scalars and vectors (`bool`, `bool2..4`)
          collapse to Java `boolean[]` with `BooleanElement`.

    Input-to-Output Examples:
        - `"math::float3"` -> `"float"`
        - `"filament::math::mat4f"` -> `"float"`
        - `"int32_t"` -> `"int"`
        - `"bool4"` -> `"bool"`
        - `"std::string"` -> `None`

    Args:
        cpp_type: C++ type string or type dictionary from IR.

    Returns:
        'float', 'int', 'bool', or None if not part of a recognized scalar family.
    """
    # Step 1: Extract raw type string if dictionary was provided
    if isinstance(cpp_type, dict):
        cpp_type = cpp_type.get("cpp_name") or cpp_type.get("qualified_name") or ""

    # Step 2: Strip outer qualifiers, namespaces, pointers, and references
    t = cpp_type.strip()
    bare = t.split("::")[-1].replace("const", "").replace("*", "").replace("&", "").strip()

    # Step 3: Match against known floating-point prefixes
    if any(bare.startswith(p) for p in ("float", "mat3", "mat4", "half")):
        return "float"

    # Step 4: Match against known integer and indexing prefixes
    if any(bare.startswith(p) for p in ("int", "uint", "size_t")):
        return "int"

    # Step 5: Match against boolean prefixes
    if bare.startswith("bool"):
        return "bool"

    return None


def get_element_entry_name(cpp_type: Union[str, Dict[str, Any]]) -> str:
    """Map a C++ type to its corresponding FloatElement/IntElement/BooleanElement enum entry.

    Architectural Purpose:
        Provides the mapping from concrete C++ template specialization types to
        the enum constant passed in Java to specify buffer element rank and dimensions.

    Mapping Table:
        `float` -> `FLOAT`, `float2` -> `FLOAT2`, `float3` -> `FLOAT3`, `float4` -> `FLOAT4`
        `mat3f` / `mat3` -> `MAT3`, `mat4f` / `mat4` -> `MAT4`
        `int` / `int32_t` / `uint` / `uint32_t` -> `INT`
        `int2` / `uint2` -> `INT2`, `int3` -> `INT3`, `int4` -> `INT4`
        `bool` -> `BOOL`, `bool2` -> `BOOL2`, `bool3` -> `BOOL3`, `bool4` -> `BOOL4`

    Args:
        cpp_type: C++ type string or type dictionary from IR.

    Returns:
        Enum constant name string (e.g. 'FLOAT', 'FLOAT3', 'MAT4', 'INT').
    """
    # Step 1: Normalize input to bare C++ type identifier
    if isinstance(cpp_type, dict):
        cpp_type = cpp_type.get("cpp_name") or cpp_type.get("qualified_name") or ""
    bare = cpp_type.strip().split("::")[-1].replace("const", "").replace("*", "").replace("&", "").strip()

    # Step 2: Lookup in canonical element entry mapping table
    mapping = {
        "float": "FLOAT",
        "float2": "FLOAT2",
        "float3": "FLOAT3",
        "float4": "FLOAT4",
        "mat3f": "MAT3",
        "mat3": "MAT3",
        "mat4f": "MAT4",
        "mat4": "MAT4",
        "int32_t": "INT",
        "int": "INT",
        "int2": "INT2",
        "int3": "INT3",
        "int4": "INT4",
        "uint32_t": "INT",
        "uint": "INT",
        "uint2": "INT2",
        "uint3": "INT3",
        "uint4": "INT4",
        "bool": "BOOL",
        "bool2": "BOOL2",
        "bool3": "BOOL3",
        "bool4": "BOOL4",
    }
    if bare in mapping:
        return mapping[bare]

    # Step 3: Default to uppercase representation if not explicitly in table
    return bare.upper()


def get_tagged_array_info(method_ir: Dict[str, Any]) -> Optional[Dict[str, Any]]:
    """Extract metadata for a method participating in dual-mode tagged array marshalling.

    Architectural Purpose:
        Analyzes a C++ method signature to locate the tagged array buffer parameter
        and its accompanying size/count argument. Divides surrounding arguments into
        prefix and suffix lists to allow `JavaEmitter` and `JniEmitter` to insert
        the synthetic `Element` type parameter and unroll offsets cleanly.

    Data Structure Produced:
        {
            "tagged_idx": int,         # Index of the tagged array argument in args list
            "cnt_idx": int,            # Index of the size_param argument in args list
            "pre_args": List[dict],    # Arguments preceding the tagged array
            "tagged_arg": dict,        # The tagged array argument dictionary
            "cnt_arg": dict,           # The size parameter argument dictionary
            "post_args": List[dict],   # Arguments following the tagged array (excluding count)
        }

    Args:
        method_ir: Method dictionary from JSON IR.

    Returns:
        Dictionary detailing argument indices and slices if found, else None.
    """
    args = method_ir.get("arguments", [])
    tagged_idx = -1
    size_arg_name = None

    # Step 1: Find the argument annotated with tagged_array
    for idx, a in enumerate(args):
        if get_tagged_array_attr(a):
            tagged_idx = idx
            size_arg_name = get_size_param_attr(a)
            break

    # If no tagged array argument exists, abort
    if tagged_idx == -1:
        return None

    tagged_arg = args[tagged_idx]
    t_str = tagged_arg["type"].get("qualified_name") or tagged_arg["type"].get("cpp_name", "") if isinstance(tagged_arg["type"], dict) else str(tagged_arg["type"])
    is_slice = "Slice" in t_str

    if not size_arg_name and not is_slice:
        return None

    cnt_idx = -1
    if size_arg_name:
        # Step 2: Locate the paired size parameter argument by matching name
        for idx, a in enumerate(args):
            if a["name"] == size_arg_name:
                cnt_idx = idx
                break

        # If paired size parameter is missing from the arguments list, abort
        if cnt_idx == -1:
            return None
        cnt_arg = args[cnt_idx]
    else:
        cnt_arg = {"name": "count", "type": {"cpp_name": "size_t"}}

    # Step 3: Segment arguments into pre, tagged, count, and post groups
    pre_args = args[:tagged_idx]
    post_args = [a for idx, a in enumerate(args) if idx > tagged_idx and idx != cnt_idx]

    return {
        "tagged_idx": tagged_idx,
        "cnt_idx": cnt_idx,
        "pre_args": pre_args,
        "tagged_arg": tagged_arg,
        "cnt_arg": cnt_arg,
        "post_args": post_args,
        "is_slice": is_slice,
    }


def is_tagged_array_method(method_ir: Dict[str, Any]) -> bool:
    """Check if a method possesses tagged array arguments.

    Args:
        method_ir: Method dictionary from IR.

    Returns:
        True if the method qualifies for tagged array overloads, False otherwise.
    """
    # Step 1: Delegate to get_tagged_array_info and test for presence
    return get_tagged_array_info(method_ir) is not None


def get_jni_scalar_type(scalar: str) -> str:
    """Map a Java primitive scalar name to its corresponding JNI primitive type.

    Input-to-Output Examples:
        - `"float"` -> `"jfloat"`
        - `"int"` -> `"jint"`
        - `"boolean"` -> `"jboolean"`
        - `"double"` -> `"jdouble"`
        - `"long"` -> `"jlong"`

    Args:
        scalar: Java primitive type name (e.g. 'float', 'int', 'boolean').

    Returns:
        JNI type string (e.g. 'jfloat', 'jint', 'jboolean').
    """
    # Step 1: Map standard Java primitives to JNI typedefs
    mapping = {
        "float": "jfloat",
        "double": "jdouble",
        "int": "jint",
        "boolean": "jboolean",
        "short": "jshort",
        "byte": "jbyte",
        "long": "jlong",
        "char": "jchar",
    }
    return mapping.get(scalar, "jfloat")


def sanitize_identifier(name: str) -> str:
    """Sanitize an identifier by appending an underscore if it collides with a Java keyword.

    Architectural Purpose:
        C++ allows identifiers such as `default`, `final`, `package`, or `native`
        for variables or parameters. In Java, these are reserved keywords and trigger
        compiler syntax errors. This function appends a trailing underscore
        (e.g., `default_`, `final_`) to guarantee syntax validity.

    Input-to-Output Examples:
        - `"package"` -> `"package_"`
        - `"final"` -> `"final_"`
        - `"width"` -> `"width"`

    Args:
        name: Proposed identifier name string.

    Returns:
        Sanitized identifier safe for Java emission.
    """
    # Step 1: Guard against empty strings
    if not name:
        return name

    # Step 2: If the name collides with a reserved Java keyword, append an underscore
    if name in JAVA_KEYWORDS:
        return f"{name}_"

    return name


def get_effective_method_name(method: Dict[str, Any]) -> str:
    """Determine the effective method name in Java, respecting alternate name annotations.

    Architectural Purpose:
        C++ method names occasionally collide with reserved keywords in Java
        (e.g. `Material::Builder::package` or `Texture::Builder::import`). In C++,
        these methods are annotated with `UTILS_APIGEN_ALTERNATE_NAME(newName)`.
        This function extracts the designated alternate name (`payload`, `importTexture`)
        to emit valid Java signatures while allowing JNI bridges to dispatch to the
        original C++ method name.

    Search Strategy:
        1. Check `method["attributes"]` for prefixes:
           `filament:apigen:alternate_name:<name>`, `filament:alternate_name:<name>`,
           or `apigen:alternate_name:<name>`.
        2. If no alternate name is annotated, verify the raw name is not a reserved
           keyword. If it is, raise a `ValueError` alerting the developer to annotate it.

    Args:
        method: Method dictionary from IR.

    Returns:
        Target Java method name string.

    Raises:
        ValueError: If raw method name is a Java reserved keyword and lacks an alternate name.
    """
    prefixes = (
        "filament:apigen:alternate_name:",
        "filament:alternate_name:",
        "apigen:alternate_name:",
    )

    # Step 1: Check for explicit alternate name annotation in attributes
    for attr in method.get("attributes", []):
        for prefix in prefixes:
            if attr.startswith(prefix):
                return attr[len(prefix):]

    # Step 2: Check for collision with Java keywords
    raw_name = method["name"]
    if raw_name in JAVA_KEYWORDS:
        raise ValueError(
            f"Method '{raw_name}' is a Java reserved keyword but lacks UTILS_APIGEN_ALTERNATE_NAME annotation."
        )

    return raw_name


def to_camel_case(name: str) -> str:
    """Convert a snake_case identifier to lowerCamelCase.

    Input-to-Output Examples:
        - `"shadow_options"` -> `"shadowOptions"`
        - `"cascade_split_positions"` -> `"cascadeSplitPositions"`
        - `"center_x"` -> `"centerX"`

    Args:
        name: Snake case identifier string.

    Returns:
        Camel case formatted string.
    """
    # Step 1: Split identifier by underscores
    parts = name.split('_')

    # Step 2: Keep first component lowercase and capitalize subsequent components
    return parts[0] + ''.join(x.title() for x in parts[1:])


def indent_lines(text: str, spaces: int = 4) -> str:
    """Indent non-empty lines of text by the specified number of spaces.

    Args:
        text: Input multi-line string.
        spaces: Number of leading spaces to prepend per line (default 4).

    Returns:
        Indented text string.
    """
    # Step 1: Split into individual lines
    lines = text.split('\n')

    # Step 2: Prepend indentation to non-empty lines, preserving blank lines
    indent = ' ' * spaces
    return '\n'.join(indent + line if line else line for line in lines)


def is_custom_enum(enum_ir: Optional[Dict[str, Any]]) -> bool:
    """Check if an enum has custom, non-sequential, or negative integer values.

    Architectural Purpose:
        Standard C++ enums with 0-indexed sequential values (`0, 1, 2, ...`)
        map directly to Java enum ordinals and are cached via `EnumCache.values()[ordinal]`.
        However, custom enums with explicit values (e.g. `COLOR0 = 0`, `COLOR1 = 1`,
        `COLOR = 0` aliases, or negative values like `FAILED = -1`) cannot use array
        ordinal indexing without `ArrayIndexOutOfBoundsException` or alias collision.
        This function identifies custom enums so that `JavaEmitter` generates an
        explicit `mValue` field and a switch-based `from(int)` reverse lookup table.

    Args:
        enum_ir: Enum dictionary from IR, or None.

    Returns:
        True if any enum entry has an explicit value differing from its sequential index.
    """
    # Step 1: Guard against null or missing enum dictionary
    if not enum_ir:
        return False

    # Step 2: Iterate through entries and check if any value diverges from its index
    entries = enum_ir.get("entries", [])
    for idx, entry in enumerate(entries):
        val = entry.get("value")
        # If value is specified and differs from sequential index, it requires custom handling
        if val is not None and val != idx:
            return True

    return False


RESERVED_FIELD_PREFIXES = ("reserved", "rfu", "padding")


def is_reserved_or_padding_field(field_name: str) -> bool:
    """Determine whether a C++ struct field represents alignment padding or reserved space.

    Architectural Purpose:
        In C/C++ POD/options structures, fields named e.g. `reserved`, `reserved1`, `reserved2`,
        `rfu`, `padding` are used for memory alignment, ABI stability, or future expansion.
        These fields should not be exposed in Java bindings or JNI parameter marshalling.

    Args:
        field_name: Name of the C++ structure field.

    Returns:
        True if the field name starts with any reserved prefix (case-insensitive).
    """
    clean_name = field_name.strip().lower().lstrip("_")
    return clean_name.startswith(RESERVED_FIELD_PREFIXES)

