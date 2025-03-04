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

#include "loader_json.h"

#include <assert.h>
#include <float.h>
#include <limits.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cJSON.h"

#include "allocation.h"
#include "loader.h"
#include "log.h"

#if COMMON_UNIX_PLATFORMS
#include <fcntl.h>
#include <sys/stat.h>
#endif

#ifdef _WIN32
static VkResult loader_read_entire_file(const struct loader_instance *inst, const char *filename, size_t *out_len,
                                        char **out_buff) {
    HANDLE file_handle = INVALID_HANDLE_VALUE;
    DWORD len = 0, read_len = 0;
    VkResult res = VK_SUCCESS;
    BOOL read_ok = false;

    int filename_utf16_size = MultiByteToWideChar(CP_UTF8, 0, filename, -1, NULL, 0);
    if (filename_utf16_size > 0) {
        wchar_t *filename_utf16 = (wchar_t *)loader_stack_alloc(filename_utf16_size * sizeof(wchar_t));
        if (MultiByteToWideChar(CP_UTF8, 0, filename, -1, filename_utf16, filename_utf16_size) == filename_utf16_size) {
            file_handle =
                CreateFileW(filename_utf16, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
        }
    }
    if (INVALID_HANDLE_VALUE == file_handle) {
        loader_log(inst, VULKAN_LOADER_ERROR_BIT, 0, "loader_get_json: Failed to open JSON file %s", filename);
        res = VK_ERROR_INITIALIZATION_FAILED;
        goto out;
    }
    len = GetFileSize(file_handle, NULL);
    if (INVALID_FILE_SIZE == len) {
        loader_log(inst, VULKAN_LOADER_ERROR_BIT, 0, "loader_get_json: Failed to read file size of JSON file %s", filename);
        res = VK_ERROR_INITIALIZATION_FAILED;
        goto out;
    }
    *out_buff = (char *)loader_instance_heap_calloc(inst, len + 1, VK_SYSTEM_ALLOCATION_SCOPE_COMMAND);
    if (NULL == *out_buff) {
        loader_log(inst, VULKAN_LOADER_ERROR_BIT, 0, "loader_get_json: Failed to allocate memory to read JSON file %s", filename);
        res = VK_ERROR_OUT_OF_HOST_MEMORY;
        goto out;
    }
    read_ok = ReadFile(file_handle, *out_buff, len, &read_len, NULL);
    if (len != read_len || false == read_ok) {
        loader_log(inst, VULKAN_LOADER_ERROR_BIT, 0, "loader_get_json: Failed to read entire JSON file %s", filename);
        res = VK_ERROR_INITIALIZATION_FAILED;
        goto out;
    }
    *out_len = len + 1;
    (*out_buff)[len] = '\0';

out:
    if (INVALID_HANDLE_VALUE != file_handle) {
        CloseHandle(file_handle);
    }
    return res;
}
#elif COMMON_UNIX_PLATFORMS
static VkResult loader_read_entire_file(const struct loader_instance *inst, const char *filename, size_t *out_len,
                                        char **out_buff) {
    FILE *file = NULL;
    struct stat stats = {0};
    VkResult res = VK_SUCCESS;

    file = fopen(filename, "rb");
    if (NULL == file) {
        loader_log(inst, VULKAN_LOADER_ERROR_BIT, 0, "loader_get_json: Failed to open JSON file %s", filename);
        res = VK_ERROR_INITIALIZATION_FAILED;
        goto out;
    }
    if (-1 == fstat(fileno(file), &stats)) {
        loader_log(inst, VULKAN_LOADER_ERROR_BIT, 0, "loader_get_json: Failed to read file size of JSON file %s", filename);
        res = VK_ERROR_INITIALIZATION_FAILED;
        goto out;
    }
    *out_buff = (char *)loader_instance_heap_calloc(inst, stats.st_size + 1, VK_SYSTEM_ALLOCATION_SCOPE_COMMAND);
    if (NULL == *out_buff) {
        loader_log(inst, VULKAN_LOADER_ERROR_BIT, 0, "loader_get_json: Failed to allocate memory to read JSON file %s", filename);
        res = VK_ERROR_OUT_OF_HOST_MEMORY;
        goto out;
    }
    if (stats.st_size != (long int)fread(*out_buff, sizeof(char), stats.st_size, file)) {
        loader_log(inst, VULKAN_LOADER_ERROR_BIT, 0, "loader_get_json: Failed to read entire JSON file %s", filename);
        res = VK_ERROR_INITIALIZATION_FAILED;
        goto out;
    }
    *out_len = stats.st_size + 1;
    (*out_buff)[stats.st_size] = '\0';

out:
    if (NULL != file) {
        fclose(file);
    }
    return res;
}
#else
#warning fopen not available on this platform
VkResult loader_read_entire_file(const struct loader_instance *inst, const char *filename, size_t *out_len, char **out_buff) {
    return VK_ERROR_INITIALIZATION_FAILED;
}
#endif

TEST_FUNCTION_EXPORT VkResult loader_get_json(const struct loader_instance *inst, const char *filename, cJSON **json) {
    char *json_buf = NULL;
    VkResult res = VK_SUCCESS;

    assert(json != NULL);

    size_t json_len = 0;
    *json = NULL;
    res = loader_read_entire_file(inst, filename, &json_len, &json_buf);
    if (VK_SUCCESS != res) {
        goto out;
    }
    bool out_of_memory = false;
    // Parse text from file
    *json = loader_cJSON_ParseWithLength(inst ? &inst->alloc_callbacks : NULL, json_buf, json_len, &out_of_memory);
    if (out_of_memory) {
        loader_log(inst, VULKAN_LOADER_ERROR_BIT, 0, "loader_get_json: Out of Memory error occurred while parsing JSON file %s.",
                   filename);
        res = VK_ERROR_OUT_OF_HOST_MEMORY;
        goto out;
    } else if (*json == NULL) {
        loader_log(inst, VULKAN_LOADER_ERROR_BIT, 0, "loader_get_json: Invalid JSON file %s.", filename);
        goto out;
    }

out:
    loader_instance_heap_free(inst, json_buf);
    if (res != VK_SUCCESS && *json != NULL) {
        loader_cJSON_Delete(*json);
        *json = NULL;
    }

    return res;
}

VkResult loader_parse_json_string_to_existing_str(cJSON *object, const char *key, size_t out_str_len, char *out_string) {
    if (NULL == key) {
        return VK_ERROR_INITIALIZATION_FAILED;
    }
    cJSON *item = loader_cJSON_GetObjectItem(object, key);
    if (NULL == item) {
        return VK_ERROR_INITIALIZATION_FAILED;
    }

    if (item->type != cJSON_String || item->valuestring == NULL) {
        return VK_ERROR_INITIALIZATION_FAILED;
    }
    bool out_of_memory = false;
    bool success = loader_cJSON_PrintPreallocated(item, out_string, (int)out_str_len, cJSON_False);
    if (out_of_memory) {
        return VK_ERROR_OUT_OF_HOST_MEMORY;
    }
    if (!success) {
        return VK_ERROR_INITIALIZATION_FAILED;
    }
    return VK_SUCCESS;
}

VkResult loader_parse_json_string(cJSON *object, const char *key, char **out_string) {
    if (NULL == key) {
        return VK_ERROR_INITIALIZATION_FAILED;
    }

    cJSON *item = loader_cJSON_GetObjectItem(object, key);
    if (NULL == item || NULL == item->valuestring) {
        return VK_ERROR_INITIALIZATION_FAILED;
    }

    bool out_of_memory = false;
    char *str = loader_cJSON_Print(item, &out_of_memory);
    if (out_of_memory || NULL == str) {
        return VK_ERROR_OUT_OF_HOST_MEMORY;
    }
    if (NULL != out_string) {
        *out_string = str;
    }
    return VK_SUCCESS;
}
VkResult loader_parse_json_array_of_strings(const struct loader_instance *inst, cJSON *object, const char *key,
                                            struct loader_string_list *string_list) {
    if (NULL == key) {
        return VK_ERROR_INITIALIZATION_FAILED;
    }
    cJSON *item = loader_cJSON_GetObjectItem(object, key);
    if (NULL == item) {
        return VK_ERROR_INITIALIZATION_FAILED;
    }

    uint32_t count = loader_cJSON_GetArraySize(item);
    if (count == 0) {
        return VK_SUCCESS;
    }

    VkResult res = create_string_list(inst, count, string_list);
    if (VK_ERROR_OUT_OF_HOST_MEMORY == res) {
        goto out;
    }
    cJSON *element = NULL;
    cJSON_ArrayForEach(element, item) {
        if (element->type != cJSON_String) {
            return VK_ERROR_INITIALIZATION_FAILED;
        }
        bool out_of_memory = false;
        char *out_data = loader_cJSON_Print(element, &out_of_memory);
        if (out_of_memory) {
            res = VK_ERROR_OUT_OF_HOST_MEMORY;
            goto out;
        }
        res = append_str_to_string_list(inst, string_list, out_data);
        if (VK_ERROR_OUT_OF_HOST_MEMORY == res) {
            goto out;
        }
    }
out:
    if (res == VK_ERROR_OUT_OF_HOST_MEMORY && NULL != string_list->list) {
        free_string_list(inst, string_list);
    }

    return res;
}
