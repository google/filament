/*
  Simple DirectMedia Layer
  Copyright (C) 1997-2018 Sam Lantinga <slouken@libsdl.org>

  This software is provided 'as-is', without any express or implied
  warranty.  In no event will the authors be held liable for any damages
  arising from the use of this software.

  Permission is granted to anyone to use this software for any purpose,
  including commercial applications, and to alter it and redistribute it
  freely, subject to the following restrictions:

  1. The origin of this software must not be misrepresented; you must not
     claim that you wrote the original software. If you use this software
     in a product, an acknowledgment in the product documentation would be
     appreciated but is not required.
  2. Altered source versions must be plainly marked as such, and must not be
     misrepresented as being the original software.
  3. This notice may not be removed or altered from any source distribution.
*/

/* 
 * @author Mark Callow, www.edgewise-consulting.com. Based on Jacob Lifshay's
 * SDL_x11vulkan.c.
 */

#include "../../SDL_internal.h"

#if SDL_VIDEO_VULKAN && SDL_VIDEO_DRIVER_COCOA

#include "SDL_cocoavideo.h"
#include "SDL_cocoawindow.h"
#include "SDL_assert.h"

#include "SDL_loadso.h"
#include "SDL_cocoametalview.h"
#include "SDL_cocoavulkan.h"
#include "SDL_syswm.h"

#include <dlfcn.h>

const char* defaultPaths[] = {
    "vulkan.framework/vulkan",
    "libvulkan.1.dylib",
    "MoltenVK.framework/MoltenVK",
    "libMoltenVK.dylib"
};

/* Since libSDL is most likely a .dylib, need RTLD_DEFAULT not RTLD_SELF. */
#define DEFAULT_HANDLE RTLD_DEFAULT

int Cocoa_Vulkan_LoadLibrary(_THIS, const char *path)
{
    VkExtensionProperties *extensions = NULL;
    Uint32 extensionCount = 0;
    SDL_bool hasSurfaceExtension = SDL_FALSE;
    SDL_bool hasMacOSSurfaceExtension = SDL_FALSE;
    PFN_vkGetInstanceProcAddr vkGetInstanceProcAddr = NULL;

    if (_this->vulkan_config.loader_handle) {
        SDL_SetError("Vulkan/MoltenVK already loaded");
        return -1;
    }

    /* Load the Vulkan loader library */
    if (!path) {
        path = SDL_getenv("SDL_VULKAN_LIBRARY");
    }

    if (!path) {
        /* MoltenVK framework, currently, v0.17.0, has a static library and is
         * the recommended way to use the package. There is likely no object to
         * load. */
        vkGetInstanceProcAddr =
         (PFN_vkGetInstanceProcAddr)dlsym(DEFAULT_HANDLE,
                                          "vkGetInstanceProcAddr");
    }

    if (vkGetInstanceProcAddr) {
        _this->vulkan_config.loader_handle = DEFAULT_HANDLE;
    } else {
        const char** paths;
        int numPaths;
        int i;

        if (path) {
            paths = &path;
            numPaths = 1;
        } else {
            /* Look for framework or .dylib packaged with the application
             * instead. */
            paths = defaultPaths;
            numPaths = SDL_arraysize(defaultPaths);
        }
        
        for (i=0; i < numPaths; i++) {
            _this->vulkan_config.loader_handle = SDL_LoadObject(paths[i]);
            if (_this->vulkan_config.loader_handle)
                break;
            else
                continue;
        }
        if (i == numPaths)
            return -1;

        SDL_strlcpy(_this->vulkan_config.loader_path, paths[i],
                    SDL_arraysize(_this->vulkan_config.loader_path));
        vkGetInstanceProcAddr = (PFN_vkGetInstanceProcAddr)SDL_LoadFunction(
            _this->vulkan_config.loader_handle, "vkGetInstanceProcAddr");
    }

    if (!vkGetInstanceProcAddr) {
        SDL_SetError("Failed to find %s in either executable or %s: %s",
                     "vkGetInstanceProcAddr",
                     _this->vulkan_config.loader_path,
                     (const char *) dlerror());
        goto fail;
    }

    _this->vulkan_config.vkGetInstanceProcAddr = (void *)vkGetInstanceProcAddr;
    _this->vulkan_config.vkEnumerateInstanceExtensionProperties =
        (void *)((PFN_vkGetInstanceProcAddr)_this->vulkan_config.vkGetInstanceProcAddr)(
            VK_NULL_HANDLE, "vkEnumerateInstanceExtensionProperties");
    if (!_this->vulkan_config.vkEnumerateInstanceExtensionProperties) {
        goto fail;
    }
    extensions = SDL_Vulkan_CreateInstanceExtensionsList(
        (PFN_vkEnumerateInstanceExtensionProperties)
            _this->vulkan_config.vkEnumerateInstanceExtensionProperties,
        &extensionCount);
    if (!extensions) {
        goto fail;
    }
    for (Uint32 i = 0; i < extensionCount; i++) {
        if (SDL_strcmp(VK_KHR_SURFACE_EXTENSION_NAME, extensions[i].extensionName) == 0) {
            hasSurfaceExtension = SDL_TRUE;
        } else if (SDL_strcmp(VK_MVK_MACOS_SURFACE_EXTENSION_NAME, extensions[i].extensionName) == 0) {
            hasMacOSSurfaceExtension = SDL_TRUE;
        }
    }
    SDL_free(extensions);
    if (!hasSurfaceExtension) {
        SDL_SetError("Installed MoltenVK/Vulkan doesn't implement the "
                     VK_KHR_SURFACE_EXTENSION_NAME " extension");
        goto fail;
    } else if (!hasMacOSSurfaceExtension) {
        SDL_SetError("Installed MoltenVK/Vulkan doesn't implement the "
                     VK_MVK_MACOS_SURFACE_EXTENSION_NAME "extension");
        goto fail;
    }
    return 0;

fail:
    SDL_UnloadObject(_this->vulkan_config.loader_handle);
    _this->vulkan_config.loader_handle = NULL;
    return -1;
}

void Cocoa_Vulkan_UnloadLibrary(_THIS)
{
    if (_this->vulkan_config.loader_handle) {
        if (_this->vulkan_config.loader_handle != DEFAULT_HANDLE) {
            SDL_UnloadObject(_this->vulkan_config.loader_handle);
        }
        _this->vulkan_config.loader_handle = NULL;
    }
}

SDL_bool Cocoa_Vulkan_GetInstanceExtensions(_THIS,
                                          SDL_Window *window,
                                          unsigned *count,
                                          const char **names)
{
    static const char *const extensionsForCocoa[] = {
        VK_KHR_SURFACE_EXTENSION_NAME, VK_MVK_MACOS_SURFACE_EXTENSION_NAME
    };
    if (!_this->vulkan_config.loader_handle) {
        SDL_SetError("Vulkan is not loaded");
        return SDL_FALSE;
    }
    return SDL_Vulkan_GetInstanceExtensions_Helper(
            count, names, SDL_arraysize(extensionsForCocoa),
            extensionsForCocoa);
}

SDL_bool Cocoa_Vulkan_CreateSurface(_THIS,
                                  SDL_Window *window,
                                  VkInstance instance,
                                  VkSurfaceKHR *surface)
{
    PFN_vkGetInstanceProcAddr vkGetInstanceProcAddr =
        (PFN_vkGetInstanceProcAddr)_this->vulkan_config.vkGetInstanceProcAddr;
    PFN_vkCreateMacOSSurfaceMVK vkCreateMacOSSurfaceMVK =
        (PFN_vkCreateMacOSSurfaceMVK)vkGetInstanceProcAddr(
                                            (VkInstance)instance,
                                            "vkCreateMacOSSurfaceMVK");
    VkMacOSSurfaceCreateInfoMVK createInfo = {};
    VkResult result;

    if (!_this->vulkan_config.loader_handle) {
        SDL_SetError("Vulkan is not loaded");
        return SDL_FALSE;
    }

    if (!vkCreateMacOSSurfaceMVK) {
        SDL_SetError(VK_MVK_MACOS_SURFACE_EXTENSION_NAME
                     " extension is not enabled in the Vulkan instance.");
        return SDL_FALSE;
    }
    createInfo.sType = VK_STRUCTURE_TYPE_MACOS_SURFACE_CREATE_INFO_MVK;
    createInfo.pNext = NULL;
    createInfo.flags = 0;
    createInfo.pView = Cocoa_Mtl_AddMetalView(window);
    result = vkCreateMacOSSurfaceMVK(instance, &createInfo,
                                       NULL, surface);
    if (result != VK_SUCCESS) {
        SDL_SetError("vkCreateMacOSSurfaceMVK failed: %s",
                     SDL_Vulkan_GetResultString(result));
        return SDL_FALSE;
    }
    return SDL_TRUE;
}

void Cocoa_Vulkan_GetDrawableSize(_THIS, SDL_Window *window, int *w, int *h)
{
    Cocoa_Mtl_GetDrawableSize(window, w, h);
}

#endif

/* vim: set ts=4 sw=4 expandtab: */
