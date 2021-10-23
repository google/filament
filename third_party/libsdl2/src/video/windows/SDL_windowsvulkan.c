/*
  Simple DirectMedia Layer
  Copyright (C) 1997-2021 Sam Lantinga <slouken@libsdl.org>

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

#if SDL_VIDEO_VULKAN && SDL_VIDEO_DRIVER_WINDOWS

#include "SDL_windowsvideo.h"
#include "SDL_windowswindow.h"

#include "SDL_loadso.h"
#include "SDL_windowsvulkan.h"
#include "SDL_syswm.h"

int WIN_Vulkan_LoadLibrary(_THIS, const char *path)
{
    VkExtensionProperties *extensions = NULL;
    Uint32 extensionCount = 0;
    Uint32 i;
    SDL_bool hasSurfaceExtension = SDL_FALSE;
    SDL_bool hasWin32SurfaceExtension = SDL_FALSE;
    PFN_vkGetInstanceProcAddr vkGetInstanceProcAddr = NULL;
    if(_this->vulkan_config.loader_handle)
        return SDL_SetError("Vulkan already loaded");

    /* Load the Vulkan loader library */
    if(!path)
        path = SDL_getenv("SDL_VULKAN_LIBRARY");
    if(!path)
        path = "vulkan-1.dll";
    _this->vulkan_config.loader_handle = SDL_LoadObject(path);
    if(!_this->vulkan_config.loader_handle)
        return -1;
    SDL_strlcpy(_this->vulkan_config.loader_path, path,
                SDL_arraysize(_this->vulkan_config.loader_path));
    vkGetInstanceProcAddr = (PFN_vkGetInstanceProcAddr)SDL_LoadFunction(
        _this->vulkan_config.loader_handle, "vkGetInstanceProcAddr");
    if(!vkGetInstanceProcAddr)
        goto fail;
    _this->vulkan_config.vkGetInstanceProcAddr = (void *)vkGetInstanceProcAddr;
    _this->vulkan_config.vkEnumerateInstanceExtensionProperties =
        (void *)((PFN_vkGetInstanceProcAddr)_this->vulkan_config.vkGetInstanceProcAddr)(
            VK_NULL_HANDLE, "vkEnumerateInstanceExtensionProperties");
    if(!_this->vulkan_config.vkEnumerateInstanceExtensionProperties)
        goto fail;
    extensions = SDL_Vulkan_CreateInstanceExtensionsList(
        (PFN_vkEnumerateInstanceExtensionProperties)
            _this->vulkan_config.vkEnumerateInstanceExtensionProperties,
        &extensionCount);
    if(!extensions)
        goto fail;
    for(i = 0; i < extensionCount; i++)
    {
        if(SDL_strcmp(VK_KHR_SURFACE_EXTENSION_NAME, extensions[i].extensionName) == 0)
            hasSurfaceExtension = SDL_TRUE;
        else if(SDL_strcmp(VK_KHR_WIN32_SURFACE_EXTENSION_NAME, extensions[i].extensionName) == 0)
            hasWin32SurfaceExtension = SDL_TRUE;
    }
    SDL_free(extensions);
    if(!hasSurfaceExtension)
    {
        SDL_SetError("Installed Vulkan doesn't implement the "
                     VK_KHR_SURFACE_EXTENSION_NAME " extension");
        goto fail;
    }
    else if(!hasWin32SurfaceExtension)
    {
        SDL_SetError("Installed Vulkan doesn't implement the "
                     VK_KHR_WIN32_SURFACE_EXTENSION_NAME "extension");
        goto fail;
    }
    return 0;

fail:
    SDL_UnloadObject(_this->vulkan_config.loader_handle);
    _this->vulkan_config.loader_handle = NULL;
    return -1;
}

void WIN_Vulkan_UnloadLibrary(_THIS)
{
    if(_this->vulkan_config.loader_handle)
    {
        SDL_UnloadObject(_this->vulkan_config.loader_handle);
        _this->vulkan_config.loader_handle = NULL;
    }
}

SDL_bool WIN_Vulkan_GetInstanceExtensions(_THIS,
                                          SDL_Window *window,
                                          unsigned *count,
                                          const char **names)
{
    static const char *const extensionsForWin32[] = {
        VK_KHR_SURFACE_EXTENSION_NAME, VK_KHR_WIN32_SURFACE_EXTENSION_NAME
    };
    if(!_this->vulkan_config.loader_handle)
    {
        SDL_SetError("Vulkan is not loaded");
        return SDL_FALSE;
    }
    return SDL_Vulkan_GetInstanceExtensions_Helper(
            count, names, SDL_arraysize(extensionsForWin32),
            extensionsForWin32);
}

SDL_bool WIN_Vulkan_CreateSurface(_THIS,
                                  SDL_Window *window,
                                  VkInstance instance,
                                  VkSurfaceKHR *surface)
{
    SDL_WindowData *windowData = (SDL_WindowData *)window->driverdata;
    PFN_vkGetInstanceProcAddr vkGetInstanceProcAddr =
        (PFN_vkGetInstanceProcAddr)_this->vulkan_config.vkGetInstanceProcAddr;
    PFN_vkCreateWin32SurfaceKHR vkCreateWin32SurfaceKHR =
        (PFN_vkCreateWin32SurfaceKHR)vkGetInstanceProcAddr(
                                            (VkInstance)instance,
                                            "vkCreateWin32SurfaceKHR");
    VkWin32SurfaceCreateInfoKHR createInfo;
    VkResult result;

    if(!_this->vulkan_config.loader_handle)
    {
        SDL_SetError("Vulkan is not loaded");
        return SDL_FALSE;
    }

    if(!vkCreateWin32SurfaceKHR)
    {
        SDL_SetError(VK_KHR_WIN32_SURFACE_EXTENSION_NAME
                     " extension is not enabled in the Vulkan instance.");
        return SDL_FALSE;
    }
    createInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
    createInfo.pNext = NULL;
    createInfo.flags = 0;
    createInfo.hinstance = windowData->hinstance;
    createInfo.hwnd = windowData->hwnd;
    result = vkCreateWin32SurfaceKHR(instance, &createInfo,
                                       NULL, surface);
    if(result != VK_SUCCESS)
    {
        SDL_SetError("vkCreateWin32SurfaceKHR failed: %s",
                     SDL_Vulkan_GetResultString(result));
        return SDL_FALSE;
    }
    return SDL_TRUE;
}

#endif

/* vi: set ts=4 sw=4 expandtab: */
