// Copyright 2019 The SwiftShader Authors. All Rights Reserved.
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

#include <vulkan/vulkan_core.h>

// Driver is used to load a Vulkan graphics driver and expose its functions.
// It is used by the unit tests to test the SwiftShader driver and optionally
// load a system vulkan driver to test against.
class Driver
{
public:
	Driver();
	~Driver();

	// loadSwiftShader attempts to load the SwiftShader vulkan driver.
	// returns true on success, false on failure.
	bool loadSwiftShader();

	// loadSystem attempts to load the system's vulkan driver.
	// returns true on success, false on failure.
	bool loadSystem();

	// load attempts to load the vulkan driver from the given path.
	// returns true on success, false on failure.
	bool load(const char *path);

	// unloads the currently loaded driver.
	// No-op if no driver is currently loaded.
	void unload();

	// isLoaded returns true if the driver is currently loaded.
	bool isLoaded() const;

	// resolve all the functions for the given VkInstance.
	bool resolve(VkInstance);

	VKAPI_ATTR PFN_vkVoidFunction(VKAPI_CALL *vk_icdGetInstanceProcAddr)(VkInstance instance, const char *pName);

	// Global vulkan function pointers.
#define VK_GLOBAL(N, R, ...) VKAPI_ATTR R(VKAPI_CALL *N)(__VA_ARGS__)
#include "VkGlobalFuncs.hpp"
#undef VK_GLOBAL

	// Per-instance vulkan function pointers.
#define VK_INSTANCE(N, R, ...) VKAPI_ATTR R(VKAPI_CALL *N)(__VA_ARGS__)
#include "VkInstanceFuncs.hpp"
#undef VK_INSTANCE

private:
	Driver(const Driver &) = delete;
	Driver(Driver &&) = delete;
	Driver &operator=(const Driver &) = delete;

	// lookup searches the loaded driver for a symbol with the given name,
	// returning the address of this symbol if found, otherwise nullptr.
	void *lookup(const char *name);

	// Helper function to lookup a symbol and cast it to the appropriate type.
	// Returns true if the symbol was found and assigned to ptr, otherwise
	// returns false.
	template<typename T>
	inline bool lookup(T *ptr, const char *name);

	void *dll;
};

template<typename T>
bool Driver::lookup(T *ptr, const char *name)
{
	void *sym = lookup(name);
	if(sym == nullptr)
	{
		return false;
	}
	*ptr = reinterpret_cast<T>(sym);
	return true;
}