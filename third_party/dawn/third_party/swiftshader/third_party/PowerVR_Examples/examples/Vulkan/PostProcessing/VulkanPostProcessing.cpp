/*!
\brief Shows how to implement post processing in vulkan.
\file VulkanPostProcessing.cpp
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
#include "PVRShell/PVRShell.h"
#include "PVRUtils/PVRUtilsVk.h"
#include "PVRCore/cameras/TPSCamera.h"

namespace BufferEntryNames {
namespace PerMesh {
const char* const MVPMatrix = "mvpMatrix";
const char* const WorldMatrix = "worldMatrix";
} // namespace PerMesh

namespace Scene {
const char* const EyePosition = "eyePosition";
const char* const InverseViewProjectionMatrix = "inverseViewProjectionMatrix";
} // namespace Scene
} // namespace BufferEntryNames

// Bloom modes
enum class BloomMode
{
	NoBloom = 0,
	GaussianOriginal,
	GaussianLinear,
	Compute,
	HybridGaussian,
	GaussianLinearTruncated,
	Kawase,
	DualFilter,
	TentFilter,
	NumBloomModes,
	DefaultMode = GaussianLinearTruncated
};

// Titles for the various bloom modes
const std::string BloomStrings[] = { "Original Image (No Post Processing)", "Gaussian (Reference Implementation)", "Gaussian (Linear Sampling)",
	"Gaussian (Compute Sliding Average)", "Hybrid Gaussian", "Truncated Gaussian (Linear Sampling)", "Kawase", "Dual Filter", "Tent Filter" };

// Files used throughout the demo
namespace Files {
// Shader file names
const char Downsample4x4VertSrcFile[] = "Downsample4x4VertShader.vsh.spv";
const char Downsample4x4FragSrcFile[] = "Downsample4x4FragShader.fsh.spv";

// Dual Filter shaders
const char DualFilterDownSampleFragSrcFile[] = "DualFilterDownSampleFragShader.fsh.spv";
const char DualFilterUpSampleFragSrcFile[] = "DualFilterUpSampleFragShader.fsh.spv";
const char DualFilterUpSampleMergedFinalPassFragSrcFile[] = "DualFilterUpSampleMergedFinalPassFragShader.fsh.spv";
const char DualFilterDownVertSrcFile[] = "DualFilterDownVertShader.vsh.spv";
const char DualFilterUpVertSrcFile[] = "DualFilterUpVertShader.vsh.spv";

// Tent Filter shaders
const char TentFilterUpSampleVertSrcFile[] = "TentFilterUpSampleVertShader.vsh.spv";
const char TentFilterFirstUpSampleFragSrcFile[] = "TentFilterFirstUpSampleFragShader.fsh.spv";
const char TentFilterUpSampleFragSrcFile[] = "TentFilterUpSampleFragShader.fsh.spv";
const char TentFilterUpSampleMergedFinalPassFragSrcFile[] = "TentFilterUpSampleMergedFinalPassFragShader.fsh.spv";

// Kawase Blur shaders
const char KawaseVertSrcFile[] = "KawaseVertShader.vsh.spv";
const char KawaseFragSrcFile[] = "KawaseFragShader.fsh.spv";

// Traditional Gaussian Blur shaders
const char GaussianFragSrcFile[] = "GaussianBlurFragmentShader.fsh.template";
const char GaussianVertSrcFile[] = "GaussianVertShader.vsh.spv";

// Linear Sampler Optimised Gaussian Blur shaders
const char LinearGaussianVertSrcFile[] = "LinearGaussianBlurVertexShader.vsh.template";
const char LinearGaussianFragSrcFile[] = "LinearGaussianBlurFragmentShader.fsh.template";

// Compute based sliding average Gaussian Blur shaders
const char GaussianComputeBlurHorizontalSrcFile[] = "ComputeGaussianBlurHorizontalShader.csh.template";
const char GaussianComputeBlurVerticalSrcFile[] = "ComputeGaussianBlurVerticalShader.csh.template";

// Post Bloom Shaders
const char PostBloomVertShaderSrcFile[] = "PostBloomVertShader.vsh.spv";
const char PostBloomFragShaderSrcFile[] = "PostBloomFragShader.fsh.spv";

// Scene Rendering shaders
const char FragShaderSrcFile[] = "FragShader.fsh.spv";
const char VertShaderSrcFile[] = "VertShader.vsh.spv";
const char SkyboxFragShaderSrcFile[] = "SkyboxFragShader.fsh.spv";
const char SkyboxVertShaderSrcFile[] = "SkyboxVertShader.vsh.spv";
} // namespace Files

// POD scene files
const char SceneFile[] = "Satyr.pod";

// Texture files
const char StatueTexFile[] = "Marble.pvr";
const char StatueNormalMapTexFile[] = "MarbleNormalMap.pvr";

struct EnvironmentTextures
{
	std::string skyboxTexture;
	std::string diffuseIrradianceMapTexture;
	float averageLuminance;
	float keyValue;
	float threshold;

	float getLinearExposure() const { return keyValue / averageLuminance; }
};

float luma(glm::vec3 color) { return glm::max(glm::dot(color, glm::vec3(0.2126f, 0.7152f, 0.0722f)), 0.0001f); }

// The following were taken from the lowest mipmap of each of the corresponding irradiance textures
float sataraNightLuminance = luma((1.0f / 6.0f) *
	(glm::vec3(55.0f, 42.0f, 13.0f) + glm::vec3(21.0f, 16.0f, 8.0f) + glm::vec3(7.0f, 5.0f, 6.0f) + glm::vec3(5.0f, 4.0f, 1.0f) + glm::vec3(72.0f, 57.0f, 19.0f) +
		glm::vec3(14.0f, 10.0f, 5.0f)));

float pinkSunriseLuminance = luma((1.0f / 6.0f) *
	(glm::vec3(104.0f, 76.0f, 106.0f) + glm::vec3(28.0f, 23.0f, 41.0f) + glm::vec3(137.0f, 110.0f, 197.0f) + glm::vec3(9.0f, 6.0f, 7.0f) + glm::vec3(129.0f, 89.0f, 113.0f) +
		glm::vec3(28.0f, 27.0f, 54.0f)));

float signalHillSunriseLuminance = luma((1.0f / 6.0f) *
	(glm::vec3(10.0f, 10.0f, 10.0f) + glm::vec3(4.0f, 4.0f, 6.0f) + glm::vec3(8.0f, 10.0f, 16.0f) + glm::vec3(4.0f, 2.0f, 0.0f) + glm::vec3(9.0f, 9.0f, 9.0f) +
		glm::vec3(4.0f, 4.0f, 5.0f)));

// Textures
EnvironmentTextures SceneTexFileNames[] = { { "satara_night_scale_0.305_rgb9e5.pvr", "satara_night_scale_0.305_rgb9e5_Irradiance.pvr", sataraNightLuminance, 9.0f, 2.6f },
	{ "pink_sunrise_rgb9e5.pvr", "pink_sunrise_rgb9e5_Irradiance.pvr", pinkSunriseLuminance, 50.0f, 0.65f },
	{ "signal_hill_sunrise_scale_0.312_rgb9e5.pvr", "signal_hill_sunrise_scale_0.312_rgb9e5_Irradiance.pvr", signalHillSunriseLuminance, 23.0f, 0.85f } };

const int NumScenes = sizeof SceneTexFileNames / sizeof SceneTexFileNames[0];

// Various defaults
const float CameraNear = 1.0f;
const float CameraFar = 1000.0f;
const float RotateY = glm::pi<float>() / 150;
const float Fov = 0.80f;
const float MinimumAcceptibleCoefficient = 0.0003f;
const uint8_t MaxFilterIterations = 10;
const uint8_t MaxKawaseIteration = 5;
const uint8_t MaxGaussianKernel = 51;
const uint8_t MaxGaussianHalfKernel = (MaxGaussianKernel - 1) / 2 + 1;

const pvr::utils::VertexBindings VertexAttribBindings[] = {
	{ "POSITION", 0 },
	{ "NORMAL", 1 },
	{ "UV0", 2 },
	{ "TANGENT", 3 },
};

// Handles the configurations being used in the demo controlling how the various bloom techniques will operate
namespace DemoConfigurations {
// Wrapper for a Kawase pass including the number of iterations in use and their kernel sizes
struct KawasePass
{
	uint32_t numIterations;
	uint32_t kernel[MaxKawaseIteration];
};

// A wrapper for the demo configuration at any time
struct DemoConfiguration
{
	uint32_t gaussianConfig;
	uint32_t linearGaussianConfig;
	uint32_t computeGaussianConfig;
	uint32_t truncatedLinearGaussianConfig;
	KawasePass kawaseConfig;
	uint32_t dualFilterConfig;
	uint32_t tentFilterConfig;
	uint32_t hybridConfig;
};

const uint32_t NumDemoConfigurations = 5;
const uint32_t DefaultDemoConfigurations = 2;
DemoConfiguration Configurations[NumDemoConfigurations]{ // Demo Blur Configurations
	DemoConfiguration{
		5, // Original Gaussian Blur
		5, // Linear Gaussian Blur
		5, // Compute Gaussian Blur
		5, // Truncated Linear Gaussian Blur
		KawasePass{ 2, { 0, 0 } }, // Kawase Blur
		2, // Dual Filter Blur
		2, // Tent Filter
		0, // Hybrid
	},
	DemoConfiguration{
		15, // Original Gaussian Blur
		15, // Linear Gaussian Blur
		15, // Compute Gaussian Blur
		11, // Truncated Linear Gaussian Blur
		KawasePass{ 3, { 0, 0, 1 } }, // Kawase Blur
		4, // Dual Filter Blur
		4, // Tent Filter
		0, // Hybrid
	},
	DemoConfiguration{
		25, // Original Gaussian Blur
		25, // Linear Gaussian Blur
		25, // Compute Gaussian Blur
		17, // Truncated Linear Gaussian Blur
		KawasePass{ 4, { 0, 0, 1, 1 } }, // Kawase Blur
		6, // Dual Filter Blur
		6, // Tent Filter
		0, // Hybrid
	},
	DemoConfiguration{
		35, // Original Gaussian Blur
		35, // Linear Gaussian Blur
		35, // Compute Gaussian Blur
		21, // Truncated Linear Gaussian Blur
		KawasePass{ 4, { 0, 1, 1, 1 } }, // Kawase Blur
		8, // Dual Filter Blur
		8, // Tent Filter
		0, // Hybrid
	},
	DemoConfiguration{
		51, // Original Gaussian Blur
		51, // Linear Gaussian Blur
		51, // Compute Gaussian Blur
		25, // Truncated Linear Gaussian Blur
		KawasePass{ 5, { 0, 0, 1, 1, 2 } }, // Kawase Blur
		10, // Dual Filter Blur
		10, // Tent Filter
		0, // Hybrid
	}
};
} // namespace DemoConfigurations

/// <summary>Prints the Gaussian weights and offsets provided in the vectors.</summary>
/// <param name="gaussianWeights">The list of Gaussian weights to print.</param>
/// <param name="gaussianOffsets">The list of Gaussian offsets to print.</param>
/// <param name="iterationsString">A string defining the number of iterations.</param>
/// <param name="weightsString">A string defining the iteration set of weights.</param>
/// <param name="offsetsString">A string defining the iteration set of offsets.</param>
void generateGaussianWeightsAndOffsetsStrings(std::vector<double>& gaussianWeights, std::vector<double>& gaussianOffsets, std::string& iterationsString, std::string& weightsString,
	std::string& offsetsString, bool duplicateWeightsStrings = false)
{
	std::string weights;
	for (uint32_t i = 0; i < gaussianWeights.size() - 1; ++i) { weights += pvr::strings::createFormatted("%.15f,", gaussianWeights[i]); }
	weights += pvr::strings::createFormatted("%.15f", gaussianWeights[gaussianWeights.size() - 1]);

	std::string offsets;
	for (uint32_t i = 0; i < gaussianOffsets.size() - 1; ++i) { offsets += pvr::strings::createFormatted("%.15f,", gaussianOffsets[i]); }
	offsets += pvr::strings::createFormatted("%.15f", gaussianOffsets[gaussianOffsets.size() - 1]);

	if (duplicateWeightsStrings)
	{
		weights += "," + weights;

		weightsString = pvr::strings::createFormatted("const mediump float gWeights[numIterations * 2] = {%s};", weights.c_str());
	}
	else
	{
		weightsString = pvr::strings::createFormatted("const mediump float gWeights[numIterations] = {%s};", weights.c_str());
		offsetsString = pvr::strings::createFormatted("const mediump float gOffsets[numIterations] = {%s};", offsets.c_str());
	}
	iterationsString = pvr::strings::createFormatted("const uint numIterations = %uu;", gaussianWeights.size());
}

/// <summary>Updates the Gaussian weights and offsets using the configuration provided.</summary>
/// <param name="kernelSize">The kernel size to generate Gaussian weights and offsets for.</param>
/// <param name="useLinearOptimisation">Specifies whether linear sampling will be used when texture sampling using the given weights and offsets,
/// if linear sampling will be used then the weights and offsets must be adjusted accordingly.</param>
/// <param name="truncateCoefficients">Specifies whether to truncate and ignore coefficients which would provide a negligible change in the resulting blurred image.</param>
/// <param name="gaussianWeights">The returned list of Gaussian weights (as double).</param>
/// <param name="gaussianOffsets">The returned list of Gaussian offsets (as double).</param>
void generateGaussianCoefficients(uint32_t kernelSize, bool useLinearOptimisation, bool truncateCoefficients, std::vector<double>& gaussianWeights, std::vector<double>& gaussianOffsets)
{
	// Ensure that the kernel given is odd in size
	assertion((kernelSize - 1) % 2 == 0);
	assertion(kernelSize <= MaxGaussianKernel);

	// generate a new set of weights and offsets based on the given configuration
	pvr::math::generateGaussianKernelWeightsAndOffsets(kernelSize, truncateCoefficients, useLinearOptimisation, gaussianWeights, gaussianOffsets, MinimumAcceptibleCoefficient);
}

bool queueFamiliesCompatible(pvrvk::ImageView& imageView, const uint32_t* queueFamilyIndices, uint32_t numQueueFamilyIndices)
{
	for (uint32_t i = 0; i < numQueueFamilyIndices; ++i)
	{
		bool foundMatch = false;
		for (uint32_t j = 0; j < imageView->getCreateInfo().getImage()->getNumQueueFamilyIndices(); ++j)
		{
			if (imageView->getCreateInfo().getImage()->getQueueFamilyIndices()[j] == queueFamilyIndices[i]) { foundMatch = true; }
		}
		if (!foundMatch) { return false; }
	}

	return true;
}

bool isCompatibleImageView(pvrvk::ImageView& imageView, pvrvk::ImageType imageType, pvrvk::Format format, const pvrvk::Extent3D& dimension, pvrvk::ImageUsageFlags usage,
	const pvrvk::ImageLayersSize& layerSize, pvrvk::SampleCountFlags samples, pvrvk::MemoryPropertyFlags memoryFlags, pvrvk::SharingMode sharingMode, pvrvk::ImageTiling tiling,
	const uint32_t* queueFamilyIndices, uint32_t numQueueFamilyIndices)
{
	if (imageView)
	{
		if (imageView->getImage()->getImageType() == imageType && imageView->getImage()->getFormat() == format && imageView->getImage()->getExtent().getWidth() == dimension.getWidth() &&
			imageView->getImage()->getExtent().getHeight() == dimension.getHeight() && imageView->getImage()->getExtent().getDepth() == dimension.getDepth() &&
			static_cast<uint32_t>(imageView->getImage()->getUsageFlags() & usage) != 0 && imageView->getImage()->getNumArrayLayers() == layerSize.getNumArrayLevels() &&
			imageView->getImage()->getNumMipLevels() == layerSize.getNumMipLevels() && imageView->getImage()->getNumSamples() == samples &&
			static_cast<uint32_t>(imageView->getImage()->getDeviceMemory()->getMemoryFlags() & memoryFlags) != 0 && imageView->getImage()->getSharingMode() == sharingMode &&
			imageView->getImage()->getTiling() == tiling && queueFamiliesCompatible(imageView, queueFamilyIndices, numQueueFamilyIndices))
		{ return true; }
	}
	return false;
}

void addImageToSharedImages(std::vector<pvr::Multi<pvrvk::ImageView>>& sharedImageViews, pvrvk::ImageView& imageView, uint32_t currentSwapchainIndex)
{
	// Naively add a new entry to the back of the shared image list if the swapchain is 0
	// This requires that images are added in "swapchain order"
	if (currentSwapchainIndex == 0)
	{
		sharedImageViews.resize(sharedImageViews.size() + 1);
		sharedImageViews[sharedImageViews.size() - 1].add(imageView);
	}
	else
	{
		for (uint32_t i = 0; i < sharedImageViews.size(); ++i)
		{
			if (isCompatibleImageView(sharedImageViews[i][0], imageView->getImage()->getImageType(), imageView->getImage()->getFormat(), imageView->getImage()->getExtent(),
					imageView->getImage()->getUsageFlags(), pvrvk::ImageLayersSize(imageView->getImage()->getNumArrayLayers(), imageView->getImage()->getNumMipLevels()),
					imageView->getImage()->getNumSamples(), imageView->getImage()->getDeviceMemory()->getMemoryFlags(), imageView->getImage()->getSharingMode(),
					imageView->getImage()->getTiling(), imageView->getImage()->getQueueFamilyIndices(), imageView->getImage()->getNumQueueFamilyIndices()))
			{ sharedImageViews[i].add(imageView); }
		}
	}
}

/// <summary>A simple pass used for rendering our statue object.</summary>
struct StatuePass
{
	pvrvk::GraphicsPipeline pipeline;
	pvrvk::PipelineLayout pipelineLayout;
	pvrvk::ImageView albeoImageView;
	pvrvk::ImageView normalMapImageView;
	pvrvk::DescriptorSetLayout descriptorSetLayout;
	pvr::Multi<pvrvk::DescriptorSet> descriptorSets;
	pvr::Multi<pvrvk::SecondaryCommandBuffer> cmdBuffers;
	pvr::utils::StructuredBufferView structuredBufferView;
	pvrvk::Buffer buffer;
	std::vector<pvrvk::Buffer> vbos;
	std::vector<pvrvk::Buffer> ibos;

	// 3D Model
	pvr::assets::ModelHandle scene;

	/// <summary>Initialises the Statue pass.</summary>
	/// <param name="assetProvider">The pvr::IAssetProvider which will be used for loading resources from memory.</param>
	/// <param name="device">The device from which the resources will be allocated.</param>
	/// <param name="swapchain">The swapchain which will determine the number of per swapchain resources to allocate.</param>
	/// <param name="commandPool">The command pool from which to allocate command buffers.</param>
	/// <param name="descriptorPool">The descriptor pool from which to allocate descriptor sets.</param>
	/// <param name="renderpass">The RenderPass to use.</param>
	/// <param name="framebuffers">The framebuffers to use.</param>
	/// <param name="vmaAllocator">A VMA allocator to use for allocating images and buffers.</param>
	/// <param name="utilityCommandBuffer">A command buffer to use for queueing up all initialisation commands. This command buffer will be submitted later by the main
	/// application.</param>
	/// <param name="pipelineCache">A pipeline cache object to use when creating pipelines.</param>
	void init(pvr::IAssetProvider& assetProvider, pvrvk::Device& device, pvrvk::Swapchain& swapchain, pvrvk::CommandPool& commandPool, pvrvk::DescriptorPool& descriptorPool,
		pvrvk::RenderPass& renderpass, pvr::utils::vma::Allocator& vmaAllocator, pvrvk::CommandBuffer& utilityCommandBuffer, pvrvk::PipelineCache& pipelineCache)
	{
		// Load the scene
		scene = pvr::assets::loadModel(assetProvider, SceneFile);

		bool requiresCommandBufferSubmission = false;
		pvr::utils::appendSingleBuffersFromModel(device, *scene, vbos, ibos, utilityCommandBuffer, requiresCommandBufferSubmission, vmaAllocator);

		createBuffer(device, swapchain, vmaAllocator);
		loadTextures(assetProvider, device, utilityCommandBuffer, vmaAllocator);

		createDescriptorSetLayout(device);
		createPipeline(assetProvider, device, renderpass, swapchain->getDimension(), pipelineCache);

		for (uint32_t i = 0; i < swapchain->getSwapchainLength(); ++i) { descriptorSets.add(descriptorPool->allocateDescriptorSet(descriptorSetLayout)); }

		for (uint32_t i = 0; i < swapchain->getSwapchainLength(); ++i) { cmdBuffers.add(commandPool->allocateSecondaryCommandBuffer()); }
	}

	/// <summary>Update the object animation.</summary>
	/// <param name="angle">The angle to use for rotating the statue.</param>
	/// <param name="viewProjectionMatrix">The view projection matrix to use for rendering.</param>
	/// <param name="swapchainIndex">The current swapchain index.</param>
	void updateAnimation(const float angle, glm::mat4& viewProjectionMatrix, uint32_t swapchainIndex)
	{
		// Calculate the model matrix
		const glm::mat4 mModel = glm::translate(glm::vec3(0.0f, 5.0f, 0.0f)) * glm::rotate(angle, glm::vec3(0.0f, 1.0f, 0.0f)) * glm::scale(glm::vec3(2.2f));

		glm::mat4 worldMatrix = mModel * scene->getWorldMatrix(scene->getNode(0).getObjectId());
		glm::mat4 mvpMatrix = viewProjectionMatrix * worldMatrix;

		structuredBufferView.getElementByName(BufferEntryNames::PerMesh::MVPMatrix, 0, swapchainIndex).setValue(mvpMatrix);
		structuredBufferView.getElementByName(BufferEntryNames::PerMesh::WorldMatrix, 0, swapchainIndex).setValue(worldMatrix);

		// if the memory property flags used by the buffers' device memory do not contain e_HOST_COHERENT_BIT then we must flush the memory
		if (static_cast<uint32_t>(buffer->getDeviceMemory()->getMemoryFlags() & pvrvk::MemoryPropertyFlags::e_HOST_COHERENT_BIT) == 0)
		{ buffer->getDeviceMemory()->flushRange(structuredBufferView.getDynamicSliceOffset(swapchainIndex), structuredBufferView.getDynamicSliceSize()); }
	}

	/// <summary>Creates any required buffers.</summary>
	/// <param name="device">The device from which the resources will be allocated.</param>
	/// <param name="swapchain">The swapchain which will determine the number of per swapchain resources to allocate.</param>
	/// <param name="vmaAllocator">A VMA allocator to use for allocating images and buffers.</param>
	void createBuffer(pvrvk::Device device, pvrvk::Swapchain& swapchain, pvr::utils::vma::Allocator& vmaAllocator)
	{
		pvr::utils::StructuredMemoryDescription desc;
		desc.addElement(BufferEntryNames::PerMesh::MVPMatrix, pvr::GpuDatatypes::mat4x4);
		desc.addElement(BufferEntryNames::PerMesh::WorldMatrix, pvr::GpuDatatypes::mat4x4);

		structuredBufferView.initDynamic(desc, scene->getNumMeshNodes() * swapchain->getSwapchainLength(), pvr::BufferUsageFlags::UniformBuffer,
			static_cast<uint32_t>(device->getPhysicalDevice()->getProperties().getLimits().getMinUniformBufferOffsetAlignment()));
		buffer = pvr::utils::createBuffer(device, pvrvk::BufferCreateInfo(structuredBufferView.getSize(), pvrvk::BufferUsageFlags::e_UNIFORM_BUFFER_BIT),
			pvrvk::MemoryPropertyFlags::e_HOST_VISIBLE_BIT,
			pvrvk::MemoryPropertyFlags::e_DEVICE_LOCAL_BIT | pvrvk::MemoryPropertyFlags::e_HOST_VISIBLE_BIT | pvrvk::MemoryPropertyFlags::e_HOST_COHERENT_BIT, vmaAllocator,
			pvr::utils::vma::AllocationCreateFlags::e_MAPPED_BIT);
		structuredBufferView.pointToMappedMemory(buffer->getDeviceMemory()->getMappedData());
	}

	/// <summary>Creates the textures used for rendering the statue.</summary>
	/// <param name="assetProvider">The pvr::IAssetProvider which will be used for loading resources from memory.</param>
	/// <param name="device">The device from which the resources will be allocated.</param>
	/// <param name="utilityCommandBuffer">A command buffer to use for queueing up all initialisation commands. This command buffer will be submitted later by the main
	/// application.</param>
	/// <param name="vmaAllocator">A VMA allocator to use for allocating images and buffers.</param>
	void loadTextures(pvr::IAssetProvider& assetProvider, pvrvk::Device device, pvrvk::CommandBuffer& utilityCommandBuffer, pvr::utils::vma::Allocator& vmaAllocator)
	{
		// Load the Texture PVR file from the disk
		pvr::Texture albedoTexture = pvr::textureLoad(*assetProvider.getAssetStream(StatueTexFile), pvr::TextureFileFormat::PVR);
		pvr::Texture normalMapTexture = pvr::textureLoad(*assetProvider.getAssetStream(StatueNormalMapTexFile), pvr::TextureFileFormat::PVR);

		// Create and Allocate Textures.
		albeoImageView = pvr::utils::uploadImageAndView(
			device, albedoTexture, true, utilityCommandBuffer, pvrvk::ImageUsageFlags::e_SAMPLED_BIT, pvrvk::ImageLayout::e_SHADER_READ_ONLY_OPTIMAL, vmaAllocator, vmaAllocator);

		normalMapImageView = pvr::utils::uploadImageAndView(device, normalMapTexture, true, utilityCommandBuffer, pvrvk::ImageUsageFlags::e_SAMPLED_BIT,
			pvrvk::ImageLayout::e_SHADER_READ_ONLY_OPTIMAL, vmaAllocator, vmaAllocator);
	}

	/// <summary>Creates the descriptor set layouts used for rendering the statue.</summary>
	/// <param name="device">The device from which the descriptor set layouts will be allocated.</param>
	void createDescriptorSetLayout(pvrvk::Device& device)
	{
		pvrvk::DescriptorSetLayoutCreateInfo descSetLayout;

		descSetLayout.setBinding(0, pvrvk::DescriptorType::e_COMBINED_IMAGE_SAMPLER, 1, pvrvk::ShaderStageFlags::e_FRAGMENT_BIT);
		descSetLayout.setBinding(1, pvrvk::DescriptorType::e_COMBINED_IMAGE_SAMPLER, 1, pvrvk::ShaderStageFlags::e_FRAGMENT_BIT);
		descSetLayout.setBinding(2, pvrvk::DescriptorType::e_COMBINED_IMAGE_SAMPLER, 1, pvrvk::ShaderStageFlags::e_FRAGMENT_BIT);
		descSetLayout.setBinding(3, pvrvk::DescriptorType::e_UNIFORM_BUFFER, 1, pvrvk::ShaderStageFlags::e_VERTEX_BIT);

		descriptorSetLayout = device->createDescriptorSetLayout(descSetLayout);

		pvrvk::PipelineLayoutCreateInfo pipelineLayoutInfo;
		pipelineLayoutInfo.setDescSetLayout(0, descriptorSetLayout);

		pvrvk::PushConstantRange pushConstantsRange;
		pushConstantsRange.setOffset(0);
		pushConstantsRange.setSize(static_cast<uint32_t>(pvr::getSize(pvr::GpuDatatypes::Float) * 2));
		pushConstantsRange.setStageFlags(pvrvk::ShaderStageFlags::e_FRAGMENT_BIT);
		pipelineLayoutInfo.setPushConstantRange(0, pushConstantsRange);

		pipelineLayout = device->createPipelineLayout(pipelineLayoutInfo);
	}

	/// <summary>Updates the descriptor sets used for rendering the statue.</summary>
	/// <param name="device">The device from which the resources will be allocated.</param>
	/// <param name="swapchainIndex">The swapchain index of the descriptor set to update.</param>
	/// <param name="diffuseIrradianceMap">The diffuse irradiance map used as a replacement to a fixed albedo.</param>
	/// <param name="samplerBilinear">A bilinear sampler object.</param>
	/// <param name="samplerTrilinear">A trilinear sampler object.</param>
	void updateDescriptorSets(pvrvk::Device& device, uint32_t swapchainIndex, pvrvk::ImageView& diffuseIrradianceMap, pvrvk::Sampler& samplerBilinear, pvrvk::Sampler& samplerTrilinear)
	{
		std::vector<pvrvk::WriteDescriptorSet> writeDescSets;

		writeDescSets.push_back(pvrvk::WriteDescriptorSet(pvrvk::DescriptorType::e_COMBINED_IMAGE_SAMPLER, descriptorSets[swapchainIndex], 0)
									.setImageInfo(0, pvrvk::DescriptorImageInfo(albeoImageView, samplerBilinear, pvrvk::ImageLayout::e_SHADER_READ_ONLY_OPTIMAL)));

		writeDescSets.push_back(pvrvk::WriteDescriptorSet(pvrvk::DescriptorType::e_COMBINED_IMAGE_SAMPLER, descriptorSets[swapchainIndex], 1)
									.setImageInfo(0, pvrvk::DescriptorImageInfo(normalMapImageView, samplerTrilinear, pvrvk::ImageLayout::e_SHADER_READ_ONLY_OPTIMAL)));

		writeDescSets.push_back(pvrvk::WriteDescriptorSet(pvrvk::DescriptorType::e_COMBINED_IMAGE_SAMPLER, descriptorSets[swapchainIndex], 2)
									.setImageInfo(0, pvrvk::DescriptorImageInfo(diffuseIrradianceMap, samplerTrilinear, pvrvk::ImageLayout::e_SHADER_READ_ONLY_OPTIMAL)));

		writeDescSets.push_back(
			pvrvk::WriteDescriptorSet(pvrvk::DescriptorType::e_UNIFORM_BUFFER, descriptorSets[swapchainIndex], 3)
				.setBufferInfo(0, pvrvk::DescriptorBufferInfo(buffer, structuredBufferView.getDynamicSliceOffset(swapchainIndex), structuredBufferView.getDynamicSliceSize())));

		device->updateDescriptorSets(writeDescSets.data(), static_cast<uint32_t>(writeDescSets.size()), nullptr, 0);
	}

	/// <summary>Creates the pipeline.</summary>
	/// <param name="assetProvider">The pvr::IAssetProvider which will be used for loading resources from memory.</param>
	/// <param name="device">The device to use for allocating the pipelines.</param>
	/// <param name="renderpass">The RenderPass to use.</param>
	/// <param name="viewportDimensions">The viewport dimensions.</param>
	/// <param name="pipelineCache">A pipeline cache object to use when creating pipeline.</param>
	void createPipeline(pvr::IAssetProvider& assetProvider, pvrvk::Device& device, const pvrvk::RenderPass& renderpass, const pvrvk::Extent2D& viewportDimensions,
		pvrvk::PipelineCache& pipelineCache)
	{
		pvrvk::GraphicsPipelineCreateInfo pipelineInfo;

		pipelineInfo.viewport.setViewportAndScissor(0,
			pvrvk::Viewport(0.0f, 0.0f, static_cast<float>(viewportDimensions.getWidth()), static_cast<float>(viewportDimensions.getHeight())),
			pvrvk::Rect2D(0, 0, viewportDimensions.getWidth(), viewportDimensions.getHeight()));

		pipelineInfo.rasterizer.setCullMode(pvrvk::CullModeFlags::e_BACK_BIT);

		// depth stencil state
		pipelineInfo.depthStencil.enableDepthWrite(true);
		pipelineInfo.depthStencil.enableDepthTest(true);
		pipelineInfo.depthStencil.setDepthCompareFunc(pvrvk::CompareOp::e_LESS);
		pipelineInfo.depthStencil.enableStencilTest(false);

		// blend state
		pipelineInfo.colorBlend.setAttachmentState(0, pvrvk::PipelineColorBlendAttachmentState());
		pipelineInfo.colorBlend.setAttachmentState(1, pvrvk::PipelineColorBlendAttachmentState());

		pipelineInfo.vertexShader.setShader(device->createShaderModule(pvrvk::ShaderModuleCreateInfo(assetProvider.getAssetStream(Files::VertShaderSrcFile)->readToEnd<uint32_t>())));
		pipelineInfo.fragmentShader.setShader(device->createShaderModule(pvrvk::ShaderModuleCreateInfo(assetProvider.getAssetStream(Files::FragShaderSrcFile)->readToEnd<uint32_t>())));

		const pvr::assets::Mesh& mesh = scene->getMesh(0);
		pipelineInfo.inputAssembler.setPrimitiveTopology(pvr::utils::convertToPVRVk(mesh.getPrimitiveType()));
		pvr::utils::populateInputAssemblyFromMesh(
			mesh, VertexAttribBindings, sizeof(VertexAttribBindings) / sizeof(VertexAttribBindings[0]), pipelineInfo.vertexInput, pipelineInfo.inputAssembler);

		pipelineInfo.renderPass = renderpass;

		pipelineInfo.pipelineLayout = pipelineLayout;

		pipeline = device->createGraphicsPipeline(pipelineInfo, pipelineCache);
	}

	/// <summary>Draws an assets::Mesh after the model view matrix has been set and the material prepared.</summary>
	/// <param name="cmdBuffer">The secondary command buffer to record rendering commands to.</param>
	/// <param name="nodeIndex">Node index of the mesh to draw</param>
	void drawMesh(pvrvk::SecondaryCommandBuffer& cmdBuffer, int nodeIndex)
	{
		const uint32_t meshId = scene->getNode(nodeIndex).getObjectId();
		const pvr::assets::Mesh& mesh = scene->getMesh(meshId);

		// bind the VBO for the mesh
		cmdBuffer->bindVertexBuffer(vbos[meshId], 0, 0);

		//  The geometry can be exported in 4 ways:
		//  - Indexed Triangle list
		//  - Non-Indexed Triangle list
		//  - Indexed Triangle strips
		//  - Non-Indexed Triangle strips
		if (mesh.getNumStrips() == 0)
		{
			// Indexed Triangle list
			if (ibos[meshId])
			{
				cmdBuffer->bindIndexBuffer(ibos[meshId], 0, pvr::utils::convertToPVRVk(mesh.getFaces().getDataType()));
				cmdBuffer->drawIndexed(0, mesh.getNumFaces() * 3, 0, 0, 1);
			}
			else
			{
				// Non-Indexed Triangle list
				cmdBuffer->draw(0, mesh.getNumFaces() * 3, 0, 1);
			}
		}
		else
		{
			uint32_t offset = 0;
			for (uint32_t i = 0; i < mesh.getNumStrips(); ++i)
			{
				if (ibos[meshId])
				{
					// Indexed Triangle strips
					cmdBuffer->bindIndexBuffer(ibos[meshId], 0, pvr::utils::convertToPVRVk(mesh.getFaces().getDataType()));
					cmdBuffer->drawIndexed(0, mesh.getStripLength(i) + 2, offset * 2, 0, 1);
				}
				else
				{
					// Non-Indexed Triangle strips
					cmdBuffer->draw(0, mesh.getStripLength(i) + 2, 0, 1);
				}
				offset += mesh.getStripLength(i) + 2;
			}
		}
	}

	/// <summary>Records the secondary command buffers for rendering the statue.</summary>
	/// <param name="swapchainIndex">The swapchain index for which the recorded command buffer will be used.</param>
	/// <param name="framebuffer">The framebuffer to render into</param>
	/// <param name="exposure">The exposure value used to 'expose' the colour prior to post processing</param>
	/// <param name="threshold">The threshold value used to determine how much of the colour to retain for the bloom</param>
	void recordCommandBuffer(uint32_t swapchainIndex, pvrvk::Framebuffer& framebuffer, float exposure, float threshold)
	{
		cmdBuffers[swapchainIndex]->begin(framebuffer, 0, pvrvk::CommandBufferUsageFlags::e_RENDER_PASS_CONTINUE_BIT);
		pvr::utils::beginCommandBufferDebugLabel(cmdBuffers[swapchainIndex], pvrvk::DebugUtilsLabel("Statue"));
		cmdBuffers[swapchainIndex]->bindPipeline(pipeline);
		cmdBuffers[swapchainIndex]->bindDescriptorSet(pvrvk::PipelineBindPoint::e_GRAPHICS, pipelineLayout, 0u, descriptorSets[swapchainIndex]);
		cmdBuffers[swapchainIndex]->pushConstants(pipelineLayout, pvrvk::ShaderStageFlags::e_FRAGMENT_BIT, 0, static_cast<uint32_t>(pvr::getSize(pvr::GpuDatatypes::Float)), &exposure);
		cmdBuffers[swapchainIndex]->pushConstants(pipelineLayout, pvrvk::ShaderStageFlags::e_FRAGMENT_BIT, static_cast<uint32_t>(pvr::getSize(pvr::GpuDatatypes::Float)),
			static_cast<uint32_t>(pvr::getSize(pvr::GpuDatatypes::Float)), &threshold);
		drawMesh(cmdBuffers[swapchainIndex], 0);
		pvr::utils::endCommandBufferDebugLabel(cmdBuffers[swapchainIndex]);
		cmdBuffers[swapchainIndex]->end();
	}
};

/// <summary>A simple pass used for rendering our skybox.</summary>
struct SkyboxPass
{
	pvrvk::GraphicsPipeline pipeline;
	pvrvk::PipelineLayout pipelineLayout;
	pvrvk::ImageView skyBoxImageView;
	pvrvk::DescriptorSetLayout descriptorSetLayout;
	pvr::Multi<pvrvk::DescriptorSet> descriptorSets;
	pvr::Multi<pvrvk::SecondaryCommandBuffer> cmdBuffers;
	uint32_t currentScene;

	/// <summary>Initialises the skybox pass.</summary>
	/// <param name="assetProvider">The pvr::IAssetProvider which will be used for loading resources from memory.</param>
	/// <param name="device">The device from which the resources will be allocated.</param>
	/// <param name="swapchain">The swapchain which will determine the number of per swapchain resources to allocate.</param>
	/// <param name="commandPool">The command pool from which to allocate command buffers.</param>
	/// <param name="descriptorPool">The descriptor pool from which to allocate descriptor sets.</param>
	/// <param name="renderpass">The RenderPass to use.</param>
	/// <param name="framebuffers">The framebuffers to use.</param>
	/// <param name="pipelineCache">A pipeline cache object to use when creating pipelines.</param>
	void init(pvr::IAssetProvider& assetProvider, pvrvk::Device& device, pvrvk::Swapchain& swapchain, pvrvk::CommandPool& commandPool, pvrvk::DescriptorPool& descriptorPool,
		pvrvk::RenderPass& renderpass, pvrvk::PipelineCache& pipelineCache)
	{
		this->currentScene = -1;
		createDescriptorSetLayout(device);
		createPipeline(assetProvider, device, renderpass, swapchain->getDimension(), pipelineCache);

		for (uint32_t i = 0; i < swapchain->getSwapchainLength(); ++i) { descriptorSets.add(descriptorPool->allocateDescriptorSet(descriptorSetLayout)); }

		for (uint32_t i = 0; i < swapchain->getSwapchainLength(); ++i) { cmdBuffers.add(commandPool->allocateSecondaryCommandBuffer()); }
	}

	/// <summary>Creates the texture used for rendering the skybox.</summary>
	/// <param name="assetProvider">The pvr::IAssetProvider which will be used for loading resources from memory.</param>
	/// <param name="device">The device from which the resources will be allocated.</param>
	/// <param name="utilityCommandBuffer">A command buffer to use for queueing up all initialisation commands. This command buffer will be submitted later by the main
	/// application.</param>
	/// <param name="vmaAllocator">A VMA allocator to use for allocating images and buffers.</param>
	/// <param name="currentScene">The current scene to use.</param>
	void loadSkyBoxImageView(pvr::IAssetProvider& assetProvider, pvrvk::Device device, pvrvk::CommandBuffer& cmdBuffer, pvr::utils::vma::Allocator& vmaAllocator, uint32_t currentScene_)
	{
		// Only load the image view if required
		if (this->currentScene != currentScene_)
		{
			skyBoxImageView.reset();

			// Load the Texture PVR file from the disk
			pvr::Texture skyBoxTexture = pvr::textureLoad(*assetProvider.getAssetStream(SceneTexFileNames[currentScene_].skyboxTexture), pvr::TextureFileFormat::PVR);

			// Create and Allocate Textures.
			skyBoxImageView = pvr::utils::uploadImageAndView(
				device, skyBoxTexture, true, cmdBuffer, pvrvk::ImageUsageFlags::e_SAMPLED_BIT, pvrvk::ImageLayout::e_SHADER_READ_ONLY_OPTIMAL, vmaAllocator, vmaAllocator);

			this->currentScene = currentScene_;
		}
	}

	/// <summary>Creates the descriptor set layouts used for rendering the statue.</summary>
	/// <param name="device">The device from which the descriptor set layouts will be allocated.</param>
	void createDescriptorSetLayout(pvrvk::Device& device)
	{
		pvrvk::DescriptorSetLayoutCreateInfo descSetLayout;

		descSetLayout.setBinding(0, pvrvk::DescriptorType::e_COMBINED_IMAGE_SAMPLER, 1, pvrvk::ShaderStageFlags::e_FRAGMENT_BIT);
		descSetLayout.setBinding(1, pvrvk::DescriptorType::e_UNIFORM_BUFFER, 1, pvrvk::ShaderStageFlags::e_VERTEX_BIT);

		descriptorSetLayout = device->createDescriptorSetLayout(descSetLayout);

		pvrvk::PipelineLayoutCreateInfo pipelineLayoutInfo;
		pipelineLayoutInfo.setDescSetLayout(0, descriptorSetLayout);

		pvrvk::PushConstantRange pushConstantsRange;
		pushConstantsRange.setOffset(0);
		pushConstantsRange.setSize(static_cast<uint32_t>(pvr::getSize(pvr::GpuDatatypes::Float) * 2));
		pushConstantsRange.setStageFlags(pvrvk::ShaderStageFlags::e_FRAGMENT_BIT);
		pipelineLayoutInfo.setPushConstantRange(0, pushConstantsRange);

		pipelineLayout = device->createPipelineLayout(pipelineLayoutInfo);
	}

	/// <summary>Updates the descriptor sets used for rendering the skybox.</summary>
	/// <param name="device">The device from which the resources will be allocated.</param>
	/// <param name="swapchainIndex">The swapchain index of the descriptor set to update.</param>
	/// <param name="samplerTrilinear">A trilinear sampler object.</param>
	/// <param name="sceneBuffer">The scene buffer.</param>
	/// <param name="sceneBufferView">Buffer view for the scene buffer.</param>
	void updateDescriptorSets(
		pvrvk::Device& device, uint32_t swapchainIndex, pvrvk::Sampler& samplerTrilinear, pvrvk::Buffer& sceneBuffer, pvr::utils::StructuredBufferView& sceneBufferView)
	{
		std::vector<pvrvk::WriteDescriptorSet> writeDescSets;
		writeDescSets.push_back(pvrvk::WriteDescriptorSet(pvrvk::DescriptorType::e_COMBINED_IMAGE_SAMPLER, descriptorSets[swapchainIndex], 0)
									.setImageInfo(0, pvrvk::DescriptorImageInfo(skyBoxImageView, samplerTrilinear, pvrvk::ImageLayout::e_SHADER_READ_ONLY_OPTIMAL)));

		writeDescSets.push_back(
			pvrvk::WriteDescriptorSet(pvrvk::DescriptorType::e_UNIFORM_BUFFER, descriptorSets[swapchainIndex], 1)
				.setBufferInfo(0, pvrvk::DescriptorBufferInfo(sceneBuffer, sceneBufferView.getDynamicSliceOffset(swapchainIndex), sceneBufferView.getDynamicSliceSize())));

		device->updateDescriptorSets(writeDescSets.data(), static_cast<uint32_t>(writeDescSets.size()), nullptr, 0);
	}

	/// <summary>Creates the pipeline.</summary>
	/// <param name="assetProvider">The pvr::IAssetProvider which will be used for loading resources from memory.</param>
	/// <param name="device">The device to use for allocating the pipelines.</param>
	/// <param name="renderpass">The RenderPass to use.</param>
	/// <param name="viewportDimensions">The viewport dimensions.</param>
	/// <param name="pipelineCache">A pipeline cache object to use when creating pipeline.</param>
	void createPipeline(pvr::IAssetProvider& assetProvider, pvrvk::Device& device, const pvrvk::RenderPass& renderpass, const pvrvk::Extent2D& viewportDimensions,
		pvrvk::PipelineCache& pipelineCache)
	{
		pvrvk::GraphicsPipelineCreateInfo pipelineInfo;

		pipelineInfo.viewport.setViewportAndScissor(0,
			pvrvk::Viewport(0.0f, 0.0f, static_cast<float>(viewportDimensions.getWidth()), static_cast<float>(viewportDimensions.getHeight())),
			pvrvk::Rect2D(0, 0, viewportDimensions.getWidth(), viewportDimensions.getHeight()));

		pipelineInfo.rasterizer.setCullMode(pvrvk::CullModeFlags::e_BACK_BIT);

		pipelineInfo.rasterizer.setFrontFaceWinding(pvrvk::FrontFace::e_COUNTER_CLOCKWISE);

		// depth stencil state
		pipelineInfo.depthStencil.enableDepthWrite(false);
		pipelineInfo.depthStencil.enableDepthTest(true);
		pipelineInfo.depthStencil.setDepthCompareFunc(pvrvk::CompareOp::e_LESS_OR_EQUAL);
		pipelineInfo.depthStencil.enableStencilTest(false);

		// blend state
		pipelineInfo.colorBlend.setAttachmentState(0, pvrvk::PipelineColorBlendAttachmentState());
		pipelineInfo.colorBlend.setAttachmentState(1, pvrvk::PipelineColorBlendAttachmentState());

		pipelineInfo.vertexShader.setShader(
			device->createShaderModule(pvrvk::ShaderModuleCreateInfo(assetProvider.getAssetStream(Files::SkyboxVertShaderSrcFile)->readToEnd<uint32_t>())));
		pipelineInfo.fragmentShader.setShader(
			device->createShaderModule(pvrvk::ShaderModuleCreateInfo(assetProvider.getAssetStream(Files::SkyboxFragShaderSrcFile)->readToEnd<uint32_t>())));

		pipelineInfo.vertexInput.clear();
		pipelineInfo.inputAssembler.setPrimitiveTopology(pvrvk::PrimitiveTopology::e_TRIANGLE_LIST);

		pipelineInfo.renderPass = renderpass;

		pipelineInfo.pipelineLayout = pipelineLayout;

		pipeline = device->createGraphicsPipeline(pipelineInfo, pipelineCache);
	}

	/// <summary>Records the secondary command buffers for rendering the skybox.</summary>
	/// <param name="swapchainIndex">The swapchain index for which the recorded command buffer will be used.</param>
	/// <param name="framebuffer">The framebuffer to render into</param>
	/// <param name="exposure">The exposure value used to 'expose' the colour prior to post processing</param>
	/// <param name="threshold">The threshold value used to determine how much of the colour to retain for the bloom</param>
	void recordCommandBuffer(uint32_t swapchainIndex, pvrvk::Framebuffer& framebuffer, float exposure, float threshold)
	{
		cmdBuffers[swapchainIndex]->begin(framebuffer, 0, pvrvk::CommandBufferUsageFlags::e_RENDER_PASS_CONTINUE_BIT);
		pvr::utils::beginCommandBufferDebugLabel(cmdBuffers[swapchainIndex], pvrvk::DebugUtilsLabel("Skybox"));
		cmdBuffers[swapchainIndex]->bindPipeline(pipeline);
		cmdBuffers[swapchainIndex]->bindDescriptorSet(pvrvk::PipelineBindPoint::e_GRAPHICS, pipelineLayout, 0u, descriptorSets[swapchainIndex]);
		cmdBuffers[swapchainIndex]->pushConstants(pipelineLayout, pvrvk::ShaderStageFlags::e_FRAGMENT_BIT, 0, static_cast<uint32_t>(pvr::getSize(pvr::GpuDatatypes::Float)), &exposure);
		cmdBuffers[swapchainIndex]->pushConstants(pipelineLayout, pvrvk::ShaderStageFlags::e_FRAGMENT_BIT, static_cast<uint32_t>(pvr::getSize(pvr::GpuDatatypes::Float)),
			static_cast<uint32_t>(pvr::getSize(pvr::GpuDatatypes::Float)), &threshold);
		cmdBuffers[swapchainIndex]->draw(0, 6);
		pvr::utils::endCommandBufferDebugLabel(cmdBuffers[swapchainIndex]);
		cmdBuffers[swapchainIndex]->end();
	}
};

/// <summary>A Downsample pass which can be used for downsampling images by 1/4 x 1/4 i.e. 1/16 resolution.</summary>
struct DownSamplePass
{
	pvrvk::DescriptorSetLayout descriptorSetLayout;
	pvrvk::PipelineLayout pipelineLayout;
	pvr::Multi<pvrvk::DescriptorSet> descriptorSets;
	pvr::Multi<pvrvk::Framebuffer> framebuffers;
	pvrvk::RenderPass renderPass;
	pvr::Multi<pvrvk::SecondaryCommandBuffer> cmdBuffers;
	pvrvk::GraphicsPipeline pipeline;
	glm::vec4 downsampleConfigs[4];

	/// <summary>Initialises the Downsample pass.</summary>
	/// <param name="assetProvider">The pvr::IAssetProvider which will be used for loading resources from memory.</param>
	/// <param name="device">The device from which the resources will be allocated.</param>
	/// <param name="swapchain">The swapchain which will determine the number of per swapchain resources to allocate.</param>
	/// <param name="commandPool">The command pool from which to allocate command buffers.</param>
	/// <param name="descriptorPool">The descriptor pool from which to allocate descriptor sets.</param>
	/// <param name="blurFramebufferDimensions">The dimensions to use for the downsample pass. These dimensions should be 1/4 x 1/4 the size of the source image.</param>
	/// <param name="inputImageViews">A set of images to downsample.</param>
	/// <param name="outputImageViews">A pre-allocated set of images to render downsampled images to.</param>
	/// <param name="sampler">A bilinear sampler object.</param>
	/// <param name="pipelineCache">A pipeline cache object to use when creating pipelines.</param>
	/// <param name="isComputeDownsample">Determines the destination image layout to use as well as the destination PipelineStageFlags.</param>
	void init(pvr::IAssetProvider& assetProvider, pvrvk::Device& device, pvrvk::Swapchain& swapchain, pvrvk::CommandPool& commandPool, pvrvk::DescriptorPool& descriptorPool,
		const glm::uvec2& blurFramebufferDimensions, pvr::Multi<pvrvk::ImageView>& inputImageViews, pvr::Multi<pvrvk::ImageView>& outputImageViews, pvrvk::Sampler& sampler,
		pvrvk::PipelineCache& pipelineCache, bool isComputeDownsample)
	{
		const glm::vec2 dimensionRatio = glm::vec2(inputImageViews[0]->getImage()->getExtent().getWidth() / outputImageViews[0]->getImage()->getExtent().getWidth(),
			inputImageViews[0]->getImage()->getExtent().getHeight() / outputImageViews[0]->getImage()->getExtent().getHeight());

		// A set of pre-calculated offsets to use for the downsample
		const glm::vec2 offsets[4] = { glm::vec2(-1.0, -1.0), glm::vec2(1.0, -1.0), glm::vec2(-1.0, 1.0), glm::vec2(1.0, 1.0) };
		const glm::vec2 step = glm::vec2(1.0f / (blurFramebufferDimensions.x * dimensionRatio.x), 1.0f / (blurFramebufferDimensions.y * dimensionRatio.y));

		downsampleConfigs[0] = glm::vec4(step * offsets[0], 0.0f, 0.0f);
		downsampleConfigs[1] = glm::vec4(step * offsets[1], 0.0f, 0.0f);
		downsampleConfigs[2] = glm::vec4(step * offsets[2], 0.0f, 0.0f);
		downsampleConfigs[3] = glm::vec4(step * offsets[3], 0.0f, 0.0f);

		createDescriptorSetLayout(device);
		createDescriptorSets(device, swapchain, descriptorPool, inputImageViews, sampler);
		createFramebuffers(device, swapchain, blurFramebufferDimensions, outputImageViews, isComputeDownsample);
		createPipeline(assetProvider, device, blurFramebufferDimensions, pipelineCache);

		for (uint32_t i = 0; i < swapchain->getSwapchainLength(); ++i) { cmdBuffers.add(commandPool->allocateSecondaryCommandBuffer()); }
	}

	/// <summary>Creates the descriptor set layout.</summary>
	/// <param name="device">The device from which the descriptor set layouts will be allocated.</param>
	void createDescriptorSetLayout(pvrvk::Device& device)
	{
		// create the pre bloom descriptor set layout
		pvrvk::DescriptorSetLayoutCreateInfo descSetInfo;
		descSetInfo.setBinding(0, pvrvk::DescriptorType::e_COMBINED_IMAGE_SAMPLER, 1u, pvrvk::ShaderStageFlags::e_FRAGMENT_BIT);

		descriptorSetLayout = device->createDescriptorSetLayout(descSetInfo);

		// create the pipeline layouts
		pvrvk::PipelineLayoutCreateInfo pipeLayoutInfo;

		pipeLayoutInfo.setDescSetLayout(0, descriptorSetLayout);

		uint32_t pushConstantsSize = static_cast<uint32_t>(pvr::getSize(pvr::GpuDatatypes::vec4) * 4);

		pvrvk::PushConstantRange pushConstantsRange;
		pushConstantsRange.setOffset(0);
		pushConstantsRange.setSize(pushConstantsSize);
		pushConstantsRange.setStageFlags(pvrvk::ShaderStageFlags::e_VERTEX_BIT);
		pipeLayoutInfo.setPushConstantRange(0, pushConstantsRange);

		pipelineLayout = device->createPipelineLayout(pipeLayoutInfo);
	}

	/// <summary>Creates the descriptor sets used for the downsample.</summary>
	/// <param name="device">The device from which the resources will be allocated.</param>
	/// <param name="swapchain">The swapchain which will determine the number of per swapchain resources to allocate.</param>
	/// <param name="descriptorPool">The descriptor pool from which to allocate descriptor sets.</param>
	/// <param name="inputImageViews">A set of images to downsample.</param>
	/// <param name="sampler">A bilinear sampler object.</param>
	void createDescriptorSets(
		pvrvk::Device& device, pvrvk::Swapchain& swapchain, pvrvk::DescriptorPool& descriptorPool, pvr::Multi<pvrvk::ImageView>& inputImageViews, pvrvk::Sampler& sampler)
	{
		std::vector<pvrvk::WriteDescriptorSet> writeDescSets;

		for (uint32_t i = 0; i < swapchain->getSwapchainLength(); ++i)
		{
			descriptorSets.add(descriptorPool->allocateDescriptorSet(descriptorSetLayout));

			writeDescSets.push_back(pvrvk::WriteDescriptorSet(pvrvk::DescriptorType::e_COMBINED_IMAGE_SAMPLER, descriptorSets[i], 0)
										.setImageInfo(0, pvrvk::DescriptorImageInfo(inputImageViews[i], sampler, pvrvk::ImageLayout::e_SHADER_READ_ONLY_OPTIMAL)));
		}

		device->updateDescriptorSets(writeDescSets.data(), static_cast<uint32_t>(writeDescSets.size()), nullptr, 0);
	}

	/// <summary>Creates the pipeline.</summary>
	/// <param name="assetProvider">The pvr::IAssetProvider which will be used for loading resources from memory.</param>
	/// <param name="device">The device to use for allocating the pipelines.</param>
	/// <param name="blurFramebufferDimensions">The downsampled framebuffer dimensions.</param>
	/// <param name="pipelineCache">A pipeline cache object to use when creating pipeline.</param>
	void createPipeline(pvr::IAssetProvider& assetProvider, pvrvk::Device& device, const glm::uvec2& blurFramebufferDimensions, pvrvk::PipelineCache& pipelineCache)
	{
		pvrvk::GraphicsPipelineCreateInfo pipelineInfo;
		pipelineInfo.viewport.setViewportAndScissor(0, pvrvk::Viewport(0.0f, 0.0f, static_cast<float>(blurFramebufferDimensions.x), static_cast<float>(blurFramebufferDimensions.y)),
			pvrvk::Rect2D(0, 0, blurFramebufferDimensions.x, blurFramebufferDimensions.y));

		pipelineInfo.rasterizer.setCullMode(pvrvk::CullModeFlags::e_FRONT_BIT);
		pipelineInfo.rasterizer.setFrontFaceWinding(pvrvk::FrontFace::e_COUNTER_CLOCKWISE);

		// disable depth writing and depth testing
		pipelineInfo.depthStencil.enableDepthWrite(false);
		pipelineInfo.depthStencil.enableDepthTest(false);

		// disable stencil testing
		pipelineInfo.depthStencil.enableStencilTest(false);

		pipelineInfo.colorBlend.setAttachmentState(0, pvrvk::PipelineColorBlendAttachmentState());

		// load and create appropriate shaders
		pipelineInfo.vertexShader.setShader(
			device->createShaderModule(pvrvk::ShaderModuleCreateInfo(assetProvider.getAssetStream(Files::Downsample4x4VertSrcFile)->readToEnd<uint32_t>())));
		pipelineInfo.fragmentShader.setShader(
			device->createShaderModule(pvrvk::ShaderModuleCreateInfo(assetProvider.getAssetStream(Files::Downsample4x4FragSrcFile)->readToEnd<uint32_t>())));

		pipelineInfo.vertexInput.clear();
		pipelineInfo.inputAssembler.setPrimitiveTopology(pvrvk::PrimitiveTopology::e_TRIANGLE_STRIP);

		pipelineInfo.pipelineLayout = pipelineLayout;

		// renderpass/subpass
		pipelineInfo.renderPass = renderPass;
		pipelineInfo.subpass = 0;

		pipeline = device->createGraphicsPipeline(pipelineInfo, pipelineCache);
	}

	/// <summary>Allocates the framebuffers used for the downsample.</summary>
	/// <param name="device">The device from which the framebuffers will be allocated.</param>
	/// <param name="swapchain">The swapchain which will determine the number of per swapchain resources to allocate.</param>
	/// <param name="blurFramebufferDimensions">The downsampled framebuffer dimensions.</param>
	/// <param name="colorImageViews">A pre-allocated set of images to render downsampled images to.</param>
	/// <param name="isComputeDownsample">Determines the destination image layout to use as well as the destination PipelineStageFlags.</param>
	void createFramebuffers(
		pvrvk::Device& device, pvrvk::Swapchain& swapchain, const glm::uvec2& blurFramebufferDimensions, pvr::Multi<pvrvk::ImageView>& colorImageViews, bool isComputeDownsample)
	{
		pvrvk::RenderPassCreateInfo renderPassInfo;

		if (isComputeDownsample)
		{
			renderPassInfo.setAttachmentDescription(0,
				pvrvk::AttachmentDescription::createColorDescription(colorImageViews[0]->getImage()->getFormat(), pvrvk::ImageLayout::e_UNDEFINED, pvrvk::ImageLayout::e_GENERAL,
					pvrvk::AttachmentLoadOp::e_DONT_CARE, pvrvk::AttachmentStoreOp::e_STORE, pvrvk::SampleCountFlags::e_1_BIT));
		}
		else
		{
			renderPassInfo.setAttachmentDescription(0,
				pvrvk::AttachmentDescription::createColorDescription(colorImageViews[0]->getImage()->getFormat(), pvrvk::ImageLayout::e_UNDEFINED,
					pvrvk::ImageLayout::e_SHADER_READ_ONLY_OPTIMAL, pvrvk::AttachmentLoadOp::e_DONT_CARE, pvrvk::AttachmentStoreOp::e_STORE, pvrvk::SampleCountFlags::e_1_BIT));
		}

		pvrvk::SubpassDescription subpass;
		subpass.setColorAttachmentReference(0, pvrvk::AttachmentReference(0, pvrvk::ImageLayout::e_COLOR_ATTACHMENT_OPTIMAL));
		renderPassInfo.setSubpass(0, subpass);

		// Add external subpass dependencies to avoid the implicit subpass dependencies
		pvrvk::SubpassDependency externalDependencies[2];
		externalDependencies[0] = pvrvk::SubpassDependency(pvrvk::SubpassExternal, 0, pvrvk::PipelineStageFlags::e_COLOR_ATTACHMENT_OUTPUT_BIT,
			pvrvk::PipelineStageFlags::e_FRAGMENT_SHADER_BIT, pvrvk::AccessFlags::e_COLOR_ATTACHMENT_WRITE_BIT, pvrvk::AccessFlags::e_SHADER_READ_BIT, pvrvk::DependencyFlags::e_NONE);

		if (isComputeDownsample)
		{
			externalDependencies[1] =
				pvrvk::SubpassDependency(0, pvrvk::SubpassExternal, pvrvk::PipelineStageFlags::e_COLOR_ATTACHMENT_OUTPUT_BIT, pvrvk::PipelineStageFlags::e_COMPUTE_SHADER_BIT,
					pvrvk::AccessFlags::e_COLOR_ATTACHMENT_WRITE_BIT, pvrvk::AccessFlags::e_SHADER_READ_BIT, pvrvk::DependencyFlags::e_NONE);
		}
		else
		{
			externalDependencies[1] =
				pvrvk::SubpassDependency(0, pvrvk::SubpassExternal, pvrvk::PipelineStageFlags::e_COLOR_ATTACHMENT_OUTPUT_BIT, pvrvk::PipelineStageFlags::e_FRAGMENT_SHADER_BIT,
					pvrvk::AccessFlags::e_COLOR_ATTACHMENT_WRITE_BIT, pvrvk::AccessFlags::e_SHADER_READ_BIT, pvrvk::DependencyFlags::e_NONE);
		}

		renderPassInfo.addSubpassDependency(externalDependencies[0]);
		renderPassInfo.addSubpassDependency(externalDependencies[1]);

		renderPass = device->createRenderPass(renderPassInfo);

		for (uint32_t i = 0; i < swapchain->getSwapchainLength(); ++i)
		{
			pvrvk::FramebufferCreateInfo createInfo;
			createInfo.setAttachment(0, colorImageViews[i]);
			createInfo.setDimensions(blurFramebufferDimensions.x, blurFramebufferDimensions.y);
			createInfo.setRenderPass(renderPass);

			framebuffers.add(device->createFramebuffer(createInfo));
		}
	}

	/// <summary>Records the commands required for the downsample.</summary>
	/// <param name="swapchainIndex">The swapchain index for which the recorded command buffer will be used.</param>
	void recordCommands(uint32_t swapchainIndex)
	{
		cmdBuffers[swapchainIndex]->begin(framebuffers[swapchainIndex], 0, pvrvk::CommandBufferUsageFlags::e_RENDER_PASS_CONTINUE_BIT);
		pvr::utils::beginCommandBufferDebugLabel(cmdBuffers[swapchainIndex], pvrvk::DebugUtilsLabel("Downsample"));
		cmdBuffers[swapchainIndex]->bindPipeline(pipeline);
		cmdBuffers[swapchainIndex]->bindDescriptorSet(pvrvk::PipelineBindPoint::e_GRAPHICS, pipelineLayout, 0u, descriptorSets[swapchainIndex]);
		cmdBuffers[swapchainIndex]->pushConstants(
			pipelineLayout, pvrvk::ShaderStageFlags::e_VERTEX_BIT, 0, static_cast<uint32_t>(pvr::getSize(pvr::GpuDatatypes::vec4) * 4), &downsampleConfigs);
		cmdBuffers[swapchainIndex]->draw(0, 3);
		pvr::utils::endCommandBufferDebugLabel(cmdBuffers[swapchainIndex]);
		cmdBuffers[swapchainIndex]->end();
	}
};

// Developed by Masaki Kawase, Bunkasha Games
// Used in DOUBLE-S.T.E.A.L. (aka Wreckless)
// From his GDC2003 Presentation: Frame Buffer Post processing Effects in  DOUBLE-S.T.E.A.L (Wreckless)
// Multiple iterations of fixed (per iteration) offset sampling
struct KawaseBlurPass
{
	pvrvk::GraphicsPipeline pipeline;
	pvrvk::DescriptorSetLayout descriptorSetLayout;
	pvrvk::PipelineLayout pipelineLayout;

	// 2 Descriptor sets are created. The descriptor sets are ping-ponged between for each Kawase blur iteration:
	//		iteration 0: (read 0 -> write 1), iteration 1: (read 1 -> write 0), iteration 2: (read 0 -> write 1) etc.
	pvr::Multi<pvrvk::DescriptorSet> descriptorSets[2];

	// Command buffers are recorded individually for each Kawase blur iteration
	pvr::Multi<pvrvk::SecondaryCommandBuffer> cmdBuffers[MaxKawaseIteration];

	// Per iteration fixed size offset
	std::vector<uint32_t> blurKernels;

	// The number of Kawase blur iterations
	uint32_t blurIterations;

	// Push constants used for the per iteration Kawase blur configuration
	glm::vec2 pushConstants[MaxKawaseIteration][4];

	// The per swapchain blurred images.
	// Note that these blurred images are not new images and are instead pointing at one of the per swapchain ping-ponged image sets depending on the number of Kawase blur iterations.
	pvr::Multi<pvrvk::ImageView> blurredImages;

	glm::uvec2 blurFramebufferDimensions;

	/// <summary>Initialises the Kawase blur pass.</summary>
	/// <param name="assetProvider">The pvr::IAssetProvider which will be used for loading resources from memory.</param>
	/// <param name="device">The device from which the resources will be allocated.</param>
	/// <param name="swapchain">The swapchain which will determine the number of per swapchain resources to allocate.</param>
	/// <param name="commandPool">The command pool from which to allocate command buffers.</param>
	/// <param name="descriptorPool">The descriptor pool from which to allocate descriptor sets.</param>
	/// <param name="blurRenderPass">The RenderPasses used for the Kawase blur passes</param>
	/// <param name="blurFramebufferDimensions">The dimensions to use for the Kawase blur iterations.</param>
	/// <param name="imageViews">The ping-ponged image views to use as sources/targets for the Kawase blur passes.</param>
	/// <param name="numImageViews">The number of ping-ponged image views to use as sources/targets for the
	/// Kawase blur passes - this must be 2.</param>
	/// <param name="sampler">The sampler object to use when sampling from the ping-ponged images during the Kawase blur
	/// passes.</param>
	/// <param name="pipelineCache">A pipeline cache object to use when creating pipelines.</param>
	void init(pvr::IAssetProvider& assetProvider, pvrvk::Device& device, pvrvk::Swapchain& swapchain, pvrvk::CommandPool& commandPool, pvrvk::DescriptorPool& descriptorPool,
		pvrvk::RenderPass& blurRenderPass, const glm::uvec2& blurFBODimensions, pvr::Multi<pvrvk::ImageView>* imageViews, uint32_t numImageViews, pvrvk::Sampler& sampler,
		pvrvk::PipelineCache& pipelineCache)
	{
		// The image views provided must be "ping-ponged" i.e. two of them which are swapped (in terms of read/write) each Kawase blur iteration
		// Using more than 2 image views would be inefficient
		assertion(numImageViews == 2);

		this->blurFramebufferDimensions = blurFBODimensions;

		createDescriptorSetLayout(device);

		createPipeline(assetProvider, device, blurRenderPass, blurFBODimensions, pipelineCache);

		// Create the ping-ponged descriptor sets
		createDescriptorSets(device, swapchain, descriptorPool, imageViews, sampler);

		// Pre-allocate all of the potential Kawase blur per swapchain command buffers
		// Recording of commands will take place later
		for (uint32_t i = 0; i < MaxKawaseIteration; ++i)
		{
			for (uint32_t j = 0; j < swapchain->getSwapchainLength(); ++j) { cmdBuffers[i].add(commandPool->allocateSecondaryCommandBuffer()); }
		}
	}

	/// <summary>Returns the blurred image for the given swapchain index.</summary>
	/// <param name="swapchainIndex">The swapchain index of the blurred image to retrieve.</param>
	/// <returns>The blurred image for the specified swapchain index.</returns>
	pvrvk::ImageView& getBlurredImage(uint32_t swapchainIndex) { return blurredImages[swapchainIndex]; }

	/// <summary>Update the Kawase blur configuration.</summary>
	/// <param name="iterationsOffsets">The offsets to use in the Kawase blur passes.</param>
	/// <param name="numIterations">The number of iterations of Kawase blur passes.</param>
	/// <param name="imageViews">The set of ping-pong images.</param>
	/// <param name="numImageViews">The number of ping-pong images.</param>
	void updateConfig(uint32_t* iterationsOffsets, uint32_t numIterations, pvr::Multi<pvrvk::ImageView>* imageViews, uint32_t numImageViews)
	{
		// reset/clear the kernels and number of iterations
		blurKernels.clear();
		blurIterations = 0;

		// calculate texture sample offsets based on the number of iterations and the kernel offset currently in use for the given iteration
		glm::vec2 pixelSize = glm::vec2(1.0f / blurFramebufferDimensions.x, 1.0f / blurFramebufferDimensions.y);

		glm::vec2 halfPixelSize = pixelSize / 2.0f;

		for (uint32_t i = 0; i < numIterations; ++i)
		{
			blurKernels.push_back(iterationsOffsets[i]);

			glm::vec2 dUV = pixelSize * glm::vec2(blurKernels[i], blurKernels[i]) + halfPixelSize;

			pushConstants[i][0] = glm::vec2(-dUV.x, dUV.y);
			pushConstants[i][1] = glm::vec2(dUV);
			pushConstants[i][2] = glm::vec2(dUV.x, -dUV.y);
			pushConstants[i][3] = glm::vec2(-dUV.x, -dUV.y);
		}
		blurIterations = numIterations;
		assertion(blurIterations <= MaxKawaseIteration);

		assertion(numImageViews == 2);

		blurredImages = imageViews[numIterations % 2];
	}

	/// <summary>Creates the Kawase blur descriptor set layout.</summary>
	/// <param name="device">The device to use for allocating the descriptor set layout.</param>
	void createDescriptorSetLayout(pvrvk::Device& device)
	{
		// Create the descriptor set layout
		pvrvk::DescriptorSetLayoutCreateInfo descSetInfo;
		descSetInfo.setBinding(0, pvrvk::DescriptorType::e_COMBINED_IMAGE_SAMPLER, 1u, pvrvk::ShaderStageFlags::e_FRAGMENT_BIT);
		descriptorSetLayout = device->createDescriptorSetLayout(descSetInfo);

		// Push constants are used for uploading the texture sample offsets
		pvrvk::PushConstantRange pushConstantsRange;
		pushConstantsRange.setOffset(0);
		pushConstantsRange.setSize(static_cast<uint32_t>(pvr::getSize(pvr::GpuDatatypes::vec2) * 4));
		pushConstantsRange.setStageFlags(pvrvk::ShaderStageFlags::e_VERTEX_BIT);

		// Create the pipeline layout
		pvrvk::PipelineLayoutCreateInfo pipeLayoutInfo;
		pipeLayoutInfo.setDescSetLayout(0, descriptorSetLayout);
		pipeLayoutInfo.setPushConstantRange(0, pushConstantsRange);
		pipelineLayout = device->createPipelineLayout(pipeLayoutInfo);
	}

	/// <summary>Creates the Kawase blur descriptor sets.</summary>
	/// <param name="device">The device to use for allocating the descriptor sets.</param>
	/// <param name="swapchain">The swapchain which will determine the number of per swapchain resources to allocate.</param>
	/// <param name="descriptorPool">The descriptor pool from which to allocate descriptor sets.</param>
	/// <param name="imageViews">The ping-ponged images to use for reading.</param>
	/// <param name="sampler">The sampler to use.</param>
	void createDescriptorSets(pvrvk::Device& device, pvrvk::Swapchain& swapchain, pvrvk::DescriptorPool& descriptorPool, pvr::Multi<pvrvk::ImageView>* imageViews, pvrvk::Sampler& sampler)
	{
		std::vector<pvrvk::WriteDescriptorSet> writeDescSets;
		for (uint32_t i = 0; i < swapchain->getSwapchainLength(); ++i)
		{
			// The number of ping-pong images is fixed at 2
			for (uint32_t j = 0; j < 2; ++j)
			{
				descriptorSets[j].add(descriptorPool->allocateDescriptorSet(descriptorSetLayout));

				writeDescSets.push_back(pvrvk::WriteDescriptorSet(pvrvk::DescriptorType::e_COMBINED_IMAGE_SAMPLER, descriptorSets[j][i], 0)
											.setImageInfo(0, pvrvk::DescriptorImageInfo(imageViews[j][i], sampler, pvrvk::ImageLayout::e_SHADER_READ_ONLY_OPTIMAL)));
			}
		}

		device->updateDescriptorSets(writeDescSets.data(), static_cast<uint32_t>(writeDescSets.size()), nullptr, 0);
	}

	/// <summary>Creates the Kawase pipeline.</summary>
	/// <param name="assetProvider">The pvr::IAssetProvider which will be used for loading resources from memory.</param>
	/// <param name="device">The device to use for allocating the pipelines.</param>
	/// <param name="blurRenderPass">The RenderPass to use for the Kawase blur iterations.</param>
	/// <param name="blurFramebufferDimensions">The dimensions of the Framebuffers used in the Kawase blur iterations.</param>
	/// <param name="pipelineCache">A pipeline cache object to use when creating pipelines.</param>
	void createPipeline(
		pvr::IAssetProvider& assetProvider, pvrvk::Device& device, pvrvk::RenderPass& blurRenderPass, const glm::uvec2& blurFBODimensions, pvrvk::PipelineCache& pipelineCache)
	{
		pvrvk::GraphicsPipelineCreateInfo pipelineInfo;
		pipelineInfo.viewport.setViewportAndScissor(0, pvrvk::Viewport(0.0f, 0.0f, static_cast<float>(blurFBODimensions.x), static_cast<float>(blurFBODimensions.y)),
			pvrvk::Rect2D(0, 0, blurFBODimensions.x, blurFBODimensions.y));

		pipelineInfo.rasterizer.setCullMode(pvrvk::CullModeFlags::e_FRONT_BIT);
		pipelineInfo.rasterizer.setFrontFaceWinding(pvrvk::FrontFace::e_COUNTER_CLOCKWISE);

		// Disable depth writing and depth testing
		pipelineInfo.depthStencil.enableDepthWrite(false);
		pipelineInfo.depthStencil.enableDepthTest(false);

		// Disable stencil testing
		pipelineInfo.depthStencil.enableStencilTest(false);

		pipelineInfo.colorBlend.setAttachmentState(0, pvrvk::PipelineColorBlendAttachmentState());

		// Load and create the Kawase blur shader modules
		pipelineInfo.vertexShader.setShader(device->createShaderModule(pvrvk::ShaderModuleCreateInfo(assetProvider.getAssetStream(Files::KawaseVertSrcFile)->readToEnd<uint32_t>())));
		pipelineInfo.fragmentShader.setShader(device->createShaderModule(pvrvk::ShaderModuleCreateInfo(assetProvider.getAssetStream(Files::KawaseFragSrcFile)->readToEnd<uint32_t>())));

		// We use attribute-less rendering
		pipelineInfo.vertexInput.clear();
		pipelineInfo.inputAssembler.setPrimitiveTopology(pvrvk::PrimitiveTopology::e_TRIANGLE_STRIP);

		pipelineInfo.pipelineLayout = pipelineLayout;
		pipelineInfo.renderPass = blurRenderPass;
		pipelineInfo.subpass = 0;

		pipeline = device->createGraphicsPipeline(pipelineInfo, pipelineCache);
	}

	/// <summary>Records the commands required for the Kawase blur iterations based on the current configuration.</summary>
	/// <param name="swapchainIndex">The swapchain index for which the recorded command buffer will be used.</param>
	/// <param name="blurFramebuffers">The ping-ponged Framebuffers to use in the Kawase blur iterations.</param>
	void recordCommands(uint32_t swapchainIndex, pvr::Multi<pvrvk::Framebuffer>* blurFramebuffers, uint32_t numFramebuffers)
	{
		// Iterate through the Kawase blur iterations
		for (uint32_t i = 0; i < blurIterations; ++i)
		{
			// Calculate the ping pong index based on the current iteration
			uint32_t pingPongIndex = i % numFramebuffers;

			cmdBuffers[i][swapchainIndex]->begin(blurFramebuffers[pingPongIndex][swapchainIndex], 0, pvrvk::CommandBufferUsageFlags::e_RENDER_PASS_CONTINUE_BIT);
			pvr::utils::beginCommandBufferDebugLabel(
				cmdBuffers[i][swapchainIndex], pvrvk::DebugUtilsLabel(pvr::strings::createFormatted("Kawase Blur - swapchain (%i): %i", swapchainIndex, i)));
			cmdBuffers[i][swapchainIndex]->pushConstants(
				pipelineLayout, pvrvk::ShaderStageFlags::e_VERTEX_BIT, 0, static_cast<uint32_t>(pvr::getSize(pvr::GpuDatatypes::vec2) * 4), &pushConstants[i]);
			cmdBuffers[i][swapchainIndex]->bindPipeline(pipeline);
			cmdBuffers[i][swapchainIndex]->bindDescriptorSet(pvrvk::PipelineBindPoint::e_GRAPHICS, pipelineLayout, 0u, descriptorSets[pingPongIndex][swapchainIndex]);
			cmdBuffers[i][swapchainIndex]->draw(0, 3);
			pvr::utils::endCommandBufferDebugLabel(cmdBuffers[i][swapchainIndex]);
			cmdBuffers[i][swapchainIndex]->end();
		}
	}

	/// <summary>Records the command buffers into the given main rendering command buffer.</summary>
	/// <param name="swapchainIndex">The swapchain index for which the recorded command buffer will be used.</param>
	/// <param name="cmdBuffers">The main command buffer into which the pre-recorded command buffers will be recorded into.</param>
	/// <param name="queue">The queue to which the command buffer will be submitted.</param>
	/// <param name="blurRenderPass">The RenderPass to use.</param>
	/// <param name="blurFramebuffers">The ping-ponged Framebuffers to use in the Kawase blur iterations.</param>
	void recordCommandsToMainCommandBuffer(
		uint32_t swapchainIndex, pvrvk::CommandBuffer& cmdBuffer, pvrvk::Queue& queue, pvrvk::RenderPass& blurRenderPass, pvr::Multi<pvrvk::Framebuffer>* blurFramebuffers)
	{
		(void)queue;
		pvrvk::ClearValue clearValue = pvrvk::ClearValue(0.0f, 0.0f, 0.0f, 1.f);

		// Iterate through the current set of Kawase blur iterations
		for (uint32_t i = 0; i < blurIterations; ++i)
		{
			uint32_t pingPongIndex = i % 2;

			cmdBuffer->beginRenderPass(blurFramebuffers[pingPongIndex][swapchainIndex], blurRenderPass,
				pvrvk::Rect2D(0, 0, blurFramebuffers[pingPongIndex][swapchainIndex]->getDimensions().getWidth(),
					blurFramebuffers[pingPongIndex][swapchainIndex]->getDimensions().getHeight()),
				false, &clearValue, 1);
			cmdBuffer->executeCommands(cmdBuffers[i][swapchainIndex]);
			cmdBuffer->endRenderPass();
		}
	}
};

// Developed by Marius Bjorge (ARM)
// Bandwidth-Efficient Rendering - siggraph2015-mmg-marius
// Filters images whilst Downsampling and Upsampling
struct DualFilterBlurPass
{
	// We only need (MaxFilterIterations - 1) images as the first image is an input to the blur pass
	// We also special case the final pass as this requires either a different pipeline or a different descriptor set/layout

	// Special cased final pass pipeline where the final upsample pass and compositing occurs in the same pipeline. This lets us avoid an extra write to memory/read from memory pass
	pvrvk::GraphicsPipeline finalPassPipeline;
	pvrvk::GraphicsPipeline finalPassBloomOnlyPipeline;

	// Pre allocated Up and Down sample pipelines for the iterations up to MaxFilterIterations
	pvrvk::GraphicsPipeline pipelines[MaxFilterIterations - 1];

	// The current set of pipelines in use for the currently selected configuration
	pvrvk::GraphicsPipeline currentPipelines[MaxFilterIterations];

	// The special cased final pass descriptor set layout
	pvrvk::DescriptorSetLayout finalPassDescriptorSetLayout;

	// The descriptor set layout used during the up and down sample passes
	pvrvk::DescriptorSetLayout descriptorSetLayout;

	// The special cased final pass pipeline layout
	pvrvk::PipelineLayout finalPassPipelineLayout;

	// The pipeline layout used during the up and down sample passes
	pvrvk::PipelineLayout pipelineLayout;

	// The special cased final pass per swapchain descriptor sets
	pvr::Multi<pvrvk::DescriptorSet> finalPassDescriptorSets;

	// The per swapchain descriptor sets for the up and down sample passes
	pvr::Multi<pvrvk::DescriptorSet> descriptorSets[MaxFilterIterations - 1];

	// The pre allocated framebuffers for the iterations up to MaxFilterIterations
	pvr::Multi<pvrvk::Framebuffer> framebuffers[MaxFilterIterations - 1];

	// The current set of framebuffers in use for the currently selected configuration
	pvr::Multi<pvrvk::Framebuffer> currentFramebuffers[MaxFilterIterations - 1];

	// The pre allocated image views for the iterations up to MaxFilterIterations
	pvr::Multi<pvrvk::ImageView> imageViews[MaxFilterIterations - 1];

	// The current set of image views in use for the currently selected configuration
	pvr::Multi<pvrvk::ImageView> currentImageViews[MaxFilterIterations - 1];

	// The set of command buffers where commands will be recorded for the current configuration
	pvr::Multi<pvrvk::SecondaryCommandBuffer> cmdBuffers[MaxFilterIterations];

	// The framebuffer dimensions for the current configuration
	std::vector<glm::vec2> currentIterationDimensions;

	// The framebuffer inverse dimensions for the current configuration
	std::vector<glm::vec2> currentIterationInverseDimensions;

	// The full set of framebuffer dimensions
	std::vector<glm::vec2> maxIterationDimensions;

	// The full set of framebuffer inverse dimensions
	std::vector<glm::vec2> maxIterationInverseDimensions;

	// The number of Dual Filter iterations currently in use
	uint32_t blurIterations;

	// The current set of push constants for the current configuration
	glm::vec2 pushConstants[MaxFilterIterations][8];

	// The final full resolution framebuffer dimensions
	glm::uvec2 framebufferDimensions;

	// The colour image format in use
	pvrvk::Format colorImageFormat;

	// The source images to blur
	pvr::Multi<pvrvk::ImageView> sourceImageViews;
	// The offscreen images used to compose the final image
	pvr::Multi<pvrvk::ImageView> offscreenImageViews;

	pvrvk::Sampler sampler;

	/// <summary>Initialises the Dual Filter blur.</summary>
	/// <param name="assetProvider">The pvr::IAssetProvider which will be used for loading resources from memory.</param>
	/// <param name="device">The device from which the resources will be allocated.</param>
	/// <param name="swapchain">The swapchain which will determine the number of per swapchain resources to allocate.</param>
	/// <param name="commandPool">The command pool from which to allocate command buffers.</param>
	/// <param name="descriptorPool">The descriptor pool from which to allocate descriptor sets.</param>
	/// <param name="blurRenderPass">The RenderPass to use.</param>
	/// <param name="offscreenImageViews">A list of offscreen images (one per swapchain) which will be used in the final image composition pass.</param>
	/// <param name="sourceImageViews">A list of source images (one per swapchain) which will be used as the source for the blur.</param>
	/// <param name="vmaAllocator">A VMA allocator to use for allocating images and buffers.</param>
	/// <param name="colorImageFormat">The colour image format to use for the Dual Filter blur.</param>
	/// <param name="framebufferDimensions_">The full size resolution framebuffer dimensions.</param>
	/// <param name="sampler">The sampler object to use when sampling from the images during the Dual Filter blur passes.</param>
	/// <param name="onScreenRenderPass">The main RenderPass used for rendering to the screen.</param>
	/// <param name="pipelineCache">A pipeline cache object to use when creating pipelines.</param>
	/// <param name="sharedImageViews">A set of shared image views which are available for use rather than allocating identical images.</param>
	void init(pvr::IAssetProvider& assetProvider, pvrvk::Device& device, pvrvk::Swapchain& swapchain, pvrvk::CommandPool& commandPool, pvrvk::DescriptorPool& descriptorPool,
		pvrvk::RenderPass& blurRenderPass, pvr::Multi<pvrvk::ImageView>& offscreenImageViews_, pvr::Multi<pvrvk::ImageView>& sourceImageViews_,
		pvr::utils::vma::Allocator& vmaAllocator, pvrvk::Format colorImageFormat_, const glm::uvec2& framebufferDimensions_, pvrvk::Sampler& sampler_,
		pvrvk::RenderPass& onScreenRenderPass, pvrvk::PipelineCache& pipelineCache, std::vector<pvr::Multi<pvrvk::ImageView>>& sharedImageViews)
	{
		this->colorImageFormat = colorImageFormat_;
		this->framebufferDimensions = framebufferDimensions_;
		this->blurIterations = -1;
		this->sampler = sampler_;
		this->offscreenImageViews = offscreenImageViews_;
		this->sourceImageViews = sourceImageViews_;

		createDescriptorSetLayouts(device);
		createDescriptorSets(swapchain, descriptorPool);

		// Calculate the maximum set of per iteration framebuffer dimensions
		// The maximum set will start from framebufferDimensions and allow for MaxFilterIterations. Note that this includes down and up sample passes
		calculateIterationDimensions();

		// Allocates the images used for each of the down/up sample passes
		allocatePingPongImages(device, swapchain, vmaAllocator, sharedImageViews);

		// Create the dual filter framebuffers
		createFramebuffers(device, swapchain, blurRenderPass);

		// Create the up and down sample pipelines
		createPipelines(assetProvider, device, blurRenderPass, onScreenRenderPass, pipelineCache);

		for (uint32_t i = 0; i < MaxFilterIterations; ++i)
		{
			for (uint32_t j = 0; j < swapchain->getSwapchainLength(); ++j) { cmdBuffers[i].add(commandPool->allocateSecondaryCommandBuffer()); }
		}
	}

	/// <summary>Returns the blurred image for the given swapchain index.</summary>
	/// <param name="swapchainIndex">The swapchain index of the blurred image to retrieve.</param>
	/// <returns>The blurred image for the specified swapchain index.</returns>
	pvrvk::ImageView& getBlurredImage(uint32_t swapchainIndex) { return currentImageViews[blurIterations - 1][swapchainIndex]; }

	/// <summary>Update the Dual Filter blur configuration.</summary>
	/// <param name="numIterations">The number of iterations of Kawase blur passes.</param>
	/// <param name="initial">Specifies whether the Dual Filter pass is being initialised.</param>
	void updateConfig(uint32_t numIterations, bool initial = false)
	{
		// We only update the Dual Filter configuration if the number of iterations has actually been modified
		if (numIterations != blurIterations || initial)
		{
			blurIterations = numIterations;

			assertion(blurIterations % 2 == 0);

			// Calculate the Dual Filter iteration dimensions based on the current Dual Filter configuration
			getIterationDimensions(currentIterationDimensions, currentIterationInverseDimensions, blurIterations);

			// Configure the Dual Filter push constant values based on the current Dual Filter configuration
			configurePushConstants();

			// Configure the set of Dual Filter ping pong images based on the current Dual Filter configuration
			configurePingPongImages();

			// Configure the set of Framebuffers based on the current Dual Filter configuration
			configureFramebuffers();

			// Configure the set of up/down samples pipelines based on the current Dual Filter configuration
			configurePipelines();
		}
	}

	/// <summary>Configure the set of up/down samples pipelines based on the current Dual Filter configuration.</summary>
	void configurePipelines()
	{
		uint32_t index = 0;
		for (; index < blurIterations / 2; ++index) { currentPipelines[index] = pipelines[index]; }

		for (uint32_t i = MaxFilterIterations - (blurIterations / 2); i < MaxFilterIterations - 1; ++i)
		{
			currentPipelines[index] = pipelines[i];
			index++;
		}
	}

	/// <summary>Configure the set of Framebuffers based on the current Dual Filter configuration.</summary>
	void configureFramebuffers()
	{
		for (uint32_t i = 0; i < MaxFilterIterations - 1; ++i) { currentFramebuffers[i].clear(); }

		uint32_t index = 0;
		for (; index < blurIterations / 2; ++index) { currentFramebuffers[index] = framebuffers[index]; }

		for (uint32_t i = MaxFilterIterations - (blurIterations / 2); i < MaxFilterIterations - 1; ++i)
		{
			currentFramebuffers[index] = framebuffers[i];
			index++;
		}
	}

	/// <summary>Configure the set of Dual Filter ping pong images based on the current Dual Filter configuration.</summary>
	virtual void configurePingPongImages()
	{
		for (uint32_t i = 0; i < MaxFilterIterations - 1; ++i) { currentImageViews[i].clear(); }

		uint32_t index = 0;
		for (; index < blurIterations / 2; ++index) { currentImageViews[index] = imageViews[index]; }

		for (uint32_t i = MaxFilterIterations - (blurIterations / 2); i < MaxFilterIterations - 1; ++i)
		{
			currentImageViews[index] = imageViews[i];
			index++;
		}
	}

	/// <summary>Calculate the full set of Dual Filter iteration dimensions.</summary>
	void calculateIterationDimensions()
	{
		maxIterationDimensions.resize(MaxFilterIterations);
		maxIterationInverseDimensions.resize(MaxFilterIterations);

		// Determine the dimensions and inverse dimensions for each iteration of the Dual Filter
		// If the original texture size is 800x600 and we are using a 4 pass Dual Filter then:
		//		Iteration 0: 400x300
		//		Iteration 1: 200x150
		//		Iteration 2: 400x300
		//		Iteration 3: 800x600
		glm::uvec2 dimension = glm::uvec2(framebufferDimensions.x, framebufferDimensions.y);

		// Calculate the dimensions and inverse dimensions top down
		for (uint32_t i = 0; i < MaxFilterIterations / 2; ++i)
		{
			dimension = glm::uvec2(glm::ceil(dimension.x / 2.0f), glm::ceil(dimension.y / 2.0f));
			maxIterationDimensions[i] = dimension;
			glm::vec2 inverseDimensions = glm::vec2(1.0f / dimension.x, 1.0f / dimension.y);
			maxIterationInverseDimensions[i] = inverseDimensions;
		}

		dimension = glm::uvec2(framebufferDimensions.x, framebufferDimensions.y);

		for (uint32_t i = MaxFilterIterations - 1; i >= MaxFilterIterations / 2; --i)
		{
			maxIterationDimensions[i] = dimension;
			glm::vec2 inverseDimensions = glm::vec2(1.0f / dimension.x, 1.0f / dimension.y);
			maxIterationInverseDimensions[i] = inverseDimensions;
			dimension = glm::uvec2(glm::ceil(dimension.x / 2.0f), glm::ceil(dimension.y / 2.0f));
		}
	}

	/// <summary>Calculate the Dual Filter iteration dimensions based on the current Dual Filter configuration.</summary>
	/// <param name="iterationDimensions">A returned list of dimensions to use for the current configuration.</param>
	/// <param name="iterationInverseDimensions">A returned list of inverse dimensions to use for the current configuration.</param>
	/// <param name="numIterations">Specifies the number of iterations used in the current configuration.</param>
	void getIterationDimensions(std::vector<glm::vec2>& iterationDimensions, std::vector<glm::vec2>& iterationInverseDimensions, uint32_t numIterations)
	{
		// Determine the dimensions and inverse dimensions for each iteration of the Dual Filter
		// If the original texture size is 800x600 and we are using a 4 pass Dual Filter then:
		//		Iteration 0: 400x300
		//		Iteration 1: 200x150
		//		Iteration 2: 400x300
		//		Iteration 3: 800x600
		iterationDimensions.clear();
		iterationInverseDimensions.clear();

		for (uint32_t i = 0; i < numIterations / 2; ++i)
		{
			iterationDimensions.push_back(maxIterationDimensions[i]);
			iterationInverseDimensions.push_back(maxIterationInverseDimensions[i]);
		}

		uint32_t index = MaxFilterIterations - (numIterations / 2);
		for (uint32_t i = numIterations / 2; i < numIterations; ++i)
		{
			iterationDimensions.push_back(maxIterationDimensions[index]);
			iterationInverseDimensions.push_back(maxIterationInverseDimensions[index]);
			index++;
		}
	}

	/// <summary>Allocates the images used for each of the down/up sample passes.</summary>
	/// <param name="device">The device from which the resources will be allocated.</param>
	/// <param name="swapchain">The swapchain which will determine the number of per swapchain resources to allocate.</param>
	/// <param name="vmaAllocator">A VMA allocator to use for allocating images and buffers.</param>
	/// <param name="sharedImageViews">A set of shared image views which are available for use rather than allocating identical images.</param>
	virtual void allocatePingPongImages(
		pvrvk::Device& device, pvrvk::Swapchain& swapchain, pvr::utils::vma::Allocator& vmaAllocator, std::vector<pvr::Multi<pvrvk::ImageView>>& sharedImageViews)
	{
		pvrvk::ImageUsageFlags imageUsage = pvrvk::ImageUsageFlags::e_COLOR_ATTACHMENT_BIT | pvrvk::ImageUsageFlags::e_SAMPLED_BIT;

		auto localSharedImageViews = sharedImageViews;

		for (uint32_t i = 0; i < swapchain->getSwapchainLength(); ++i)
		{
			for (uint32_t j = 0; j < MaxFilterIterations / 2; ++j)
			{
				pvrvk::Extent3D extent =
					pvrvk::Extent3D(static_cast<uint32_t>(maxIterationDimensions[j].x), static_cast<uint32_t>(maxIterationDimensions[j].y), static_cast<uint32_t>(1.0f));

				// Attempt to find a compatible image to cut down on the maximum number of images allocated
				bool foundCompatible = false;
				uint32_t compatibleImageIndex = -1;
				for (uint32_t k = 0; k < localSharedImageViews.size(); ++k)
				{
					if (isCompatibleImageView(localSharedImageViews[k][i], pvrvk::ImageType::e_2D, colorImageFormat, extent, imageUsage, pvrvk::ImageLayersSize(),
							pvrvk::SampleCountFlags::e_1_BIT, pvrvk::MemoryPropertyFlags::e_DEVICE_LOCAL_BIT, pvrvk::SharingMode::e_EXCLUSIVE, pvrvk::ImageTiling::e_OPTIMAL, nullptr, 0))
					{
						foundCompatible = true;
						compatibleImageIndex = k;
						break;
					}
				}
				if (foundCompatible)
				{
					// add and remove from the back
					imageViews[j].add(sharedImageViews[compatibleImageIndex][(uint32_t)sharedImageViews[compatibleImageIndex].size() - 1 - i]);
					localSharedImageViews[compatibleImageIndex].resize((uint32_t)localSharedImageViews[compatibleImageIndex].size() - 1);
				}
				else
				{
					pvrvk::Image blurColorTexture = pvr::utils::createImage(device, pvrvk::ImageCreateInfo(pvrvk::ImageType::e_2D, colorImageFormat, extent, imageUsage),
						pvrvk::MemoryPropertyFlags::e_DEVICE_LOCAL_BIT, pvrvk::MemoryPropertyFlags::e_NONE, vmaAllocator);

					imageViews[j].add(device->createImageView(pvrvk::ImageViewCreateInfo(blurColorTexture)));
					addImageToSharedImages(sharedImageViews, imageViews[j].back(), i);
				}
			}

			// We're able to reuse images between up/down sample passes. This can help us keep down the total number of images in flight
			uint32_t k = 0;
			for (uint32_t j = MaxFilterIterations / 2; j < MaxFilterIterations - 1; ++j)
			{
				uint32_t reuseIndex = (MaxFilterIterations / 2) - 1 - (k + 1);

				imageViews[j].add(imageViews[reuseIndex][i]);
				k++;
			}
		}
	}

	/// <summary>Allocates the framebuffers used for each of the down/up sample passes.</summary>
	/// <param name="device">The device from which the framebuffers will be allocated.</param>
	/// <param name="swapchain">The swapchain which will determine the number of per swapchain resources to allocate.</param>
	/// <param name="blurRenderPass">The RenderPass to use.</param>
	void createFramebuffers(pvrvk::Device& device, pvrvk::Swapchain& swapchain, pvrvk::RenderPass& blurRenderPass)
	{
		for (uint32_t i = 0; i < swapchain->getSwapchainLength(); ++i)
		{
			pvrvk::FramebufferCreateInfo createInfo;
			createInfo.setRenderPass(blurRenderPass);

			for (uint32_t j = 0; j < MaxFilterIterations - 1; ++j)
			{
				createInfo.setDimensions(static_cast<uint32_t>(maxIterationDimensions[j].x), static_cast<uint32_t>(maxIterationDimensions[j].y));
				createInfo.setAttachment(0, imageViews[j][i]);
				framebuffers[j].add(device->createFramebuffer(createInfo));
			}
		}
	}

	/// <summary>Creates the descriptor set layouts used for the various up/down sample passes.</summary>
	/// <param name="device">The device from which the descriptor set layouts will be allocated.</param>
	virtual void createDescriptorSetLayouts(pvrvk::Device& device)
	{
		pvrvk::DescriptorSetLayoutCreateInfo descSetInfo;
		descSetInfo.setBinding(0, pvrvk::DescriptorType::e_COMBINED_IMAGE_SAMPLER, 1u, pvrvk::ShaderStageFlags::e_FRAGMENT_BIT);
		descriptorSetLayout = device->createDescriptorSetLayout(descSetInfo);

		// The final pass uses a number of extra resources compared to the other passes
		descSetInfo.setBinding(1, pvrvk::DescriptorType::e_COMBINED_IMAGE_SAMPLER, 1u, pvrvk::ShaderStageFlags::e_FRAGMENT_BIT);
		descSetInfo.setBinding(2, pvrvk::DescriptorType::e_UNIFORM_BUFFER, 1u, pvrvk::ShaderStageFlags::e_FRAGMENT_BIT);

		finalPassDescriptorSetLayout = device->createDescriptorSetLayout(descSetInfo);

		pvrvk::PipelineLayoutCreateInfo pipeLayoutInfo;

		// Push constants are used for uploading the texture sample offsets
		pvrvk::PushConstantRange pushConstantsRanges;
		pushConstantsRanges.setOffset(0);
		pushConstantsRanges.setSize(static_cast<uint32_t>(pvr::getSize(pvr::GpuDatatypes::vec2) * 8 + pvr::getSize(pvr::GpuDatatypes::Float)));
		pushConstantsRanges.setStageFlags(pvrvk::ShaderStageFlags::e_VERTEX_BIT | pvrvk::ShaderStageFlags::e_FRAGMENT_BIT);

		pipeLayoutInfo.setDescSetLayout(0, descriptorSetLayout);
		pipeLayoutInfo.setPushConstantRange(0, pushConstantsRanges);

		pipelineLayout = device->createPipelineLayout(pipeLayoutInfo);

		pipeLayoutInfo.setDescSetLayout(0, finalPassDescriptorSetLayout);
		finalPassPipelineLayout = device->createPipelineLayout(pipeLayoutInfo);
	}

	/// <summary>Creates the descriptor sets used for up/down sample passes.</summary>
	/// <param name="swapchain">The swapchain which will determine the number of per swapchain resources to allocate.</param>
	/// <param name="descriptorPool">The descriptor pool from which to allocate descriptor sets.</param>
	virtual void createDescriptorSets(pvrvk::Swapchain& swapchain, pvrvk::DescriptorPool& descriptorPool)
	{
		for (uint32_t i = 0; i < swapchain->getSwapchainLength(); ++i)
		{
			for (uint32_t j = 0; j < MaxFilterIterations - 1; ++j) { descriptorSets[j].add(descriptorPool->allocateDescriptorSet(descriptorSetLayout)); }
			finalPassDescriptorSets.add(descriptorPool->allocateDescriptorSet(finalPassDescriptorSetLayout));
		}
	}

	/// <summary>Updates the descriptor sets used for each of the down/up sample passes.</summary>
	/// <param name="device">The device from which the updateDescriptorSets call will be piped.</param>
	/// <param name="swapchainIndex">The swapchain index of the descriptor set to update.</param>
	virtual void updateDescriptorSets(pvrvk::Device& device, uint32_t swapchainIndex)
	{
		// First pass
		std::vector<pvrvk::WriteDescriptorSet> writeDescSets;
		writeDescSets.push_back(pvrvk::WriteDescriptorSet(pvrvk::DescriptorType::e_COMBINED_IMAGE_SAMPLER, descriptorSets[0][swapchainIndex], 0)
									.setImageInfo(0, pvrvk::DescriptorImageInfo(sourceImageViews[swapchainIndex], sampler, pvrvk::ImageLayout::e_SHADER_READ_ONLY_OPTIMAL)));

		// Down/up sample passes
		for (uint32_t j = 1; j < blurIterations - 1; ++j)
		{
			writeDescSets.push_back(pvrvk::WriteDescriptorSet(pvrvk::DescriptorType::e_COMBINED_IMAGE_SAMPLER, descriptorSets[j][swapchainIndex], 0)
										.setImageInfo(0, pvrvk::DescriptorImageInfo(currentImageViews[j - 1][swapchainIndex], sampler, pvrvk::ImageLayout::e_SHADER_READ_ONLY_OPTIMAL)));
		}

		// Final pass
		writeDescSets.push_back(
			pvrvk::WriteDescriptorSet(pvrvk::DescriptorType::e_COMBINED_IMAGE_SAMPLER, finalPassDescriptorSets[swapchainIndex], 0)
				.setImageInfo(0, pvrvk::DescriptorImageInfo(currentImageViews[blurIterations - 2][swapchainIndex], sampler, pvrvk::ImageLayout::e_SHADER_READ_ONLY_OPTIMAL)));

		writeDescSets.push_back(pvrvk::WriteDescriptorSet(pvrvk::DescriptorType::e_COMBINED_IMAGE_SAMPLER, finalPassDescriptorSets[swapchainIndex], 1)
									.setImageInfo(0, pvrvk::DescriptorImageInfo(offscreenImageViews[swapchainIndex], sampler, pvrvk::ImageLayout::e_SHADER_READ_ONLY_OPTIMAL)));

		device->updateDescriptorSets(writeDescSets.data(), static_cast<uint32_t>(writeDescSets.size()), nullptr, 0);
	}

	/// <summary>Creates the pipelines.</summary>
	/// <param name="assetProvider">The pvr::IAssetProvider which will be used for loading resources from memory.</param>
	/// <param name="device">The device to use for allocating the pipelines.</param>
	/// <param name="blurRenderPass">The RenderPass to use.</param>
	/// <param name="onScreenRenderPass">The on screen RenderPass used for the final Dual Filter pass.</param>
	/// <param name="pipelineCache">A pipeline cache object to use when creating pipelines.</param>
	virtual void createPipelines(
		pvr::IAssetProvider& assetProvider, pvrvk::Device& device, pvrvk::RenderPass& blurRenderPass, pvrvk::RenderPass& onScreenRenderPass, pvrvk::PipelineCache& pipelineCache)
	{
		pvrvk::GraphicsPipelineCreateInfo pipelineInfo;
		pipelineInfo.rasterizer.setCullMode(pvrvk::CullModeFlags::e_FRONT_BIT);
		pipelineInfo.rasterizer.setFrontFaceWinding(pvrvk::FrontFace::e_COUNTER_CLOCKWISE);

		// Disable depth writing and depth testing
		pipelineInfo.depthStencil.enableDepthWrite(false);
		pipelineInfo.depthStencil.enableDepthTest(false);

		// Disable stencil testing
		pipelineInfo.depthStencil.enableStencilTest(false);
		pipelineInfo.colorBlend.setAttachmentState(0, pvrvk::PipelineColorBlendAttachmentState());

		// We use attribute-less rendering
		pipelineInfo.vertexInput.clear();
		pipelineInfo.inputAssembler.setPrimitiveTopology(pvrvk::PrimitiveTopology::e_TRIANGLE_STRIP);

		pipelineInfo.renderPass = blurRenderPass;
		pipelineInfo.subpass = 0;

		// Create the up and down sample pipelines using the appropriate dimensions and shaders
		for (uint32_t j = 0; j < MaxFilterIterations - 1; ++j)
		{
			// Downsample
			if (j < MaxFilterIterations / 2)
			{
				pipelineInfo.vertexShader.setShader(
					device->createShaderModule(pvrvk::ShaderModuleCreateInfo(assetProvider.getAssetStream(Files::DualFilterDownVertSrcFile)->readToEnd<uint32_t>())));
				pipelineInfo.fragmentShader.setShader(
					device->createShaderModule(pvrvk::ShaderModuleCreateInfo(assetProvider.getAssetStream(Files::DualFilterDownSampleFragSrcFile)->readToEnd<uint32_t>())));
			}
			// Upsample
			else
			{
				pipelineInfo.vertexShader.setShader(
					device->createShaderModule(pvrvk::ShaderModuleCreateInfo(assetProvider.getAssetStream(Files::DualFilterUpVertSrcFile)->readToEnd<uint32_t>())));
				pipelineInfo.fragmentShader.setShader(
					device->createShaderModule(pvrvk::ShaderModuleCreateInfo(assetProvider.getAssetStream(Files::DualFilterUpSampleFragSrcFile)->readToEnd<uint32_t>())));
			}

			pipelineInfo.viewport.setViewportAndScissor(0,
				pvrvk::Viewport(0.0f, 0.0f, static_cast<float>(maxIterationDimensions[j].x), static_cast<float>(maxIterationDimensions[j].y)),
				pvrvk::Rect2D(0, 0, static_cast<uint32_t>(maxIterationDimensions[j].x), static_cast<uint32_t>(maxIterationDimensions[j].y)));

			pipelineInfo.pipelineLayout = pipelineLayout;
			pipelines[j] = device->createGraphicsPipeline(pipelineInfo, pipelineCache);
			pipelineInfo.viewport.clear();
		}

		// Create the final Dual filter pass pipeline
		{
			pipelineInfo.renderPass = onScreenRenderPass;

			pipelineInfo.viewport.setViewportAndScissor(0,
				pvrvk::Viewport(0.0f, 0.0f, static_cast<float>(maxIterationDimensions.back().x), static_cast<float>(maxIterationDimensions.back().y)),
				pvrvk::Rect2D(0, 0, static_cast<uint32_t>(maxIterationDimensions.back().x), static_cast<uint32_t>(maxIterationDimensions.back().y)));

			pipelineInfo.pipelineLayout = finalPassPipelineLayout;
			pipelineInfo.fragmentShader.setShader(
				device->createShaderModule(pvrvk::ShaderModuleCreateInfo(assetProvider.getAssetStream(Files::DualFilterUpSampleMergedFinalPassFragSrcFile)->readToEnd<uint32_t>())));
			finalPassPipeline = device->createGraphicsPipeline(pipelineInfo, pipelineCache);

			// Enable bloom only
			int enabled = 1;
			pipelineInfo.fragmentShader.setShaderConstant(0, pvrvk::ShaderConstantInfo(0, &enabled, static_cast<uint32_t>(pvr::getSize(pvr::GpuDatatypes::Integer))));
			finalPassBloomOnlyPipeline = device->createGraphicsPipeline(pipelineInfo, pipelineCache);
		}
	}

	/// <summary>Configure the Dual Filter push constant values based on the current Dual Filter configuration.</summary>
	virtual void configurePushConstants()
	{
		for (uint32_t i = 0; i < blurIterations; ++i)
		{
			// Downsample
			if (i < blurIterations / 2)
			{
				glm::vec2 pixelSize = glm::vec2(currentIterationInverseDimensions[i]);

				glm::vec2 halfPixelSize = pixelSize / 2.0f;
				glm::vec2 dUV = pixelSize + halfPixelSize;

				pushConstants[i][0] = glm::vec2(-dUV);
				pushConstants[i][1] = glm::vec2(dUV);
				pushConstants[i][2] = glm::vec2(dUV.x, -dUV.y);
				pushConstants[i][3] = glm::vec2(-dUV.x, dUV.y);
			}
			// Upsample
			else
			{
				glm::vec2 pixelSize = glm::vec2(currentIterationInverseDimensions[i]);

				glm::vec2 halfPixelSize = pixelSize / 2.0f;
				glm::vec2 dUV = pixelSize + halfPixelSize;

				pushConstants[i][0] = glm::vec2(-dUV.x * 2.0, 0.0);
				pushConstants[i][1] = glm::vec2(-dUV.x, dUV.y);
				pushConstants[i][2] = glm::vec2(0.0, dUV.y * 2.0);
				pushConstants[i][3] = glm::vec2(dUV.x, dUV.y);
				pushConstants[i][4] = glm::vec2(dUV.x * 2.0, 0.0);
				pushConstants[i][5] = glm::vec2(dUV.x, -dUV.y);
				pushConstants[i][6] = glm::vec2(0.0, -dUV.y * 2.0);
				pushConstants[i][7] = glm::vec2(-dUV.x, -dUV.y);
			}
		}
	}

	/// <summary>Records the commands required for the Dual Filter blur iterations based on the current configuration.</summary>
	/// <param name="swapchainIndex">The swapchain index for which the recorded command buffer will be used.</param>
	/// <param name="onScreenFramebuffer">The on screen Framebuffer.</param>
	/// <param name="renderBloomOnly">The exposure to use in the tone mapping.</param>
	/// <param name="exposure">The exposure to use in the tone mapping.</param>
	virtual void recordCommands(uint32_t swapchainIndex, pvrvk::Framebuffer& onScreenFramebuffer, bool renderBloomOnly, float exposure)
	{
		for (uint32_t i = 0; i < blurIterations; ++i)
		{
			// Special case the final Dual Filter iteration
			if (i == blurIterations - 1)
			{
				cmdBuffers[i][swapchainIndex]->begin(onScreenFramebuffer, 0, pvrvk::CommandBufferUsageFlags::e_RENDER_PASS_CONTINUE_BIT);
				pvr::utils::beginCommandBufferDebugLabel(
					cmdBuffers[i][swapchainIndex], pvrvk::DebugUtilsLabel(pvr::strings::createFormatted("Dual Filter Blur (Final Pass) - swapchain (%i): %i", swapchainIndex, i)));
				cmdBuffers[i][swapchainIndex]->pushConstants(finalPassPipeline->getPipelineLayout(), pvrvk::ShaderStageFlags::e_VERTEX_BIT | pvrvk::ShaderStageFlags::e_FRAGMENT_BIT,
					0, static_cast<uint32_t>(pvr::getSize(pvr::GpuDatatypes::vec2) * 8), &pushConstants[i]);

				if (renderBloomOnly) { cmdBuffers[i][swapchainIndex]->bindPipeline(finalPassBloomOnlyPipeline); }
				else
				{
					cmdBuffers[i][swapchainIndex]->bindPipeline(finalPassPipeline);
				}

				cmdBuffers[i][swapchainIndex]->pushConstants(pipelineLayout, pvrvk::ShaderStageFlags::e_VERTEX_BIT | pvrvk::ShaderStageFlags::e_FRAGMENT_BIT,
					static_cast<uint32_t>(pvr::getSize(pvr::GpuDatatypes::vec2) * 8), static_cast<uint32_t>(pvr::getSize(pvr::GpuDatatypes::Float)), &exposure);

				cmdBuffers[i][swapchainIndex]->bindDescriptorSet(
					pvrvk::PipelineBindPoint::e_GRAPHICS, finalPassPipeline->getPipelineLayout(), 0u, finalPassDescriptorSets[swapchainIndex]);
			}
			// Down/Up sample passes
			else
			{
				cmdBuffers[i][swapchainIndex]->begin(currentFramebuffers[i][swapchainIndex], 0, pvrvk::CommandBufferUsageFlags::e_RENDER_PASS_CONTINUE_BIT);
				pvr::utils::beginCommandBufferDebugLabel(
					cmdBuffers[i][swapchainIndex], pvrvk::DebugUtilsLabel(pvr::strings::createFormatted("Dual filter Blur - swapchain (%i): %i", swapchainIndex, i)));
				cmdBuffers[i][swapchainIndex]->pushConstants(currentPipelines[i]->getPipelineLayout(), pvrvk::ShaderStageFlags::e_VERTEX_BIT | pvrvk::ShaderStageFlags::e_FRAGMENT_BIT,
					0, static_cast<uint32_t>(pvr::getSize(pvr::GpuDatatypes::vec2) * 8), &pushConstants[i]);
				cmdBuffers[i][swapchainIndex]->bindPipeline(currentPipelines[i]);
				cmdBuffers[i][swapchainIndex]->bindDescriptorSet(pvrvk::PipelineBindPoint::e_GRAPHICS, currentPipelines[i]->getPipelineLayout(), 0u, descriptorSets[i][swapchainIndex]);
			}

			cmdBuffers[i][swapchainIndex]->draw(0, 3);
			pvr::utils::endCommandBufferDebugLabel(cmdBuffers[i][swapchainIndex]);
			cmdBuffers[i][swapchainIndex]->end();
		}
	}

	/// <summary>Records the command buffers into the given main rendering command buffer.</summary>
	/// <param name="swapchainIndex">The swapchain index for which the recorded command buffer will be used.</param>
	/// <param name="cmdBuffers">The main command buffer into which the pre-recorded command buffers will be recorded into.</param>
	/// <param name="queue">The queue to which the command buffer will be submitted.</param>
	/// <param name="blurRenderPass">The RenderPass to use for down/up sampling.</param>
	/// <param name="onScreenRenderPass">The on screen RenderPass to use in the final Dual Filter pass.</param>
	/// <param name="onScreenFramebuffer">The on screen Framebuffers to use in the final Dual Filter pass.</param>
	/// <param name="onScreenClearValues">The clear values used in the final Dual Filter pass.</param>
	/// <param name="numOnScreenClearValues">The number of clear colour values to use in the final Dual Filter pass.</param>
	virtual void recordCommandsToMainCommandBuffer(uint32_t swapchainIndex, pvrvk::CommandBuffer& cmdBuffer, pvrvk::Queue& queue, pvrvk::RenderPass& blurRenderPass,
		pvrvk::RenderPass& onScreenRenderPass, pvrvk::Framebuffer& onScreenFramebuffer, pvrvk::ClearValue* onScreenClearValues, uint32_t numOnScreenClearValues)
	{
		(void)queue;
		pvrvk::ClearValue clearValue = pvrvk::ClearValue(0.0f, 0.0f, 0.0f, 1.f);

		for (uint32_t i = 0; i < blurIterations; ++i)
		{
			// Special Case the final Dual Filter pass
			if (i == blurIterations - 1)
			{
				cmdBuffer->beginRenderPass(onScreenFramebuffer, onScreenRenderPass,
					pvrvk::Rect2D(0, 0, static_cast<uint32_t>(currentIterationDimensions[i].x), static_cast<uint32_t>(currentIterationDimensions[i].y)), false, onScreenClearValues,
					numOnScreenClearValues);
				cmdBuffer->executeCommands(cmdBuffers[i][swapchainIndex]);
			}
			// Down/Up sample passes
			else
			{
				cmdBuffer->beginRenderPass(currentFramebuffers[i][swapchainIndex], blurRenderPass,
					pvrvk::Rect2D(0, 0, static_cast<uint32_t>(currentIterationDimensions[i].x), static_cast<uint32_t>(currentIterationDimensions[i].y)), false, &clearValue, 1);
				cmdBuffer->executeCommands(cmdBuffers[i][swapchainIndex]);
				cmdBuffer->endRenderPass();
			}
		}
	}
};

// Presented in "Next Generation Post Processing In Call Of Duty Advanced Warfare" by Jorge Jimenez
// Filters whilst Downsampling and Upsampling
// Downsamples:
//	Used for preventing aliasing artefacts
//		A = downsample4(FullRes)
//		B = downsample4(A)
//		C = downsample4(B)
//		D = downsample4(C)
//		E = downsample4(D)
// Upsamples:
//	Used for image quality and smooth results
//	Upsampling progressively using bilinear filtering is equivalent to bi-quadratic b-spline filtering
//	We do the sum with the previous mip as we upscale
//		E' = E
//		D' = D + blur(E')
//		C' = C + blur(D')
//		B' = B + blur(C')
//		A' = A + blur(B')
//	The tent filter (3x3) - uses a radius parameter : 1 2 1
//												      2 4 2 * 1/16
//													  1 2 1
// Described here: http://www.iryoku.com/next-generation-post-processing-in-call-of-duty-advanced-warfare
// We make use of the DualFilterBlurPass as these passes share many similarities
struct DownAndTentFilterBlurPass : public DualFilterBlurPass
{
	// Up sample pass image dependencies - these are dependencies on the downsampled mipmap i.e upsample D' is dependent on down sampled image D
	pvr::Multi<std::vector<pvrvk::ImageView>> upSampleIterationImageDependencies[MaxFilterIterations / 2 - 1];

	// Defines a scale to use for offsetting the tent offsets
	glm::vec2 tentScales[MaxFilterIterations / 2];

	// A set of downsample passes
	DownSamplePass downsamplePasses[MaxFilterIterations / 2];

	// The descriptor set layout for the up sample iterations
	pvrvk::DescriptorSetLayout firstUpSampleDescriptorSetLayout;

	// The special cased first up sample pass per swapchain descriptor sets
	pvr::Multi<pvrvk::DescriptorSet> firstUpSampleDescriptorSets;

	// The pipeline layout for the up sample iterations
	pvrvk::PipelineLayout firstUpSamplePipelineLayout;

	// A special cased up sample pipeline - we have one per potential Filter iteration as the dimension used is important
	pvrvk::GraphicsPipeline firstUpSamplePipelines[MaxFilterIterations / 2 - 1];

	/// <summary>Initialises the Dual Filter blur.</summary>
	/// <param name="assetProvider">The pvr::IAssetProvider which will be used for loading resources from memory.</param>
	/// <param name="device">The device from which the resources will be allocated.</param>
	/// <param name="swapchain">The swapchain which will determine the number of per swapchain resources to allocate.</param>
	/// <param name="commandPool">The command pool from which to allocate command buffers.</param>
	/// <param name="descriptorPool">The descriptor pool from which to allocate descriptor sets.</param>
	/// <param name="blurRenderPass">The RenderPass to use for down/up sampling.</param>
	/// <param name="inOffscreenImageViews">A list of offscreen images (one per swapchain) which will be used in the final image composition pass.</param>
	/// <param name="inSourceImageViews">A list of source images (one per swapchain) which will be used as the source for the blur.</param>
	/// <param name="vmaAllocator">A VMA allocator to use for allocating images and buffers.</param>
	/// <param name="inColorImageFormat">The colour image format to use for the Dual Filter blur.</param>
	/// <param name="inFramebufferDimensions">The full size resolution framebuffer dimensions.</param>
	/// <param name="inSampler">The sampler object to use when sampling from the images during the Dual Filter blur passes.</param>
	/// <param name="onScreenRenderPass">The main RenderPass used for rendering to the screen.</param>
	/// <param name="pipelineCache">A pipeline cache object to use when creating pipelines.</param>
	/// <param name="sharedImageViews">A set of shared image views which are available for use rather than allocating identical images.</param>
	void init(pvr::IAssetProvider& assetProvider, pvrvk::Device& device, pvrvk::Swapchain& swapchain, pvrvk::CommandPool& commandPool, pvrvk::DescriptorPool& descriptorPool,
		pvrvk::RenderPass& blurRenderPass, pvr::Multi<pvrvk::ImageView>& inOffscreenImageViews, pvr::Multi<pvrvk::ImageView>& inSourceImageViews,
		pvr::utils::vma::Allocator& vmaAllocator, pvrvk::Format inColorImageFormat, const glm::uvec2& inFramebufferDimensions, pvrvk::Sampler& inSampler,
		pvrvk::RenderPass& onScreenRenderPass, pvrvk::PipelineCache& pipelineCache, std::vector<pvr::Multi<pvrvk::ImageView>>& sharedImageViews)
	{
		// These parameters are used to scale the tent filter so that it does not map directly to pixels and may have "holes"
		tentScales[0] = glm::vec2(1.0f, 1.0f);
		tentScales[1] = glm::vec2(1.0f, 1.0f);
		tentScales[2] = glm::vec2(1.0f, 1.0f);
		tentScales[3] = glm::vec2(1.0f, 1.0f);
		tentScales[4] = glm::vec2(1.0f, 1.0f);

		DualFilterBlurPass::init(assetProvider, device, swapchain, commandPool, descriptorPool, blurRenderPass, inOffscreenImageViews, inSourceImageViews, vmaAllocator,
			inColorImageFormat, inFramebufferDimensions, inSampler, onScreenRenderPass, pipelineCache, sharedImageViews);

		for (uint32_t i = 0; i < MaxFilterIterations / 2; ++i)
		{
			pvr::Multi<pvrvk::ImageView>& sourceImages = inSourceImageViews;
			pvr::Multi<pvrvk::ImageView>& destinationImages = imageViews[i];
			if (i != 0) { sourceImages = imageViews[i - 1]; }
			downsamplePasses[i].init(
				assetProvider, device, swapchain, commandPool, descriptorPool, maxIterationDimensions[i], sourceImages, destinationImages, inSampler, pipelineCache, false);
		}
	}

	/// <summary>Creates the descriptor sets used for up/down sample passes.</summary>
	/// <param name="swapchain">The swapchain which will determine the number of per swapchain resources to allocate.</param>
	/// <param name="descriptorPool">The descriptor pool from which to allocate descriptor sets.</param>
	void createDescriptorSets(pvrvk::Swapchain& swapchain, pvrvk::DescriptorPool& descriptorPool) override
	{
		for (uint32_t i = 0; i < swapchain->getSwapchainLength(); ++i)
		{
			for (uint32_t j = 0; j < MaxFilterIterations / 2 - 1; ++j) { descriptorSets[j].add(descriptorPool->allocateDescriptorSet(descriptorSetLayout)); }
			firstUpSampleDescriptorSets.add(descriptorPool->allocateDescriptorSet(firstUpSampleDescriptorSetLayout));
			finalPassDescriptorSets.add(descriptorPool->allocateDescriptorSet(finalPassDescriptorSetLayout));
		}
	}

	/// <summary>Allocates the images used for each of the down/up sample passes.</summary>
	/// <param name="device">The device from which the resources will be allocated.</param>
	/// <param name="swapchain">The swapchain which will determine the number of per swapchain resources to allocate.</param>
	/// <param name="vmaAllocator">A VMA allocator to use for allocating images and buffers.</param>
	/// <param name="sharedImageViews">A set of shared image views which are available for use rather than allocating identical images.</param>
	void allocatePingPongImages(
		pvrvk::Device& device, pvrvk::Swapchain& swapchain, pvr::utils::vma::Allocator& vmaAllocator, std::vector<pvr::Multi<pvrvk::ImageView>>& sharedImageViews) override
	{
		pvrvk::ImageUsageFlags imageUsage = pvrvk::ImageUsageFlags::e_COLOR_ATTACHMENT_BIT | pvrvk::ImageUsageFlags::e_SAMPLED_BIT;

		auto localSharedImageViews = sharedImageViews;

		for (uint32_t i = 0; i < swapchain->getSwapchainLength(); ++i)
		{
			for (uint32_t j = 0; j < MaxFilterIterations - 1; ++j)
			{
				pvrvk::Extent3D extent = pvrvk::Extent3D(static_cast<uint32_t>(maxIterationDimensions[j].x), static_cast<uint32_t>(maxIterationDimensions[j].y), 1);

				// Attempt to find a compatible image to cut down on the maximum number of images allocated
				bool foundCompatible = false;
				uint32_t compatibleImageIndex = -1;
				for (uint32_t k = 0; k < localSharedImageViews.size(); ++k)
				{
					if (isCompatibleImageView(localSharedImageViews[k][i], pvrvk::ImageType::e_2D, colorImageFormat, extent, imageUsage, pvrvk::ImageLayersSize(),
							pvrvk::SampleCountFlags::e_1_BIT, pvrvk::MemoryPropertyFlags::e_DEVICE_LOCAL_BIT, pvrvk::SharingMode::e_EXCLUSIVE, pvrvk::ImageTiling::e_OPTIMAL, nullptr, 0))
					{
						foundCompatible = true;
						compatibleImageIndex = k;
						break;
					}
				}
				if (foundCompatible)
				{
					// add and remove from the back
					imageViews[j].add(sharedImageViews[compatibleImageIndex][(uint32_t)sharedImageViews[compatibleImageIndex].size() - 1 - i]);
					localSharedImageViews[compatibleImageIndex].resize((uint32_t)localSharedImageViews[compatibleImageIndex].size() - 1);
				}
				else
				{
					pvrvk::Image blurColorTexture = pvr::utils::createImage(device, pvrvk::ImageCreateInfo(pvrvk::ImageType::e_2D, colorImageFormat, extent, imageUsage),
						pvrvk::MemoryPropertyFlags::e_DEVICE_LOCAL_BIT, pvrvk::MemoryPropertyFlags::e_NONE, vmaAllocator);

					imageViews[j].add(device->createImageView(pvrvk::ImageViewCreateInfo(blurColorTexture)));
					addImageToSharedImages(sharedImageViews, imageViews[j].back(), i);
				}
			}
		}
	}

	/// <summary>Configure the set of Tent Filter ping pong images based on the current Tent Filter configuration.</summary>
	void configurePingPongImages() override
	{
		for (uint32_t i = 0; i < MaxFilterIterations - 1; ++i) { currentImageViews[i].clear(); }

		uint32_t index = 0;
		for (; index < blurIterations / 2; ++index) { currentImageViews[index] = imageViews[index]; }

		for (uint32_t i = 0; i < (blurIterations / 2) - 1; ++i)
		{
			currentImageViews[index] = imageViews[(MaxFilterIterations - (blurIterations / 2)) + i];
			index++;
		}
	}

	/// <summary>Creates the descriptor set layouts used for up/down sample passes.</summary>
	/// <param name="device">The device from which the descriptor set layouts will be allocated.</param>
	void createDescriptorSetLayouts(pvrvk::Device& device) override
	{
		// create the pre bloom descriptor set layout
		pvrvk::DescriptorSetLayoutCreateInfo descSetInfo;
		descSetInfo.setBinding(0, pvrvk::DescriptorType::e_COMBINED_IMAGE_SAMPLER, 1u, pvrvk::ShaderStageFlags::e_FRAGMENT_BIT);
		firstUpSampleDescriptorSetLayout = device->createDescriptorSetLayout(descSetInfo);

		descSetInfo.setBinding(1, pvrvk::DescriptorType::e_COMBINED_IMAGE_SAMPLER, 1u, pvrvk::ShaderStageFlags::e_FRAGMENT_BIT);
		descriptorSetLayout = device->createDescriptorSetLayout(descSetInfo);

		descSetInfo.setBinding(2, pvrvk::DescriptorType::e_COMBINED_IMAGE_SAMPLER, 1u, pvrvk::ShaderStageFlags::e_FRAGMENT_BIT);
		descSetInfo.setBinding(3, pvrvk::DescriptorType::e_UNIFORM_BUFFER, 1u, pvrvk::ShaderStageFlags::e_FRAGMENT_BIT);
		finalPassDescriptorSetLayout = device->createDescriptorSetLayout(descSetInfo);

		// create the pipeline layouts
		pvrvk::PipelineLayoutCreateInfo pipeLayoutInfo;
		pipeLayoutInfo.setDescSetLayout(0, firstUpSampleDescriptorSetLayout);
		firstUpSamplePipelineLayout = device->createPipelineLayout(pipeLayoutInfo);

		pipeLayoutInfo.setDescSetLayout(0, descriptorSetLayout);

		// Push constants are used for uploading the texture sample offsets
		pvrvk::PushConstantRange pushConstantsRanges;
		pushConstantsRanges.setOffset(0);
		pushConstantsRanges.setStageFlags(pvrvk::ShaderStageFlags::e_VERTEX_BIT | pvrvk::ShaderStageFlags::e_FRAGMENT_BIT);
		pushConstantsRanges.setSize(static_cast<uint32_t>(pvr::getSize(pvr::GpuDatatypes::vec2) * 8 + pvr::getSize(pvr::GpuDatatypes::Float)));
		pipeLayoutInfo.setPushConstantRange(0, pushConstantsRanges);

		pipelineLayout = device->createPipelineLayout(pipeLayoutInfo);

		pipeLayoutInfo.setDescSetLayout(0, finalPassDescriptorSetLayout);
		finalPassPipelineLayout = device->createPipelineLayout(pipeLayoutInfo);
	}

	/// <summary>Updates the descriptor sets used for each of the down/up sample passes.</summary>
	/// <param name="device">The device from which the updateDescriptorSets call will be piped.</param>
	/// <param name="swapchainIndex">The swapchain index of the descriptor set to update.</param>
	void updateDescriptorSets(pvrvk::Device& device, uint32_t swapchainIndex) override
	{
		std::vector<pvrvk::WriteDescriptorSet> writeDescSets;

		uint32_t upSampleDescriptorIndex = 0;

		// first Upsample
		writeDescSets.push_back(
			pvrvk::WriteDescriptorSet(pvrvk::DescriptorType::e_COMBINED_IMAGE_SAMPLER, firstUpSampleDescriptorSets[swapchainIndex], 0)
				.setImageInfo(0, pvrvk::DescriptorImageInfo(currentImageViews[blurIterations / 2 - 1][swapchainIndex], sampler, pvrvk::ImageLayout::e_SHADER_READ_ONLY_OPTIMAL)));

		// upsample
		for (uint32_t i = blurIterations / 2 + 1; i < blurIterations - 1; ++i)
		{
			writeDescSets.push_back(pvrvk::WriteDescriptorSet(pvrvk::DescriptorType::e_COMBINED_IMAGE_SAMPLER, descriptorSets[upSampleDescriptorIndex][swapchainIndex], 0)
										.setImageInfo(0, pvrvk::DescriptorImageInfo(currentImageViews[i - 1][swapchainIndex], sampler, pvrvk::ImageLayout::e_SHADER_READ_ONLY_OPTIMAL)));

			writeDescSets.push_back(pvrvk::WriteDescriptorSet(pvrvk::DescriptorType::e_COMBINED_IMAGE_SAMPLER, descriptorSets[upSampleDescriptorIndex][swapchainIndex], 1)
										.setImageInfo(0,
											pvrvk::DescriptorImageInfo(currentImageViews[blurIterations / 2 - 1 - upSampleDescriptorIndex][swapchainIndex], sampler,
												pvrvk::ImageLayout::e_SHADER_READ_ONLY_OPTIMAL)));
			upSampleDescriptorIndex++;
		}

		// Final pass
		writeDescSets.push_back(
			pvrvk::WriteDescriptorSet(pvrvk::DescriptorType::e_COMBINED_IMAGE_SAMPLER, finalPassDescriptorSets[swapchainIndex], 0)
				.setImageInfo(0, pvrvk::DescriptorImageInfo(currentImageViews[blurIterations - 2][swapchainIndex], sampler, pvrvk::ImageLayout::e_SHADER_READ_ONLY_OPTIMAL)));

		writeDescSets.push_back(pvrvk::WriteDescriptorSet(pvrvk::DescriptorType::e_COMBINED_IMAGE_SAMPLER, finalPassDescriptorSets[swapchainIndex], 1)
									.setImageInfo(0, pvrvk::DescriptorImageInfo(currentImageViews[0][swapchainIndex], sampler, pvrvk::ImageLayout::e_SHADER_READ_ONLY_OPTIMAL)));

		writeDescSets.push_back(pvrvk::WriteDescriptorSet(pvrvk::DescriptorType::e_COMBINED_IMAGE_SAMPLER, finalPassDescriptorSets[swapchainIndex], 2)
									.setImageInfo(0, pvrvk::DescriptorImageInfo(offscreenImageViews[swapchainIndex], sampler, pvrvk::ImageLayout::e_SHADER_READ_ONLY_OPTIMAL)));

		device->updateDescriptorSets(writeDescSets.data(), static_cast<uint32_t>(writeDescSets.size()), nullptr, 0);

		upSampleIterationImageDependencies->clear();
		uint32_t downsampledImageIndex = 0;

		// The last entry into the downSampledImageViews array
		uint32_t lastDownSampledImageIndex = blurIterations / 2 - 1;
		// Ignore the last entry as this pass is special cased as the "first up sample"
		uint32_t currentDownSampledImageIndex = lastDownSampledImageIndex - 1;

		for (uint32_t i = blurIterations / 2 + 1; i < blurIterations; ++i)
		{
			upSampleIterationImageDependencies[downsampledImageIndex][swapchainIndex].push_back(currentImageViews[currentDownSampledImageIndex][swapchainIndex]);
			currentDownSampledImageIndex--;
			downsampledImageIndex++;
		}
	}

	/// <summary>Creates the pipelines.</summary>
	/// <param name="assetProvider">The pvr::IAssetProvider which will be used for loading resources from memory.</param>
	/// <param name="device">The device to use for allocating the pipelines.</param>
	/// <param name="blurRenderPass">The RenderPass to use for down/up sampling.</param>
	/// <param name="onScreenRenderPass">The on screen RenderPass used for the final Tent Filter pass.</param>
	/// <param name="pipelineCache">A pipeline cache object to use when creating pipelines.</param>
	void createPipelines(pvr::IAssetProvider& assetProvider, pvrvk::Device& device, pvrvk::RenderPass& blurRenderPass, pvrvk::RenderPass& onScreenRenderPass,
		pvrvk::PipelineCache& pipelineCache) override
	{
		pvrvk::GraphicsPipelineCreateInfo pipelineInfo;
		pipelineInfo.rasterizer.setCullMode(pvrvk::CullModeFlags::e_FRONT_BIT);
		pipelineInfo.rasterizer.setFrontFaceWinding(pvrvk::FrontFace::e_COUNTER_CLOCKWISE);

		// Disable depth writing and depth testing
		pipelineInfo.depthStencil.enableDepthWrite(false);
		pipelineInfo.depthStencil.enableDepthTest(false);

		// Disable stencil testing
		pipelineInfo.depthStencil.enableStencilTest(false);

		pipelineInfo.colorBlend.setAttachmentState(0, pvrvk::PipelineColorBlendAttachmentState());

		// We use attribute-less rendering
		pipelineInfo.vertexInput.clear();
		pipelineInfo.inputAssembler.setPrimitiveTopology(pvrvk::PrimitiveTopology::e_TRIANGLE_STRIP);

		pipelineInfo.renderPass = blurRenderPass;
		pipelineInfo.subpass = 0;

		uint32_t firstUpSampleIndex = 0;
		// Create the up sample pipelines using the appropriate dimensions and shaders
		for (uint32_t i = MaxFilterIterations / 2; i < MaxFilterIterations - 1; ++i)
		{
			pipelineInfo.vertexShader.setShader(
				device->createShaderModule(pvrvk::ShaderModuleCreateInfo(assetProvider.getAssetStream(Files::TentFilterUpSampleVertSrcFile)->readToEnd<uint32_t>())));
			pipelineInfo.fragmentShader.setShader(
				device->createShaderModule(pvrvk::ShaderModuleCreateInfo(assetProvider.getAssetStream(Files::TentFilterUpSampleFragSrcFile)->readToEnd<uint32_t>())));
			pipelineInfo.pipelineLayout = pipelineLayout;

			pipelineInfo.viewport.setViewportAndScissor(0,
				pvrvk::Viewport(0.0f, 0.0f, static_cast<float>(maxIterationDimensions[i].x), static_cast<float>(maxIterationDimensions[i].y)),
				pvrvk::Rect2D(0, 0, static_cast<uint32_t>(maxIterationDimensions[i].x), static_cast<uint32_t>(maxIterationDimensions[i].y)));

			pipelines[i] = device->createGraphicsPipeline(pipelineInfo, pipelineCache);

			// Special cased the first up sample pipelines
			pipelineInfo.vertexShader.setShader(
				device->createShaderModule(pvrvk::ShaderModuleCreateInfo(assetProvider.getAssetStream(Files::PostBloomVertShaderSrcFile)->readToEnd<uint32_t>())));
			pipelineInfo.fragmentShader.setShader(
				device->createShaderModule(pvrvk::ShaderModuleCreateInfo(assetProvider.getAssetStream(Files::TentFilterFirstUpSampleFragSrcFile)->readToEnd<uint32_t>())));
			pipelineInfo.pipelineLayout = firstUpSamplePipelineLayout;

			firstUpSamplePipelines[firstUpSampleIndex] = device->createGraphicsPipeline(pipelineInfo, pipelineCache);
			firstUpSampleIndex++;

			pipelineInfo.viewport.clear();
		}

		// Create the final Tent filter pass pipeline
		{
			pipelineInfo.renderPass = onScreenRenderPass;

			pipelineInfo.viewport.setViewportAndScissor(0,
				pvrvk::Viewport(0.0f, 0.0f, static_cast<float>(maxIterationDimensions.back().x), static_cast<float>(maxIterationDimensions.back().y)),
				pvrvk::Rect2D(0, 0, static_cast<uint32_t>(maxIterationDimensions.back().x), static_cast<uint32_t>(maxIterationDimensions.back().y)));

			pipelineInfo.pipelineLayout = finalPassPipelineLayout;
			pipelineInfo.vertexShader.setShader(
				device->createShaderModule(pvrvk::ShaderModuleCreateInfo(assetProvider.getAssetStream(Files::TentFilterUpSampleVertSrcFile)->readToEnd<uint32_t>())));
			pipelineInfo.fragmentShader.setShader(
				device->createShaderModule(pvrvk::ShaderModuleCreateInfo(assetProvider.getAssetStream(Files::TentFilterUpSampleMergedFinalPassFragSrcFile)->readToEnd<uint32_t>())));
			finalPassPipeline = device->createGraphicsPipeline(pipelineInfo, pipelineCache);

			// Enable bloom only
			int enabled = 1;
			pipelineInfo.fragmentShader.setShaderConstant(0, pvrvk::ShaderConstantInfo(0, &enabled, static_cast<uint32_t>(pvr::getSize(pvr::GpuDatatypes::Integer))));
			finalPassBloomOnlyPipeline = device->createGraphicsPipeline(pipelineInfo, pipelineCache);
		}
	}

	/// <summary>Configure the Tent Filter push constant values based on the current Tent Filter configuration.</summary>
	void configurePushConstants() override
	{
		const glm::vec2 offsets[8] = { glm::vec2(-1.0, 1.0), glm::vec2(0.0, 1.0), glm::vec2(1.0, 1.0), glm::vec2(1.0, 0.0), glm::vec2(1.0, -1.0), glm::vec2(0.0, -1.0),
			glm::vec2(-1.0, -1.0), glm::vec2(-1.0, 0.0) };

		uint32_t tentScaleIndex = 0;
		// The tent filter passes only start after the first up sample pass has finished
		for (uint32_t i = blurIterations / 2; i < blurIterations; ++i)
		{
			pushConstants[i][0] = glm::vec2(1.0f / (currentIterationDimensions[i].x * 0.5), 1.0f / (currentIterationDimensions[i].y * 0.5)) * offsets[0] * tentScales[tentScaleIndex];
			pushConstants[i][1] = glm::vec2(1.0f / (currentIterationDimensions[i].x * 0.5), 1.0f / (currentIterationDimensions[i].y * 0.5)) * offsets[1] * tentScales[tentScaleIndex];
			pushConstants[i][2] = glm::vec2(1.0f / (currentIterationDimensions[i].x * 0.5), 1.0f / (currentIterationDimensions[i].y * 0.5)) * offsets[2] * tentScales[tentScaleIndex];
			pushConstants[i][3] = glm::vec2(1.0f / (currentIterationDimensions[i].x * 0.5), 1.0f / (currentIterationDimensions[i].y * 0.5)) * offsets[3] * tentScales[tentScaleIndex];
			pushConstants[i][4] = glm::vec2(1.0f / (currentIterationDimensions[i].x * 0.5), 1.0f / (currentIterationDimensions[i].y * 0.5)) * offsets[4] * tentScales[tentScaleIndex];
			pushConstants[i][5] = glm::vec2(1.0f / (currentIterationDimensions[i].x * 0.5), 1.0f / (currentIterationDimensions[i].y * 0.5)) * offsets[5] * tentScales[tentScaleIndex];
			pushConstants[i][6] = glm::vec2(1.0f / (currentIterationDimensions[i].x * 0.5), 1.0f / (currentIterationDimensions[i].y * 0.5)) * offsets[6] * tentScales[tentScaleIndex];
			pushConstants[i][7] = glm::vec2(1.0f / (currentIterationDimensions[i].x * 0.5), 1.0f / (currentIterationDimensions[i].y * 0.5)) * offsets[7] * tentScales[tentScaleIndex];
			tentScaleIndex++;
		}
	}

	using DualFilterBlurPass::recordCommands;
	/// <summary>Records the commands required for the Tent Filter blur iterations based on the current configuration.</summary>
	/// <param name="swapchainIndex">The swapchain index for which the recorded command buffer will be used.</param>
	/// <param name="onScreenFramebuffer">The on screen Framebuffer.</param>
	/// <param name="renderBloomOnly">Indicates that only the bloom should be rendered to the screen.</param>
	/// <param name="queue">The queue being used.</param>
	/// <param name="sourceImageView">The source image to blur.</param>
	/// <param name="exposure">The exposure to use in the tone mapping.</param>
	void recordCommands(uint32_t swapchainIndex, pvrvk::Framebuffer& onScreenFramebuffer, bool renderBloomOnly, pvrvk::Queue& queue, pvrvk::ImageView&, float exposure)
	{
		(void)queue;

		uint32_t upSampleIndex = 0;

		uint32_t i = 0;

		// Perform downsamples using separate passes
		for (; i < blurIterations / 2; ++i) { downsamplePasses[i].recordCommands(swapchainIndex); }

		if (blurIterations > 2)
		{
			cmdBuffers[i][swapchainIndex]->begin(currentFramebuffers[i][swapchainIndex], 0, pvrvk::CommandBufferUsageFlags::e_RENDER_PASS_CONTINUE_BIT);
			pvr::utils::beginCommandBufferDebugLabel(
				cmdBuffers[i][swapchainIndex], pvrvk::DebugUtilsLabel(pvr::strings::createFormatted("Tent Blur (First Up Sample Pass) - swapchain (%i): %i", swapchainIndex, i)));
			cmdBuffers[i][swapchainIndex]->bindPipeline(firstUpSamplePipelines[MaxFilterIterations / 2 - i]);
			cmdBuffers[i][swapchainIndex]->bindDescriptorSet(pvrvk::PipelineBindPoint::e_GRAPHICS, firstUpSamplePipelineLayout, 0u, firstUpSampleDescriptorSets[swapchainIndex]);
			cmdBuffers[i][swapchainIndex]->draw(0, 3);
			pvr::utils::endCommandBufferDebugLabel(cmdBuffers[i][swapchainIndex]);
			cmdBuffers[i][swapchainIndex]->end();
			i++;

			// Handle the other up sample passes
			for (; i < blurIterations - 1; ++i)
			{
				cmdBuffers[i][swapchainIndex]->begin(currentFramebuffers[i][swapchainIndex], 0, pvrvk::CommandBufferUsageFlags::e_RENDER_PASS_CONTINUE_BIT);
				pvr::utils::beginCommandBufferDebugLabel(
					cmdBuffers[i][swapchainIndex], pvrvk::DebugUtilsLabel(pvr::strings::createFormatted("Tent Blur (Up Sample Pass) - swapchain (%i): %i", swapchainIndex, i)));
				cmdBuffers[i][swapchainIndex]->bindPipeline(currentPipelines[i]);
				cmdBuffers[i][swapchainIndex]->pushConstants(pipelineLayout, pvrvk::ShaderStageFlags::e_VERTEX_BIT | pvrvk::ShaderStageFlags::e_FRAGMENT_BIT, 0,
					static_cast<uint32_t>(pvr::getSize(pvr::GpuDatatypes::vec2) * 8), &pushConstants[i]);
				cmdBuffers[i][swapchainIndex]->bindDescriptorSet(pvrvk::PipelineBindPoint::e_GRAPHICS, pipelineLayout, 0u, descriptorSets[upSampleIndex][swapchainIndex]);
				cmdBuffers[i][swapchainIndex]->draw(0, 3);
				pvr::utils::endCommandBufferDebugLabel(cmdBuffers[i][swapchainIndex]);
				cmdBuffers[i][swapchainIndex]->end();
				upSampleIndex++;
			}
		}

		// Special case the final up sample pass
		cmdBuffers[i][swapchainIndex]->begin(onScreenFramebuffer, 0, pvrvk::CommandBufferUsageFlags::e_RENDER_PASS_CONTINUE_BIT);
		pvr::utils::beginCommandBufferDebugLabel(
			cmdBuffers[i][swapchainIndex], pvrvk::DebugUtilsLabel(pvr::strings::createFormatted("Tent Blur (Final Pass) - swapchain (%i): %i", swapchainIndex, i)));
		if (renderBloomOnly) { cmdBuffers[i][swapchainIndex]->bindPipeline(finalPassBloomOnlyPipeline); }
		else
		{
			cmdBuffers[i][swapchainIndex]->bindPipeline(finalPassPipeline);
		}
		cmdBuffers[i][swapchainIndex]->pushConstants(finalPassPipeline->getPipelineLayout(), pvrvk::ShaderStageFlags::e_VERTEX_BIT | pvrvk::ShaderStageFlags::e_FRAGMENT_BIT, 0,
			static_cast<uint32_t>(pvr::getSize(pvr::GpuDatatypes::vec2) * 8), &pushConstants[i]);
		cmdBuffers[i][swapchainIndex]->pushConstants(finalPassPipeline->getPipelineLayout(), pvrvk::ShaderStageFlags::e_VERTEX_BIT | pvrvk::ShaderStageFlags::e_FRAGMENT_BIT,
			static_cast<uint32_t>(pvr::getSize(pvr::GpuDatatypes::vec2) * 8), static_cast<uint32_t>(pvr::getSize(pvr::GpuDatatypes::Float)), &exposure);
		cmdBuffers[i][swapchainIndex]->bindDescriptorSet(pvrvk::PipelineBindPoint::e_GRAPHICS, finalPassPipeline->getPipelineLayout(), 0u, finalPassDescriptorSets[swapchainIndex]);
		cmdBuffers[i][swapchainIndex]->draw(0, 3);
		pvr::utils::endCommandBufferDebugLabel(cmdBuffers[i][swapchainIndex]);
		cmdBuffers[i][swapchainIndex]->end();
	}

	/// <summary>Records the command buffers into the given main rendering command buffer.</summary>
	/// <param name="swapchainIndex">The swapchain index for which the recorded command buffer will be used.</param>
	/// <param name="cmdBuffers">The main command buffer into which the pre-recorded command buffers will be recorded into.</param>
	/// <param name="queue">The queue to which the command buffer will be submitted.</param>
	/// <param name="blurRenderPass">The RenderPass to use for down/up sampling.</param>
	/// <param name="onScreenRenderPass">The on screen RenderPass to use in the final Tent Filter pass.</param>
	/// <param name="onScreenFramebuffer">The on screen Framebuffers to use in the final Tent Filter pass.</param>
	/// <param name="onScreenClearValues">The clear values used in the final Tent Filter pass.</param>
	/// <param name="numOnScreenClearValues">The number of clear colour values to use in the final Tent Filter pass.</param>
	void recordCommandsToMainCommandBuffer(uint32_t swapchainIndex, pvrvk::CommandBuffer& cmdBuffer, pvrvk::Queue& queue, pvrvk::RenderPass& blurRenderPass,
		pvrvk::RenderPass& onScreenRenderPass, pvrvk::Framebuffer& onScreenFramebuffer, pvrvk::ClearValue* onScreenClearValues, uint32_t numOnScreenClearValues) override
	{
		pvrvk::ClearValue clearValue = pvrvk::ClearValue(0.0f, 0.0f, 0.0f, 1.f);

		pvrvk::ImageLayout sourceImageLayout = pvrvk::ImageLayout::e_SHADER_READ_ONLY_OPTIMAL;
		pvrvk::ImageLayout destinationImageLayout = pvrvk::ImageLayout::e_SHADER_READ_ONLY_OPTIMAL;

		for (uint32_t i = 0; i < blurIterations / 2; ++i)
		{
			cmdBuffer->beginRenderPass(downsamplePasses[i].framebuffers[swapchainIndex], blurRenderPass,
				pvrvk::Rect2D(0, 0, static_cast<uint32_t>(currentIterationDimensions[i].x), static_cast<uint32_t>(currentIterationDimensions[i].y)), false, &clearValue, 1);
			cmdBuffer->executeCommands(downsamplePasses[i].cmdBuffers[swapchainIndex]);
			cmdBuffer->endRenderPass();
		}

		uint32_t upSampleIndex = 0;
		for (uint32_t i = blurIterations / 2; i < blurIterations - 1; ++i)
		{
			// Take care of the extra image dependencies the up sample passes require
			for (uint32_t j = 0; j < upSampleIterationImageDependencies[upSampleIndex][swapchainIndex].size(); ++j)
			{
				pvrvk::MemoryBarrierSet layoutTransition;
				layoutTransition.addBarrier(pvrvk::ImageMemoryBarrier(pvrvk::AccessFlags::e_COLOR_ATTACHMENT_WRITE_BIT, pvrvk::AccessFlags::e_SHADER_READ_BIT,
					upSampleIterationImageDependencies[upSampleIndex][swapchainIndex][j]->getImage(), pvrvk::ImageSubresourceRange(pvrvk::ImageAspectFlags::e_COLOR_BIT, 0u, 1u, 0u, 1u),
					sourceImageLayout, destinationImageLayout, queue->getFamilyIndex(), queue->getFamilyIndex()));
				cmdBuffer->pipelineBarrier(pvrvk::PipelineStageFlags::e_COLOR_ATTACHMENT_OUTPUT_BIT, pvrvk::PipelineStageFlags::e_FRAGMENT_SHADER_BIT, layoutTransition);
			}

			upSampleIndex++;

			cmdBuffer->beginRenderPass(currentFramebuffers[i][swapchainIndex], blurRenderPass,
				pvrvk::Rect2D(0, 0, static_cast<uint32_t>(currentIterationDimensions[i].x), static_cast<uint32_t>(currentIterationDimensions[i].y)), false, &clearValue, 1);
			cmdBuffer->executeCommands(cmdBuffers[i][swapchainIndex]);
			cmdBuffer->endRenderPass();
		}

		// Handle the special cased final up sample whereby we merge the upsample blur with the tone mapping pass to save on write/reads
		cmdBuffer->beginRenderPass(onScreenFramebuffer, onScreenRenderPass,
			pvrvk::Rect2D(0, 0, static_cast<uint32_t>(currentIterationDimensions[blurIterations - 1].x), static_cast<uint32_t>(currentIterationDimensions[blurIterations - 1].y)),
			false, onScreenClearValues, numOnScreenClearValues);
		cmdBuffer->executeCommands(cmdBuffers[blurIterations - 1][swapchainIndex]);
	}

	virtual ~DownAndTentFilterBlurPass() {}
};

// A Gaussian Blur Pass
struct GaussianBlurPass
{
	// Horizontal and Vertical graphics pipelines
	pvrvk::GraphicsPipeline horizontalPipelines[DemoConfigurations::NumDemoConfigurations];
	pvrvk::GraphicsPipeline verticalPipelines[DemoConfigurations::NumDemoConfigurations];

	pvrvk::DescriptorSetLayout descriptorSetLayout;
	pvrvk::PipelineLayout pipelineLayout;

	// Horizontal and Vertical descriptor sets
	pvr::Multi<pvrvk::DescriptorSet> horizontalDescriptorSets;
	pvr::Multi<pvrvk::DescriptorSet> verticalDescriptorSets;

	// Horizontal and Vertical command buffers
	pvr::Multi<pvrvk::SecondaryCommandBuffer> horizontalBlurCommandBuffers;
	pvr::Multi<pvrvk::SecondaryCommandBuffer> verticalBlurCommandBuffers;

	pvr::Multi<pvrvk::ImageView> blurredImages;

	uint32_t currentKernelConfig;

	// Gaussian offsets and weights
	std::vector<std::vector<double>> gaussianOffsets;
	std::vector<std::vector<double>> gaussianWeights;

	std::vector<std::string> perKernelSizeIterationsStrings;
	std::vector<std::string> perKernelSizeWeightsStrings;
	std::vector<std::string> perKernelSizeOffsetsStrings;

	float inverseFramebufferWidth;
	float inverseFramebufferHeight;
	std::string inverseFramebufferWidthString;
	std::string inverseFramebufferHeightString;

	/// <summary>Initialises the Gaussian blur pass.</summary>
	/// <param name="assetProvider">The pvr::IAssetProvider which will be used for loading resources from memory.</param>
	/// <param name="device">The device from which the resources will be allocated.</param>
	/// <param name="swapchain">The swapchain which will determine the number of per swapchain resources to allocate.</param>
	/// <param name="commandPool">The command pool from which to allocate command buffers.</param>
	/// <param name="descriptorPool">The descriptor pool from which to allocate descriptor sets.</param>
	/// <param name="vmaAllocator">A VMA allocator to use for allocating images and buffers.</param>
	/// <param name="blurRenderPass">The RenderPass to use.</param>
	/// <param name="blurFramebufferDimensions">The dimensions used for the blur.</param>
	/// <param name="horizontalBlurImageViews">A set of images (per swapchain) to use as the destination for a horizontal blur.</param>
	/// <param name="verticalBlurImageViews">A set of images (per swapchain) to use as the destination for a vertical blur.</param>
	/// <param name="sampler">A bilinear sampler object.</param>
	/// <param name="pipelineCache">A pipeline cache object to use when creating pipelines.</param>
	void init(pvr::IAssetProvider& assetProvider, pvrvk::Device& device, pvrvk::Swapchain& swapchain, pvrvk::CommandPool& commandPool, pvrvk::DescriptorPool& descriptorPool,
		pvr::utils::vma::Allocator& vmaAllocator, pvrvk::RenderPass& blurRenderPass, const glm::uvec2& blurFramebufferDimensions,
		pvr::Multi<pvrvk::ImageView>& horizontalBlurImageViews, pvr::Multi<pvrvk::ImageView>& verticalBlurImageViews, pvrvk::Sampler& sampler, pvrvk::PipelineCache& pipelineCache)
	{
		(void)vmaAllocator;
		createDescriptorSetLayout(device);

		inverseFramebufferWidth = 1.0f / blurFramebufferDimensions.x;
		inverseFramebufferHeight = 1.0f / blurFramebufferDimensions.y;
		gaussianWeights.resize(DemoConfigurations::NumDemoConfigurations);
		gaussianOffsets.resize(DemoConfigurations::NumDemoConfigurations);

		generatePerConfigGaussianCoefficients();
		generateGaussianShaderStrings();

		createPipelines(assetProvider, device, blurRenderPass, blurFramebufferDimensions, pipelineCache);
		createDescriptorSets(device, swapchain, descriptorPool, horizontalBlurImageViews, verticalBlurImageViews, sampler);

		for (uint32_t i = 0; i < swapchain->getSwapchainLength(); ++i)
		{
			horizontalBlurCommandBuffers.add(commandPool->allocateSecondaryCommandBuffer());
			verticalBlurCommandBuffers.add(commandPool->allocateSecondaryCommandBuffer());
		}

		blurredImages = verticalBlurImageViews;
	}

	/// <summary>Updates the kernel configuration currently in use.</summary>
	/// <param name="kernelSizeConfig">The kernel size.</param>
	void updateKernelConfig(uint32_t kernelSizeConfig) { currentKernelConfig = kernelSizeConfig; }

	virtual void generatePerConfigGaussianCoefficients()
	{
		for (uint32_t i = 0; i < DemoConfigurations::NumDemoConfigurations; ++i)
		{ generateGaussianCoefficients(DemoConfigurations::Configurations[i].gaussianConfig, false, false, gaussianWeights[i], gaussianOffsets[i]); }
	}

	/// <summary>Generates the Gaussian weights and offsets strings used by the various Gaussian shaders.</summary>
	virtual void generateGaussianShaderStrings()
	{
		// Generate per kernel size weights, offsets and iterations strings
		perKernelSizeIterationsStrings.resize(DemoConfigurations::NumDemoConfigurations);
		perKernelSizeWeightsStrings.resize(DemoConfigurations::NumDemoConfigurations);
		perKernelSizeOffsetsStrings.resize(DemoConfigurations::NumDemoConfigurations);

		for (uint32_t i = 0; i < DemoConfigurations::NumDemoConfigurations; ++i)
		{
			generateGaussianWeightsAndOffsetsStrings(
				gaussianWeights[i], gaussianOffsets[i], perKernelSizeIterationsStrings[i], perKernelSizeWeightsStrings[i], perKernelSizeOffsetsStrings[i]);
		}
		inverseFramebufferWidthString = pvr::strings::createFormatted("const highp float inverseFramebufferWidth = %.15f;", inverseFramebufferWidth);
		inverseFramebufferHeightString = pvr::strings::createFormatted("const highp float inverseFramebufferHeight = %.15f;", inverseFramebufferHeight);
	}

	/// <summary>Creates the descriptor set layouts used for up/down sample passes.</summary>
	/// <param name="device">The device from which the descriptor set layouts will be allocated.</param>
	virtual void createDescriptorSetLayout(pvrvk::Device& device)
	{
		pvrvk::DescriptorSetLayoutCreateInfo descSetInfo;
		descSetInfo.setBinding(0, pvrvk::DescriptorType::e_COMBINED_IMAGE_SAMPLER, 1u, pvrvk::ShaderStageFlags::e_FRAGMENT_BIT);
		descriptorSetLayout = device->createDescriptorSetLayout(descSetInfo);

		pvrvk::PipelineLayoutCreateInfo pipeLayoutInfo;
		pipeLayoutInfo.setDescSetLayout(0, descriptorSetLayout);
		pipelineLayout = device->createPipelineLayout(pipeLayoutInfo);
	}

	/// <summary>Creates the descriptor sets.</summary>
	/// <param name="device">The device to use for allocating the descriptor sets.</param>
	/// <param name="swapchain">The swapchain which will determine the number of per swapchain resources to allocate.</param>
	/// <param name="descriptorPool">The descriptor pool from which to allocate descriptor sets.</param>
	/// <param name="horizontalBlurImageViews">A set of images (per swapchain) to use as the destination for a horizontal blur.</param>
	/// <param name="verticalBlurImageViews">A set of images (per swapchain) to use as the destination for a vertical blur (this is also the destination for a downsample or at
	/// least is the source image to do a horizontal blur).</param>
	/// <param name="sampler">A bilinear sampler object.</param>
	virtual void createDescriptorSets(pvrvk::Device& device, pvrvk::Swapchain& swapchain, pvrvk::DescriptorPool& descriptorPool,
		pvr::Multi<pvrvk::ImageView>& horizontalBlurImageViews, pvr::Multi<pvrvk::ImageView>& verticalBlurImageViews, pvrvk::Sampler& sampler)
	{
		std::vector<pvrvk::WriteDescriptorSet> writeDescSets;

		for (uint32_t i = 0; i < swapchain->getSwapchainLength(); ++i)
		{
			// Descriptor sets for the Horizontal Blur Pass
			horizontalDescriptorSets.add(descriptorPool->allocateDescriptorSet(descriptorSetLayout));
			writeDescSets.push_back(pvrvk::WriteDescriptorSet(pvrvk::DescriptorType::e_COMBINED_IMAGE_SAMPLER, horizontalDescriptorSets[i], 0)
										.setImageInfo(0, pvrvk::DescriptorImageInfo(verticalBlurImageViews[i], sampler, pvrvk::ImageLayout::e_SHADER_READ_ONLY_OPTIMAL)));

			// Descriptor sets for the Vertical Blur Pass
			verticalDescriptorSets.add(descriptorPool->allocateDescriptorSet(descriptorSetLayout));
			writeDescSets.push_back(pvrvk::WriteDescriptorSet(pvrvk::DescriptorType::e_COMBINED_IMAGE_SAMPLER, verticalDescriptorSets[i], 0)
										.setImageInfo(0, pvrvk::DescriptorImageInfo(horizontalBlurImageViews[i], sampler, pvrvk::ImageLayout::e_SHADER_READ_ONLY_OPTIMAL)));
		}

		device->updateDescriptorSets(writeDescSets.data(), static_cast<uint32_t>(writeDescSets.size()), nullptr, 0);
	}

	/// <summary>Creates the pipelines.</summary>
	/// <param name="assetProvider">The pvr::IAssetProvider which will be used for loading resources from memory.</param>
	/// <param name="device">The device to use for allocating the pipelines.</param>
	/// <param name="renderPass">The RenderPass to use for Gaussian blur.</param>
	/// <param name="blurFramebufferDimensions">The dimensions of the Framebuffers used in the Gaussian blur iterations.</param>
	/// <param name="pipelineCache">A pipeline cache object to use when creating pipelines.</param>
	virtual void createPipelines(
		pvr::IAssetProvider& assetProvider, pvrvk::Device& device, pvrvk::RenderPass& renderPass, const glm::uvec2& blurFramebufferDimensions, pvrvk::PipelineCache& pipelineCache)
	{
		pvrvk::ShaderModule horizontalFragShaderModules[DemoConfigurations::NumDemoConfigurations];
		pvrvk::ShaderModule verticalFragShaderModules[DemoConfigurations::NumDemoConfigurations];

		// Generate the Gaussian blur fragment shader modules
		for (uint32_t i = 0; i < DemoConfigurations::NumDemoConfigurations; ++i)
		{
			// Load the base Gaussian fragment shader
			auto fragShaderStream = assetProvider.getAssetStream(Files::GaussianFragSrcFile);

			// Load the base Gaussian fragment shader into a string
			// At this point the fragment shader is missing its templated arguments and will not compile "as is"
			std::string shaderSource;
			fragShaderStream->readIntoString(shaderSource);

			// Insert the templates into the base shader
			// The reference Gaussian fragment shader requires the number of iterations, the weights for each iteration and the direction to sample
			std::string horizontalShaderString = pvr::strings::createFormatted(shaderSource.c_str(), perKernelSizeIterationsStrings[i].c_str(), perKernelSizeOffsetsStrings[i].c_str(),
				perKernelSizeWeightsStrings[i].c_str(), inverseFramebufferWidthString.c_str(), inverseFramebufferHeightString.c_str(), "1.0, 0.0");
			std::string verticalShaderString = pvr::strings::createFormatted(shaderSource.c_str(), perKernelSizeIterationsStrings[i].c_str(), perKernelSizeOffsetsStrings[i].c_str(),
				perKernelSizeWeightsStrings[i].c_str(), inverseFramebufferWidthString.c_str(), inverseFramebufferHeightString.c_str(), "0.0, 1.0");

			// Create shader modules using the auto generated shader sources
			horizontalFragShaderModules[i] = pvr::utils::createShaderModule(device, horizontalShaderString, pvrvk::ShaderStageFlags::e_FRAGMENT_BIT);
			verticalFragShaderModules[i] = pvr::utils::createShaderModule(device, verticalShaderString, pvrvk::ShaderStageFlags::e_FRAGMENT_BIT);
		}

		pvrvk::GraphicsPipelineCreateInfo pipelineInfo;
		pipelineInfo.viewport.setViewportAndScissor(0, pvrvk::Viewport(0.0f, 0.0f, static_cast<float>(blurFramebufferDimensions.x), static_cast<float>(blurFramebufferDimensions.y)),
			pvrvk::Rect2D(0, 0, blurFramebufferDimensions.x, blurFramebufferDimensions.y));

		pipelineInfo.rasterizer.setCullMode(pvrvk::CullModeFlags::e_FRONT_BIT);
		pipelineInfo.rasterizer.setFrontFaceWinding(pvrvk::FrontFace::e_COUNTER_CLOCKWISE);

		// disable depth writing and depth testing
		pipelineInfo.depthStencil.enableDepthWrite(false);
		pipelineInfo.depthStencil.enableDepthTest(false);

		// disable stencil testing
		pipelineInfo.depthStencil.enableStencilTest(false);

		pipelineInfo.colorBlend.setAttachmentState(0, pvrvk::PipelineColorBlendAttachmentState());

		pipelineInfo.vertexInput.clear();
		pipelineInfo.inputAssembler.setPrimitiveTopology(pvrvk::PrimitiveTopology::e_TRIANGLE_STRIP);

		pipelineInfo.pipelineLayout = pipelineLayout;

		// renderpass/subpass
		pipelineInfo.renderPass = renderPass;
		pipelineInfo.subpass = 0;

		pipelineInfo.vertexShader.setShader(device->createShaderModule(pvrvk::ShaderModuleCreateInfo(assetProvider.getAssetStream(Files::GaussianVertSrcFile)->readToEnd<uint32_t>())));

		for (uint32_t i = 0; i < DemoConfigurations::NumDemoConfigurations; ++i)
		{
			// Horizontal Pipeline
			pipelineInfo.fragmentShader.setShader(horizontalFragShaderModules[i]);
			horizontalPipelines[i] = device->createGraphicsPipeline(pipelineInfo, pipelineCache);
		}

		for (uint32_t i = 0; i < DemoConfigurations::NumDemoConfigurations; ++i)
		{
			// Vertical Pipeline
			pipelineInfo.fragmentShader.setShader(verticalFragShaderModules[i]);
			verticalPipelines[i] = device->createGraphicsPipeline(pipelineInfo, pipelineCache);
		}
	}

	/// <summary>Records the commands required for the Gaussian blur.</summary>
	/// <param name="swapchainIndex">The swapchain index for which the recorded command buffer will be used.</param>
	/// <param name="horizontalBlurFramebuffers">The framebuffers to use in the horizontal blur pass.</param>
	/// <param name="verticalBlurFramebuffers">The framebuffers to use in the vertical blur pass.</param>
	virtual void recordCommands(uint32_t swapchainIndex, pvr::Multi<pvrvk::Framebuffer>& horizontalBlurFramebuffers, pvr::Multi<pvrvk::Framebuffer>& verticalBlurFramebuffers)
	{
		// Record the commands to use for carrying out the horizontal Gaussian blur pass
		{
			horizontalBlurCommandBuffers[swapchainIndex]->begin(horizontalBlurFramebuffers[swapchainIndex], 0, pvrvk::CommandBufferUsageFlags::e_RENDER_PASS_CONTINUE_BIT);

			pvr::utils::beginCommandBufferDebugLabel(
				horizontalBlurCommandBuffers[swapchainIndex], pvrvk::DebugUtilsLabel(pvr::strings::createFormatted("Gaussian Blur (horizontal) - swapchain (%i)", swapchainIndex)));

			horizontalBlurCommandBuffers[swapchainIndex]->bindPipeline(horizontalPipelines[currentKernelConfig]);
			horizontalBlurCommandBuffers[swapchainIndex]->bindDescriptorSet(pvrvk::PipelineBindPoint::e_GRAPHICS, pipelineLayout, 0u, horizontalDescriptorSets[swapchainIndex]);

			horizontalBlurCommandBuffers[swapchainIndex]->draw(0, 3);
			pvr::utils::endCommandBufferDebugLabel(horizontalBlurCommandBuffers[swapchainIndex]);
			horizontalBlurCommandBuffers[swapchainIndex]->end();
		}

		// Record the commands to use for carrying out the vertical Gaussian blur pass
		{
			verticalBlurCommandBuffers[swapchainIndex]->begin(verticalBlurFramebuffers[swapchainIndex], 0, pvrvk::CommandBufferUsageFlags::e_RENDER_PASS_CONTINUE_BIT);

			pvr::utils::beginCommandBufferDebugLabel(
				verticalBlurCommandBuffers[swapchainIndex], pvrvk::DebugUtilsLabel(pvr::strings::createFormatted("Gaussian Blur (vertical) - swapchain (%i)", swapchainIndex)));

			verticalBlurCommandBuffers[swapchainIndex]->bindPipeline(verticalPipelines[currentKernelConfig]);
			verticalBlurCommandBuffers[swapchainIndex]->bindDescriptorSet(pvrvk::PipelineBindPoint::e_GRAPHICS, pipelineLayout, 0u, verticalDescriptorSets[swapchainIndex]);

			verticalBlurCommandBuffers[swapchainIndex]->draw(0, 3);
			pvr::utils::endCommandBufferDebugLabel(verticalBlurCommandBuffers[swapchainIndex]);
			verticalBlurCommandBuffers[swapchainIndex]->end();
		}
	}

	/// <summary>Records the command buffers into the given main rendering command buffer.</summary>
	/// <param name="swapchainIndex">The swapchain index for which the recorded command buffer will be used.</param>
	/// <param name="cmdBuffers">The main command buffer into which the pre-recorded command buffers will be recorded into.</param>
	/// <param name="queue">The queue to which the command buffer will be submitted.</param>
	/// <param name="blurRenderPass">The RenderPass to use for the blur.</param>
	/// <param name="horizontalBlurFramebuffers">The framebuffers to use in the horizontal blur pass.</param>
	/// <param name="verticalBlurFramebuffers">The framebuffers to use in the vertical blur pass.</param>
	virtual void recordCommandsToMainCommandBuffer(uint32_t swapchainIndex, pvrvk::CommandBuffer& cmdBuffers, pvrvk::Queue& queue, pvrvk::RenderPass& blurRenderPass,
		pvr::Multi<pvrvk::Framebuffer>& horizontalBlurFramebuffers, pvr::Multi<pvrvk::Framebuffer>& verticalBlurFramebuffers)
	{
		(void)queue;
		pvrvk::ClearValue clearValue = pvrvk::ClearValue(0.0f, 0.0f, 0.0f, 1.f);

		// Horizontal Blur
		{
			cmdBuffers->beginRenderPass(horizontalBlurFramebuffers[swapchainIndex], blurRenderPass,
				pvrvk::Rect2D(0, 0, horizontalBlurFramebuffers[swapchainIndex]->getDimensions().getWidth(), horizontalBlurFramebuffers[swapchainIndex]->getDimensions().getHeight()),
				false, &clearValue, 1);

			cmdBuffers->executeCommands(horizontalBlurCommandBuffers[swapchainIndex]);
			cmdBuffers->endRenderPass();
		}

		// Note the use of explicit external subpass dependencies which ensure the vertical blur occurs after the horizontal blur

		// Vertical Blur
		{
			cmdBuffers->beginRenderPass(verticalBlurFramebuffers[swapchainIndex], blurRenderPass,
				pvrvk::Rect2D(0, 0, verticalBlurFramebuffers[swapchainIndex]->getDimensions().getWidth(), verticalBlurFramebuffers[swapchainIndex]->getDimensions().getHeight()),
				false, &clearValue, 1);

			cmdBuffers->executeCommands(verticalBlurCommandBuffers[swapchainIndex]);
			cmdBuffers->endRenderPass();
		}
	}

	/// <summary>Returns the blurred image for the given swapchain index.</summary>
	/// <param name="swapchainIndex">The swapchain index of the blurred image to retrieve.</param>
	/// <returns>The blurred image for the specified swapchain index.</returns>
	pvrvk::ImageView& getBlurredImage(uint32_t swapchainIndex) { return blurredImages[swapchainIndex]; }

	virtual ~GaussianBlurPass() {}
};

// A Compute shader based Gaussian Blur Pass
struct ComputeBlurPass : public GaussianBlurPass
{
	// Horizontal and Vertical compute pipelines
	pvrvk::ComputePipeline horizontalComputePipelines[DemoConfigurations::NumDemoConfigurations];
	pvrvk::ComputePipeline verticalComputePipelines[DemoConfigurations::NumDemoConfigurations];

	std::vector<std::string> perKernelSizeCacheStrings;

	virtual ~ComputeBlurPass() {}

	void generatePerConfigGaussianCoefficients() override
	{
		for (uint32_t i = 0; i < DemoConfigurations::NumDemoConfigurations; ++i)
		{ generateGaussianCoefficients(DemoConfigurations::Configurations[i].computeGaussianConfig, false, false, gaussianWeights[i], gaussianOffsets[i]); }
	}

	void createDescriptorSetLayout(pvrvk::Device& device) override
	{
		// create the descriptor set layout
		pvrvk::DescriptorSetLayoutCreateInfo descSetInfo;
		descSetInfo.setBinding(0, pvrvk::DescriptorType::e_STORAGE_IMAGE, 1u, pvrvk::ShaderStageFlags::e_COMPUTE_BIT);
		descSetInfo.setBinding(1, pvrvk::DescriptorType::e_STORAGE_IMAGE, 1u, pvrvk::ShaderStageFlags::e_COMPUTE_BIT);

		descriptorSetLayout = device->createDescriptorSetLayout(descSetInfo);

		// create the pipeline layouts
		pvrvk::PipelineLayoutCreateInfo pipeLayoutInfo;

		pipeLayoutInfo.setDescSetLayout(0, descriptorSetLayout);

		pipelineLayout = device->createPipelineLayout(pipeLayoutInfo);
	}

	void createDescriptorSets(pvrvk::Device& device, pvrvk::Swapchain& swapchain, pvrvk::DescriptorPool& descriptorPool, pvr::Multi<pvrvk::ImageView>& horizontalBlurImageViews,
		pvr::Multi<pvrvk::ImageView>& verticalBlurImageViews, pvrvk::Sampler& sampler) override
	{
		std::vector<pvrvk::WriteDescriptorSet> writeDescSets;

		for (uint32_t i = 0; i < swapchain->getSwapchainLength(); ++i)
		{
			// Descriptor sets for the Horizontal Blur Pass
			horizontalDescriptorSets.add(descriptorPool->allocateDescriptorSet(descriptorSetLayout));
			writeDescSets.push_back(pvrvk::WriteDescriptorSet(pvrvk::DescriptorType::e_STORAGE_IMAGE, horizontalDescriptorSets[i], 0)
										.setImageInfo(0, pvrvk::DescriptorImageInfo(verticalBlurImageViews[i], sampler, pvrvk::ImageLayout::e_GENERAL)));

			writeDescSets.push_back(pvrvk::WriteDescriptorSet(pvrvk::DescriptorType::e_STORAGE_IMAGE, horizontalDescriptorSets[i], 1)
										.setImageInfo(0, pvrvk::DescriptorImageInfo(horizontalBlurImageViews[i], sampler, pvrvk::ImageLayout::e_GENERAL)));

			// Descriptor sets for the Vertical Blur Pass
			verticalDescriptorSets.add(descriptorPool->allocateDescriptorSet(descriptorSetLayout));
			writeDescSets.push_back(pvrvk::WriteDescriptorSet(pvrvk::DescriptorType::e_STORAGE_IMAGE, verticalDescriptorSets[i], 0)
										.setImageInfo(0, pvrvk::DescriptorImageInfo(horizontalBlurImageViews[i], sampler, pvrvk::ImageLayout::e_GENERAL)));

			writeDescSets.push_back(pvrvk::WriteDescriptorSet(pvrvk::DescriptorType::e_STORAGE_IMAGE, verticalDescriptorSets[i], 1)
										.setImageInfo(0, pvrvk::DescriptorImageInfo(verticalBlurImageViews[i], sampler, pvrvk::ImageLayout::e_GENERAL)));
		}

		device->updateDescriptorSets(writeDescSets.data(), static_cast<uint32_t>(writeDescSets.size()), nullptr, 0);
	}

	/// <summary>Generates the Gaussian weights and offsets strings used by the various Gaussian shaders.</summary>
	void generateGaussianShaderStrings() override
	{
		// Generate per kernel size weights, offsets and iterations strings
		perKernelSizeIterationsStrings.resize(DemoConfigurations::NumDemoConfigurations);
		perKernelSizeWeightsStrings.resize(DemoConfigurations::NumDemoConfigurations);
		perKernelSizeOffsetsStrings.resize(DemoConfigurations::NumDemoConfigurations);

		// Compute shaders also need the per row/column colour cache
		perKernelSizeCacheStrings.resize(DemoConfigurations::NumDemoConfigurations);

		for (uint32_t i = 0; i < DemoConfigurations::NumDemoConfigurations; ++i)
		{
			generateGaussianWeightsAndOffsetsStrings(
				gaussianWeights[i], gaussianOffsets[i], perKernelSizeIterationsStrings[i], perKernelSizeWeightsStrings[i], perKernelSizeOffsetsStrings[i], true);

			// Construct the compute shader specific per row/column colour cache strings
			std::string cache = "";
			for (uint32_t j = 0; j < ((gaussianWeights[i].size() * 2) - 1); j++) { cache += "0.0,"; }
			cache += "0.0";

			perKernelSizeCacheStrings[i] = pvr::strings::createFormatted("mediump float f[numIterations * 2] = {%s};", cache.c_str());
		}
	}

	void createPipelines(pvr::IAssetProvider& assetProvider, pvrvk::Device& device, pvrvk::RenderPass& renderPass, const glm::uvec2& blurFramebufferDimensions,
		pvrvk::PipelineCache& pipelineCache) override
	{
		(void)renderPass;
		(void)blurFramebufferDimensions;
		pvrvk::ComputePipelineCreateInfo pipelineInfo;

		pvrvk::ShaderModule horizontalShaderModules[DemoConfigurations::NumDemoConfigurations];
		pvrvk::ShaderModule verticalShaderModules[DemoConfigurations::NumDemoConfigurations];

		std::string formatString = "";

		// The templated compute shaders require that the image formats are specified in the shader
		if (device->getPhysicalDevice()->getFeatures().getShaderStorageImageExtendedFormats()) { formatString = "r16f"; }
		// Special case platforms without support for shader storage image extended formats (features.ShaderStorageImageExtendedFormats)
		// if features.ShaderStorageImageExtendedFormats is not supported then fallback to the less efficient rgba16f shaders
		else
		{
			formatString = "rgba16f";
		}

		pipelineInfo.pipelineLayout = pipelineLayout;

		// Generate the Gaussian blur compute shader modules
		for (uint32_t i = 0; i < DemoConfigurations::NumDemoConfigurations; ++i)
		{
			// Load the base Gaussian compute shaders
			// The horizontal compute shader performs a sliding average across each row of the image
			auto horizontalComputeShaderStream = assetProvider.getAssetStream(Files::GaussianComputeBlurHorizontalSrcFile);
			// The vertical compute shader performs a sliding average across each column of the image
			auto verticalComputeShaderStream = assetProvider.getAssetStream(Files::GaussianComputeBlurVerticalSrcFile);

			// Load the base Gaussian compute shaders into strings
			// At this point the compute shaders are missing its templated arguments and will not compile as they are
			std::string horizontalShaderSource;
			horizontalComputeShaderStream->readIntoString(horizontalShaderSource);
			std::string verticalShaderSource;
			verticalComputeShaderStream->readIntoString(verticalShaderSource);

			// Insert the templates into the base shaders
			// The reference Gaussian compute shaders require the format of the images to use, the number of iterations,
			// the weights for each iteration and the per kernel size caches
			std::string horizontalShaderString = pvr::strings::createFormatted(horizontalShaderSource.c_str(), formatString.c_str(), formatString.c_str(),
				perKernelSizeIterationsStrings[i].c_str(), perKernelSizeWeightsStrings[i].c_str(), perKernelSizeCacheStrings[i].c_str());
			std::string verticalShaderString = pvr::strings::createFormatted(verticalShaderSource.c_str(), formatString.c_str(), formatString.c_str(),
				perKernelSizeIterationsStrings[i].c_str(), perKernelSizeWeightsStrings[i].c_str(), perKernelSizeCacheStrings[i].c_str());

			// Create shader modules using the auto generated shader sources
			horizontalShaderModules[i] = pvr::utils::createShaderModule(device, horizontalShaderString, pvrvk::ShaderStageFlags::e_COMPUTE_BIT);
			verticalShaderModules[i] = pvr::utils::createShaderModule(device, verticalShaderString, pvrvk::ShaderStageFlags::e_COMPUTE_BIT);

			pipelineInfo.computeShader.setShader(horizontalShaderModules[i]);
			horizontalComputePipelines[i] = device->createComputePipeline(pipelineInfo, pipelineCache);

			pipelineInfo.computeShader.setShader(verticalShaderModules[i]);
			verticalComputePipelines[i] = device->createComputePipeline(pipelineInfo, pipelineCache);
		}
	}

	using GaussianBlurPass::recordCommands;
	void recordCommands(uint32_t swapchainIndex, pvr::Multi<pvrvk::ImageView>& horizontalBlurImages, pvr::Multi<pvrvk::ImageView>& verticalBlurImages, pvrvk::Queue& queue)
	{
		// horizontal
		{
			horizontalBlurCommandBuffers[swapchainIndex]->reset();
			horizontalBlurCommandBuffers[swapchainIndex]->begin();
			pvr::utils::beginCommandBufferDebugLabel(horizontalBlurCommandBuffers[swapchainIndex], pvrvk::DebugUtilsLabel("Compute Blur Horizontal"));
			horizontalBlurCommandBuffers[swapchainIndex]->bindPipeline(horizontalComputePipelines[currentKernelConfig]);
			horizontalBlurCommandBuffers[swapchainIndex]->bindDescriptorSet(pvrvk::PipelineBindPoint::e_COMPUTE, pipelineLayout, 0u, horizontalDescriptorSets[swapchainIndex]);

			// Draw a quad
			// dispatch x = image.height / 32
			// dispatch y = 1
			// dispatch z = 1
			horizontalBlurCommandBuffers[swapchainIndex]->dispatch(static_cast<uint32_t>(glm::ceil(horizontalBlurImages[swapchainIndex]->getImage()->getHeight() / 32.0f)), 1, 1);
			pvr::utils::endCommandBufferDebugLabel(horizontalBlurCommandBuffers[swapchainIndex]);

			pvrvk::MemoryBarrierSet layoutTransition;
			// Set up a barrier to pass the image from our horizontal compute shader to our vertical compute shader.
			layoutTransition.addBarrier(pvrvk::ImageMemoryBarrier(pvrvk::AccessFlags::e_SHADER_WRITE_BIT, pvrvk::AccessFlags::e_SHADER_READ_BIT,
				horizontalBlurImages[swapchainIndex]->getImage(), pvrvk::ImageSubresourceRange(pvrvk::ImageAspectFlags::e_COLOR_BIT), pvrvk::ImageLayout::e_GENERAL,
				pvrvk::ImageLayout::e_GENERAL, queue->getFamilyIndex(), queue->getFamilyIndex()));
			horizontalBlurCommandBuffers[swapchainIndex]->pipelineBarrier(
				pvrvk::PipelineStageFlags::e_COMPUTE_SHADER_BIT, pvrvk::PipelineStageFlags::e_COMPUTE_SHADER_BIT, layoutTransition);

			horizontalBlurCommandBuffers[swapchainIndex]->end();
		}

		// vertical
		{
			verticalBlurCommandBuffers[swapchainIndex]->reset();
			verticalBlurCommandBuffers[swapchainIndex]->begin();
			pvr::utils::beginCommandBufferDebugLabel(verticalBlurCommandBuffers[swapchainIndex], pvrvk::DebugUtilsLabel("Compute Blur Vertical"));
			verticalBlurCommandBuffers[swapchainIndex]->bindPipeline(verticalComputePipelines[currentKernelConfig]);
			verticalBlurCommandBuffers[swapchainIndex]->bindDescriptorSet(pvrvk::PipelineBindPoint::e_COMPUTE, pipelineLayout, 0u, verticalDescriptorSets[swapchainIndex]);

			// Draw a quad
			// dispatch x = image.width / 32
			// dispatch y = 1
			// dispatch z = 1
			verticalBlurCommandBuffers[swapchainIndex]->dispatch(static_cast<uint32_t>(glm::ceil(verticalBlurImages[swapchainIndex]->getImage()->getWidth() / 32.0f)), 1, 1);
			pvr::utils::endCommandBufferDebugLabel(verticalBlurCommandBuffers[swapchainIndex]);

			pvrvk::ImageLayout sourceImageLayout = pvrvk::ImageLayout::e_GENERAL;
			pvrvk::ImageLayout destinationImageLayout = pvrvk::ImageLayout::e_SHADER_READ_ONLY_OPTIMAL;

			pvrvk::MemoryBarrierSet layoutTransitions;
			layoutTransitions.addBarrier(pvrvk::ImageMemoryBarrier(pvrvk::AccessFlags::e_SHADER_WRITE_BIT, pvrvk::AccessFlags::e_SHADER_READ_BIT,
				horizontalBlurImages[swapchainIndex]->getImage(), pvrvk::ImageSubresourceRange(pvrvk::ImageAspectFlags::e_COLOR_BIT), sourceImageLayout, destinationImageLayout,
				queue->getFamilyIndex(), queue->getFamilyIndex()));
			layoutTransitions.addBarrier(pvrvk::ImageMemoryBarrier(pvrvk::AccessFlags::e_SHADER_WRITE_BIT, pvrvk::AccessFlags::e_SHADER_READ_BIT,
				verticalBlurImages[swapchainIndex]->getImage(), pvrvk::ImageSubresourceRange(pvrvk::ImageAspectFlags::e_COLOR_BIT), sourceImageLayout, destinationImageLayout,
				queue->getFamilyIndex(), queue->getFamilyIndex()));

			verticalBlurCommandBuffers[swapchainIndex]->pipelineBarrier(
				pvrvk::PipelineStageFlags::e_COMPUTE_SHADER_BIT, pvrvk::PipelineStageFlags::e_FRAGMENT_SHADER_BIT, layoutTransitions);

			verticalBlurCommandBuffers[swapchainIndex]->end();
		}
	}

	using GaussianBlurPass::recordCommandsToMainCommandBuffer;
	virtual void recordCommandsToMainCommandBuffer(uint32_t swapchainIndex, pvrvk::CommandBuffer& cmdBuffers)
	{
		cmdBuffers->executeCommands(horizontalBlurCommandBuffers[swapchainIndex]);
		cmdBuffers->executeCommands(verticalBlurCommandBuffers[swapchainIndex]);
	}
};

// A Linear sampler optimised Gaussian Blur Pass
struct LinearGaussianBlurPass : public GaussianBlurPass
{
	void generatePerConfigGaussianCoefficients() override
	{
		for (uint32_t i = 0; i < DemoConfigurations::NumDemoConfigurations; ++i)
		{ generateGaussianCoefficients(DemoConfigurations::Configurations[i].linearGaussianConfig, true, false, gaussianWeights[i], gaussianOffsets[i]); }
	}

	/// <summary>Generates the Gaussian weights and offsets strings used by the various Gaussian shaders.</summary>
	void generateGaussianShaderStrings() override
	{
		// Generate per kernel size weights, offsets and iterations strings
		perKernelSizeIterationsStrings.resize(DemoConfigurations::NumDemoConfigurations);
		perKernelSizeWeightsStrings.resize(DemoConfigurations::NumDemoConfigurations);
		perKernelSizeOffsetsStrings.resize(DemoConfigurations::NumDemoConfigurations);

		for (uint32_t i = 0; i < DemoConfigurations::NumDemoConfigurations; ++i)
		{
			generateGaussianWeightsAndOffsetsStrings(
				gaussianWeights[i], gaussianOffsets[i], perKernelSizeIterationsStrings[i], perKernelSizeWeightsStrings[i], perKernelSizeOffsetsStrings[i]);
		}
		inverseFramebufferWidthString = pvr::strings::createFormatted("const highp float inverseFramebufferWidth = %.15f;", inverseFramebufferWidth);
		inverseFramebufferHeightString = pvr::strings::createFormatted("const highp float inverseFramebufferHeight = %.15f;", inverseFramebufferHeight);
	}

	void createPipelines(pvr::IAssetProvider& assetProvider, pvrvk::Device& device, pvrvk::RenderPass& renderPass, const glm::uvec2& blurFramebufferDimensions,
		pvrvk::PipelineCache& pipelineCache) override
	{
		// Vertex Shader Modules
		pvrvk::ShaderModule horizontalVertexShaderModules[DemoConfigurations::NumDemoConfigurations];
		pvrvk::ShaderModule verticalVertexShaderModules[DemoConfigurations::NumDemoConfigurations];

		// Fragment Shader Module
		pvrvk::ShaderModule fragShaderModules[DemoConfigurations::NumDemoConfigurations];

		// Generate the Gaussian blur shader modules
		for (uint32_t i = 0; i < DemoConfigurations::NumDemoConfigurations; ++i)
		{
			// Load the linear optimised Gaussian vertex shader
			auto vertexShaderStream = assetProvider.getAssetStream(Files::LinearGaussianVertSrcFile);

			// Load the linear optimised Gaussian vertex shader into a string
			// At this point the vertex shader is missing its templated arguments and won't compile as is
			std::string vertexShaderSource;
			vertexShaderStream->readIntoString(vertexShaderSource);

			// Insert the templates into the linear optimised vertex shader
			// The linear optimised Gaussian vertex shaders require the number of iterations, the offsets for each iteration, the number of texture coordinates
			// and the direction to sample
			std::string horizontalShaderString = pvr::strings::createFormatted(vertexShaderSource.c_str(), perKernelSizeIterationsStrings[i].c_str(),
				perKernelSizeOffsetsStrings[i].c_str(), inverseFramebufferWidthString.c_str(), inverseFramebufferHeightString.c_str(), "1.0, 0.0");
			std::string verticalShaderString = pvr::strings::createFormatted(vertexShaderSource.c_str(), perKernelSizeIterationsStrings[i].c_str(),
				perKernelSizeOffsetsStrings[i].c_str(), inverseFramebufferWidthString.c_str(), inverseFramebufferHeightString.c_str(), "0.0, 1.0");

			// Linear Optimised Gaussian Blur Vertex Shaders
			// Create shader modules for the horizontal and vertical linearly optimised Gaussian blur vertex shaders
			horizontalVertexShaderModules[i] = pvr::utils::createShaderModule(device, horizontalShaderString, pvrvk::ShaderStageFlags::e_VERTEX_BIT);
			verticalVertexShaderModules[i] = pvr::utils::createShaderModule(device, verticalShaderString, pvrvk::ShaderStageFlags::e_VERTEX_BIT);

			// Load the linear optimised Gaussian fragment shader
			auto fragmentShaderStream = assetProvider.getAssetStream(Files::LinearGaussianFragSrcFile);

			// Load the linear optimised Gaussian fragment shader into a string
			// At this point the fragment shader is missing its templated arguments and won't compile as is
			std::string fragShaderSource;
			fragmentShaderStream->readIntoString(fragShaderSource);

			// Insert the templates into the fragment shader
			// The linear optimised Gaussian fragment shader requires the number of iterations, the weights for each iteration and the number of texture coordinates
			std::string fragmentShaderString =
				pvr::strings::createFormatted(fragShaderSource.c_str(), perKernelSizeIterationsStrings[i].c_str(), perKernelSizeWeightsStrings[i].c_str());

			// Linear Optimised Gaussian Blur Fragment Shader
			// Create a shader module for the Linear optimised Gaussian blur fragment shader
			fragShaderModules[i] = pvr::utils::createShaderModule(device, fragmentShaderString, pvrvk::ShaderStageFlags::e_FRAGMENT_BIT);
		}

		pvrvk::GraphicsPipelineCreateInfo pipelineInfo;
		pipelineInfo.viewport.setViewportAndScissor(0u, pvrvk::Viewport(0.0f, 0.0f, static_cast<float>(blurFramebufferDimensions.x), static_cast<float>(blurFramebufferDimensions.y)),
			pvrvk::Rect2D(0, 0, blurFramebufferDimensions.x, blurFramebufferDimensions.y));

		pipelineInfo.rasterizer.setCullMode(pvrvk::CullModeFlags::e_FRONT_BIT);
		pipelineInfo.rasterizer.setFrontFaceWinding(pvrvk::FrontFace::e_COUNTER_CLOCKWISE);

		// disable depth writing and depth testing
		pipelineInfo.depthStencil.enableDepthWrite(false);
		pipelineInfo.depthStencil.enableDepthTest(false);

		// disable stencil testing
		pipelineInfo.depthStencil.enableStencilTest(false);

		pipelineInfo.colorBlend.setAttachmentState(0, pvrvk::PipelineColorBlendAttachmentState());

		pipelineInfo.vertexInput.clear();
		pipelineInfo.inputAssembler.setPrimitiveTopology(pvrvk::PrimitiveTopology::e_TRIANGLE_STRIP);

		pipelineInfo.pipelineLayout = pipelineLayout;

		// renderpass/subpass
		pipelineInfo.renderPass = renderPass;
		pipelineInfo.subpass = 0;

		for (uint32_t i = 0; i < DemoConfigurations::NumDemoConfigurations; ++i)
		{
			// Set common shader constants for the vertex shaders
			pipelineInfo.vertexShader.setShaderConstant(0, pvrvk::ShaderConstantInfo(0, &blurFramebufferDimensions.x, static_cast<uint32_t>(pvr::getSize(pvr::GpuDatatypes::uinteger))));
			pipelineInfo.vertexShader.setShaderConstant(1, pvrvk::ShaderConstantInfo(1, &blurFramebufferDimensions.y, static_cast<uint32_t>(pvr::getSize(pvr::GpuDatatypes::uinteger))));

			pipelineInfo.fragmentShader.setShader(fragShaderModules[i]);

			pipelineInfo.vertexShader.setShader(horizontalVertexShaderModules[i]);
			horizontalPipelines[i] = device->createGraphicsPipeline(pipelineInfo, pipelineCache);

			pipelineInfo.vertexShader.setShader(verticalVertexShaderModules[i]);
			verticalPipelines[i] = device->createGraphicsPipeline(pipelineInfo, pipelineCache);
		}
	}

	void recordCommands(uint32_t swapchainIndex, pvr::Multi<pvrvk::Framebuffer>& horizontalBlurFramebuffers, pvr::Multi<pvrvk::Framebuffer>& verticalBlurFramebuffers) override
	{
		// Record the commands to use for carrying out the horizontal Gaussian blur pass
		{
			horizontalBlurCommandBuffers[swapchainIndex]->begin(horizontalBlurFramebuffers[swapchainIndex], 0, pvrvk::CommandBufferUsageFlags::e_RENDER_PASS_CONTINUE_BIT);

			pvr::utils::beginCommandBufferDebugLabel(horizontalBlurCommandBuffers[swapchainIndex],
				pvrvk::DebugUtilsLabel(pvr::strings::createFormatted("Linear Gaussian Blur (horizontal) - swapchain (%i)", swapchainIndex)));

			horizontalBlurCommandBuffers[swapchainIndex]->bindPipeline(horizontalPipelines[currentKernelConfig]);

			horizontalBlurCommandBuffers[swapchainIndex]->bindDescriptorSet(pvrvk::PipelineBindPoint::e_GRAPHICS, pipelineLayout, 0u, horizontalDescriptorSets[swapchainIndex]);
			horizontalBlurCommandBuffers[swapchainIndex]->draw(0, 3);
			pvr::utils::endCommandBufferDebugLabel(horizontalBlurCommandBuffers[swapchainIndex]);
			horizontalBlurCommandBuffers[swapchainIndex]->end();
		}

		// Record the commands to use for carrying out the vertical Gaussian blur pass
		{
			verticalBlurCommandBuffers[swapchainIndex]->begin(verticalBlurFramebuffers[swapchainIndex], 0, pvrvk::CommandBufferUsageFlags::e_RENDER_PASS_CONTINUE_BIT);

			pvr::utils::beginCommandBufferDebugLabel(verticalBlurCommandBuffers[swapchainIndex],
				pvrvk::DebugUtilsLabel(pvr::strings::createFormatted("Linear Gaussian Blur (vertical) - swapchain (%i)", swapchainIndex)));

			verticalBlurCommandBuffers[swapchainIndex]->bindPipeline(verticalPipelines[currentKernelConfig]);

			verticalBlurCommandBuffers[swapchainIndex]->bindDescriptorSet(pvrvk::PipelineBindPoint::e_GRAPHICS, pipelineLayout, 0u, verticalDescriptorSets[swapchainIndex]);

			verticalBlurCommandBuffers[swapchainIndex]->draw(0, 3);
			pvr::utils::endCommandBufferDebugLabel(verticalBlurCommandBuffers[swapchainIndex]);
			verticalBlurCommandBuffers[swapchainIndex]->end();
		}
	}
	virtual ~LinearGaussianBlurPass() {}
};

// A Truncated Linear sampler optimised Gaussian Blur Pass
struct TruncatedLinearGaussianBlurPass : public LinearGaussianBlurPass
{
	void generatePerConfigGaussianCoefficients() override
	{
		for (uint32_t i = 0; i < DemoConfigurations::NumDemoConfigurations; ++i)
		{ generateGaussianCoefficients(DemoConfigurations::Configurations[i].truncatedLinearGaussianConfig, true, true, gaussianWeights[i], gaussianOffsets[i]); }
	}
	virtual ~TruncatedLinearGaussianBlurPass(){};
};

// A Hybrid Gaussian Blur pass making use of a horizontal Compute shader pass followed by a Fragment based Vertical Gaussian Blur Pass
struct HybridGaussianBlurPass
{
	// The Compute shader based Gaussian Blur pass - we will only be making use of the horizontal blur resources
	ComputeBlurPass* computeBlurPass;
	// The Fragment shader based Gaussian Blur pass - we will only be making use of the vertical blur resources
	TruncatedLinearGaussianBlurPass* linearBlurPass;

	// Command buffers for the horizontal and vertical blur passes
	pvr::Multi<pvrvk::SecondaryCommandBuffer> horizontalBlurCommandBuffers;
	pvr::Multi<pvrvk::SecondaryCommandBuffer> verticalBlurCommandBuffers;

	/// <summary>A minimal initialisation function as no extra resources are created for this type of blur pass and instead we make use of the compute and fragment based passes.</summary>
	/// <param name="inComputeBlurPass">The Compute shader based Gaussian Blur pass - we will only be making use of the horizontal blur resources.</param>
	/// <param name="inLinearBlurPass">The Fragment shader based Gaussian Blur pass - we will only be making use of the vertical blur resources.</param>
	/// <param name="swapchain">The swapchain which will determine the number of per swapchain resources to allocate.</param>
	/// <param name="commandPool">The command pool from which to allocate command buffers.</param>
	void init(ComputeBlurPass* inComputeBlurPass, TruncatedLinearGaussianBlurPass* inLinearBlurPass, pvrvk::Swapchain& swapchain, pvrvk::CommandPool& commandPool)
	{
		this->computeBlurPass = inComputeBlurPass;
		this->linearBlurPass = inLinearBlurPass;

		for (uint32_t i = 0; i < swapchain->getSwapchainLength(); ++i)
		{
			horizontalBlurCommandBuffers.add(commandPool->allocateSecondaryCommandBuffer());
			verticalBlurCommandBuffers.add(commandPool->allocateSecondaryCommandBuffer());
		}
	}

	/// <summary>Records the commands required for the Hybrid Gaussian blur.</summary>
	/// <param name="swapchainIndex">The swapchain index for which the recorded command buffer will be used.</param>
	/// <param name="horizontalBlurFramebuffers">The framebuffers to use in the horizontal blur pass.</param>
	/// <param name="verticalBlurFramebuffers">The framebuffers to use in the vertical blur pass.</param>
	/// <param name="queue">The queue to which the command buffer will be submitted.</param>
	void recordCommands(uint32_t swapchainIndex, pvr::Multi<pvrvk::Framebuffer>& horizontalBlurFramebuffers, pvr::Multi<pvrvk::Framebuffer>& verticalBlurFramebuffers, pvrvk::Queue& queue)
	{
		// horizontal compute based Gaussian blur pass
		{
			horizontalBlurCommandBuffers[swapchainIndex]->reset();
			horizontalBlurCommandBuffers[swapchainIndex]->begin();
			pvr::utils::beginCommandBufferDebugLabel(horizontalBlurCommandBuffers[swapchainIndex], pvrvk::DebugUtilsLabel("Compute Blur Horizontal"));
			horizontalBlurCommandBuffers[swapchainIndex]->bindPipeline(computeBlurPass->horizontalComputePipelines[computeBlurPass->currentKernelConfig]);
			horizontalBlurCommandBuffers[swapchainIndex]->bindDescriptorSet(
				pvrvk::PipelineBindPoint::e_COMPUTE, computeBlurPass->pipelineLayout, 0u, computeBlurPass->horizontalDescriptorSets[swapchainIndex]);

			// Draw a quad
			// dispatch x = image.height / 32
			// dispatch y = 1
			// dispatch z = 1
			horizontalBlurCommandBuffers[swapchainIndex]->dispatch(
				static_cast<uint32_t>(glm::ceil(horizontalBlurFramebuffers[swapchainIndex]->getDimensions().getHeight() / 32.0f)), 1, 1);
			pvr::utils::endCommandBufferDebugLabel(horizontalBlurCommandBuffers[swapchainIndex]);

			pvrvk::MemoryBarrierSet layoutTransition;
			// Set up a barrier to pass the image from our horizontal compute shader to our vertical fragment shader.
			layoutTransition.addBarrier(pvrvk::ImageMemoryBarrier(pvrvk::AccessFlags::e_SHADER_WRITE_BIT, pvrvk::AccessFlags::e_SHADER_READ_BIT,
				horizontalBlurFramebuffers[swapchainIndex]->getAttachment(0)->getImage(), pvrvk::ImageSubresourceRange(pvrvk::ImageAspectFlags::e_COLOR_BIT),
				pvrvk::ImageLayout::e_GENERAL, pvrvk::ImageLayout::e_SHADER_READ_ONLY_OPTIMAL, queue->getFamilyIndex(), queue->getFamilyIndex()));
			horizontalBlurCommandBuffers[swapchainIndex]->pipelineBarrier(
				pvrvk::PipelineStageFlags::e_COMPUTE_SHADER_BIT, pvrvk::PipelineStageFlags::e_FRAGMENT_SHADER_BIT, layoutTransition);

			horizontalBlurCommandBuffers[swapchainIndex]->end();
		}

		// vertical fragment based Gaussian blur pass
		{
			verticalBlurCommandBuffers[swapchainIndex]->reset();
			verticalBlurCommandBuffers[swapchainIndex]->begin(verticalBlurFramebuffers[swapchainIndex], 0, pvrvk::CommandBufferUsageFlags::e_RENDER_PASS_CONTINUE_BIT);
			pvr::utils::beginCommandBufferDebugLabel(verticalBlurCommandBuffers[swapchainIndex],
				pvrvk::DebugUtilsLabel(pvr::strings::createFormatted("Linear Gaussian Blur (vertical) - swapchain (%i)", swapchainIndex)));

			verticalBlurCommandBuffers[swapchainIndex]->bindPipeline(linearBlurPass->verticalPipelines[linearBlurPass->currentKernelConfig]);

			verticalBlurCommandBuffers[swapchainIndex]->bindDescriptorSet(
				pvrvk::PipelineBindPoint::e_GRAPHICS, linearBlurPass->pipelineLayout, 0u, linearBlurPass->verticalDescriptorSets[swapchainIndex]);

			verticalBlurCommandBuffers[swapchainIndex]->draw(0, 3);
			pvr::utils::endCommandBufferDebugLabel(verticalBlurCommandBuffers[swapchainIndex]);
			verticalBlurCommandBuffers[swapchainIndex]->end();
		}
	}

	/// <summary>Records the command buffers into the given main rendering command buffer.</summary>
	/// <param name="swapchainIndex">The swapchain index for which the recorded command buffer will be used.</param>
	/// <param name="cmdBuffers">The main command buffer into which the pre-recorded command buffers will be recorded into.</param>
	/// <param name="blurRenderPass">The RenderPass to use for the blur.</param>
	/// <param name="horizontalBlurFramebuffers">The framebuffers to use in the horizontal blur pass.</param>
	/// <param name="verticalBlurFramebuffers">The framebuffers to use in the vertical blur pass.</param>
	void recordCommandsToMainCommandBuffer(uint32_t swapchainIndex, pvrvk::CommandBuffer& cmdBuffers, pvrvk::RenderPass& blurRenderPass,
		pvr::Multi<pvrvk::Framebuffer>& horizontalBlurFramebuffers, pvr::Multi<pvrvk::Framebuffer>& verticalBlurFramebuffers)
	{
		(void)horizontalBlurFramebuffers;
		// Compute horizontal pass
		cmdBuffers->executeCommands(horizontalBlurCommandBuffers[swapchainIndex]);

		// Fragment vertical pass
		pvrvk::ClearValue clearValue = pvrvk::ClearValue(0.0f, 0.0f, 0.0f, 1.f);
		cmdBuffers->beginRenderPass(verticalBlurFramebuffers[swapchainIndex], blurRenderPass,
			pvrvk::Rect2D(0, 0, verticalBlurFramebuffers[swapchainIndex]->getDimensions().getWidth(), verticalBlurFramebuffers[swapchainIndex]->getDimensions().getHeight()), false,
			&clearValue, 1);
		cmdBuffers->executeCommands(verticalBlurCommandBuffers[swapchainIndex]);
		cmdBuffers->endRenderPass();
	}
};

// Post bloom composition pass
struct PostBloomPass
{
	pvrvk::PipelineLayout pipelineLayout;
	pvrvk::GraphicsPipeline defaultPipeline;
	pvrvk::GraphicsPipeline bloomOnlyPipeline;
	pvrvk::DescriptorSetLayout descriptorSetLayout;
	pvr::Multi<pvrvk::DescriptorSet> descriptorSets;
	pvr::Multi<pvrvk::SecondaryCommandBuffer> cmdBuffers;

	/// <summary>Initialises the Post Bloom Pass.</summary>
	/// <param name="assetProvider">The pvr::IAssetProvider which will be used for loading resources from memory.</param>
	/// <param name="device">The device from which the resources will be allocated.</param>
	/// <param name="swapchain">The swapchain which will determine the number of per swapchain resources to allocate.</param>
	/// <param name="commandPool">The command pool from which to allocate command buffers.</param>
	/// <param name="descriptorPool">The descriptor pool from which to allocate descriptor sets.</param>
	/// <param name="vmaAllocator">A VMA allocator to use for allocating images and buffers.</param>
	/// <param name="renderPass">The RenderPass to use.</param>
	/// <param name="pipelineCache">A pipeline cache object to use when creating pipelines.</param>
	void init(pvr::IAssetProvider& assetProvider, pvrvk::Device& device, pvrvk::Swapchain& swapchain, pvrvk::CommandPool& commandPool, pvrvk::DescriptorPool& descriptorPool,
		pvr::utils::vma::Allocator& vmaAllocator, pvrvk::RenderPass& renderPass, pvrvk::PipelineCache& pipelineCache)
	{
		(void)vmaAllocator;
		createDescriptorSetLayout(device);
		createDescriptorSets(swapchain, descriptorPool);
		createPipeline(assetProvider, device, swapchain, renderPass, pipelineCache);

		for (uint32_t i = 0; i < swapchain->getSwapchainLength(); ++i) { cmdBuffers.add(commandPool->allocateSecondaryCommandBuffer()); }
	}

	/// <summary>Creates the descriptor set layout.</summary>
	/// <param name="device">The device from which the descriptor set layouts will be allocated.</param>
	void createDescriptorSetLayout(pvrvk::Device& device)
	{
		pvrvk::DescriptorSetLayoutCreateInfo descSetInfo;
		descSetInfo.setBinding(0, pvrvk::DescriptorType::e_COMBINED_IMAGE_SAMPLER, 1u, pvrvk::ShaderStageFlags::e_FRAGMENT_BIT);
		descSetInfo.setBinding(1, pvrvk::DescriptorType::e_COMBINED_IMAGE_SAMPLER, 1u, pvrvk::ShaderStageFlags::e_FRAGMENT_BIT);
		descSetInfo.setBinding(2, pvrvk::DescriptorType::e_COMBINED_IMAGE_SAMPLER, 1u, pvrvk::ShaderStageFlags::e_FRAGMENT_BIT);
		descSetInfo.setBinding(3, pvrvk::DescriptorType::e_UNIFORM_BUFFER, 1u, pvrvk::ShaderStageFlags::e_VERTEX_BIT);

		descriptorSetLayout = device->createDescriptorSetLayout(descSetInfo);

		pvrvk::PipelineLayoutCreateInfo pipeLayoutInfo;
		pipeLayoutInfo.setDescSetLayout(0, descriptorSetLayout);

		pvrvk::PushConstantRange pushConstantsRange;
		pushConstantsRange.setOffset(0);
		pushConstantsRange.setSize(static_cast<uint32_t>(pvr::getSize(pvr::GpuDatatypes::Float)));
		pushConstantsRange.setStageFlags(pvrvk::ShaderStageFlags::e_FRAGMENT_BIT);
		pipeLayoutInfo.setPushConstantRange(0, pushConstantsRange);

		pipelineLayout = device->createPipelineLayout(pipeLayoutInfo);
	}

	/// <summary>Creates the descriptor sets.</summary>
	/// <param name="swapchain">The swapchain which will determine the number of per swapchain resources to allocate.</param>
	/// <param name="descriptorPool">The descriptor pool from which to allocate descriptor sets.</param>
	void createDescriptorSets(pvrvk::Swapchain& swapchain, pvrvk::DescriptorPool& descriptorPool)
	{
		for (uint32_t i = 0; i < swapchain->getSwapchainLength(); ++i) { descriptorSets.add(descriptorPool->allocateDescriptorSet(descriptorSetLayout)); }
	}

	/// <summary>Updates the descriptor sets used based on the original image and bloomed image.</summary>
	/// <param name="device">The device from which the updateDescriptorSets call will be piped.</param>
	/// <param name="swapchainIndex">The swapchain index of the descriptor set to update.</param>
	/// <param name="originalImageView">The original unblurred image view.</param>
	/// <param name="blurredImageView">The blurred image view.</param>
	/// <param name="bilinearSampler">The sampler object to use when sampling from the images.</param>
	/// <param name="sceneBufferView">Buffer view for the scene buffer.</param>
	/// <param name="sceneBuffer">The scene buffer.</param>
	/// <param name="diffuseIrradianceMap">The diffuse irradiance map used as a replacement to a fixed albedo.</param>
	/// <param name="samplerTrilinear">A trilinear sampler object.</param>
	void updateDescriptorSets(pvrvk::Device& device, uint32_t swapchainIndex, pvrvk::ImageView& originalImageView, pvrvk::ImageView& blurredImageView, pvrvk::Sampler& bilinearSampler,
		pvrvk::Buffer& sceneBuffer, pvr::utils::StructuredBufferView& sceneBufferView, pvrvk::ImageView& diffuseIrradianceMap, pvrvk::Sampler& samplerTrilinear)
	{
		std::vector<pvrvk::WriteDescriptorSet> writeDescSets;

		writeDescSets.push_back(pvrvk::WriteDescriptorSet(pvrvk::DescriptorType::e_COMBINED_IMAGE_SAMPLER, descriptorSets[swapchainIndex], 0)
									.setImageInfo(0, pvrvk::DescriptorImageInfo(blurredImageView, bilinearSampler, pvrvk::ImageLayout::e_SHADER_READ_ONLY_OPTIMAL)));

		writeDescSets.push_back(pvrvk::WriteDescriptorSet(pvrvk::DescriptorType::e_COMBINED_IMAGE_SAMPLER, descriptorSets[swapchainIndex], 1)
									.setImageInfo(0, pvrvk::DescriptorImageInfo(originalImageView, bilinearSampler, pvrvk::ImageLayout::e_SHADER_READ_ONLY_OPTIMAL)));

		writeDescSets.push_back(pvrvk::WriteDescriptorSet(pvrvk::DescriptorType::e_COMBINED_IMAGE_SAMPLER, descriptorSets[swapchainIndex], 2)
									.setImageInfo(0, pvrvk::DescriptorImageInfo(diffuseIrradianceMap, samplerTrilinear, pvrvk::ImageLayout::e_SHADER_READ_ONLY_OPTIMAL)));

		writeDescSets.push_back(
			pvrvk::WriteDescriptorSet(pvrvk::DescriptorType::e_UNIFORM_BUFFER, descriptorSets[swapchainIndex], 3)
				.setBufferInfo(0, pvrvk::DescriptorBufferInfo(sceneBuffer, sceneBufferView.getDynamicSliceOffset(swapchainIndex), sceneBufferView.getDynamicSliceSize())));

		device->updateDescriptorSets(writeDescSets.data(), static_cast<uint32_t>(writeDescSets.size()), nullptr, 0);
	}

	/// <summary>Creates the Post bloom pipeline.</summary>
	/// <param name="assetProvider">The pvr::IAssetProvider which will be used for loading resources from memory.</param>
	/// <param name="swapchain">The swapchain which will determine the number of per swapchain resources to allocate.</param>
	/// <param name="renderPass">The RenderPass to use.</param>
	/// <param name="pipelineCache">A pipeline cache object to use when creating pipelines.</param>
	void createPipeline(pvr::IAssetProvider& assetProvider, pvrvk::Device& device, pvrvk::Swapchain& swapchain, pvrvk::RenderPass& renderPass, pvrvk::PipelineCache& pipelineCache)
	{
		pvrvk::GraphicsPipelineCreateInfo pipelineInfo;
		pipelineInfo.viewport.setViewportAndScissor(0,
			pvrvk::Viewport(0.0f, 0.0f, static_cast<float>(swapchain->getDimension().getWidth()), static_cast<float>(swapchain->getDimension().getHeight())),
			pvrvk::Rect2D(0, 0, swapchain->getDimension().getWidth(), swapchain->getDimension().getHeight()));

		pipelineInfo.rasterizer.setCullMode(pvrvk::CullModeFlags::e_FRONT_BIT);
		pipelineInfo.rasterizer.setFrontFaceWinding(pvrvk::FrontFace::e_COUNTER_CLOCKWISE);

		// disable depth writing and depth testing
		pipelineInfo.depthStencil.enableDepthWrite(false);
		pipelineInfo.depthStencil.enableDepthTest(false);

		// disable stencil testing
		pipelineInfo.depthStencil.enableStencilTest(false);

		pipelineInfo.colorBlend.setAttachmentState(0, pvrvk::PipelineColorBlendAttachmentState());

		// load and create appropriate shaders
		pipelineInfo.vertexShader.setShader(
			device->createShaderModule(pvrvk::ShaderModuleCreateInfo(assetProvider.getAssetStream(Files::PostBloomVertShaderSrcFile)->readToEnd<uint32_t>())));
		pipelineInfo.fragmentShader.setShader(
			device->createShaderModule(pvrvk::ShaderModuleCreateInfo(assetProvider.getAssetStream(Files::PostBloomFragShaderSrcFile)->readToEnd<uint32_t>())));

		pipelineInfo.vertexInput.clear();
		pipelineInfo.inputAssembler.setPrimitiveTopology(pvrvk::PrimitiveTopology::e_TRIANGLE_STRIP);

		pipelineInfo.pipelineLayout = pipelineLayout;

		// renderpass/subpass
		pipelineInfo.renderPass = renderPass;
		pipelineInfo.subpass = 0;

		defaultPipeline = device->createGraphicsPipeline(pipelineInfo, pipelineCache);

		// Enable bloom only
		int enabled = 1;
		pipelineInfo.fragmentShader.setShaderConstant(0, pvrvk::ShaderConstantInfo(0, &enabled, static_cast<uint32_t>(pvr::getSize(pvr::GpuDatatypes::Integer))));
		bloomOnlyPipeline = device->createGraphicsPipeline(pipelineInfo, pipelineCache);
	}

	/// <summary>Records the secondary command buffers for the post bloom composition pass.</summary>
	/// <param name="swapchainIndex">The swapchain index for which the recorded command buffer will be used.</param>
	/// <param name="framebuffer">The framebuffer to render into</param>
	/// <param name="renderBloomOnly">Indicates that only the bloom should be rendered to the screen</param>
	/// <param name="exposure">The exposure to use in the tone mapping.</param>
	void recordCommandBuffer(uint32_t swapchainIndex, pvrvk::Framebuffer& framebuffer, bool renderBloomOnly, float exposure)
	{
		cmdBuffers[swapchainIndex]->begin(framebuffer, 0, pvrvk::CommandBufferUsageFlags::e_RENDER_PASS_CONTINUE_BIT);
		pvr::utils::beginCommandBufferDebugLabel(cmdBuffers[swapchainIndex], pvrvk::DebugUtilsLabel("PostBloom"));
		if (renderBloomOnly) { cmdBuffers[swapchainIndex]->bindPipeline(bloomOnlyPipeline); }
		else
		{
			cmdBuffers[swapchainIndex]->bindPipeline(defaultPipeline);
		}
		cmdBuffers[swapchainIndex]->pushConstants(pipelineLayout, pvrvk::ShaderStageFlags::e_FRAGMENT_BIT, 0, static_cast<uint32_t>(pvr::getSize(pvr::GpuDatatypes::Float)), &exposure);
		cmdBuffers[swapchainIndex]->bindDescriptorSet(pvrvk::PipelineBindPoint::e_GRAPHICS, pipelineLayout, 0u, descriptorSets[swapchainIndex]);
		cmdBuffers[swapchainIndex]->draw(0, 3);
		pvr::utils::endCommandBufferDebugLabel(cmdBuffers[swapchainIndex]);
		cmdBuffers[swapchainIndex]->end();
	}
};

struct DeviceResources
{
	pvrvk::Instance instance;
	pvr::utils::DebugUtilsCallbacks debugUtilsCallbacks;
	pvrvk::Device device;
	pvrvk::DescriptorPool descriptorPool;
	pvrvk::CommandPool commandPool;
	pvrvk::Swapchain swapchain;
	pvr::utils::vma::Allocator vmaAllocator;
	pvrvk::Queue queues[2];
	pvrvk::PipelineCache pipelineCache;

	// On screen resources
	pvr::Multi<pvrvk::Framebuffer> onScreenFramebuffers;
	pvrvk::RenderPass onScreenRenderPass;

	// Off screen resources
	pvr::Multi<pvrvk::Framebuffer> offScreenFramebuffers;
	pvrvk::RenderPass offScreenRenderPass;
	pvr::Multi<pvrvk::ImageView> depthStencilImages;

	// Synchronisation primitives
	pvrvk::Semaphore imageAcquiredSemaphores[static_cast<uint32_t>(pvrvk::FrameworkCaps::MaxSwapChains)];
	pvrvk::Semaphore presentationSemaphores[static_cast<uint32_t>(pvrvk::FrameworkCaps::MaxSwapChains)];
	pvrvk::Fence perFrameResourcesFences[static_cast<uint32_t>(pvrvk::FrameworkCaps::MaxSwapChains)];

	// Textures
	pvr::Multi<pvrvk::ImageView> luminanceImageViews;
	pvr::Multi<pvrvk::ImageView> offScreenColorImageViews;
	pvrvk::ImageView diffuseIrradianceMapImageViews[NumScenes];

	std::vector<pvr::Multi<pvrvk::ImageView>> sharedBlurImageViews;

	// Bloom resources
	pvrvk::RenderPass blurRenderPass;
	pvrvk::RenderPass hybridBlurRenderPass;
	pvr::Multi<pvrvk::Framebuffer> blurFramebuffers[2];
	pvr::Multi<pvrvk::Framebuffer> hybridBlurFramebuffers[2];

	// Samplers
	pvrvk::Sampler samplerNearest;
	pvrvk::Sampler samplerBilinear;
	pvrvk::Sampler samplerTrilinear;

	// Command Buffers
	pvr::Multi<pvrvk::CommandBuffer> mainCommandBuffers;
	pvrvk::CommandBuffer utilityCommandBuffer;
	pvr::Multi<pvrvk::SecondaryCommandBuffer> bloomUiRendererCommandBuffers;
	pvr::Multi<pvrvk::SecondaryCommandBuffer> uiRendererCommandBuffers;

	// Passes
	StatuePass statuePass;
	SkyboxPass skyBoxPass;
	DownSamplePass downsamplePass;
	DownSamplePass computeDownsamplePass;
	GaussianBlurPass gaussianBlurPass;
	LinearGaussianBlurPass linearGaussianBlurPass;
	TruncatedLinearGaussianBlurPass truncatedLinearGaussianBlurPass;
	DualFilterBlurPass dualFilterBlurPass;
	DownAndTentFilterBlurPass downAndTentFilterBlurPass;
	ComputeBlurPass computeBlurPass;
	HybridGaussianBlurPass hybridGaussianBlurPass;
	KawaseBlurPass kawaseBlurPass;
	PostBloomPass postBloomPass;

	// UIRenderers used to display text
	pvr::ui::UIRenderer uiRenderer;

	// Buffers and their views
	pvr::utils::StructuredBufferView sceneBufferView;
	pvrvk::Buffer sceneBuffer;

	~DeviceResources()
	{
		if (device)
		{
			device->waitIdle();
			uint32_t l = swapchain->getSwapchainLength();
			for (uint32_t i = 0; i < l; ++i)
			{
				if (perFrameResourcesFences[i]) perFrameResourcesFences[i]->wait();
			}
		}
	}
};

/// <summary>Class implementing the Shell functions.</summary>
class VulkanPostProcessing : public pvr::Shell
{
	std::unique_ptr<DeviceResources> _deviceResources;

	pvrvk::Format _luminanceColorFormat;
	pvrvk::Format _storageImageLuminanceColorFormat;
	pvrvk::ImageTiling _storageImageTiling;

	glm::uvec2 _blurFramebufferDimensions;
	glm::vec2 _blurInverseFramebufferDimensions;
	uint32_t _blurScale;

	// Synchronisation counters
	uint32_t _swapchainIndex;
	uint32_t _frameId;
	uint32_t _queueIndex;

	bool _animateObject;
	bool _animateCamera;
	float _objectAngleY;
	float _cameraAngle;
	pvr::TPSCamera _camera;
	float _logicTime;
	float _modeSwitchTime;
	bool _isManual;
	float _modeDuration;

	glm::mat4 _viewMatrix;
	glm::mat4 _projectionMatrix;
	glm::mat4 _viewProjectionMatrix;

	BloomMode _blurMode;

	uint32_t _currentDemoConfiguration;
	uint32_t _currentScene;

	bool _mustRecordPrimaryCommandBuffer[static_cast<uint32_t>(pvrvk::FrameworkCaps::MaxSwapChains)];
	bool _mustUpdatePerSwapchainDemoConfig[static_cast<uint32_t>(pvrvk::FrameworkCaps::MaxSwapChains)];

	bool _renderOnlyBloom;

	std::string _currentBlurString;

	bool _useMultiQueue;

	float _exposure;
	float _threshold;
	size_t _pingPongImageIndices[2];

public:
	VulkanPostProcessing() {}

	virtual pvr::Result initApplication();
	virtual pvr::Result initView();
	virtual pvr::Result releaseView();
	virtual pvr::Result quitApplication();
	virtual pvr::Result renderFrame();

	void recordBlurCommands(BloomMode blurMode, uint32_t swapchainIndex);
	void createSceneBuffers();
	void createBlurRenderPass();
	void createHybridBlurRenderPass();
	void createBlurFramebuffers();
	void createHybridBlurFramebuffers();
	void createSamplers();
	void allocatePingPongImages();
	void createOffScreenFramebuffers();
	void createUiRenderer();
	void updateBlurDescription();
	void recordUIRendererCommands(uint32_t swapchainIndex, pvr::Multi<pvrvk::SecondaryCommandBuffer>& commandBuffers);
	void updateDemoConfigs();
	void handleDesktopInput();
	void eventMappedInput(pvr::SimplifiedInput e);
	void updateBloomConfiguration();
	void updateAnimation();
	void updateDynamicSceneData();
	void recordMainCommandBuffer(uint32_t swapchainIndex);
};

/// <summary>Code in initApplication() will be called by Shell once per run, before the rendering context is created.
/// Used to initialize variables that are not dependent on it (e.g. external modules, loading meshes, etc.) If the rendering
/// context is lost, initApplication() will not be called again.</summary>
/// <returns>Result::Success if no error occurred.</returns>
pvr::Result VulkanPostProcessing::initApplication()
{
	this->setStencilBitsPerPixel(0);

	// Default demo properties
	_animateObject = true;
	_animateCamera = false;
	_objectAngleY = 0.0f;
	_cameraAngle = 240.0f;
	_camera.setDistanceFromTarget(200.f);
	_camera.setHeight(-15.f);
	_blurScale = 4;
	_frameId = 0;
	_queueIndex = 0;
	_logicTime = 0.0f;
	_modeSwitchTime = 0.0f;
	_isManual = false;
	_modeDuration = 1.5f;
	_currentScene = 0;
	_renderOnlyBloom = false;

	// Handle command line arguments including "blurmode", "blursize" and "bloom"
	const pvr::CommandLine& commandOptions = getCommandLine();
	int32_t intBlurMode = -1;
	if (commandOptions.getIntOption("-blurmode", intBlurMode))
	{
		if (intBlurMode > static_cast<int32_t>(BloomMode::NumBloomModes)) { _blurMode = BloomMode::DefaultMode; }
		else
		{
			_isManual = true;
			_blurMode = static_cast<BloomMode>(intBlurMode);
		}
	}
	else
	{
		_blurMode = BloomMode::DefaultMode;
	}

	int32_t intConfigSize = -1;
	if (commandOptions.getIntOption("-blursize", intConfigSize))
	{
		if (intConfigSize > static_cast<int32_t>(DemoConfigurations::NumDemoConfigurations)) { _currentDemoConfiguration = DemoConfigurations::DefaultDemoConfigurations; }
		else
		{
			_isManual = true;
			_currentDemoConfiguration = static_cast<uint32_t>(intConfigSize);
		}
	}
	else
	{
		_currentDemoConfiguration = DemoConfigurations::DefaultDemoConfigurations;
	}

	commandOptions.getBoolOptionSetTrueIfPresent("-bloom", _renderOnlyBloom);

	return pvr::Result::Success;
}

/// <summary>Code in initView() will be called by Shell upon initialization or after a change in the rendering context.
/// Used to initialize variables that are dependent on the rendering context(e.g.textures, vertex buffers, etc.)</summary>
/// <returns>Result::Success if no error occurred.</returns>
pvr::Result VulkanPostProcessing::initView()
{
	_deviceResources = std::make_unique<DeviceResources>();

	// Create instance and retrieve compatible physical devices
	_deviceResources->instance = pvr::utils::createInstance(this->getApplicationName());

	if (_deviceResources->instance->getNumPhysicalDevices() == 0)
	{
		setExitMessage("Unable not find a compatible Vulkan physical device.");
		return pvr::Result::UnknownError;
	}

	// Create the surface
	pvrvk::Surface surface =
		pvr::utils::createSurface(_deviceResources->instance, _deviceResources->instance->getPhysicalDevice(0), this->getWindow(), this->getDisplay(), this->getConnection());

	// Create a default set of debug utils messengers or debug callbacks using either VK_EXT_debug_utils or VK_EXT_debug_report respectively
	_deviceResources->debugUtilsCallbacks = pvr::utils::createDebugUtilsCallbacks(_deviceResources->instance);

	pvr::utils::QueuePopulateInfo queueCreateInfos[] = {
		{ pvrvk::QueueFlags::e_GRAPHICS_BIT | pvrvk::QueueFlags::e_COMPUTE_BIT, surface }, // Queue 0
		{ pvrvk::QueueFlags::e_GRAPHICS_BIT | pvrvk::QueueFlags::e_COMPUTE_BIT, surface } // Queue 1
	};
	pvr::utils::QueueAccessInfo queueAccessInfos[2];
	_deviceResources->device = pvr::utils::createDeviceAndQueues(_deviceResources->instance->getPhysicalDevice(0), queueCreateInfos, 2, queueAccessInfos);

	_deviceResources->queues[0] = _deviceResources->device->getQueue(queueAccessInfos[0].familyId, queueAccessInfos[0].queueId);
	_deviceResources->queues[1] = _deviceResources->device->getQueue(queueAccessInfos[1].familyId, queueAccessInfos[1].queueId);

	// In the future we may want to improve our flexibility with regards to making use of multiple queues but for now to support multi queue the queue must support
	// Graphics + Compute + WSI support.
	// Other multi queue approaches may be possible i.e. making use of additional queues which do not support graphics/WSI
	_useMultiQueue = false;

	if (queueAccessInfos[1].familyId != -1 && queueAccessInfos[1].queueId != -1)
	{
		_deviceResources->queues[1] = _deviceResources->device->getQueue(queueAccessInfos[1].familyId, queueAccessInfos[1].queueId);

		if (_deviceResources->queues[0]->getFamilyIndex() == _deviceResources->queues[1]->getFamilyIndex())
		{
			_useMultiQueue = true;
			Log(LogLevel::Information, "Multiple queues support e_GRAPHICS_BIT + e_COMPUTE_BIT + WSI. These queues will be used to ping-pong work each frame");
		}
		else
		{
			Log(LogLevel::Information, "Queues are from a different Family. We cannot ping-pong work each frame");
		}
	}
	else
	{
		Log(LogLevel::Information, "Only a single queue supports e_GRAPHICS_BIT + e_COMPUTE_BIT + WSI. We cannot ping-pong work each frame");
	}
	pvrvk::SurfaceCapabilitiesKHR surfaceCapabilities = _deviceResources->instance->getPhysicalDevice(0)->getSurfaceCapabilities(surface);

	// validate the supported swapchain image usage
	pvrvk::ImageUsageFlags swapchainImageUsage = pvrvk::ImageUsageFlags::e_COLOR_ATTACHMENT_BIT;
	if (pvr::utils::isImageUsageSupportedBySurface(surfaceCapabilities, pvrvk::ImageUsageFlags::e_TRANSFER_SRC_BIT))
	{ swapchainImageUsage |= pvrvk::ImageUsageFlags::e_TRANSFER_SRC_BIT; }
	_luminanceColorFormat = pvrvk::Format::e_R16_SFLOAT;

	// Create memory allocator
	_deviceResources->vmaAllocator = pvr::utils::vma::createAllocator(pvr::utils::vma::AllocatorCreateInfo(_deviceResources->device));

	// We do not support automatic MSAA for this demo.
	if (getDisplayAttributes().aaSamples > 1)
	{
		Log(LogLevel::Warning, "Full Screen Multisample Antialiasing requested, but not supported for this demo's configuration.");
		getDisplayAttributes().aaSamples = 1;
	}

	// Create the swapchain, framebuffers and main rendering images
	// Note the use of the colour attachment load operation (pvrvk::AttachmentLoadOp::e_DONT_CARE). The final composition pass will be a full screen render
	// so we don't need to clear the attachment prior to rendering to the image as each pixel will get a new value either way
	// The final render is a full screen pass, so no depth is required (see below)...
	auto swapChainCreateOutput = pvr::utils::createSwapchainRenderpassFramebuffers(_deviceResources->device, surface, getDisplayAttributes(),
		pvr::utils::CreateSwapchainParameters()
			.setAllocator(_deviceResources->vmaAllocator)
			.setColorImageUsageFlags(swapchainImageUsage)
			.enableDepthBuffer(false)
			.setColorLoadOp(pvrvk::AttachmentLoadOp::e_DONT_CARE));

	_deviceResources->swapchain = swapChainCreateOutput.swapchain;
	_deviceResources->onScreenRenderPass = swapChainCreateOutput.renderPass;
	_deviceResources->onScreenFramebuffers = swapChainCreateOutput.framebuffer;

	// ... however we need to create the exact same attachments as we would have, only attach them to the g-buffer, not the on-screen pass.
	pvr::utils::createAttachmentImages(_deviceResources->depthStencilImages, _deviceResources->device, _deviceResources->swapchain->getSwapchainLength(),
		pvr::utils::getSupportedDepthStencilFormat(_deviceResources->device, getDisplayAttributes()), _deviceResources->swapchain->getDimension(),
		pvrvk::ImageUsageFlags::e_DEPTH_STENCIL_ATTACHMENT_BIT | pvrvk::ImageUsageFlags::e_TRANSIENT_ATTACHMENT_BIT, pvrvk::SampleCountFlags::e_1_BIT, _deviceResources->vmaAllocator);

	// calculate the frame buffer width and heights
	_blurFramebufferDimensions = glm::uvec2(this->getWidth() / _blurScale, this->getHeight() / _blurScale);
	_blurInverseFramebufferDimensions = glm::vec2(1.0f / _blurFramebufferDimensions.x, 1.0f / _blurFramebufferDimensions.y);

	// Calculates the projection matrices
	bool bRotate = isFullScreen() && isScreenRotated();
	if (bRotate)
	{
		_projectionMatrix =
			pvr::math::perspectiveFov(pvr::Api::Vulkan, Fov, static_cast<float>(getHeight()), static_cast<float>(getWidth()), CameraNear, CameraFar, glm::pi<float>() * .5f);
	}
	else
	{
		_projectionMatrix = pvr::math::perspectiveFov(pvr::Api::Vulkan, Fov, static_cast<float>(getWidth()), static_cast<float>(getHeight()), CameraNear, CameraFar);
	}

	// Get current swap index
	_swapchainIndex = _deviceResources->swapchain->getSwapchainIndex();

	// Determine whether shader storage image extended formats is supported on the current platform
	// Ideally we would choose e_R16_SFLOAT but the physical device must have support for features.ShaderStorageImageExtendedFormats
	// If features.ShaderStorageImageExtendedFormats isn't supported then we'll fallback to e_R16G16B16A16_SFLOAT
	// e_R16G16B16A16_SFLOAT may already be preferred as it allows for the handling of coloured blooms i.e. coloured light sources
	if (_deviceResources->instance->getPhysicalDevice(0)->getFeatures().getShaderStorageImageExtendedFormats())
	{
		pvrvk::Format extendedFormat = _luminanceColorFormat;
		pvrvk::FormatProperties prop = _deviceResources->instance->getPhysicalDevice(0)->getFormatProperties(extendedFormat);
		if ((prop.getOptimalTilingFeatures() & pvrvk::FormatFeatureFlags::e_STORAGE_IMAGE_BIT) != 0)
		{
			_storageImageLuminanceColorFormat = extendedFormat;
			_storageImageTiling = pvrvk::ImageTiling::e_OPTIMAL;
		}
		else if ((prop.getLinearTilingFeatures() & pvrvk::FormatFeatureFlags::e_STORAGE_IMAGE_BIT) != 0)
		{
			_storageImageLuminanceColorFormat = extendedFormat;
			_storageImageTiling = pvrvk::ImageTiling::e_LINEAR;
		}

		// Ensure that the format being used supports Linear Sampling
		if ((prop.getOptimalTilingFeatures() & pvrvk::FormatFeatureFlags::e_SAMPLED_IMAGE_FILTER_LINEAR_BIT) == 0) { assertion(0); }
	}
	else
	{
		pvrvk::Format format = pvrvk::Format::e_R16G16B16A16_SFLOAT;
		pvrvk::FormatProperties prop = _deviceResources->instance->getPhysicalDevice(0)->getFormatProperties(format);
		if ((prop.getOptimalTilingFeatures() & pvrvk::FormatFeatureFlags::e_STORAGE_IMAGE_BIT) != 0)
		{
			_storageImageLuminanceColorFormat = format;
			_storageImageTiling = pvrvk::ImageTiling::e_OPTIMAL;
		}
		else if ((prop.getLinearTilingFeatures() & pvrvk::FormatFeatureFlags::e_STORAGE_IMAGE_BIT) != 0)
		{
			_storageImageLuminanceColorFormat = format;
			_storageImageTiling = pvrvk::ImageTiling::e_LINEAR;
		}

		// Ensure that the format being used supports Linear Sampling
		if ((prop.getOptimalTilingFeatures() & pvrvk::FormatFeatureFlags::e_SAMPLED_IMAGE_FILTER_LINEAR_BIT) == 0) { assertion(0); }
	}

	// create the command pool and the descriptor pool
	_deviceResources->commandPool = _deviceResources->device->createCommandPool(
		pvrvk::CommandPoolCreateInfo(_deviceResources->queues[0]->getFamilyIndex(), pvrvk::CommandPoolCreateFlags::e_RESET_COMMAND_BUFFER_BIT));

	// This demo application makes use of quite a large number of Images and Buffers and therefore we're making possible for the descriptor pool to allocate descriptors with various limits.maxDescriptorSet*
	_deviceResources->descriptorPool = _deviceResources->device->createDescriptorPool(pvrvk::DescriptorPoolCreateInfo()
																						  .setMaxDescriptorSets(150)
																						  .addDescriptorInfo(pvrvk::DescriptorType::e_COMBINED_IMAGE_SAMPLER, 200)
																						  .addDescriptorInfo(pvrvk::DescriptorType::e_STORAGE_IMAGE, 20)
																						  .addDescriptorInfo(pvrvk::DescriptorType::e_UNIFORM_BUFFER, 200));

	// create the utility commandbuffer which will be used for image layout transitions and buffer/image uploads.
	_deviceResources->utilityCommandBuffer = _deviceResources->commandPool->allocateCommandBuffer();
	_deviceResources->utilityCommandBuffer->begin();

	// Create the pipeline cache
	_deviceResources->pipelineCache = _deviceResources->device->createPipelineCache();

	// create demo scene buffers
	createSceneBuffers();

	// Allocate two images to use which can be "ping-ponged" between when applying various filters/blurs
	// Pass 1: Read From 1, Render to 0
	// Pass 2: Read From 0, Render to 1
	allocatePingPongImages();

	// Create the HDR offscreen framebuffers
	createOffScreenFramebuffers();

	// Create the samplers used for various texture sampling
	createSamplers();

	// transition the shared blur images ready for their first use
	for (uint32_t i = 0; i < _deviceResources->swapchain->getSwapchainLength(); ++i)
	{
		for (uint32_t j = 0; j < _deviceResources->sharedBlurImageViews.size(); j++)
		{
			pvr::utils::setImageLayout(_deviceResources->sharedBlurImageViews[j][i]->getImage(), pvrvk::ImageLayout::e_UNDEFINED, pvrvk::ImageLayout::e_SHADER_READ_ONLY_OPTIMAL,
				_deviceResources->utilityCommandBuffer);
		}
	}

	// The diffuse irradiance maps are small so we load them all up front
	for (uint32_t i = 0; i < NumScenes; ++i)
	{
		pvr::Texture diffuseIrradianceMapTexture = pvr::textureLoad(*getAssetStream(SceneTexFileNames[i].diffuseIrradianceMapTexture), pvr::TextureFileFormat::PVR);

		// Create and Allocate Textures
		_deviceResources->diffuseIrradianceMapImageViews[i] =
			pvr::utils::uploadImageAndView(_deviceResources->device, diffuseIrradianceMapTexture, true, _deviceResources->utilityCommandBuffer,
				pvrvk::ImageUsageFlags::e_SAMPLED_BIT, pvrvk::ImageLayout::e_SHADER_READ_ONLY_OPTIMAL, _deviceResources->vmaAllocator, _deviceResources->vmaAllocator);
	}

	// Create the main scene rendering passes
	_deviceResources->statuePass.init(*this, _deviceResources->device, _deviceResources->swapchain, _deviceResources->commandPool, _deviceResources->descriptorPool,
		_deviceResources->offScreenRenderPass, _deviceResources->vmaAllocator, _deviceResources->utilityCommandBuffer, _deviceResources->pipelineCache);

	_deviceResources->skyBoxPass.init(*this, _deviceResources->device, _deviceResources->swapchain, _deviceResources->commandPool, _deviceResources->descriptorPool,
		_deviceResources->offScreenRenderPass, _deviceResources->pipelineCache);

	// Create bloom RenderPasses and Framebuffers
	createBlurRenderPass();
	createBlurFramebuffers();

	createHybridBlurRenderPass();
	createHybridBlurFramebuffers();

	// Create the downsample passes
	{
		_deviceResources->downsamplePass.init(*this, _deviceResources->device, _deviceResources->swapchain, _deviceResources->commandPool, _deviceResources->descriptorPool,
			_blurFramebufferDimensions, _deviceResources->luminanceImageViews, _deviceResources->sharedBlurImageViews[_pingPongImageIndices[0]], _deviceResources->samplerBilinear,
			_deviceResources->pipelineCache, false);

		_deviceResources->computeDownsamplePass.init(*this, _deviceResources->device, _deviceResources->swapchain, _deviceResources->commandPool, _deviceResources->descriptorPool,
			_blurFramebufferDimensions, _deviceResources->luminanceImageViews, _deviceResources->sharedBlurImageViews[_pingPongImageIndices[0]], _deviceResources->samplerBilinear,
			_deviceResources->pipelineCache, true);
	}

	// Create the post bloom composition pass
	_deviceResources->postBloomPass.init(*this, _deviceResources->device, _deviceResources->swapchain, _deviceResources->commandPool, _deviceResources->descriptorPool,
		_deviceResources->vmaAllocator, _deviceResources->onScreenRenderPass, _deviceResources->pipelineCache);

	// Initialise the Blur Passes
	// Gaussian Blurs
	{
		uint32_t horizontalPassPingPongImageIndex = 1;
		uint32_t verticalPassPingPongImageIndex = 0;

		_deviceResources->gaussianBlurPass.init(*this, _deviceResources->device, _deviceResources->swapchain, _deviceResources->commandPool, _deviceResources->descriptorPool,
			_deviceResources->vmaAllocator, _deviceResources->blurRenderPass, _blurFramebufferDimensions,
			_deviceResources->sharedBlurImageViews[_pingPongImageIndices[horizontalPassPingPongImageIndex]],
			_deviceResources->sharedBlurImageViews[_pingPongImageIndices[verticalPassPingPongImageIndex]], _deviceResources->samplerNearest, _deviceResources->pipelineCache);

		_deviceResources->linearGaussianBlurPass.init(*this, _deviceResources->device, _deviceResources->swapchain, _deviceResources->commandPool, _deviceResources->descriptorPool,
			_deviceResources->vmaAllocator, _deviceResources->blurRenderPass, _blurFramebufferDimensions,
			_deviceResources->sharedBlurImageViews[_pingPongImageIndices[horizontalPassPingPongImageIndex]],
			_deviceResources->sharedBlurImageViews[_pingPongImageIndices[verticalPassPingPongImageIndex]], _deviceResources->samplerBilinear, _deviceResources->pipelineCache);

		_deviceResources->truncatedLinearGaussianBlurPass.init(*this, _deviceResources->device, _deviceResources->swapchain, _deviceResources->commandPool,
			_deviceResources->descriptorPool, _deviceResources->vmaAllocator, _deviceResources->blurRenderPass, _blurFramebufferDimensions,
			_deviceResources->sharedBlurImageViews[_pingPongImageIndices[horizontalPassPingPongImageIndex]],
			_deviceResources->sharedBlurImageViews[_pingPongImageIndices[verticalPassPingPongImageIndex]], _deviceResources->samplerBilinear, _deviceResources->pipelineCache);

		_deviceResources->computeBlurPass.init(*this, _deviceResources->device, _deviceResources->swapchain, _deviceResources->commandPool, _deviceResources->descriptorPool,
			_deviceResources->vmaAllocator, _deviceResources->blurRenderPass, _blurFramebufferDimensions,
			_deviceResources->sharedBlurImageViews[_pingPongImageIndices[horizontalPassPingPongImageIndex]],
			_deviceResources->sharedBlurImageViews[_pingPongImageIndices[verticalPassPingPongImageIndex]], _deviceResources->samplerNearest, _deviceResources->pipelineCache);

		_deviceResources->hybridGaussianBlurPass.init(
			&_deviceResources->computeBlurPass, &_deviceResources->truncatedLinearGaussianBlurPass, _deviceResources->swapchain, _deviceResources->commandPool);
	}

	// Kawase Blur
	{
		std::vector<pvr::Multi<pvrvk::ImageView>> kawaseImages;
		kawaseImages.emplace_back(_deviceResources->sharedBlurImageViews[_pingPongImageIndices[0]]);
		kawaseImages.emplace_back(_deviceResources->sharedBlurImageViews[_pingPongImageIndices[1]]);

		_deviceResources->kawaseBlurPass.init(*this, _deviceResources->device, _deviceResources->swapchain, _deviceResources->commandPool, _deviceResources->descriptorPool,
			_deviceResources->blurRenderPass, _blurFramebufferDimensions, kawaseImages.data(), 2, _deviceResources->samplerBilinear, _deviceResources->pipelineCache);
	}

	// Dual Filter Blur
	{
		_deviceResources->dualFilterBlurPass.init(*this, _deviceResources->device, _deviceResources->swapchain, _deviceResources->commandPool, _deviceResources->descriptorPool,
			_deviceResources->blurRenderPass, _deviceResources->offScreenColorImageViews, _deviceResources->luminanceImageViews, _deviceResources->vmaAllocator,
			_luminanceColorFormat, glm::uvec2(this->getWidth(), this->getHeight()), _deviceResources->samplerBilinear, _deviceResources->onScreenRenderPass,
			_deviceResources->pipelineCache, _deviceResources->sharedBlurImageViews);
	}

	// Down Sample and Tent filter blur pass
	{
		_deviceResources->downAndTentFilterBlurPass.init(*this, _deviceResources->device, _deviceResources->swapchain, _deviceResources->commandPool,
			_deviceResources->descriptorPool, _deviceResources->blurRenderPass, _deviceResources->offScreenColorImageViews, _deviceResources->luminanceImageViews,
			_deviceResources->vmaAllocator, _luminanceColorFormat, glm::uvec2(this->getWidth(), this->getHeight()), _deviceResources->samplerBilinear,
			_deviceResources->onScreenRenderPass, _deviceResources->pipelineCache, _deviceResources->sharedBlurImageViews);
	}

	_deviceResources->utilityCommandBuffer->end();

	pvrvk::SubmitInfo submitInfo;
	submitInfo.commandBuffers = &_deviceResources->utilityCommandBuffer;
	submitInfo.numCommandBuffers = 1;
	_deviceResources->queues[0]->submit(&submitInfo, 1);
	_deviceResources->queues[0]->waitIdle(); // wait

	// signal that buffers need updating and command buffers need recording
	for (uint32_t i = 0; i < _deviceResources->swapchain->getSwapchainLength(); ++i)
	{
		_mustRecordPrimaryCommandBuffer[i] = true;
		_mustUpdatePerSwapchainDemoConfig[i] = true;
	}

	// Update the demo configuration
	updateDemoConfigs();

	// create the synchronisation primitives
	for (uint32_t i = 0; i < _deviceResources->swapchain->getSwapchainLength(); ++i)
	{
		_deviceResources->presentationSemaphores[i] = _deviceResources->device->createSemaphore();
		_deviceResources->imageAcquiredSemaphores[i] = _deviceResources->device->createSemaphore();
		_deviceResources->perFrameResourcesFences[i] = _deviceResources->device->createFence(pvrvk::FenceCreateFlags::e_SIGNALED_BIT);
	}

	// initialise the UI Renderers
	createUiRenderer();

	// Record UI renderer command buffers
	for (uint32_t i = 0; i < _deviceResources->swapchain->getSwapchainLength(); ++i)
	{
		// record bloom command buffers
		recordUIRendererCommands(i, _deviceResources->uiRendererCommandBuffers);
		recordUIRendererCommands(i, _deviceResources->bloomUiRendererCommandBuffers);
	}

	for (uint32_t i = 0; i < _deviceResources->swapchain->getSwapchainLength(); ++i)
	{ _deviceResources->mainCommandBuffers.add(_deviceResources->commandPool->allocateCommandBuffer()); }
	return pvr::Result::Success;
}

/// <summary>Main rendering loop function of the program. The shell will call this function every frame</summary>
/// <returns>Result::Success if no error occurred.</summary>
pvr::Result VulkanPostProcessing::renderFrame()
{
	handleDesktopInput();

	_deviceResources->swapchain->acquireNextImage(uint64_t(-1), _deviceResources->imageAcquiredSemaphores[_frameId]);

	_swapchainIndex = _deviceResources->swapchain->getSwapchainIndex();

	_deviceResources->perFrameResourcesFences[_swapchainIndex]->wait();
	_deviceResources->perFrameResourcesFences[_swapchainIndex]->reset();

	// update dynamic buffers
	updateDynamicSceneData();

	// Re-record command buffers on demand
	if (_mustRecordPrimaryCommandBuffer[_swapchainIndex])
	{
		recordMainCommandBuffer(_swapchainIndex);
		_mustRecordPrimaryCommandBuffer[_swapchainIndex] = false;
	}

	pvrvk::SubmitInfo submitInfo;
	pvrvk::PipelineStageFlags submitWaitFlags = pvrvk::PipelineStageFlags::e_COLOR_ATTACHMENT_OUTPUT_BIT;
	submitInfo.waitDstStageMask = &submitWaitFlags;
	submitInfo.waitSemaphores = &_deviceResources->imageAcquiredSemaphores[_frameId];
	submitInfo.numWaitSemaphores = 1;
	submitInfo.signalSemaphores = &_deviceResources->presentationSemaphores[_frameId];
	submitInfo.numSignalSemaphores = 1;
	submitInfo.numCommandBuffers = 1;
	submitInfo.commandBuffers = &_deviceResources->mainCommandBuffers[_swapchainIndex];

	// Ping pong between multiple VkQueues
	// It's important to realise that in Vulkan, pipeline barriers only observe their barriers within the VkQueue they are submitted to.
	// When we use BloomMode::Compute || BloomMode::HybridGaussian we are introducing a Fragment -> Compute -> Fragment chain, which if left
	// unattended can cause compute pipeline bubbles meaning we can quite easily hit into per frame workload serialisation as shown below:
	// Compute Workload             |1----|                  |2----|
	// Fragment Workload     |1----|       |1---||1--||2----|       |2---||2--|

	// The Compute -> Fragment pipeline used after our Compute pipeline stage for synchronising between the pipeline stages has further, less obvious unintended consequences
	// in that when using only a single VkQueue this pipeline barrier enforces a barrier between all Compute work *before* the barrier and all Fragment work *after* the
	// barrier. This barrier means that even though we can see compute pipeline bubbles that could potentially be interleaved with Fragment work the barrier enforces against
	// this behaviour. This is where Vulkan really shines over OpenGL ES in terms of giving explicit control of work submission to the application. We make use of two Vulkan
	// VkQueue objects which are submitted to in a ping-ponged fashion. Each VkQueue only needs to observe barriers used in command buffers which are submitted to them meaning
	// there are no barriers enforced between the two sets of separate commands other than the presentation synchronisation logic. This simple change allows us to observe the
	// following workload scheduling: Compute Workload                |1----|    |2----| Fragment Workload      |1----||2----|  |1---||1--||2---||2--|
	_deviceResources->queues[_queueIndex]->submit(&submitInfo, 1, _deviceResources->perFrameResourcesFences[_swapchainIndex]);

	if (this->shouldTakeScreenshot())
	{
		pvr::utils::takeScreenshot(_deviceResources->queues[_queueIndex], _deviceResources->commandPool, _deviceResources->swapchain, _swapchainIndex,
			this->getScreenshotFileName(), _deviceResources->vmaAllocator, _deviceResources->vmaAllocator);
	}

	pvrvk::PresentInfo presentInfo;
	presentInfo.waitSemaphores = &_deviceResources->presentationSemaphores[_frameId];
	presentInfo.numWaitSemaphores = 1;
	presentInfo.swapchains = &_deviceResources->swapchain;
	presentInfo.numSwapchains = 1;
	presentInfo.imageIndices = &_swapchainIndex;

	// As above we must present using the same VkQueue as submitted to previously
	_deviceResources->queues[_queueIndex]->present(presentInfo);

	_frameId = (_frameId + 1) % _deviceResources->swapchain->getSwapchainLength();

	if (_useMultiQueue) { _queueIndex = (_queueIndex + 1) % 2; }

	return pvr::Result::Success;
}

/// <summary>Code in releaseView() will be called by Shell when the application quits or before a change in the rendering context.</summary>
/// <returns>Result::Success if no error occurred.</returns>
pvr::Result VulkanPostProcessing::releaseView()
{
	_deviceResources.reset();
	return pvr::Result::Success;
}

/// <summary>Code in quitApplication() will be called by Shell once per run, just before exiting the program.
/// quitApplication() will not be called every time the rendering context is lost, only before application exit.</summary>
/// <returns>Result::Success if no error occurred.</returns>
pvr::Result VulkanPostProcessing::quitApplication() { return pvr::Result::Success; }

/// <summary>Creates The UI renderer.</summary>
void VulkanPostProcessing::createUiRenderer()
{
	_deviceResources->uiRenderer.init(getWidth(), getHeight(), isFullScreen(), _deviceResources->onScreenRenderPass, 0, getBackBufferColorspace() == pvr::ColorSpace::sRGB,
		_deviceResources->commandPool, _deviceResources->queues[0]);

	_deviceResources->uiRenderer.getDefaultTitle()->setText("PostProcessing");
	_deviceResources->uiRenderer.getDefaultTitle()->commitUpdates();
	_deviceResources->uiRenderer.getDefaultControls()->setText("Left / right: Blur Mode\n"
															   "Up / Down: Blur Size\n"
															   "Action 1: Enable/Disable Bloom\n"
															   "Action 2: Enable/Disable Animation\n"
															   "Action 3: Change Scene\n");
	_deviceResources->uiRenderer.getDefaultControls()->commitUpdates();

	updateBlurDescription();
	_deviceResources->uiRenderer.getDefaultDescription()->setText(_currentBlurString);
	_deviceResources->uiRenderer.getDefaultDescription()->commitUpdates();
}

/// <summary>Updates the description for the currently used blur technique.</summary>
void VulkanPostProcessing::updateBlurDescription()
{
	switch (_blurMode)
	{
	case (BloomMode::NoBloom): {
		_currentBlurString = BloomStrings[static_cast<uint32_t>(_blurMode)];
		break;
	}
	case (BloomMode::GaussianOriginal): {
		uint32_t numSamples = static_cast<uint32_t>(_deviceResources->gaussianBlurPass.gaussianOffsets[_currentDemoConfiguration].size());
		_currentBlurString = BloomStrings[static_cast<uint32_t>(_blurMode)] + "\n" +
			pvr::strings::createFormatted("Kernel Size = %u (%u + %u taps)", DemoConfigurations::Configurations[_currentDemoConfiguration].gaussianConfig, numSamples, numSamples);
		break;
	}
	case (BloomMode::GaussianLinear): {
		uint32_t numSamples = static_cast<uint32_t>(_deviceResources->linearGaussianBlurPass.gaussianOffsets[_currentDemoConfiguration].size());
		_currentBlurString = BloomStrings[static_cast<uint32_t>(_blurMode)] + "\n" +
			pvr::strings::createFormatted("Kernel Size = %u (%u + %u taps)", DemoConfigurations::Configurations[_currentDemoConfiguration].linearGaussianConfig, numSamples, numSamples);
		break;
	}
	case (BloomMode::GaussianLinearTruncated): {
		uint32_t numSamples = static_cast<uint32_t>(_deviceResources->truncatedLinearGaussianBlurPass.gaussianOffsets[_currentDemoConfiguration].size());
		_currentBlurString = BloomStrings[static_cast<uint32_t>(_blurMode)] + "\n" +
			pvr::strings::createFormatted(
				"Kernel Size = %u (%u + %u taps)", DemoConfigurations::Configurations[_currentDemoConfiguration].truncatedLinearGaussianConfig, numSamples, numSamples);
		break;
	}
	case (BloomMode::Compute): {
		uint32_t numSamples = static_cast<uint32_t>(_deviceResources->computeBlurPass.gaussianOffsets[_currentDemoConfiguration].size());
		_currentBlurString = BloomStrings[static_cast<uint32_t>(_blurMode)] + "\n" +
			pvr::strings::createFormatted(
				"Kernel Size = %u (Sliding Average)", DemoConfigurations::Configurations[_currentDemoConfiguration].computeGaussianConfig, numSamples, numSamples);
		break;
	}
	case (BloomMode::DualFilter): {
		uint32_t numSamples = _deviceResources->dualFilterBlurPass.blurIterations / 2;
		_currentBlurString = BloomStrings[static_cast<uint32_t>(_blurMode)] + "\n" +
			pvr::strings::createFormatted(
				"Iterations = %u (%u Downsamples, %u Upsamples)", DemoConfigurations::Configurations[_currentDemoConfiguration].dualFilterConfig, numSamples, numSamples);
		break;
	}
	case (BloomMode::TentFilter): {
		uint32_t numSamples = _deviceResources->downAndTentFilterBlurPass.blurIterations / 2;
		_currentBlurString = BloomStrings[static_cast<uint32_t>(_blurMode)] + "\n" +
			pvr::strings::createFormatted(
				"Iterations = %u (%u Downsamples, %u Upsamples)", DemoConfigurations::Configurations[_currentDemoConfiguration].tentFilterConfig, numSamples, numSamples);
		break;
	}
	case (BloomMode::HybridGaussian): {
		uint32_t numComputeSamples = static_cast<uint32_t>(_deviceResources->hybridGaussianBlurPass.computeBlurPass->gaussianOffsets[_currentDemoConfiguration].size());
		uint32_t numLinearSamples = static_cast<uint32_t>(_deviceResources->hybridGaussianBlurPass.linearBlurPass->gaussianOffsets[_currentDemoConfiguration].size());
		_currentBlurString = BloomStrings[static_cast<uint32_t>(_blurMode)] + "\n" +
			pvr::strings::createFormatted("Horizontal Compute %u taps, Vertical Linear Gaussian %u taps)", numComputeSamples, numLinearSamples);
		break;
	}
	case (BloomMode::Kawase): {
		std::string kernelString = "";
		uint32_t numIterations = _deviceResources->kawaseBlurPass.blurIterations;

		for (uint32_t i = 0; i < numIterations - 1; ++i)
		{ kernelString += pvr::strings::createFormatted("%u,", DemoConfigurations::Configurations[_currentDemoConfiguration].kawaseConfig.kernel[i]); }
		kernelString += pvr::strings::createFormatted("%u", DemoConfigurations::Configurations[_currentDemoConfiguration].kawaseConfig.kernel[numIterations - 1]);

		_currentBlurString = BloomStrings[static_cast<uint32_t>(_blurMode)] + "\n" + pvr::strings::createFormatted("%u Iterations: %s", numIterations, kernelString.c_str());
		break;
	}
	default: throw pvr::UnsupportedOperationError("Unsupported BlurMode.");
	}

	Log(LogLevel::Information, "Current blur mode: \"%s\"", BloomStrings[static_cast<int32_t>(_blurMode)].c_str());
	Log(LogLevel::Information, "Current blur size configuration: \"%u\"", _currentDemoConfiguration);
}

/// <summary>Creates the main scene buffer.</summary>
void VulkanPostProcessing::createSceneBuffers()
{
	pvr::utils::StructuredMemoryDescription desc;
	desc.addElement(BufferEntryNames::Scene::InverseViewProjectionMatrix, pvr::GpuDatatypes::mat4x4);
	desc.addElement(BufferEntryNames::Scene::EyePosition, pvr::GpuDatatypes::vec3);

	_deviceResources->sceneBufferView.initDynamic(desc, _deviceResources->swapchain->getSwapchainLength(), pvr::BufferUsageFlags::UniformBuffer,
		_deviceResources->device->getPhysicalDevice()->getProperties().getLimits().getMinUniformBufferOffsetAlignment());

	_deviceResources->sceneBuffer =
		pvr::utils::createBuffer(_deviceResources->device, pvrvk::BufferCreateInfo(_deviceResources->sceneBufferView.getSize(), pvrvk::BufferUsageFlags::e_UNIFORM_BUFFER_BIT),
			pvrvk::MemoryPropertyFlags::e_HOST_VISIBLE_BIT, pvrvk::MemoryPropertyFlags::e_HOST_COHERENT_BIT, _deviceResources->vmaAllocator);

	_deviceResources->sceneBufferView.pointToMappedMemory(_deviceResources->sceneBuffer->getDeviceMemory()->getMappedData());
}

/// <summary>Records the main command buffers used for rendering the demo.</summary>
/// <param name="swapchainIndex">The swapchain index for which the recorded command buffer will be used.</param>
void VulkanPostProcessing::recordMainCommandBuffer(uint32_t swapchainIndex)
{
	_deviceResources->mainCommandBuffers[swapchainIndex]->reset();
	_deviceResources->mainCommandBuffers[swapchainIndex]->begin();

	pvr::utils::beginCommandBufferDebugLabel(
		_deviceResources->mainCommandBuffers[swapchainIndex], pvrvk::DebugUtilsLabel(pvr::strings::createFormatted("Render Scene - swapchain: %i", swapchainIndex)));

	const pvrvk::ClearValue offScreenClearValues[] = { pvrvk::ClearValue(0.0f, 0.0f, 0.0f, 1.f), pvrvk::ClearValue(0.0f, 0.0f, 0.0f, 1.f),
		pvrvk::ClearValue::createDefaultDepthStencilClearValue() };

	_deviceResources->skyBoxPass.loadSkyBoxImageView(*this, _deviceResources->device, _deviceResources->mainCommandBuffers[swapchainIndex], _deviceResources->vmaAllocator, _currentScene);

	// Update the scene rendering descriptor sets
	_deviceResources->statuePass.updateDescriptorSets(_deviceResources->device, swapchainIndex, _deviceResources->diffuseIrradianceMapImageViews[_currentScene],
		_deviceResources->samplerBilinear, _deviceResources->samplerTrilinear);
	_deviceResources->skyBoxPass.updateDescriptorSets(
		_deviceResources->device, swapchainIndex, _deviceResources->samplerTrilinear, _deviceResources->sceneBuffer, _deviceResources->sceneBufferView);

	// record command buffers used for rendering the main scene
	_deviceResources->statuePass.recordCommandBuffer(swapchainIndex, _deviceResources->offScreenFramebuffers[swapchainIndex], _exposure, _threshold);
	_deviceResources->skyBoxPass.recordCommandBuffer(swapchainIndex, _deviceResources->offScreenFramebuffers[swapchainIndex], _exposure, _threshold);

	// Render the main scene
	_deviceResources->mainCommandBuffers[swapchainIndex]->beginRenderPass(_deviceResources->offScreenFramebuffers[swapchainIndex], _deviceResources->offScreenRenderPass,
		pvrvk::Rect2D(0, 0, getWidth(), getHeight()), false, offScreenClearValues, ARRAY_SIZE(offScreenClearValues));
	_deviceResources->mainCommandBuffers[swapchainIndex]->executeCommands(_deviceResources->statuePass.cmdBuffers[swapchainIndex]);
	_deviceResources->mainCommandBuffers[swapchainIndex]->executeCommands(_deviceResources->skyBoxPass.cmdBuffers[swapchainIndex]);
	_deviceResources->mainCommandBuffers[swapchainIndex]->endRenderPass();

	pvr::utils::endCommandBufferDebugLabel(_deviceResources->mainCommandBuffers[swapchainIndex]);

	pvrvk::ClearValue clearValues = pvrvk::ClearValue(0.0f, 0.0f, 0.0f, 1.f);

	// When using Dual/Tent filter no downsample is required as they take care of downsampling themselves
	if (!(_blurMode == BloomMode::DualFilter || _blurMode == BloomMode::TentFilter))
	{
		// Use a special cased downsample pass when the next pass will be using compute
		if (_blurMode == BloomMode::Compute || _blurMode == BloomMode::HybridGaussian)
		{
			_deviceResources->computeDownsamplePass.recordCommands(swapchainIndex);

			_deviceResources->mainCommandBuffers[swapchainIndex]->beginRenderPass(_deviceResources->computeDownsamplePass.framebuffers[swapchainIndex],
				_deviceResources->computeDownsamplePass.renderPass, pvrvk::Rect2D(0, 0, _blurFramebufferDimensions.x, _blurFramebufferDimensions.y), false, &clearValues, 1);
			_deviceResources->mainCommandBuffers[swapchainIndex]->executeCommands(_deviceResources->computeDownsamplePass.cmdBuffers[swapchainIndex]);
			_deviceResources->mainCommandBuffers[swapchainIndex]->endRenderPass();
		}
		else
		{
			_deviceResources->downsamplePass.recordCommands(swapchainIndex);

			_deviceResources->mainCommandBuffers[swapchainIndex]->beginRenderPass(_deviceResources->downsamplePass.framebuffers[swapchainIndex],
				_deviceResources->downsamplePass.renderPass, pvrvk::Rect2D(0, 0, _blurFramebufferDimensions.x, _blurFramebufferDimensions.y), false, &clearValues, 1);
			_deviceResources->mainCommandBuffers[swapchainIndex]->executeCommands(_deviceResources->downsamplePass.cmdBuffers[swapchainIndex]);
			_deviceResources->mainCommandBuffers[swapchainIndex]->endRenderPass();
		}
	}

	if (_blurMode != BloomMode::NoBloom)
	{
		// Record the current set of commands for bloom
		recordBlurCommands(_blurMode, swapchainIndex);

		switch (_blurMode)
		{
		case (BloomMode::GaussianOriginal): {
			_deviceResources->gaussianBlurPass.recordCommandsToMainCommandBuffer(swapchainIndex, _deviceResources->mainCommandBuffers[swapchainIndex], _deviceResources->queues[0],
				_deviceResources->blurRenderPass, _deviceResources->blurFramebuffers[0], _deviceResources->blurFramebuffers[1]);
			break;
		}
		case (BloomMode::GaussianLinear): {
			_deviceResources->linearGaussianBlurPass.recordCommandsToMainCommandBuffer(swapchainIndex, _deviceResources->mainCommandBuffers[swapchainIndex],
				_deviceResources->queues[0], _deviceResources->blurRenderPass, _deviceResources->blurFramebuffers[0], _deviceResources->blurFramebuffers[1]);
			break;
		}
		case (BloomMode::GaussianLinearTruncated): {
			_deviceResources->truncatedLinearGaussianBlurPass.recordCommandsToMainCommandBuffer(swapchainIndex, _deviceResources->mainCommandBuffers[swapchainIndex],
				_deviceResources->queues[0], _deviceResources->blurRenderPass, _deviceResources->blurFramebuffers[0], _deviceResources->blurFramebuffers[1]);
			break;
		}
		case (BloomMode::Compute): {
			// Graphics to Compute pipeline barrier (Downsample -> Compute Blur (horizontal))
			// Add a pipelineBarrier between fragment write (Downsample) -> shader read (Compute Blur (horizontal))
			{
				pvrvk::ImageLayout sourceImageLayout = pvrvk::ImageLayout::e_SHADER_READ_ONLY_OPTIMAL;
				pvrvk::ImageLayout destinationImageLayout = pvrvk::ImageLayout::e_GENERAL;

				pvrvk::MemoryBarrierSet layoutTransitions;
				layoutTransitions.addBarrier(pvrvk::ImageMemoryBarrier(pvrvk::AccessFlags::e_COLOR_ATTACHMENT_WRITE_BIT, pvrvk::AccessFlags::e_SHADER_WRITE_BIT,
					_deviceResources->sharedBlurImageViews[_pingPongImageIndices[1]][swapchainIndex]->getImage(), pvrvk::ImageSubresourceRange(pvrvk::ImageAspectFlags::e_COLOR_BIT),
					sourceImageLayout, destinationImageLayout, _deviceResources->queues[0]->getFamilyIndex(), _deviceResources->queues[0]->getFamilyIndex()));

				_deviceResources->mainCommandBuffers[swapchainIndex]->pipelineBarrier(
					pvrvk::PipelineStageFlags::e_COLOR_ATTACHMENT_OUTPUT_BIT, pvrvk::PipelineStageFlags::e_COMPUTE_SHADER_BIT, layoutTransitions);
			}

			_deviceResources->computeBlurPass.recordCommandsToMainCommandBuffer(swapchainIndex, _deviceResources->mainCommandBuffers[swapchainIndex]);
			break;
		}
		case (BloomMode::Kawase): {
			_deviceResources->kawaseBlurPass.recordCommandsToMainCommandBuffer(swapchainIndex, _deviceResources->mainCommandBuffers[swapchainIndex], _deviceResources->queues[0],
				_deviceResources->blurRenderPass, _deviceResources->blurFramebuffers);
			break;
		}
		case (BloomMode::DualFilter): {
			_deviceResources->dualFilterBlurPass.recordCommandsToMainCommandBuffer(swapchainIndex, _deviceResources->mainCommandBuffers[swapchainIndex], _deviceResources->queues[0],
				_deviceResources->blurRenderPass, _deviceResources->onScreenRenderPass, _deviceResources->onScreenFramebuffers[swapchainIndex], &clearValues, 1);
			break;
		}
		case (BloomMode::TentFilter): {
			_deviceResources->downAndTentFilterBlurPass.recordCommandsToMainCommandBuffer(swapchainIndex, _deviceResources->mainCommandBuffers[swapchainIndex],
				_deviceResources->queues[0], _deviceResources->blurRenderPass, _deviceResources->onScreenRenderPass, _deviceResources->onScreenFramebuffers[swapchainIndex],
				&clearValues, 1);
			break;
		}
		case (BloomMode::HybridGaussian): {
			// Graphics to Compute pipeline barrier (Downsample -> Compute Blur (horizontal))
			// Add a pipelineBarrier between fragment write (Downsample) -> shader read (Compute Blur (horizontal))
			{
				pvrvk::ImageLayout sourceImageLayout = pvrvk::ImageLayout::e_SHADER_READ_ONLY_OPTIMAL;
				pvrvk::ImageLayout destinationImageLayout = pvrvk::ImageLayout::e_GENERAL;

				pvrvk::MemoryBarrierSet layoutTransitions;
				layoutTransitions.addBarrier(pvrvk::ImageMemoryBarrier(pvrvk::AccessFlags::e_COLOR_ATTACHMENT_WRITE_BIT, pvrvk::AccessFlags::e_SHADER_WRITE_BIT,
					_deviceResources->sharedBlurImageViews[_pingPongImageIndices[1]][swapchainIndex]->getImage(), pvrvk::ImageSubresourceRange(pvrvk::ImageAspectFlags::e_COLOR_BIT),
					sourceImageLayout, destinationImageLayout, _deviceResources->queues[0]->getFamilyIndex(), _deviceResources->queues[0]->getFamilyIndex()));

				_deviceResources->mainCommandBuffers[swapchainIndex]->pipelineBarrier(
					pvrvk::PipelineStageFlags::e_COLOR_ATTACHMENT_OUTPUT_BIT, pvrvk::PipelineStageFlags::e_COMPUTE_SHADER_BIT, layoutTransitions);
			}

			_deviceResources->hybridGaussianBlurPass.recordCommandsToMainCommandBuffer(swapchainIndex, _deviceResources->mainCommandBuffers[swapchainIndex],
				_deviceResources->blurRenderPass, _deviceResources->hybridBlurFramebuffers[0], _deviceResources->hybridBlurFramebuffers[1]);

			break;
		}
		default: throw pvr::UnsupportedOperationError("Unsupported BlurMode.");
		}
	}

	// If Dual or Tent filter then the composition is taken care of during the final up sample
	if (_blurMode != BloomMode::DualFilter && _blurMode != BloomMode::TentFilter)
	{
		_deviceResources->mainCommandBuffers[swapchainIndex]->beginRenderPass(
			_deviceResources->onScreenFramebuffers[swapchainIndex], _deviceResources->onScreenRenderPass, pvrvk::Rect2D(0, 0, getWidth(), getHeight()), false, &clearValues, 1);

		// Ensure the post bloom pass uses the correct blurred image for the current blur mode
		switch (_blurMode)
		{
		case (BloomMode::GaussianOriginal): {
			_deviceResources->postBloomPass.updateDescriptorSets(_deviceResources->device, swapchainIndex, _deviceResources->offScreenColorImageViews[swapchainIndex],
				_deviceResources->gaussianBlurPass.getBlurredImage(swapchainIndex), _deviceResources->samplerBilinear, _deviceResources->sceneBuffer,
				_deviceResources->sceneBufferView, _deviceResources->diffuseIrradianceMapImageViews[_currentScene], _deviceResources->samplerTrilinear);
			break;
		}
		case (BloomMode::GaussianLinear): {
			_deviceResources->postBloomPass.updateDescriptorSets(_deviceResources->device, swapchainIndex, _deviceResources->offScreenColorImageViews[swapchainIndex],
				_deviceResources->linearGaussianBlurPass.getBlurredImage(swapchainIndex), _deviceResources->samplerBilinear, _deviceResources->sceneBuffer,
				_deviceResources->sceneBufferView, _deviceResources->diffuseIrradianceMapImageViews[_currentScene], _deviceResources->samplerTrilinear);
			break;
		}
		case (BloomMode::Compute): {
			_deviceResources->postBloomPass.updateDescriptorSets(_deviceResources->device, swapchainIndex, _deviceResources->offScreenColorImageViews[swapchainIndex],
				_deviceResources->computeBlurPass.getBlurredImage(swapchainIndex), _deviceResources->samplerBilinear, _deviceResources->sceneBuffer,
				_deviceResources->sceneBufferView, _deviceResources->diffuseIrradianceMapImageViews[_currentScene], _deviceResources->samplerTrilinear);
			break;
		}
		case (BloomMode::GaussianLinearTruncated): {
			_deviceResources->postBloomPass.updateDescriptorSets(_deviceResources->device, swapchainIndex, _deviceResources->offScreenColorImageViews[swapchainIndex],
				_deviceResources->truncatedLinearGaussianBlurPass.getBlurredImage(swapchainIndex), _deviceResources->samplerBilinear, _deviceResources->sceneBuffer,
				_deviceResources->sceneBufferView, _deviceResources->diffuseIrradianceMapImageViews[_currentScene], _deviceResources->samplerTrilinear);
			break;
		}
		case (BloomMode::Kawase): {
			_deviceResources->postBloomPass.updateDescriptorSets(_deviceResources->device, swapchainIndex, _deviceResources->offScreenColorImageViews[swapchainIndex],
				_deviceResources->kawaseBlurPass.getBlurredImage(swapchainIndex), _deviceResources->samplerBilinear, _deviceResources->sceneBuffer,
				_deviceResources->sceneBufferView, _deviceResources->diffuseIrradianceMapImageViews[_currentScene], _deviceResources->samplerTrilinear);
			break;
		}
		case (BloomMode::HybridGaussian): {
			_deviceResources->postBloomPass.updateDescriptorSets(_deviceResources->device, swapchainIndex, _deviceResources->offScreenColorImageViews[swapchainIndex],
				_deviceResources->hybridGaussianBlurPass.linearBlurPass->getBlurredImage(swapchainIndex), _deviceResources->samplerBilinear, _deviceResources->sceneBuffer,
				_deviceResources->sceneBufferView, _deviceResources->diffuseIrradianceMapImageViews[_currentScene], _deviceResources->samplerTrilinear);
			break;
		}
		case (BloomMode::NoBloom): {
			_deviceResources->postBloomPass.updateDescriptorSets(_deviceResources->device, swapchainIndex, _deviceResources->offScreenColorImageViews[swapchainIndex],
				_deviceResources->sharedBlurImageViews[_pingPongImageIndices[0]][swapchainIndex], _deviceResources->samplerBilinear, _deviceResources->sceneBuffer,
				_deviceResources->sceneBufferView, _deviceResources->diffuseIrradianceMapImageViews[_currentScene], _deviceResources->samplerTrilinear);
			break;
		}
		default: throw pvr::UnsupportedOperationError("Unsupported BlurMode.");
		}

		_deviceResources->postBloomPass.recordCommandBuffer(swapchainIndex, _deviceResources->onScreenFramebuffers[swapchainIndex], _renderOnlyBloom, _exposure);
		_deviceResources->mainCommandBuffers[swapchainIndex]->executeCommands(_deviceResources->postBloomPass.cmdBuffers[swapchainIndex]);
	}

	_deviceResources->mainCommandBuffers[swapchainIndex]->executeCommands(_deviceResources->bloomUiRendererCommandBuffers[swapchainIndex]);
	_deviceResources->mainCommandBuffers[swapchainIndex]->endRenderPass();
	_deviceResources->mainCommandBuffers[swapchainIndex]->end();
}

/// <summary>Records the commands necessary for the current bloom technique.</summary>
/// <param name="swapchainIndex">The swapchain index for which the recorded command buffer will be used.</param>
void VulkanPostProcessing::recordBlurCommands(BloomMode blurMode, uint32_t swapchainIndex)
{
	switch (blurMode)
	{
	case (BloomMode::GaussianOriginal): {
		_deviceResources->gaussianBlurPass.recordCommands(swapchainIndex, _deviceResources->blurFramebuffers[0], _deviceResources->blurFramebuffers[1]);
		break;
	}
	case (BloomMode::GaussianLinear): {
		_deviceResources->linearGaussianBlurPass.recordCommands(swapchainIndex, _deviceResources->blurFramebuffers[0], _deviceResources->blurFramebuffers[1]);
		break;
	}
	case (BloomMode::GaussianLinearTruncated): {
		_deviceResources->truncatedLinearGaussianBlurPass.recordCommands(swapchainIndex, _deviceResources->blurFramebuffers[0], _deviceResources->blurFramebuffers[1]);
		break;
	}
	case (BloomMode::Compute): {
		_deviceResources->computeBlurPass.recordCommands(swapchainIndex, _deviceResources->sharedBlurImageViews[_pingPongImageIndices[0]],
			_deviceResources->sharedBlurImageViews[_pingPongImageIndices[1]], _deviceResources->queues[_queueIndex]);
		break;
	}
	case (BloomMode::Kawase): {
		_deviceResources->kawaseBlurPass.recordCommands(swapchainIndex, _deviceResources->blurFramebuffers, 2);
		break;
	}
	case (BloomMode::DualFilter): {
		_deviceResources->dualFilterBlurPass.recordCommands(swapchainIndex, _deviceResources->onScreenFramebuffers[swapchainIndex], _renderOnlyBloom, _exposure);
		break;
	}
	case (BloomMode::TentFilter): {
		_deviceResources->downAndTentFilterBlurPass.recordCommands(swapchainIndex, _deviceResources->onScreenFramebuffers[swapchainIndex], _renderOnlyBloom,
			_deviceResources->queues[_queueIndex], _deviceResources->luminanceImageViews[_deviceResources->swapchain->getSwapchainIndex()], _exposure);
		break;
	}
	case (BloomMode::HybridGaussian): {
		_deviceResources->hybridGaussianBlurPass.recordCommands(
			swapchainIndex, _deviceResources->hybridBlurFramebuffers[0], _deviceResources->hybridBlurFramebuffers[1], _deviceResources->queues[_queueIndex]);
		break;
	}
	default: throw pvr::UnsupportedOperationError("Unsupported BlurMode.");
	}
}

/// <summary>Allocates the various ping pong image views used throughout the demo.</summary>
void VulkanPostProcessing::allocatePingPongImages()
{
	pvrvk::Extent3D dimension = pvrvk::Extent3D(_blurFramebufferDimensions.x, _blurFramebufferDimensions.y, 1u);

	// Allocate the luminance render targets (we need to ping pong between 2 targets)
	pvrvk::ImageUsageFlags storageImageUsage = pvrvk::ImageUsageFlags::e_COLOR_ATTACHMENT_BIT | pvrvk::ImageUsageFlags::e_SAMPLED_BIT | pvrvk::ImageUsageFlags::e_STORAGE_BIT;

	_deviceResources->sharedBlurImageViews.resize(_deviceResources->sharedBlurImageViews.size() + 2);

	_pingPongImageIndices[0] = _deviceResources->sharedBlurImageViews.size() - 2;
	_pingPongImageIndices[1] = _deviceResources->sharedBlurImageViews.size() - 1;

	for (uint32_t i = 0; i < _deviceResources->swapchain->getSwapchainLength(); ++i)
	{
		for (uint32_t j = 0; j < 2; ++j)
		{
			pvrvk::Image blurColorTexture = pvr::utils::createImage(_deviceResources->device,
				pvrvk::ImageCreateInfo(pvrvk::ImageType::e_2D, _storageImageLuminanceColorFormat, dimension, storageImageUsage, 1, 1, pvrvk::SampleCountFlags::e_1_BIT,
					pvrvk::ImageCreateFlags(0), _storageImageTiling),
				pvrvk::MemoryPropertyFlags::e_DEVICE_LOCAL_BIT, pvrvk::MemoryPropertyFlags::e_NONE, _deviceResources->vmaAllocator, pvr::utils::vma::AllocationCreateFlags::e_NONE);
			_deviceResources->sharedBlurImageViews[_pingPongImageIndices[j]].add(_deviceResources->device->createImageView(pvrvk::ImageViewCreateInfo(blurColorTexture)));
		}
	}
}

/// <summary>Creates the various samplers used throughout the demo.</summary>
void VulkanPostProcessing::createSamplers()
{
	pvrvk::SamplerCreateInfo samplerInfo;
	samplerInfo.wrapModeU = samplerInfo.wrapModeV = samplerInfo.wrapModeW = pvrvk::SamplerAddressMode::e_CLAMP_TO_EDGE;

	samplerInfo.minFilter = pvrvk::Filter::e_LINEAR;
	samplerInfo.magFilter = pvrvk::Filter::e_LINEAR;
	samplerInfo.mipMapMode = pvrvk::SamplerMipmapMode::e_NEAREST;
	_deviceResources->samplerBilinear = _deviceResources->device->createSampler(samplerInfo);

	samplerInfo.minFilter = pvrvk::Filter::e_NEAREST;
	samplerInfo.magFilter = pvrvk::Filter::e_NEAREST;
	_deviceResources->samplerNearest = _deviceResources->device->createSampler(samplerInfo);

	samplerInfo.magFilter = pvrvk::Filter::e_LINEAR;
	samplerInfo.minFilter = pvrvk::Filter::e_LINEAR;
	samplerInfo.mipMapMode = pvrvk::SamplerMipmapMode::e_LINEAR;
	_deviceResources->samplerTrilinear = _deviceResources->device->createSampler(samplerInfo);
}

/// <summary>Create the framebuffers which will be used in the various bloom passes.</summary>
void VulkanPostProcessing::createBlurFramebuffers()
{
	for (uint32_t i = 0; i < 2; ++i)
	{
		for (uint32_t j = 0; j < _deviceResources->swapchain->getSwapchainLength(); ++j)
		{
			pvrvk::FramebufferCreateInfo createInfo;
			createInfo.setAttachment(0, _deviceResources->sharedBlurImageViews[!_pingPongImageIndices[i]][j]);
			createInfo.setDimensions(_blurFramebufferDimensions.x, _blurFramebufferDimensions.y);
			createInfo.setRenderPass(_deviceResources->blurRenderPass);

			_deviceResources->blurFramebuffers[i].add(_deviceResources->device->createFramebuffer(createInfo));
		}
	}
}

/// <summary>Create the framebuffers which will be used in the hybrid bloom pass.</summary>
void VulkanPostProcessing::createHybridBlurFramebuffers()
{
	for (uint32_t i = 0; i < _deviceResources->swapchain->getSwapchainLength(); ++i)
	{
		pvrvk::FramebufferCreateInfo createInfo;
		createInfo.setAttachment(0, _deviceResources->sharedBlurImageViews[_pingPongImageIndices[1]][i]);
		createInfo.setDimensions(_blurFramebufferDimensions.x, _blurFramebufferDimensions.y);
		createInfo.setRenderPass(_deviceResources->hybridBlurRenderPass);

		_deviceResources->hybridBlurFramebuffers[0].add(_deviceResources->device->createFramebuffer(createInfo));
	}

	for (uint32_t i = 0; i < _deviceResources->swapchain->getSwapchainLength(); ++i)
	{
		pvrvk::FramebufferCreateInfo createInfo;
		createInfo.setAttachment(0, _deviceResources->sharedBlurImageViews[_pingPongImageIndices[0]][i]);
		createInfo.setDimensions(_blurFramebufferDimensions.x, _blurFramebufferDimensions.y);
		createInfo.setRenderPass(_deviceResources->blurRenderPass);

		_deviceResources->hybridBlurFramebuffers[1].add(_deviceResources->device->createFramebuffer(createInfo));
	}
}

/// <summary>Create the RenderPasses used in the various bloom passes.</summary>
void VulkanPostProcessing::createBlurRenderPass()
{
	pvrvk::RenderPassCreateInfo renderPassInfo;

	renderPassInfo.setAttachmentDescription(0,
		pvrvk::AttachmentDescription::createColorDescription(_deviceResources->sharedBlurImageViews[_pingPongImageIndices[0]][0]->getFormat(), pvrvk::ImageLayout::e_UNDEFINED,
			pvrvk::ImageLayout::e_SHADER_READ_ONLY_OPTIMAL, pvrvk::AttachmentLoadOp::e_DONT_CARE, pvrvk::AttachmentStoreOp::e_STORE, pvrvk::SampleCountFlags::e_1_BIT));

	pvrvk::SubpassDescription subpass;
	subpass.setColorAttachmentReference(0, pvrvk::AttachmentReference(0, pvrvk::ImageLayout::e_COLOR_ATTACHMENT_OPTIMAL));
	renderPassInfo.setSubpass(0, subpass);

	// Add external subpass dependencies to avoid the implicit subpass dependencies and to provide more optimal pipeline stage task synchronisation
	pvrvk::SubpassDependency externalDependencies[2];
	externalDependencies[0] = pvrvk::SubpassDependency(pvrvk::SubpassExternal, 0, pvrvk::PipelineStageFlags::e_COLOR_ATTACHMENT_OUTPUT_BIT,
		pvrvk::PipelineStageFlags::e_FRAGMENT_SHADER_BIT, pvrvk::AccessFlags::e_COLOR_ATTACHMENT_WRITE_BIT, pvrvk::AccessFlags::e_SHADER_READ_BIT, pvrvk::DependencyFlags::e_NONE);

	externalDependencies[1] = pvrvk::SubpassDependency(0, pvrvk::SubpassExternal, pvrvk::PipelineStageFlags::e_COLOR_ATTACHMENT_OUTPUT_BIT,
		pvrvk::PipelineStageFlags::e_FRAGMENT_SHADER_BIT, pvrvk::AccessFlags::e_COLOR_ATTACHMENT_WRITE_BIT, pvrvk::AccessFlags::e_SHADER_READ_BIT, pvrvk::DependencyFlags::e_NONE);

	renderPassInfo.addSubpassDependency(externalDependencies[0]);
	renderPassInfo.addSubpassDependency(externalDependencies[1]);

	_deviceResources->blurRenderPass = _deviceResources->device->createRenderPass(renderPassInfo);
}

/// <summary>Create the RenderPasses used in the hybrid bloom passes.</summary>
void VulkanPostProcessing::createHybridBlurRenderPass()
{
	pvrvk::RenderPassCreateInfo renderPassInfo;

	renderPassInfo.setAttachmentDescription(0,
		pvrvk::AttachmentDescription::createColorDescription(_deviceResources->sharedBlurImageViews[_pingPongImageIndices[0]][0]->getFormat(), pvrvk::ImageLayout::e_GENERAL,
			pvrvk::ImageLayout::e_SHADER_READ_ONLY_OPTIMAL, pvrvk::AttachmentLoadOp::e_DONT_CARE, pvrvk::AttachmentStoreOp::e_STORE, pvrvk::SampleCountFlags::e_1_BIT));

	pvrvk::SubpassDescription subpass;
	subpass.setColorAttachmentReference(0, pvrvk::AttachmentReference(0, pvrvk::ImageLayout::e_COLOR_ATTACHMENT_OPTIMAL));
	renderPassInfo.setSubpass(0, subpass);

	// Add external subpass dependencies to avoid the implicit subpass dependencies and to provide more optimal pipeline stage task synchronisation
	pvrvk::SubpassDependency externalDependencies[2];
	externalDependencies[0] = pvrvk::SubpassDependency(pvrvk::SubpassExternal, 0, pvrvk::PipelineStageFlags::e_COMPUTE_SHADER_BIT, pvrvk::PipelineStageFlags::e_FRAGMENT_SHADER_BIT,
		pvrvk::AccessFlags::e_SHADER_WRITE_BIT, pvrvk::AccessFlags::e_SHADER_READ_BIT, pvrvk::DependencyFlags::e_NONE);

	externalDependencies[1] = pvrvk::SubpassDependency(0, pvrvk::SubpassExternal, pvrvk::PipelineStageFlags::e_COLOR_ATTACHMENT_OUTPUT_BIT,
		pvrvk::PipelineStageFlags::e_FRAGMENT_SHADER_BIT, pvrvk::AccessFlags::e_COLOR_ATTACHMENT_WRITE_BIT, pvrvk::AccessFlags::e_SHADER_READ_BIT, pvrvk::DependencyFlags::e_NONE);

	renderPassInfo.addSubpassDependency(externalDependencies[0]);
	renderPassInfo.addSubpassDependency(externalDependencies[1]);

	_deviceResources->hybridBlurRenderPass = _deviceResources->device->createRenderPass(renderPassInfo);
}

/// <summary>Create the offscreen framebuffers used in the application.</summary>
void VulkanPostProcessing::createOffScreenFramebuffers()
{
	pvrvk::RenderPassCreateInfo renderPassInfo;

	// Off-Screen attachment
	renderPassInfo.setAttachmentDescription(0,
		pvrvk::AttachmentDescription::createColorDescription(pvrvk::Format::e_R16G16B16A16_SFLOAT, pvrvk::ImageLayout::e_UNDEFINED, pvrvk::ImageLayout::e_SHADER_READ_ONLY_OPTIMAL,
			pvrvk::AttachmentLoadOp::e_CLEAR, pvrvk::AttachmentStoreOp::e_STORE, pvrvk::SampleCountFlags::e_1_BIT));

	renderPassInfo.setAttachmentDescription(1,
		pvrvk::AttachmentDescription::createColorDescription(_luminanceColorFormat, pvrvk::ImageLayout::e_UNDEFINED, pvrvk::ImageLayout::e_SHADER_READ_ONLY_OPTIMAL,
			pvrvk::AttachmentLoadOp::e_CLEAR, pvrvk::AttachmentStoreOp::e_STORE, pvrvk::SampleCountFlags::e_1_BIT));

	renderPassInfo.setAttachmentDescription(2,
		pvrvk::AttachmentDescription::createDepthStencilDescription(_deviceResources->depthStencilImages[0]->getImage()->getFormat(), pvrvk::ImageLayout::e_UNDEFINED,
			pvrvk::ImageLayout::e_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, pvrvk::AttachmentLoadOp::e_CLEAR, pvrvk::AttachmentStoreOp::e_DONT_CARE, pvrvk::AttachmentLoadOp::e_CLEAR,
			pvrvk::AttachmentStoreOp::e_DONT_CARE, pvrvk::SampleCountFlags::e_1_BIT));

	pvrvk::SubpassDescription offscreenSubpass;
	offscreenSubpass.setColorAttachmentReference(0, pvrvk::AttachmentReference(0, pvrvk::ImageLayout::e_COLOR_ATTACHMENT_OPTIMAL));
	offscreenSubpass.setColorAttachmentReference(1, pvrvk::AttachmentReference(1, pvrvk::ImageLayout::e_COLOR_ATTACHMENT_OPTIMAL));
	offscreenSubpass.setDepthStencilAttachmentReference(pvrvk::AttachmentReference(2, pvrvk::ImageLayout::e_DEPTH_STENCIL_ATTACHMENT_OPTIMAL));
	renderPassInfo.setSubpass(0, offscreenSubpass);

	// Add external subpass dependencies to avoid the implicit subpass dependencies and to provide more optimal pipeline stage task synchronisation
	pvrvk::SubpassDependency externalDependencies[2];
	externalDependencies[0] = pvrvk::SubpassDependency(pvrvk::SubpassExternal, 0, pvrvk::PipelineStageFlags::e_COLOR_ATTACHMENT_OUTPUT_BIT,
		pvrvk::PipelineStageFlags::e_FRAGMENT_SHADER_BIT, pvrvk::AccessFlags::e_COLOR_ATTACHMENT_WRITE_BIT, pvrvk::AccessFlags::e_SHADER_READ_BIT, pvrvk::DependencyFlags::e_NONE);

	externalDependencies[1] = pvrvk::SubpassDependency(0, pvrvk::SubpassExternal, pvrvk::PipelineStageFlags::e_COLOR_ATTACHMENT_OUTPUT_BIT,
		pvrvk::PipelineStageFlags::e_FRAGMENT_SHADER_BIT, pvrvk::AccessFlags::e_COLOR_ATTACHMENT_WRITE_BIT, pvrvk::AccessFlags::e_SHADER_READ_BIT, pvrvk::DependencyFlags::e_NONE);

	renderPassInfo.addSubpassDependency(externalDependencies[0]);
	renderPassInfo.addSubpassDependency(externalDependencies[1]);

	// Create the renderpass
	_deviceResources->offScreenRenderPass = _deviceResources->device->createRenderPass(renderPassInfo);

	const pvrvk::Extent3D& dimension = pvrvk::Extent3D(_deviceResources->swapchain->getDimension().getWidth(), _deviceResources->swapchain->getDimension().getHeight(), 1u);
	for (uint32_t i = 0; i < _deviceResources->swapchain->getSwapchainLength(); ++i)
	{
		pvrvk::ImageUsageFlags imageUsage = pvrvk::ImageUsageFlags::e_COLOR_ATTACHMENT_BIT | pvrvk::ImageUsageFlags::e_SAMPLED_BIT;

		// Allocate the HDR luminance texture
		pvrvk::Image luminanceColorTexture =
			pvr::utils::createImage(_deviceResources->device, pvrvk::ImageCreateInfo(pvrvk::ImageType::e_2D, _luminanceColorFormat, dimension, imageUsage),
				pvrvk::MemoryPropertyFlags::e_DEVICE_LOCAL_BIT, pvrvk::MemoryPropertyFlags::e_NONE, _deviceResources->vmaAllocator);

		_deviceResources->luminanceImageViews.add(_deviceResources->device->createImageView(pvrvk::ImageViewCreateInfo(luminanceColorTexture)));

		pvrvk::FramebufferCreateInfo offScreenFramebufferCreateInfo;

		// Allocate the HDR colour texture
		pvrvk::Image colorTexture = pvr::utils::createImage(_deviceResources->device,
			pvrvk::ImageCreateInfo(pvrvk::ImageType::e_2D, pvrvk::Format::e_R16G16B16A16_SFLOAT, dimension,
				pvrvk::ImageUsageFlags::e_COLOR_ATTACHMENT_BIT | pvrvk::ImageUsageFlags::e_SAMPLED_BIT | pvrvk::ImageUsageFlags::e_INPUT_ATTACHMENT_BIT),
			pvrvk::MemoryPropertyFlags::e_DEVICE_LOCAL_BIT, pvrvk::MemoryPropertyFlags::e_NONE, _deviceResources->vmaAllocator);

		_deviceResources->offScreenColorImageViews.add(_deviceResources->device->createImageView(pvrvk::ImageViewCreateInfo(colorTexture)));

		offScreenFramebufferCreateInfo.setAttachment(0, _deviceResources->offScreenColorImageViews[i]);
		offScreenFramebufferCreateInfo.setAttachment(1, _deviceResources->luminanceImageViews[i]);
		offScreenFramebufferCreateInfo.setAttachment(2, _deviceResources->depthStencilImages[i]);
		offScreenFramebufferCreateInfo.setDimensions(_deviceResources->swapchain->getDimension());
		offScreenFramebufferCreateInfo.setRenderPass(_deviceResources->offScreenRenderPass);

		_deviceResources->offScreenFramebuffers[i] = _deviceResources->device->createFramebuffer(offScreenFramebufferCreateInfo);
	}
}

/// <summary>Update the various dynamic scene data used in the application.</summary>
void VulkanPostProcessing::updateDynamicSceneData()
{
	// Update object animations
	updateAnimation();

	// Update the animation data used in the statue pass
	_deviceResources->statuePass.updateAnimation(_objectAngleY, _viewProjectionMatrix, _deviceResources->swapchain->getSwapchainIndex());

	// Update the Scene Buffer
	_deviceResources->sceneBufferView.getElementByName(BufferEntryNames::Scene::InverseViewProjectionMatrix, 0, _swapchainIndex).setValue(glm::inverse(_viewProjectionMatrix));
	_deviceResources->sceneBufferView.getElementByName(BufferEntryNames::Scene::EyePosition, 0, _swapchainIndex).setValue(_camera.getCameraPosition());

	_exposure = SceneTexFileNames[_currentScene].getLinearExposure();
	_threshold = SceneTexFileNames[_currentScene].threshold;

	// Update any bloom configuration buffers currently required
	if (_mustUpdatePerSwapchainDemoConfig[_deviceResources->swapchain->getSwapchainIndex()])
	{
		switch (_blurMode)
		{
		case (BloomMode::DualFilter): {
			_deviceResources->dualFilterBlurPass.updateDescriptorSets(_deviceResources->device, _deviceResources->swapchain->getSwapchainIndex());
			break;
		}
		case (BloomMode::TentFilter): {
			_deviceResources->downAndTentFilterBlurPass.updateDescriptorSets(_deviceResources->device, _deviceResources->swapchain->getSwapchainIndex());
			break;
		}
		default: break;
		}

		_mustUpdatePerSwapchainDemoConfig[_deviceResources->swapchain->getSwapchainIndex()] = false;
	}
}

/// <summary>Update the animations for the current frame.</summary>
void VulkanPostProcessing::updateAnimation()
{
	if (_animateCamera)
	{
		_cameraAngle += 0.15f;

		if (_cameraAngle >= 360.0f) { _cameraAngle = _cameraAngle - 360.f; }
	}

	_camera.setTargetLookAngle(_cameraAngle);

	_viewMatrix = _camera.getViewMatrix();
	_viewProjectionMatrix = _projectionMatrix * _viewMatrix;

	if (_animateObject) { _objectAngleY += RotateY * 0.03f * getFrameTime(); }

	float dt = getFrameTime() * 0.001f;
	_logicTime += dt;
	if (_logicTime > 10000000) { _logicTime = 0; }

	if (!_isManual)
	{
		if (_logicTime > _modeSwitchTime + _modeDuration)
		{
			_modeSwitchTime = _logicTime;

			if (_blurMode != BloomMode::NoBloom)
			{
				// Increase the demo configuration
				_currentDemoConfiguration = (_currentDemoConfiguration + 1) % DemoConfigurations::NumDemoConfigurations;
			}
			// Change to the next bloom mode
			if (_currentDemoConfiguration == 0 || _blurMode == BloomMode::NoBloom)
			{
				uint32_t currentBlurMode = static_cast<uint32_t>(_blurMode);
				currentBlurMode += 1;
				currentBlurMode = (currentBlurMode + static_cast<uint32_t>(BloomMode::NumBloomModes)) % static_cast<uint32_t>(BloomMode::NumBloomModes);
				_blurMode = static_cast<BloomMode>(currentBlurMode);
			}

			if (_blurMode == BloomMode::NoBloom) { (++_currentScene) %= NumScenes; }

			updateBloomConfiguration();
		}
	}
}

/// <summary>Update the demo configuration in use. Calculates Gaussian weights and offsets, images being used, framebuffers being used etc.</summary>
void VulkanPostProcessing::updateDemoConfigs()
{
	switch (_blurMode)
	{
	case (BloomMode::GaussianOriginal): {
		_deviceResources->gaussianBlurPass.updateKernelConfig(_currentDemoConfiguration);
		break;
	}
	case (BloomMode::GaussianLinear): {
		_deviceResources->linearGaussianBlurPass.updateKernelConfig(_currentDemoConfiguration);
		break;
	}
	case (BloomMode::GaussianLinearTruncated): {
		_deviceResources->truncatedLinearGaussianBlurPass.updateKernelConfig(_currentDemoConfiguration);
		break;
	}
	case (BloomMode::Kawase): {
		std::vector<pvr::Multi<pvrvk::ImageView>> kawaseImages;
		kawaseImages.emplace_back(_deviceResources->sharedBlurImageViews[_pingPongImageIndices[0]]);
		kawaseImages.emplace_back(_deviceResources->sharedBlurImageViews[_pingPongImageIndices[1]]);

		_deviceResources->kawaseBlurPass.updateConfig(DemoConfigurations::Configurations[_currentDemoConfiguration].kawaseConfig.kernel,
			DemoConfigurations::Configurations[_currentDemoConfiguration].kawaseConfig.numIterations, kawaseImages.data(), 2);
		break;
	}
	case (BloomMode::Compute): {
		_deviceResources->computeBlurPass.updateKernelConfig(_currentDemoConfiguration);
		break;
	}
	case (BloomMode::DualFilter): {
		_deviceResources->dualFilterBlurPass.updateConfig(DemoConfigurations::Configurations[_currentDemoConfiguration].dualFilterConfig);
		break;
	}
	case (BloomMode::TentFilter): {
		_deviceResources->downAndTentFilterBlurPass.updateConfig(DemoConfigurations::Configurations[_currentDemoConfiguration].dualFilterConfig);
		break;
	}
	case (BloomMode::HybridGaussian): {
		_deviceResources->truncatedLinearGaussianBlurPass.updateKernelConfig(_currentDemoConfiguration);
		_deviceResources->computeBlurPass.updateKernelConfig(_currentDemoConfiguration);
		break;
	}
	default: break;
	}
}

/// <summary>Update the bloom configuration.</summary>
void VulkanPostProcessing::updateBloomConfiguration()
{
	updateDemoConfigs();

	updateBlurDescription();
	_deviceResources->uiRenderer.getDefaultDescription()->setText(_currentBlurString);
	_deviceResources->uiRenderer.getDefaultDescription()->commitUpdates();

	for (uint32_t i = 0; i < _deviceResources->swapchain->getSwapchainLength(); ++i)
	{
		_mustRecordPrimaryCommandBuffer[i] = true;
		_mustUpdatePerSwapchainDemoConfig[i] = true;
	}
}

void VulkanPostProcessing::handleDesktopInput()
{
#ifdef PVR_PLATFORM_IS_DESKTOP
	if (isKeyPressed(pvr::Keys::PageDown))
	{
		SceneTexFileNames[_currentScene].keyValue *= .85f;
		for (uint32_t i = 0; i < _deviceResources->swapchain->getSwapchainLength(); ++i)
		{
			_mustRecordPrimaryCommandBuffer[i] = true;
			_mustUpdatePerSwapchainDemoConfig[i] = true;
		}
	}
	if (isKeyPressed(pvr::Keys::PageUp))
	{
		SceneTexFileNames[_currentScene].keyValue *= 1.15f;
		for (uint32_t i = 0; i < _deviceResources->swapchain->getSwapchainLength(); ++i)
		{
			_mustRecordPrimaryCommandBuffer[i] = true;
			_mustUpdatePerSwapchainDemoConfig[i] = true;
		}
	}

	SceneTexFileNames[_currentScene].keyValue = glm::clamp(SceneTexFileNames[_currentScene].keyValue, 0.001f, 100.0f);

	if (isKeyPressed(pvr::Keys::SquareBracketLeft))
	{
		SceneTexFileNames[_currentScene].threshold -= 0.05f;
		for (uint32_t i = 0; i < _deviceResources->swapchain->getSwapchainLength(); ++i)
		{
			_mustRecordPrimaryCommandBuffer[i] = true;
			_mustUpdatePerSwapchainDemoConfig[i] = true;
		}
	}
	if (isKeyPressed(pvr::Keys::SquareBracketRight))
	{
		SceneTexFileNames[_currentScene].threshold += 0.05f;
		for (uint32_t i = 0; i < _deviceResources->swapchain->getSwapchainLength(); ++i)
		{
			_mustRecordPrimaryCommandBuffer[i] = true;
			_mustUpdatePerSwapchainDemoConfig[i] = true;
		}
	}

	SceneTexFileNames[_currentScene].threshold = glm::clamp(SceneTexFileNames[_currentScene].threshold, 0.05f, 20.0f);
#endif
}

/// <summary>Handles user input and updates live variables accordingly.</summary>
void VulkanPostProcessing::eventMappedInput(pvr::SimplifiedInput e)
{
	switch (e)
	{
	case pvr::SimplifiedInput::Up: {
		_currentDemoConfiguration = (_currentDemoConfiguration + 1) % DemoConfigurations::NumDemoConfigurations;
		updateBloomConfiguration();
		_isManual = true;
		break;
	}
	case pvr::SimplifiedInput::Down: {
		if (_currentDemoConfiguration == 0) { _currentDemoConfiguration = DemoConfigurations::NumDemoConfigurations; }
		_currentDemoConfiguration = (_currentDemoConfiguration - 1) % DemoConfigurations::NumDemoConfigurations;
		updateBloomConfiguration();
		_isManual = true;
		break;
	}
	case pvr::SimplifiedInput::Left: {
		uint32_t currentBlurMode = static_cast<uint32_t>(_blurMode);
		currentBlurMode -= 1;
		currentBlurMode = (currentBlurMode + static_cast<uint32_t>(BloomMode::NumBloomModes)) % static_cast<uint32_t>(BloomMode::NumBloomModes);
		_blurMode = static_cast<BloomMode>(currentBlurMode);
		updateBloomConfiguration();
		_isManual = true;
		break;
	}
	case pvr::SimplifiedInput::Right: {
		uint32_t currentBlurMode = static_cast<uint32_t>(_blurMode);
		currentBlurMode += 1;
		currentBlurMode = (currentBlurMode + static_cast<uint32_t>(BloomMode::NumBloomModes)) % static_cast<uint32_t>(BloomMode::NumBloomModes);
		_blurMode = static_cast<BloomMode>(currentBlurMode);
		updateBloomConfiguration();
		_isManual = true;
		break;
	}
	case pvr::SimplifiedInput::ActionClose: {
		this->exitShell();
		break;
	}
	case pvr::SimplifiedInput::Action1: {
		_renderOnlyBloom = !_renderOnlyBloom;
		for (uint32_t i = 0; i < _deviceResources->swapchain->getSwapchainLength(); ++i) { _mustRecordPrimaryCommandBuffer[i] = true; }
		break;
	}
	case pvr::SimplifiedInput::Action2: {
		_animateObject = !_animateObject;
		_animateCamera = !_animateCamera;
		for (uint32_t i = 0; i < _deviceResources->swapchain->getSwapchainLength(); ++i) { _mustRecordPrimaryCommandBuffer[i] = true; }
		break;
	}
	case pvr::SimplifiedInput::Action3: {
		(++_currentScene) %= NumScenes;
		for (uint32_t i = 0; i < _deviceResources->swapchain->getSwapchainLength(); ++i) { _mustRecordPrimaryCommandBuffer[i] = true; }
		break;
	}
	default: {
		break;
	}
	}
}

/// <summary>Records the UI Renderer commands.</summary>
/// <param name="swapchainIndex">The swapchain index for which the recorded command buffer will be used.</param>
/// <param name="commandBuffers">The secondary command buffer to which UI Renderer commands should be recorded.</param>
void VulkanPostProcessing::recordUIRendererCommands(uint32_t swapchainIndex, pvr::Multi<pvrvk::SecondaryCommandBuffer>& cmdBuffers)
{
	cmdBuffers.add(_deviceResources->commandPool->allocateSecondaryCommandBuffer());

	cmdBuffers[swapchainIndex]->begin(_deviceResources->onScreenFramebuffers[swapchainIndex], 0, pvrvk::CommandBufferUsageFlags::e_RENDER_PASS_CONTINUE_BIT);

	_deviceResources->uiRenderer.beginRendering(cmdBuffers[swapchainIndex]);

	_deviceResources->uiRenderer.getSdkLogo()->render();
	_deviceResources->uiRenderer.getDefaultTitle()->render();
	_deviceResources->uiRenderer.getDefaultControls()->render();
	_deviceResources->uiRenderer.getDefaultDescription()->render();
	_deviceResources->uiRenderer.endRendering();
	cmdBuffers[swapchainIndex]->end();
}

/// <summary>This function must be implemented by the user of the shell. The user should return its pvr::Shell object defining the behaviour of the application.</summary>
/// <returns>Return a unique ptr to the demo supplied by the user.</returns>
std::unique_ptr<pvr::Shell> pvr::newDemo() { return std::make_unique<VulkanPostProcessing>(); }
