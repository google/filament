/*!
\brief	This example demonstrates how to use Physically based rendering using Metallic-Roughness work flow showcasing 2 scenes (helmet and sphere) with Image based lighting
		(IBL). The Technique presented here is based on Epic Games publication http://blog.selfshadow.com/publications/s2013-shading-course/karis/s2013_pbs_epic_notes_v2.pdf
\file	OpenGLESImageBasedLighting.cpp
\author	PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/

/*!
	IBL Description
	Material: Metallic-Roughness
	============================
	- Albedo map: This is a raw colour of the material. This map shouldn't contains any shading information like Ambient Occlusion which is
	very often baked in the diffuse map for Phong model.
	It does not only influence the diffuse colour, but also the specular colour of the material as well.
	When the metalness is one(metallic material) the base colour is the specular.

	- MetallicRoughness map: The metallic-roughness texture.
	The metalness values are sampled from the B channel and roughness values are sampled from the G channel, other channels are ignored.

	BRDF
	====
	*Diffuse BRDF: Lambertian diffuse
	f = Cliff / PI
	Cdiff: Diffuse albedo of the material.

	*Specular BRDF: Cook-Torrance
	f = D * F * G / (4 * (N.L) * (N.V));
	D: NDF (Normal Distribution function), It computes the distribution of the micro-facets for the shaded surface
	F: Describes how light reflects and refracts at the intersection of two different media (most often in computer graphics : Air and the shaded surface)
	G: Defines the shadowing from the micro-facets
	N.L:  is the dot product between the normal of the shaded surface and the light direction.
	N.V is the dot product between the normal of the shaded surface and the view direction.

	IBL work-flow
	============
	IBL is one of the most common technique for implementing global illumination. The basic idea is to make use of environment maps as light source.

	IBL Diffuse:
	Normally when Lambert diffuse is used in games, it is the light colour multiplied by the visibility factor( N dot L).
	But when using In-directional lighting (IBL) the visibility factor is not considered because the light is coming from everywhere.
	So the diffuse factor is the light colour.

	All the pixels in the environment map is a light source, so when shading a point it has to be lit by many pixels from the environment map.
	Sampling multiple texels for shading a single point is not practical for real-time application. Therefore these samples are precomputed
	in the diffuse irradiance map. So at run time it would be a single texture fetch for the given reflection direction.

	IBL Specular & BRDF_LUT:
	Specular reflections look shiny when the roughness values is low and it become blurry when the roughness value is high.
	This is encoded in the specular irradiance texture.

	We use the same technique, Split-Sum-Approximation presented by Epics Games, each mip level of this image contains the environment map specular reflectance.
	Mip level 0 contains samples for roughness value 0, and the remaining mip levels get blurry for each mip level as the roughness value increases to 1.

	The samples encoded in this map is the result of the specular BRDF of the environment map. For each pixels in the environment map,
	computes the Cook-Torrance micro-facet BRDF and stores those results.

	Using the mip map for storing blurred images for each roughness value has one draw back, Specular aliasing.
	This is most pronounced for level 0. Since we are using the mip map for storing different roughness, mip maps cannot combat this.
	For this reason, we are using the environment map itself as the first level. This adds an additional texture read, but eliminates the worst
	of specular aliasing. Other mip map levels doesn't have this issue as they are blurred and low resolution.
*/

#include "PVRShell/PVRShell.h"
#include "PVRUtils/PVRUtilsGles.h"
#include "PVRUtils/OpenGLES/PBRUtilsGles.h"
#include "PVRAssets/fileio/GltfReader.h"
#include "PVRCore/cameras/TPSCamera.h"
#include "PVRCore/textureio/TextureWriterPVR.h"

// Content file names
// Shaders
const char PBRVertShaderFileName[] = "PBRVertShader.vsh";
const char PBRFragShaderFileName[] = "PBRFragShader.fsh";
const char SkyboxVertShaderFileName[] = "SkyboxVertShader.vsh";
const char SkyboxFragShaderFileName[] = "SkyboxFragShader.fsh";

// Scenes
const char HelmetModelFileName[] = "damagedHelmet.gltf";
const char SphereModelFileName[] = "sphere.pod";

// Textures
const std::string SkyboxTexFileName[] = {
	"satara_night_scale_0.305_rgb9e5", //
	"misty_pines_rgb9e5", //
};

int currentSkybox = 0;

const std::string SkyboxTexFileExtension = ".pvr";
const std::string DiffuseIrradianceMapTexFileSuffix = "_Irradiance.pvr";
const std::string PrefilteredEnvMapTexFileSuffix = "_Prefiltered.pvr";
const std::string BrdfLUTTexFile = "brdfLUT.pvr";

enum
{
	IrradianceMapDim = 64,
	PrefilterEnvMapDim = 256,
	NumSphereRows = 4,
	NumSphereColumns = 6,
	NumInstances = NumSphereRows * NumSphereColumns,
	NumSkyBoxes = sizeof(SkyboxTexFileName) / sizeof(SkyboxTexFileName[0])
};

const float rotationSpeed = .01f;

const float fov = 65.f;

const glm::vec3 lightDir = glm::normalize(glm::vec3(-0.5f, -0.5f, -0.5f));
const glm::vec3 lightColor = glm::vec3(0.f, 0.f, 0.f);

float exposure = 1.;

enum class Models
{
	Helmet,
	Sphere,
	NumModels
};

struct Material
{
	glm::vec3 albedo; // std140 offset 0
	float roughness; // std140 packed at the end of RGB (offset: 12 bytes)
	float metallic; // std140 next item                (offset: 16 bytes)
	float _padding1; // Since the Materials will be an array, std140 requires
	float _padding2; // the whole struct to be padded to a vec4
	float _padding3; // Total size: 32 bytes
} materials[25];

class SkyboxPass
{
public:
	SkyboxPass() : program(0), skyBoxMap(0), irradianceMap(0), prefilteredMap(0), numPrefilteredMipLevels(0), uboBuffer(0), isBufferStorageExtSupported(false) {}
	void init(pvr::IAssetProvider& assetProvider, bool inIsBufferStorageExtSupported)
	{
		cleanup();

		this->isBufferStorageExtSupported = inIsBufferStorageExtSupported;

		// load the environment map.
		skyBoxMap = pvr::utils::textureUpload(assetProvider, SkyboxTexFileName[currentSkybox] + SkyboxTexFileExtension);

		debugThrowOnApiError("Setting skybox params");
		std::string irradianceFileName = SkyboxTexFileName[currentSkybox] + DiffuseIrradianceMapTexFileSuffix;

		// Generating the irradiance map very well happen online, possible in a once-off step, but because it may take some time,
		// it is better to happen beforehand.

		// The commented-out try-catch could be a typical way to generate them online.

		// try
		{
			irradianceMap = pvr::utils::textureUpload(assetProvider, irradianceFileName);
		}
		// catch (const pvr::FileNotFoundError&) // Not exists
		//{
		// pvr::Texture irradianceMapData;
		// pvr::utils::generateIrradianceMap(skyBoxMap, irradianceMapData, irradianceMap);
		// pvr::assetWriters::writePVR(irradianceMapData, pvr::FileStream(irradianceFileName, "wb"));
		//}

		std::string preFilteredMap = SkyboxTexFileName[currentSkybox] + PrefilteredEnvMapTexFileSuffix;

		// Same with the PreFiltered map

		// try
		{
			pvr::Texture preFilteredMapData;
			prefilteredMap = pvr::utils::textureUpload(assetProvider, preFilteredMap, preFilteredMapData);
			numPrefilteredMipLevels = preFilteredMapData.getNumMipMapLevels();
		}
		// catch (pvr::FileNotFoundError&)
		//{
		//	pvr::Texture preFilteredMapData;
		//	// Discard the last two mip maps. From our experimentation throwing away "a few" mip levels, keeping the last as 16x16~4x4 avoids the worst of
		//	// blocky texel artefacts for materials with roughness values close to 1.0 and with large smoothly curved surfaces (e.g. a rough sphere).
		//	// However, the more mip maps that are discarded, the less accurate the blurring of the mip map.
		//	const uint32_t DISCARD_SPECULAR_MIP_LEVELS = 4;
		//	pvr::utils::generatePreFilteredMapMipMapStyle(skyBoxMap, preFilteredMapData, prefilteredMap, PrefilterEnvMapDim, false, DISCARD_SPECULAR_MIP_LEVELS);
		//	numPrefilteredMipLevels = preFilteredMapData.getNumMipMapLevels();

		//	pvr::assetWriters::writePVR(preFilteredMapData, pvr::FileStream(preFilteredMap, "wb"));
		//}

		std::vector<const char*> defines;
		// Note that the tone mapping that we use does not work with (or need) SRGB gamma correction

		// create the program
		program = pvr::utils::createShaderProgram(
			assetProvider, SkyboxVertShaderFileName, SkyboxFragShaderFileName, nullptr, nullptr, 0, defines.data(), static_cast<uint32_t>(defines.size()));

		// set up the uniform buffer
		pvr::utils::StructuredMemoryDescription viewDesc;
		viewDesc.addElement("InvVPMatrix", pvr::GpuDatatypes::mat4x4);
		viewDesc.addElement("EyePos", pvr::GpuDatatypes::vec3);
		uboView.init(viewDesc);

		gl::GenBuffers(1, &uboBuffer);
		gl::BindBuffer(GL_UNIFORM_BUFFER, uboBuffer);
		gl::BufferData(GL_UNIFORM_BUFFER, static_cast<GLsizeiptr>(uboView.getSize()), nullptr, GL_DYNAMIC_DRAW);

		// if GL_EXT_buffer_storage is supported then map the buffer upfront and never unmap it
		if (isBufferStorageExtSupported)
		{
			gl::BindBuffer(GL_COPY_READ_BUFFER, uboBuffer);
			gl::ext::BufferStorageEXT(GL_COPY_READ_BUFFER, (GLsizei)uboView.getSize(), nullptr, GL_MAP_WRITE_BIT_EXT | GL_MAP_PERSISTENT_BIT_EXT | GL_MAP_COHERENT_BIT_EXT);

			void* memory =
				gl::MapBufferRange(GL_COPY_READ_BUFFER, 0, static_cast<GLsizeiptr>(uboView.getSize()), GL_MAP_WRITE_BIT_EXT | GL_MAP_PERSISTENT_BIT_EXT | GL_MAP_COHERENT_BIT_EXT);
			uboView.pointToMappedMemory(memory);
		}
	}

	void cleanup()
	{
		pvr::utils::deleteTexturesAndZero(skyBoxMap, irradianceMap, prefilteredMap);

		if (program)
		{
			gl::DeleteProgram(program);
			program = 0;
		}

		if (uboBuffer) { gl::DeleteBuffers(1, &uboBuffer); }
	}

	uint32_t getNumPrefilteredMipLevels() const { return numPrefilteredMipLevels; }

	GLuint getDiffuseIrradianceMap() { return irradianceMap; }

	GLuint getPrefilteredMap() { return prefilteredMap; }
	GLuint getEnvironmentMap() { return skyBoxMap; }

	void render(const glm::mat4& viewProjMtx, const glm::vec3& eyePos)
	{
		gl::ActiveTexture(GL_TEXTURE9);
		gl::BindTexture(GL_TEXTURE_CUBE_MAP, skyBoxMap);

		// Disable the depth testing, no need.
		gl::Disable(GL_DEPTH_TEST);
		gl::BindBufferRange(GL_UNIFORM_BUFFER, 0, uboBuffer, 0, static_cast<GLsizeiptr>(uboView.getSize()));

		void* mappedMemory = nullptr;
		if (!isBufferStorageExtSupported)
		{
			gl::BindBuffer(GL_UNIFORM_BUFFER, uboBuffer);
			mappedMemory = gl::MapBufferRange(GL_UNIFORM_BUFFER, 0, (size_t)uboView.getSize(), GL_MAP_WRITE_BIT);
			uboView.pointToMappedMemory(mappedMemory);
		}

		uboView.getElement(0).setValue(glm::inverse(viewProjMtx));
		uboView.getElement(1).setValue(eyePos);

		if (!isBufferStorageExtSupported) { gl::UnmapBuffer(GL_UNIFORM_BUFFER); }

		gl::UseProgram(program);
		gl::Uniform1f(3, exposure);
		gl::DrawArrays(GL_TRIANGLES, 0, 6);
	}

	~SkyboxPass() { cleanup(); }

private:
	// Generates specular irradiance map.
	GLuint program;
	GLuint skyBoxMap;
	GLuint irradianceMap; // Diffuse irradiance
	GLuint prefilteredMap; // specular filtered map.
	uint32_t numPrefilteredMipLevels;
	pvr::utils::StructuredBufferView uboView;
	GLuint uboBuffer;
	bool isBufferStorageExtSupported;

public:
};

class SpherePass
{
public:
	/// <summary>initialise the sphere's program</summary>
	/// <param name="assetProvider">Asset provider for loading assets from disk.</param>
	void init(pvr::IAssetProvider& assetProvider, bool srgbFramebuffer, bool isBufferStorageSupported)
	{
		const pvr::utils::VertexBindings_Name vertexBindings[] = { { "POSITION", "inVertex" }, { "NORMAL", "inNormal" } };

		std::vector<const char*> defines;
		if (srgbFramebuffer) { defines.push_back("FRAMEBUFFER_SRGB"); }

		program =
			pvr::utils::createShaderProgram(assetProvider, PBRVertShaderFileName, PBRFragShaderFileName, nullptr, nullptr, 0, defines.data(), static_cast<uint32_t>(defines.size()));

		model = pvr::assets::loadModel(assetProvider, SphereModelFileName);
		pvr::utils::appendSingleBuffersFromModel(*model, vbos, ibos);
		vertexConfiguration = createInputAssemblyFromMesh(model->getMesh(0), vertexBindings, ARRAY_SIZE(vertexBindings));

		// Sphere instances
		// offset the position for each sphere instances
		const glm::vec3 albedos[] = {
			glm::vec3(0.971519, 0.959915, 0.915324), // Silver Metallic
			glm::vec3(1, 0.765557, 0.336057), // Gold Metallic
			glm::vec3(.75f), // White Plastic
			glm::vec3(.01f, .05f, .2f), // Blue Plastic
		};
		const float roughness[NumSphereColumns] = { .9f, 0.6f, 0.35f, 0.25f, 0.15f, 0.0f };

		// set the per sphere material property. Creating a grid of spheres:  4x6, two rows metallic two plastic, each row a different colour, going from rough to smooth.
		for (uint32_t i = 0; i < NumSphereRows; ++i)
		{
			for (uint32_t j = 0; j < NumSphereColumns; ++j)
			{
				auto& mat = materials[i * NumSphereColumns + j];
				mat.albedo = albedos[i]; // One colour per row.
				mat.roughness = roughness[j]; // Smooth to rough
				mat.metallic = float(i < 2) * 1.0f; // set the first 2 row set metallicity to 1.0 and the two last to 0.0
			}
		}
		gl::GenBuffers(1, &materialUbo);
		gl::BindBuffer(GL_UNIFORM_BUFFER, materialUbo);
		if (isBufferStorageSupported) { gl::ext::BufferStorageEXT(GL_UNIFORM_BUFFER, sizeof(materials), materials, 0); }
		else
		{
			gl::BufferData(GL_UNIFORM_BUFFER, sizeof(materials), materials, GL_STATIC_DRAW);
		}
		gl::BindBuffer(GL_UNIFORM_BUFFER, 0);
	}

	/// <summary>Destructor for the sphere pass</summary>
	~SpherePass()
	{
		gl::DeleteBuffers(static_cast<GLsizei>(vbos.size()), vbos.data());
		gl::DeleteBuffers(static_cast<GLsizei>(ibos.size()), ibos.data());
		gl::DeleteProgram(program);
	}

	/// <summary>Renders the sphere scene</summary>
	void render()
	{
		debugThrowOnApiError("begin Render Sphere Scene");
		gl::UseProgram(program);
		debugThrowOnApiError("bind sphere pass program");

		for (uint32_t node = 0; node < model->getNumMeshNodes(); ++node)
		{
			uint32_t meshId = model->getMeshNode(node).getObjectId();
			gl::BindBuffer(GL_ARRAY_BUFFER, vbos[meshId]);
			gl::BindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibos[meshId]);
			gl::BindBufferBase(GL_UNIFORM_BUFFER, 3, materialUbo);

			const pvr::assets::Mesh& mesh = model->getMesh(meshId);
			for (uint32_t i = 0; i < vertexConfiguration.attributes.size(); ++i)
			{
				auto& attrib = vertexConfiguration.attributes[i];
				auto& binding = vertexConfiguration.bindings[0];
				gl::EnableVertexAttribArray(attrib.index);
				gl::VertexAttribPointer(attrib.index, attrib.width, pvr::utils::convertToGles(attrib.format), dataTypeIsNormalised(attrib.format), binding.strideInBytes,
					reinterpret_cast<const void*>(static_cast<uintptr_t>(attrib.offsetInBytes)));
			}
			debugThrowOnApiError("Render Node (before draw)");
			gl::DrawElementsInstanced(GL_TRIANGLES, mesh.getNumFaces() * 3, pvr::utils::convertToGles(mesh.getFaces().getDataType()), nullptr, NumInstances);

			for (uint32_t i = 0; i < vertexConfiguration.attributes.size(); ++i)
			{
				auto& attrib = vertexConfiguration.attributes[i];
				gl::DisableVertexAttribArray(attrib.index);
			}
			debugThrowOnApiError("Render Node (after draw)");
		}
	}

private:
	pvr::assets::ModelHandle model;
	GLuint program;
	std::vector<GLuint> vbos;
	std::vector<GLuint> ibos;
	GLuint materialUbo;
	pvr::utils::VertexConfiguration vertexConfiguration;
};

class HelmetPass
{
public:
	void init(pvr::IAssetProvider& assetProvider, bool srgbFramebuffer)
	{
		model = pvr::assets::loadModel(assetProvider, HelmetModelFileName);

		pvr::utils::appendSingleBuffersFromModel(*model, vbos, ibos);

		// Load the texture
		loadTextures(assetProvider);

		createProgram(assetProvider, srgbFramebuffer);
	}

	GLuint getProgram() { return program; }

	pvr::assets::ModelHandle& getModel() { return model; }

	GLuint getAlbedoMap() { return textures[0]; }

	GLuint getOcclusionMetallicRoughnessMap() { return textures[1]; }

	GLuint getNormalMap() { return textures[2]; }

	GLuint getEmissiveMap() { return textures[3]; }

	~HelmetPass()
	{
		if (program) { gl::DeleteProgram(program); }
		if (vbos.size())
		{
			gl::DeleteBuffers(static_cast<GLsizei>(vbos.size()), vbos.data());
			vbos.clear();
		}
		if (ibos.size())
		{
			gl::DeleteBuffers(static_cast<GLsizei>(ibos.size()), ibos.data());
			ibos.clear();
		}

		gl::DeleteTextures(static_cast<GLsizei>(textures.size()), textures.data());
	}

	void render()
	{
		// bind the albedo texture
		gl::ActiveTexture(GL_TEXTURE0);
		gl::BindTexture(GL_TEXTURE_2D, textures[0]);

		// bind the metallic roughness texture
		gl::ActiveTexture(GL_TEXTURE1);
		gl::BindTexture(GL_TEXTURE_2D, textures[1]);

		// bind the normal texture
		gl::ActiveTexture(GL_TEXTURE2);
		gl::BindTexture(GL_TEXTURE_2D, textures[2]);

		// bind the emissive map
		gl::ActiveTexture(GL_TEXTURE3);
		gl::BindTexture(GL_TEXTURE_2D, textures[3]);

		// render the helmet
		gl::UseProgram(program);
		for (uint32_t i = 0; i < model->getNumMeshNodes(); ++i) { renderMesh(i); }
	}

private:
	void loadTextures(pvr::IAssetProvider& assetProvider)
	{
		for (uint32_t i = 0; i < model->getNumTextures(); ++i)
		{
			std::unique_ptr<pvr::Stream> stream = assetProvider.getAssetStream(model->getTexture(i).getName());
			pvr::Texture tex = pvr::textureLoad(*stream, pvr::TextureFileFormat::PVR);
			textures.push_back(pvr::utils::textureUpload(tex, false, true).image);
		}
	}

	void createProgram(pvr::IAssetProvider& assetProvider, bool srgbFramebuffer)
	{
		// Create the PBR program
		std::vector<const char*> defines;
		defines.push_back("MATERIAL_TEXTURES");
		defines.push_back("NORMAL_MAP");
		if (srgbFramebuffer) { defines.push_back("FRAMEBUFFER_SRGB"); }

		program =
			pvr::utils::createShaderProgram(assetProvider, PBRVertShaderFileName, PBRFragShaderFileName, nullptr, nullptr, 0, defines.data(), static_cast<uint32_t>(defines.size()));
		const pvr::assets::Mesh& mesh = model->getMesh(0);
		const pvr::utils::VertexBindings_Name vertexBindings[] = { { "POSITION", "inVertex" }, { "NORMAL", "inNormal" }, { "UV0", "inTexCoord" }, { "TANGENT", "inTangent" } };
		vertexConfiguration = createInputAssemblyFromMesh(mesh, vertexBindings, ARRAY_SIZE(vertexBindings));
	}

	void renderMesh(uint32_t meshNodeId)
	{
		debugThrowOnApiError("ERROR: renderMesh begin");
		const pvr::assets::Model::Node* pNode = &model->getMeshNode(meshNodeId);
		const uint32_t meshId = pNode->getObjectId();
		// Gets pMesh referenced by the pNode
		const pvr::assets::Mesh* pMesh = &model->getMesh(meshId);

		// bind the vertex and index buffer
		gl::BindBuffer(GL_ARRAY_BUFFER, vbos[meshId]);
		gl::BindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibos[meshId]);
		debugThrowOnApiError("ERROR: Failed to bind vertex and index buffer");

		// set the vertex attribute pointers
		for (size_t i = 0; i < vertexConfiguration.attributes.size(); ++i)
		{
			auto& attrib = vertexConfiguration.attributes[i];
			auto& binding = vertexConfiguration.bindings[0];

			gl::EnableVertexAttribArray(attrib.index);
			gl::VertexAttribPointer(attrib.index, attrib.width, pvr::utils::convertToGles(attrib.format), dataTypeIsNormalised(attrib.format), binding.strideInBytes,
				reinterpret_cast<const void*>(static_cast<uintptr_t>(attrib.offsetInBytes)));
			debugThrowOnApiError("ERROR");
		}
		gl::DrawElements(GL_TRIANGLES, pMesh->getNumFaces() * 3, pvr::utils::convertToGles(pMesh->getFaces().getDataType()), nullptr);

		for (uint32_t i = 0; i < vertexConfiguration.attributes.size(); ++i)
		{
			auto& attrib = vertexConfiguration.attributes[i];
			gl::DisableVertexAttribArray(attrib.index);
		}
		debugThrowOnApiError("ERROR");
	}

	pvr::assets::ModelHandle model;
	pvr::utils::VertexConfiguration vertexConfiguration;
	std::vector<GLuint> vbos;
	std::vector<GLuint> ibos;
	GLuint program;
	std::vector<GLuint> textures;
};

/// <summary>Class implementing the pvr::Shell functions.</summary>
class OpenGLESImageBasedLighting : public pvr::Shell
{
	struct DeviceResources
	{
		pvr::EglContext context;

		GLuint uboStatic; // static UBO
		GLuint uboPerFrame; // static UBO
		GLuint uboPerModel;
		GLuint samplerBilinear; // Sampler to use for the BRDF
		GLuint samplerTrilinearFull; // Sampler to use for "normal" textures
		GLuint samplerTrilinearLodClamped; // Sampler to use for the reflections

		// UIRenderer used to display text
		pvr::ui::UIRenderer uiRenderer;

		SkyboxPass skyboxPass;
		SpherePass spherePass;
		HelmetPass helmetPass;

		GLuint brdfLUT;

		DeviceResources() {}
		~DeviceResources()
		{
			gl::DeleteBuffers(1, &uboStatic);
			gl::DeleteBuffers(1, &uboPerFrame);
			gl::DeleteBuffers(1, &uboPerModel);
			gl::DeleteSamplers(1, &samplerBilinear);
			gl::DeleteSamplers(1, &samplerTrilinearFull);
			gl::DeleteSamplers(1, &samplerTrilinearLodClamped);
		}
	};

	std::unique_ptr<DeviceResources> _deviceResources;

	pvr::utils::StructuredBufferView _uboPerSceneBufferView;
	pvr::utils::StructuredBufferView _uboPerModelBufferView;
	pvr::utils::StructuredBufferView _uboPerFrameBufferView;
	std::vector<char> _uboDynamicData;
	std::vector<char> _uboMaterialData;
	glm::mat4 _projMtx;
	pvr::TPSOrbitCamera _camera;
	Models _currentModel = Models::Helmet;
	bool _pause = false;
	bool _isBufferStorageExtSupported;

public:
	virtual pvr::Result initApplication();
	virtual pvr::Result initView();
	virtual pvr::Result releaseView();
	virtual pvr::Result quitApplication();
	virtual pvr::Result renderFrame();
	void createUbo();
	void setDefaultOpenglState();

	virtual void eventMappedInput(pvr::SimplifiedInput key)
	{
		float oldexposure = exposure;
		switch (key)
		{
		case pvr::SimplifiedInput::Left:
			exposure *= .75;
			if (oldexposure > 1.f && exposure < 1.f) { exposure = 1.f; }
			break;
		case pvr::SimplifiedInput::Right:
			exposure *= 1.25;
			if (oldexposure < 1.f && exposure > 1.f) { exposure = 1.f; }
			break;

		case pvr::SimplifiedInput::Action2: {
			uint32_t currentModel = static_cast<uint32_t>(_currentModel);
			currentModel -= 1;
			currentModel = currentModel % static_cast<uint32_t>(Models::NumModels);
			_currentModel = static_cast<Models>(currentModel);
			break;
		}
		case pvr::SimplifiedInput::Action1: {
			_pause = !_pause;
			break;
		}
		case pvr::SimplifiedInput::Action3: {
			(++currentSkybox) %= NumSkyBoxes;

			_deviceResources->skyboxPass.init(*this, _isBufferStorageExtSupported);
			break;
		}

		case pvr::SimplifiedInput::ActionClose: {
			this->exitShell();
			break;
		}
		default: {
			break;
		}
		}
	}
};

/// <summary>Code in initApplication() will be called by Shell once per run, before the rendering context is created.
/// Used to initialize variables that are not dependent on it (e.g. external modules, loading meshes, etc.). If the rendering
/// context is lost, initApplication() will not be called again.</summary>
pvr::Result OpenGLESImageBasedLighting::initApplication()
{
	// The tone mapping that we use does not work with (or need) SRGB gamma correction
	setBackBufferColorspace(pvr::ColorSpace::lRGB);
	return pvr::Result::Success;
}

/// <summary>Code in quitApplication() will be called by Shell once per run, just before exiting the program.
/// quitApplication() will not be called every time the rendering context is lost, only before application exit.</summary>
/// <returns>Result::Success if no error occurred.</returns>
pvr::Result OpenGLESImageBasedLighting::quitApplication() { return pvr::Result::Success; }

/// <summary>Code in initView() will be called by Shell upon initialization or after a change in the rendering context.
/// Used to initialize variables that are dependent on the rendering context(e.g.textures, vertex buffers, etc.)</summary>
/// <returns>Result::Success if no error occurred.</returns>
pvr::Result OpenGLESImageBasedLighting::initView()
{
	_deviceResources = std::make_unique<DeviceResources>();
	_deviceResources->context = pvr::createEglContext();

	// Create the context. The minimum OpenGLES version must be OpenGL ES 3.1
	_deviceResources->context->init(getWindow(), getDisplay(), getDisplayAttributes(), pvr::Api::OpenGLES31);

	// We make use of GL_EXT_buffer_storage wherever possible
	_isBufferStorageExtSupported = gl::isGlExtensionSupported("GL_EXT_buffer_storage");

	// Initialise the uirenderer
	_deviceResources->uiRenderer.init(getWidth(), getHeight(), isFullScreen(), getBackBufferColorspace() == pvr::ColorSpace::sRGB);
	_deviceResources->uiRenderer.getDefaultTitle()->setText("ImageBasedLighting");
	_deviceResources->uiRenderer.getDefaultControls()->setText("Action 1: Pause\n"
															   "Action 2: Change model\n"
															   "Action 3: Change scene\n");
	_deviceResources->uiRenderer.getDefaultTitle()->commitUpdates();
	_deviceResources->uiRenderer.getDefaultControls()->commitUpdates();

	_deviceResources->skyboxPass.init(*this, _isBufferStorageExtSupported);
	_deviceResources->helmetPass.init(*this, getBackBufferColorspace() == pvr::ColorSpace::sRGB);
	_deviceResources->spherePass.init(*this, getBackBufferColorspace() == pvr::ColorSpace::sRGB, _isBufferStorageExtSupported);

	// set the view port dimension back.
	gl::Viewport(0, 0, getWidth(), getHeight());

	// create the static ubo
	createUbo();

	_deviceResources->brdfLUT = pvr::utils::textureUpload(*this, BrdfLUTTexFile, false);

	// BRDF is of course pre-generated. To generate it
	// pvr::Texture brdflut = pvr::utils::generateCookTorranceBRDFLUT();
	// pvr::assetWriters::AssetWriterPVR(pvr::FileStream::createFileStream(BrdfLUTTexFile, "w")).writeAsset(brdflut);

	gl::GenSamplers(1, &_deviceResources->samplerTrilinearFull);
	gl::GenSamplers(1, &_deviceResources->samplerTrilinearLodClamped);
	gl::GenSamplers(1, &_deviceResources->samplerBilinear);
	debugThrowOnApiError("Error generating samplers");

	gl::SamplerParameteri(_deviceResources->samplerTrilinearFull, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	gl::SamplerParameteri(_deviceResources->samplerTrilinearFull, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	gl::SamplerParameteri(_deviceResources->samplerTrilinearFull, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	gl::SamplerParameteri(_deviceResources->samplerTrilinearFull, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	gl::SamplerParameteri(_deviceResources->samplerTrilinearLodClamped, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	gl::SamplerParameteri(_deviceResources->samplerTrilinearLodClamped, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	gl::SamplerParameteri(_deviceResources->samplerTrilinearLodClamped, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	gl::SamplerParameteri(_deviceResources->samplerTrilinearLodClamped, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	gl::SamplerParameterf(_deviceResources->samplerTrilinearLodClamped, GL_TEXTURE_MIN_LOD, 2.f); // 256

	gl::SamplerParameteri(_deviceResources->samplerBilinear, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	gl::SamplerParameteri(_deviceResources->samplerBilinear, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	gl::SamplerParameteri(_deviceResources->samplerBilinear, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	gl::SamplerParameteri(_deviceResources->samplerBilinear, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	debugThrowOnApiError("Error defining sampler parameters");

	gl::BindSampler(0, _deviceResources->samplerTrilinearFull); // Material:
	gl::BindSampler(1, _deviceResources->samplerTrilinearFull); // ...
	gl::BindSampler(2, _deviceResources->samplerTrilinearFull); // ...
	gl::BindSampler(3, _deviceResources->samplerTrilinearFull); // ...
	gl::BindSampler(4, _deviceResources->samplerTrilinearFull); // ...
	gl::BindSampler(5, _deviceResources->samplerTrilinearFull); // Environment: Irradiance
	gl::BindSampler(6, _deviceResources->samplerTrilinearFull); // Environment: Pre-filtered reflection map

	gl::BindSampler(7, _deviceResources->samplerBilinear); // BRDF: No mip maps!

	gl::BindSampler(8, _deviceResources->samplerTrilinearLodClamped); // Environment map, used for Reflections

	gl::BindSampler(9, _deviceResources->samplerTrilinearFull); // Environment map, used for Rendering
	debugThrowOnApiError("Error binding samplers");

	if (isScreenRotated())
	{
		_projMtx = pvr::math::perspectiveFov(
			pvr::Api::OpenGLES31, glm::radians(fov), static_cast<float>(this->getHeight()), static_cast<float>(this->getWidth()), 0.1f, 2000.f, glm::pi<float>() * .5f);
	}
	else
	{
		_projMtx = pvr::math::perspectiveFov(pvr::Api::OpenGLES31, glm::radians(fov), static_cast<float>(this->getWidth()), static_cast<float>(this->getHeight()), 0.1f, 2000.f);
	}

	// setup the camera
	_camera.setDistanceFromTarget(50.f);
	_camera.setInclination(10.f);
	setDefaultOpenglState();
	return pvr::Result::Success;
}

void OpenGLESImageBasedLighting::setDefaultOpenglState()
{
	gl::DepthMask(GL_TRUE);
	gl::CullFace(GL_BACK);
	gl::FrontFace(GL_CCW);
	gl::Enable(GL_CULL_FACE);
	gl::Enable(GL_DEPTH_TEST);
}

/// <summary>Code in releaseView() will be called by Shell when the application quits or before a change in the rendering context.</summary>
/// <returns>Result::Success if no error occurred.</returns>
pvr::Result OpenGLESImageBasedLighting::releaseView()
{
	_deviceResources.reset();
	return pvr::Result::Success;
}

/// <summary>Main rendering loop function of the program. The shell will call this function every frame</summary>
/// <returns>Result::Success if no error occurred.</summary>
pvr::Result OpenGLESImageBasedLighting::renderFrame()
{
	debugThrowOnApiError("Begin Frame");

	gl::Clear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	if (!_pause) { _camera.addAzimuth(getFrameTime() * rotationSpeed); }

	if (this->isKeyPressed(pvr::Keys::A)) { _camera.addAzimuth(getFrameTime() * -.1f); }
	if (this->isKeyPressed(pvr::Keys::D)) { _camera.addAzimuth(getFrameTime() * .1f); }

	if (this->isKeyPressed(pvr::Keys::W)) { _camera.addInclination(getFrameTime() * .1f); }
	if (this->isKeyPressed(pvr::Keys::S)) { _camera.addInclination(getFrameTime() * -.1f); }

	gl::BindBuffer(GL_UNIFORM_BUFFER, _deviceResources->uboPerFrame);
	void* mappedMemory = nullptr;
	if (!_isBufferStorageExtSupported)
	{
		mappedMemory = gl::MapBufferRange(GL_UNIFORM_BUFFER, 0, (size_t)_uboPerFrameBufferView.getSize(), GL_MAP_WRITE_BIT);
		_uboPerFrameBufferView.pointToMappedMemory(mappedMemory);
	}
	const glm::mat4 viewProj = _projMtx * _camera.getViewMatrix();
	_uboPerFrameBufferView.getElement(0).setValue(viewProj);
	_uboPerFrameBufferView.getElement(1).setValue(_camera.getCameraPosition());
	static float emissiveScale = 0.0f;
	static float emissiveStrength = 1.;
	emissiveStrength += .15f;

	if (emissiveStrength >= glm::pi<float>()) { emissiveStrength = 0.0f; }
	emissiveScale = std::abs(glm::cos(emissiveStrength)) + .75f;

	_uboPerFrameBufferView.getElement(2).setValue(emissiveScale);
	_uboPerFrameBufferView.getElement(3).setValue(exposure);

	if (!_isBufferStorageExtSupported) { gl::UnmapBuffer(GL_UNIFORM_BUFFER); }

	// render the skybox
	_deviceResources->skyboxPass.render(viewProj, _camera.getCameraPosition());

	gl::Enable(GL_DEPTH_TEST);

	// bind the diffuse irradiance map
	gl::ActiveTexture(GL_TEXTURE5);
	gl::BindTexture(GL_TEXTURE_CUBE_MAP, _deviceResources->skyboxPass.getDiffuseIrradianceMap());

	// bind the specular irradiance map
	gl::ActiveTexture(GL_TEXTURE6);
	gl::BindTexture(GL_TEXTURE_CUBE_MAP, _deviceResources->skyboxPass.getPrefilteredMap());

	// bind the brdf lut texture
	gl::ActiveTexture(GL_TEXTURE7);
	gl::BindTexture(GL_TEXTURE_2D, _deviceResources->brdfLUT);

	// bind the environment map
	gl::ActiveTexture(GL_TEXTURE8);
	gl::BindTexture(GL_TEXTURE_CUBE_MAP, _deviceResources->skyboxPass.getEnvironmentMap());

	gl::BindBufferRange(GL_UNIFORM_BUFFER, 0, _deviceResources->uboStatic, 0, static_cast<GLsizeiptr>(_uboPerSceneBufferView.getSize()));
	debugThrowOnApiError("ERROR");

	gl::BindBufferRange(GL_UNIFORM_BUFFER, 1, _deviceResources->uboPerFrame, 0, static_cast<GLsizeiptr>(_uboPerFrameBufferView.getSize()));
	debugThrowOnApiError("ERROR");

	gl::BindBufferRange(GL_UNIFORM_BUFFER, 2, _deviceResources->uboPerModel, _uboPerModelBufferView.getDynamicSliceOffset(static_cast<uint32_t>(_currentModel)),
		static_cast<GLsizeiptr>(_uboPerModelBufferView.getDynamicSliceSize()));
	debugThrowOnApiError("ERROR");

	if (_currentModel == Models::Helmet) { _deviceResources->helmetPass.render(); }
	else
	{
		_deviceResources->spherePass.render();
	}

	_deviceResources->uiRenderer.beginRendering();
	_deviceResources->uiRenderer.getDefaultTitle()->render();
	_deviceResources->uiRenderer.getDefaultControls()->render();
	_deviceResources->uiRenderer.getSdkLogo()->render();
	_deviceResources->uiRenderer.endRendering();

	if (this->shouldTakeScreenshot()) { pvr::utils::takeScreenshot(this->getScreenshotFileName(), this->getWidth(), this->getHeight()); }

	_deviceResources->context->swapBuffers();
	return pvr::Result::Success;
}

void OpenGLESImageBasedLighting::createUbo()
{
	debugThrowOnApiError("ERROR");
	{
		// Static ubo
		// The following elements are static therefore they get updated once in the initial step.
		pvr::utils::StructuredMemoryDescription memDesc;
		memDesc.addElement("lightDir", pvr::GpuDatatypes::vec3);
		memDesc.addElement("lightColor", pvr::GpuDatatypes::vec3);
		memDesc.addElement("numPrefilteredMipLevels", pvr::GpuDatatypes::uinteger);
		_uboPerSceneBufferView.init(memDesc);
		gl::GenBuffers(1, &_deviceResources->uboStatic);
		gl::BindBuffer(GL_UNIFORM_BUFFER, _deviceResources->uboStatic);
		gl::BufferData(GL_UNIFORM_BUFFER, static_cast<GLsizeiptr>(_uboPerSceneBufferView.getSize()), nullptr, GL_DYNAMIC_DRAW);

		void* memory = gl::MapBufferRange(GL_UNIFORM_BUFFER, 0, (size_t)_uboPerSceneBufferView.getSize(), GL_MAP_WRITE_BIT);
		_uboPerSceneBufferView.pointToMappedMemory(memory);
		_uboPerSceneBufferView.getElement(0).setValue(lightDir);
		_uboPerSceneBufferView.getElement(1).setValue(lightColor);
		_uboPerSceneBufferView.getElement(2).setValue(_deviceResources->skyboxPass.getNumPrefilteredMipLevels());
		gl::UnmapBuffer(GL_UNIFORM_BUFFER);
	}
	debugThrowOnApiError("ERROR");

	// Model
	{
		pvr::utils::StructuredMemoryDescription memDesc;
		memDesc.addElement("ModelMatrix", pvr::GpuDatatypes::mat4x4);
		GLint uniformAlignment = 0;
		gl::GetIntegerv(GL_UNIFORM_BUFFER_OFFSET_ALIGNMENT, &uniformAlignment);
		_uboPerModelBufferView.initDynamic(memDesc, 2, pvr::BufferUsageFlags::UniformBuffer, uniformAlignment);
		std::vector<char> _uboModelData(static_cast<const unsigned int>(_uboPerModelBufferView.getSize()));

		_uboPerModelBufferView.pointToMappedMemory(_uboModelData.data());
		_uboPerModelBufferView.getElement(0, 0, 0).setValue(glm::eulerAngleXY(glm::radians(0.f), glm::radians(120.f)) * glm::scale(glm::vec3(22.0f)));
		_uboPerModelBufferView.getElement(0, 0, 1).setValue(glm::scale(glm::vec3(4.5f)));

		gl::GenBuffers(1, &_deviceResources->uboPerModel);
		gl::BindBuffer(GL_UNIFORM_BUFFER, _deviceResources->uboPerModel);

		// if GL_EXT_buffer_storage is supported then map the buffer upfront and never unmap it
		if (_isBufferStorageExtSupported) { gl::ext::BufferStorageEXT(GL_UNIFORM_BUFFER, (GLsizei)_uboPerModelBufferView.getSize(), _uboModelData.data(), 0); }
		else
		{
			gl::BufferData(GL_UNIFORM_BUFFER, static_cast<GLsizeiptr>(_uboPerModelBufferView.getSize()), _uboModelData.data(), GL_STATIC_DRAW);
		}
	}

	{
		pvr::utils::StructuredMemoryDescription memDesc;
		memDesc.addElement("VPMatrix", pvr::GpuDatatypes::mat4x4);
		memDesc.addElement("camPos", pvr::GpuDatatypes::vec3);
		memDesc.addElement("emissiveIntensity", pvr::GpuDatatypes::Float);
		memDesc.addElement("exposure", pvr::GpuDatatypes::Float);
		_uboPerFrameBufferView.init(memDesc);
		_uboDynamicData.resize(static_cast<const unsigned int>(_uboPerFrameBufferView.getSize()));
		gl::GenBuffers(1, &_deviceResources->uboPerFrame);
		gl::BindBuffer(GL_UNIFORM_BUFFER, _deviceResources->uboPerFrame);
		gl::BufferData(GL_UNIFORM_BUFFER, static_cast<GLsizeiptr>(_uboPerFrameBufferView.getSize()), nullptr, GL_DYNAMIC_DRAW);

		// if GL_EXT_buffer_storage is supported then map the buffer upfront and never unmap it
		if (_isBufferStorageExtSupported)
		{
			gl::BindBuffer(GL_COPY_READ_BUFFER, _deviceResources->uboPerFrame);
			gl::ext::BufferStorageEXT(GL_COPY_READ_BUFFER, (GLsizei)_uboPerFrameBufferView.getSize(), nullptr, GL_MAP_WRITE_BIT_EXT | GL_MAP_PERSISTENT_BIT_EXT | GL_MAP_COHERENT_BIT_EXT);

			void* memory = gl::MapBufferRange(
				GL_COPY_READ_BUFFER, 0, static_cast<GLsizeiptr>(_uboPerFrameBufferView.getSize()), GL_MAP_WRITE_BIT_EXT | GL_MAP_PERSISTENT_BIT_EXT | GL_MAP_COHERENT_BIT_EXT);
			_uboPerFrameBufferView.pointToMappedMemory(memory);
		}
	}
	debugThrowOnApiError("ERROR");
}

/// <summary>This function must be implemented by the user of the shell. The user should return its pvr::Shell object defining the behaviour of the application.</summary>
/// <returns>Return a unique ptr to the demo supplied by the user.</returns>
std::unique_ptr<pvr::Shell> pvr::newDemo() { return std::make_unique<OpenGLESImageBasedLighting>(); }
