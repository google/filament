// Copyright 2021 The SwiftShader Authors. All Rights Reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//    http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "VulkanTester.hpp"
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>

namespace fs = std::filesystem;

// By default, load SwiftShader via loader
#ifndef LOAD_NATIVE_DRIVER
#	define LOAD_NATIVE_DRIVER 0
#endif

#ifndef LOAD_SWIFTSHADER_DIRECTLY
#	define LOAD_SWIFTSHADER_DIRECTLY 0
#endif

#if LOAD_NATIVE_DRIVER && LOAD_SWIFTSHADER_DIRECTLY
#	error Enable only one of LOAD_NATIVE_DRIVER and LOAD_SWIFTSHADER_DIRECTLY
#endif

// By default, enable validation layers in DEBUG builds
#if !defined(ENABLE_VALIDATION_LAYERS) && !defined(NDEBUG)
#	define ENABLE_VALIDATION_LAYERS 1
#endif

#if defined(_WIN32)
#	define OS_WINDOWS 1
#elif defined(__APPLE__)
#	include "dlfcn.h"
#	define OS_MAC 1
#elif defined(__ANDROID__)
#	include "dlfcn.h"
#	define OS_ANDROID 1
#elif defined(__linux__)
#	include "dlfcn.h"
#	define OS_LINUX 1
#elif defined(__Fuchsia__)
#	include <zircon/dlfcn.h>
#	define OS_FUCHSIA 1
#else
#	error Unimplemented platform
#endif

// TODO: move to its own header/cpp
// Wraps a single environment variable, allowing it to be set
// and automatically restored on destruction.
class ScopedSetEnvVar
{
public:
	ScopedSetEnvVar(std::string name)
	    : name(name)
	{
		assert(!name.empty());
	}

	ScopedSetEnvVar(std::string name, std::string value)
	    : name(name)
	{
		set(value);
	}

	~ScopedSetEnvVar()
	{
		restore();
	}

	void set(std::string value)
	{
		restore();
		if(auto ov = getEnv(name.data()))
		{
			oldValue = ov;
		}
		putEnv((name + std::string("=") + value).c_str());
	}

	void restore()
	{
		if(!oldValue.empty())
		{
			putEnv((name + std::string("=") + oldValue).c_str());
			oldValue.clear();
		}
	}

private:
	void putEnv(const char *env)
	{
		// POSIX putenv needs 'env' to live beyond the call
		envCopy = env;
#if OS_WINDOWS
		[[maybe_unused]] auto r = ::_putenv(envCopy.c_str());
		assert(r == 0);
#else
		[[maybe_unused]] auto r = ::putenv(const_cast<char *>(envCopy.c_str()));
		assert(r == 0);
#endif
	}

	const char *getEnv(const char *name)
	{
		return ::getenv(name);
	}

	std::string name;
	std::string oldValue;
	std::string envCopy;
};

// Generates a temporary icd.json file that sets library_path at the input driverPath,
// and sets VK_ICD_FILENAMES environment variable to this file, restoring the env var
// and deleting the temp file on destruction.
class ScopedSetIcdFilenames
{
public:
	ScopedSetIcdFilenames() = default;
	ScopedSetIcdFilenames(const char *driverPath)
	{
		std::ofstream fout(icdFileName);
		assert(fout && "Failed to create generated icd file");
		fout << R"raw({ "file_format_version": "1.0.0", "ICD": { "library_path": ")raw" << driverPath << R"raw(", "api_version": "1.0.5" } } )raw";
		fout.close();

		setEnvVar.set(icdFileName);
	}

	~ScopedSetIcdFilenames()
	{
		if(fs::exists("vk_swiftshader_generated_icd.json"))
		{
			fs::remove("vk_swiftshader_generated_icd.json");
		}
	}

private:
	static constexpr const char *icdFileName = "vk_swiftshader_generated_icd.json";
	ScopedSetEnvVar setEnvVar{ "VK_ICD_FILENAMES" };
};

namespace {

std::vector<const char *> getDriverPaths()
{
#if OS_WINDOWS
#	if !defined(STANDALONE)
	// The DLL is delay loaded (see BUILD.gn), so we can load
	// the correct ones from Chrome's swiftshader subdirectory.
	// HMODULE libvulkan = LoadLibraryA("swiftshader\\libvulkan.dll");
	// EXPECT_NE((HMODULE)NULL, libvulkan);
	// return true;
#		error TODO: !STANDALONE
#	elif defined(NDEBUG)
#		if defined(_WIN64)
	return { "./build/Release_x64/vk_swiftshader.dll",
		     "./build/Release/vk_swiftshader.dll",
		     "./build/RelWithDebInfo/vk_swiftshader.dll",
		     "./build/vk_swiftshader.dll",
		     "./vk_swiftshader.dll" };
#		else
	return { "./build/Release_Win32/vk_swiftshader.dll",
		     "./build/Release/vk_swiftshader.dll",
		     "./build/RelWithDebInfo/vk_swiftshader.dll",
		     "./build/vk_swiftshader.dll",
		     "./vk_swiftshader.dll" };
#		endif
#	else
#		if defined(_WIN64)
	return { "./build/Debug_x64/vk_swiftshader.dll",
		     "./build/Debug/vk_swiftshader.dll",
		     "./build/vk_swiftshader.dll",
		     "./vk_swiftshader.dll" };
#		else
	return { "./build/Debug_Win32/vk_swiftshader.dll",
		     "./build/Debug/vk_swiftshader.dll",
		     "./build/vk_swiftshader.dll",
		     "./vk_swiftshader.dll" };
#		endif
#	endif
#elif OS_MAC
	return { "./build/Darwin/libvk_swiftshader.dylib",
		     "swiftshader/libvk_swiftshader.dylib",
		     "libvk_swiftshader.dylib" };
#elif OS_LINUX
	return { "./build/Linux/libvk_swiftshader.so",
		     "swiftshader/libvk_swiftshader.so",
		     "./libvk_swiftshader.so",
		     "libvk_swiftshader.so" };
#elif OS_ANDROID || OS_FUCHSIA
	return
	{
		"libvk_swiftshader.so"
	}
#else
#	error Unimplemented platform
	return {};
#endif
}

bool fileExists(const char *path)
{
	std::ifstream f(path);
	return f.good();
}

std::string findDriverPath()
{
	for(auto &path : getDriverPaths())
	{
		if(fileExists(path))
			return path;
	}

#if(OS_LINUX || OS_ANDROID || OS_FUCHSIA)
	// On Linux-based OSes, the lib path may be resolved by dlopen
	for(auto &path : getDriverPaths())
	{
		auto lib = dlopen(path, RTLD_LAZY | RTLD_LOCAL);
		if(lib)
		{
			char libPath[2048] = { '\0' };
			dlinfo(lib, RTLD_DI_ORIGIN, libPath);
			dlclose(lib);
			return std::string{ libPath } + "/" + path;
		}
	}
#endif

	return {};
}

}  // namespace

VulkanTester::VulkanTester() = default;

VulkanTester::~VulkanTester()
{
	device.waitIdle();
	device.destroy(nullptr);
	if(debugReport) instance.destroy(debugReport);
	instance.destroy(nullptr);
}

void VulkanTester::initialize()
{
	dl = loadDriver();
	assert(dl && dl->success());

	PFN_vkGetInstanceProcAddr vkGetInstanceProcAddr = dl->getProcAddress<PFN_vkGetInstanceProcAddr>("vkGetInstanceProcAddr");
	VULKAN_HPP_DEFAULT_DISPATCHER.init(vkGetInstanceProcAddr);

	vk::InstanceCreateInfo instanceCreateInfo;
	std::vector<const char *> extensionNames
	{
		VK_KHR_SURFACE_EXTENSION_NAME,
#if USE_HEADLESS_SURFACE
		    VK_EXT_HEADLESS_SURFACE_EXTENSION_NAME,
#endif
#if defined(VK_USE_PLATFORM_WIN32_KHR)
		    VK_KHR_WIN32_SURFACE_EXTENSION_NAME,
#endif
	};
#if ENABLE_VALIDATION_LAYERS
	extensionNames.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
#endif

	std::vector<const char *> layerNames;
#if ENABLE_VALIDATION_LAYERS
	auto addLayerIfAvailable = [](std::vector<const char *> &layers, const char *layer) {
		static auto layerProperties = vk::enumerateInstanceLayerProperties();
		if(std::find_if(layerProperties.begin(), layerProperties.end(), [layer](auto &lp) {
			   return strcmp(layer, lp.layerName) == 0;
		   }) != layerProperties.end())
		{
			// std::cout << "Enabled layer: " << layer << std::endl;
			layers.push_back(layer);
		}
	};

	addLayerIfAvailable(layerNames, "VK_LAYER_KHRONOS_validation");
	addLayerIfAvailable(layerNames, "VK_LAYER_LUNARG_standard_validation");
#endif

	instanceCreateInfo.ppEnabledExtensionNames = extensionNames.data();
	instanceCreateInfo.enabledExtensionCount = static_cast<uint32_t>(extensionNames.size());
	instanceCreateInfo.ppEnabledLayerNames = layerNames.data();
	instanceCreateInfo.enabledLayerCount = static_cast<uint32_t>(layerNames.size());

	instance = vk::createInstance(instanceCreateInfo, nullptr);
	VULKAN_HPP_DEFAULT_DISPATCHER.init(instance);

#if ENABLE_VALIDATION_LAYERS
	if(VULKAN_HPP_DEFAULT_DISPATCHER.vkCreateDebugUtilsMessengerEXT)
	{
		vk::DebugUtilsMessengerCreateInfoEXT debugInfo;
		debugInfo.messageSeverity =
		    // vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose |
		    vk::DebugUtilsMessageSeverityFlagBitsEXT::eError |
		    vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning;

		debugInfo.messageType = vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral |
		                        vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation |
		                        vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance;

		PFN_vkDebugUtilsMessengerCallbackEXT debugInfoCallback =
		    [](
		        VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
		        VkDebugUtilsMessageTypeFlagsEXT messageTypes,
		        const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData,
		        void *pUserData) -> VkBool32 {
			// assert(false);
			std::cerr << "[DebugInfoCallback] " << pCallbackData->pMessage << std::endl;
			return VK_FALSE;
		};

		debugInfo.pfnUserCallback = debugInfoCallback;
		debugReport = instance.createDebugUtilsMessengerEXT(debugInfo);
	}
#endif

	std::vector<vk::PhysicalDevice> physicalDevices = instance.enumeratePhysicalDevices();
	assert(!physicalDevices.empty());
	physicalDevice = physicalDevices[0];

	const float defaultQueuePriority = 0.0f;
	vk::DeviceQueueCreateInfo queueCreateInfo;
	queueCreateInfo.queueFamilyIndex = queueFamilyIndex;
	queueCreateInfo.queueCount = 1;
	queueCreateInfo.pQueuePriorities = &defaultQueuePriority;

	std::vector<const char *> deviceExtensions = {
		VK_KHR_SWAPCHAIN_EXTENSION_NAME,
	};

	vk::DeviceCreateInfo deviceCreateInfo;
	deviceCreateInfo.queueCreateInfoCount = 1;
	deviceCreateInfo.pQueueCreateInfos = &queueCreateInfo;
	deviceCreateInfo.ppEnabledExtensionNames = deviceExtensions.data();
	deviceCreateInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());

	device = physicalDevice.createDevice(deviceCreateInfo, nullptr);

	queue = device.getQueue(queueFamilyIndex, 0);
}

std::unique_ptr<vk::DynamicLoader> VulkanTester::loadDriver()
{
	if(LOAD_NATIVE_DRIVER)
	{
		return std::make_unique<vk::DynamicLoader>();
	}

	auto driverPath = findDriverPath();
	assert(!driverPath.empty());

	if(LOAD_SWIFTSHADER_DIRECTLY)
	{
		return std::make_unique<vk::DynamicLoader>(driverPath);
	}

	// Load SwiftShader via loader

	// Set VK_ICD_FILENAMES env var so it gets picked up by the loading of the ICD driver
	setIcdFilenames = std::make_unique<ScopedSetIcdFilenames>(driverPath.c_str());

	std::unique_ptr<vk::DynamicLoader> dl;
#ifndef VULKAN_HPP_NO_EXCEPTIONS
	try
	{
		dl = std::make_unique<vk::DynamicLoader>();
	}
	catch(std::exception &ex)
	{
		std::cerr << "vk::DynamicLoader exception: " << ex.what() << std::endl;
		std::cerr << "Falling back to loading SwiftShader directly (i.e. no validation layers)" << std::endl;
		dl = std::make_unique<vk::DynamicLoader>(driverPath);
	}
#else
	dl = std::make_unique<vk::DynamicLoader>();
#endif

	return dl;
}
