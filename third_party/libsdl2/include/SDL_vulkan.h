/*
  Simple DirectMedia Layer
  Copyright (C) 2017, Mark Callow

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

/**
 *  \file SDL_vulkan.h
 *
 *  Header file for functions to creating Vulkan surfaces on SDL windows.
 */

#ifndef SDL_vulkan_h_
#define SDL_vulkan_h_

#include "SDL_video.h"

#include "begin_code.h"
/* Set up for C function definitions, even when using C++ */
#ifdef __cplusplus
extern "C" {
#endif

/* Avoid including vulkan.h, don't define VkInstance if it's already included */
#ifdef VULKAN_H_
#define NO_SDL_VULKAN_TYPEDEFS
#endif
#ifndef NO_SDL_VULKAN_TYPEDEFS
#define VK_DEFINE_HANDLE(object) typedef struct object##_T* object;

#if defined(__LP64__) || defined(_WIN64) || defined(__x86_64__) || defined(_M_X64) || defined(__ia64) || defined (_M_IA64) || defined(__aarch64__) || defined(__powerpc64__)
#define VK_DEFINE_NON_DISPATCHABLE_HANDLE(object) typedef struct object##_T *object;
#else
#define VK_DEFINE_NON_DISPATCHABLE_HANDLE(object) typedef uint64_t object;
#endif

VK_DEFINE_HANDLE(VkInstance)
VK_DEFINE_NON_DISPATCHABLE_HANDLE(VkSurfaceKHR)

#endif /* !NO_SDL_VULKAN_TYPEDEFS */

typedef VkInstance SDL_vulkanInstance;
typedef VkSurfaceKHR SDL_vulkanSurface; /* for compatibility with Tizen */

/**
 *  \name Vulkan support functions
 *
 *  \note SDL_Vulkan_GetInstanceExtensions & SDL_Vulkan_CreateSurface API
 *        is compatable with Tizen's implementation of Vulkan in SDL.
 */
/* @{ */

/**
 *  \brief Dynamically load a Vulkan loader library.
 *
 *  \param [in] path The platform dependent Vulkan loader library name, or
 *              \c NULL.
 *
 *  \return \c 0 on success, or \c -1 if the library couldn't be loaded.
 *
 *  If \a path is NULL SDL will use the value of the environment variable
 *  \c SDL_VULKAN_LIBRARY, if set, otherwise it loads the default Vulkan
 *  loader library.
 *
 *  This should be called after initializing the video driver, but before
 *  creating any Vulkan windows. If no Vulkan loader library is loaded, the
 *  default library will be loaded upon creation of the first Vulkan window.
 *
 *  \note It is fairly common for Vulkan applications to link with \a libvulkan
 *        instead of explicitly loading it at run time. This will work with
 *        SDL provided the application links to a dynamic library and both it
 *        and SDL use the same search path.
 *
 *  \note If you specify a non-NULL \c path, an application should retrieve all
 *        of the Vulkan functions it uses from the dynamic library using
 *        \c SDL_Vulkan_GetVkGetInstanceProcAddr() unless you can guarantee
 *        \c path points to the same vulkan loader library the application
 *        linked to.
 *
 *  \note On Apple devices, if \a path is NULL, SDL will attempt to find
 *        the vkGetInstanceProcAddr address within all the mach-o images of
 *        the current process. This is because it is fairly common for Vulkan
 *        applications to link with libvulkan (and historically MoltenVK was
 *        provided as a static library). If it is not found then, on macOS, SDL
 *        will attempt to load \c vulkan.framework/vulkan, \c libvulkan.1.dylib,
 *        \c MoltenVK.framework/MoltenVK and \c libMoltenVK.dylib in that order.
 *        On iOS SDL will attempt to load \c libMoltenVK.dylib. Applications
 *        using a dynamic framework or .dylib must ensure it is included in its
 *        application bundle.
 *
 *  \note On non-Apple devices, application linking with a static libvulkan is
 *        not supported. Either do not link to the Vulkan loader or link to a
 *        dynamic library version.
 *
 *  \note This function will fail if there are no working Vulkan drivers
 *        installed.
 *
 *  \sa SDL_Vulkan_GetVkGetInstanceProcAddr()
 *  \sa SDL_Vulkan_UnloadLibrary()
 */
extern DECLSPEC int SDLCALL SDL_Vulkan_LoadLibrary(const char *path);

/**
 *  \brief Get the address of the \c vkGetInstanceProcAddr function.
 *
 *  \note This should be called after either calling SDL_Vulkan_LoadLibrary
 *        or creating an SDL_Window with the SDL_WINDOW_VULKAN flag.
 */
extern DECLSPEC void *SDLCALL SDL_Vulkan_GetVkGetInstanceProcAddr(void);

/**
 *  \brief Unload the Vulkan loader library previously loaded by
 *         \c SDL_Vulkan_LoadLibrary().
 *
 *  \sa SDL_Vulkan_LoadLibrary()
 */
extern DECLSPEC void SDLCALL SDL_Vulkan_UnloadLibrary(void);

/**
 *  \brief Get the names of the Vulkan instance extensions needed to create
 *         a surface with \c SDL_Vulkan_CreateSurface().
 *
 *  \param [in]     window Window for which the required Vulkan instance
 *                  extensions should be retrieved
 *  \param [in,out] count pointer to an \c unsigned related to the number of
 *                  required Vulkan instance extensions
 *  \param [out]    names \c NULL or a pointer to an array to be filled with the
 *                  required Vulkan instance extensions
 *
 *  \return \c SDL_TRUE on success, \c SDL_FALSE on error.
 *
 *  If \a pNames is \c NULL, then the number of required Vulkan instance
 *  extensions is returned in pCount. Otherwise, \a pCount must point to a
 *  variable set to the number of elements in the \a pNames array, and on
 *  return the variable is overwritten with the number of names actually
 *  written to \a pNames. If \a pCount is less than the number of required
 *  extensions, at most \a pCount structures will be written. If \a pCount
 *  is smaller than the number of required extensions, \c SDL_FALSE will be
 *  returned instead of \c SDL_TRUE, to indicate that not all the required
 *  extensions were returned.
 *
 *  \note The returned list of extensions will contain \c VK_KHR_surface
 *        and zero or more platform specific extensions
 *
 *  \note The extension names queried here must be enabled when calling
 *        VkCreateInstance, otherwise surface creation will fail.
 *
 *  \note \c window should have been created with the \c SDL_WINDOW_VULKAN flag.
 *
 *  \code
 *  unsigned int count;
 *  // get count of required extensions
 *  if(!SDL_Vulkan_GetInstanceExtensions(window, &count, NULL))
 *      handle_error();
 *
 *  static const char *const additionalExtensions[] =
 *  {
 *      VK_EXT_DEBUG_REPORT_EXTENSION_NAME, // example additional extension
 *  };
 *  size_t additionalExtensionsCount = sizeof(additionalExtensions) / sizeof(additionalExtensions[0]);
 *  size_t extensionCount = count + additionalExtensionsCount;
 *  const char **names = malloc(sizeof(const char *) * extensionCount);
 *  if(!names)
 *      handle_error();
 *
 *  // get names of required extensions
 *  if(!SDL_Vulkan_GetInstanceExtensions(window, &count, names))
 *      handle_error();
 *
 *  // copy additional extensions after required extensions
 *  for(size_t i = 0; i < additionalExtensionsCount; i++)
 *      names[i + count] = additionalExtensions[i];
 *
 *  VkInstanceCreateInfo instanceCreateInfo = {};
 *  instanceCreateInfo.enabledExtensionCount = extensionCount;
 *  instanceCreateInfo.ppEnabledExtensionNames = names;
 *  // fill in rest of instanceCreateInfo
 *
 *  VkInstance instance;
 *  // create the Vulkan instance
 *  VkResult result = vkCreateInstance(&instanceCreateInfo, NULL, &instance);
 *  free(names);
 *  \endcode
 *
 *  \sa SDL_Vulkan_CreateSurface()
 */
extern DECLSPEC SDL_bool SDLCALL SDL_Vulkan_GetInstanceExtensions(
														SDL_Window *window,
														unsigned int *pCount,
														const char **pNames);

/**
 *  \brief Create a Vulkan rendering surface for a window.
 *
 *  \param [in]  window   SDL_Window to which to attach the rendering surface.
 *  \param [in]  instance handle to the Vulkan instance to use.
 *  \param [out] surface  pointer to a VkSurfaceKHR handle to receive the
 *                        handle of the newly created surface.
 *
 *  \return \c SDL_TRUE on success, \c SDL_FALSE on error.
 *
 *  \code
 *  VkInstance instance;
 *  SDL_Window *window;
 *
 *  // create instance and window
 *
 *  // create the Vulkan surface
 *  VkSurfaceKHR surface;
 *  if(!SDL_Vulkan_CreateSurface(window, instance, &surface))
 *      handle_error();
 *  \endcode
 *
 *  \note \a window should have been created with the \c SDL_WINDOW_VULKAN flag.
 *
 *  \note \a instance should have been created with the extensions returned
 *        by \c SDL_Vulkan_CreateSurface() enabled.
 *
 *  \sa SDL_Vulkan_GetInstanceExtensions()
 */
extern DECLSPEC SDL_bool SDLCALL SDL_Vulkan_CreateSurface(
												SDL_Window *window,
												VkInstance instance,
												VkSurfaceKHR* surface);

/**
 *  \brief Get the size of a window's underlying drawable in pixels (for use
 *         with setting viewport, scissor & etc).
 *
 *  \param window   SDL_Window from which the drawable size should be queried
 *  \param w        Pointer to variable for storing the width in pixels,
 *                  may be NULL
 *  \param h        Pointer to variable for storing the height in pixels,
 *                  may be NULL
 *
 * This may differ from SDL_GetWindowSize() if we're rendering to a high-DPI
 * drawable, i.e. the window was created with SDL_WINDOW_ALLOW_HIGHDPI on a
 * platform with high-DPI support (Apple calls this "Retina"), and not disabled
 * by the \c SDL_HINT_VIDEO_HIGHDPI_DISABLED hint.
 *
 *  \note On macOS high-DPI support must be enabled for an application by
 *        setting NSHighResolutionCapable to true in its Info.plist.
 *
 *  \sa SDL_GetWindowSize()
 *  \sa SDL_CreateWindow()
 */
extern DECLSPEC void SDLCALL SDL_Vulkan_GetDrawableSize(SDL_Window * window,
                                                        int *w, int *h);

/* @} *//* Vulkan support functions */

/* Ends C function definitions when using C++ */
#ifdef __cplusplus
}
#endif
#include "close_code.h"

#endif /* SDL_vulkan_h_ */
