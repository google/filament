/*!
\brief Implements a 3D navigation renderer.
\file OpenGLESNavigation3D.cpp
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
#include "PVRShell/PVRShell.h"
#include "PVRUtils/PVRUtilsGles.h"
#define NAV_3D
#include "../../common/NavDataProcess.h"
#include "PVRCore/cameras/TPSCamera.h"
const float CameraMoveSpeed = 2.f;
static const float CamHeight = .35f;
static uint32_t routeIndex = 0;
const float nearClip = .1f;
const float farClip = 10.f;

// Camera Settings
const float CameraRotationSpeed = .5f;
const float CamRotationTime = 10000.f;

// Alpha, luminance texture.
const char* RoadTexFile = "Road.pvr";
const char* MapFile = "map.osm";
const char* FontFile = "font.pvr";

// Shaders
const char FragShaderSrcFile[] = "FragShader_ES2.fsh";
const char VertShaderSrcFile[] = "VertShader_ES2.vsh";
const char AAFragShaderSrcFile[] = "AA_FragShader_ES2.fsh";
const char AAVertShaderSrcFile[] = "AA_VertShader_ES2.vsh";
const char PlanarShadowFragShaderSrcFile[] = "PlanarShadow_FragShader_ES2.fsh";
const char PlanarShadowVertShaderSrcFile[] = "PlanarShadow_VertShader_ES2.vsh";
const char PerVertexLightVertShaderSrcFile[] = "PerVertexLight_VertShader_ES2.vsh";

enum class PipelineState
{
	RoadPipe,
	FillPipe,
	OutlinePipe,
	PlanarShaderPipe,
	BuildingPipe,
};

struct ShaderProgramFill
{
	GLuint program;
	enum Uniform
	{
		UniformTransform,
		UniformColor,
		UniformCount
	};
	int32_t uniformLocation[UniformCount];

	ShaderProgramFill()
	{
		uniformLocation[0] = -1;
		uniformLocation[1] = -1;
	}
};

struct ShaderProgramRoad
{
	GLuint program;
	enum Uniform
	{
		UniformTransform,
		UniformColor,
		UniformCount
	};
	int32_t uniformLocation[UniformCount];
	ShaderProgramRoad()
	{
		uniformLocation[0] = -1;
		uniformLocation[1] = -1;
	}
};

struct ShaderProgramPlanerShadow
{
	GLuint program;
	enum Uniform
	{
		UniformTransform,
		UniformShadowMatrix,
		UniformCount
	};
	int32_t uniformLocation[UniformCount];
	ShaderProgramPlanerShadow()
	{
		uniformLocation[0] = -1;
		uniformLocation[1] = -1;
	}
};

struct ShaderProgramBuilding
{
	GLuint program;
	enum Uniform
	{
		UniformTransform,
		UniformViewMatrix,
		UniformLightDir,
		UniformColor,
		UniformCount
	};
	int32_t uniformLocation[UniformCount];
	ShaderProgramBuilding()
	{
		uniformLocation[0] = -1;
		uniformLocation[1] = -1;
	}
};

struct DeviceResources
{
	// Graphics context.
	pvr::EglContext context;

	// Pipelines
	ShaderProgramRoad roadPipe;
	ShaderProgramFill fillPipe;
	ShaderProgramFill outlinePipe;
	ShaderProgramPlanerShadow planarShadowPipe;
	ShaderProgramBuilding buildingPipe;

	// Descriptor set for texture
	GLuint roadTex, fontTex;

	pvr::ui::Text text;
	pvr::ui::UIRenderer uiRenderer;
};

struct Plane
{
	glm::vec3 normal;
	float distance;

	Plane(glm::vec4 n)
	{
		float invLen = 1.0f / glm::length(glm::vec3(n));
		normal = glm::vec3(n) * invLen;
		distance = n.w * invLen;
	}

	Plane() : normal(glm::vec3()), distance(0.0f) {}
};

struct TileRenderingResources
{
	GLuint vbo;
	GLuint ibo;

	// Add car parking to indices
	uint32_t parkingNum;

	// Add road area ways to indices
	uint32_t areaNum;

	// Add road area outlines to indices
	uint32_t roadAreaOutlineNum;

	// Add roads to indices
	uint32_t motorwayNum;
	uint32_t trunkRoadNum;
	uint32_t primaryRoadNum;
	uint32_t secondaryRoadNum;
	uint32_t serviceRoadNum;
	uint32_t otherRoadNum;

	// Add buildings to indices
	uint32_t buildNum;

	// Add inner ways to indices
	uint32_t innerNum;

	TileRenderingResources()
	{
		vbo = 0;
		ibo = 0;
		parkingNum = 0;
		areaNum = 0;
		roadAreaOutlineNum = 0;
		motorwayNum = 0;
		trunkRoadNum = 0;
		primaryRoadNum = 0;
		secondaryRoadNum = 0;
		serviceRoadNum = 0;
		otherRoadNum = 0;
		buildNum = 0;
		innerNum = 0;
	}
};

/// <summary>implementing the pvr::Shell functions.</summary>
class OGLESNavigation3D : public pvr::Shell
{
public:
	// PVR shell functions
	pvr::Result initApplication() override;
	pvr::Result quitApplication() override;
	pvr::Result initView() override;
	pvr::Result releaseView() override;
	pvr::Result renderFrame() override;

	void createShadowMatrix()
	{
		const glm::vec4 ground = glm::vec4(0.0, 1.0, 0.0, 0.0);
		const glm::vec4 light = glm::vec4(glm::normalize(glm::vec3(0.25f, 2.4f, -1.15f)), 0.0f);
		const float d = glm::dot(ground, light);

		_shadowMatrix[0][0] = static_cast<float>(d - light.x * ground.x);
		_shadowMatrix[1][0] = static_cast<float>(0.0 - light.x * ground.y);
		_shadowMatrix[2][0] = static_cast<float>(0.0 - light.x * ground.z);
		_shadowMatrix[3][0] = static_cast<float>(0.0 - light.x * ground.w);

		_shadowMatrix[0][1] = static_cast<float>(0.0 - light.y * ground.x);
		_shadowMatrix[1][1] = static_cast<float>(d - light.y * ground.y);
		_shadowMatrix[2][1] = static_cast<float>(0.0 - light.y * ground.z);
		_shadowMatrix[3][1] = static_cast<float>(0.0 - light.y * ground.w);

		_shadowMatrix[0][2] = static_cast<float>(0.0 - light.z * ground.x);
		_shadowMatrix[1][2] = static_cast<float>(0.0 - light.z * ground.y);
		_shadowMatrix[2][2] = static_cast<float>(d - light.z * ground.z);
		_shadowMatrix[3][2] = static_cast<float>(0.0 - light.z * ground.w);

		_shadowMatrix[0][3] = static_cast<float>(0.0 - light.w * ground.x);
		_shadowMatrix[1][3] = static_cast<float>(0.0 - light.w * ground.y);
		_shadowMatrix[2][3] = static_cast<float>(0.0 - light.w * ground.z);
		_shadowMatrix[3][3] = static_cast<float>(d - light.w * ground.w);
	}

private:
	std::unique_ptr<NavDataProcess> _OSMdata;

	// Graphics resources - buffers, samplers, descriptors.
	std::unique_ptr<DeviceResources> _deviceResources;

	glm::vec4 _clearColor;

	glm::vec4 _roadAreaColor;
	glm::vec4 _motorwayColor;
	glm::vec4 _trunkRoadColor;
	glm::vec4 _primaryRoadColor;
	glm::vec4 _secondaryRoadColor;
	glm::vec4 _serviceRoadColor;
	glm::vec4 _otherRoadColor;
	glm::vec4 _parkingColor;
	glm::vec4 _outlineColor;

	struct GlesStateTracker
	{
		GLuint boundTextures[4];
		GLuint boundProgram;
		GlesStateTracker() : boundProgram(0) { memset(boundTextures, 0, sizeof(boundTextures)); }
	} _glesStates;

	std::vector<std::vector<std::unique_ptr<TileRenderingResources>>> _tileRenderingResources;

	// Uniforms
	glm::mat4 _viewProjMatrix;
	glm::mat4 _viewMatrix;

	glm::vec3 _lightDir;

	// Transformation variables
	glm::mat4 _perspectiveMatrix;

	pvr::math::ViewingFrustum _viewFrustum;

	// Window variables
	uint32_t _windowWidth;
	uint32_t _windowHeight;

	// Map tile dimensions
	uint32_t _numRows;
	uint32_t _numCols;

	float _totalRouteDistance;
	float _keyFrameTime;
	std::string _currentRoad;

	glm::mat4 _shadowMatrix;

	struct CameraTracking
	{
		glm::vec3 translation;
		glm::mat4 camRotation;
		glm::vec3 look;
		glm::vec3 up;

		CameraTracking() : translation(0.0f), camRotation(0.0f) {}
	} cameraInfo;
	pvr::TPSCamera _camera;

	void createBuffers();
	bool loadTexture();
	void setUniforms();

	void executeCommands();
	void updateAnimation();
	void calculateTransform();
	void calculateClipPlanes();
	bool inFrustum(glm::vec2 min, glm::vec2 max);
	void executeCommands(const TileRenderingResources& tileRes);
	void createPrograms();
	void setPipelineStates(PipelineState pipelineState);
	// Calculate the key frame time between one point to another.
	float calculateRouteKeyFrameTime(const glm::dvec2& start, const glm::dvec2& end) { return ::calculateRouteKeyFrameTime(start, end, _totalRouteDistance, CameraMoveSpeed); }

	glm::vec3 _cameraTranslation;
	template<class ShaderProgram>
	void bindProgram(const ShaderProgram& program);

	void bindTexture(uint32_t index, GLuint texture)
	{
		// if (glesStates.boundTextures[index] != texture)
		{
			gl::ActiveTexture(GL_TEXTURE0 + index);
			gl::BindTexture(GL_TEXTURE_2D, texture);
			debugThrowOnApiError("OGLESNavigation3D::bindTexture");
			_glesStates.boundTextures[index] = texture;
		}
	}

public:
	OGLESNavigation3D() : _totalRouteDistance(0.0f), _keyFrameTime(0.0f), _shadowMatrix(1.0), _cameraTranslation(0.0f) {}
};

/// <summary>Code in initApplication() will be called by the Shell once per run, before the rendering context is created.
/// Used to initialize variables that are not dependent on it  (e.g. external modules, loading meshes, etc.)
/// If the rendering context is lost, initApplication() will not be called again.</summary>
/// <returns>Return pvr::Result::Success if no error occurred.</returns>
pvr::Result OGLESNavigation3D::initApplication()
{
	// Disable gamma correction in the framebuffer.
	setBackBufferColorspace(pvr::ColorSpace::lRGB);
	// WARNING: This should not be done lightly. This example has taken care of linear/sRGB colour space conversion appropriately and has been tuned specifically
	// for performance/colour space correctness.

	_OSMdata = std::make_unique<NavDataProcess>(getAssetStream(MapFile), glm::ivec2(getWidth(), getHeight()));
	pvr::Result result = _OSMdata->loadAndProcessData();

	if (result != pvr::Result::Success) return result;

	createShadowMatrix();

	// perform gamma correction of the linear space colours so that they can do used directly without further thinking about Linear/sRGB colour space conversions
	// This should not be done lightly. This example in most cases only passes through hard-coded colour values and uses them directly without applying any
	// maths to their values and so can be performed safely.
	// When rendering the buildings we do apply maths to these colours and as such the colour space conversion has been taken care of appropriately
	_clearColor = pvr::utils::convertLRGBtoSRGB(ClearColorLinearSpace);
	_roadAreaColor = pvr::utils::convertLRGBtoSRGB(RoadAreaColorLinearSpace);
	_motorwayColor = pvr::utils::convertLRGBtoSRGB(MotorwayColorLinearSpace);
	_trunkRoadColor = pvr::utils::convertLRGBtoSRGB(TrunkRoadColorLinearSpace);
	_primaryRoadColor = pvr::utils::convertLRGBtoSRGB(PrimaryRoadColorLinearSpace);
	_secondaryRoadColor = pvr::utils::convertLRGBtoSRGB(SecondaryRoadColorLinearSpace);
	_serviceRoadColor = pvr::utils::convertLRGBtoSRGB(ServiceRoadColorLinearSpace);
	_otherRoadColor = pvr::utils::convertLRGBtoSRGB(OtherRoadColorLinearSpace);
	_parkingColor = pvr::utils::convertLRGBtoSRGB(ParkingColorLinearSpace);
	_outlineColor = pvr::utils::convertLRGBtoSRGB(OutlineColorLinearSpace);

	return pvr::Result::Success;
}

/// <summary>Code in initView() will be called by PVRShell upon initialization or after a change in the rendering context.
/// Used to initialize variables that are dependent on the rendering context.(e.g. textures, vertex buffers, etc.)</summary>
/// <returns>Return Result::Success if no error occurred.</returns>
pvr::Result OGLESNavigation3D::initView()
{
	_deviceResources = std::make_unique<DeviceResources>();

	// Acquire graphics _deviceResources->context.
	_deviceResources->context = pvr::createEglContext();
	_deviceResources->context->init(getWindow(), getDisplay(), getDisplayAttributes());
	// Initialise uiRenderer
	_deviceResources->uiRenderer.init(getWidth(), getHeight(), isFullScreen(), getBackBufferColorspace() == pvr::ColorSpace::sRGB);

	_windowWidth = static_cast<uint32_t>(_deviceResources->uiRenderer.getRenderingDimX());
	_windowHeight = static_cast<uint32_t>(_deviceResources->uiRenderer.getRenderingDimY());

	Log(LogLevel::Information, "Initialising Tile Data");

	_OSMdata->initTiles();
	_numRows = _OSMdata->getNumRows();
	_numCols = _OSMdata->getNumCols();
	_tileRenderingResources.resize(_numCols);

	for (uint32_t i = 0; i < _numCols; ++i) { _tileRenderingResources[i].resize(_numRows); }
	_deviceResources->uiRenderer.getDefaultTitle()->setText("Navigation3D");
	_deviceResources->uiRenderer.getDefaultTitle()->commitUpdates();

	if (!loadTexture()) { return pvr::Result::UnknownError; }

	_deviceResources->text = _deviceResources->uiRenderer.createText();
	_deviceResources->text->setColor(0.0f, 0.0f, 0.0f, 1.0f);
	_deviceResources->text->setPixelOffset(0.0f, static_cast<float>(-int32_t(_windowHeight / 3)));
	_deviceResources->text->commitUpdates();

	createPrograms();
	setUniforms();
	createBuffers();
	_OSMdata->convertRoute(glm::dvec2(0), 0, 0, _totalRouteDistance);

	cameraInfo.translation.x = static_cast<float>(_OSMdata->getRouteData()[0].point.x);
	cameraInfo.translation.z = static_cast<float>(_OSMdata->getRouteData()[0].point.y);
	cameraInfo.translation.y = CamHeight;

	const glm::dvec2& camStartPosition = _OSMdata->getRouteData()[routeIndex].point;
	_camera.setTargetPosition(glm::vec3(camStartPosition.x, 0.f, camStartPosition.y));
	_camera.setHeight(CamHeight);
	_camera.setDistanceFromTarget(1.0f);
	_currentRoad = _OSMdata->getRouteData()[routeIndex].name;
	return pvr::Result::Success;
}

void OGLESNavigation3D::createPrograms()
{
	// Enable or disable gamma correction based on if it is automatically performed on the framebuffer or we need to do it in the shader.
	const char* defines[] = { "GAMMA_CORRECTION" };
	uint32_t numDefines = 1;

	const char* attribNames[] = { "myVertex", "texCoord", "normal" };
	const uint16_t attribIndicies[] = { 0, 1, 2 };

	// Road program
	{
		_deviceResources->roadPipe.program = pvr::utils::createShaderProgram(*this, AAVertShaderSrcFile, AAFragShaderSrcFile, attribNames, attribIndicies, ARRAY_SIZE(attribNames));
		_deviceResources->roadPipe.uniformLocation[ShaderProgramRoad::UniformTransform] = gl::GetUniformLocation(_deviceResources->roadPipe.program, "transform");
		_deviceResources->roadPipe.uniformLocation[ShaderProgramRoad::UniformColor] = gl::GetUniformLocation(_deviceResources->roadPipe.program, "myColor");

		gl::UseProgram(_deviceResources->roadPipe.program);
		gl::Uniform1i(gl::GetUniformLocation(_deviceResources->roadPipe.program, "sTexture"), 0);
	}

	// Fill program and Outline program
	{
		_deviceResources->fillPipe.program = pvr::utils::createShaderProgram(*this, VertShaderSrcFile, FragShaderSrcFile, attribNames, attribIndicies, ARRAY_SIZE(attribNames));
		_deviceResources->fillPipe.uniformLocation[ShaderProgramFill::UniformTransform] = gl::GetUniformLocation(_deviceResources->fillPipe.program, "transform");
		_deviceResources->fillPipe.uniformLocation[ShaderProgramFill::UniformColor] = gl::GetUniformLocation(_deviceResources->fillPipe.program, "myColor");
		_deviceResources->outlinePipe = _deviceResources->fillPipe;
	}

	// Building program
	{
		_deviceResources->buildingPipe.program =
			pvr::utils::createShaderProgram(*this, PerVertexLightVertShaderSrcFile, FragShaderSrcFile, attribNames, attribIndicies, ARRAY_SIZE(attribNames), defines, numDefines);
		_deviceResources->buildingPipe.uniformLocation[ShaderProgramBuilding::UniformTransform] = gl::GetUniformLocation(_deviceResources->buildingPipe.program, "transform");
		_deviceResources->buildingPipe.uniformLocation[ShaderProgramBuilding::UniformViewMatrix] = gl::GetUniformLocation(_deviceResources->buildingPipe.program, "viewMatrix");
		_deviceResources->buildingPipe.uniformLocation[ShaderProgramBuilding::UniformLightDir] = gl::GetUniformLocation(_deviceResources->buildingPipe.program, "lightDir");
		_deviceResources->buildingPipe.uniformLocation[ShaderProgramBuilding::UniformColor] = gl::GetUniformLocation(_deviceResources->buildingPipe.program, "myColor");
	}

	// Planar shadow program
	{
		_deviceResources->planarShadowPipe.program =
			pvr::utils::createShaderProgram(*this, PlanarShadowVertShaderSrcFile, PlanarShadowFragShaderSrcFile, attribNames, attribIndicies, ARRAY_SIZE(attribNames));
		_deviceResources->planarShadowPipe.uniformLocation[ShaderProgramPlanerShadow::UniformTransform] =
			gl::GetUniformLocation(_deviceResources->planarShadowPipe.program, "transform");
		_deviceResources->planarShadowPipe.uniformLocation[ShaderProgramPlanerShadow::UniformShadowMatrix] =
			gl::GetUniformLocation(_deviceResources->planarShadowPipe.program, "shadowMatrix");
	}
}

void OGLESNavigation3D::setPipelineStates(PipelineState pipelineState)
{
	gl::EnableVertexAttribArray(0); // pos
	gl::EnableVertexAttribArray(1); // tex
	gl::EnableVertexAttribArray(2); // normal

	gl::VertexAttribPointer(0, 3, GL_FLOAT, false, 0, 0);
	gl::VertexAttribPointer(1, 2, GL_FLOAT, false, 0, (void*)(sizeof(float) * 3));
	gl::VertexAttribPointer(2, 3, GL_FLOAT, false, 0, (void*)(sizeof(float) * 5));

	gl::Disable(GL_BLEND);
	gl::CullFace(GL_NONE);
	gl::Enable(GL_DEPTH);
	gl::DepthFunc(GL_LEQUAL);
	// Classic Alpha blending, but preserving framebuffer alpha to avoid artefacts on compositors that actually use the alpha value.
	gl::BlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ZERO, GL_ONE);
	gl::BlendEquation(GL_FUNC_ADD);

	switch (pipelineState)
	{
	case PipelineState::RoadPipe: gl::Enable(GL_BLEND); break;
	case PipelineState::PlanarShaderPipe:
		gl::Enable(GL_BLEND);
		gl::StencilFunc(GL_EQUAL, 0, 0xff);
		gl::StencilOp(GL_KEEP, GL_KEEP, GL_INCR_WRAP);
		gl::DepthFunc(GL_LESS);
		break;
	default: break;
	}
}

/// <summary>Load a texture from file using PVR Asset Store, create a trilinear sampler, create a description set.</summary>
/// <returns>Return true if no error occurred, false if the sampler descriptor set is not valid.</returns>
bool OGLESNavigation3D::loadTexture()
{
	/// Road Texture
	pvr::Texture tex = pvr::textureLoad(*getAssetStream(RoadTexFile), pvr::TextureFileFormat::PVR);

	pvr::utils::TextureUploadResults uploadResultRoadTex = pvr::utils::textureUpload(tex, _deviceResources->context->getApiVersion() == pvr::Api::OpenGLES2, true);

	_deviceResources->roadTex = uploadResultRoadTex.image;
	gl::BindTexture(GL_TEXTURE_2D, _deviceResources->roadTex);
	gl::TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	gl::TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	gl::TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	gl::TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	//	gl::TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT)

	/// Font Texture
	tex = pvr::textureLoad(*getAssetStream(FontFile), pvr::TextureFileFormat::PVR);
	pvr::utils::TextureUploadResults uploadResulFontTex = pvr::utils::textureUpload(tex, _deviceResources->context->getApiVersion() == pvr::Api::OpenGLES2, true);
	_deviceResources->fontTex = uploadResulFontTex.image;
	gl::BindTexture(GL_TEXTURE_2D, _deviceResources->fontTex);
	gl::TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	gl::TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	gl::TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	gl::TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	return true;
}

/// <summary>Setup uniforms used for drawing the map. </summary>
void OGLESNavigation3D::setUniforms()
{
	_perspectiveMatrix = _deviceResources->uiRenderer.getScreenRotation() *
		pvr::math::perspectiveFov(_deviceResources->context->getApiVersion(), glm::radians(45.0f), float(_windowWidth), float(_windowHeight), nearClip, farClip);
}

/// <summary>Creates vertex and index buffers and records the secondary command buffers for each tile.</summary>
void OGLESNavigation3D::createBuffers()
{
	uint32_t col = 0;
	uint32_t row = 0;

	for (auto& tileCol : _OSMdata->getTiles())
	{
		for (Tile& tile : tileCol)
		{
			_tileRenderingResources[col][row] = std::make_unique<TileRenderingResources>();
			TileRenderingResources& tileResource = *_tileRenderingResources[col][row];

			// Set the min and max coordinates for the tile
			tile.screenMin = remap(tile.min, _OSMdata->getTiles()[0][0].min, _OSMdata->getTiles()[0][0].max, glm::dvec2(-5, -5), glm::dvec2(5, 5));
			tile.screenMax = remap(tile.max, _OSMdata->getTiles()[0][0].min, _OSMdata->getTiles()[0][0].max, glm::dvec2(-5, -5), glm::dvec2(5, 5));

			// Create vertices for tile
			for (auto nodeIterator = tile.nodes.begin(); nodeIterator != tile.nodes.end(); ++nodeIterator)
			{
				nodeIterator->second.index = static_cast<uint32_t>(tile.vertices.size());

				glm::vec2 remappedPos =
					glm::vec2(remap(nodeIterator->second.coords, _OSMdata->getTiles()[0][0].min, _OSMdata->getTiles()[0][0].max, glm::dvec2(-5, -5), glm::dvec2(5, 5)));
				glm::vec3 vertexPos = glm::vec3(remappedPos.x, nodeIterator->second.height, remappedPos.y);

				Tile::VertexData vertData(vertexPos, nodeIterator->second.texCoords);

				tile.vertices.push_back(vertData);
			}

			// Add car parking to indices
			tileResource.parkingNum = generateIndices(tile, tile.parkingWays);

			// Add road area ways to indices
			tileResource.areaNum = generateIndices(tile, tile.areaWays);

			// Add road area outlines to indices
			tileResource.roadAreaOutlineNum = generateIndices(tile, tile.areaOutlineIds);

			// Add roads to indices
			tileResource.motorwayNum = generateIndices(tile, tile.roadWays, RoadTypes::Motorway);
			tileResource.trunkRoadNum = generateIndices(tile, tile.roadWays, RoadTypes::Trunk);
			tileResource.primaryRoadNum = generateIndices(tile, tile.roadWays, RoadTypes::Primary);
			tileResource.secondaryRoadNum = generateIndices(tile, tile.roadWays, RoadTypes::Secondary);
			tileResource.serviceRoadNum = generateIndices(tile, tile.roadWays, RoadTypes::Service);
			tileResource.otherRoadNum = generateIndices(tile, tile.roadWays, RoadTypes::Other);

			// Add buildings to indices
			tileResource.buildNum = generateIndices(tile, tile.buildWays);

			// Add inner ways to indices
			tileResource.innerNum = generateIndices(tile, tile.innerWays);

			generateNormals(tile, static_cast<uint32_t>(tile.indices.size() - (tileResource.innerNum + tileResource.buildNum)), tileResource.buildNum);

			// Create vertex and index buffers
			// Interleaved vertex buffer (vertex position + texCoord)
			gl::GenBuffers(1, &_tileRenderingResources[col][row]->vbo);
			const uint32_t vboSize = static_cast<uint32_t>(tile.vertices.size() * sizeof(tile.vertices[0]));
			gl::BindBuffer(GL_ARRAY_BUFFER, _tileRenderingResources[col][row]->vbo);
			gl::BufferData(GL_ARRAY_BUFFER, vboSize, tile.vertices.data(), GL_STATIC_DRAW);

			const uint32_t iboSize = static_cast<uint32_t>(tile.indices.size() * sizeof(tile.indices[0]));
			gl::GenBuffers(1, &_tileRenderingResources[col][row]->ibo);
			gl::BindBuffer(GL_ELEMENT_ARRAY_BUFFER, _tileRenderingResources[col][row]->ibo);
			gl::BufferData(GL_ELEMENT_ARRAY_BUFFER, iboSize, tile.indices.data(), GL_STATIC_DRAW);
			row++;
		}
		row = 0;
		col++;
	}
}

/// <summary>Main rendering loop function of the program. The shell will call this function every frame.</summary>
/// <returns>Return Result::Success if no error occurred.</returns>
pvr::Result OGLESNavigation3D::renderFrame()
{
	updateAnimation();
	calculateTransform();
	calculateClipPlanes();

	// Update commands
	executeCommands();

	if (this->shouldTakeScreenshot()) { pvr::utils::takeScreenshot(this->getScreenshotFileName(), this->getWidth(), this->getHeight()); }

	_deviceResources->context->swapBuffers();
	return pvr::Result::Success;
}

/// <summary>Handle user input.</summary>
void OGLESNavigation3D::updateAnimation()
{
	if (_OSMdata->getRouteData().size() == 0) { return; }

	static const float rotationOffset = -90.f;
	static bool turning = false;
	static float animTime = 0.0f;
	static float rotateTime = 0.0f;
	static float currentRotationTime = 0.0f;
	static float currentRotation = static_cast<float>(_OSMdata->getRouteData()[routeIndex].rotation);
	static glm::dvec2 camStartPosition = _OSMdata->getRouteData()[routeIndex].point;
	static glm::dvec2 camEndPosition;
	static glm::dvec2 camLerpPos = glm::dvec2(0.0f);
	static bool destinationReached = false;
	static float routeRestartTime = 0.;
	float dt = float(getFrameTime());
	camEndPosition = _OSMdata->getRouteData()[routeIndex + 1].point;
	const uint32_t lastRouteIndex = routeIndex;
	_keyFrameTime = calculateRouteKeyFrameTime(camStartPosition, camEndPosition);
	// Do the translation if the camera is not turning
	if (destinationReached && routeRestartTime >= 2000)
	{
		destinationReached = false;
		routeRestartTime = 0.0f;
	}
	if (destinationReached)
	{
		routeRestartTime += dt;
		return;
	}

	if (!turning)
	{
		// Interpolate between two positions.
		camLerpPos = glm::mix(camStartPosition, camEndPosition, animTime / _keyFrameTime);

		cameraInfo.translation = glm::vec3(camLerpPos.x, CamHeight, camLerpPos.y);
		_camera.setTargetPosition(glm::vec3(camLerpPos.x, 0.0f, camLerpPos.y));
		_camera.setTargetLookAngle(currentRotation + rotationOffset);
	}
	if (animTime >= _keyFrameTime)
	{
		const float r1 = static_cast<float>(_OSMdata->getRouteData()[routeIndex].rotation);
		const float r2 = static_cast<float>(_OSMdata->getRouteData()[routeIndex + 1].rotation);

		if ((!turning && fabs(r2 - r1) > 3.f) || (turning))
		{
			float diff = r2 - r1;
			float absDiff = fabs(diff);
			if (absDiff > 180.f)
			{
				if (diff > 0.f) // if the difference is positive angle then do negative rotation
					diff = -(360.f - absDiff);
				else // else do a positive rotation
					diff = (360.f - absDiff);
			}
			absDiff = fabs(diff); // get the abs
			rotateTime = 18.f * absDiff; // 18ms for an angle * angle diff

			currentRotationTime += dt;
			currentRotationTime = glm::clamp(currentRotationTime, 0.0f, rotateTime);
			if (currentRotationTime >= rotateTime) { turning = false; }
			else
			{
				turning = true;
				currentRotation = glm::mix(r1, r1 + diff, currentRotationTime / rotateTime);
				_camera.setTargetLookAngle(currentRotation + rotationOffset);
			}
		}
	}
	if (animTime >= _keyFrameTime && !turning)
	{
		turning = false;
		currentRotationTime = 0.0f;
		rotateTime = 0.0f;
		// Iterate through the route
		if (++routeIndex == _OSMdata->getRouteData().size() - 1)
		{
			currentRotation = static_cast<float>(_OSMdata->getRouteData()[0].rotation);
			routeIndex = 0;
			destinationReached = true;
			routeRestartTime = 0.f;
		}
		else
		{
			currentRotation = static_cast<float>(_OSMdata->getRouteData()[routeIndex].rotation);
		}
		animTime = 0.0f;
		// Reset the route.
		camStartPosition = _OSMdata->getRouteData()[routeIndex].point;
	}
	if (lastRouteIndex != routeIndex) { _currentRoad = _OSMdata->getRouteData()[routeIndex].name; }
	_viewMatrix = _camera.getViewMatrix();

	animTime += dt;
}

/// <summary>Calculate the View Projection Matrix.</summary>
void OGLESNavigation3D::calculateTransform()
{
	_lightDir = glm::normalize(glm::mat3(_viewMatrix) * glm::vec3(0.25f, -2.4f, -1.15f));
	_viewProjMatrix = _perspectiveMatrix * _viewMatrix;
}

/// <summary>Record the primary command buffer.</summary>
void OGLESNavigation3D::executeCommands()
{
	gl::ClearColor(_clearColor.r, _clearColor.g, _clearColor.b, _clearColor.a);
	gl::Clear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
	for (uint32_t i = 0; i < _numCols; ++i)
	{
		for (uint32_t j = 0; j < _numRows; ++j)
		{
			// Only queue up commands if the tile is visible.
			if (inFrustum(_OSMdata->getTiles()[i][j].screenMin, _OSMdata->getTiles()[i][j].screenMax)) { executeCommands(*_tileRenderingResources[i][j]); }
		}
	}

	_deviceResources->text->setText(_currentRoad);
	_deviceResources->text->commitUpdates();

	// Render UI elements.
	_deviceResources->uiRenderer.beginRendering();
	_deviceResources->text->render();
	_deviceResources->uiRenderer.getDefaultTitle()->render();
	_deviceResources->uiRenderer.getSdkLogo()->render();
	_deviceResources->uiRenderer.endRendering();
}

/// <summary>Capture frustum planes from the current View Projection matrix.</summary>
void OGLESNavigation3D::calculateClipPlanes() { pvr::math::getFrustumPlanes(_deviceResources->context->getApiVersion(), _viewProjMatrix, _viewFrustum); }

/// <summary>Tests whether a 2D bounding box is intersected or enclosed by a view frustum.
/// Only the near, far, left and right planes of the view frustum are taken into consideration to optimize the intersection test.</summary>
/// <param name="min">The minimum co-ordinates of the bounding box.</param>
/// <param name="max">The maximum co-ordinates of the bounding box.</param>
/// <returns>Returns True if inside the view frustum, false if outside.</returns>
bool OGLESNavigation3D::inFrustum(glm::vec2 min, glm::vec2 max)
{
	// Test the axis-aligned bounding box against each frustum plane,
	// cull if all points are outside of one the view frustum planes.
	pvr::math::AxisAlignedBox aabb;
	aabb.setMinMax(glm::vec3(min.x, 0.f, min.y), glm::vec3(max.x, 5.0f, max.y));
	return pvr::math::aabbInFrustum(aabb, _viewFrustum);
}
template<class ShaderProgram>
void OGLESNavigation3D::bindProgram(const ShaderProgram& program)
{
	if (program.program != _glesStates.boundProgram)
	{
		gl::UseProgram(program.program);
		_glesStates.boundProgram = program.program;
	}
}

void OGLESNavigation3D::executeCommands(const TileRenderingResources& tileRes)
{
	gl::EnableVertexAttribArray(0);
	gl::EnableVertexAttribArray(1);
	gl::EnableVertexAttribArray(2);

	gl::BindBuffer(GL_ARRAY_BUFFER, tileRes.vbo);
	gl::BindBuffer(GL_ELEMENT_ARRAY_BUFFER, tileRes.ibo);
	gl::VertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Tile::VertexData), (const void*)0);
	gl::VertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Tile::VertexData), (const void*)(sizeof(float) * 3));
	gl::VertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(Tile::VertexData), (const void*)(sizeof(float) * 5));
	gl::Disable(GL_CULL_FACE);
	gl::DepthMask(GL_TRUE);
	gl::Enable(GL_DEPTH_TEST);

	gl::DepthFunc(GL_LEQUAL);
	gl::FrontFace(GL_CCW);
	gl::Disable(GL_BLEND);

	uint32_t offset = 0;
	const uint32_t parkingNum = tileRes.parkingNum;
	const uint32_t areaNum = tileRes.areaNum;
	const uint32_t motorwayNum = tileRes.motorwayNum;
	const uint32_t roadAreaOutlineNum = tileRes.roadAreaOutlineNum;
	const uint32_t trunkRoadNum = tileRes.trunkRoadNum;
	const uint32_t primaryRoadNum = tileRes.primaryRoadNum;
	const uint32_t secondaryRoadNum = tileRes.secondaryRoadNum;
	const uint32_t serviceRoadNum = tileRes.serviceRoadNum;
	const uint32_t otherRoadNum = tileRes.otherRoadNum;
	const uint32_t buildNum = tileRes.buildNum;
	const uint32_t innerNum = tileRes.innerNum;
	if (parkingNum > 0)
	{
		bindProgram(_deviceResources->fillPipe);
		gl::UniformMatrix4fv(_deviceResources->fillPipe.uniformLocation[ShaderProgramFill::Uniform::UniformTransform], 1, false, glm::value_ptr(_viewProjMatrix));
		gl::Uniform4fv(_deviceResources->fillPipe.uniformLocation[ShaderProgramFill::Uniform::UniformColor], 1, glm::value_ptr(_parkingColor));
		gl::DrawElements(GL_TRIANGLES, parkingNum, GL_UNSIGNED_INT, nullptr);
		offset += parkingNum * sizeof(uint32_t);
	}
	if (areaNum > 0)
	{
		const ShaderProgramFill& program = _deviceResources->fillPipe;
		bindProgram(program);
		gl::UniformMatrix4fv(program.uniformLocation[ShaderProgramFill::Uniform::UniformTransform], 1, false, glm::value_ptr(_viewProjMatrix));
		gl::Uniform4fv(program.uniformLocation[ShaderProgramFill::Uniform::UniformColor], 1, glm::value_ptr(_roadAreaColor));
		gl::DrawElements(GL_TRIANGLES, areaNum, GL_UNSIGNED_INT, reinterpret_cast<const void*>(static_cast<uintptr_t>(offset)));
		offset += areaNum * sizeof(uint32_t);
	}
	if (roadAreaOutlineNum > 0)
	{
		const ShaderProgramFill& program = _deviceResources->outlinePipe;
		bindProgram(program);
		gl::UniformMatrix4fv(program.uniformLocation[ShaderProgramFill::Uniform::UniformTransform], 1, false, glm::value_ptr(_viewProjMatrix));
		gl::Uniform4fv(program.uniformLocation[ShaderProgramFill::Uniform::UniformColor], 1, glm::value_ptr(_outlineColor));
		gl::DrawElements(GL_LINES, roadAreaOutlineNum, GL_UNSIGNED_INT, reinterpret_cast<const void*>(static_cast<uintptr_t>(offset)));
		offset += roadAreaOutlineNum * sizeof(uint32_t);
	}

	// Draw the roads
	const ShaderProgramRoad& program = _deviceResources->roadPipe;
	gl::Enable(GL_BLEND);
	// Classic Alpha blending, but preserving framebuffer alpha to avoid artefacts on compositors that actually use the alpha value.
	gl::BlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ZERO, GL_ONE);
	gl::BlendEquation(GL_FUNC_ADD);

	// Motorways
	bindProgram(program);
	gl::UniformMatrix4fv(program.uniformLocation[ShaderProgramRoad::Uniform::UniformTransform], 1, false, glm::value_ptr(_viewProjMatrix));
	bindTexture(0, _deviceResources->roadTex);
	if (motorwayNum > 0)
	{
		gl::Uniform4fv(program.uniformLocation[ShaderProgramRoad::UniformColor], 1, glm::value_ptr(_motorwayColor));
		gl::DrawElements(GL_TRIANGLES, motorwayNum, GL_UNSIGNED_INT, reinterpret_cast<const void*>(static_cast<uintptr_t>(offset)));
		offset += motorwayNum * sizeof(uint32_t);
	}

	// Trunk roads
	if (trunkRoadNum > 0)
	{
		gl::Uniform4fv(program.uniformLocation[ShaderProgramRoad::UniformColor], 1, glm::value_ptr(_trunkRoadColor));
		gl::DrawElements(GL_TRIANGLES, trunkRoadNum, GL_UNSIGNED_INT, reinterpret_cast<const void*>(static_cast<uintptr_t>(offset)));
		offset += trunkRoadNum * sizeof(uint32_t);
	}

	// Primary roads
	if (primaryRoadNum > 0)
	{
		gl::Uniform4fv(program.uniformLocation[ShaderProgramRoad::UniformColor], 1, glm::value_ptr(_primaryRoadColor));
		gl::DrawElements(GL_TRIANGLES, primaryRoadNum, GL_UNSIGNED_INT, reinterpret_cast<const void*>(static_cast<uintptr_t>(offset)));
		offset += primaryRoadNum * sizeof(uint32_t);
	}

	// Road roads
	if (secondaryRoadNum > 0)
	{
		gl::Uniform4fv(program.uniformLocation[ShaderProgramRoad::UniformColor], 1, glm::value_ptr(_secondaryRoadColor));
		gl::DrawElements(GL_TRIANGLES, secondaryRoadNum, GL_UNSIGNED_INT, reinterpret_cast<const void*>(static_cast<uintptr_t>(offset)));
		offset += secondaryRoadNum * sizeof(uint32_t);
	}
	// Service Roads
	if (serviceRoadNum > 0)
	{
		gl::Uniform4fv(program.uniformLocation[ShaderProgramRoad::UniformColor], 1, glm::value_ptr(_serviceRoadColor));
		gl::DrawElements(GL_TRIANGLES, serviceRoadNum, GL_UNSIGNED_INT, reinterpret_cast<const void*>(static_cast<uintptr_t>(offset)));
		offset += serviceRoadNum * sizeof(uint32_t);
	}

	// Other (any other roads)
	if (otherRoadNum > 0)
	{
		gl::Uniform4fv(program.uniformLocation[ShaderProgramRoad::UniformColor], 1, glm::value_ptr(_otherRoadColor));
		gl::DrawElements(GL_TRIANGLES, otherRoadNum, GL_UNSIGNED_INT, reinterpret_cast<const void*>(static_cast<uintptr_t>(offset)));
		offset += otherRoadNum * sizeof(uint32_t);
	}
	// Draw the buildings & shadows
	if (buildNum > 0)
	{
		const ShaderProgramBuilding& buildingProgram = _deviceResources->buildingPipe;
		bindProgram(buildingProgram);

		gl::UniformMatrix4fv(buildingProgram.uniformLocation[ShaderProgramBuilding::Uniform::UniformTransform], 1, false, glm::value_ptr(_viewProjMatrix));
		gl::UniformMatrix4fv(buildingProgram.uniformLocation[ShaderProgramBuilding::Uniform::UniformViewMatrix], 1, false, glm::value_ptr(_viewMatrix));
		gl::Uniform3fv(buildingProgram.uniformLocation[ShaderProgramBuilding::Uniform::UniformLightDir], 1, glm::value_ptr(_lightDir));
		gl::Uniform4fv(buildingProgram.uniformLocation[ShaderProgramBuilding::Uniform::UniformColor], 1, glm::value_ptr(BuildingColorLinearSpace));

		gl::DrawElements(GL_TRIANGLES, buildNum, GL_UNSIGNED_INT, reinterpret_cast<const void*>(static_cast<uintptr_t>(offset)));

		// Planar shadows for buildings only.
		// Classic Alpha blending, but preserving framebuffer alpha to avoid artefacts on compositors that actually use the alpha value.
		gl::Enable(GL_BLEND);
		gl::BlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ZERO, GL_ONE);
		gl::StencilFunc(GL_EQUAL, 0x0, 0xff);
		gl::StencilOp(GL_KEEP, GL_KEEP, GL_INCR_WRAP);
		gl::Enable(GL_STENCIL_TEST);
		gl::BlendEquation(GL_FUNC_ADD);
		const ShaderProgramPlanerShadow& shadowProgram = _deviceResources->planarShadowPipe;
		bindProgram(shadowProgram);
		gl::UniformMatrix4fv(shadowProgram.uniformLocation[ShaderProgramPlanerShadow::Uniform::UniformTransform], 1, false, glm::value_ptr(_viewProjMatrix));
		gl::UniformMatrix4fv(shadowProgram.uniformLocation[ShaderProgramPlanerShadow::Uniform::UniformShadowMatrix], 1, false, glm::value_ptr(_shadowMatrix));

		gl::DrawElements(GL_TRIANGLES, buildNum, GL_UNSIGNED_INT, reinterpret_cast<const void*>(static_cast<uintptr_t>(offset)));
		offset += buildNum * sizeof(uint32_t);
		gl::Disable(GL_STENCIL_TEST);
		gl::Disable(GL_BLEND);
	}

	if (innerNum > 0)
	{
		const ShaderProgramFill& program2 = _deviceResources->fillPipe;
		bindProgram(program2);
		gl::UniformMatrix4fv(program2.uniformLocation[ShaderProgramFill::Uniform::UniformTransform], 1, false, glm::value_ptr(_viewProjMatrix));

		gl::Uniform4fv(program2.uniformLocation[ShaderProgramFill::Uniform::UniformColor], 1, glm::value_ptr(_clearColor));
		gl::DrawElements(GL_TRIANGLES, innerNum, GL_UNSIGNED_INT, reinterpret_cast<const void*>(static_cast<uintptr_t>(offset)));
		offset += innerNum * sizeof(uint32_t);
	}
}

/// <summary>Code in releaseView() will be called by Shell when the application quits or before a change in the rendering context. </summary>
/// <returns>Result::Success if no error occurred.</returns>
pvr::Result OGLESNavigation3D::releaseView()
{
	// Clean up tile rendering resource data.
	for (auto& resourceCol : _tileRenderingResources)
	{
		for (auto& resource : resourceCol) { resource.reset(0); }
	}

	_OSMdata.reset();

	// Reset context and associated resources.
	_deviceResources.reset(0);

	return pvr::Result::Success;
}

/// <sumary>Code in quitApplication() will be called by PVRShell once per run, just before exiting the program.
/// If the rendering context is lost, quitApplication() will not be called.</summary>
/// <returns>Return Result::Success if no error occurred.</returns>
pvr::Result OGLESNavigation3D::quitApplication() { return pvr::Result::Success; }

/// <summary>This function must be implemented by the user of the shell. The user should return its pvr::Shell object defining the behaviour of the application.</summary>
/// <returns>Return a unique ptr to the demo supplied by the user.</returns>
std::unique_ptr<pvr::Shell> pvr::newDemo() { return std::make_unique<OGLESNavigation3D>(); }
