/*
  Copyright (c) 2009-2017 Dave Gamble and cJSON contributors

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

#ifndef cJSON__h
#define cJSON__h

#ifdef __cplusplus
extern "C" {
#endif

#define CJSON_HIDE_SYMBOLS

#if !defined(__WINDOWS__) && (defined(WIN32) || defined(WIN64) || defined(_MSC_VER) || defined(_WIN32))
#define __WINDOWS__
#endif

#ifdef __WINDOWS__

/* When compiling for windows, we specify a specific calling convention to avoid issues where we are being called from a project
with a different default calling convention.  For windows you have 3 define options:

CJSON_HIDE_SYMBOLS - Define this in the case where you don't want to ever dllexport symbols
CJSON_EXPORT_SYMBOLS - Define this on library build when you want to dllexport symbols (default)
CJSON_IMPORT_SYMBOLS - Define this if you want to dllimport symbol

For *nix builds that support visibility attribute, you can define similar behavior by

setting default visibility to hidden by adding
-fvisibility=hidden (for gcc)
or
-xldscope=hidden (for sun cc)
to CFLAGS

then using the CJSON_API_VISIBILITY flag to "export" the same symbols the way CJSON_EXPORT_SYMBOLS does

*/

#define CJSON_CDECL __cdecl
#define CJSON_STDCALL __stdcall

/* export symbols by default, this is necessary for copy pasting the C and header file */
#if !defined(CJSON_HIDE_SYMBOLS) && !defined(CJSON_IMPORT_SYMBOLS) && !defined(CJSON_EXPORT_SYMBOLS)
#define CJSON_EXPORT_SYMBOLS
#endif

#if defined(CJSON_HIDE_SYMBOLS)
#define CJSON_PUBLIC(type) type CJSON_STDCALL
#elif defined(CJSON_EXPORT_SYMBOLS)
#define CJSON_PUBLIC(type) __declspec(dllexport) type CJSON_STDCALL
#elif defined(CJSON_IMPORT_SYMBOLS)
#define CJSON_PUBLIC(type) __declspec(dllimport) type CJSON_STDCALL
#endif
#else /* !__WINDOWS__ */
#define CJSON_CDECL
#define CJSON_STDCALL

#if (defined(__GNUC__) || defined(__SUNPRO_CC) || defined(__SUNPRO_C)) && defined(CJSON_API_VISIBILITY)
#define CJSON_PUBLIC(type) __attribute__((visibility("default"))) type
#else
#define CJSON_PUBLIC(type) type
#endif
#endif

/* project version */
#define CJSON_VERSION_MAJOR 1
#define CJSON_VERSION_MINOR 7
#define CJSON_VERSION_PATCH 18

#include <stddef.h>
#include <stdbool.h>

#include "vk_loader_platform.h"

/* cJSON Types: */
#define cJSON_Invalid (0)
#define cJSON_False (1 << 0)
#define cJSON_True (1 << 1)
#define cJSON_NULL (1 << 2)
#define cJSON_Number (1 << 3)
#define cJSON_String (1 << 4)
#define cJSON_Array (1 << 5)
#define cJSON_Object (1 << 6)
#define cJSON_Raw (1 << 7) /* raw json */

#define cJSON_IsReference 256
#define cJSON_StringIsConst 512

/* loader type declarations for allocation hooks */
typedef struct VkAllocationCallbacks VkAllocationCallbacks;
struct loader_instance;

/* The cJSON structure: */
typedef struct cJSON {
    /* next/prev allow you to walk array/object chains. Alternatively, use GetArraySize/GetArrayItem/GetObjectItem */
    struct cJSON *next;
    struct cJSON *prev;
    /* An array or object item will have a child pointer pointing to a chain of the items in the array/object. */
    struct cJSON *child;

    /* The type of the item, as above. */
    int type;

    /* The item's string, if type==cJSON_String  and type == cJSON_Raw */
    char *valuestring;
    /* writing to valueint is DEPRECATED, use cJSON_SetNumberValue instead */
    int valueint;
    /* The item's number, if type==cJSON_Number */
    double valuedouble;

    /* The item's name string, if this item is the child of, or is in the list of subitems of an object. */
    char *string;
    /* pointer to the allocation callbacks to use */
    const VkAllocationCallbacks *pAllocator;
} cJSON;

typedef int cJSON_bool;

/* Limits how deeply nested arrays/objects can be before cJSON rejects to parse them.
 * This is to prevent stack overflows. */
#ifndef CJSON_NESTING_LIMIT
#define CJSON_NESTING_LIMIT 1000
#endif

/* Limits the length of circular references can be before cJSON rejects to parse them.
 * This is to prevent stack overflows. */
#ifndef CJSON_CIRCULAR_LIMIT
#define CJSON_CIRCULAR_LIMIT 10000
#endif

/* Memory Management: the caller is always responsible to free instthe results from all variants of loader_cJSON_Parse (with
 * loader_cJSON_Delete) and loader_loader_cJSON_Print (with stdlib free, cJSON_Hooks.free_fn, or cJSON_free as appropriate). The
 * exception is cJSON_PrintPreallocated, where the caller has full responsibility of the buffer. */
/* Supply a block of JSON, and this returns a cJSON object you can interrogate. */
CJSON_PUBLIC(cJSON *) loader_cJSON_Parse(const VkAllocationCallbacks *pAllocator, const char *value, bool *out_of_memory);
CJSON_PUBLIC(cJSON *)
loader_cJSON_ParseWithLength(const VkAllocationCallbacks *pAllocator, const char *value, size_t buffer_length, bool *out_of_memory);
/* ParseWithOpts allows you to require (and check) that the JSON is null terminated, and to retrieve the pointer to the final byte
 * parsed. */
/* If you supply a ptr in return_parse_end and parsing fails, then return_parse_end will contain a pointer to the error so will
 * match cJSON_GetErrorPtr(). */
CJSON_PUBLIC(cJSON *)
loader_cJSON_ParseWithOpts(const VkAllocationCallbacks *pAllocator, const char *value, const char **return_parse_end,
                           cJSON_bool require_null_terminated, bool *out_of_memory);
CJSON_PUBLIC(cJSON *)
loader_cJSON_ParseWithLengthOpts(const VkAllocationCallbacks *pAllocator, const char *value, size_t buffer_length,
                                 const char **return_parse_end, cJSON_bool require_null_terminated, bool *out_of_memory);

/* Render a cJSON entity to text for transfer/storage. */
TEST_FUNCTION_EXPORT CJSON_PUBLIC(char *) loader_cJSON_Print(const cJSON *item, bool *out_of_memory);
/* Render a cJSON entity to text for transfer/storage without any formatting. */
CJSON_PUBLIC(char *) loader_cJSON_PrintUnformatted(const cJSON *item, bool *out_of_memory);
/* Render a cJSON entity to text using a buffered strategy. prebuffer is a guess at the final size. guessing well reduces
 * reallocation. fmt=0 gives unformatted, =1 gives formatted */
CJSON_PUBLIC(char *)
loader_cJSON_PrintBuffered(const cJSON *item, int prebuffer, cJSON_bool fmt, bool *out_of_memory);
/* Render a cJSON entity to text using a buffer already allocated in memory with given length. Returns 1 on success and 0 on
 * failure. */
/* NOTE: cJSON is not always 100% accurate in estimating how much memory it will use, so to be safe allocate 5 bytes more than you
 * actually need */
CJSON_PUBLIC(cJSON_bool)
loader_cJSON_PrintPreallocated(cJSON *item, char *buffer, const int length, const cJSON_bool format);
/* Delete a cJSON entity and all subentities. */
TEST_FUNCTION_EXPORT CJSON_PUBLIC(void) loader_cJSON_Delete(cJSON *item);

/* Returns the number of items in an array (or object). */
CJSON_PUBLIC(int) loader_cJSON_GetArraySize(const cJSON *array);
/* Retrieve item number "index" from array "array". Returns NULL if unsuccessful. */
CJSON_PUBLIC(cJSON *) loader_cJSON_GetArrayItem(const cJSON *array, int index);
/* Get item "string" from object. Case insensitive. */
CJSON_PUBLIC(cJSON *) loader_cJSON_GetObjectItem(const cJSON *const object, const char *const string);
CJSON_PUBLIC(cJSON *) loader_cJSON_GetObjectItemCaseSensitive(const cJSON *const object, const char *const string);
CJSON_PUBLIC(cJSON_bool) loader_cJSON_HasObjectItem(const cJSON *object, const char *string);
/* For analysing failed parses. This returns a pointer to the parse error. You'll probably need to look a few chars back to make
 * sense of it. Defined when loader_cJSON_Parse() returns 0. 0 when loader_cJSON_Parse() succeeds. */
CJSON_PUBLIC(const char *) cJSON_GetErrorPtr(void);

/* Check item type and return its value */
CJSON_PUBLIC(char *) loader_cJSON_GetStringValue(const cJSON *const item);
CJSON_PUBLIC(double) loader_cJSON_GetNumberValue(const cJSON *const item);

/* These functions check the type of an item */
CJSON_PUBLIC(cJSON_bool) loader_cJSON_IsInvalid(const cJSON *const item);
CJSON_PUBLIC(cJSON_bool) loader_cJSON_IsFalse(const cJSON *const item);
CJSON_PUBLIC(cJSON_bool) loader_cJSON_IsTrue(const cJSON *const item);
CJSON_PUBLIC(cJSON_bool) loader_cJSON_IsBool(const cJSON *const item);
CJSON_PUBLIC(cJSON_bool) loader_cJSON_IsNull(const cJSON *const item);
CJSON_PUBLIC(cJSON_bool) loader_cJSON_IsNumber(const cJSON *const item);
CJSON_PUBLIC(cJSON_bool) loader_cJSON_IsString(const cJSON *const item);
CJSON_PUBLIC(cJSON_bool) loader_cJSON_IsArray(const cJSON *const item);
CJSON_PUBLIC(cJSON_bool) loader_cJSON_IsObject(const cJSON *const item);
CJSON_PUBLIC(cJSON_bool) loader_cJSON_IsRaw(const cJSON *const item);

/* Macro for iterating over an array or object */
#define cJSON_ArrayForEach(element, array) \
    for (element = (array != NULL) ? (array)->child : NULL; element != NULL; element = element->next)

#ifdef __cplusplus
}
#endif

#endif
