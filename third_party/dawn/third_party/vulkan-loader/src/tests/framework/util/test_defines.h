/*
 * Copyright (c) 2025 The Khronos Group Inc.
 * Copyright (c) 2025 Valve Corporation
 * Copyright (c) 2025 LunarG, Inc.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and/or associated documentation files (the "Materials"), to
 * deal in the Materials without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Materials, and to permit persons to whom the Materials are
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice(s) and this permission notice shall be included in
 * all copies or substantial portions of the Materials.
 *
 * THE MATERIALS ARE PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 *
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
 * DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
 * OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE MATERIALS OR THE
 * USE OR OTHER DEALINGS IN THE MATERIALS.
 *
 */

#pragma once

#if defined(__GNUC__) && __GNUC__ >= 4
#define FRAMEWORK_EXPORT __attribute__((visibility("default")))
#elif defined(__SUNPRO_C) && (__SUNPRO_C >= 0x590)
#define FRAMEWORK_EXPORT __attribute__((visibility("default")))
#elif defined(_WIN32)
#define FRAMEWORK_EXPORT __declspec(dllexport)
#else
#define FRAMEWORK_EXPORT
#endif

// Set of platforms with a common set of functionality which is queried throughout the program
#if defined(__linux__) || defined(__APPLE__) || defined(__Fuchsia__) || defined(__QNX__) || defined(__FreeBSD__) || \
    defined(__OpenBSD__) || defined(__NetBSD__) || defined(__DragonFly__) || defined(__GNU__)
#define TESTING_COMMON_UNIX_PLATFORMS 1
#else
#define TESTING_COMMON_UNIX_PLATFORMS 0
#endif

#define TESTING_VULKAN_DIR "vulkan/"

#include FRAMEWORK_CONFIG_HEADER

enum class ManifestCategory { implicit_layer, explicit_layer, icd, settings };

// Controls whether to create a manifest and where to put it
enum class ManifestDiscoveryType {
    generic,            // put the manifest in the regular locations
    unsecured_generic,  // put the manifest in a user folder rather than system
    none,               // Do not write the manifest anywhere (for Direct Driver Loading)
    null_dir,           // put the manifest in the 'null_dir' which the loader does not search in (D3DKMT for instance)
    env_var,            // use the corresponding env-var for it
    add_env_var,        // use the corresponding add-env-var for it
    override_folder,    // add to a special folder for the override layer to use
#if defined(_WIN32)
    windows_app_package,  // let the app package search find it
#endif
#if defined(__APPLE__)
    macos_bundle,  // place it in a location only accessible to macos bundles
#endif
};

enum class LibraryPathType {
    absolute,              // default for testing - the exact path of the binary
    relative,              // Relative to the manifest file
    default_search_paths,  // Dont add any path information to the library_path - force the use of the default search paths
};

// Locations that files can go in the test framework
enum class ManifestLocation {
    null,
    driver,
    driver_env_var,
    explicit_layer,
    explicit_layer_env_var,
    explicit_layer_add_env_var,
    implicit_layer,
    implicit_layer_env_var,
    implicit_layer_add_env_var,
    override_layer,
#if defined(_WIN32)
    windows_app_package,
#endif
#if defined(__APPLE__)
    macos_bundle,
#endif
    settings_location,
    unsecured_driver,
    unsecured_explicit_layer,
    unsecured_implicit_layer,
    unsecured_settings,
};
