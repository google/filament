// Copyright 2018 The SwiftShader Authors. All Rights Reserved.
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

// Vulkan unit tests that provide coverage for functionality not tested by
// the dEQP test suite. Also used as a smoke test.

#include "Device.hpp"
#include "Driver.hpp"

#include "gmock/gmock.h"
#include "gtest/gtest.h"

class BasicTest : public testing::Test
{
protected:
	static Driver driver;

	void SetUp() override
	{
		ASSERT_TRUE(driver.loadSwiftShader());
	}

	void TearDown() override
	{
		driver.unload();
	}
};

Driver BasicTest::driver;

TEST_F(BasicTest, ICD_Check)
{
	auto createInstance = driver.vk_icdGetInstanceProcAddr(VK_NULL_HANDLE, "vkCreateInstance");
	EXPECT_NE(createInstance, nullptr);

	auto enumerateInstanceExtensionProperties =
	    driver.vk_icdGetInstanceProcAddr(VK_NULL_HANDLE, "vkEnumerateInstanceExtensionProperties");
	EXPECT_NE(enumerateInstanceExtensionProperties, nullptr);

	auto enumerateInstanceLayerProperties =
	    driver.vk_icdGetInstanceProcAddr(VK_NULL_HANDLE, "vkEnumerateInstanceLayerProperties");
	EXPECT_NE(enumerateInstanceLayerProperties, nullptr);

	auto enumerateInstanceVersion = driver.vk_icdGetInstanceProcAddr(VK_NULL_HANDLE, "vkEnumerateInstanceVersion");
	EXPECT_NE(enumerateInstanceVersion, nullptr);

	auto bad_function = driver.vk_icdGetInstanceProcAddr(VK_NULL_HANDLE, "bad_function");
	EXPECT_EQ(bad_function, nullptr);
}

TEST_F(BasicTest, Version)
{
	uint32_t apiVersion = 0;
	uint32_t expectedVersion = static_cast<uint32_t>(VK_API_VERSION_1_3);
	VkResult result = driver.vkEnumerateInstanceVersion(&apiVersion);
	EXPECT_EQ(apiVersion, expectedVersion);

	const VkInstanceCreateInfo createInfo = {
		VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,  // sType
		nullptr,                                 // pNext
		0,                                       // flags
		nullptr,                                 // pApplicationInfo
		0,                                       // enabledLayerCount
		nullptr,                                 // ppEnabledLayerNames
		0,                                       // enabledExtensionCount
		nullptr,                                 // ppEnabledExtensionNames
	};
	VkInstance instance = VK_NULL_HANDLE;
	result = driver.vkCreateInstance(&createInfo, nullptr, &instance);
	EXPECT_EQ(result, VK_SUCCESS);

	ASSERT_TRUE(driver.resolve(instance));

	uint32_t pPhysicalDeviceCount = 0;
	result = driver.vkEnumeratePhysicalDevices(instance, &pPhysicalDeviceCount, nullptr);
	EXPECT_EQ(result, VK_SUCCESS);
	EXPECT_EQ(pPhysicalDeviceCount, 1U);

	VkPhysicalDevice pPhysicalDevice = VK_NULL_HANDLE;
	result = driver.vkEnumeratePhysicalDevices(instance, &pPhysicalDeviceCount, &pPhysicalDevice);
	EXPECT_EQ(result, VK_SUCCESS);
	EXPECT_NE(pPhysicalDevice, (VkPhysicalDevice)VK_NULL_HANDLE);

	VkPhysicalDeviceProperties physicalDeviceProperties;
	driver.vkGetPhysicalDeviceProperties(pPhysicalDevice, &physicalDeviceProperties);
	EXPECT_EQ(physicalDeviceProperties.apiVersion, expectedVersion);
	EXPECT_EQ(physicalDeviceProperties.deviceID, 0xC0DEU);
	EXPECT_EQ(physicalDeviceProperties.deviceType, VK_PHYSICAL_DEVICE_TYPE_CPU);

	EXPECT_NE(strstr(physicalDeviceProperties.deviceName, "SwiftShader Device"), nullptr);

	VkPhysicalDeviceProperties2 physicalDeviceProperties2;
	VkPhysicalDeviceDriverPropertiesKHR physicalDeviceDriverProperties;
	physicalDeviceProperties2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2;
	physicalDeviceProperties2.pNext = &physicalDeviceDriverProperties;
	physicalDeviceDriverProperties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DRIVER_PROPERTIES_KHR;
	physicalDeviceDriverProperties.pNext = nullptr;
	physicalDeviceDriverProperties.driverID = (VkDriverIdKHR)0;
	driver.vkGetPhysicalDeviceProperties2(pPhysicalDevice, &physicalDeviceProperties2);
	EXPECT_EQ(physicalDeviceDriverProperties.driverID, VK_DRIVER_ID_GOOGLE_SWIFTSHADER_KHR);

	driver.vkDestroyInstance(instance, nullptr);
}
/*
TEST_F(BasicTest, UnsupportedDeviceExtension_DISABLED)
{
    uint32_t apiVersion = 0;
    VkResult result = driver.vkEnumerateInstanceVersion(&apiVersion);

    const VkInstanceCreateInfo createInfo = {
        VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,  // sType
        nullptr,                                 // pNext
        0,                                       // flags
        nullptr,                                 // pApplicationInfo
        0,                                       // enabledLayerCount
        nullptr,                                 // ppEnabledLayerNames
        0,                                       // enabledExtensionCount
        nullptr,                                 // ppEnabledExtensionNames
    };
    VkInstance instance = VK_NULL_HANDLE;
    result = driver.vkCreateInstance(&createInfo, nullptr, &instance);
    EXPECT_EQ(result, VK_SUCCESS);

    ASSERT_TRUE(driver.resolve(instance));

    VkBaseInStructure unsupportedExt = { VK_STRUCTURE_TYPE_SHADER_MODULE_VALIDATION_CACHE_CREATE_INFO_EXT, nullptr };

    // Gather all physical devices
    std::vector<VkPhysicalDevice> physicalDevices;
    result = Device::GetPhysicalDevices(&driver, instance, physicalDevices);
    EXPECT_EQ(result, VK_SUCCESS);

    // Inspect each physical device's queue families for compute support.
    for(auto physicalDevice : physicalDevices)
    {
        int queueFamilyIndex = Device::GetComputeQueueFamilyIndex(&driver, physicalDevice);
        if(queueFamilyIndex < 0)
        {
            continue;
        }

        const float queuePrioritory = 1.0f;
        const VkDeviceQueueCreateInfo deviceQueueCreateInfo = {
            VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,  // sType
            nullptr,                                     // pNext
            0,                                           // flags
            (uint32_t)queueFamilyIndex,                  // queueFamilyIndex
            1,                                           // queueCount
            &queuePrioritory,                            // pQueuePriorities
        };

        const VkDeviceCreateInfo deviceCreateInfo = {
            VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,  // sType
            &unsupportedExt,                       // pNext
            0,                                     // flags
            1,                                     // queueCreateInfoCount
            &deviceQueueCreateInfo,                // pQueueCreateInfos
            0,                                     // enabledLayerCount
            nullptr,                               // ppEnabledLayerNames
            0,                                     // enabledExtensionCount
            nullptr,                               // ppEnabledExtensionNames
            nullptr,                               // pEnabledFeatures
        };

        VkDevice device;
        result = driver.vkCreateDevice(physicalDevice, &deviceCreateInfo, nullptr, &device);
        EXPECT_EQ(result, VK_SUCCESS);
        driver.vkDestroyDevice(device, nullptr);
    }

    driver.vkDestroyInstance(instance, nullptr);
}
*/
