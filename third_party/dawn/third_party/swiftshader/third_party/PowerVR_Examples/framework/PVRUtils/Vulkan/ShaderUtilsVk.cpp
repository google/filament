/*!
\brief Implementations of the shader utility functions
\file PVRUtils/Vulkan/ShaderUtilsVk.cpp
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
//!\cond NO_DOXYGEN
#include "PVRUtils/Vulkan/ShaderUtilsVk.h"
#include "PVRVk/ShaderModuleVk.h"
#include "PVRCore/strings/StringFunctions.h"
#include "glslang/Public/ShaderLang.h"
#include "SPIRV/GlslangToSpv.h"

namespace pvr {
namespace utils {

namespace {
/// <summary>Used to initialise glslang - the constructor must be called exactly once per process.</summary>
class GlslangProcessInitialiser
{
public:
	/// <summary>The constructor should be called exactly once prior to using glslang.</summary>
	GlslangProcessInitialiser() { glslang::InitializeProcess(); }
	/// <summary>The destructor should be called exactly once after using glslang.</summary>
	~GlslangProcessInitialiser() { glslang::FinalizeProcess(); }
};

struct TBuiltInResourceInitialiser
{
	std::unique_ptr<TBuiltInResource> tBuiltInResourcePtr;
	TBuiltInResourceInitialiser(pvrvk::Device device) : tBuiltInResourcePtr(std::make_unique<TBuiltInResource>())
	{
		// Copied from StandAlone/ResourceLimits.cpp
		TBuiltInResource defaultTBuiltInResource = { /* .MaxLights = */ 32,
			/* .MaxClipPlanes = */ 6,
			/* .MaxTextureUnits = */ 32,
			/* .MaxTextureCoords = */ 32,
			/* .MaxVertexAttribs = */ 64,
			/* .MaxVertexUniformComponents = */ 4096,
			/* .MaxVaryingFloats = */ 64,
			/* .MaxVertexTextureImageUnits = */ 32,
			/* .MaxCombinedTextureImageUnits = */ 80,
			/* .MaxTextureImageUnits = */ 32,
			/* .MaxFragmentUniformComponents = */ 4096,
			/* .MaxDrawBuffers = */ 32,
			/* .MaxVertexUniformVectors = */ 128,
			/* .MaxVaryingVectors = */ 8,
			/* .MaxFragmentUniformVectors = */ 16,
			/* .MaxVertexOutputVectors = */ 16,
			/* .MaxFragmentInputVectors = */ 15,
			/* .MinProgramTexelOffset = */ -8,
			/* .MaxProgramTexelOffset = */ 7,
			/* .MaxClipDistances = */ 8,
			/* .MaxComputeWorkGroupCountX = */ 65535,
			/* .MaxComputeWorkGroupCountY = */ 65535,
			/* .MaxComputeWorkGroupCountZ = */ 65535,
			/* .MaxComputeWorkGroupSizeX = */ 1024,
			/* .MaxComputeWorkGroupSizeY = */ 1024,
			/* .MaxComputeWorkGroupSizeZ = */ 64,
			/* .MaxComputeUniformComponents = */ 1024,
			/* .MaxComputeTextureImageUnits = */ 16,
			/* .MaxComputeImageUniforms = */ 8,
			/* .MaxComputeAtomicCounters = */ 8,
			/* .MaxComputeAtomicCounterBuffers = */ 1,
			/* .MaxVaryingComponents = */ 60,
			/* .MaxVertexOutputComponents = */ 64,
			/* .MaxGeometryInputComponents = */ 64,
			/* .MaxGeometryOutputComponents = */ 128,
			/* .MaxFragmentInputComponents = */ 128,
			/* .MaxImageUnits = */ 8,
			/* .MaxCombinedImageUnitsAndFragmentOutputs = */ 8,
			/* .MaxCombinedShaderOutputResources = */ 8,
			/* .MaxImageSamples = */ 0,
			/* .MaxVertexImageUniforms = */ 0,
			/* .MaxTessControlImageUniforms = */ 0,
			/* .MaxTessEvaluationImageUniforms = */ 0,
			/* .MaxGeometryImageUniforms = */ 0,
			/* .MaxFragmentImageUniforms = */ 8,
			/* .MaxCombinedImageUniforms = */ 8,
			/* .MaxGeometryTextureImageUnits = */ 16,
			/* .MaxGeometryOutputVertices = */ 256,
			/* .MaxGeometryTotalOutputComponents = */ 1024,
			/* .MaxGeometryUniformComponents = */ 1024,
			/* .MaxGeometryVaryingComponents = */ 64,
			/* .MaxTessControlInputComponents = */ 128,
			/* .MaxTessControlOutputComponents = */ 128,
			/* .MaxTessControlTextureImageUnits = */ 16,
			/* .MaxTessControlUniformComponents = */ 1024,
			/* .MaxTessControlTotalOutputComponents = */ 4096,
			/* .MaxTessEvaluationInputComponents = */ 128,
			/* .MaxTessEvaluationOutputComponents = */ 128,
			/* .MaxTessEvaluationTextureImageUnits = */ 16,
			/* .MaxTessEvaluationUniformComponents = */ 1024,
			/* .MaxTessPatchComponents = */ 120,
			/* .MaxPatchVertices = */ 32,
			/* .MaxTessGenLevel = */ 64,
			/* .MaxViewports = */ 16,
			/* .MaxVertexAtomicCounters = */ 0,
			/* .MaxTessControlAtomicCounters = */ 0,
			/* .MaxTessEvaluationAtomicCounters = */ 0,
			/* .MaxGeometryAtomicCounters = */ 0,
			/* .MaxFragmentAtomicCounters = */ 8,
			/* .MaxCombinedAtomicCounters = */ 8,
			/* .MaxAtomicCounterBindings = */ 1,
			/* .MaxVertexAtomicCounterBuffers = */ 0,
			/* .MaxTessControlAtomicCounterBuffers = */ 0,
			/* .MaxTessEvaluationAtomicCounterBuffers = */ 0,
			/* .MaxGeometryAtomicCounterBuffers = */ 0,
			/* .MaxFragmentAtomicCounterBuffers = */ 1,
			/* .MaxCombinedAtomicCounterBuffers = */ 1,
			/* .MaxAtomicCounterBufferSize = */ 16384,
			/* .MaxTransformFeedbackBuffers = */ 4,
			/* .MaxTransformFeedbackInterleavedComponents = */ 64,
			/* .MaxCullDistances = */ 8,
			/* .MaxCombinedClipAndCullDistances = */ 8,
			/* .MaxSamples = */ 4,
			/* .maxMeshOutputVerticesNV = */ 256,
			/* .maxMeshOutputPrimitivesNV = */ 512,
			/* .maxMeshWorkGroupSizeX_NV = */ 32,
			/* .maxMeshWorkGroupSizeY_NV = */ 1,
			/* .maxMeshWorkGroupSizeZ_NV = */ 1,
			/* .maxTaskWorkGroupSizeX_NV = */ 32,
			/* .maxTaskWorkGroupSizeY_NV = */ 1,
			/* .maxTaskWorkGroupSizeZ_NV = */ 1,
			/* .maxMeshViewCountNV = */ 4,

			/* .limits = */
			{
				/* .nonInductiveForLoops = */ 1,
				/* .whileLoops = */ 1,
				/* .doWhileLoops = */ 1,
				/* .generalUniformIndexing = */ 1,
				/* .generalAttributeMatrixVectorIndexing = */ 1,
				/* .generalVaryingIndexing = */ 1,
				/* .generalSamplerIndexing = */ 1,
				/* .generalVariableIndexing = */ 1,
				/* .generalConstantMatrixVectorIndexing = */ 1,
			} };

		// Initialise TBuiltInResource to a set of provided defaults
		*tBuiltInResourcePtr = defaultTBuiltInResource;

		// Initialise the Vulkan specific TBuiltInResource members
		tBuiltInResourcePtr->maxClipDistances = static_cast<int>(device->getPhysicalDevice()->getProperties().getLimits().getMaxClipDistances());
		tBuiltInResourcePtr->maxCombinedClipAndCullDistances = static_cast<int>(device->getPhysicalDevice()->getProperties().getLimits().getMaxCombinedClipAndCullDistances());
		tBuiltInResourcePtr->maxCombinedImageUniforms = static_cast<int>(device->getPhysicalDevice()->getProperties().getLimits().getMaxPerStageDescriptorStorageImages());
		tBuiltInResourcePtr->maxCombinedShaderOutputResources = static_cast<int>(device->getPhysicalDevice()->getProperties().getLimits().getMaxFragmentCombinedOutputResources());
		tBuiltInResourcePtr->maxComputeImageUniforms = static_cast<int>(device->getPhysicalDevice()->getProperties().getLimits().getMaxPerStageDescriptorStorageImages());
		tBuiltInResourcePtr->maxComputeWorkGroupCountX = static_cast<int>(device->getPhysicalDevice()->getProperties().getLimits().getMaxComputeWorkGroupCount()[0]);
		tBuiltInResourcePtr->maxComputeWorkGroupCountY = static_cast<int>(device->getPhysicalDevice()->getProperties().getLimits().getMaxComputeWorkGroupCount()[1]);
		tBuiltInResourcePtr->maxComputeWorkGroupCountZ = static_cast<int>(device->getPhysicalDevice()->getProperties().getLimits().getMaxComputeWorkGroupCount()[2]);
		tBuiltInResourcePtr->maxComputeWorkGroupSizeX = static_cast<int>(device->getPhysicalDevice()->getProperties().getLimits().getMaxComputeWorkGroupSize()[0]);
		tBuiltInResourcePtr->maxComputeWorkGroupSizeY = static_cast<int>(device->getPhysicalDevice()->getProperties().getLimits().getMaxComputeWorkGroupSize()[1]);
		tBuiltInResourcePtr->maxComputeWorkGroupSizeZ = static_cast<int>(device->getPhysicalDevice()->getProperties().getLimits().getMaxComputeWorkGroupSize()[2]);
		tBuiltInResourcePtr->maxCullDistances = static_cast<int>(device->getPhysicalDevice()->getProperties().getLimits().getMaxCullDistances());
		tBuiltInResourcePtr->maxFragmentImageUniforms = static_cast<int>(device->getPhysicalDevice()->getProperties().getLimits().getMaxPerStageDescriptorStorageImages());
		tBuiltInResourcePtr->maxFragmentInputComponents = static_cast<int>(device->getPhysicalDevice()->getProperties().getLimits().getMaxFragmentInputComponents());
		tBuiltInResourcePtr->maxFragmentInputVectors = static_cast<int>(device->getPhysicalDevice()->getProperties().getLimits().getMaxFragmentInputComponents() / 4);
		tBuiltInResourcePtr->maxGeometryImageUniforms = static_cast<int>(device->getPhysicalDevice()->getProperties().getLimits().getMaxPerStageDescriptorStorageImages());
		tBuiltInResourcePtr->maxGeometryInputComponents = static_cast<int>(device->getPhysicalDevice()->getProperties().getLimits().getMaxGeometryInputComponents());
		tBuiltInResourcePtr->maxGeometryOutputComponents = static_cast<int>(device->getPhysicalDevice()->getProperties().getLimits().getMaxGeometryOutputComponents());
		tBuiltInResourcePtr->maxGeometryOutputVertices = static_cast<int>(device->getPhysicalDevice()->getProperties().getLimits().getMaxGeometryOutputComponents() / 4);
		tBuiltInResourcePtr->maxGeometryTotalOutputComponents = static_cast<int>(device->getPhysicalDevice()->getProperties().getLimits().getMaxGeometryTotalOutputComponents());
		tBuiltInResourcePtr->maxGeometryVaryingComponents = static_cast<int>(device->getPhysicalDevice()->getProperties().getLimits().getMaxGeometryInputComponents());
		tBuiltInResourcePtr->maxImageSamples =
			static_cast<int>(pvr::utils::getNumSamplesFromSampleCountFlags(device->getPhysicalDevice()->getProperties().getLimits().getSampledImageIntegerSampleCounts()));
		tBuiltInResourcePtr->maxPatchVertices = static_cast<int>(device->getPhysicalDevice()->getProperties().getLimits().getMaxTessellationPatchSize());
		tBuiltInResourcePtr->maxProgramTexelOffset = static_cast<int>(device->getPhysicalDevice()->getProperties().getLimits().getMaxTexelOffset());
		tBuiltInResourcePtr->maxSamples =
			static_cast<int>(std::max(pvr::utils::getNumSamplesFromSampleCountFlags(device->getPhysicalDevice()->getProperties().getLimits().getStorageImageSampleCounts()),
				pvr::utils::getNumSamplesFromSampleCountFlags(device->getPhysicalDevice()->getProperties().getLimits().getSampledImageIntegerSampleCounts())));
		tBuiltInResourcePtr->maxTessControlImageUniforms = static_cast<int>(device->getPhysicalDevice()->getProperties().getLimits().getMaxPerStageDescriptorStorageImages());
		tBuiltInResourcePtr->maxTessControlInputComponents =
			static_cast<int>(device->getPhysicalDevice()->getProperties().getLimits().getMaxTessellationControlPerVertexInputComponents());
		tBuiltInResourcePtr->maxTessControlOutputComponents =
			static_cast<int>(device->getPhysicalDevice()->getProperties().getLimits().getMaxTessellationControlPerVertexOutputComponents());
		tBuiltInResourcePtr->maxTessControlTotalOutputComponents =
			static_cast<int>(device->getPhysicalDevice()->getProperties().getLimits().getMaxTessellationControlTotalOutputComponents());
		tBuiltInResourcePtr->maxTessEvaluationImageUniforms = static_cast<int>(device->getPhysicalDevice()->getProperties().getLimits().getMaxPerStageDescriptorStorageImages());
		tBuiltInResourcePtr->maxTessEvaluationInputComponents =
			static_cast<int>(device->getPhysicalDevice()->getProperties().getLimits().getMaxTessellationEvaluationInputComponents());
		tBuiltInResourcePtr->maxTessEvaluationOutputComponents =
			static_cast<int>(device->getPhysicalDevice()->getProperties().getLimits().getMaxTessellationEvaluationOutputComponents());
		tBuiltInResourcePtr->maxTessGenLevel = static_cast<int>(device->getPhysicalDevice()->getProperties().getLimits().getMaxTessellationGenerationLevel());
		tBuiltInResourcePtr->maxTessPatchComponents = static_cast<int>(device->getPhysicalDevice()->getProperties().getLimits().getMaxTessellationPatchSize());
		tBuiltInResourcePtr->maxVertexAttribs = static_cast<int>(device->getPhysicalDevice()->getProperties().getLimits().getMaxVertexInputAttributes());
		tBuiltInResourcePtr->maxVertexImageUniforms = static_cast<int>(device->getPhysicalDevice()->getProperties().getLimits().getMaxPerStageDescriptorStorageImages());
		tBuiltInResourcePtr->maxVertexOutputComponents = static_cast<int>(device->getPhysicalDevice()->getProperties().getLimits().getMaxVertexOutputComponents());
		tBuiltInResourcePtr->maxVertexOutputVectors = static_cast<int>(device->getPhysicalDevice()->getProperties().getLimits().getMaxVertexOutputComponents() / 4);
		tBuiltInResourcePtr->maxViewports = static_cast<int>(device->getPhysicalDevice()->getProperties().getLimits().getMaxViewports());
		tBuiltInResourcePtr->minProgramTexelOffset = static_cast<int>(device->getPhysicalDevice()->getProperties().getLimits().getMinTexelOffset());

		if (device->getEnabledExtensionTable().extTransformFeedbackEnabled)
		{
			tBuiltInResourcePtr->maxTransformFeedbackBuffers = static_cast<int>(device->getTransformFeedbackProperties().getMaxTransformFeedbackBuffers());
			tBuiltInResourcePtr->maxTransformFeedbackInterleavedComponents = static_cast<int>(device->getTransformFeedbackProperties().getMaxTransformFeedbackBufferDataSize() / 4);
		}
	}
};

// Convert from pvrvk::ShaderStageFlags to EShLanguage
EShLanguage getCompilerLanguageShaderType(pvrvk::ShaderStageFlags shaderStageFlags)
{
	EShLanguage glslShaderType;
	switch (shaderStageFlags)
	{
	case pvrvk::ShaderStageFlags::e_VERTEX_BIT: glslShaderType = EShLanguage::EShLangVertex; break;
	case pvrvk::ShaderStageFlags::e_FRAGMENT_BIT: glslShaderType = EShLanguage::EShLangFragment; break;
	case pvrvk::ShaderStageFlags::e_COMPUTE_BIT: glslShaderType = EShLanguage::EShLangCompute; break;
	case pvrvk::ShaderStageFlags::e_GEOMETRY_BIT: glslShaderType = EShLanguage::EShLangGeometry; break;
	case pvrvk::ShaderStageFlags::e_TESSELLATION_CONTROL_BIT: glslShaderType = EShLanguage::EShLangTessControl; break;
	case pvrvk::ShaderStageFlags::e_TESSELLATION_EVALUATION_BIT: glslShaderType = EShLanguage::EShLangTessEvaluation; break;
	default: throw InvalidOperationError("getGlslShaderType: Unknown shader type requested.");
	}

	return glslShaderType;
}

typedef enum
{
	EShTargetVulkan_1_0 = (1 << 22),
	EShTargetVulkan_1_1 = (1 << 22) | (1 << 12),
	EShTargetOpenGL_450 = 450,
} EShTargetClientVersion;
} // namespace

pvrvk::ShaderModule createShaderModule(pvrvk::Device& device, std::string& shaderSource, pvrvk::ShaderStageFlags shaderStageFlags, pvrvk::ShaderModuleCreateFlags flags,
	const char* const* defines, uint32_t numDefines)
{
	static const GlslangProcessInitialiser glslangInitialiser;
	static const TBuiltInResourceInitialiser glslangResources(device);

	// Determine the EShLanguage shader type
	const EShLanguage glslangShaderStage = getCompilerLanguageShaderType(shaderStageFlags);
	std::unique_ptr<glslang::TProgram> glslangProgramPtr = std::make_unique<glslang::TProgram>();
	std::unique_ptr<glslang::TShader> glslangShaderPtr = std::make_unique<glslang::TShader>(glslangShaderStage);

	// Determine whether a version string is present
	std::string::size_type versionBegin = shaderSource.find("#version");
	std::string::size_type versionEnd = 0;
	std::string sourceDataStr;
	// If a version string is present then update the versionBegin variable to the position after the version string
	if (versionBegin != std::string::npos)
	{
		versionEnd = shaderSource.find("\n", versionBegin);
		sourceDataStr.append(shaderSource.begin() + versionBegin, shaderSource.begin() + versionBegin + versionEnd);
		sourceDataStr.append("\n");
	}
	else
	{
		versionBegin = 0;
	}
	// Insert the defines after the version string if one is present
	for (uint32_t i = 0; i < numDefines; ++i)
	{
		sourceDataStr.append("#define ");
		sourceDataStr.append(defines[i]);
		sourceDataStr.append("\n");
	}
	sourceDataStr.append("\n");
	sourceDataStr.append(shaderSource.begin() + versionBegin + versionEnd, shaderSource.end());

	const char* const shaderSourceChar = sourceDataStr.c_str();

	glslangShaderPtr->setStrings(&shaderSourceChar, 1);

	// ShaderLang.cpp specifies the following "use 100 for ES environment, 110 for desktop". Our main target is to use the ES environment so use 100
	const int defaultVersion = 100;
	// Enable various messages determining what errors and warnings are given
	EShMessages messages = static_cast<EShMessages>(EShMessages::EShMsgDefault | EShMessages::EShMsgSpvRules | EShMessages::EShMsgVulkanRules);

	uint32_t apiVersion = device->getPhysicalDevice()->getInstance()->getCreateInfo().getApplicationInfo().getApiVersion();
	uint32_t minor = VK_VERSION_MINOR(apiVersion);
	glslang::EShTargetClientVersion vulkanClientVersion = glslang::EShTargetVulkan_1_0;
	glslang::EShTargetLanguageVersion targetSpirvVersion = glslang::EShTargetSpv_1_0;
	if (minor >= 1)
	{
		vulkanClientVersion = glslang::EShTargetVulkan_1_1;
		// Vulkan 1.1 implementations must support SPIR-V 1.3
		targetSpirvVersion = glslang::EShTargetSpv_1_3;
	}

	glslangShaderPtr->setEnvClient(glslang::EShClientVulkan, vulkanClientVersion);
	glslangShaderPtr->setEnvTarget(glslang::EShTargetSpv, targetSpirvVersion);

	// Parse the shader
	bool parseResult = glslangShaderPtr->parse(glslangResources.tBuiltInResourcePtr.get(), defaultVersion, false, messages);

	if (!parseResult)
	{
		std::string debugLog = glslangShaderPtr->getInfoDebugLog();
		std::string infoLog = glslangShaderPtr->getInfoLog();

		throw pvrvk::ErrorValidationFailedEXT(
			pvr::strings::createFormatted("pvr::utils::createShaderModule shader compilation failed. Error log is: %s\nDebug Log is: %s", infoLog.c_str(), debugLog.c_str()).c_str());
	}

	// Add the shader to the program
	glslangProgramPtr->addShader(glslangShaderPtr.get());

	// Link the program
	bool linkResult = glslangProgramPtr->link(messages);

	if (!linkResult)
	{
		std::string debugLog = glslangProgramPtr->getInfoDebugLog();
		std::string infoLog = glslangProgramPtr->getInfoLog();

		throw pvrvk::ErrorValidationFailedEXT(
			pvr::strings::createFormatted("pvr::utils::createShaderModule Program linking failed. Error log is: %s\nDebug Log is: %s", infoLog.c_str(), debugLog.c_str()).c_str());
	}

	// Retrieve the intermediate representation of the glslang program
	glslang::TIntermediate* intermediatePtr = glslangProgramPtr->getIntermediate(glslangShaderStage);

	if (intermediatePtr == nullptr) { throw pvrvk::ErrorUnknown("pvr::utils::createShaderModule Unable to retrieve intermediate representation of the glslang program"); }

	// Convert the intermediate representation to a SPIR-V blob
	std::vector<unsigned int> spirvBlob;
	spv::SpvBuildLogger logger;
	glslang::SpvOptions spvOptions;
	glslang::GlslangToSpv(*intermediatePtr, spirvBlob, &logger, &spvOptions);

	if (!spirvBlob.size())
	{ throw pvrvk::ErrorUnknown("pvr::utils::createShaderModule Unable to retrieve spirv blob from the intermediate representation of the glslang program"); }

	if (logger.getAllMessages().length() > 0)
	{ throw pvrvk::ErrorUnknown(pvr::strings::createFormatted("pvr::utils::createShaderModule GlslangToSpv failed. Error log is: %s", logger.getAllMessages().c_str()).c_str()); }

	// Create the shader module using the spirv blob
	pvrvk::ShaderModuleCreateInfo createInfo;
	createInfo.setFlags(flags);
	createInfo.setShaderSources(spirvBlob);

	return device->createShaderModule(createInfo);
}

pvrvk::ShaderModule createShaderModule(pvrvk::Device& device, const Stream& shaderStream, pvrvk::ShaderStageFlags shaderStageFlags, pvrvk::ShaderModuleCreateFlags flags,
	const char* const* defines, uint32_t numDefines)
{
	std::string shaderSource;
	shaderStream.readIntoString(shaderSource);

	return createShaderModule(device, shaderSource, shaderStageFlags, flags, defines, numDefines);
}
} // namespace utils
} // namespace pvr
//!\endcond
