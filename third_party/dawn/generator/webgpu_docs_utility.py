#!/usr/bin/env python3
# Copyright 2025 The Dawn & Tint Authors
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are met:
#
# 1. Redistributions of source code must retain the above copyright notice, this
#    list of conditions and the following disclaimer.
#
# 2. Redistributions in binary form must reproduce the above copyright notice,
#    this list of conditions and the following disclaimer in the documentation
#    and/or other materials provided with the distribution.
#
# 3. Neither the name of the copyright holder nor the names of its
#    contributors may be used to endorse or promote products derived from
#    this software without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
# AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
# DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
# FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
# DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
# SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
# CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
# OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
# OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
"""Utilities for processing JSON documentation in Dawn."""

from collections import defaultdict
import json
import re

# ####################################################################################
# Language-Specific Doc Cleaners
# To add support for a new language, create a new `clean_doc_for_...` function
# and add it in the `LANGUAGE_DOC_HANDLER`.
# ####################################################################################


def is_method_reference(ref, object_methods):
    """Checks if a reference is a valid method of an object.

    The reference can be a C-style function name (e.g., wgpuBufferMapAsync)
    or an object/method name (e.g., BufferMapAsync). This function handles
    various formats by stripping prefixes and converting to camelCase for
    validation against the list of known methods.
    """
    cleaned_ref = ref.rstrip('.,)')
    if not object_methods:
        return False

    object_ref = cleaned_ref
    if object_ref.lower().startswith("wgpu"):
        object_ref = object_ref[4:]

    for obj_name, methods in object_methods.items():
        if object_ref.startswith(obj_name):
            method_part = object_ref[len(obj_name):].replace("-", "")
            if method_part:
                method_camel = method_part[0].lower() + method_part[1:]
                if method_camel in methods:
                    return True
    return False


def is_other_reference_valid(ref, valid_names_normalized):
    """Checks if a reference is a valid hyphenated, wgpu-prefixed, or simple name.

    Handles cases like 'my-struct-name', 'wgpuMyObject', or 'MyEnum' by
    normalizing them to a consistent format for validation.
    """
    cleaned_ref = ref.rstrip('.,)')
    if "-" in cleaned_ref:
        normalized_ref = cleaned_ref.replace("-", "").lower()
        return normalized_ref in valid_names_normalized
    elif cleaned_ref.lower().startswith("wgpu"):
        rest = cleaned_ref[4:]
        return "_" in rest or rest.lower() in valid_names_normalized
    else:
        return cleaned_ref.lower() in valid_names_normalized


def log_warning(log_file, message):
    """Appends a warning message to a specified log file."""
    if log_file:
        log_file.write(f"WARNING: {message}\n")


def check_and_log_incomplete_docs(log_file,
                                  item_name,
                                  item_type,
                                  item_parts,
                                  part_docs_map,
                                  part_type='arguments'):
    """
    Checks for incomplete documentation for an item's parts (e.g., arguments,
    members, entries) and logs warnings.
    """
    if not item_parts:
        return True

    missing_parts = []
    found_parts = []
    for part in item_parts:
        part_name_snake = part.name.snake_case()

        part_doc = part_docs_map.get(part_name_snake)

        if not part_doc or not part_doc.strip():
            missing_parts.append(part.name.get())
        else:
            found_parts.append(part.name.get())

    if missing_parts:
        if not found_parts:
            log_warning(
                log_file,
                f"Missing documentation for all {part_type} of {item_type} '{item_name}'."
            )
        else:
            log_warning(
                log_file,
                f"Missing documentation for {part_type} of {item_type} '{item_name}': {', '.join(missing_parts)}. "
                f"(Found for: {', '.join(found_parts)})")
        return False

    return True


def clean_doc_for_kotlin(doc,
                         valid_names_normalized,
                         enum_names,
                         params=None,
                         object_methods=None):
    """Cleans a docstring by validating and formatting references for Kotlin KDoc."""
    # These are special-cased references that are not part of the API but are
    # used in the documentation. They are manually mapped in the `dawn_kotlin` JSON config.
    KOTLIN_HARDCODED_LINK_REFS = [
        "CallbackStatuses",
        "LocalizableHumanReadableMessageString",
        "CallbackReentrancy",
        "ErrorScopes",
        "DeviceError",
        "DeviceRelease",
        "Asynchronous-Operations",
        "MappedRangeBehavior",
    ]
    if not doc:
        return doc

    kdocs_blocklist = params.get("kdocs_blocklist")
    kdocs_replacements = params.get("kdocs_replacements")

    for old, new in kdocs_replacements.items():
        doc = doc.replace(old, new)

    if any(sub in doc for sub in kdocs_blocklist):
        return ""

    # Example: WGPU_COPY_SRC -> Constants.COPY_SRC
    # Converts C-style constants to Kotlin-style constant references.
    doc = re.sub(r"WGPU_([A-Z][A-Z_]*)", r"Constants.\1", doc)

    # Example: See @ref WGPUTextureView -> @see WGPUTextureView
    # Normalizes Doxygen-style @ref tags to standard @see tags for KDoc.
    doc = re.sub(r"(?:[Ss]ee\s+)?@ref\s+(\S+)", r"@see \1", doc)
    doc = doc.replace("TODO", "").replace("\n", " ").strip()

    # Example: `true` -> {@code true}, NULL -> {@code null}
    # Wraps boolean/null literals in @code blocks, stripping optional backticks
    # and converting to lowercase.
    doc = re.sub(r"`?\b(true|false|null|NULL)\b`?",
                 lambda m: '{@code ' + m.group(1).lower() + '}', doc)

    # Example: @see WGPUAdapter -> finds "WGPUAdapter"
    # Extracts all referenced names from @see tags to validate them.
    references = re.findall(r"@see (\S+)", doc)
    for ref in references:
        # Convert C++ scope resolution (::) to Kotlin (.).
        if "::" in ref:
            new_ref = ref.replace("::", ".")
            doc = doc.replace(ref, new_ref)
            continue

        if ref.startswith("Constants."):
            continue

        if any(s in ref for s in KOTLIN_HARDCODED_LINK_REFS):
            is_valid = True
        else:
            is_valid = (is_method_reference(ref, object_methods) or
                        is_other_reference_valid(ref, valid_names_normalized))

        # Discard docstring if any reference is invalid to avoid incorrect KDoc.
        if not is_valid:
            return ""

    # Example: ::WGPUAdapter -> WGPUAdapter
    # Removes C++ scope resolution operators before WGPU/wgpu prefixes.
    doc = re.sub(r"::(wgpu|WGPU)", r"\1", doc)

    # Example: @see WGPUAdapter -> @see Adapter
    # Strips the standard 'WGPU' or 'wgpu' prefix from type names for cleaner,
    # more idiomatic Kotlin references.
    doc = re.sub(r"(?:WGPU|wgpu)(\S+)", r"\1", doc)

    # Example: InstanceWaitAny -> @see Instance.waitAny
    # Converts combined object and method names (PascalCase) into qualified
    # Kotlin references (@see Object.method) if the method is valid.
    if object_methods:
        for obj_name, methods in object_methods.items():
            # Pattern to find ObjectNameMethodName combinations.
            pattern = r'\b(' + re.escape(obj_name) + r')([A-Z][a-zA-Z0-9_]*)\b'

            def replacer(match):
                """Converts a matched object-method name to a qualified KDoc @see link."""
                object_name = match.group(
                    1)  # The object name, e.g., "Instance"
                arg_name = match.group(2)  # The arg name, e.g., "WaitAny"
                y_camel = arg_name[0].lower() + arg_name[1:]  # e.g., "waitAny"
                if y_camel in methods:
                    kotlin_str = f'{object_name}.{y_camel}'
                    return kotlin_str if "@see" in doc else f'@see {kotlin_str}'
                return match.group(0)  # Return original if not a valid method

            doc = re.sub(pattern, replacer, doc)

    # Example: @see WGPUBufferUsage_CopySrc -> @see WGPUBufferUsage.CopySrc
    # Converts C-style enum member references (e.g., EnumName_MemberName) to
    # Kotlin-style qualified references (e.g., EnumName.MemberName).
    def convert_c_style_enum_ref(match):
        enum_name, member_name = match.groups()
        if enum_name not in enum_names:
            return match.group(0)
        replacement = f"@see {enum_name}.{member_name}"
        return replacement

    # Example: @see WGPUTextureDimension_2D -> @see WGPUTextureDimension._2D
    # Handles C-style enum members that start with a digit.
    doc = re.sub(r"@see (\w+)_(\d\w*)", r"@see \1._\2", doc)

    # Example: @see WGPUBufferUsage_CopySrc -> @see WGPUBufferUsage.CopySrc
    # Converts standard C-style enum references in @see tags to Kotlin-style.
    doc = re.sub(r"@see (\w+)_(\w+)", convert_c_style_enum_ref, doc)

    # Example: @see my-struct-name -> @see mystructname
    # Removes hyphens from references inside @see tags.
    doc = re.sub(r"@see (\S+-\S+)",
                 lambda m: f"@see {m.group(1).replace('-', '')}", doc)

    # Example: @see Adapter.requestDevice -> @see [Adapter.requestDevice]
    # Wraps qualified member references in @see tags with brackets for KDoc linking.
    doc = re.sub(r"(@see\s+)(\w+\.\w+)", r"\1[\2]", doc)

    # Example: @see Adapter -> @see GPUAdapter
    # Prefixes object names with 'GPU' for Kotlin references.
    if object_methods:
        for obj_name in object_methods.keys():
            doc = re.sub(r'\b' + re.escape(obj_name) + r'\b', 'GPU' + obj_name,
                         doc)

    return doc


# ####################################################################################
# Core Doc Extraction Logic
# ####################################################################################

TARGETED_CATEGORIES = [
    "structure",
    "enum",
    "bitmask",
    "callback function",
    "function",
    "object",
    "constant",
]

# Handler of language-specific cleaning functions
LANGUAGE_DOC_HANDLER = {
    "kotlin": clean_doc_for_kotlin,
}


def clean_raw_doc(doc):
    """Performs basic cleaning of a raw docstring."""
    if not doc or doc.strip() == "TODO":
        return ""
    return doc.replace("\n", " ").strip()


def build_doc_map(by_category, json_data, params=None):
    """Builds a nested, language-agnostic documentation map from JSON data.

  This map contains raw doc strings without any formatting.

  Args:
      by_category: Categorized API data.
      json_data: The raw JSON data.
      params: Language-specific parameters.

  Returns:
      A nested dictionary representing the documentation map.
  """
    if not json_data:
        return {}

    doc_map = defaultdict(lambda: defaultdict(dict))

    # Create a fast lookup for items by name within each category.
    data_lookups = {}
    for category, items in json_data.items():
        if not isinstance(items, list):
            continue

        name_to_item_map = {}
        for item in items:
            # Filter out empty or unnamed items
            if item and "name" in item:
                name_to_item_map[item["name"]] = item
        data_lookups[category] = name_to_item_map

    def find_in_data(category, name):
        """Finds an item in the JSON data with suffix fallback logic."""
        lookup = data_lookups.get(category, {})
        if name in lookup:
            return lookup[name]
        # Handle cases where the JSON doc key doesn't include the full suffix.
        # For example, a function `queue work done` in the JSON
        # corresponds to `queue_work_done_callback` in the source data.
        suffixes_to_try = ["_callback"]
        for suffix in suffixes_to_try:
            if name.endswith(suffix):
                trimmed_name = name[:-len(suffix)]
                if trimmed_name in lookup:
                    return lookup[trimmed_name]
        return None

    def extract_named_docs(dict_list):
        """Extracts name-doc pairs from a list of dictionary items."""
        return {
            item["name"]: clean_raw_doc(item.get("doc"))
            for item in dict_list if item and "name" in item and "doc" in item
        }

    def process_common_doc(category, item_name, data_item):
        """Extracts the primary docstring for a given API item."""
        if "doc" in data_item:
            doc_map[category][item_name]["doc"] = clean_raw_doc(
                data_item["doc"])

    def process_constant(category, item_name, data_item):
        """Extracts documentation for a constant."""
        if "doc" in data_item:
            doc_map[category][item_name] = clean_raw_doc(data_item["doc"])

    def process_struct(category, item_name, data_item):
        """Extracts documentation for a struct and its members."""
        process_common_doc(category, item_name, data_item)
        if "members" in data_item:
            doc_map[category][item_name]["members"] = extract_named_docs(
                data_item["members"])

    def process_collection(category, item_name, data_item):
        """Extracts documentation for a collection (enum/bitmask) and its entries."""
        process_common_doc(category, item_name, data_item)
        if "entries" in data_item:
            doc_map[category][item_name]["entries"] = extract_named_docs(
                data_item["entries"])

    def process_function(category, item_name, data_item):
        """Extracts documentation for a function, its arguments, and return value."""
        process_common_doc(category, item_name, data_item)
        item_docs = doc_map[category][item_name]
        if "args" in data_item:
            item_docs["args"] = extract_named_docs(data_item["args"])
        if data_item.get("returns", {}).get("doc"):
            item_docs["returns_doc"] = clean_raw_doc(
                data_item["returns"]["doc"])

    def process_object(category, item_name, data_item):
        """Extracts documentation for an object, its methods, and their parameters."""
        process_common_doc(category, item_name, data_item)
        method_docs = defaultdict(dict)
        for method in data_item.get("methods", []):
            if not (method and "name" in method):
                continue
            m_docs = method_docs[method["name"]]
            if "doc" in method:
                m_docs["doc"] = clean_raw_doc(method["doc"])
            if method.get("returns", {}).get("doc"):
                m_docs["returns_doc"] = clean_raw_doc(method["returns"]["doc"])
            if "args" in method:
                m_docs["args"] = extract_named_docs(method["args"])
        if method_docs:
            doc_map[category][item_name]["methods"] = method_docs

    # Maps API categories to their corresponding names in the JSON data.
    category_map = {
        "structure": "structs",
        "enum": "enums",
        "bitmask": "bitflags",
        "callback function": "callbacks",
        "function": "functions",
        "object": "objects",
        "constant": "constants",
    }
    # Maps JSON data categories to their respective processing functions.
    handler_map = {
        "structs": process_struct,
        "enums": process_collection,
        "bitflags": process_collection,
        "callbacks": process_function,
        "functions": process_function,
        "objects": process_object,
        "constants": process_constant,
    }

    # Create a lookup for API objects from by_category
    by_category_lookups = {}
    for category, items in by_category.items():
        data_category = category_map.get(category)
        if not data_category:
            continue
        name_to_item_map = {}
        for item in items:
            name_to_item_map[item.name.get()] = item
        by_category_lookups[data_category] = name_to_item_map

    # Iterate through categorized API data and extract docs from corresponding JSON data.
    for category, items in by_category.items():
        data_category = category_map.get(category)
        if not data_category or data_category not in handler_map:
            continue
        handler = handler_map[data_category]
        for item in items:
            item_name_snake = item.name.snake_case()
            data_item = find_in_data(data_category, item_name_snake)
            if data_item:
                handler(data_category, item.name.get(), data_item)

    doc_warn_log_filepath = params.get("doc_warn_log_filepath")
    if doc_warn_log_filepath:
        with open(doc_warn_log_filepath, 'w') as log_f:
            # Log incomplete documentation by iterating through the generated doc_map.
            log_incomplete_documentation(log_f, doc_map, by_category_lookups)

    return cleanup_doc_map(doc_map=doc_map,
                           by_category=by_category,
                           params=params)


def log_incomplete_documentation(log_file, doc_map, by_category_lookups):
    """
    Logs incomplete documentation by iterating through the generated doc_map.
    """
    for data_category, items_in_doc_map in doc_map.items():
        name_to_obj = by_category_lookups.get(data_category)
        if not name_to_obj:
            continue
        for item_name, doc_node in items_in_doc_map.items():
            item_obj = name_to_obj.get(item_name)
            if not item_obj:
                continue

            # Check for the main documentation of the item.
            is_main_doc_missing = False
            if data_category == 'constants':
                if not doc_node or not doc_node.strip():
                    is_main_doc_missing = True
            elif isinstance(doc_node, dict):
                if not doc_node.get('doc') or not doc_node.get('doc').strip():
                    is_main_doc_missing = True

            if is_main_doc_missing:
                log_warning(
                    log_file,
                    f"Missing main documentation for {data_category} '{item_name}'."
                )

            # Check for documentation of item parts (members, arguments, etc.).
            if data_category == 'structs':
                check_and_log_incomplete_docs(log_file,
                                              item_name,
                                              data_category,
                                              item_obj.members,
                                              doc_node.get("members", {}),
                                              part_type='members')
            elif data_category in ['callbacks', 'functions']:
                check_and_log_incomplete_docs(log_file,
                                              item_name,
                                              data_category,
                                              item_obj.arguments,
                                              doc_node.get("args", {}),
                                              part_type='arguments')
            elif data_category == 'objects':
                method_docs = doc_node.get("methods", {})
                if not method_docs and item_obj.methods:
                    log_warning(
                        log_file,
                        f"Missing documentation for all methods of object '{item_name}'."
                    )
                    continue

                for method_obj in item_obj.methods:
                    method_name_camel = method_obj.name.camelCase()
                    method_name_snake = method_obj.name.snake_case()
                    method_full_name = f"{item_name}.{method_name_camel}"

                    doc_for_method = method_docs.get(method_name_snake)

                    if not doc_for_method or not doc_for_method.get(
                            'doc') or not doc_for_method.get('doc').strip():
                        log_warning(
                            log_file,
                            f"Missing main documentation for method '{method_full_name}'."
                        )

                    if doc_for_method:
                        check_and_log_incomplete_docs(log_file,
                                                      method_full_name,
                                                      "method",
                                                      method_obj.arguments,
                                                      doc_for_method.get(
                                                          "args", {}),
                                                      part_type='arguments')


def cleanup_doc_map(doc_map, by_category, params):
    """Post-processes the documentation map to fix and validate cross-references for a specific target language.

  Args:
      doc_map: The raw documentation map from build_doc_map.
      by_category: The categorized API data structure from dawn.json.
      params: Language specific parameters

  Returns:
      The post-processed documentation map.
  """
    if "language" not in params:
        return doc_map

    language = params.get("language")

    cleaner_func = LANGUAGE_DOC_HANDLER.get(language)
    if not cleaner_func:
        return doc_map

    # Collect all valid method names for each object for doc generation.
    object_methods = defaultdict(set)
    for obj in by_category.get("object", []):
        obj_name = obj.name.CamelCase()
        for method in obj.methods:
            object_methods[obj_name].add(method.name.camelCase())

    # Create a set of all valid, normalized names for quick lookups.
    all_valid_names_normalized = set()
    for cat in TARGETED_CATEGORIES:
        for item in by_category.get(cat, []):
            all_valid_names_normalized.add(item.name.CamelCase().lower())

    # Create a set of all enum and bitmask names for reference cleaning.
    enum_and_bitflag_names = set()
    for cat in ("enum", "bitmask"):
        for item in by_category.get(cat, []):
            enum_and_bitflag_names.add(item.name.CamelCase())

    def recursive_clean(d):
        """Recursively traverses the doc map and applies the language-specific cleaner."""
        if isinstance(d, dict):
            for k, v in d.items():
                if isinstance(v, (dict, list)):
                    recursive_clean(v)
                elif isinstance(v, str):
                    # Apply the selected language cleaner
                    d[k] = cleaner_func(
                        v,
                        valid_names_normalized=all_valid_names_normalized,
                        enum_names=enum_and_bitflag_names,
                        params=params,
                        object_methods=object_methods,
                    )
        elif isinstance(d, list):
            for item in d:
                recursive_clean(item)

    recursive_clean(doc_map)

    return doc_map


def load_json_data(path: str):
    """Loads and parses JSON data from a file."""
    with open(path) as f:
        return json.load(f)
