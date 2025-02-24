/*!*********************************************************************************************************************
\File         vk_getProcAddrs.h
\Title        Vulkan GetProcAddrs Header
\Author       PowerVR by Imagination, Developer Technology Team.
\Copyright    Copyright(c) Imagination Technologies Limited.
\brief        Header file for setting up the function pointers for the Vulkan API functions.
***********************************************************************************************************************/
#pragma once
#define PVR_MAX_SWAPCHAIN_IMAGES 3
#define VK_NO_PROTOTYPES

#ifdef __linux__
#ifdef __ANDROID__
#define _ANDROID 1
#else
#define _LINUX 1
#endif
#endif // __linux__
#ifdef _LINUX
#ifdef BUILD_XLIB
#include "X11/Xutil.h"
#define VK_USE_PLATFORM_XLIB_KHR
#endif
#ifdef BUILD_XCB
#define VK_USE_PLATFORM_XCB_KHR
#endif
#ifdef BUILD_WAYLAND
#define VK_USE_PLATFORM_WAYLAND_KHR
#endif
#include <unistd.h>
#include <dlfcn.h>
#include <cstring>
#include <memory>
#define LOGI(...) ((void)printf(__VA_ARGS__))
#define LOGW(...) ((void)fprintf(stderr, __VA_ARGS__))
#define LOGE(...) ((void)fprintf(stderr, __VA_ARGS__))

typedef void* LIBTYPE;

#endif

#ifdef _ANDROID
#define VK_USE_PLATFORM_ANDROID_KHR
#include <android/log.h>
#include <android_native_app_glue.h>
#include <unistd.h>
#include <dlfcn.h>
#include <dlfcn.h>
#include <unistd.h>
#define LOGI(...) ((void)__android_log_print(ANDROID_LOG_INFO, "com.imgtec.vk", __VA_ARGS__))
#define LOGW(...) ((void)__android_log_print(ANDROID_LOG_WARN, "com.imgtec.vk", __VA_ARGS__))
#define LOGE(...) ((void)__android_log_print(ANDROID_LOG_WARN, "com.imgtec.vk", __VA_ARGS__))
typedef void* LIBTYPE;
#endif

#if _WIN32
#define VK_USE_PLATFORM_WIN32_KHR
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <Windows.h>
#include <tchar.h>
typedef HMODULE LIBTYPE;
#endif

#if defined(__APPLE__)
#define _APPLE 1
#define VK_USE_PLATFORM_MACOS_MVK
#include <unistd.h>
#include <dlfcn.h>
#include <cstring>
#include <memory>
#include "CoreFoundation/CoreFoundation.h"
static const char* g_pszEnvVar = "PVRTRACE_LIB_PATH";
#define LOGI(...) ((void)printf(__VA_ARGS__))
#define LOGW(...) ((void)fprintf(stderr, __VA_ARGS__))
#define LOGE(...) ((void)fprintf(stderr, __VA_ARGS__))
typedef void* LIBTYPE;
#endif

#if !defined(VK_USE_PLATFORM_WIN32_KHR) && !defined(VK_USE_PLATFORM_ANDROID_KHR) && !defined(VK_USE_PLATFORM_XLIB_KHR) && !defined(VK_USE_PLATFORM_XCB_KHR) && \
	!defined(VK_USE_PLATFORM_WAYLAND_KHR) && !defined(VK_USE_PLATFORM_MACOS_MVK)
#define USE_PLATFORM_NULLWS
#endif

#include "vulkan/vulkan.h"
#include <algorithm>
#include <cstdlib>
#include <cstdio>
#include <string>
#include <cmath>
#include <vector>
#include <cassert>
#include <cstdarg>
#undef CreateSemaphore
#undef CreateEvent

#define PVR_VULKAN_FUNCTION_POINTER_DECLARATION(function_name) static PFN_vk##function_name function_name;

/*!*********************************************************************************************************************
\brief   This class's static members are function pointers to the Vulkan functions. The Shell kicks off their initialisation on
before context creation. Use the class name like a namespace: vk::CreateInstance.
***********************************************************************************************************************/
/*!*********************************************************************************************************************
\brief   This class's static members are function pointers to the Vulkan functions. The Shell kicks off their initialisation on
before context creation. Use the class name like a namespace: vk::CreateInstance.
***********************************************************************************************************************/
class vk
{
public:
	static bool initVulkan(); //!< Automatically called just before context initialisation.
	static bool initVulkanInstance(VkInstance instance); //!< Automatically called just before context initialisation.
	static bool initVulkanDevice(VkDevice device); //!< Automatically called just before context initialisation.

#ifdef VK_USE_PLATFORM_WIN32_KHR
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(CreateWin32SurfaceKHR)
#endif
#ifdef VK_USE_PLATFORM_ANDROID_KHR
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(CreateAndroidSurfaceKHR)
#endif
#ifdef VK_USE_PLATFORM_XLIB_KHR
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(CreateXlibSurfaceKHR)
#endif
#ifdef VK_USE_PLATFORM_XCB_KHR
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(CreateXcbSurfaceKHR)
#endif
#ifdef VK_USE_PLATFORM_WAYLAND_KHR
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(CreateWaylandSurfaceKHR)
#endif
#ifdef VK_USE_PLATFORM_MACOS_MVK
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(CreateMacOSSurfaceMVK)
#endif

#ifdef USE_PLATFORM_NULLWS
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(GetPhysicalDeviceDisplayPropertiesKHR)
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(GetDisplayModePropertiesKHR)
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(CreateDisplayPlaneSurfaceKHR)
#endif

	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(GetPhysicalDeviceSurfaceSupportKHR)
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(GetPhysicalDeviceSurfacePresentModesKHR)
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(GetPhysicalDeviceSurfaceCapabilitiesKHR)
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(GetPhysicalDeviceSurfaceFormatsKHR)
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(CreateSwapchainKHR)
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(GetSwapchainImagesKHR)
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(AcquireNextImageKHR)
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(QueuePresentKHR)
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(DestroySwapchainKHR)
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(CreateInstance)
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(EnumeratePhysicalDevices)
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(DestroyInstance)
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(DestroySurfaceKHR)

	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(GetInstanceProcAddr)
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(GetDeviceProcAddr)

	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(GetPhysicalDeviceFeatures)
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(GetPhysicalDeviceFormatProperties)
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(GetPhysicalDeviceImageFormatProperties)
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(GetPhysicalDeviceProperties)
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(GetPhysicalDeviceQueueFamilyProperties)
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(GetPhysicalDeviceMemoryProperties)
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(CreateDevice)
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(DestroyDevice)
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(EnumerateInstanceExtensionProperties)
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(EnumerateDeviceExtensionProperties)
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(EnumerateInstanceLayerProperties)
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(EnumerateDeviceLayerProperties)
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(GetDeviceQueue)
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(QueueSubmit)
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(QueueWaitIdle)
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(DeviceWaitIdle)
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(AllocateMemory)
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(FreeMemory)
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(MapMemory)
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(UnmapMemory)
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(FlushMappedMemoryRanges)
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(InvalidateMappedMemoryRanges)
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(GetDeviceMemoryCommitment)
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(BindBufferMemory)
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(BindImageMemory)
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(GetBufferMemoryRequirements)
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(GetImageMemoryRequirements)
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(GetImageSparseMemoryRequirements)
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(GetPhysicalDeviceSparseImageFormatProperties)
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(QueueBindSparse)
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(CreateFence)
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(DestroyFence)
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(ResetFences)
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(GetFenceStatus)
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(WaitForFences)
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(CreateSemaphore)
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(DestroySemaphore)
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(CreateEvent)
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(DestroyEvent)
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(GetEventStatus)
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(SetEvent)
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(ResetEvent)
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(CreateQueryPool)
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(DestroyQueryPool)
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(GetQueryPoolResults)
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(CreateBuffer)
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(DestroyBuffer)
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(CreateBufferView)
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(DestroyBufferView)
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(CreateImage)
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(DestroyImage)
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(GetImageSubresourceLayout)
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(CreateImageView)
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(DestroyImageView)
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(CreateShaderModule)
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(DestroyShaderModule)
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(CreatePipelineCache)
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(DestroyPipelineCache)
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(GetPipelineCacheData)
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(MergePipelineCaches)
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(CreateGraphicsPipelines)
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(CreateComputePipelines)
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(DestroyPipeline)
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(CreatePipelineLayout)
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(DestroyPipelineLayout)
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(CreateSampler)
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(DestroySampler)
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(CreateDescriptorSetLayout)
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(DestroyDescriptorSetLayout)
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(CreateDescriptorPool)
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(DestroyDescriptorPool)
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(ResetDescriptorPool)
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(AllocateDescriptorSets)
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(FreeDescriptorSets)
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(UpdateDescriptorSets)
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(CreateFramebuffer)
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(DestroyFramebuffer)
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(CreateRenderPass)
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(DestroyRenderPass)
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(GetRenderAreaGranularity)
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(CreateCommandPool)
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(DestroyCommandPool)
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(ResetCommandPool)
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(AllocateCommandBuffers)
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(FreeCommandBuffers)
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(BeginCommandBuffer)
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(EndCommandBuffer)
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(ResetCommandBuffer)
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(CmdBindPipeline)
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(CmdSetViewport)
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(CmdSetScissor)
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(CmdSetLineWidth)
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(CmdSetDepthBias)
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(CmdSetBlendConstants)
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(CmdSetDepthBounds)
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(CmdSetStencilCompareMask)
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(CmdSetStencilWriteMask)
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(CmdSetStencilReference)
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(CmdBindDescriptorSets)
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(CmdBindIndexBuffer)
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(CmdBindVertexBuffers)
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(CmdDraw)
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(CmdDrawIndexed)
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(CmdDrawIndirect)
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(CmdDrawIndexedIndirect)
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(CmdDispatch)
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(CmdDispatchIndirect)
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(CmdCopyBuffer)
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(CmdCopyImage)
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(CmdBlitImage)
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(CmdCopyBufferToImage)
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(CmdCopyImageToBuffer)
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(CmdUpdateBuffer)
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(CmdFillBuffer)
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(CmdClearColorImage)
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(CmdClearDepthStencilImage)
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(CmdClearAttachments)
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(CmdResolveImage)
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(CmdSetEvent)
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(CmdResetEvent)
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(CmdWaitEvents)
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(CmdPipelineBarrier)
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(CmdBeginQuery)
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(CmdEndQuery)
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(CmdResetQueryPool)
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(CmdWriteTimestamp)
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(CmdCopyQueryPoolResults)
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(CmdPushConstants)
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(CmdBeginRenderPass)
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(CmdNextSubpass)
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(CmdEndRenderPass)
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(CmdExecuteCommands)

	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(CreateDebugReportCallbackEXT)
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(DebugReportMessageEXT)
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(DestroyDebugReportCallbackEXT)
};

static const char* procAddressMessageTypes[] = {
	"INFORMATION: ",
	"ERROR: ",
};

inline void logOutput(bool error, const char* const formatString, va_list argumentList)
{
#if defined(__ANDROID__)
	// Note: There may be issues displaying 64bits values with this function
	// Note: This function will truncate long messages
	__android_log_vprint(error ? ANDROID_LOG_ERROR : ANDROID_LOG_INFO, "com.powervr.Example", formatString, argumentList);
#elif defined(__QNXNTO__)
	vslogf(1, error ? _SLOG_INFO : _SLOG_ERROR, formatString, argumentList);
#else // Not android Not QNX
	FILE* logfile = fopen("log.txt", "w");
	static char buffer[4096];
	va_list tempList;
	memset(buffer, 0, sizeof(buffer));
#if (defined _MSC_VER) // Pre VS2013
	tempList = argumentList;
#else
	va_copy(tempList, argumentList);
#endif
	if (logfile != NULL)
	{
		vsnprintf(buffer, 4095, formatString, argumentList);
		fprintf(logfile, "%s%s\n", procAddressMessageTypes[static_cast<uint32_t>(error)], buffer);
		fclose(logfile);
	}
#if defined(_WIN32)

	if (IsDebuggerPresent())
	{
		OutputDebugString(procAddressMessageTypes[static_cast<uint32_t>(error)]);
		OutputDebugString(buffer);
		OutputDebugString("\n");
	}
	else
#endif
	{
		printf("%s%s\n", procAddressMessageTypes[error], buffer);
	}
#endif
}

#if _ANDROID
inline LIBTYPE openLibrary(const char* pszPath)
{
	LIBTYPE lt = dlopen(pszPath, RTLD_LAZY | RTLD_GLOBAL);
	if (!lt)
	{
		const char* err = dlerror();
		if (err)
		{
			LOGE("dlopen failed with error ");
			LOGE("%s", err);
		}
	}
	return lt;
}
#endif

inline void Log(bool error, const char* const formatString, ...)
{
	va_list argumentList;
	va_start(argumentList, formatString);
	logOutput(error, formatString, argumentList);
	va_end(argumentList);
}

struct NativeLibrary
{
public:
	/*!*********************************************************************************************************************
	\brief   Load a library with the specified filename.z
	\param   libraryPath The path to find the library (name or Path+name).
	***********************************************************************************************************************/
	NativeLibrary(const std::string& LibPath)
	{
#if _WIN32
		_hostLib = LoadLibraryA(LibPath.c_str());

		if (!_hostLib) { Log(true, "Could not load host library '%s", LibPath.c_str()); }
		Log(false, "Host library '%s' loaded", LibPath.c_str());
#endif
#if _LINUX
		_hostLib = dlopen(LibPath.c_str(), RTLD_LAZY | RTLD_GLOBAL);

		if (!_hostLib)
		{
			LOGE("Could not load host library '%s'\n", LibPath.c_str());

			const char* err = dlerror();

			if (err) { LOGE("dlopen failed with error: %s => %s\n", err, LibPath.c_str()); }

			char pathMod[256];
			strcpy(pathMod, "./");
			strcat(pathMod, LibPath.c_str());

			_hostLib = dlopen(pathMod, RTLD_LAZY | RTLD_GLOBAL);

			if (!_hostLib)
			{
				const char* err = dlerror();

				if (err) { LOGE("dlopen failed with error: %s => %s\n", err, pathMod); }
			}
			else
			{
				LOGE("dlopen loaded (MOD PATH) %s\n", pathMod);
			}
		}
		LOGI("Host library '%s' loaded\n", LibPath.c_str());
#endif
#if _APPLE
		const char* pszPath = LibPath.c_str();
		CFBundleRef mainBundle = CFBundleGetMainBundle();
		CFURLRef resourceURL = CFBundleCopyPrivateFrameworksURL(mainBundle);
		char path[PATH_MAX];
		if (CFURLGetFileSystemRepresentation(resourceURL, TRUE, (UInt8*)path, PATH_MAX))
		{
			CFRelease(resourceURL);

			// --- Set a global environment variable to point to this path (for VFrame usage)
			const char* slash = strrchr(pszPath, '/');
			if (slash)
			{
				char szPath[FILENAME_MAX];
				memset(szPath, 0, sizeof(szPath));
				strncpy(szPath, pszPath, slash - pszPath);
				setenv(g_pszEnvVar, szPath, 1);
			}
			else
			{
				// Use the current bundle path
				std::string framework = std::string(path) + "/../Frameworks/";
				setenv(g_pszEnvVar, framework.c_str(), 1);
			}

			// --- Make a temp symlink
			char szTempFile[FILENAME_MAX];
			memset(szTempFile, 0, sizeof(szTempFile));

			char tmpdir[PATH_MAX];
			size_t n = confstr(_CS_DARWIN_USER_TEMP_DIR, tmpdir, sizeof(tmpdir));
			if ((n <= 0) || (n >= sizeof(tmpdir))) { strlcpy(tmpdir, getenv("TMPDIR"), sizeof(tmpdir)); }

			strcat(szTempFile, tmpdir);
			strcat(szTempFile, "tmp.XXXXXX");

			if (mkstemp(szTempFile))
			{
				if (symlink(pszPath, szTempFile) == 0)
				{
					_hostLib = dlopen(szTempFile, RTLD_LAZY | RTLD_GLOBAL);
					remove(szTempFile);
				}
			}

			// --- Can't find the lib? Check the application framework folder instead.
			if (!_hostLib)
			{
				std::string framework = std::string(path) + std::string("/") + pszPath;
				_hostLib = dlopen(framework.c_str(), RTLD_LAZY | RTLD_GLOBAL);

				if (!_hostLib)
				{
					const char* err = dlerror();
					if (err)
					{
						// NSLog(@"dlopen failed with error: %s => %@", err, framework);
					}
				}
			}
		}
#endif
#if _ANDROID
		size_t start = 0;
		std::string tmp;

		while (!_hostLib)
		{
			size_t end = LibPath.find_first_of(';', start);

			if (end == std::string::npos) { tmp = LibPath.substr(start, LibPath.length() - start); }
			else
			{
				tmp = LibPath.substr(start, end - start);
			}
			if (!tmp.empty())
			{
				_hostLib = openLibrary(tmp.c_str());

				if (!_hostLib)
				{
					// Remove the last character, in case a new line character sneaked in.
					tmp = tmp.substr(0, tmp.size() - 1);
					_hostLib = openLibrary(tmp.c_str());
				}
			}
			if (end == std::string::npos) { break; }
			start = end + 1;
		}
		if (!_hostLib) { LOGE("Could not load host library '%s'", LibPath.c_str()); }
		LOGI("Host library '%s' loaded", LibPath.c_str());
#endif
	}
	~NativeLibrary() { CloseLib(); }

	/*!*********************************************************************************************************************
	\brief   Get a function pointer from the library.
	\param   functionName  The name of the function to retrieve the pointer to.
	\return  The function pointer as a void pointer. Null if failed. Cast to the proper type.
	***********************************************************************************************************************/
	void* getFunction(const char* functionName)
	{
		if (_hostLib)
		{
#if _WIN32
			void* pFn = reinterpret_cast<void*>(GetProcAddress(_hostLib, functionName));
			if (pFn == NULL) { Log(true, "Could not get function %s", functionName); }
#endif

#if _LINUX || _ANDROID || _APPLE
			void* pFn = dlsym(_hostLib, functionName);
			if (pFn == NULL) { LOGE("Could not get function %s\n", functionName); }
#endif
			return pFn;
		}
		return NULL;
	}

	/*!*********************************************************************************************************************
	\brief   Get a function pointer from the library.
	\tparam  PtrType_ The type of the function pointer
	\param   functionName  The name of the function to retrieve the pointer to
	\return  The function pointer. Null if failed.
	***********************************************************************************************************************/
	template<typename PtrType_>
	PtrType_ getFunction(const char* functionName)
	{
		return reinterpret_cast<PtrType_>(getFunction(functionName));
	}

	/*!*********************************************************************************************************************
	\brief   Release this library.
	***********************************************************************************************************************/
	void CloseLib()
	{
		if (_hostLib)
		{
#if _WIN32
			FreeLibrary(_hostLib);
#endif
#if _LINUX || _ANDROID || _APPLE
			dlclose(_hostLib);
#endif
			_hostLib = 0;
		}
	}

protected:
	LIBTYPE _hostLib;
};

#define PVR_STR(x) #x
#define PVR_VULKAN_GET_INSTANCE_POINTER(instance, function_name) function_name = (PFN_vk##function_name)GetInstanceProcAddr(instance, "vk" PVR_STR(function_name));
#define PVR_VULKAN_GET_DEVICE_POINTER(device, function_name) function_name = (PFN_vk##function_name)GetDeviceProcAddr(device, "vk" PVR_STR(function_name));
