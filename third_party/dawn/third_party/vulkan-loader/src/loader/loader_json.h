/*
  Copyright (c) 2015-2021 The Khronos Group Inc.
  Copyright (c) 2015-2021 Valve Corporation
  Copyright (c) 2015-2021 LunarG, Inc.

  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files (the "Software"), to deal
  in the Software without restriction, including without limitation the rights
  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
  copies of the Software, and to permit persons to whom the Software is
  furnished to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included in
  all copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
  THE SOFTWARE.
*/

#pragma once

#include <stddef.h>

#include <vulkan/vulkan_core.h>

#include <cJSON.h>

// Forward decls
struct loader_instance;
struct loader_string_list;

// Read a JSON file into a buffer.
//
// @return -  A pointer to a cJSON object representing the JSON parse tree.
//            This returned buffer should be freed by caller.
TEST_FUNCTION_EXPORT VkResult loader_get_json(const struct loader_instance *inst, const char *filename, cJSON **json);

// Given a cJSON object, find the string associated with the key and puts an pre-allocated string into out_string.
// Length is given by out_str_len, and this function truncates the string with a null terminator if it the provided space isn't
// large enough.
VkResult loader_parse_json_string_to_existing_str(cJSON *object, const char *key, size_t out_str_len, char *out_string);

// Given a cJSON object, find the string associated with the key and puts an allocated string into out_string.
// It is the callers responsibility to free out_string.
VkResult loader_parse_json_string(cJSON *object, const char *key, char **out_string);

// Given a cJSON object, find the array of strings associated with they key and writes the count into out_count and data into
// out_array_of_strings. It is the callers responsibility to free out_array_of_strings.
VkResult loader_parse_json_array_of_strings(const struct loader_instance *inst, cJSON *object, const char *key,
                                            struct loader_string_list *string_list);
