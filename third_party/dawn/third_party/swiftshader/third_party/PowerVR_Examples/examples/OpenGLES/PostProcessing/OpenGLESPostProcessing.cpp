/*!
\brief Shows how to do a post processing effects
\file OpenGLESPostProcessing.cpp
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
#include "PVRShell/PVRShell.h"
#include "PVRUtils/PVRUtilsGles.h"
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

enum class OffscreenAttachments
{
	Offscreen = 0,
	Luminance = 1,
	DepthStencil = 2,
	NumAttachments
};

enum class OffscreenWithIMGFramebufferDownsampleAttachments
{
	Offscreen = 0,
	DownsampledLuminance = 1,
	DepthStencil = 2,
	NumAttachments
};

enum class BloomAttachments
{
	Bloom
};

// Titles for the various bloom modes
const std::string BloomStrings[] = { "Original Image (No Post Processing)", "Gaussian (Reference Implementation)", "Gaussian (Linear Sampling)",
	"Gaussian (Compute Sliding Average)", "Hybrid Gaussian", "Truncated Gaussian (Linear Sampling)", "Kawase", "Dual Filter", "Tent Filter" };

// Files used throughout the demo
namespace Files {
// Shader file names
const char Downsample2x2VertSrcFile[] = "Downsample2x2VertShader.vsh";
const char Downsample2x2FragSrcFile[] = "Downsample2x2FragShader.fsh";
const char Downsample4x4VertSrcFile[] = "Downsample4x4VertShader.vsh";
const char Downsample4x4FragSrcFile[] = "Downsample4x4FragShader.fsh";

// Dual Filter shaders
const char DualFilterDownSampleFragSrcFile[] = "DualFilterDownSampleFragShader.fsh";
const char DualFilterUpSampleFragSrcFile[] = "DualFilterUpSampleFragShader.fsh";
const char DualFilterUpSampleMergedFinalPassFragSrcFile[] = "DualFilterUpSampleMergedFinalPassFragShader.fsh";
const char DualFilterDownVertSrcFile[] = "DualFilterDownVertShader.vsh";
const char DualFilterUpVertSrcFile[] = "DualFilterUpVertShader.vsh";

// Tent Filter shaders
const char TentFilterUpSampleVertSrcFile[] = "TentFilterUpSampleVertShader.vsh";
const char TentFilterFirstUpSampleFragSrcFile[] = "TentFilterFirstUpSampleFragShader.fsh";
const char TentFilterUpSampleFragSrcFile[] = "TentFilterUpSampleFragShader.fsh";
const char TentFilterUpSampleMergedFinalPassFragSrcFile[] = "TentFilterUpSampleMergedFinalPassFragShader.fsh";

// Kawase Blur shaders
const char KawaseVertSrcFile[] = "KawaseVertShader.vsh";
const char KawaseFragSrcFile[] = "KawaseFragShader.fsh";

// Traditional Gaussian Blur shaders
const char GaussianFragSrcFile[] = "GaussianBlurFragmentShader.fsh.in";
const char GaussianVertSrcFile[] = "GaussianVertShader.vsh";

// Linear Sampler Optimised Gaussian Blur shaders
const char LinearGaussianVertSrcFile[] = "LinearGaussianBlurVertexShader.vsh.in";
const char LinearGaussianFragSrcFile[] = "LinearGaussianBlurFragmentShader.fsh.in";

// Compute based sliding average Gaussian Blur shaders
const char GaussianComputeBlurHorizontalSrcFile[] = "ComputeGaussianBlurHorizontalShader.csh.in";
const char GaussianComputeBlurVerticalSrcFile[] = "ComputeGaussianBlurVerticalShader.csh.in";

// Post Bloom Shaders
const char PostBloomVertShaderSrcFile[] = "PostBloomVertShader.vsh";
const char PostBloomFragShaderSrcFile[] = "PostBloomFragShader.fsh";

// Scene Rendering shaders
const char FragShaderSrcFile[] = "FragShader.fsh";
const char VertShaderSrcFile[] = "VertShader.vsh";
const char SkyboxFragShaderSrcFile[] = "SkyboxFragShader.fsh";
const char SkyboxVertShaderSrcFile[] = "SkyboxVertShader.vsh";
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

const pvr::utils::VertexBindings_Name VertexBindings[] = { { "POSITION", "inVertex" }, { "NORMAL", "inNormal" }, { "UV0", "inTexCoord" }, { "TANGENT", "inTangent" } };

enum class AttributeIndices
{
	VertexArray = 0,
	NormalArray = 1,
	TexCoordArray = 2,
	TangentArray = 3
};

// Provides a simple wrapper around a framebuffer and its given attachments
struct Framebuffer
{
	GLuint framebuffer;
	std::vector<GLuint> attachments;
	glm::uvec2 dimensions;

	Framebuffer() : framebuffer(-1), dimensions(1) {}
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

/// <summary>This class is added as the Debug Callback. Redirects the debug output to the Log object.</summary>
inline void GL_APIENTRY debugCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* userParam)
{
	(void)severity;
	(void)userParam;
	(void)length;
	Log(LogLevel::Debug, "[%d|%d|%d] %s", (int)source, (int)type, (int)id, message);
}

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

		weightsString = pvr::strings::createFormatted("const mediump float gWeights[numIterations * 2u] = float[numIterations * 2u](%s);", weights.c_str());
	}
	else
	{
		weightsString = pvr::strings::createFormatted("const mediump float gWeights[numIterations] = float[numIterations](%s);", weights.c_str());
		offsetsString = pvr::strings::createFormatted("const mediump float gOffsets[numIterations] = float[numIterations](%s);", offsets.c_str());
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

// A simple pass used for rendering our statue object
struct StatuePass
{
	GLuint program;
	GLuint albedoTexture;
	GLuint normalMapTexture;
	GLuint vao;
	std::vector<GLuint> vbos;
	std::vector<GLuint> ibos;
	pvr::utils::VertexConfiguration vertexConfigurations;
	pvr::utils::VertexConfiguration vertexConfiguration;
	pvr::utils::StructuredBufferView structuredBufferView;
	GLuint buffer;
	void* mappedMemory;
	bool isBufferStorageExtSupported;

	GLint exposureUniformLocation;
	GLint thresholdUniformLocation;

	// 3D Model
	pvr::assets::ModelHandle scene;

	/// <summary>Initialises the Statue pass.</summary>
	/// <param name="assetProvider">The pvr::IAssetProvider which will be used for loading resources from memory.</param>
	/// <param name="isBufferStorageExtSupported">True if GL_EXT_buffer_storage is supported.</param>
	void init(pvr::IAssetProvider& assetProvider, bool isBufferStorageExtSupported_)
	{
		this->isBufferStorageExtSupported = isBufferStorageExtSupported_;

		// Load the scene
		scene = pvr::assets::loadModel(assetProvider, SceneFile);
		pvr::utils::appendSingleBuffersFromModel(*scene, vbos, ibos);

		bindVertexSpecification(scene->getMesh(0), VertexBindings, 4);

		// Create and Allocate Textures.
		albedoTexture = pvr::utils::textureUpload(assetProvider, StatueTexFile);
		normalMapTexture = pvr::utils::textureUpload(assetProvider, StatueNormalMapTexFile);
		createProgram(assetProvider);
		createBuffer();

		debugThrowOnApiError("StatuePass init");
	}

	/// <summary>Binds a vertex specification and creates a VertexArray for it.</summary>
	/// <param name="mesh">The pvr::assets::Mesh from which to bind its vertex specification.</param>
	/// <param name="vertexBindingsName">The vertex binding names.</param>
	/// <param name="numVertexBindings">The number of vertex binding names pointed to by vertexBindingsName.</param>
	/// <param name="vao">A created and returned Vertex Array Object.</param>
	/// <param name="vbo">A pre-created vbo.</param>
	/// <param name="ibo">A pre-created ibo.</param>
	void bindVertexSpecification(const pvr::assets::Mesh& mesh, const pvr::utils::VertexBindings_Name* const vertexBindingsName, const uint32_t numVertexBindings)
	{
		vertexConfiguration = pvr::utils::createInputAssemblyFromMesh(mesh, vertexBindingsName, (uint16_t)numVertexBindings);

		gl::GenVertexArrays(1, &vao);
		gl::BindVertexArray(vao);
		gl::BindVertexBuffer(0, vbos[0], 0, mesh.getStride(0));
		gl::BindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibos[0]);

		for (auto it = vertexConfiguration.attributes.begin(), end = vertexConfiguration.attributes.end(); it != end; ++it)
		{
			gl::EnableVertexAttribArray(it->index);
			gl::VertexAttribBinding(it->index, 0);
			gl::VertexAttribFormat(it->index, it->width, pvr::utils::convertToGles(it->format), pvr::dataTypeIsNormalised(it->format), static_cast<intptr_t>(it->offsetInBytes));
		}

		gl::BindVertexArray(0);
	}

	/// <summary>Creates any required buffers.</summary>
	void createBuffer()
	{
		pvr::utils::StructuredMemoryDescription desc;
		desc.addElement(BufferEntryNames::PerMesh::MVPMatrix, pvr::GpuDatatypes::mat4x4);
		desc.addElement(BufferEntryNames::PerMesh::WorldMatrix, pvr::GpuDatatypes::mat4x4);

		structuredBufferView.initDynamic(desc);

		gl::GenBuffers(1, &buffer);
		gl::BindBuffer(GL_UNIFORM_BUFFER, buffer);
		gl::BufferData(GL_UNIFORM_BUFFER, (size_t)structuredBufferView.getSize(), nullptr, GL_DYNAMIC_DRAW);

		// if GL_EXT_buffer_storage is supported then map the buffer upfront and never unmap it
		if (isBufferStorageExtSupported)
		{
			gl::BindBuffer(GL_COPY_READ_BUFFER, buffer);
			gl::ext::BufferStorageEXT(GL_COPY_READ_BUFFER, (GLsizei)structuredBufferView.getSize(), 0, GL_MAP_WRITE_BIT_EXT | GL_MAP_PERSISTENT_BIT_EXT | GL_MAP_COHERENT_BIT_EXT);

			mappedMemory = gl::MapBufferRange(
				GL_COPY_READ_BUFFER, 0, static_cast<GLsizeiptr>(structuredBufferView.getSize()), GL_MAP_WRITE_BIT_EXT | GL_MAP_PERSISTENT_BIT_EXT | GL_MAP_COHERENT_BIT_EXT);
			structuredBufferView.pointToMappedMemory(mappedMemory);
		}
	}

	/// <summary>Create the rendering program used for rendering the statue.</summary>
	/// <param name="assetProvider">The pvr::IAssetProvider which will be used for loading resources from memory.</param>
	void createProgram(pvr::IAssetProvider& assetProvider)
	{
		const char* attributeNames[] = { VertexBindings[0].variableName.c_str(), VertexBindings[1].variableName.c_str(), VertexBindings[2].variableName.c_str(),
			VertexBindings[3].variableName.c_str() };
		const uint16_t attributeIndices[] = { static_cast<uint16_t>(AttributeIndices::VertexArray), static_cast<uint16_t>(AttributeIndices::NormalArray),
			static_cast<uint16_t>(AttributeIndices::TexCoordArray), static_cast<uint16_t>(AttributeIndices::TangentArray) };
		const uint32_t numAttributes = 4;

		program = pvr::utils::createShaderProgram(assetProvider, Files::VertShaderSrcFile, Files::FragShaderSrcFile, attributeNames, attributeIndices, numAttributes);
		gl::UseProgram(program);
		gl::Uniform1i(gl::GetUniformLocation(program, "sBaseTex"), 0);
		gl::Uniform1i(gl::GetUniformLocation(program, "sNormalMap"), 1);
		gl::Uniform1i(gl::GetUniformLocation(program, "irradianceMap"), 2);

		exposureUniformLocation = gl::GetUniformLocation(program, "linearExposure");
		thresholdUniformLocation = gl::GetUniformLocation(program, "threshold");
	}

	/// <summary>Update the object animation.</summary>
	/// <param name="angle">The angle to use for rotating the statue.</param>
	/// <param name="viewProjectionMatrix">The view projection matrix to use for rendering.</param>
	void updateAnimation(const float angle, glm::mat4& viewProjectionMatrix)
	{
		// Calculate the model matrix
		const glm::mat4 mModel = glm::translate(glm::vec3(0.0f, 5.0f, 0.0f)) * glm::rotate(angle, glm::vec3(0.0f, 1.0f, 0.0f)) * glm::scale(glm::vec3(2.2f));

		glm::mat4 worldMatrix = mModel * scene->getWorldMatrix(scene->getNode(0).getObjectId());
		glm::mat4 mvpMatrix = viewProjectionMatrix * worldMatrix;

		if (!isBufferStorageExtSupported)
		{
			gl::BindBuffer(GL_UNIFORM_BUFFER, buffer);
			mappedMemory = gl::MapBufferRange(GL_UNIFORM_BUFFER, 0, (size_t)structuredBufferView.getSize(), GL_MAP_WRITE_BIT);
			structuredBufferView.pointToMappedMemory(mappedMemory);
		}

		structuredBufferView.getElementByName(BufferEntryNames::PerMesh::MVPMatrix).setValue(mvpMatrix);
		structuredBufferView.getElementByName(BufferEntryNames::PerMesh::WorldMatrix).setValue(worldMatrix);

		if (!isBufferStorageExtSupported) { gl::UnmapBuffer(GL_UNIFORM_BUFFER); }
	}

	/// <summary>Draws an assets::Mesh after the model view matrix has been set and the material prepared.</summary>
	/// <param name="nodeIndex">Node index of the mesh to draw</param>
	void renderMesh(uint32_t nodeIndex)
	{
		const uint32_t meshId = scene->getNode(nodeIndex).getObjectId();
		const pvr::assets::Mesh& mesh = scene->getMesh(meshId);

		gl::BindVertexArray(vao);
		GLenum primitiveType = pvr::utils::convertToGles(mesh.getPrimitiveType());
		if (mesh.getMeshInfo().isIndexed)
		{
			auto indextype = mesh.getFaces().getDataType();
			GLenum indexgltype = (indextype == pvr::IndexType::IndexType16Bit ? GL_UNSIGNED_SHORT : GL_UNSIGNED_INT);
			// Indexed Triangle list
			gl::DrawElements(primitiveType, mesh.getNumFaces() * 3, indexgltype, 0);
		}
		else
		{
			// Non-Indexed Triangle list
			gl::DrawArrays(primitiveType, 0, mesh.getNumFaces() * 3);
		}

		gl::BindVertexArray(0);
	}

	/// <summary>Renders the statue.</summary>
	/// <param name="irradianceMap">The irradiance map</param>
	/// <param name="samplerTrilinear">The trilinear sampler to use</param>
	/// <param name="irradianceSampler">A sampler to use for sampling the irradiance map</param>
	/// <param name="exposure">The exposure value used to 'expose' the colour prior to post processing</param>
	/// <param name="threshold">The threshold value used to determine how much of the colour to retain for the bloom</param>
	void render(GLuint irradianceMap, GLuint samplerTrilinear, GLuint irradianceSampler, float exposure, float threshold)
	{
		debugThrowOnApiError("StatuePass before render");
		gl::BindBufferRange(GL_UNIFORM_BUFFER, 0, buffer, 0, static_cast<GLsizeiptr>(structuredBufferView.getSize()));

		gl::ActiveTexture(GL_TEXTURE0);
		gl::BindSampler(0, samplerTrilinear);
		gl::BindTexture(GL_TEXTURE_2D, albedoTexture);

		gl::ActiveTexture(GL_TEXTURE1);
		gl::BindSampler(1, samplerTrilinear);
		gl::BindTexture(GL_TEXTURE_2D, normalMapTexture);

		gl::ActiveTexture(GL_TEXTURE2);
		gl::BindSampler(2, irradianceSampler);
		gl::BindTexture(GL_TEXTURE_CUBE_MAP, irradianceMap);

		gl::UseProgram(program);
		gl::Uniform1f(exposureUniformLocation, exposure);
		gl::Uniform1f(thresholdUniformLocation, threshold);
		renderMesh(0);
		debugThrowOnApiError("StatuePass after render");
	}
};

// A simple pass used for rendering our skybox
struct SkyboxPass
{
	GLuint program;
	GLuint skyBoxTextures[NumScenes];
	GLint exposureUniformLocation;
	GLint thresholdUniformLocation;

	/// <summary>Initialises the skybox pass.</summary>
	/// <param name="assetProvider">The pvr::IAssetProvider which will be used for loading resources from memory.</param>
	void init(pvr::IAssetProvider& assetProvider)
	{
		loadSkyBoxTextures(assetProvider);
		createProgram(assetProvider);
	}

	/// <summary>Creates the textures used for rendering the skybox.</summary>
	/// <param name="assetProvider">The pvr::IAssetProvider which will be used for loading resources from memory.</param>
	void loadSkyBoxTextures(pvr::IAssetProvider& assetProvider)
	{
		for (uint32_t i = 0; i < NumScenes; ++i)
		{
			// Load the Texture PVR file from the disk
			skyBoxTextures[i] = pvr::utils::textureUpload(assetProvider, SceneTexFileNames[i].skyboxTexture);
		}
	}

	/// <summary>Create the rendering program used for rendering the skybox.</summary>
	/// <param name="assetProvider">The pvr::IAssetProvider which will be used for loading resources from memory.</param>
	void createProgram(pvr::IAssetProvider& assetProvider)
	{
		program = pvr::utils::createShaderProgram(assetProvider, Files::SkyboxVertShaderSrcFile, Files::SkyboxFragShaderSrcFile, nullptr, nullptr, 0);
		gl::UseProgram(program);
		gl::Uniform1i(gl::GetUniformLocation(program, "skybox"), 0);
		exposureUniformLocation = gl::GetUniformLocation(program, "linearExposure");
		thresholdUniformLocation = gl::GetUniformLocation(program, "threshold");
	}

	/// <summary>Renders the skybox.</summary>
	/// <param name="sceneBuffer">The scene buffer</param>
	/// <param name="sceneBufferSize">The size of the scene buffer</param>
	/// <param name="samplerTrilinear">The trilinear sampler to use</param>
	/// <param name="exposure">The exposure value used to 'expose' the colour prior to post processing</param>
	/// <param name="threshold">The threshold value used to determine how much of the colour to retain for the bloom</param>
	/// <param name="currentScene">The current scene to use.</param>
	void render(GLuint sceneBuffer, GLsizeiptr sceneBufferSize, GLuint samplerTrilinear, float exposure, float threshold, uint32_t currentScene)
	{
		debugThrowOnApiError("Skybox Pass before render");
		gl::BindBufferRange(GL_UNIFORM_BUFFER, 0, sceneBuffer, 0, sceneBufferSize);

		gl::ActiveTexture(GL_TEXTURE0);
		gl::BindTexture(GL_TEXTURE_CUBE_MAP, skyBoxTextures[currentScene]);
		gl::BindSampler(0, samplerTrilinear);

		gl::UseProgram(program);
		gl::Uniform1f(exposureUniformLocation, exposure);
		gl::Uniform1f(thresholdUniformLocation, threshold);
		gl::DrawArrays(GL_TRIANGLES, 0, 6);
		debugThrowOnApiError("Skybox Pass after render");
	}
};

// A Downsample pass which can be used for downsampling images by 1/2 x 1/2 i.e. 1/4 resolution
struct DownSamplePass2x2
{
	GLuint program;
	GLuint framebuffer;
	glm::uvec2 downsampleDimensions;

	/// <summary>Initialises the Downsample pass.</summary>
	/// <param name="assetProvider">The pvr::IAssetProvider which will be used for loading resources from memory.</param>
	/// <param name="outputTexture">The colour texture use as the output of the downsample.</param>
	/// <param name="destinationImageDimensions">The dimensions of the destination image which contains the downsampled image.</param>
	virtual void init(pvr::IAssetProvider& assetProvider, GLuint outputTexture, const glm::uvec2& destinationImageDimensions)
	{
		createProgram(assetProvider);
		this->downsampleDimensions = destinationImageDimensions;
		createFramebuffer(outputTexture);
		debugThrowOnApiError("DownSamplePass2x2 init");
	}

	virtual void createFramebuffer(GLuint outputTexture)
	{
		gl::GenFramebuffers(1, &framebuffer);
		gl::BindFramebuffer(GL_DRAW_FRAMEBUFFER, framebuffer);
		gl::FramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, outputTexture, 0);
		gl::FramebufferParameteri(GL_DRAW_FRAMEBUFFER, GL_FRAMEBUFFER_DEFAULT_WIDTH, downsampleDimensions.x);
		gl::FramebufferParameteri(GL_DRAW_FRAMEBUFFER, GL_FRAMEBUFFER_DEFAULT_HEIGHT, downsampleDimensions.y);
		pvr::utils::checkFboStatus();
	}

	/// <summary>Create the rendering program used for downsampling the luminance texture.</summary>
	/// <param name="assetProvider">The pvr::IAssetProvider which will be used for loading resources from memory.</param>
	virtual void createProgram(pvr::IAssetProvider& assetProvider)
	{
		program = pvr::utils::createShaderProgram(assetProvider, Files::Downsample2x2VertSrcFile, Files::Downsample2x2FragSrcFile, nullptr, nullptr, 0);
		gl::UseProgram(program);
		gl::Uniform1i(gl::GetUniformLocation(program, "sTexture"), 0);
	}

	/// <summary>Performs a downsample of the provided texture.</summary>
	/// <param name="sourceTexture">The source texture to downsample</param>
	/// <param name="samplerBilinear">The bilinear sampler to use</param>
	virtual void render(GLuint sourceTexture, GLuint samplerBilinear)
	{
		debugThrowOnApiError("Downsample Pass before render");

		gl::Viewport(0, 0, downsampleDimensions.x, downsampleDimensions.y);
		gl::BindFramebuffer(GL_DRAW_FRAMEBUFFER, framebuffer);
		gl::Clear(GL_COLOR_BUFFER_BIT);

		gl::ActiveTexture(GL_TEXTURE0);
		gl::BindTexture(GL_TEXTURE_2D, sourceTexture);
		gl::BindSampler(0, samplerBilinear);

		gl::UseProgram(program);
		gl::DrawArrays(GL_TRIANGLE_STRIP, 0, 3);
		debugThrowOnApiError("Downsample Pass after render");
	}
};

/// <summary>A Downsample pass which can be used for downsampling images by 1/4 x 1/4 i.e. 1/4 resolution</summary>
struct DownSamplePass4x4 : public DownSamplePass2x2
{
	GLint downsampleConfigUniformLocations[4];
	glm::vec2 downsampleConfigs[4];

	/// <summary>Initialises the Downsample pass.</summary>
	/// <param name="assetProvider">The pvr::IAssetProvider which will be used for loading resources from memory.</param>
	/// <param name="outputTexture">The texture to use as the output of the downsample.</param>
	/// <param name="destinationImageDimensions">The dimensions of the destination image which contains the downsampled image.</param>
	/// <param name="sourceImageDimensions">The dimensions of the source image to be downsampled.</param>
	void init(pvr::IAssetProvider& assetProvider, GLuint outputTexture, const glm::uvec2& destinationImageDimensions, const glm::uvec2& sourceImageDimensions)
	{
		const glm::vec2 dimensionRatio = glm::vec2(sourceImageDimensions.x / destinationImageDimensions.x, sourceImageDimensions.y / destinationImageDimensions.y);

		// A set of pre-calculated offsets to use for the downsample
		const glm::vec2 offsets[4] = { glm::vec2(-1.0, -1.0), glm::vec2(1.0, -1.0), glm::vec2(-1.0, 1.0), glm::vec2(1.0, 1.0) };

		downsampleConfigs[0] = glm::vec2(1.0f / (destinationImageDimensions.x * dimensionRatio.x), 1.0f / (destinationImageDimensions.y * dimensionRatio.y)) * offsets[0];
		downsampleConfigs[1] = glm::vec2(1.0f / (destinationImageDimensions.x * dimensionRatio.x), 1.0f / (destinationImageDimensions.y * dimensionRatio.y)) * offsets[1];
		downsampleConfigs[2] = glm::vec2(1.0f / (destinationImageDimensions.x * dimensionRatio.x), 1.0f / (destinationImageDimensions.y * dimensionRatio.y)) * offsets[2];
		downsampleConfigs[3] = glm::vec2(1.0f / (destinationImageDimensions.x * dimensionRatio.x), 1.0f / (destinationImageDimensions.y * dimensionRatio.y)) * offsets[3];

		DownSamplePass2x2::init(assetProvider, outputTexture, destinationImageDimensions);

		debugThrowOnApiError("DownSamplePass init");
	}

	/// <summary>Create the rendering program used for downsampling the luminance texture.</summary>
	/// <param name="assetProvider">The pvr::IAssetProvider which will be used for loading resources from memory.</param>
	void createProgram(pvr::IAssetProvider& assetProvider) override
	{
		program = pvr::utils::createShaderProgram(assetProvider, Files::Downsample4x4VertSrcFile, Files::Downsample4x4FragSrcFile, nullptr, nullptr, 0);
		gl::UseProgram(program);
		downsampleConfigUniformLocations[0] = gl::GetUniformLocation(program, "downsampleConfigs[0]");
		downsampleConfigUniformLocations[1] = gl::GetUniformLocation(program, "downsampleConfigs[1]");
		downsampleConfigUniformLocations[2] = gl::GetUniformLocation(program, "downsampleConfigs[2]");
		downsampleConfigUniformLocations[3] = gl::GetUniformLocation(program, "downsampleConfigs[3]");

		gl::Uniform2fv(downsampleConfigUniformLocations[0], 1, glm::value_ptr(downsampleConfigs[0]));
		gl::Uniform2fv(downsampleConfigUniformLocations[1], 1, glm::value_ptr(downsampleConfigs[1]));
		gl::Uniform2fv(downsampleConfigUniformLocations[2], 1, glm::value_ptr(downsampleConfigs[2]));
		gl::Uniform2fv(downsampleConfigUniformLocations[3], 1, glm::value_ptr(downsampleConfigs[3]));
		gl::Uniform1i(gl::GetUniformLocation(program, "sTexture"), 0);
	}
};

// Developed by Masaki Kawase, Bunkasha Games
// Used in DOUBLE-S.T.E.A.L. (aka Wreckless)
// From his GDC2003 Presentation: Frame Buffer Post processing Effects in  DOUBLE-S.T.E.A.L (Wreckless)
// Multiple iterations of fixed (per iteration) offset sampling
struct KawaseBlurPass
{
	GLuint program;

	// Per iteration fixed size offset
	std::vector<uint32_t> blurKernels;

	// The number of Kawase blur iterations
	uint32_t blurIterations;

	// Uniforms used for the per iteration Kawase blur configuration
	glm::vec2 configUniforms[MaxKawaseIteration][4];

	uint32_t blurredImageIndex;

	GLint blurConfigLocations[4];

	glm::uvec2 framebufferDimensions;

	/// <summary>Initialises the Kawase blur pass.</summary>
	/// <param name="assetProvider">The pvr::IAssetProvider which will be used for loading resources from memory.</param>
	/// <param name="framebufferDimensions">The dimensions to use for the Kawase blur iterations.</param>
	void init(pvr::IAssetProvider& assetProvider, const glm::uvec2& framebufferDimensions_)
	{
		createProgram(assetProvider);
		blurredImageIndex = static_cast<uint32_t>(-1);
		this->framebufferDimensions = framebufferDimensions_;

		debugThrowOnApiError("KawaseBlurPass init");
	}

	/// <summary>Returns the blurred image for the given configuration.</summary>
	/// <returns>The blurred image for the current configuration.</returns>
	uint32_t getBlurredImageIndex() { return blurredImageIndex; }

	/// <summary>Update the Kawase blur configuration.</summary>
	/// <param name="iterationsOffsets">The offsets to use in the Kawase blur passes.</param>
	/// <param name="numIterations">The number of iterations of Kawase blur passes.</param>
	void updateConfig(uint32_t* iterationsOffsets, uint32_t numIterations)
	{
		// reset/clear the kernels and number of iterations
		blurKernels.clear();
		blurIterations = 0;

		// calculate texture sample offsets based on the number of iterations and the kernel offset currently in use for the given iteration
		glm::vec2 pixelSize = glm::vec2(1.0f / framebufferDimensions.x, 1.0f / framebufferDimensions.y);

		glm::vec2 halfPixelSize = pixelSize / 2.0f;

		for (uint32_t i = 0; i < numIterations; ++i)
		{
			blurKernels.push_back(iterationsOffsets[i]);

			glm::vec2 dUV = pixelSize * glm::vec2(blurKernels[i], blurKernels[i]) + halfPixelSize;

			configUniforms[i][0] = glm::vec2(-dUV.x, dUV.y);
			configUniforms[i][1] = glm::vec2(dUV);
			configUniforms[i][2] = glm::vec2(dUV.x, -dUV.y);
			configUniforms[i][3] = glm::vec2(-dUV.x, -dUV.y);
		}
		blurIterations = numIterations;
		assertion(blurIterations <= MaxKawaseIteration);

		blurredImageIndex = !(numIterations % 2);
	}

	/// <summary>Creates the Kawase program.</summary>
	/// <param name="assetProvider">The pvr::IAssetProvider which will be used for loading resources from memory.</param>
	void createProgram(pvr::IAssetProvider& assetProvider)
	{
		program = pvr::utils::createShaderProgram(assetProvider, Files::KawaseVertSrcFile, Files::KawaseFragSrcFile, nullptr, nullptr, 0);
		gl::UseProgram(program);
		gl::Uniform1i(gl::GetUniformLocation(program, "sTexture"), 0);

		blurConfigLocations[0] = gl::GetUniformLocation(program, "blurConfigs[0]");
		blurConfigLocations[1] = gl::GetUniformLocation(program, "blurConfigs[1]");
		blurConfigLocations[2] = gl::GetUniformLocation(program, "blurConfigs[2]");
		blurConfigLocations[3] = gl::GetUniformLocation(program, "blurConfigs[3]");
	}

	/// <summary>Performs a Kawase blur using the current configuration.</summary>
	/// <param name="sourceTexture">The source texture to downsample</param>
	/// <param name="framebuffers">The framebuffers to use in the Kawase blur.</param>
	/// <param name="numFramebuffers">The number of framebuffers to use.</param>
	/// <param name="samplerBilinear">The sampler object to use when sampling from the ping-ponged images during the Kawase blur passes.</param>
	void render(GLuint sourceTexture, Framebuffer* framebuffers, uint32_t numFramebuffers, GLuint samplerBilinear)
	{
		// Iterate through the Kawase blur iterations
		for (uint32_t i = 0; i < blurIterations; ++i)
		{
			debugThrowOnApiError("Kawase Pass before render");
			// calculate the ping pong index based on the current iteration
			uint32_t pingPongIndex = i % numFramebuffers;

			gl::BindFramebuffer(GL_DRAW_FRAMEBUFFER, framebuffers[pingPongIndex].framebuffer);
			gl::Clear(GL_COLOR_BUFFER_BIT);

			GLuint currentTexture = -1;
			if (i == 0) { currentTexture = sourceTexture; }
			else
			{
				currentTexture = framebuffers[(i - 1) % numFramebuffers].attachments[static_cast<uint32_t>(BloomAttachments::Bloom)];
			}

			gl::ActiveTexture(GL_TEXTURE0);
			gl::BindTexture(GL_TEXTURE_2D, currentTexture);
			gl::BindSampler(0, samplerBilinear);

			gl::UseProgram(program);
			gl::Uniform2fv(blurConfigLocations[0], 1, glm::value_ptr(configUniforms[i][0]));
			gl::Uniform2fv(blurConfigLocations[1], 1, glm::value_ptr(configUniforms[i][1]));
			gl::Uniform2fv(blurConfigLocations[2], 1, glm::value_ptr(configUniforms[i][2]));
			gl::Uniform2fv(blurConfigLocations[3], 1, glm::value_ptr(configUniforms[i][3]));
			gl::DrawArrays(GL_TRIANGLE_STRIP, 0, 3);
			debugThrowOnApiError("Kawase Pass after render");
		}
	}
};

// Developed by Marius Bj�rge (ARM)
// Bandwidth-Efficient Rendering - siggraph2015-mmg-marius
// Filters images whilst Downsampling and Upsampling
struct DualFilterBlurPass
{
	// We only need (MaxFilterIterations - 1) images as the first image is an input to the blur pass
	// We also special case the final pass as this requires either a different pipeline or a different descriptor set/layout

	// Special cased final pass pipeline where the final upsample pass and compositing occurs in the same pipeline. This lets us avoid an extra write to memory/read from memory pass
	GLuint finalPassProgram;
	GLuint finalPassBloomOnlyProgram;

	GLuint upSampleProgram;
	GLuint downSampleProgram;

	// The pre allocated framebuffers for the iterations up to MaxFilterIterations
	GLuint framebuffers[MaxFilterIterations - 1];

	// The current set of framebuffers in use for the currently selected configuration
	GLuint currentFramebuffers[MaxFilterIterations - 1];

	// The pre allocated image views for the iterations up to MaxFilterIterations
	GLuint textures[MaxFilterIterations - 1];

	// The current set of image views in use for the currently selected configuration
	GLuint currentTextures[MaxFilterIterations - 1];

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

	// The current set of uniforms for the current configuration
	glm::vec2 configUniforms[MaxFilterIterations][8];

	// The final full resolution framebuffer dimensions
	glm::uvec2 framebufferDimensions;

	// The colour image format in use
	GLuint colorImageFormat;

	GLint upSampleBlurConfigLocations[8];
	GLint downSampleBlurConfigLocations[4];
	GLint finalUpSampleBlurConfigLocations[8];
	GLint finalUpSampleBlurBloomOnlyConfigLocations[8];

	GLint exposureUniformLocation;

	/// <summary>Initialises the Dual Filter blur.</summary>
	/// <param name="assetProvider">The pvr::IAssetProvider which will be used for loading resources from memory.</param>
	/// <param name="colorImageFormat">The colour image format to use for the Dual Filter blur.</param>
	/// <param name="framebufferDimensions">The full size resolution framebuffer dimensions.</param>
	void init(pvr::IAssetProvider& assetProvider, GLuint colorImageFormat_, const glm::uvec2& framebufferDimensions_, bool srgbFramebuffer)
	{
		this->colorImageFormat = colorImageFormat_;
		this->framebufferDimensions = framebufferDimensions_;
		this->blurIterations = static_cast<uint32_t>(-1);

		// Calculate the maximum set of per iteration framebuffer dimensions
		// The maximum set will start from framebufferDimensions and allow for MaxFilterIterations. Note that this includes down and up sample passes
		calculateIterationDimensions();

		// Allocates the images used for each of the down/up sample passes
		allocatePingPongTextures();

		// Create the dual filter framebuffers
		createFramebuffers();

		// Create the up and down sample programs
		createPrograms(assetProvider, srgbFramebuffer);
	}

	/// <summary>Returns the blurred texture.</summary>
	/// <returns>The blurred texture.</returns>
	GLuint getBlurredTexture() { return currentTextures[blurIterations - 1]; }

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

			// Configure the Dual Filter uniform values based on the current Dual Filter configuration
			configureConfigUniforms();

			// Configure the set of Dual Filter ping pong images based on the current Dual Filter configuration
			configurePingPongTextures();

			// Configure the set of Framebuffers based on the current Dual Filter configuration
			configureFramebuffers();
		}
	}

	/// <summary>Configure the set of Framebuffers based on the current Dual Filter configuration.</summary>
	virtual void configureFramebuffers()
	{
		uint32_t index = 0;
		for (; index < blurIterations / 2; ++index) { currentFramebuffers[index] = framebuffers[index]; }

		for (uint32_t i = MaxFilterIterations - (blurIterations / 2); i < MaxFilterIterations - 1; ++i)
		{
			currentFramebuffers[index] = framebuffers[i];
			index++;
		}
	}

	/// <summary>Configure the set of Dual Filter ping pong images based on the current Dual Filter configuration.</summary>
	virtual void configurePingPongTextures()
	{
		uint32_t index = 0;
		for (; index < blurIterations / 2; ++index) { currentTextures[index] = textures[index]; }

		for (uint32_t i = MaxFilterIterations - (blurIterations / 2); i < MaxFilterIterations - 1; ++i)
		{
			currentTextures[index] = textures[i];
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
		glm::uvec2 dimension =
			glm::uvec2(glm::ceil(framebufferDimensions.x / glm::pow(2, MaxFilterIterations / 2)), glm::ceil(framebufferDimensions.y / glm::pow(2, MaxFilterIterations / 2)));

		for (int32_t i = (MaxFilterIterations / 2) - 1; i >= 0; --i)
		{
			maxIterationDimensions[i] = dimension;
			glm::vec2 inverseDimensions = glm::vec2(1.0f / dimension.x, 1.0f / dimension.y);
			maxIterationInverseDimensions[i] = inverseDimensions;
			dimension = glm::uvec2(glm::ceil(dimension.x * 2.0f), glm::ceil(dimension.y * 2.0f));
		}

		dimension =
			glm::uvec2(glm::ceil(framebufferDimensions.x / glm::pow(2, MaxFilterIterations / 2 - 1)), glm::ceil(framebufferDimensions.y / glm::pow(2, MaxFilterIterations / 2 - 1)));

		for (uint32_t i = MaxFilterIterations / 2; i < MaxFilterIterations - 1; ++i)
		{
			maxIterationDimensions[i] = dimension;
			glm::vec2 inverseDimensions = glm::vec2(1.0f / dimension.x, 1.0f / dimension.y);
			maxIterationInverseDimensions[i] = inverseDimensions;
			dimension = glm::uvec2(glm::ceil(dimension.x * 2.0f), glm::ceil(dimension.y * 2.0f));
		}

		dimension = glm::uvec2(glm::ceil(framebufferDimensions.x), glm::ceil(framebufferDimensions.y));

		maxIterationDimensions[MaxFilterIterations - 1] = dimension;
		glm::vec2 inverseDimensions = glm::vec2(1.0f / dimension.x, 1.0f / dimension.y);
		maxIterationInverseDimensions[MaxFilterIterations - 1] = inverseDimensions;
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

	/// <summary>Allocates the textures used for each of the down / up sample passes.</summary>
	virtual void allocatePingPongTextures()
	{
		for (uint32_t i = 0; i < MaxFilterIterations / 2; ++i)
		{
			gl::GenTextures(1, &textures[i]);
			gl::BindTexture(GL_TEXTURE_2D, textures[i]);
			gl::TexStorage2D(GL_TEXTURE_2D, 1, colorImageFormat, static_cast<GLsizei>(maxIterationDimensions[i].x), static_cast<GLsizei>(maxIterationDimensions[i].y));
		}

		// We're able to reuse images between up/down sample passes. This can help us keep down the total number of images in flight
		uint32_t k = 0;
		for (uint32_t i = MaxFilterIterations / 2; i < MaxFilterIterations - 1; ++i)
		{
			uint32_t reuseIndex = (MaxFilterIterations / 2) - 1 - (k + 1);
			textures[i] = textures[reuseIndex];
			k++;
		}
	}

	/// <summary>Allocates the framebuffers used for each of the down / up sample passes.</summary>
	virtual void createFramebuffers()
	{
		for (uint32_t i = 0; i < MaxFilterIterations - 1; ++i)
		{
			gl::GenFramebuffers(1, &framebuffers[i]);
			gl::BindFramebuffer(GL_DRAW_FRAMEBUFFER, framebuffers[i]);
			gl::FramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textures[i], 0);
			gl::FramebufferParameteri(GL_DRAW_FRAMEBUFFER, GL_FRAMEBUFFER_DEFAULT_WIDTH, static_cast<GLint>(maxIterationDimensions[i].x));
			gl::FramebufferParameteri(GL_DRAW_FRAMEBUFFER, GL_FRAMEBUFFER_DEFAULT_HEIGHT, static_cast<GLint>(maxIterationDimensions[i].y));
		}
	}

	void getUpSampleConfigUniformLocations(GLint upSampleBlurConfigLocations_[8], GLuint program, const std::string& uniformLocationName)
	{
		for (uint32_t i = 0; i < 8; ++i)
		{ upSampleBlurConfigLocations_[i] = gl::GetUniformLocation(program, pvr::strings::createFormatted(std::string(uniformLocationName + "[%i]").c_str(), i).c_str()); }
	}

	void setUpSampleConfigUniforms(GLint upSampleBlurConfigLocations_[8], const glm::vec2 upSampleConfigs[8])
	{
		for (uint32_t i = 0; i < 8; ++i) { gl::Uniform2fv(upSampleBlurConfigLocations_[i], 1, glm::value_ptr(upSampleConfigs[i])); }
	}

	/// <summary>Creates the Dual Filter programs.</summary>
	/// <param name="assetProvider">The pvr::IAssetProvider which will be used for loading resources from memory.</param>
	virtual void createPrograms(pvr::IAssetProvider& assetProvider, bool srgbFramebuffer)
	{
		// Enable or disable gamma correction based on if it is automatically performed on the framebuffer or we need to do it in the shader.
		std::vector<const char*> defines;
		if (srgbFramebuffer) { defines.push_back("FRAMEBUFFER_SRGB"); }

		downSampleProgram = pvr::utils::createShaderProgram(assetProvider, Files::DualFilterDownVertSrcFile, Files::DualFilterDownSampleFragSrcFile, nullptr, nullptr, 0);
		upSampleProgram = pvr::utils::createShaderProgram(assetProvider, Files::DualFilterUpVertSrcFile, Files::DualFilterUpSampleFragSrcFile, nullptr, nullptr, 0);

		gl::UseProgram(downSampleProgram);
		gl::Uniform1i(gl::GetUniformLocation(downSampleProgram, "sTexture"), 0);
		downSampleBlurConfigLocations[0] = gl::GetUniformLocation(downSampleProgram, "blurConfigs[0]");
		downSampleBlurConfigLocations[1] = gl::GetUniformLocation(downSampleProgram, "blurConfigs[1]");
		downSampleBlurConfigLocations[2] = gl::GetUniformLocation(downSampleProgram, "blurConfigs[2]");
		downSampleBlurConfigLocations[3] = gl::GetUniformLocation(downSampleProgram, "blurConfigs[3]");

		gl::UseProgram(upSampleProgram);
		gl::Uniform1i(gl::GetUniformLocation(upSampleProgram, "sTexture"), 0);
		getUpSampleConfigUniformLocations(upSampleBlurConfigLocations, upSampleProgram, "blurConfigs");

		finalPassProgram = pvr::utils::createShaderProgram(assetProvider, Files::DualFilterUpVertSrcFile, Files::DualFilterUpSampleMergedFinalPassFragSrcFile, nullptr, nullptr, 0,
			defines.data(), static_cast<uint32_t>(defines.size()));

		gl::UseProgram(finalPassProgram);
		getUpSampleConfigUniformLocations(finalUpSampleBlurConfigLocations, finalPassProgram, "blurConfigs");
		gl::Uniform1i(gl::GetUniformLocation(finalPassProgram, "sBlurTexture"), 0);
		gl::Uniform1i(gl::GetUniformLocation(finalPassProgram, "sOffScreenTexture"), 1);
		exposureUniformLocation = gl::GetUniformLocation(finalPassProgram, "linearExposure");

		defines.push_back("RENDER_BLOOM");
		finalPassBloomOnlyProgram = pvr::utils::createShaderProgram(assetProvider, Files::DualFilterUpVertSrcFile, Files::DualFilterUpSampleMergedFinalPassFragSrcFile, nullptr,
			nullptr, 0, defines.data(), static_cast<uint32_t>(defines.size()));

		gl::UseProgram(finalPassBloomOnlyProgram);
		getUpSampleConfigUniformLocations(finalUpSampleBlurBloomOnlyConfigLocations, finalPassBloomOnlyProgram, "blurConfigs");
		gl::Uniform1i(gl::GetUniformLocation(finalPassBloomOnlyProgram, "sBlurTexture"), 0);
	}

	/// <summary>Configure the Dual Filter uniforms values based on the current Dual Filter configuration.</summary>
	virtual void configureConfigUniforms()
	{
		for (uint32_t i = 0; i < blurIterations; ++i)
		{
			// Downsample
			if (i < blurIterations / 2)
			{
				glm::vec2 pixelSize = glm::vec2(currentIterationInverseDimensions[i]);

				glm::vec2 halfPixelSize = pixelSize / 2.0f;
				glm::vec2 dUV = pixelSize + halfPixelSize;

				configUniforms[i][0] = glm::vec2(-dUV);
				configUniforms[i][1] = glm::vec2(dUV);
				configUniforms[i][2] = glm::vec2(dUV.x, -dUV.y);
				configUniforms[i][3] = glm::vec2(-dUV.x, dUV.y);
			}
			// Upsample
			else
			{
				glm::vec2 pixelSize = glm::vec2(currentIterationInverseDimensions[i]);

				glm::vec2 halfPixelSize = pixelSize / 2.0f;
				glm::vec2 dUV = pixelSize + halfPixelSize;

				configUniforms[i][0] = glm::vec2(-dUV.x * 2.0, 0.0);
				configUniforms[i][1] = glm::vec2(-dUV.x, dUV.y);
				configUniforms[i][2] = glm::vec2(0.0, dUV.y * 2.0);
				configUniforms[i][3] = glm::vec2(dUV.x, dUV.y);
				configUniforms[i][4] = glm::vec2(dUV.x * 2.0, 0.0);
				configUniforms[i][5] = glm::vec2(dUV.x, -dUV.y);
				configUniforms[i][6] = glm::vec2(0.0, -dUV.y * 2.0);
				configUniforms[i][7] = glm::vec2(-dUV.x, -dUV.y);
			}
		}
	}

	/// <summary>Renders the Dual Filter blur iterations based on the current configuration.</summary>
	/// <param name="sourceTexture">The source texture.</param>
	/// <param name="offscreenTexture">The offscreen texture.</param>
	/// <param name="onScreenFbo">The on screen fbo.</param>
	/// <param name="samplerBilinear">The sampler object to use when sampling Dual Filter.</param>
	/// <param name="exposure">The exposure to use in the tone mapping.</param>
	virtual void render(GLuint sourceTexture, GLuint offscreenTexture, GLuint onScreenFbo, GLuint samplerBilinear, bool renderBloomOnly, float exposure)
	{
		gl::ActiveTexture(GL_TEXTURE0);
		gl::BindSampler(0, samplerBilinear);

		uint32_t i = 0;

		// Downsample passes
		for (; i < blurIterations / 2; ++i)
		{
			gl::Viewport(0, 0, static_cast<GLsizei>(currentIterationDimensions[i].x), static_cast<GLsizei>(currentIterationDimensions[i].y));

			debugThrowOnApiError("Dual Filter First Downsample before render");
			gl::BindFramebuffer(GL_DRAW_FRAMEBUFFER, currentFramebuffers[i]);
			gl::Clear(GL_COLOR_BUFFER_BIT);

			if (i == 0) { gl::BindTexture(GL_TEXTURE_2D, sourceTexture); }
			else
			{
				gl::BindTexture(GL_TEXTURE_2D, currentTextures[i - 1]);
			}

			gl::UseProgram(downSampleProgram);
			gl::Uniform2fv(downSampleBlurConfigLocations[0], 1, glm::value_ptr(configUniforms[i][0]));
			gl::Uniform2fv(downSampleBlurConfigLocations[1], 1, glm::value_ptr(configUniforms[i][1]));
			gl::Uniform2fv(downSampleBlurConfigLocations[2], 1, glm::value_ptr(configUniforms[i][2]));
			gl::Uniform2fv(downSampleBlurConfigLocations[3], 1, glm::value_ptr(configUniforms[i][3]));

			gl::DrawArrays(GL_TRIANGLE_STRIP, 0, 3);

			debugThrowOnApiError("Dual Filter First Downsample after render");
		}

		// Up sample passes
		for (; i < blurIterations - 1; ++i)
		{
			debugThrowOnApiError("Dual Filter First Upsample before render");

			gl::Viewport(0, 0, static_cast<GLsizei>(currentIterationDimensions[i].x), static_cast<GLsizei>(currentIterationDimensions[i].y));

			gl::UseProgram(upSampleProgram);
			setUpSampleConfigUniforms(upSampleBlurConfigLocations, configUniforms[i]);

			gl::BindFramebuffer(GL_DRAW_FRAMEBUFFER, currentFramebuffers[i]);
			gl::Clear(GL_COLOR_BUFFER_BIT);

			gl::BindTexture(GL_TEXTURE_2D, currentTextures[i - 1]);

			gl::DrawArrays(GL_TRIANGLE_STRIP, 0, 3);
			debugThrowOnApiError("Dual Filter First Upsample after render");
		}

		// Final Up sample
		debugThrowOnApiError("Dual Filter Final Pass before render");

		gl::Viewport(0, 0, static_cast<GLsizei>(currentIterationDimensions[i].x), static_cast<GLsizei>(currentIterationDimensions[i].y));

		gl::BindFramebuffer(GL_DRAW_FRAMEBUFFER, onScreenFbo);
		gl::Clear(GL_COLOR_BUFFER_BIT);

		gl::BindTexture(GL_TEXTURE_2D, currentTextures[blurIterations - 2]);

		if (renderBloomOnly)
		{
			gl::UseProgram(finalPassBloomOnlyProgram);
			setUpSampleConfigUniforms(finalUpSampleBlurBloomOnlyConfigLocations, configUniforms[i]);
		}
		else
		{
			gl::ActiveTexture(GL_TEXTURE1);
			gl::BindTexture(GL_TEXTURE_2D, offscreenTexture);
			gl::BindSampler(1, samplerBilinear);

			gl::UseProgram(finalPassProgram);
			setUpSampleConfigUniforms(finalUpSampleBlurConfigLocations, configUniforms[i]);
			gl::Uniform1f(exposureUniformLocation, exposure);
		}

		gl::DrawArrays(GL_TRIANGLE_STRIP, 0, 3);
		debugThrowOnApiError("Dual Filter Final Up sample Pass after render");
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
	// Defines a scale to use for offsetting the tent offsets
	glm::vec2 tentScales[MaxFilterIterations / 2];

	// A set of downsample passes
	DownSamplePass4x4 downsamplePasses[MaxFilterIterations / 2];

	GLuint firstUpSampleProgram;

	/// <summary>Initialises the Dual Filter blur.</summary>
	/// <param name="assetProvider">The pvr::IAssetProvider which will be used for loading resources from memory.</param>
	/// <param name="colorImageFormat">The colour image format to use for the Dual Filter blur.</param>
	/// <param name="inFramebufferDimensions">The full size resolution framebuffer dimensions.</param>
	/// <param name="isIMGFramebufferDownsampleSupported">Specifies whether the extension GL_IMG_framebuffer_downsample is supported.</param>
	void init(pvr::IAssetProvider& assetProvider, GLuint inColorImageFormat, const glm::uvec2& inFramebufferDimensions, bool srgbFramebuffer)
	{
		// These parameters are used to scale the tent filter so that it does not map directly to pixels and may have "holes"
		tentScales[0] = glm::vec2(1.0f, 1.0f);
		tentScales[1] = glm::vec2(1.0f, 1.0f);
		tentScales[2] = glm::vec2(1.0f, 1.0f);
		tentScales[3] = glm::vec2(1.0f, 1.0f);
		tentScales[4] = glm::vec2(1.0f, 1.0f);

		DualFilterBlurPass::init(assetProvider, inColorImageFormat, inFramebufferDimensions, srgbFramebuffer);

		for (uint32_t i = 0; i < MaxFilterIterations / 2; ++i)
		{
			downsamplePasses[i].init(assetProvider, textures[i], glm::uvec2(maxIterationDimensions[i].x, maxIterationDimensions[i].y),
				glm::uvec2(maxIterationDimensions[i].x * 2.0, maxIterationDimensions[i].y * 2.0));
		}
	}

	void allocatePingPongTextures() override
	{
		for (uint32_t i = 0; i < MaxFilterIterations - 1; ++i)
		{
			gl::GenTextures(1, &textures[i]);
			gl::BindTexture(GL_TEXTURE_2D, textures[i]);
			gl::TexStorage2D(GL_TEXTURE_2D, 1, colorImageFormat, static_cast<GLsizei>(maxIterationDimensions[i].x), static_cast<GLsizei>(maxIterationDimensions[i].y));
		}
	}

	virtual void createPrograms(pvr::IAssetProvider& assetProvider, bool srgbFramebuffer) override
	{
		// Enable or disable gamma correction based on if it is automatically performed on the framebuffer or we need to do it in the shader.
		std::vector<const char*> defines;
		if (srgbFramebuffer) { defines.push_back("FRAMEBUFFER_SRGB"); }

		firstUpSampleProgram = pvr::utils::createShaderProgram(assetProvider, Files::PostBloomVertShaderSrcFile, Files::TentFilterFirstUpSampleFragSrcFile, nullptr, nullptr, 0);
		gl::UseProgram(firstUpSampleProgram);
		gl::Uniform1i(gl::GetUniformLocation(firstUpSampleProgram, "sDownsampledImage"), 0);

		upSampleProgram = pvr::utils::createShaderProgram(assetProvider, Files::TentFilterUpSampleVertSrcFile, Files::TentFilterUpSampleFragSrcFile, nullptr, nullptr, 0);
		gl::UseProgram(upSampleProgram);
		gl::Uniform1i(gl::GetUniformLocation(upSampleProgram, "sCurrentBlurredImage"), 0);
		gl::Uniform1i(gl::GetUniformLocation(upSampleProgram, "sDownsampledCurrentMipLevel"), 1);
		getUpSampleConfigUniformLocations(upSampleBlurConfigLocations, upSampleProgram, "upSampleConfigs");

		finalPassProgram = pvr::utils::createShaderProgram(assetProvider, Files::TentFilterUpSampleVertSrcFile, Files::TentFilterUpSampleMergedFinalPassFragSrcFile, nullptr,
			nullptr, 0, defines.data(), static_cast<uint32_t>(defines.size()));
		gl::UseProgram(finalPassProgram);
		getUpSampleConfigUniformLocations(finalUpSampleBlurConfigLocations, finalPassProgram, "upSampleConfigs");
		gl::Uniform1i(gl::GetUniformLocation(finalPassProgram, "sCurrentBlurredImage"), 0);
		gl::Uniform1i(gl::GetUniformLocation(finalPassProgram, "sDownsampledCurrentMipLevel"), 1);
		gl::Uniform1i(gl::GetUniformLocation(finalPassProgram, "sOffScreenTexture"), 2);
		exposureUniformLocation = gl::GetUniformLocation(finalPassProgram, "linearExposure");

		defines.push_back("RENDER_BLOOM");
		finalPassBloomOnlyProgram = pvr::utils::createShaderProgram(assetProvider, Files::TentFilterUpSampleVertSrcFile, Files::TentFilterUpSampleMergedFinalPassFragSrcFile,
			nullptr, nullptr, 0, defines.data(), static_cast<uint32_t>(defines.size()));

		gl::UseProgram(finalPassBloomOnlyProgram);
		getUpSampleConfigUniformLocations(finalUpSampleBlurBloomOnlyConfigLocations, finalPassBloomOnlyProgram, "upSampleConfigs");
		gl::Uniform1i(gl::GetUniformLocation(finalPassBloomOnlyProgram, "sCurrentBlurredImage"), 0);
		gl::Uniform1i(gl::GetUniformLocation(finalPassBloomOnlyProgram, "sDownsampledCurrentMipLevel"), 1);
	}

	virtual void configureConfigUniforms() override
	{
		const glm::vec2 offsets[8] = { glm::vec2(-1.0, 1.0), glm::vec2(0.0, 1.0), glm::vec2(1.0, 1.0), glm::vec2(1.0, 0.0), glm::vec2(1.0, -1.0), glm::vec2(0.0, -1.0),
			glm::vec2(-1.0, -1.0), glm::vec2(-1.0, 0.0) };

		uint32_t tentScaleIndex = 0;
		// The tent filter passes only start after the first up sample pass has finished
		for (uint32_t i = blurIterations / 2; i < blurIterations; ++i)
		{
			configUniforms[i][0] = glm::vec2(1.0f / (currentIterationDimensions[i].x * 0.5), 1.0f / (currentIterationDimensions[i].y * 0.5)) * offsets[0] * tentScales[tentScaleIndex];
			configUniforms[i][1] = glm::vec2(1.0f / (currentIterationDimensions[i].x * 0.5), 1.0f / (currentIterationDimensions[i].y * 0.5)) * offsets[1] * tentScales[tentScaleIndex];
			configUniforms[i][2] = glm::vec2(1.0f / (currentIterationDimensions[i].x * 0.5), 1.0f / (currentIterationDimensions[i].y * 0.5)) * offsets[2] * tentScales[tentScaleIndex];
			configUniforms[i][3] = glm::vec2(1.0f / (currentIterationDimensions[i].x * 0.5), 1.0f / (currentIterationDimensions[i].y * 0.5)) * offsets[3] * tentScales[tentScaleIndex];
			configUniforms[i][4] = glm::vec2(1.0f / (currentIterationDimensions[i].x * 0.5), 1.0f / (currentIterationDimensions[i].y * 0.5)) * offsets[4] * tentScales[tentScaleIndex];
			configUniforms[i][5] = glm::vec2(1.0f / (currentIterationDimensions[i].x * 0.5), 1.0f / (currentIterationDimensions[i].y * 0.5)) * offsets[5] * tentScales[tentScaleIndex];
			configUniforms[i][6] = glm::vec2(1.0f / (currentIterationDimensions[i].x * 0.5), 1.0f / (currentIterationDimensions[i].y * 0.5)) * offsets[6] * tentScales[tentScaleIndex];
			configUniforms[i][7] = glm::vec2(1.0f / (currentIterationDimensions[i].x * 0.5), 1.0f / (currentIterationDimensions[i].y * 0.5)) * offsets[7] * tentScales[tentScaleIndex];
			tentScaleIndex++;
		}
	}

	virtual void render(GLuint sourceTexture, GLuint offscreenTexture, GLuint onScreenFbo, GLuint samplerBilinear, bool renderBloomOnly, float exposure) override
	{
		uint32_t downsampledIndex = 1;

		GLenum drawBuffers[] = { GL_COLOR_ATTACHMENT0 };
		gl::DrawBuffers(1, drawBuffers);

		uint32_t i = 0;

		// Perform downsamples using separate passes
		for (; i < blurIterations / 2; ++i)
		{
			GLuint inputTexture = -1;
			if (i == 0) { inputTexture = sourceTexture; }
			else
			{
				inputTexture = currentTextures[i - 1];
			}
			downsamplePasses[i].render(inputTexture, samplerBilinear);
		}

		// Upsample
		for (; i < blurIterations - 1; ++i)
		{
			debugThrowOnApiError("Tent Filter Up sample Pass before render");
			gl::Viewport(0, 0, static_cast<GLsizei>(currentIterationDimensions[i].x), static_cast<GLsizei>(currentIterationDimensions[i].y));
			gl::BindFramebuffer(GL_DRAW_FRAMEBUFFER, currentFramebuffers[i]);
			gl::Clear(GL_COLOR_BUFFER_BIT);

			gl::ActiveTexture(GL_TEXTURE0);
			gl::BindTexture(GL_TEXTURE_2D, currentTextures[i - 1]);
			gl::BindSampler(0, samplerBilinear);

			if (i == blurIterations / 2) { gl::UseProgram(firstUpSampleProgram); }
			else
			{
				gl::ActiveTexture(GL_TEXTURE1);
				gl::BindTexture(GL_TEXTURE_2D, currentTextures[blurIterations / 2 - 1 - downsampledIndex]);
				gl::BindSampler(1, samplerBilinear);
				downsampledIndex++;

				gl::UseProgram(upSampleProgram);
				setUpSampleConfigUniforms(upSampleBlurConfigLocations, configUniforms[i]);
			}

			gl::DrawArrays(GL_TRIANGLE_STRIP, 0, 3);
			debugThrowOnApiError("Tent Filter Pass after render");
		}

		// Final pass up sample
		debugThrowOnApiError("Tent Filter Final Up sample Pass before render");
		gl::Viewport(0, 0, static_cast<GLsizei>(currentIterationDimensions[i].x), static_cast<GLsizei>(currentIterationDimensions[i].y));
		gl::BindFramebuffer(GL_DRAW_FRAMEBUFFER, onScreenFbo);
		gl::Clear(GL_COLOR_BUFFER_BIT);

		if (renderBloomOnly)
		{
			gl::ActiveTexture(GL_TEXTURE0);
			gl::BindTexture(GL_TEXTURE_2D, currentTextures[i - 1]);
			gl::BindSampler(0, samplerBilinear);

			gl::ActiveTexture(GL_TEXTURE1);
			gl::BindTexture(GL_TEXTURE_2D, currentTextures[0]);
			gl::BindSampler(1, samplerBilinear);

			gl::UseProgram(finalPassBloomOnlyProgram);
			setUpSampleConfigUniforms(finalUpSampleBlurBloomOnlyConfigLocations, configUniforms[i]);
		}
		else
		{
			gl::ActiveTexture(GL_TEXTURE0);
			gl::BindTexture(GL_TEXTURE_2D, currentTextures[i - 1]);
			gl::BindSampler(0, samplerBilinear);

			gl::ActiveTexture(GL_TEXTURE1);
			gl::BindTexture(GL_TEXTURE_2D, currentTextures[0]);
			gl::BindSampler(1, samplerBilinear);

			gl::ActiveTexture(GL_TEXTURE2);
			gl::BindTexture(GL_TEXTURE_2D, offscreenTexture);
			gl::BindSampler(2, samplerBilinear);

			gl::UseProgram(finalPassProgram);
			setUpSampleConfigUniforms(finalUpSampleBlurConfigLocations, configUniforms[i]);
			gl::Uniform1f(exposureUniformLocation, exposure);
		}

		gl::DrawArrays(GL_TRIANGLE_STRIP, 0, 3);
		debugThrowOnApiError("Tent Filter Final Up sample Pass after render");
	}
};

/// <summary>A Gaussian Blur Pass.</summary>
struct GaussianBlurPass
{
	// Horizontal and Vertical graphics pipelines
	GLuint horizontalPrograms[DemoConfigurations::NumDemoConfigurations];
	GLuint verticalPrograms[DemoConfigurations::NumDemoConfigurations];

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
	/// <param name="blurFramebufferDimensions">The dimensions used for the blur.</param>
	void init(pvr::IAssetProvider& assetProvider, const glm::uvec2& blurFramebufferDimensions)
	{
		inverseFramebufferWidth = 1.0f / blurFramebufferDimensions.x;
		inverseFramebufferHeight = 1.0f / blurFramebufferDimensions.y;

		gaussianWeights.resize(DemoConfigurations::NumDemoConfigurations);
		gaussianOffsets.resize(DemoConfigurations::NumDemoConfigurations);

		generatePerConfigGaussianCoefficients();
		generateGaussianShaderStrings();

		createPrograms(assetProvider);

		debugThrowOnApiError("GaussianBlurPass init");
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

	/// <summary>Creates the Gaussian blur programs.</summary>
	/// <param name="assetProvider">The pvr::IAssetProvider which will be used for loading resources from memory.</param>
	virtual void createPrograms(pvr::IAssetProvider& assetProvider)
	{
		GLuint horizontalFragShaders[DemoConfigurations::NumDemoConfigurations];
		GLuint verticalFragShaders[DemoConfigurations::NumDemoConfigurations];

		// Generate the Gaussian blur fragment shaders
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

			// Create shaders using the auto generated shader sources
			horizontalFragShaders[i] = pvr::utils::loadShader(horizontalShaderString, pvr::ShaderType::FragmentShader, nullptr, 0);
			verticalFragShaders[i] = pvr::utils::loadShader(verticalShaderString, pvr::ShaderType::FragmentShader, nullptr, 0);
		}

		// Load the base Gaussian vertex shader
		auto vertexShaderStream = assetProvider.getAssetStream(Files::GaussianVertSrcFile);
		std::string vertexShaderSource;
		vertexShaderStream->readIntoString(vertexShaderSource);

		GLuint vertexShader = pvr::utils::loadShader(vertexShaderSource, pvr::ShaderType::VertexShader, nullptr, 0);

		for (uint32_t i = 0; i < DemoConfigurations::NumDemoConfigurations; ++i)
		{
			GLuint shaders[2] = { vertexShader, horizontalFragShaders[i] };
			// Horizontal Program
			horizontalPrograms[i] = pvr::utils::createShaderProgram(shaders, 2, nullptr, nullptr, 0, nullptr);

			gl::UseProgram(horizontalPrograms[i]);
			gl::Uniform1i(gl::GetUniformLocation(horizontalPrograms[i], "sTexture"), 0);
		}

		for (uint32_t i = 0; i < DemoConfigurations::NumDemoConfigurations; ++i)
		{
			GLuint shaders[2] = { vertexShader, verticalFragShaders[i] };
			// Vertical Program
			verticalPrograms[i] = pvr::utils::createShaderProgram(shaders, 2, nullptr, nullptr, 0, nullptr);

			gl::UseProgram(verticalPrograms[i]);
			gl::Uniform1i(gl::GetUniformLocation(verticalPrograms[i], "sTexture"), 0);
		}
	}

	/// <summary>Performs a Gaussian Blur on the source texture.</summary>
	/// <param name="sourceTexture">The source texture to perform a Gaussian Blur on.</param>
	/// <param name="horizontalBlurFramebuffer">The framebuffer to use for the horizontal Gaussian blur.</param>
	/// <param name="verticalBlurFramebuffer">The framebuffer to use for the vertical Gaussian blur.</param>
	/// <param name="samplerBilinear">The sampler object to use when sampling.</param>
	virtual void render(GLuint sourceTexture, Framebuffer& horizontalBlurFramebuffer, Framebuffer& verticalBlurFramebuffer, GLuint samplerBilinear)
	{
		debugThrowOnApiError("Gaussian Blur Pass before render");
		gl::BindFramebuffer(GL_DRAW_FRAMEBUFFER, horizontalBlurFramebuffer.framebuffer);
		gl::Clear(GL_COLOR_BUFFER_BIT);

		gl::ActiveTexture(GL_TEXTURE0);
		gl::BindTexture(GL_TEXTURE_2D, sourceTexture);
		gl::BindSampler(0, samplerBilinear);

		gl::UseProgram(horizontalPrograms[currentKernelConfig]);
		gl::DrawArrays(GL_TRIANGLE_STRIP, 0, 3);

		gl::BindFramebuffer(GL_DRAW_FRAMEBUFFER, verticalBlurFramebuffer.framebuffer);
		gl::Clear(GL_COLOR_BUFFER_BIT);

		gl::BindTexture(GL_TEXTURE_2D, horizontalBlurFramebuffer.attachments[static_cast<uint32_t>(BloomAttachments::Bloom)]);
		gl::UseProgram(verticalPrograms[currentKernelConfig]);
		gl::DrawArrays(GL_TRIANGLE_STRIP, 0, 3);
		debugThrowOnApiError("Gaussian Blur Pass after render");
	}
};

/// <summary>A Compute shader based Gaussian Blur Pass.</summary>
struct ComputeBlurPass : public GaussianBlurPass
{
	std::vector<std::string> perKernelSizeCacheStrings;

	void generatePerConfigGaussianCoefficients() override
	{
		for (uint32_t i = 0; i < DemoConfigurations::NumDemoConfigurations; ++i)
		{ generateGaussianCoefficients(DemoConfigurations::Configurations[i].computeGaussianConfig, false, false, gaussianWeights[i], gaussianOffsets[i]); }
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

			perKernelSizeCacheStrings[i] = pvr::strings::createFormatted("mediump float f[numIterations * 2u] = float[numIterations * 2u](%s);", cache.c_str());
		}
	}

	virtual void createPrograms(pvr::IAssetProvider& assetProvider) override
	{
		GLuint horizontalShaders[DemoConfigurations::NumDemoConfigurations];
		GLuint verticalShaders[DemoConfigurations::NumDemoConfigurations];

		// Generate the Gaussian blur compute shaders
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
			std::string horizontalShaderString = pvr::strings::createFormatted(
				horizontalShaderSource.c_str(), perKernelSizeIterationsStrings[i].c_str(), perKernelSizeWeightsStrings[i].c_str(), perKernelSizeCacheStrings[i].c_str());
			std::string verticalShaderString = pvr::strings::createFormatted(
				verticalShaderSource.c_str(), perKernelSizeIterationsStrings[i].c_str(), perKernelSizeWeightsStrings[i].c_str(), perKernelSizeCacheStrings[i].c_str());

			// Create shaders using the auto generated shader sources
			horizontalShaders[i] = pvr::utils::loadShader(horizontalShaderString, pvr::ShaderType::ComputeShader, nullptr, 0);
			verticalShaders[i] = pvr::utils::loadShader(verticalShaderString, pvr::ShaderType::ComputeShader, nullptr, 0);

			// Horizontal Program
			horizontalPrograms[i] = pvr::utils::createShaderProgram(&horizontalShaders[i], 1, nullptr, nullptr, 0, nullptr);

			// Vertical Program
			verticalPrograms[i] = pvr::utils::createShaderProgram(&verticalShaders[i], 1, nullptr, nullptr, 0, nullptr);
		}
	}

	using GaussianBlurPass::render;
	virtual void render(GLuint sourceTexture, Framebuffer& horizontalBlurFramebuffer, Framebuffer& verticalBlurFramebuffer, GLenum imageFormat) override
	{
		debugThrowOnApiError("Compute Gaussian Blur Pass before render");

		assert(horizontalBlurFramebuffer.dimensions.x == verticalBlurFramebuffer.dimensions.x && horizontalBlurFramebuffer.dimensions.y == verticalBlurFramebuffer.dimensions.y);

		// horizontal
		{
			// We Execute the Compute shader, we bind the input and output texture.
			gl::UseProgram(horizontalPrograms[currentKernelConfig]);

			gl::BindImageTexture(0, sourceTexture, 0, GL_FALSE, 0, GL_READ_ONLY, imageFormat);
			gl::BindImageTexture(1, horizontalBlurFramebuffer.attachments[static_cast<uint32_t>(BloomAttachments::Bloom)], 0, GL_FALSE, 0, GL_WRITE_ONLY, imageFormat);

			gl::DispatchCompute(static_cast<uint32_t>(glm::ceil(horizontalBlurFramebuffer.dimensions.y / 32.0f)), 1, 1);
			gl::MemoryBarrier(GL_TEXTURE_FETCH_BARRIER_BIT);
			debugThrowOnApiError("Compute Gaussian Blur Pass after horizontal pass");
		}

		// vertical
		{
			gl::UseProgram(verticalPrograms[currentKernelConfig]);

			gl::BindImageTexture(0, horizontalBlurFramebuffer.attachments[static_cast<uint32_t>(BloomAttachments::Bloom)], 0, GL_FALSE, 0, GL_READ_ONLY, imageFormat);
			gl::BindImageTexture(1, verticalBlurFramebuffer.attachments[static_cast<uint32_t>(BloomAttachments::Bloom)], 0, GL_FALSE, 0, GL_WRITE_ONLY, imageFormat);

			gl::DispatchCompute(static_cast<uint32_t>(glm::ceil(horizontalBlurFramebuffer.dimensions.x / 32.0f)), 1, 1);
			gl::MemoryBarrier(GL_TEXTURE_FETCH_BARRIER_BIT);
			debugThrowOnApiError("Compute Gaussian Blur Pass after vertical pass");
		}

		debugThrowOnApiError("Compute Gaussian Blur Pass after render");
	}
};

/// <summary>A Linear sampler optimised Gaussian Blur Pass.</summary>
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

	virtual void createPrograms(pvr::IAssetProvider& assetProvider) override
	{
		// Vertex Shaders
		GLuint horizontalVertexShaders[DemoConfigurations::NumDemoConfigurations];
		GLuint verticalVertexShaders[DemoConfigurations::NumDemoConfigurations];

		// Fragment Shaders
		GLuint fragShaders[DemoConfigurations::NumDemoConfigurations];

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
			horizontalVertexShaders[i] = pvr::utils::loadShader(horizontalShaderString, pvr::ShaderType::VertexShader, nullptr, 0);
			verticalVertexShaders[i] = pvr::utils::loadShader(verticalShaderString, pvr::ShaderType::VertexShader, nullptr, 0);

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
			fragShaders[i] = pvr::utils::loadShader(fragmentShaderString, pvr::ShaderType::FragmentShader, nullptr, 0);
		}

		for (uint32_t i = 0; i < DemoConfigurations::NumDemoConfigurations; ++i)
		{
			GLuint shaders[2] = { horizontalVertexShaders[i], fragShaders[i] };
			// Horizontal Program
			horizontalPrograms[i] = pvr::utils::createShaderProgram(shaders, 2, nullptr, nullptr, 0, nullptr);
			gl::UseProgram(horizontalPrograms[i]);
			gl::Uniform1i(gl::GetUniformLocation(horizontalPrograms[i], "sTexture"), 0);

			shaders[0] = verticalVertexShaders[i];

			// Vertical Program
			verticalPrograms[i] = pvr::utils::createShaderProgram(shaders, 2, nullptr, nullptr, 0, nullptr);
			gl::UseProgram(verticalPrograms[i]);
			gl::Uniform1i(gl::GetUniformLocation(verticalPrograms[i], "sTexture"), 0);
		}
	}

	virtual void render(GLuint sourceTexture, Framebuffer& horizontalBlurFramebuffer, Framebuffer& verticalBlurFramebuffer, GLuint samplerBilinear) override
	{
		debugThrowOnApiError("Linear Gaussian Blur Pass before render");
		gl::BindFramebuffer(GL_DRAW_FRAMEBUFFER, horizontalBlurFramebuffer.framebuffer);
		gl::Clear(GL_COLOR_BUFFER_BIT);

		gl::ActiveTexture(GL_TEXTURE0);
		gl::BindTexture(GL_TEXTURE_2D, sourceTexture);
		gl::BindSampler(0, samplerBilinear);

		gl::UseProgram(horizontalPrograms[currentKernelConfig]);
		gl::DrawArrays(GL_TRIANGLE_STRIP, 0, 3);

		gl::BindFramebuffer(GL_DRAW_FRAMEBUFFER, verticalBlurFramebuffer.framebuffer);
		gl::Clear(GL_COLOR_BUFFER_BIT);

		gl::BindTexture(GL_TEXTURE_2D, horizontalBlurFramebuffer.attachments[static_cast<uint32_t>(BloomAttachments::Bloom)]);
		gl::UseProgram(verticalPrograms[currentKernelConfig]);
		gl::DrawArrays(GL_TRIANGLE_STRIP, 0, 3);
		debugThrowOnApiError("Linear Gaussian Blur Pass after render");
	}
};

/// <summary>A Truncated Linear sampler optimised Gaussian Blur Pass.</summary>
struct TruncatedLinearGaussianBlurPass : public LinearGaussianBlurPass
{
	void generatePerConfigGaussianCoefficients() override
	{
		for (uint32_t i = 0; i < DemoConfigurations::NumDemoConfigurations; ++i)
		{ generateGaussianCoefficients(DemoConfigurations::Configurations[i].truncatedLinearGaussianConfig, true, true, gaussianWeights[i], gaussianOffsets[i]); }
	}
};

/// <summary>A Hybrid Gaussian Blur pass making use of a horizontal Compute shader pass followed by a Fragment based Vertical Gaussian Blur Pass.</summary>
struct HybridGaussianBlurPass
{
	// The Compute shader based Gaussian Blur pass - we will only be making use of the horizontal blur resources
	ComputeBlurPass* computeBlurPass;
	// The Fragment shader based Gaussian Blur pass - we will only be making use of the vertical blur resources
	TruncatedLinearGaussianBlurPass* linearBlurPass;

	/// <summary>A minimal initialisation function as no extra resources are created for this type of blur pass and instead we make use of the compute and fragment based passes.</summary>
	/// <param name="inComputeBlurPass">The Compute shader based Gaussian Blur pass - we will only be making use of the horizontal blur resources.</param>
	/// <param name="inLinearBlurPass">The Fragment shader based Gaussian Blur pass - we will only be making use of the vertical blur resources.</param>
	void init(ComputeBlurPass* inComputeBlurPass, TruncatedLinearGaussianBlurPass* inLinearBlurPass)
	{
		this->computeBlurPass = inComputeBlurPass;
		this->linearBlurPass = inLinearBlurPass;
	}

	/// <summary>Renders A hybrid Gaussian blur based on the current configuration.</summary>
	/// <param name="sourceTexture">The source texture to blur.</param>
	/// <param name="horizontalBlurFramebuffer">The framebuffer to use in the horizontal Gaussian blur.</param>
	/// <param name="verticalBlurFramebuffer">The framebuffer to use in the vertical Gaussian blur.</param>
	/// <param name="samplerBilinear">The sampler object to use when sampling.</param>
	void render(GLuint sourceTexture, Framebuffer& horizontalBlurFramebuffer, Framebuffer& verticalBlurFramebuffer, GLuint samplerBilinear, GLenum imageFormat)
	{
		debugThrowOnApiError("Hybrid Gaussian Blur Pass before render");

		assert(horizontalBlurFramebuffer.dimensions.x == verticalBlurFramebuffer.dimensions.x && horizontalBlurFramebuffer.dimensions.y == verticalBlurFramebuffer.dimensions.y);

		// horizontal
		{
			// We Execute the Compute shader, we bind the input and output texture.
			gl::UseProgram(this->computeBlurPass->horizontalPrograms[this->computeBlurPass->currentKernelConfig]);
			gl::BindImageTexture(0, sourceTexture, 0, GL_FALSE, 0, GL_READ_ONLY, imageFormat);
			gl::BindImageTexture(1, horizontalBlurFramebuffer.attachments[static_cast<uint32_t>(BloomAttachments::Bloom)], 0, GL_FALSE, 0, GL_WRITE_ONLY, imageFormat);
			gl::DispatchCompute(static_cast<uint32_t>(glm::ceil(horizontalBlurFramebuffer.dimensions.y / 32.0f)), 1, 1);
			gl::MemoryBarrier(GL_TEXTURE_FETCH_BARRIER_BIT);
		}

		// vertical
		{
			gl::BindFramebuffer(GL_DRAW_FRAMEBUFFER, verticalBlurFramebuffer.framebuffer);
			gl::Clear(GL_COLOR_BUFFER_BIT);
			gl::BindTexture(GL_TEXTURE_2D, horizontalBlurFramebuffer.attachments[static_cast<uint32_t>(BloomAttachments::Bloom)]);
			gl::BindSampler(0, samplerBilinear);
			gl::UseProgram(this->linearBlurPass->verticalPrograms[this->linearBlurPass->currentKernelConfig]);
			gl::DrawArrays(GL_TRIANGLE_STRIP, 0, 3);
		}
		debugThrowOnApiError("Hybrid Gaussian Blur Pass after render");
	}
};

/// <summary>Post bloom composition pass.</summary>
struct PostBloomPass
{
	GLuint defaultProgram;
	GLuint bloomOnlyProgram;
	void* mappedMemory;
	GLint exposureUniformLocation;

	/// <summary>Initialises the Post Bloom pass.</summary>
	/// <param name="assetProvider">The pvr::IAssetProvider which will be used for loading resources from memory.</param>
	void init(pvr::IAssetProvider& assetProvider, bool srgbFramebuffer)
	{
		createProgram(assetProvider, srgbFramebuffer);

		debugThrowOnApiError("PostBloomPass init");
	}

	/// <summary>Creates the Kawase program.</summary>
	/// <param name="assetProvider">The pvr::IAssetProvider which will be used for loading resources from memory.</param>
	void createProgram(pvr::IAssetProvider& assetProvider, bool srgbFramebuffer)
	{
		// Enable or disable gamma correction based on if it is automatically performed on the framebuffer or we need to do it in the shader.
		std::vector<const char*> defines;
		if (srgbFramebuffer) { defines.push_back("FRAMEBUFFER_SRGB"); }

		defaultProgram = pvr::utils::createShaderProgram(
			assetProvider, Files::PostBloomVertShaderSrcFile, Files::PostBloomFragShaderSrcFile, nullptr, nullptr, 0, defines.data(), static_cast<uint32_t>(defines.size()));

		gl::UseProgram(defaultProgram);
		gl::Uniform1i(gl::GetUniformLocation(defaultProgram, "sBlurTexture"), 0);
		gl::Uniform1i(gl::GetUniformLocation(defaultProgram, "sOffScreenTexture"), 1);
		exposureUniformLocation = gl::GetUniformLocation(defaultProgram, "linearExposure");

		defines.push_back("RENDER_BLOOM");
		bloomOnlyProgram = pvr::utils::createShaderProgram(
			assetProvider, Files::PostBloomVertShaderSrcFile, Files::PostBloomFragShaderSrcFile, nullptr, nullptr, 0, defines.data(), static_cast<uint32_t>(defines.size()));

		gl::UseProgram(bloomOnlyProgram);
		gl::Uniform1i(gl::GetUniformLocation(bloomOnlyProgram, "sBlurTexture"), 0);
	}

	/// <summary>Renders the post bloom composition pass.</summary>
	/// <param name="blurTexture">The bloomed luminance texture.</param>
	/// <param name="originalTexture">The original HDR texture.</param>
	/// <param name="samplerBilinear">The sampler object to use when sampling.</param>
	/// <param name="renderBloomOnly">Render the bloom only.</param>
	/// <param name="exposure">The exposure to use in the tone mapping.</param>
	void render(GLuint blurTexture, GLuint originalTexture, GLuint samplerBilinear, bool renderBloomOnly, float exposure)
	{
		debugThrowOnApiError("Post Bloom Pass before render");

		if (renderBloomOnly)
		{
			gl::ActiveTexture(GL_TEXTURE0);
			gl::BindTexture(GL_TEXTURE_2D, blurTexture);
			gl::BindSampler(0, samplerBilinear);

			gl::UseProgram(bloomOnlyProgram);
		}
		else
		{
			gl::ActiveTexture(GL_TEXTURE0);
			gl::BindTexture(GL_TEXTURE_2D, blurTexture);
			gl::BindSampler(0, samplerBilinear);
			gl::ActiveTexture(GL_TEXTURE1);
			gl::BindTexture(GL_TEXTURE_2D, originalTexture);
			gl::BindSampler(1, samplerBilinear);

			gl::UseProgram(defaultProgram);
			gl::Uniform1f(exposureUniformLocation, exposure);
		}

		gl::DrawArrays(GL_TRIANGLE_STRIP, 0, 3);

		debugThrowOnApiError("Post Bloom Pass after render");
	}
};

/// <summary>Class implementing the Shell functions.</summary>
class OpenGLESPostProcessing : public pvr::Shell
{
	pvr::EglContext _context;

	// Framebuffers
	Framebuffer _offScreenFramebuffer;
	Framebuffer _offScreenFramebufferUsingIMGDownsample;
	Framebuffer _blurFramebuffers[2];
	Framebuffer _computeBlurFramebuffers[2];

	// Textures
	GLuint _diffuseIrradianceTextures[NumScenes];
	GLuint _offScreenTexture;

	// Samplers
	GLuint _samplerNearest;
	GLuint _samplerBilinear;
	GLuint _samplerTrilinear;

	GLuint _depthStencilTexture;

	// UIRenderers used to display text
	pvr::ui::UIRenderer _uiRenderer;

	// Buffers
	pvr::utils::StructuredBufferView _sceneBufferView;
	GLuint _sceneBuffer;

	SkyboxPass _skyBoxPass;
	StatuePass _statuePass;
	PostBloomPass _postBloomPass;

	// Blur Passes
	GaussianBlurPass _gaussianBlurPass;
	LinearGaussianBlurPass _linearGaussianBlurPass;
	TruncatedLinearGaussianBlurPass _truncatedLinearGaussianBlurPass;

	DualFilterBlurPass _dualFilterBlurPass;
	DownAndTentFilterBlurPass _downAndTentFilterBlurPass;
	ComputeBlurPass _computeBlurPass;
	HybridGaussianBlurPass _hybridGaussianBlurPass;

	KawaseBlurPass _kawaseBlurPass;

	DownSamplePass2x2 _downsamplePass2x2;
	DownSamplePass4x4 _downsamplePass4x4;

	DownSamplePass2x2 _computeDownsamplePass2x2;
	DownSamplePass4x4 _computeDownsamplePass4x4;

	GLenum _luminanceColorFormat;
	GLenum _computeLuminanceColorFormat;
	GLenum _offscreenColorFormat;

	glm::uvec2 _blurFramebufferDimensions;
	glm::vec2 _blurInverseFramebufferDimensions;
	uint32_t _blurScale;
	uint32_t _IMGFramebufferScale;

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

	bool _isIMGFramebufferDownsampleSupported;
	bool _isBufferStorageExtSupported;

	bool _renderOnlyBloom;

	std::string _currentBlurString;

	GLenum _drawBuffers[1];
	GLenum _mrtDrawBuffers[2];

	float _exposure;
	float _threshold;

public:
	OpenGLESPostProcessing() {}

	virtual pvr::Result initApplication();
	virtual pvr::Result initView();
	virtual pvr::Result releaseView();
	virtual pvr::Result quitApplication();
	virtual pvr::Result renderFrame();

	void createSceneBuffer();
	void getDownScaleFactor(GLint& xDownscale, GLint& yDownscale);
	void createBlurFramebuffers();
	void createFramebufferAndAttachment(GLuint& framebuffer, GLuint& texture, GLenum attachmentFormat, const glm::uvec2& dimension);
	void createSamplers();
	void createOffScreenFramebuffers();
	void createUiRenderer();
	void updateBlurDescription();
	void renderUI();
	void updateDemoConfigs();
	void handleDesktopInput();
	void eventMappedInput(pvr::SimplifiedInput e);
	void updateBloomConfiguration();
	void updateAnimation();
	void updateDynamicSceneData();
};

/// <summary>Code in initApplication() will be called by Shell once per run, before the rendering context is created.
/// Used to initialize variables that are not dependent on it (e.g. external modules, loading meshes, etc.) If the rendering
/// context is lost, initApplication() will not be called again.</summary>
/// <returns>Result::Success if no error occurred.</returns>
pvr::Result OpenGLESPostProcessing::initApplication()
{
	this->setStencilBitsPerPixel(0);

	_animateObject = true;
	_animateCamera = false;
	_objectAngleY = 0.0f;
	_cameraAngle = 240.0f;
	_camera.setDistanceFromTarget(200.f);
	_camera.setHeight(-15.f);
	_blurScale = 4;
	_IMGFramebufferScale = -1;
	_logicTime = 0.0f;
	_modeSwitchTime = 0.0f;
	_isManual = false;
	_modeDuration = 1.5f;
	_currentScene = 0;
	_renderOnlyBloom = false;

	_drawBuffers[0] = GL_COLOR_ATTACHMENT0;
	_mrtDrawBuffers[0] = GL_COLOR_ATTACHMENT0;
	_mrtDrawBuffers[1] = GL_COLOR_ATTACHMENT1;

	_isIMGFramebufferDownsampleSupported = false;
	_isBufferStorageExtSupported = false;

	// Handle command line arguments including "blurmode", "blursize" and "bloom"
	const pvr::CommandLine& commandOptions = getCommandLine();
	int32_t intBloomMode = -1;
	if (commandOptions.getIntOption("-blurmode", intBloomMode))
	{
		if (intBloomMode > static_cast<int32_t>(BloomMode::NumBloomModes)) { _blurMode = BloomMode::DefaultMode; }
		else
		{
			_isManual = true;
			_blurMode = static_cast<BloomMode>(intBloomMode);
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
			_currentDemoConfiguration = intConfigSize;
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
/// Used to initialize variables that are dependent on the rendering context(e.g.textures, vertex buffers, etc.).</summary>
/// <returns>Result::Success if no error occurred.</returns>
pvr::Result OpenGLESPostProcessing::initView()
{
	_context = pvr::createEglContext();
	_context->init(getWindow(), getDisplay(), getDisplayAttributes(), pvr::Api::OpenGLES31);

	debugThrowOnApiError("InitView Begin");

	if (gl::isGlExtensionSupported("GL_KHR_debug")) { gl::ext::DebugMessageCallbackKHR(&debugCallback, NULL); }

	// Check for GL_IMG_framebuffer_downsample support
	if (gl::isGlExtensionSupported("GL_IMG_framebuffer_downsample")) { _isIMGFramebufferDownsampleSupported = true; }

	// Determine the extent of the support for GL_IMG_framebuffer_downsample
	if (_isIMGFramebufferDownsampleSupported)
	{
		GLint xDownscale, yDownscale;
		getDownScaleFactor(xDownscale, yDownscale);

		Log("Using GL_IMG_framebuffer_downsample");
		Log("Chosen Downsampling factor: %i, %i", xDownscale, yDownscale);

		_IMGFramebufferScale = static_cast<uint32_t>(xDownscale);
	}

	// Ensure the extension GL_EXT_color_buffer_float is supported
	if (!gl::isGlExtensionSupported("GL_EXT_color_buffer_float"))
	{
		setExitMessage("GL_EXT_color_buffer_float is not supported.");
		return pvr::Result::UnknownError;
	}

	// We make use of GL_EXT_buffer_storage wherever possible
	_isBufferStorageExtSupported = gl::isGlExtensionSupported("GL_EXT_buffer_storage");

	_luminanceColorFormat = GL_R16F;
	// Only a subset of formats have support for Image Load Store.
	// A subset of these also support linear filtering.
	// We use GL_RGBA16F as it has support for both
	_computeLuminanceColorFormat = GL_RGBA16F;
	_offscreenColorFormat = GL_RGBA16F;

	// calculate the frame buffer width and heights
	_blurFramebufferDimensions = glm::uvec2(this->getWidth() / _blurScale, this->getHeight() / _blurScale);
	_blurInverseFramebufferDimensions = glm::vec2(1.0f / _blurFramebufferDimensions.x, 1.0f / _blurFramebufferDimensions.y);

	// Calculates the projection matrices
	bool bRotate = isFullScreen() && isScreenRotated();
	if (bRotate)
	{
		_projectionMatrix =
			pvr::math::perspectiveFov(_context->getApiVersion(), Fov, static_cast<float>(getHeight()), static_cast<float>(getWidth()), CameraNear, CameraFar, glm::pi<float>() * .5f);
	}
	else
	{
		_projectionMatrix = pvr::math::perspectiveFov(_context->getApiVersion(), Fov, static_cast<float>(getWidth()), static_cast<float>(getHeight()), CameraNear, CameraFar);
	}

	// create demo buffers
	createSceneBuffer();

	for (uint32_t i = 0; i < NumScenes; ++i) { _diffuseIrradianceTextures[i] = pvr::utils::textureUpload(*this, SceneTexFileNames[i].diffuseIrradianceMapTexture); }

	// Creates the offscreen framebuffers along with their attachments
	// The framebuffers and images can then be "ping-ponged" between when applying various filters/blurs
	// Pass 1: Read From 1, Render to 0
	// Pass 2: Read From 0, Render to 1
	createOffScreenFramebuffers();

	// Create the samplers used for various texture sampling
	createSamplers();

	_statuePass.init(*this, _isBufferStorageExtSupported);
	_skyBoxPass.init(*this);

	createBlurFramebuffers();

	// Create the downsample passes
	_downsamplePass2x2.init(*this, _blurFramebuffers[1].attachments[static_cast<uint32_t>(BloomAttachments::Bloom)], _blurFramebufferDimensions);
	_downsamplePass4x4.init(
		*this, _blurFramebuffers[1].attachments[static_cast<uint32_t>(BloomAttachments::Bloom)], _blurFramebufferDimensions, glm::uvec2(this->getWidth(), this->getHeight()));

	_computeDownsamplePass2x2.init(*this, _computeBlurFramebuffers[1].attachments[static_cast<uint32_t>(BloomAttachments::Bloom)], _blurFramebufferDimensions);
	_computeDownsamplePass4x4.init(*this, _computeBlurFramebuffers[1].attachments[static_cast<uint32_t>(BloomAttachments::Bloom)], _blurFramebufferDimensions,
		glm::uvec2(this->getWidth(), this->getHeight()));

	_postBloomPass.init(*this, getBackBufferColorspace() == pvr::ColorSpace::sRGB);

	// Initialise the Blur Passes
	// Gaussian Blurs
	{
		_gaussianBlurPass.init(*this, _blurFramebufferDimensions);
		_linearGaussianBlurPass.init(*this, _blurFramebufferDimensions);
		_truncatedLinearGaussianBlurPass.init(*this, _blurFramebufferDimensions);
		_computeBlurPass.init(*this, _blurFramebufferDimensions);
		_hybridGaussianBlurPass.init(&_computeBlurPass, &_truncatedLinearGaussianBlurPass);
	}

	// Kawase Blur
	{
		_kawaseBlurPass.init(*this, _blurFramebufferDimensions);
	}

	// Dual Filter Blur
	{
		_dualFilterBlurPass.init(*this, _luminanceColorFormat, glm::uvec2(this->getWidth(), this->getHeight()), getBackBufferColorspace() == pvr::ColorSpace::sRGB);
	}

	// Down Sample and Tent filter blur pass
	{
		_downAndTentFilterBlurPass.init(*this, _luminanceColorFormat, glm::uvec2(this->getWidth(), this->getHeight()), getBackBufferColorspace() == pvr::ColorSpace::sRGB);
	}

	// Update the demo configuration
	updateDemoConfigs();

	// initialise the UI Renderers
	createUiRenderer();

	// Set basic default state
	gl::BindFramebuffer(GL_FRAMEBUFFER, _context->getOnScreenFbo());
	gl::UseProgram(0);

	gl::Disable(GL_BLEND);
	gl::Disable(GL_STENCIL_TEST);

	gl::Enable(GL_DEPTH_TEST); // depth test
	gl::DepthMask(GL_TRUE); // depth write enabled
	gl::DepthFunc(GL_LESS);

	gl::Enable(GL_CULL_FACE);
	gl::CullFace(GL_FRONT);
	gl::FrontFace(GL_CW);

	gl::ClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	gl::ClearDepthf(1.0f);
	gl::ClearStencil(0);

	return pvr::Result::Success;
}

/// <summary>Main rendering loop function of the program. The shell will call this function every frame</summary>
/// <returns>Result::Success if no error occurred.</summary>
pvr::Result OpenGLESPostProcessing::renderFrame()
{
	debugThrowOnApiError("Frame begin");

	handleDesktopInput();

	// update dynamic buffers
	updateDynamicSceneData();

	// Set the viewport for full screen rendering
	gl::Viewport(0, 0, static_cast<GLsizei>(getWidth()), static_cast<GLsizei>(getHeight()));

	// Bind the offscreen framebuffer appropriately
	// Note that the DualFilter and TentFilter take care of their own downsampling
	if (_blurMode == BloomMode::DualFilter || _blurMode == BloomMode::TentFilter) { gl::BindFramebuffer(GL_DRAW_FRAMEBUFFER, _offScreenFramebuffer.framebuffer); }
	else
	{
		// Make use of IMG_framebuffer_downsample extension
		if (_isIMGFramebufferDownsampleSupported) { gl::BindFramebuffer(GL_DRAW_FRAMEBUFFER, _offScreenFramebufferUsingIMGDownsample.framebuffer); }
		else
		{
			gl::BindFramebuffer(GL_DRAW_FRAMEBUFFER, _offScreenFramebuffer.framebuffer);
		}
	}

	// The scene rendering requires the use of 2 draw buffers
	// 1. The offscreen texture
	// 2. The Luminance colour buffer
	gl::DrawBuffers(2, _mrtDrawBuffers);

	// Clear the colour and depth
	gl::Clear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Perform Scene rendering
	gl::CullFace(GL_BACK);
	gl::FrontFace(GL_CCW);
	gl::Enable(GL_DEPTH_TEST);
	gl::DepthFunc(GL_LESS);
	_statuePass.render(_diffuseIrradianceTextures[_currentScene], _samplerTrilinear, _samplerTrilinear, _exposure, _threshold);

	gl::DepthFunc(GL_LEQUAL);
	_skyBoxPass.render(_sceneBuffer, static_cast<GLsizeiptr>(_sceneBufferView.getSize()), _samplerTrilinear, _exposure, _threshold, _currentScene);

	// Disable depth testing, from this point onwards we don't need depth
	gl::Disable(GL_DEPTH_TEST);

	{
		std::vector<GLenum> invalidateAttachments;
		invalidateAttachments.push_back(GL_DEPTH_ATTACHMENT);
		invalidateAttachments.push_back(GL_STENCIL_ATTACHMENT);
		gl::InvalidateFramebuffer(GL_FRAMEBUFFER, (GLsizei)invalidateAttachments.size(), &invalidateAttachments[0]);
	}

	// Set draw buffers for rendering to only a single attachment
	gl::DrawBuffers(1, _drawBuffers);

	// Perform a downsample if the bloom mode is not DualFilter or TentFilter
	if (!(_blurMode == BloomMode::DualFilter || _blurMode == BloomMode::TentFilter))
	{
		// If isIMGFramebufferDownsampleSupported is supported a native 2x2 downsample must be supported which we have made use of in the previous pass.
		// This means that at this point _downSampledLuminanceTexture already contains a downsampled image (1/4 resolution) but if the scale is 2x2 then
		// we still need to perform another downsample of the image to get it into our desired 1/16 resolution.
		if (_isIMGFramebufferDownsampleSupported && _IMGFramebufferScale == 2)
		{
			if (_blurMode == BloomMode::Compute || _blurMode == BloomMode::HybridGaussian)
			{
				_computeDownsamplePass2x2.render(
					_offScreenFramebufferUsingIMGDownsample.attachments[static_cast<uint32_t>(OffscreenWithIMGFramebufferDownsampleAttachments::DownsampledLuminance)], _samplerBilinear);
			}
			else
			{
				_downsamplePass2x2.render(
					_offScreenFramebufferUsingIMGDownsample.attachments[static_cast<uint32_t>(OffscreenWithIMGFramebufferDownsampleAttachments::DownsampledLuminance)], _samplerBilinear);
			}
		}
		// If IMG_framebuffer_downsample is not supported then just do a 1/4 x 1/4 downsample
		else
		{
			if (_blurMode == BloomMode::Compute || _blurMode == BloomMode::HybridGaussian)
			{
				_computeDownsamplePass4x4.render(
					_offScreenFramebuffer.attachments[static_cast<uint32_t>(OffscreenWithIMGFramebufferDownsampleAttachments::DownsampledLuminance)], _samplerBilinear);
			}
			else
			{
				_downsamplePass4x4.render(_offScreenFramebuffer.attachments[static_cast<uint32_t>(OffscreenAttachments::Luminance)], _samplerBilinear);
			}
		}
	}

	if (_blurMode != BloomMode::NoBloom)
	{
		// Render the bloom
		switch (_blurMode)
		{
		case (BloomMode::GaussianOriginal):
		{
			_gaussianBlurPass.render(_blurFramebuffers[1].attachments[static_cast<uint32_t>(BloomAttachments::Bloom)], _blurFramebuffers[0], _blurFramebuffers[1], _samplerNearest);
			break;
		}

		case (BloomMode::GaussianLinear):
		{
			_linearGaussianBlurPass.render(_blurFramebuffers[1].attachments[static_cast<uint32_t>(BloomAttachments::Bloom)], _blurFramebuffers[0], _blurFramebuffers[1], _samplerBilinear);
			break;
		}
		case (BloomMode::GaussianLinearTruncated):
		{
			_truncatedLinearGaussianBlurPass.render(
				_blurFramebuffers[1].attachments[static_cast<uint32_t>(BloomAttachments::Bloom)], _blurFramebuffers[0], _blurFramebuffers[1], _samplerBilinear);
			break;
		}
		case (BloomMode::Compute):
		{
			_computeBlurPass.render(_computeBlurFramebuffers[1].attachments[static_cast<uint32_t>(BloomAttachments::Bloom)], _computeBlurFramebuffers[0],
				_computeBlurFramebuffers[1], _computeLuminanceColorFormat);
			break;
		}
		case (BloomMode::Kawase):
		{
			_kawaseBlurPass.render(_blurFramebuffers[1].attachments[static_cast<uint32_t>(BloomAttachments::Bloom)], _blurFramebuffers, 2, _samplerBilinear);
			break;
		}
		case (BloomMode::DualFilter):
		{
			_dualFilterBlurPass.render(_offScreenFramebuffer.attachments[static_cast<uint32_t>(OffscreenAttachments::Luminance)], _offScreenTexture, _context->getOnScreenFbo(),
				_samplerBilinear, _renderOnlyBloom, _exposure);
			break;
		}
		case (BloomMode::TentFilter):
		{
			_downAndTentFilterBlurPass.render(_offScreenFramebuffer.attachments[static_cast<uint32_t>(OffscreenAttachments::Luminance)], _offScreenTexture,
				_context->getOnScreenFbo(), _samplerBilinear, _renderOnlyBloom, _exposure);
			break;
		}
		case (BloomMode::HybridGaussian):
		{
			_hybridGaussianBlurPass.render(_computeBlurFramebuffers[1].attachments[static_cast<uint32_t>(BloomAttachments::Bloom)], _computeBlurFramebuffers[0],
				_computeBlurFramebuffers[1], _samplerBilinear, _computeLuminanceColorFormat);
			break;
		}
		default: throw pvr::UnsupportedOperationError("Unsupported BlurMode.");
		}
	}

	// If Dual or Tent filter then the composition is taken care of during the final up sample
	if (_blurMode != BloomMode::DualFilter && _blurMode != BloomMode::TentFilter)
	{
		GLuint blurredTexture = static_cast<GLuint>(-1);

		// Ensure the post bloom pass uses the correct blurred image for the current blur mode
		switch (_blurMode)
		{
		case (BloomMode::GaussianOriginal):
		{
			blurredTexture = _blurFramebuffers[1].attachments[static_cast<uint32_t>(BloomAttachments::Bloom)];
			break;
		}
		case (BloomMode::GaussianLinear):
		{
			blurredTexture = _blurFramebuffers[1].attachments[static_cast<uint32_t>(BloomAttachments::Bloom)];
			break;
		}
		case (BloomMode::GaussianLinearTruncated):
		{
			blurredTexture = _blurFramebuffers[1].attachments[static_cast<uint32_t>(BloomAttachments::Bloom)];
			break;
		}
		case (BloomMode::Compute):
		{
			blurredTexture = _computeBlurFramebuffers[1].attachments[static_cast<uint32_t>(BloomAttachments::Bloom)];
			break;
		}
		case (BloomMode::Kawase):
		{
			blurredTexture = _blurFramebuffers[_kawaseBlurPass.getBlurredImageIndex()].attachments[static_cast<uint32_t>(BloomAttachments::Bloom)];
			break;
		}
		case (BloomMode::HybridGaussian):
		{
			blurredTexture = _computeBlurFramebuffers[1].attachments[static_cast<uint32_t>(BloomAttachments::Bloom)];
			break;
		}
		case (BloomMode::DualFilter):
		{
			blurredTexture = _dualFilterBlurPass.getBlurredTexture();
			break;
		}
		case (BloomMode::TentFilter):
		{
			blurredTexture = _downAndTentFilterBlurPass.getBlurredTexture();
			break;
		}
		case (BloomMode::NoBloom):
		{
			blurredTexture = _blurFramebuffers[1].attachments[static_cast<uint32_t>(BloomAttachments::Bloom)];
			break;
		}
		default: throw pvr::UnsupportedOperationError("Unsupported BlurMode.");
		}

		gl::Viewport(0, 0, static_cast<GLsizei>(getWidth()), static_cast<GLsizei>(getHeight()));

		gl::BindFramebuffer(GL_DRAW_FRAMEBUFFER, _context->getOnScreenFbo());
		gl::Clear(GL_COLOR_BUFFER_BIT);
		_postBloomPass.render(blurredTexture, _offScreenTexture, _samplerBilinear, _renderOnlyBloom, _exposure);
	}

	renderUI();

	{
		std::vector<GLenum> invalidateAttachments;
		invalidateAttachments.push_back(GL_DEPTH);
		invalidateAttachments.push_back(GL_STENCIL);
		gl::InvalidateFramebuffer(GL_FRAMEBUFFER, (GLsizei)invalidateAttachments.size(), &invalidateAttachments[0]);
	}

	debugThrowOnApiError("Frame end");

	if (this->shouldTakeScreenshot()) { pvr::utils::takeScreenshot(this->getScreenshotFileName(), this->getWidth(), this->getHeight()); }

	_context->swapBuffers();

	return pvr::Result::Success;
}

/// <summary>Code in releaseView() will be called by Shell when the application quits or before a change in the rendering context.</summary>
/// <returns>Result::Success if no error occurred.</returns>
pvr::Result OpenGLESPostProcessing::releaseView() { return pvr::Result::Success; }

/// <summary>Code in quitApplication() will be called by Shell once per run, just before exiting the program.
/// quitApplication() will not be called every time the rendering context is lost, only before application exit.</summary>
/// <returns>Result::Success if no error occurred.</returns>
pvr::Result OpenGLESPostProcessing::quitApplication() { return pvr::Result::Success; }

/// <summary>Creates The UI renderer.</summary>
void OpenGLESPostProcessing::createUiRenderer()
{
	_uiRenderer.init(getWidth(), getHeight(), isFullScreen(), getBackBufferColorspace() == pvr::ColorSpace::sRGB);

	_uiRenderer.getDefaultTitle()->setText("PostProcessing");
	_uiRenderer.getDefaultTitle()->commitUpdates();
	_uiRenderer.getDefaultControls()->setText("Left / right: Blur Mode\n"
											  "Up / Down: Blur Size\n"
											  "Action 1: Enable/Disable Bloom\n"
											  "Action 2: Enable/Disable Animation\n"
											  "Action 3: Change Scene\n");
	_uiRenderer.getDefaultControls()->commitUpdates();

	updateBlurDescription();
	_uiRenderer.getDefaultDescription()->setText(_currentBlurString);
	_uiRenderer.getDefaultDescription()->commitUpdates();

	debugThrowOnApiError("createUiRenderer");
}

/// <summary>Updates the description for the currently used blur technique.</summary>
void OpenGLESPostProcessing::updateBlurDescription()
{
	switch (_blurMode)
	{
	case (BloomMode::NoBloom):
	{
		_currentBlurString = BloomStrings[static_cast<uint32_t>(_blurMode)];
		break;
	}
	case (BloomMode::GaussianOriginal):
	{
		uint32_t numSamples = static_cast<uint32_t>(_gaussianBlurPass.gaussianOffsets[_currentDemoConfiguration].size());
		_currentBlurString = BloomStrings[static_cast<uint32_t>(_blurMode)] + "\n" +
			pvr::strings::createFormatted("Kernel Size = %u (%u + %u taps)", DemoConfigurations::Configurations[_currentDemoConfiguration].gaussianConfig, numSamples, numSamples);
		break;
	}
	case (BloomMode::GaussianLinear):
	{
		uint32_t numSamples = static_cast<uint32_t>(_linearGaussianBlurPass.gaussianOffsets[_currentDemoConfiguration].size());
		_currentBlurString = BloomStrings[static_cast<uint32_t>(_blurMode)] + "\n" +
			pvr::strings::createFormatted("Kernel Size = %u (%u + %u taps)", DemoConfigurations::Configurations[_currentDemoConfiguration].linearGaussianConfig, numSamples, numSamples);
		break;
	}
	case (BloomMode::GaussianLinearTruncated):
	{
		uint32_t numSamples = static_cast<uint32_t>(_truncatedLinearGaussianBlurPass.gaussianOffsets[_currentDemoConfiguration].size());
		_currentBlurString = BloomStrings[static_cast<uint32_t>(_blurMode)] + "\n" +
			pvr::strings::createFormatted(
				"Kernel Size = %u (%u + %u taps)", DemoConfigurations::Configurations[_currentDemoConfiguration].truncatedLinearGaussianConfig, numSamples, numSamples);
		break;
	}
	case (BloomMode::Compute):
	{
		uint32_t numSamples = static_cast<uint32_t>(_computeBlurPass.gaussianOffsets[_currentDemoConfiguration].size());
		_currentBlurString = BloomStrings[static_cast<uint32_t>(_blurMode)] + "\n" +
			pvr::strings::createFormatted(
				"Kernel Size = %u (Sliding Average)", DemoConfigurations::Configurations[_currentDemoConfiguration].computeGaussianConfig, numSamples, numSamples);
		break;
	}
	case (BloomMode::DualFilter):
	{
		uint32_t numSamples = _dualFilterBlurPass.blurIterations / 2;
		_currentBlurString = BloomStrings[static_cast<uint32_t>(_blurMode)] + "\n" +
			pvr::strings::createFormatted(
				"Iterations = %u (%u Downsamples, %u Upsamples)", DemoConfigurations::Configurations[_currentDemoConfiguration].dualFilterConfig, numSamples, numSamples);
		break;
	}
	case (BloomMode::TentFilter):
	{
		uint32_t numSamples = _downAndTentFilterBlurPass.blurIterations / 2;
		_currentBlurString = BloomStrings[static_cast<uint32_t>(_blurMode)] + "\n" +
			pvr::strings::createFormatted(
				"Iterations = %u (%u Downsamples, %u Upsamples)", DemoConfigurations::Configurations[_currentDemoConfiguration].tentFilterConfig, numSamples, numSamples);
		break;
	}
	case (BloomMode::HybridGaussian):
	{
		uint32_t numComputeSamples = static_cast<uint32_t>(_hybridGaussianBlurPass.computeBlurPass->gaussianOffsets[_currentDemoConfiguration].size());
		uint32_t numLinearSamples = static_cast<uint32_t>(_hybridGaussianBlurPass.linearBlurPass->gaussianOffsets[_currentDemoConfiguration].size());
		_currentBlurString = BloomStrings[static_cast<uint32_t>(_blurMode)] + "\n" +
			pvr::strings::createFormatted("Horizontal Compute %u taps, Vertical Linear Gaussian %u taps)", numComputeSamples, numLinearSamples);
		break;
	}
	case (BloomMode::Kawase):
	{
		std::string kernelString = "";
		uint32_t numIterations = _kawaseBlurPass.blurIterations;

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
void OpenGLESPostProcessing::createSceneBuffer()
{
	pvr::utils::StructuredMemoryDescription desc;
	desc.addElement(BufferEntryNames::Scene::InverseViewProjectionMatrix, pvr::GpuDatatypes::mat4x4);
	desc.addElement(BufferEntryNames::Scene::EyePosition, pvr::GpuDatatypes::vec3);

	_sceneBufferView.init(desc);

	gl::GenBuffers(1, &_sceneBuffer);
	gl::BindBuffer(GL_UNIFORM_BUFFER, _sceneBuffer);
	gl::BufferData(GL_UNIFORM_BUFFER, static_cast<GLsizeiptr>(_sceneBufferView.getSize()), nullptr, GL_DYNAMIC_DRAW);

	// if GL_EXT_buffer_storage is supported then map the buffer upfront and never unmap it
	if (_isBufferStorageExtSupported)
	{
		gl::BindBuffer(GL_COPY_READ_BUFFER, _sceneBuffer);
		gl::ext::BufferStorageEXT(GL_COPY_READ_BUFFER, (GLsizei)_sceneBufferView.getSize(), 0, GL_MAP_WRITE_BIT_EXT | GL_MAP_PERSISTENT_BIT_EXT | GL_MAP_COHERENT_BIT_EXT);

		void* memory = gl::MapBufferRange(
			GL_COPY_READ_BUFFER, 0, static_cast<GLsizeiptr>(_sceneBufferView.getSize()), GL_MAP_WRITE_BIT_EXT | GL_MAP_PERSISTENT_BIT_EXT | GL_MAP_COHERENT_BIT_EXT);
		_sceneBufferView.pointToMappedMemory(memory);
	}
}

/// <summary>Creates the various samplers used throughout the demo.</summary>
void OpenGLESPostProcessing::createSamplers()
{
	gl::GenSamplers(1, &_samplerTrilinear);
	gl::GenSamplers(1, &_samplerBilinear);
	gl::GenSamplers(1, &_samplerNearest);

	gl::SamplerParameteri(_samplerTrilinear, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	gl::SamplerParameteri(_samplerTrilinear, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	gl::SamplerParameteri(_samplerTrilinear, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	gl::SamplerParameteri(_samplerTrilinear, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	gl::SamplerParameteri(_samplerTrilinear, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	gl::SamplerParameteri(_samplerBilinear, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	gl::SamplerParameteri(_samplerBilinear, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	gl::SamplerParameteri(_samplerBilinear, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	gl::SamplerParameteri(_samplerBilinear, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	gl::SamplerParameteri(_samplerBilinear, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	gl::SamplerParameteri(_samplerNearest, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	gl::SamplerParameteri(_samplerNearest, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	gl::SamplerParameteri(_samplerNearest, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	gl::SamplerParameteri(_samplerNearest, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	gl::SamplerParameteri(_samplerNearest, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	debugThrowOnApiError("createSamplers");
}

void OpenGLESPostProcessing::createFramebufferAndAttachment(GLuint& framebuffer, GLuint& texture, GLenum attachmentFormat, const glm::uvec2& dimension)
{
	gl::GenTextures(1, &texture);
	gl::BindTexture(GL_TEXTURE_2D, texture);
	gl::TexStorage2D(GL_TEXTURE_2D, 1, attachmentFormat, static_cast<GLsizei>(dimension.x), static_cast<GLsizei>(dimension.y));

	gl::GenFramebuffers(1, &framebuffer);
	gl::BindFramebuffer(GL_DRAW_FRAMEBUFFER, framebuffer);
	gl::FramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture, 0);
	gl::FramebufferParameteri(GL_DRAW_FRAMEBUFFER, GL_FRAMEBUFFER_DEFAULT_WIDTH, static_cast<GLint>(dimension.x));
	gl::FramebufferParameteri(GL_DRAW_FRAMEBUFFER, GL_FRAMEBUFFER_DEFAULT_HEIGHT, static_cast<GLint>(dimension.y));

	pvr::utils::checkFboStatus();
}

/// <summary>Create the framebuffers which will be used in the various bloom passes.</summary>
void OpenGLESPostProcessing::createBlurFramebuffers()
{
	for (uint32_t i = 0; i < 2; ++i)
	{
		// Create the downsized resolution framebuffer used for rendering
		{
			_blurFramebuffers[i].attachments.resize(1);

			createFramebufferAndAttachment(_blurFramebuffers[i].framebuffer, _blurFramebuffers[i].attachments[static_cast<uint32_t>(BloomAttachments::Bloom)],
				_luminanceColorFormat, _blurFramebufferDimensions);

			_blurFramebuffers[i].dimensions.x = _blurFramebufferDimensions.x;
			_blurFramebuffers[i].dimensions.y = _blurFramebufferDimensions.y;
		}

		// Create the downsized resolution framebuffer used for compute tasks
		// This is necessary as _computeLuminanceColorFormat does not match _luminanceColorFormat. The format selected for storing the luminance values
		// is not supported for image load store so we use a larger sized image format
		{
			_computeBlurFramebuffers[i].attachments.resize(1);

			createFramebufferAndAttachment(_computeBlurFramebuffers[i].framebuffer, _computeBlurFramebuffers[i].attachments[static_cast<uint32_t>(BloomAttachments::Bloom)],
				_computeLuminanceColorFormat, _blurFramebufferDimensions);

			_computeBlurFramebuffers[i].dimensions.x = _blurFramebufferDimensions.x;
			_computeBlurFramebuffers[i].dimensions.y = _blurFramebufferDimensions.y;
		}
		debugThrowOnApiError("createBlurFramebuffers init");
	}
}

/// <summary>Determine the maximum down scale factor supported by the GL_IMG_framebuffer_downsample extension.</summary>
/// <param name="xDownscale">The maximum x downscale factor.</param>
/// <param name="yDownscale">The maximum y downscale factor.</param>
void OpenGLESPostProcessing::getDownScaleFactor(GLint& xDownscale, GLint& yDownscale)
{
	Log("Supported Downsampling factors:");

	xDownscale = 1;
	yDownscale = 1;

	// Query the number of available scales
	GLint numScales;
	gl::GetIntegerv(GL_NUM_DOWNSAMPLE_SCALES_IMG, &numScales);

	// 2 scale modes are supported as minimum, so only need to check for
	// better than 2x2 if more modes are exposed.
	if (numScales > 2)
	{
		// Try to select most aggressive scaling.
		GLint bestScale = 1;
		GLint tempScale[2];
		GLint i;
		for (i = 0; i < numScales; ++i)
		{
			gl::GetIntegeri_v(GL_DOWNSAMPLE_SCALES_IMG, i, tempScale);

			Log("	Downsampling factor: %i, %i", tempScale[0], tempScale[1]);

			// If the scaling is more aggressive, update our x/y scale values.
			if (tempScale[0] * tempScale[1] > bestScale)
			{
				xDownscale = tempScale[0];
				yDownscale = tempScale[1];
			}
		}
	}
	else
	{
		xDownscale = 2;
		yDownscale = 2;
	}
}

/// <summary>Create the offscreen framebuffers and various attachments used in the application.</summary>
void OpenGLESPostProcessing::createOffScreenFramebuffers()
{
	// Offscreen texture
	gl::GenTextures(1, &_offScreenTexture);
	gl::BindTexture(GL_TEXTURE_2D, _offScreenTexture);
	gl::TexStorage2D(GL_TEXTURE_2D, 1, _offscreenColorFormat, static_cast<GLsizei>(getWidth()), static_cast<GLsizei>(getHeight()));
	debugThrowOnApiError("createOffScreenFramebuffers - created offscreen colour texture");

	gl::GenTextures(1, &_depthStencilTexture);
	gl::BindTexture(GL_TEXTURE_2D, _depthStencilTexture);
	gl::TexStorage2D(GL_TEXTURE_2D, 1, GL_DEPTH24_STENCIL8, static_cast<GLsizei>(getWidth()), static_cast<GLsizei>(getHeight()));
	debugThrowOnApiError("createOffScreenFramebuffers - created depth stencil texture");

	// Make use of the previously created textures
	_offScreenFramebuffer.attachments.resize(static_cast<uint32_t>(OffscreenAttachments::NumAttachments));
	_offScreenFramebuffer.attachments[static_cast<uint32_t>(OffscreenAttachments::Offscreen)] = _offScreenTexture;
	_offScreenFramebuffer.attachments[static_cast<uint32_t>(OffscreenAttachments::DepthStencil)] = _depthStencilTexture;

	// Full size luminance texture
	gl::GenTextures(1, &_offScreenFramebuffer.attachments[static_cast<uint32_t>(OffscreenAttachments::Luminance)]);
	gl::BindTexture(GL_TEXTURE_2D, _offScreenFramebuffer.attachments[static_cast<uint32_t>(OffscreenAttachments::Luminance)]);
	gl::TexStorage2D(GL_TEXTURE_2D, 1, _luminanceColorFormat, static_cast<GLsizei>(getWidth()), static_cast<GLsizei>(getHeight()));
	debugThrowOnApiError("createOffScreenFramebuffers - created full size luminance texture");

	// Create the offscreen framebuffer
	gl::GenFramebuffers(1, &_offScreenFramebuffer.framebuffer);
	gl::BindFramebuffer(GL_DRAW_FRAMEBUFFER, _offScreenFramebuffer.framebuffer);
	gl::FramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, _offScreenFramebuffer.attachments[static_cast<uint32_t>(OffscreenAttachments::Offscreen)], 0);
	gl::FramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, _offScreenFramebuffer.attachments[static_cast<uint32_t>(OffscreenAttachments::Luminance)], 0);
	gl::FramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, _offScreenFramebuffer.attachments[static_cast<uint32_t>(OffscreenAttachments::DepthStencil)], 0);

	_offScreenFramebuffer.dimensions.x = getWidth();
	_offScreenFramebuffer.dimensions.y = getHeight();
	gl::FramebufferParameteri(GL_DRAW_FRAMEBUFFER, GL_FRAMEBUFFER_DEFAULT_WIDTH, _offScreenFramebuffer.dimensions.x);
	gl::FramebufferParameteri(GL_DRAW_FRAMEBUFFER, GL_FRAMEBUFFER_DEFAULT_HEIGHT, _offScreenFramebuffer.dimensions.y);

	debugThrowOnApiError("createOffScreenNoDownsampleFramebuffers - created offscreen Framebuffer");
	pvr::utils::checkFboStatus();

	// Fbo used for the offscreen rendering
	if (_isIMGFramebufferDownsampleSupported)
	{
		// Make use of the previously created textures
		_offScreenFramebufferUsingIMGDownsample.attachments.resize(static_cast<uint32_t>(OffscreenWithIMGFramebufferDownsampleAttachments::NumAttachments));
		_offScreenFramebuffer.attachments[static_cast<uint32_t>(OffscreenWithIMGFramebufferDownsampleAttachments::Offscreen)] = _offScreenTexture;
		_offScreenFramebuffer.attachments[static_cast<uint32_t>(OffscreenWithIMGFramebufferDownsampleAttachments::DepthStencil)] = _depthStencilTexture;

		// Create the downsampled luminance texture
		gl::GenTextures(1, &_offScreenFramebufferUsingIMGDownsample.attachments[static_cast<uint32_t>(OffscreenWithIMGFramebufferDownsampleAttachments::DownsampledLuminance)]);
		gl::BindTexture(
			GL_TEXTURE_2D, _offScreenFramebufferUsingIMGDownsample.attachments[static_cast<uint32_t>(OffscreenWithIMGFramebufferDownsampleAttachments::DownsampledLuminance)]);
		gl::TexStorage2D(GL_TEXTURE_2D, 1, _luminanceColorFormat, getWidth() / _IMGFramebufferScale, getHeight() / _IMGFramebufferScale);
		debugThrowOnApiError("createOffScreenFramebuffers - created downsample luminance texture");

		// Create the offscreen framebuffer which makes use of IMG_framebuffer_downsample for downsampling the luminance directly
		gl::GenFramebuffers(1, &_offScreenFramebufferUsingIMGDownsample.framebuffer);
		gl::BindFramebuffer(GL_DRAW_FRAMEBUFFER, _offScreenFramebufferUsingIMGDownsample.framebuffer);
		gl::FramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
			_offScreenFramebuffer.attachments[static_cast<uint32_t>(OffscreenWithIMGFramebufferDownsampleAttachments::Offscreen)], 0);

		// Make use of IMG_framebuffer_downsample and attach the lower resolution luminance texture
		gl::ext::FramebufferTexture2DDownsampleIMG(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D,
			_offScreenFramebufferUsingIMGDownsample.attachments[static_cast<uint32_t>(OffscreenWithIMGFramebufferDownsampleAttachments::DownsampledLuminance)], 0u,
			static_cast<GLint>(_IMGFramebufferScale), static_cast<GLint>(_IMGFramebufferScale));
		gl::FramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D,
			_offScreenFramebuffer.attachments[static_cast<uint32_t>(OffscreenWithIMGFramebufferDownsampleAttachments::DepthStencil)], 0);

		_offScreenFramebufferUsingIMGDownsample.dimensions.x = getWidth();
		_offScreenFramebufferUsingIMGDownsample.dimensions.y = getHeight();
		gl::FramebufferParameteri(GL_DRAW_FRAMEBUFFER, GL_FRAMEBUFFER_DEFAULT_WIDTH, static_cast<GLint>(_offScreenFramebufferUsingIMGDownsample.dimensions.x));
		gl::FramebufferParameteri(GL_DRAW_FRAMEBUFFER, GL_FRAMEBUFFER_DEFAULT_HEIGHT, static_cast<GLint>(_offScreenFramebufferUsingIMGDownsample.dimensions.y));

		debugThrowOnApiError("createOffScreenFramebuffers - created offscreen Framebuffer");
		pvr::utils::checkFboStatus();
	}

	debugThrowOnApiError("createOffScreenFramebuffers");
}

/// <summary>Update the various dynamic scene data used in the application.</summary>
void OpenGLESPostProcessing::updateDynamicSceneData()
{
	// Update object animations
	updateAnimation();

	_exposure = SceneTexFileNames[_currentScene].getLinearExposure();
	_threshold = SceneTexFileNames[_currentScene].threshold;

	// Update the animation data used in the statue pass
	_statuePass.updateAnimation(_objectAngleY, _viewProjectionMatrix);

	{
		void* mappedMemory = nullptr;
		if (!_isBufferStorageExtSupported)
		{
			gl::BindBuffer(GL_UNIFORM_BUFFER, _sceneBuffer);
			mappedMemory = gl::MapBufferRange(GL_UNIFORM_BUFFER, 0, static_cast<GLsizeiptr>(_sceneBufferView.getSize()), GL_MAP_WRITE_BIT);
			_sceneBufferView.pointToMappedMemory(mappedMemory);
		}

		_sceneBufferView.getElementByName(BufferEntryNames::Scene::InverseViewProjectionMatrix).setValue(glm::inverse(_viewProjectionMatrix));
		_sceneBufferView.getElementByName(BufferEntryNames::Scene::EyePosition).setValue(_camera.getCameraPosition());

		if (!_isBufferStorageExtSupported) { gl::UnmapBuffer(GL_UNIFORM_BUFFER); }
	}
}

/// <summary>Update the animations for the current frame.</summary>
void OpenGLESPostProcessing::updateAnimation()
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
void OpenGLESPostProcessing::updateDemoConfigs()
{
	switch (_blurMode)
	{
	case (BloomMode::GaussianOriginal):
	{
		_gaussianBlurPass.updateKernelConfig(_currentDemoConfiguration);
		break;
	}
	case (BloomMode::GaussianLinear):
	{
		_linearGaussianBlurPass.updateKernelConfig(_currentDemoConfiguration);
		break;
	}
	case (BloomMode::GaussianLinearTruncated):
	{
		_truncatedLinearGaussianBlurPass.updateKernelConfig(_currentDemoConfiguration);
		break;
	}
	case (BloomMode::Kawase):
	{
		_kawaseBlurPass.updateConfig(DemoConfigurations::Configurations[_currentDemoConfiguration].kawaseConfig.kernel,
			DemoConfigurations::Configurations[_currentDemoConfiguration].kawaseConfig.numIterations);
		break;
	}
	case (BloomMode::Compute):
	{
		_computeBlurPass.updateKernelConfig(_currentDemoConfiguration);
		break;
	}
	case (BloomMode::DualFilter):
	{
		_dualFilterBlurPass.updateConfig(DemoConfigurations::Configurations[_currentDemoConfiguration].dualFilterConfig);
		break;
	}
	case (BloomMode::TentFilter):
	{
		_downAndTentFilterBlurPass.updateConfig(DemoConfigurations::Configurations[_currentDemoConfiguration].dualFilterConfig);
		break;
	}
	case (BloomMode::HybridGaussian):
	{
		_truncatedLinearGaussianBlurPass.updateKernelConfig(_currentDemoConfiguration);
		_computeBlurPass.updateKernelConfig(_currentDemoConfiguration);
		break;
	}
	default: break;
	}
	debugThrowOnApiError("updateDemoConfigs");
}

/// <summary>Update the bloom configuration.</summary>
void OpenGLESPostProcessing::updateBloomConfiguration()
{
	updateDemoConfigs();

	updateBlurDescription();
	_uiRenderer.getDefaultDescription()->setText(_currentBlurString);
	_uiRenderer.getDefaultDescription()->commitUpdates();
}

void OpenGLESPostProcessing::handleDesktopInput()
{
#ifdef PVR_PLATFORM_IS_DESKTOP
	if (isKeyPressed(pvr::Keys::PageDown)) { SceneTexFileNames[_currentScene].keyValue *= .85f; }
	if (isKeyPressed(pvr::Keys::PageUp)) { SceneTexFileNames[_currentScene].keyValue *= 1.15f; }

	SceneTexFileNames[_currentScene].keyValue = glm::clamp(SceneTexFileNames[_currentScene].keyValue, 0.001f, 100.0f);

	if (isKeyPressed(pvr::Keys::SquareBracketLeft)) { SceneTexFileNames[_currentScene].threshold -= 0.05f; }
	if (isKeyPressed(pvr::Keys::SquareBracketRight)) { SceneTexFileNames[_currentScene].threshold += 0.05f; }

	SceneTexFileNames[_currentScene].threshold = glm::clamp(SceneTexFileNames[_currentScene].threshold, 0.05f, 20.0f);
#endif
}

/// <summary>Handles user input and updates live variables accordingly.</summary>
void OpenGLESPostProcessing::eventMappedInput(pvr::SimplifiedInput e)
{
	switch (e)
	{
	case pvr::SimplifiedInput::Up:
	{
		_currentDemoConfiguration = (_currentDemoConfiguration + 1) % DemoConfigurations::NumDemoConfigurations;
		updateBloomConfiguration();
		_isManual = true;
		break;
	}
	case pvr::SimplifiedInput::Down:
	{
		if (_currentDemoConfiguration == 0) { _currentDemoConfiguration = DemoConfigurations::NumDemoConfigurations; }
		_currentDemoConfiguration = (_currentDemoConfiguration - 1) % DemoConfigurations::NumDemoConfigurations;
		updateBloomConfiguration();
		_isManual = true;
		break;
	}
	case pvr::SimplifiedInput::Left:
	{
		uint32_t currentBloomMode = static_cast<uint32_t>(_blurMode);
		currentBloomMode -= 1;
		currentBloomMode = (currentBloomMode + static_cast<uint32_t>(BloomMode::NumBloomModes)) % static_cast<uint32_t>(BloomMode::NumBloomModes);
		_blurMode = static_cast<BloomMode>(currentBloomMode);
		updateBloomConfiguration();
		_isManual = true;
		break;
	}
	case pvr::SimplifiedInput::Right:
	{
		uint32_t currentBloomMode = static_cast<uint32_t>(_blurMode);
		currentBloomMode += 1;
		currentBloomMode = (currentBloomMode + static_cast<uint32_t>(BloomMode::NumBloomModes)) % static_cast<uint32_t>(BloomMode::NumBloomModes);
		_blurMode = static_cast<BloomMode>(currentBloomMode);
		updateBloomConfiguration();
		_isManual = true;
		break;
	}
	case pvr::SimplifiedInput::ActionClose:
	{
		this->exitShell();
		break;
	}
	case pvr::SimplifiedInput::Action1:
	{
		_renderOnlyBloom = !_renderOnlyBloom;
		break;
	}
	case pvr::SimplifiedInput::Action2:
	{
		_animateObject = !_animateObject;
		_animateCamera = !_animateCamera;
		break;
	}
	case pvr::SimplifiedInput::Action3:
	{
		(++_currentScene) %= NumScenes;
		break;
	}
	default:
	{
		break;
	}
	}
}

/// <summary>Render the UI.</summary>
void OpenGLESPostProcessing::renderUI()
{
	_uiRenderer.beginRendering();
	_uiRenderer.getSdkLogo()->render();
	_uiRenderer.getDefaultTitle()->render();
	_uiRenderer.getDefaultControls()->render();
	_uiRenderer.getDefaultDescription()->render();
	_uiRenderer.endRendering();
}

/// <summary>This function must be implemented by the user of the shell. The user should return its pvr::Shell object defining the behaviour of the application.</summary>
/// <returns>Return a unique ptr to the demo supplied by the user.</returns>
std::unique_ptr<pvr::Shell> pvr::newDemo() { return std::make_unique<OpenGLESPostProcessing>(); }
