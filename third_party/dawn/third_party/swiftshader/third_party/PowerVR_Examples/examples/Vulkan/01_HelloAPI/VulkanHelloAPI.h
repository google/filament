/*!*********************************************************************************************************************
\File         VulkanHelloAPI.h
\Title        VulkanHello API Header
\Author       PowerVR by Imagination, Developer Technology Team.
\Copyright    Copyright(c) Imagination Technologies Limited.
\brief        Header file for VulkanHelloAPI class. It also contains helper functions and structs.
***********************************************************************************************************************/
#pragma once
#include "vk_getProcAddrs.h"
#include <string>
#include <array>

#include <sstream>
#include <iostream>

// The Surface Data structure is different based on the platform being used.
// The structure is defined and its members, inside Vulkan-provided preprocessors.

#ifdef VK_USE_PLATFORM_WIN32_KHR
struct SurfaceData
{
	float width, height;

	HINSTANCE connection;
	HWND window;

	SurfaceData() = default;
};
#endif

#ifdef VK_USE_PLATFORM_XLIB_KHR
struct SurfaceData
{
	float width, height;

	Display* display;
	Window window;

	SurfaceData() {}
};
#endif

#ifdef VK_USE_PLATFORM_XCB_KHR
struct SurfaceData
{
	float width, height;

	xcb_connection_t* connection;
	xcb_screen_t* screen;
	xcb_window_t window;

	uint32_t deleteWindowAtom;

	SurfaceData() {}
};
#endif

#ifdef VK_USE_PLATFORM_ANDROID_KHR
struct SurfaceData
{
	float width, height;

	ANativeWindow* window;

	SurfaceData() { width = height = 0; }
};

#endif

#ifdef VK_USE_PLATFORM_WAYLAND_KHR
struct SurfaceData
{
	float width, height;

	wl_display* display;
	wl_surface* surface;
	wl_registry* wlRegistry;
	wl_compositor* wlCompositor;
	wl_shell* wlShell;
	wl_seat* wlSeat;
	wl_pointer* wlPointer;
	wl_shell_surface* wlShellSurface;

	SurfaceData()
	{
		width = height = 0;
		display = NULL;
		surface = NULL;
		wlRegistry = NULL;
		wlCompositor = NULL;
		wlShell = NULL;
		wlSeat = NULL;
		wlPointer = NULL;
		wlShellSurface = NULL;
	}
};
#endif

#ifdef VK_USE_PLATFORM_MACOS_MVK
struct SurfaceData
{
    float width, height;

    void* view;

    SurfaceData()
    {
        width = height = 0;
        view = NULL;
    }
};
#endif

#ifdef USE_PLATFORM_NULLWS
struct SurfaceData
{
	float width, height;

	VkDisplayKHR nativeDisplay;
	VkSurfaceKHR surface;

	SurfaceData()
	{
		nativeDisplay = VK_NULL_HANDLE;
		surface = VK_NULL_HANDLE;
		width = height = 0;
	}
};
#endif

#ifdef DEBUG
#define PVR_DEBUG 1
#define LOGERRORSONLY 0
#endif

// Returns a human readable string from VKResults.
static std::string debugGetVKResultString(const VkResult inRes)
{
	switch (inRes)
	{
	case 0: return "VK_SUCCESS";
	case 1: return "VK_NOT_READY";
	case 2: return "VK_TIMEOUT";
	case 3: return "VK_EVENT_SET";
	case 4: return "VK_EVENT_RESET";
	case 5: return "VK_INCOMPLETE";

	case -1: return "VK_ERROR_OUT_OF_HOST_MEMORY";
	case -2: return "VK_ERROR_OUT_OF_DEVICE_MEMORY";
	case -3: return "VK_ERROR_INITIALIZATION_FAILED";
	case -4: return "VK_ERROR_DEVICE_LOST";
	case -5: return "VK_ERROR_MEMORY_MAP_FAILED";
	case -6: return "VK_ERROR_LAYER_NOT_PRESENT";
	case -7: return "VK_ERROR_EXTENSION_NOT_PRESENT";
	case -8: return "VK_ERROR_FEATURE_NOT_PRESENT";
	case -9: return "VK_ERROR_INCOMPATIBLE_DRIVER";
	case -10: return "VK_ERROR_TOO_MANY_OBJECTS";
	case -11: return "VK_ERROR_FORMAT_NOT_SUPPORTED";
	case -12: return "VK_ERROR_FRAGMENTED_POOL";

	default: return "Unknown VkResult Value";
	}
}

// Logs returns from Vulkan function calls.
inline void debugAssertFunctionResult(const VkResult inRes, const std::string inOperation)
{
#ifdef DEBUG
	if (LOGERRORSONLY)
	{
		Log(true, (inOperation + " -- " + debugGetVKResultString(inRes)).c_str());
		assert(inRes == VK_SUCCESS);
	}
#endif
}

// Constants used throughout the example.
#define FENCE_TIMEOUT std::numeric_limits<uint64_t>::max()
#define NUM_DESCRIPTOR_SETS 2

const float PI = 3.14159265359f;
const float TORAD = PI / 180.0f;

class VulkanHelloAPI
{
private:
	// Custom structs that encapsulates related data to help keep track of
	// the multiple aspects of different Vulkan objects.
	struct SwapchainImage
	{
		VkImage image;
		VkImageView view;
	};

	struct BufferData
	{
		VkBuffer buffer;
		VkDeviceMemory memory;
		size_t size;
		VkMemoryPropertyFlags memPropFlags;
		void* mappedData;
		VkDescriptorBufferInfo bufferInfo;

		BufferData() : buffer(VK_NULL_HANDLE), memory(VK_NULL_HANDLE), size(0), memPropFlags(0), mappedData(nullptr) {}
	};

	struct TextureData
	{
		std::vector<uint8_t> data;
		VkExtent2D textureDimensions;
		VkImage image;
		VkDeviceMemory memory;
		VkImageView view;
		VkSampler sampler;
	};

	struct AppManager
	{
		std::vector<const char*> instanceLayerNames;
		std::vector<const char*> instanceExtensionNames;
		std::vector<const char*> deviceExtensionNames;

		std::vector<VkPhysicalDevice> gpus;
		std::vector<VkQueueFamilyProperties> queueFamilyProperties;
		std::vector<SwapchainImage> swapChainImages;
		std::vector<VkCommandBuffer> cmdBuffers;
		std::vector<VkFramebuffer> frameBuffers;
		std::vector<VkDescriptorSetLayout> descriptorSetLayouts;

		std::vector<VkSemaphore> acquireSemaphore;
		std::vector<VkSemaphore> presentSemaphores;
		std::vector<VkFence> frameFences;

		VkInstance instance;
		VkPhysicalDevice physicalDevice;

		VkPhysicalDeviceMemoryProperties deviceMemoryProperties;
		VkPhysicalDeviceProperties deviceProperties;
		uint32_t graphicsQueueFamilyIndex;
		uint32_t presentQueueFamilyIndex;
		VkDevice device;
		VkQueue graphicQueue;
		VkQueue presentQueue;
		VkSurfaceKHR surface;
		VkSurfaceFormatKHR surfaceFormat;
		VkSwapchainKHR swapchain;
		VkPresentModeKHR presentMode;
		VkExtent2D swapchainExtent;
		VkPipelineShaderStageCreateInfo shaderStages[2];
		VkRenderPass renderPass;
		VkPipelineLayout pipelineLayout;
		VkPipeline pipeline;
		VkCommandPool commandPool;
		VkViewport viewport;
		VkRect2D scissor;
		VkDescriptorPool descriptorPool;
		VkDescriptorSet dynamicDescSet;
		VkDescriptorSet staticDescSet;
		VkDescriptorSetLayout staticDescriptorSetLayout;
		VkDescriptorSetLayout dynamicDescriptorSetLayout;

		BufferData vertexBuffer;
		BufferData dynamicUniformBufferData;
		TextureData texture;

		float angle;
		uint32_t offset;
	};

	struct Vertex
	{
		float x, y, z, w; // coordinates.
		float u, v; // texture UVs.
	};

	std::array<std::array<float, 4>, 4> viewProj = std::array<std::array<float, 4>, 4>();

	// Check type of memory using the device memory properties.
	bool getMemoryTypeFromProperties(const VkPhysicalDeviceMemoryProperties& memory_properties, uint32_t typeBits, VkFlags requirements_mask, uint32_t* typeIndex)
	{
		// Search memory types to find first index with those properties.
		for (uint32_t i = 0; i < memory_properties.memoryTypeCount; i++)
		{
			if ((typeBits & 1) == 1)
			{
				// Type is available, does it match user properties?
				if ((memory_properties.memoryTypes[i].propertyFlags & requirements_mask) == requirements_mask)
				{
					*typeIndex = i;
					return true;
				}
			}
			typeBits >>= 1;
		}
		// No memory types matched, return failure.
		return false;
	}
	// This method checks for physical device compatibility.
	VkPhysicalDevice getCompatibleDevice();

	// Get a compatible queue family with the properties, and it needs to be a graphical one.
	void getCompatibleQueueFamilies(uint32_t& graphicsfamilyindex, uint32_t& presentfamilyindex);

	// Make sure the extent is correct, and if not, set the same sizes as the window.
	VkExtent2D getCorrectExtent(const VkSurfaceCapabilitiesKHR& inSurfCap);

	void initUniformBuffers();

public:
	// Initialise the validation layers.
	std::vector<std::string> initLayers();

	// Initialise the needed extensions.
	std::vector<std::string> initInstanceExtensions();
	std::vector<std::string> initDeviceExtensions();

	// Initialise the application and instance.
	void initApplicationAndInstance(std::vector<std::string>& extensionNames, std::vector<std::string>& layerNames);

	// Fetch the physical devices and get a compatible one.
	void initPhysicalDevice();

	// Find queues families and individual queues from device.
	void initQueuesFamilies();

	// Initialise the logical device.
	void initLogicalDevice(std::vector<std::string>& deviceExtensions);

	// Fetch queues from device.
	void initQueues();

	// Create the surface to be rendered on (platform-dependent).
	void initSurface();

	// Create the swapchain.
	void initSwapChain();

	// Create the images and image views to be used with the swapchain.
	void initImagesAndViews();

	// Create vertex buffers to draw the primitive.
	void initVertexBuffers();

	// Create a texture to apply to the primitive.
	void initTexture();

	// Create a descriptor pool and allocate descriptor sets for the buffers.
	void initDescriptorPoolAndSet();

	// Compile and convert the shaders that will be used.
	void initShaders();

	// Create the pipeline to use for the rendering.
	void initPipeline();

	// Create the render pass to use for rendering the triangle.
	void initRenderPass();

	// Create the frame buffers for rendering.
	void initFrameBuffers();

	// Create the command buffer to be sent to the GPU from the command pool.
	void initCommandPoolAndBuffer();

	// Initialise the viewport and scissor for the rendering.
	void initViewportAndScissor();

	// Create the semaphore to deal with the command queue.
	void initSemaphoreAndFence();

	// Generic method to initialise buffers. Both vertex and uniform buffers use this.
	void createBuffer(BufferData& inBuffer, const uint8_t* inData, const VkBufferUsageFlags& inUsage);

	// Generic method for creating a dynamic uniform buffer.
	void createDynamicUniformBuffer(BufferData& inBuffer);

	// Generic method for creating a shader module.
	void createShaderModule(const uint32_t* spvShader, size_t spvShaderSize, int indx, VkShaderStageFlagBits shaderStage);

	// Generates a texture without having to load an image file.
	void generateTexture();

	// Changes the rotation of the per-frame uniform buffer.
	void applyRotation(int idx = 0);

	// Initialises all the needed Vulkan objects, but calling all the Init__ methods.
	void initialize();

	// Cleans up everything when the application is finished with.
	void deinitialize();

	// Record the command buffer for rendering the example.
	void recordCommandBuffer();

	// Execute the command buffer and present the result to the surface.
	void drawFrame();

	// Holds all the Vulkan handles that global access is required for.
	AppManager appManager;

	// Used for debugging mostly; to show the VKResult return from the Vulkan function calls.
	VkResult lastRes;

	// Keeps track of the frame for synchronisation purposes.
	int frameId;

	// Surface data needed to distinguish between the different platforms.
	SurfaceData surfaceData;
};
