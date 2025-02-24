/*!
\brief Implements a 2D navigation renderer.
\file OpenGLESNavigation2D.cpp
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
#include "PVRShell/PVRShell.h"
#include "PVRAssets/PVRAssets.h"
#include "PVRUtils/PVRUtilsGles.h"
#include "../../common/NavDataProcess.h"

float MapScreenAlignRotation = 0.0f;
const float CameraMoveSpeed = 100.f;
const float CameraRotationSpeed = 80.f;
const float CamRotationTime = 5000.f;
const pvr::utils::VertexBindings_Name vertexBindings[] = {
	{ "POSITION", "inVertex" },
	{ "UV0", "inTexCoords" },
};

namespace AttributeIndices {
enum Enum
{
	VertexArray = 0,
	TexCoordArray = 2
};
}

const pvr::StringHash SpriteFileNames[BuildingType::None] = {
	pvr::StringHash("shop.pvr"),
	pvr::StringHash("bar.pvr"),
	pvr::StringHash("cafe.pvr"),
	pvr::StringHash("fastfood.pvr"),
	pvr::StringHash("pub.pvr"),
	pvr::StringHash("college.pvr"),
	pvr::StringHash("library.pvr"),
	pvr::StringHash("university.pvr"),
	pvr::StringHash("ATM.pvr"),
	pvr::StringHash("bank.pvr"),
	pvr::StringHash("restaurant.pvr"),
	pvr::StringHash("doctors.pvr"),
	pvr::StringHash("dentist.pvr"),
	pvr::StringHash("hospital.pvr"),
	pvr::StringHash("pharmacy.pvr"),
	pvr::StringHash("cinema.pvr"),
	pvr::StringHash("casino.pvr"),
	pvr::StringHash("theatre.pvr"),
	pvr::StringHash("fire.pvr"),
	pvr::StringHash("courthouse.pvr"),
	pvr::StringHash("police.pvr"),
	pvr::StringHash("postoffice.pvr"),
	pvr::StringHash("toilets.pvr"),
	pvr::StringHash("worship.pvr"),
	pvr::StringHash("petrol.pvr"),
	pvr::StringHash("parking.pvr"),
	pvr::StringHash("other.pvr"),
	pvr::StringHash("postbox.pvr"),
	pvr::StringHash("vets.pvr"),
	pvr::StringHash("embassy.pvr"),
	pvr::StringHash("hairdresser.pvr"),
	pvr::StringHash("butcher.pvr"),
	pvr::StringHash("optician.pvr"),
	pvr::StringHash("florist.pvr"),
};

struct Icon
{
	pvr::ui::Image image;
};

struct Label
{
	pvr::ui::Text text;
};

struct AmenityIconGroup
{
	pvr::ui::PixelGroup group;
	Icon icon;
	IconData iconData;
};

struct AmenityLabelGroup
{
	pvr::ui::PixelGroup group;
	Label label;
	IconData iconData;
};

enum class CameraMode
{
	Auto,
	Manual
};

struct TileRenderProperties
{
	uint32_t parkingNum;
	uint32_t buildNum;
	uint32_t innerNum;
	uint32_t areaNum;
	uint32_t serviceRoadNum;
	uint32_t otherRoadNum;
	uint32_t secondaryRoadNum;
	uint32_t primaryRoadNum;
	uint32_t trunkRoadNum;
	uint32_t motorwayNum;
};

struct TileRenderingResources
{
	GLuint vbo;
	GLuint ibo;
	GLuint vao;

	std::shared_ptr<pvr::ui::UIRenderer> renderer;

	pvr::ui::Font font;
	pvr::ui::PixelGroup tileGroup[LOD::Count];
	pvr::ui::PixelGroup cameraRotateGroup[LOD::Count];
	std::vector<Label> labels[LOD::Count];
	std::vector<AmenityIconGroup> amenityIcons[LOD::Count];
	std::vector<AmenityLabelGroup> amenityLabels[LOD::Count];

	uint32_t col;
	uint32_t row;
	TileRenderProperties properties;

	void reset()
	{
		gl::DeleteBuffers(1, &vbo);
		gl::DeleteBuffers(1, &ibo);
		gl::DeleteBuffers(1, &vao);
		for (uint32_t i = 0; i < static_cast<uint32_t>(LOD::Count); ++i)
		{
			cameraRotateGroup[i].reset();
			labels[i].clear();
			amenityIcons[i].clear();
			amenityLabels[i].clear();
			tileGroup[i].reset();
		}
		font.reset();
		renderer.reset();
	}

	// Sprites for icons
	pvr::ui::Image spriteImages[BuildingType::None];

	TileRenderingResources() {}
};

struct DeviceResources
{
	// Graphics context
	pvr::EglContext context;

	// Programs
	GLuint roadProgram;
	GLuint fillProgram;

	GLint roadColorUniformLocation;
	GLint roadTransformUniformLocation;

	GLint fillColorUniformLocation;
	GLint fillTransformUniformLocation;

	pvr::utils::VertexConfiguration vertexConfiguration;

	// Frame and primary command buffers
	GLuint fbo;

	// Texture atlas meta data.
	pvr::TextureHeader texAtlasHeader;
	// Array of UV offsets into the texture atlas.
	pvr::Rectanglef atlasOffsets[BuildingType::None];
	// Raw texture atlas containing all sprites.
	GLuint texAtlas;

	// Font texture data
	GLuint fontTexture;
	pvr::Texture fontHeader;
	GLuint fontSampler;

	std::vector<TileRenderingResources*> renderqueue;

	GLint defaultFbo;

	// UIRenderer used to display text
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

// Alpha, luminance texture.
const char* MapFile = "map.osm";
const char* FontFile = "font.pvr";
float scales[LOD::Count] = { 10.0f, 7.0f, 5.0f, 3.0f, 2.0f };
float MapScales[LOD::Count] = { 11.0f, 10.0f, 7.0f, 5.0f, 2.0f };

/// <summary>implementing the pvr::Shell functions.<summary>
class OGLESNavigation2D : public pvr::Shell
{
	std::unique_ptr<NavDataProcess> _OSMdata;

	// Graphics resources
	std::unique_ptr<DeviceResources> _deviceResources;
	std::vector<std::vector<TileRenderingResources>> _tileRenderingResources;

	uint16_t _currentScaleLevel;

	// Uniforms
	glm::mat4 _mapMVPMtx;

	// Transformation variables
	glm::vec2 _translation;
	float _scale;
	glm::mat4 _projMtx;
	float _rotation;

	pvr::math::ViewingFrustum _viewFrustum;

	// Map tile dimensions
	uint32_t _numRows;
	uint32_t _numCols;

	float _totalRouteDistance;
	float _keyFrameTime;
	CameraMode _cameraMode;

	pvr::ui::GLStateTracker _stateTracker;

	glm::dvec2 _mapWorldDim;

	float _timePassed;
	bool _increaseScale;
	bool _scaleChange;
	bool _updateRotation;
	bool _turning;
	uint16_t _previousScaleLevel;
	uint32_t _routeIndex;
	float _animTime;
	float _rotateTime;
	float _rotateAnimTime;
	float _screenWidth, _screenHeight;

	glm::vec4 _clearColor;

	glm::vec4 _roadAreaColor;
	glm::vec4 _motorwayColor;
	glm::vec4 _trunkRoadColor;
	glm::vec4 _primaryRoadColor;
	glm::vec4 _secondaryRoadColor;
	glm::vec4 _serviceRoadColor;
	glm::vec4 _otherRoadColor;
	glm::vec4 _parkingColor;
	glm::vec4 _buildingColor;
	glm::vec4 _outlineColor;

	glm::mat4 _mapProjMtx;

public:
	// PVR shell functions
	pvr::Result initApplication() override;
	pvr::Result quitApplication() override;
	pvr::Result initView() override;
	pvr::Result releaseView() override;
	pvr::Result renderFrame() override;

	void bindAndClearFramebuffer();
	void setDefaultStates();
	void initializeRenderers(TileRenderingResources* begin, TileRenderingResources* end, const uint32_t col, const uint32_t row);
	void createBuffers();
	void loadTexture();
	void initRoute();

	void render();
	void updateLabels(uint32_t col, uint32_t row);
	void updateAmenities(uint32_t col, uint32_t row);
	void updateGroups(uint32_t col, uint32_t row);
	void updateAnimation();
	void calculateClipPlanes();
	bool inFrustum(glm::vec2 min, glm::vec2 max);
	void renderTile(const Tile& tile, TileRenderingResources& renderingResources);
	void createUIRendererItems();
	void eventMappedInput(pvr::SimplifiedInput e) override;
	void resetCameraVariables();
	void updateSubtitleText();

	OGLESNavigation2D() : _projMtx(1.0), _rotation(0.0f), _totalRouteDistance(0.0f), _cameraMode(CameraMode::Auto) {}

	void handleInput();

private:
	void recalculateTheScale()
	{
		pvr::DisplayAttributes displayAttrib;
		float scaleFactor;
		if (isScreenRotated()) { scaleFactor = static_cast<float>(getHeight()) / displayAttrib.height; }
		else
		{
			scaleFactor = static_cast<float>(getWidth()) / displayAttrib.width;
		}
		for (uint32_t i = 0; i < LOD::Count; ++i)
		{
			MapScales[i] = MapScales[i] * scaleFactor;
			scales[i] = scales[i] * scaleFactor;
		}
	}
};

void OGLESNavigation2D::resetCameraVariables()
{
	_routeIndex = 0;
	_currentScaleLevel = LOD::L4;
	_previousScaleLevel = _currentScaleLevel;
	_scale = scales[_currentScaleLevel];
	_rotation = static_cast<float>(_OSMdata->getRouteData()[_routeIndex].rotation);
	_keyFrameTime = 0.0f;

	_timePassed = 0.0f;
	_animTime = 0.0f;
	_updateRotation = true;
	_rotateTime = 0.0f;
	_rotateAnimTime = 0.0f;
	_turning = false;
	_increaseScale = false;
	_scaleChange = false;
	_translation = _OSMdata->getRouteData()[_routeIndex].point;
}

/// <summary>Handles user input and updates live variables accordingly.<summary>
void OGLESNavigation2D::eventMappedInput(pvr::SimplifiedInput e)
{
	switch (e)
	{
	case pvr::SimplifiedInput::ActionClose: this->exitShell(); break;
	case pvr::SimplifiedInput::Action1:
		if (_cameraMode == CameraMode::Auto) { _cameraMode = CameraMode::Manual; }
		else
		{
			_cameraMode = CameraMode::Auto;
		}
		resetCameraVariables();
		updateSubtitleText();
		break;
	default: break;
	}
}

/// <summary>Code in initApplication() will be called by Shell once per run, before the rendering context is created.
/// Used to initialize variables that are not dependent on it(e.g.external modules, loading meshes, etc.).If the rendering
/// context is lost, initApplication() will not be called again.</summary>
/// <returns>Result::Success if no error occurred.</returns>
pvr::Result OGLESNavigation2D::initApplication()
{
	// Disable gamma correction in the framebuffer.
	setBackBufferColorspace(pvr::ColorSpace::lRGB);
	// WARNING: This should not be done lightly. This example only passes through textures or hard coded colour values.
	// If you do that, you should ensure that your textures will end up giving you the correct values. If you use
	// normal sRGB textures, they will NOT provide you with the values you except (they will look too dark).
	// Also linear operations will not work correctly. Again in this example this is not a problem as we have tweaked
	// all values manually for visual effect and there is no lighting maths going on.

	setDepthBitsPerPixel(0);
	setStencilBitsPerPixel(0);

	// Load and process the map.
	_OSMdata = std::make_unique<NavDataProcess>(getAssetStream(MapFile), glm::ivec2(getWidth(), getHeight()));
	pvr::Result result = _OSMdata->loadAndProcessData();

	Log(LogLevel::Information, "MAP SIZE IS: [ %d x %d ] TILES", _OSMdata->getNumRows(), _OSMdata->getNumCols());

	// perform gamma correction of the linear space colours so that they can do used directly without further thinking about Linear/sRGB colour space conversions
	// This should not be done lightly. This example only passes through hard coded colour values and uses them directly without applying any
	// maths to their values and so can be performed safely.
	_clearColor = pvr::utils::convertLRGBtoSRGB(ClearColorLinearSpace);
	_roadAreaColor = pvr::utils::convertLRGBtoSRGB(RoadAreaColorLinearSpace);
	_motorwayColor = pvr::utils::convertLRGBtoSRGB(MotorwayColorLinearSpace);
	_trunkRoadColor = pvr::utils::convertLRGBtoSRGB(TrunkRoadColorLinearSpace);
	_primaryRoadColor = pvr::utils::convertLRGBtoSRGB(PrimaryRoadColorLinearSpace);
	_secondaryRoadColor = pvr::utils::convertLRGBtoSRGB(SecondaryRoadColorLinearSpace);
	_serviceRoadColor = pvr::utils::convertLRGBtoSRGB(ServiceRoadColorLinearSpace);
	_otherRoadColor = pvr::utils::convertLRGBtoSRGB(OtherRoadColorLinearSpace);
	_parkingColor = pvr::utils::convertLRGBtoSRGB(ParkingColorLinearSpace);
	_buildingColor = pvr::utils::convertLRGBtoSRGB(BuildingColorLinearSpace);
	_outlineColor = pvr::utils::convertLRGBtoSRGB(OutlineColorLinearSpace);

	return result;
}

/// <summary>Code in initView() will be called by Shell upon initialization or after a change
/// in the rendering context. Used to initialize variables that are dependent on the
/// rendering context(e.g.textures, vertex buffers, etc.).</summary>
/// <returns>Result::Success if no error occurred.</returns>
pvr::Result OGLESNavigation2D::initView()
{
	_deviceResources = std::make_unique<DeviceResources>();

	_deviceResources->context = pvr::createEglContext();
	_deviceResources->context->init(getWindow(), getDisplay(), getDisplayAttributes(), pvr::Api::OpenGLES3);

	recalculateTheScale();
	resetCameraVariables();

	gl::GetIntegerv(GL_FRAMEBUFFER_BINDING, &_deviceResources->defaultFbo);
	gl::BindFramebuffer(GL_FRAMEBUFFER, _deviceResources->defaultFbo);

	if (_deviceResources->context->getApiVersion() == pvr::Api::OpenGLES2)
	{
		if (!gl::isGlExtensionSupported("GL_OES_vertex_array_object"))
		{
			setExitMessage("Unable to create vertex array objects as extension 'GL_OES_vertex_array_object' is unsupported.");
			return pvr::Result::InitializationError;
		}
	}

	loadTexture();

	_numRows = _OSMdata->getNumRows();
	_numCols = _OSMdata->getNumCols();

	Log(LogLevel::Information, "Initialising Tile Data");

	_mapWorldDim = getMapWorldDimensions(*_OSMdata, _numCols, _numRows);

	_OSMdata->initTiles();

	_tileRenderingResources.resize(_numCols);
	for (uint32_t i = 0; i < _numCols; ++i) { _tileRenderingResources[i].resize(_numRows); }

	pvr::utils::VertexAttributeInfo vertexInfo[] = { pvr::utils::VertexAttributeInfo(0, pvr::DataType::Float32, 3, 0, "myVertex"),
		pvr::utils::VertexAttributeInfo(1, pvr::DataType::Float32, 2, sizeof(float) * 3, "texCoord") };

	_deviceResources->vertexConfiguration.addVertexAttribute(0, vertexInfo[0]);
	_deviceResources->vertexConfiguration.addVertexAttribute(0, vertexInfo[1]);
	_deviceResources->vertexConfiguration.setInputBinding(0, sizeof(float) * 5);
	_deviceResources->vertexConfiguration.topology = pvr::PrimitiveTopology::TriangleList;

	const char* attributeNames[] = { vertexBindings[0].variableName.c_str(), vertexBindings[1].variableName.c_str() };
	const uint16_t attributeIndices[] = { static_cast<uint16_t>(AttributeIndices::VertexArray), static_cast<uint16_t>(AttributeIndices::TexCoordArray) };
	const uint32_t numAttributes = 2;

	{
		if (!(_deviceResources->roadProgram = pvr::utils::createShaderProgram(*this, "AA_VertShader.vsh", "AA_FragShader.fsh", attributeNames, attributeIndices, numAttributes)))
		{
			setExitMessage("Unable to create road program (%s, %s)", "AA_VertShader.vsh", "AA_FragShader.fsh");
			return pvr::Result::UnknownError;
		}

		_deviceResources->roadColorUniformLocation = gl::GetUniformLocation(_deviceResources->roadProgram, "myColor");
		_deviceResources->roadTransformUniformLocation = gl::GetUniformLocation(_deviceResources->roadProgram, "transform");
	}
	// For the roads
	gl::BlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ZERO, GL_ONE);

	{
		if (!(_deviceResources->fillProgram = pvr::utils::createShaderProgram(*this, "VertShader.vsh", "FragShader.fsh", attributeNames, attributeIndices, numAttributes)))
		{
			setExitMessage("Unable to create fill program (%s, %s)", "VertShader.vsh", "FragShader.fsh");
			return pvr::Result::UnknownError;
		}

		_deviceResources->fillColorUniformLocation = gl::GetUniformLocation(_deviceResources->fillProgram, "myColor");
		_deviceResources->fillTransformUniformLocation = gl::GetUniformLocation(_deviceResources->fillProgram, "transform");
	}

	Log(LogLevel::Information, "Remapping item coordinate data");
	remapItemCoordinates(*_OSMdata, _numCols, _numRows, _mapWorldDim);

	Log(LogLevel::Information, "Creating UI renderer items");
	createUIRendererItems();

	_screenWidth = static_cast<float>(getWidth());
	_screenHeight = static_cast<float>(getHeight());

	if (isScreenRotated()) { std::swap(_screenWidth, _screenHeight); }

	_projMtx = pvr::math::ortho(_deviceResources->context->getApiVersion(), 0.0f, static_cast<float>(_screenWidth), 0.0f, static_cast<float>(_screenHeight));

	_mapProjMtx = _tileRenderingResources[0][0].renderer->getScreenRotation() * _projMtx;

	Log(LogLevel::Information, "Creating per Tile buffers");
	createBuffers();

	Log(LogLevel::Information, "Converting Route");
	initRoute();

	_deviceResources->uiRenderer.init(getWidth(), getHeight(), isFullScreen(), getBackBufferColorspace() == pvr::ColorSpace::sRGB);

	_deviceResources->uiRenderer.getDefaultTitle()->setText("Navigation2D");

	_deviceResources->uiRenderer.getDefaultTitle()->commitUpdates();
	updateSubtitleText();

	gl::BindFramebuffer(GL_FRAMEBUFFER, _deviceResources->defaultFbo);
	gl::ClearColor(_clearColor.r, _clearColor.g, _clearColor.b, _clearColor.a);
	gl::ClearDepthf(1.0f);
	gl::ClearStencil(0);

	setDefaultStates();

	return pvr::Result::Success;
}

void OGLESNavigation2D::updateSubtitleText()
{
	if (_cameraMode == CameraMode::Auto) { _deviceResources->uiRenderer.getDefaultDescription()->setText(pvr::strings::createFormatted("Automatic Camera Mode")); }
	else
	{
		_deviceResources->uiRenderer.getDefaultDescription()->setText("Manual Camera Model use up/down/left/right to control the camera");
		_deviceResources->uiRenderer.getDefaultDescription()->setText("Manual Camera Mode\n"
																	  "up/down/left/right to move the camera\n"
																	  "w/s zoom in and out\n"
																	  "a/d to rotate");
	}
	_deviceResources->uiRenderer.getDefaultDescription()->commitUpdates();
}

void OGLESNavigation2D::handleInput()
{
	if (_cameraMode == CameraMode::Manual)
	{
		const float dt = float(getFrameTime());
		const float transDelta = dt;
		int right = isKeyPressed(pvr::Keys::Right) - isKeyPressed(pvr::Keys::Left);
		int up = isKeyPressed(pvr::Keys::Up) - isKeyPressed(pvr::Keys::Down);
		if (isKeyPressed(pvr::Keys::W) && _cameraMode == CameraMode::Manual) { _scale *= 1.05f; }
		if (isKeyPressed(pvr::Keys::S) && _cameraMode == CameraMode::Manual)
		{
			_scale *= .95f;
			_scale = glm::max(_scale, 0.1f);
		}

		if (isKeyPressed(pvr::Keys::A) && _cameraMode == CameraMode::Manual) { _rotation += dt * .1f; }
		if (isKeyPressed(pvr::Keys::D) && _cameraMode == CameraMode::Manual) { _rotation -= dt * .1f; }

		if (_rotation <= -180) { _rotation += 360; }
		if (_rotation > 180) { _rotation -= 360; }

		float fup = (-transDelta * up / _scale) * glm::cos(glm::pi<float>() * _rotation / 180) + (transDelta * right / _scale) * glm::sin(glm::pi<float>() * _rotation / 180);
		float fright = (-transDelta * up / _scale) * glm::sin(glm::pi<float>() * _rotation / 180) - (transDelta * right / _scale) * glm::cos(glm::pi<float>() * _rotation / 180);

		_translation.x += fright;
		_translation.y += fup;

		MapScreenAlignRotation = 0.f;
	}
	else
	{
		MapScreenAlignRotation = -90.f;
	}
}

/// <summary>Main rendering loop function of the program. The shell will call this function every frame.</summary>
/// <returns>Result::Success if no error occurred.</returns>
pvr::Result OGLESNavigation2D::renderFrame()
{
	debugThrowOnApiError("Frame begin");
	handleInput();
	updateAnimation();
	float rotation = glm::radians(_rotation + MapScreenAlignRotation);

	_mapMVPMtx = _mapProjMtx *
		glm::translate(glm::vec3(_translation.x + _screenWidth * .5 /*centre the map*/, _translation.y + _screenHeight * .5 /*centre the map*/, 0.0f)) // final transform
		* glm::translate(glm::vec3(-_translation.x, -_translation.y, 0.0f)) // undo the translation
		* glm::rotate(rotation, glm::vec3(0.0f, 0.0f, 1.0f)) // rotate
		* glm::scale(glm::vec3(_scale, _scale, 1.0f)) // scale the focus area
		* glm::translate(glm::vec3(_translation.x, _translation.y, 0.0f)); // translate the camera to the centre of the current focus area

	calculateClipPlanes();

	render();

	// UIRENDERER
	{
		// render UI
		_deviceResources->uiRenderer.beginRendering();
		_deviceResources->uiRenderer.getSdkLogo()->render();
		_deviceResources->uiRenderer.getDefaultTitle()->render();
		_deviceResources->uiRenderer.getDefaultDescription()->render();
		_deviceResources->uiRenderer.endRendering();
	}

	debugThrowOnApiError("Frame end");

	if (this->shouldTakeScreenshot()) { pvr::utils::takeScreenshot(this->getScreenshotFileName(), this->getWidth(), this->getHeight()); }

	_deviceResources->context->swapBuffers();

	return pvr::Result::Success;
}

/// <summary>Code in releaseView() will be called by Shell when the application quits.</summary>
/// <returns>Result::Success if no error occurred.</returns>
pvr::Result OGLESNavigation2D::releaseView()
{
	// Clean up tile rendering resource data.
	_tileRenderingResources.clear();

	// Reset context and associated resources.
	_deviceResources.reset();

	return pvr::Result::Success;
}

/// <summary>Code in quitApplication() will be called by pvr::Shell once per run, just before exiting the program.</summary>
/// <returns>Result::Success if no error occurred.</returns>
pvr::Result OGLESNavigation2D::quitApplication()
{
	_OSMdata.reset();

	return pvr::Result::Success;
}

void OGLESNavigation2D::setDefaultStates()
{
	gl::BindFramebuffer(GL_FRAMEBUFFER, _deviceResources->context->getOnScreenFbo());
	gl::UseProgram(0);

	// disable most states
	gl::Disable(GL_BLEND);
	gl::Disable(GL_DEPTH_TEST);
	gl::Disable(GL_STENCIL_TEST);
	gl::DepthMask(false);
	gl::StencilMask(0);

	// Disable back face culling
	gl::Disable(GL_CULL_FACE);
	gl::CullFace(GL_BACK);

	gl::FrontFace(GL_CCW);

	gl::Viewport(0, 0, getWidth(), getHeight());
}

void OGLESNavigation2D::bindAndClearFramebuffer()
{
	gl::BindFramebuffer(GL_FRAMEBUFFER, _deviceResources->defaultFbo);
	gl::Clear(GL_COLOR_BUFFER_BIT);
}

void OGLESNavigation2D::initializeRenderers(TileRenderingResources* begin, TileRenderingResources* end, const uint32_t col, const uint32_t row)
{
	begin->renderer = std::make_shared<pvr::ui::UIRenderer>();
	auto& renderer = *begin->renderer;
	renderer.init(getWidth(), getHeight(), isFullScreen(), getBackBufferColorspace() == pvr::ColorSpace::sRGB);

	if (_deviceResources->context->getApiVersion() != pvr::Api::OpenGLES2)
	{ begin->font = begin->renderer->createFont(_deviceResources->fontTexture, _deviceResources->fontHeader, _deviceResources->fontSampler); }
	else
	{
		begin->font = begin->renderer->createFont(_deviceResources->fontTexture, _deviceResources->fontHeader);
	}

	begin->col = col;
	begin->row = row;

	auto& tile = _OSMdata->getTiles()[col][row];

	for (uint32_t lod = 0; lod < LOD::Count; ++lod)
	{
		for (uint32_t iconIndex = 0; iconIndex < tile.icons[lod].size(); iconIndex++)
		{
			for (uint32_t i = 0; i < BuildingType::None; ++i)
			{
				if (tile.icons[lod][iconIndex].buildingType == BuildingType::Shop + i)
				{
					begin->spriteImages[i] = begin->renderer->createImageFromAtlas(
						_deviceResources->texAtlas, _deviceResources->atlasOffsets[i], _deviceResources->texAtlasHeader.getWidth(), _deviceResources->texAtlasHeader.getHeight());
					begin->spriteImages[i]->commitUpdates();

					_stateTracker = begin->renderer->getStateTracker();
				}
			}
		}
	}

	for (auto it = begin + 1; it < end; ++it)
	{
		it->font = begin->font;
		it->renderer = begin->renderer;
		for (uint32_t lod = 0; lod < LOD::Count; ++lod)
		{
			for (uint32_t iconIndex = 0; iconIndex < tile.icons[lod].size(); iconIndex++)
			{
				for (uint32_t i = 0; i < BuildingType::None; ++i)
				{
					if (tile.icons[lod][iconIndex].buildingType == BuildingType::Shop + i) { it->spriteImages[i] = begin->spriteImages[i]; }
				}
			}
		}
		it->col = begin->col;
		it->row = begin->row;
	}
}

void OGLESNavigation2D::renderTile(const Tile& tile, TileRenderingResources& renderingResources)
{
	(void)tile;
	uint32_t offset = 0;

	// Bind the vertex and index buffers for the tile
	if (_stateTracker.vao != static_cast<GLint>(renderingResources.vao))
	{
		if (_deviceResources->context->getApiVersion() != pvr::Api::OpenGLES2) { gl::BindVertexArray(renderingResources.vao); }
		else
		{
			gl::ext::BindVertexArrayOES(renderingResources.vao);
		}
		_stateTracker.vao = renderingResources.vao;
		_stateTracker.vaoChanged = true;
	}

	if (_stateTracker.activeTextureUnit != 0 || _stateTracker.activeTextureUnitChanged)
	{
		_stateTracker.activeTextureUnit = GL_TEXTURE0;
		gl::ActiveTexture(GL_TEXTURE0);
		_stateTracker.activeTextureUnitChanged = true;
	}
	else
	{
		_stateTracker.activeTextureUnitChanged = false;
	}

	if (_stateTracker.boundTexture != static_cast<GLint>(_deviceResources->texAtlas) || _stateTracker.boundTextureChanged)
	{
		_stateTracker.boundTexture = _deviceResources->texAtlas;
		gl::BindTexture(GL_TEXTURE_2D, _deviceResources->texAtlas);

		_stateTracker.boundTextureChanged = true;
	}
	else
	{
		_stateTracker.boundTextureChanged = false;
	}

	if (renderingResources.properties.parkingNum > 0 || renderingResources.properties.buildNum > 0 || renderingResources.properties.innerNum > 0 ||
		renderingResources.properties.areaNum > 0)
	{
		if (_stateTracker.activeProgram != static_cast<GLint>(_deviceResources->fillProgram))
		{
			gl::UseProgram(_deviceResources->fillProgram);
			_stateTracker.activeProgram = _deviceResources->fillProgram;
			_stateTracker.activeProgramChanged = true;
		}

		if (_stateTracker.blendEnabled)
		{
			gl::Disable(GL_BLEND);
			_stateTracker.blendEnabled = false;
			_stateTracker.blendEnabledChanged = true;
		}

		// Draw the car parking
		if (renderingResources.properties.parkingNum > 0)
		{
			gl::UniformMatrix4fv(_deviceResources->fillTransformUniformLocation, 1, GL_FALSE, glm::value_ptr(_mapMVPMtx));
			gl::Uniform4fv(_deviceResources->fillColorUniformLocation, 1, glm::value_ptr(_parkingColor));

			gl::DrawElements(GL_TRIANGLES, renderingResources.properties.parkingNum, GL_UNSIGNED_INT, 0);
			offset += renderingResources.properties.parkingNum;
		}

		// Draw the buildings
		if (renderingResources.properties.buildNum > 0)
		{
			gl::UniformMatrix4fv(_deviceResources->fillTransformUniformLocation, 1, GL_FALSE, glm::value_ptr(_mapMVPMtx));
			gl::Uniform4fv(_deviceResources->fillColorUniformLocation, 1, glm::value_ptr(_buildingColor));

			gl::DrawElements(GL_TRIANGLES, renderingResources.properties.buildNum, GL_UNSIGNED_INT, reinterpret_cast<const void*>(static_cast<uintptr_t>(offset * 4)));
			offset += renderingResources.properties.buildNum;
		}

		// Draw the insides of car parking and buildings for polygons with holes
		if (renderingResources.properties.innerNum > 0)
		{
			gl::UniformMatrix4fv(_deviceResources->fillTransformUniformLocation, 1, GL_FALSE, glm::value_ptr(_mapMVPMtx));
			gl::Uniform4fv(_deviceResources->fillColorUniformLocation, 1, glm::value_ptr(_clearColor));

			gl::DrawElements(GL_TRIANGLES, renderingResources.properties.innerNum, GL_UNSIGNED_INT, reinterpret_cast<const void*>(static_cast<uintptr_t>(offset * 4)));
			offset += renderingResources.properties.innerNum;
		}

		// Draw the road areas
		if (renderingResources.properties.areaNum > 0)
		{
			gl::UniformMatrix4fv(_deviceResources->fillTransformUniformLocation, 1, GL_FALSE, glm::value_ptr(_mapMVPMtx));
			gl::Uniform4fv(_deviceResources->fillColorUniformLocation, 1, glm::value_ptr(_roadAreaColor));

			gl::DrawElements(GL_TRIANGLES, renderingResources.properties.areaNum, GL_UNSIGNED_INT, reinterpret_cast<const void*>(static_cast<uintptr_t>(offset * 4)));
			offset += renderingResources.properties.areaNum;
		}
	}

	if (renderingResources.properties.serviceRoadNum > 0 || renderingResources.properties.otherRoadNum > 0 || renderingResources.properties.secondaryRoadNum > 0 ||
		renderingResources.properties.primaryRoadNum > 0 || renderingResources.properties.trunkRoadNum > 0 || renderingResources.properties.motorwayNum > 0)
	{
		if (_stateTracker.activeProgram != static_cast<GLint>(_deviceResources->roadProgram))
		{
			gl::UseProgram(_deviceResources->roadProgram);
			_stateTracker.activeProgram = _deviceResources->roadProgram;
			_stateTracker.activeProgramChanged = true;
		}

		if (!_stateTracker.blendEnabled)
		{
			gl::Enable(GL_BLEND);
			_stateTracker.blendEnabled = true;
			_stateTracker.blendEnabledChanged = true;
		}

		gl::UniformMatrix4fv(_deviceResources->roadTransformUniformLocation, 1, GL_FALSE, glm::value_ptr(_mapMVPMtx));

		/**** Draw the roads ****/
		// REVERSE order of importance.
		// Service Roads
		if (renderingResources.properties.serviceRoadNum > 0)
		{
			gl::Uniform4fv(_deviceResources->roadColorUniformLocation, 1, glm::value_ptr(_serviceRoadColor));
			gl::DrawElements(GL_TRIANGLES, renderingResources.properties.serviceRoadNum, GL_UNSIGNED_INT, reinterpret_cast<const void*>(static_cast<uintptr_t>(offset * 4)));
			offset += renderingResources.properties.serviceRoadNum;
		}

		// Other (any other roads)
		if (renderingResources.properties.otherRoadNum > 0)
		{
			gl::Uniform4fv(_deviceResources->roadColorUniformLocation, 1, glm::value_ptr(_otherRoadColor));
			gl::DrawElements(GL_TRIANGLES, renderingResources.properties.otherRoadNum, GL_UNSIGNED_INT, reinterpret_cast<const void*>(static_cast<uintptr_t>(offset * 4)));
			offset += renderingResources.properties.otherRoadNum;
		}

		// Secondary Roads
		if (renderingResources.properties.secondaryRoadNum > 0)
		{
			gl::Uniform4fv(_deviceResources->roadColorUniformLocation, 1, glm::value_ptr(_secondaryRoadColor));
			gl::DrawElements(GL_TRIANGLES, renderingResources.properties.secondaryRoadNum, GL_UNSIGNED_INT, reinterpret_cast<const void*>(static_cast<uintptr_t>(offset * 4)));
			offset += renderingResources.properties.secondaryRoadNum;
		}

		// Primary Roads
		if (renderingResources.properties.primaryRoadNum > 0)
		{
			gl::Uniform4fv(_deviceResources->roadColorUniformLocation, 1, glm::value_ptr(_primaryRoadColor));
			gl::DrawElements(GL_TRIANGLES, renderingResources.properties.primaryRoadNum, GL_UNSIGNED_INT, reinterpret_cast<const void*>(static_cast<uintptr_t>(offset * 4)));

			offset += renderingResources.properties.primaryRoadNum;
		}

		// Trunk Roads
		if (renderingResources.properties.trunkRoadNum > 0)
		{
			gl::Uniform4fv(_deviceResources->roadColorUniformLocation, 1, glm::value_ptr(_trunkRoadColor));
			gl::DrawElements(GL_TRIANGLES, renderingResources.properties.trunkRoadNum, GL_UNSIGNED_INT, reinterpret_cast<const void*>(static_cast<uintptr_t>(offset * 4)));
			offset += renderingResources.properties.trunkRoadNum;
		}

		if (renderingResources.properties.motorwayNum > 0)
		{
			gl::Uniform4fv(_deviceResources->roadColorUniformLocation, 1, glm::value_ptr(_motorwayColor));
			gl::DrawElements(GL_TRIANGLES, renderingResources.properties.motorwayNum, GL_UNSIGNED_INT, reinterpret_cast<const void*>(static_cast<uintptr_t>(offset * 4)));
			offset += renderingResources.properties.motorwayNum;
		}
	}
}

/// <summary>Load a texture from file using PVR Asset Store, create a trilinear sampler, create a description set.</summary>
/// <returns>Return true if no error occurred, false if the sampler descriptor set is not valid.</returns>
void OGLESNavigation2D::loadTexture()
{
	// load the diffuse texture
	_deviceResources->fontTexture = pvr::utils::textureUpload(*this, FontFile, _deviceResources->fontHeader, _deviceResources->context->getApiVersion() == pvr::Api::OpenGLES2);

	if (_deviceResources->context->getApiVersion() != pvr::Api::OpenGLES2)
	{
		// create font sampler
		gl::GenSamplers(1, &_deviceResources->fontSampler);

		gl::SamplerParameteri(_deviceResources->fontSampler, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		gl::SamplerParameteri(_deviceResources->fontSampler, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		gl::SamplerParameteri(_deviceResources->fontSampler, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		gl::SamplerParameteri(_deviceResources->fontSampler, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

		debugThrowOnApiError("Unable to create the font sampler");
	}
	else
	{
		gl::BindTexture(GL_TEXTURE_2D, _deviceResources->fontTexture);
		_stateTracker.boundTexture = _deviceResources->fontTexture;

		gl::TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		gl::TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		gl::TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		gl::TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

		debugThrowOnApiError("Unable to setup the texture parameters for the font texture");
	}

	// Load & generate texture atlas for icons.
	pvr::utils::generateTextureAtlas(*this, SpriteFileNames, _deviceResources->atlasOffsets, BuildingType::None, &_deviceResources->texAtlas, &_deviceResources->texAtlasHeader,
		_deviceResources->context->getApiVersion() == pvr::Api::OpenGLES2);

	if (_deviceResources->context->getApiVersion() == pvr::Api::OpenGLES2)
	{
		gl::BindTexture(GL_TEXTURE_2D, _deviceResources->texAtlas);
		_stateTracker.boundTexture = _deviceResources->texAtlas;

		gl::TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		gl::TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		gl::TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		gl::TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	}
	pvr::utils::throwOnGlError("[OGLESNavigation2D::loadTexture] Failed to create textures");
}

/// <summary>Converts pre-computed route into the appropriate co-ordinate space and calculates the routes total true distance
/// and partial distances between each node which is used later to animate the route.</summary>
void OGLESNavigation2D::initRoute()
{
	_OSMdata->convertRoute(_mapWorldDim, _numCols, _numRows, _totalRouteDistance);
	if (_cameraMode == CameraMode::Auto)
	{
		// Initial weighting for first iteration of the animation
		_keyFrameTime = 0.0f;
		_rotation = static_cast<float>(_OSMdata->getRouteData()[0].rotation);
	}
}

/// <summary>Creates vertex and index buffers and records the secondary command buffers for each tile.</summary>
void OGLESNavigation2D::createBuffers()
{
	// get the map dimension
	// calculate the aspect ratio
	// rescale the map with the aspect ratio

	for (uint32_t col = 0; col < _OSMdata->getTiles().size(); ++col)
	{
		auto& tileCol = _OSMdata->getTiles()[col];
		for (uint32_t row = 0; row < tileCol.size(); ++row)
		{
			Tile& tile = tileCol[row];

			// Create vertices for tile
			for (auto nodeIterator = tile.nodes.begin(); nodeIterator != tile.nodes.end(); ++nodeIterator)
			{
				nodeIterator->second.index = static_cast<uint32_t>(tile.vertices.size());

				Tile::VertexData vertData(glm::vec3(remap(nodeIterator->second.coords, _OSMdata->getTiles()[0][0].min, _OSMdata->getTiles()[_numCols - 1][_numRows - 1].max,
														-_mapWorldDim * .5, _mapWorldDim * .5),
											  0.0f),
					nodeIterator->second.texCoords);

				tile.vertices.push_back(vertData);
			}

			auto& renderingResources = _tileRenderingResources[col][row];

			// Add car parking to indices
			renderingResources.properties.parkingNum = generateIndices(tile, tile.parkingWays);
			// Add buildings to indices
			renderingResources.properties.buildNum = generateIndices(tile, tile.buildWays);
			// Add inner ways to indices
			renderingResources.properties.innerNum = generateIndices(tile, tile.innerWays);
			// Add road area ways to indices
			renderingResources.properties.areaNum = generateIndices(tile, tile.areaWays);
			// Add roads to indices
			renderingResources.properties.serviceRoadNum = generateIndices(tile, tile.roadWays, RoadTypes::Service);
			renderingResources.properties.otherRoadNum = generateIndices(tile, tile.roadWays, RoadTypes::Other);
			renderingResources.properties.secondaryRoadNum = generateIndices(tile, tile.roadWays, RoadTypes::Secondary);
			renderingResources.properties.primaryRoadNum = generateIndices(tile, tile.roadWays, RoadTypes::Primary);
			renderingResources.properties.trunkRoadNum = generateIndices(tile, tile.roadWays, RoadTypes::Trunk);
			renderingResources.properties.motorwayNum = generateIndices(tile, tile.roadWays, RoadTypes::Motorway);

			// Create vertex and index buffers
			// Interleaved vertex buffer (vertex position + texCoord)
			if (tile.vertices.size())
			{
				auto& tileRes = _tileRenderingResources[col][row];

				{
					// vertices buffer
					gl::GenBuffers(1, &tileRes.vbo);
					gl::BindBuffer(GL_ARRAY_BUFFER, tileRes.vbo);

					std::vector<float> verticesTemp(5 * sizeof(float) * tile.vertices.size());
					for (uint32_t k = 0; k < tile.vertices.size(); ++k)
					{
						memcpy(&verticesTemp[k * 5], &tile.vertices[k].pos, sizeof(float) * 3);
						memcpy(&verticesTemp[k * 5 + 3], &tile.vertices[k].texCoord, sizeof(float) * 2);
					}
					uint32_t vboSize = static_cast<uint32_t>(verticesTemp.size() * sizeof(verticesTemp[0]));
					gl::BufferData(GL_ARRAY_BUFFER, vboSize, verticesTemp.data(), GL_STATIC_DRAW);
				}

				{
					// indices buffer
					gl::GenBuffers(1, &tileRes.ibo);
					gl::BindBuffer(GL_ARRAY_BUFFER, tileRes.ibo);
					uint32_t iboSize = static_cast<uint32_t>(tile.indices.size() * sizeof(tile.indices[0]));
					gl::BufferData(GL_ARRAY_BUFFER, iboSize, tile.indices.data(), GL_STATIC_DRAW);
				}

				if (_deviceResources->context->getApiVersion() != pvr::Api::OpenGLES2)
				{
					gl::GenVertexArrays(1, &tileRes.vao);
					gl::BindVertexArray(tileRes.vao);
				}
				else
				{
					gl::ext::GenVertexArraysOES(1, &tileRes.vao);
					gl::ext::BindVertexArrayOES(tileRes.vao);
				}

				GLsizei stride = sizeof(float) * 5;
				gl::BindBuffer(GL_ARRAY_BUFFER, tileRes.vbo);
				gl::BindBuffer(GL_ELEMENT_ARRAY_BUFFER, tileRes.ibo);

				// enable vertex attribute pointers
				for (auto it = _deviceResources->vertexConfiguration.attributes.begin(), end = _deviceResources->vertexConfiguration.attributes.end(); it != end; ++it)
				{
					gl::EnableVertexAttribArray(it->index);
					GLenum type = pvr::utils::convertToGles(it->format);
					bool isNormalised = pvr::dataTypeIsNormalised(it->format);
					auto offset = it->offsetInBytes;

					gl::VertexAttribPointer(it->index, it->width, type, isNormalised, stride, reinterpret_cast<const void*>(static_cast<uintptr_t>(offset)));

					_stateTracker.vertexAttribArray[it->index] = GL_TRUE;
					_stateTracker.vertexAttribArrayChanged[it->index] = true;

					_stateTracker.vertexAttribBindings[it->index] = it->index;
					_stateTracker.vertexAttribSizes[it->index] = it->width;
					_stateTracker.vertexAttribTypes[it->index] = type;
					_stateTracker.vertexAttribNormalized[it->index] = isNormalised;
					_stateTracker.vertexAttribStride[it->index] = 0;
					_stateTracker.vertexAttribOffset[it->index] = reinterpret_cast<GLvoid*>(static_cast<uintptr_t>(offset));

					_stateTracker.vertexAttribPointerChanged[it->index] = true;
				}

				if (_deviceResources->context->getApiVersion() != pvr::Api::OpenGLES2) { gl::BindVertexArray(0); }
				else
				{
					gl::ext::BindVertexArrayOES(0);
				}
			}
		}
	}
}

/// <summary>Update animation using pre-computed path for the camera to follow.</summary>
void OGLESNavigation2D::updateAnimation()
{
	static const float scaleAnimTime = 1000.0f;
	static const float scaleGracePeriod = 8000.0f;
	static float r1 = 0.0f;
	static float r2 = 0.0f;
	static float restartTimer = 0.0f;
	static bool destinationReached = false;
	float dt = float(getFrameTime());
	static const float restartTimeWait = 3000.f;
	if (destinationReached && restartTimer >= restartTimeWait)
	{
		destinationReached = false;
		restartTimer = 0.0f;
		resetCameraVariables();
	}
	else if (destinationReached)
	{
		restartTimer += dt;
		return;
	}

	_keyFrameTime = calculateRouteKeyFrameTime(_OSMdata->getRouteData()[_routeIndex].point, _OSMdata->getRouteData()[_routeIndex + 1].point, _totalRouteDistance, CameraMoveSpeed);
	if (_cameraMode == CameraMode::Auto)
	{
		if (!_turning)
		{
			// Interpolate between two positions.
			_translation = glm::mix(_OSMdata->getRouteData()[_routeIndex].point, _OSMdata->getRouteData()[_routeIndex + 1].point, _animTime / _keyFrameTime);
			_animTime += dt / _scale;
		}
		const bool isDestinationReached = (_routeIndex + 1) == _OSMdata->getRouteData().size() - 1;
		if (_animTime >= _keyFrameTime)
		{
			r1 = static_cast<float>(_OSMdata->getRouteData()[_routeIndex].rotation);
			r2 = static_cast<float>(_OSMdata->getRouteData()[_routeIndex + 1].rotation);
			const float angleDiff = fabs(r1 - r2);
			// Find the shortest rotation
			if (angleDiff > 180.0f)
			{
				if (r1 > r2) { r2 += 360.0f; }
				else
				{
					r2 -= 360.0f;
				}
			}

			float diff = (r2 > r1) ? r2 - r1 : r1 - r2;
			// Calculate the time to animate the _rotation based on angle.

			_rotateTime = 15.f * diff; // 15ms (for 1 degree) * diff
			_rotateAnimTime += dt;
			_rotateAnimTime = glm::clamp(_rotateAnimTime, 0.0f, _rotateTime);
			if (diff > 2.f && !isDestinationReached)
			{
				_rotation = glm::mix(r1, r2, _rotateAnimTime / _rotateTime);
				_turning = true;
			}
			if (_rotateAnimTime >= _rotateTime) { _turning = false; }
		}

		if (_animTime >= _keyFrameTime && !_turning)
		{
			_rotateAnimTime = 0.0f;
			_animTime = 0.0f;

			// Iterate through the route
			if (++_routeIndex == _OSMdata->getRouteData().size() - 1)
			{
				destinationReached = true;
				return;
			}
		}
	}
	_timePassed += dt;

	// Check for _scale changes
	if (_cameraMode == CameraMode::Manual)
	{
		_currentScaleLevel = LOD::L4;
		for (int32_t i = LOD::L4; i >= 0; --i)
		{
			if (_scale > scales[_currentScaleLevel]) { _currentScaleLevel = (uint16_t)i; }
			else
			{
				break;
			}
		}
	}
	else
	{
		if (_timePassed >= scaleGracePeriod)
		{
			_previousScaleLevel = _currentScaleLevel;
			if (_increaseScale)
			{
				if (++_currentScaleLevel == LOD::L4) { _increaseScale = false; }
			}
			else
			{
				if (--_currentScaleLevel == LOD::L1) { _increaseScale = true; }
			}
			_timePassed = 0.0f;
			_scaleChange = _previousScaleLevel != _currentScaleLevel;
		}

		if (_scaleChange)
		{
			if (_timePassed >= scaleAnimTime) { _scaleChange = false; }
			// interpolate
			_scale = glm::mix(MapScales[_previousScaleLevel], MapScales[_currentScaleLevel], _timePassed / scaleAnimTime);
		}
	}
}

bool skipAmenityLabel(AmenityLabelData& labelData, Label& label, glm::dvec3& extent)
{
	// Check if labels overlap.
	// Almost half extent (dividing by 1.95 to leave some padding between text) of the scaled text.
	float halfExtent_x = label.text->getScaledDimension().x / 1.95f;

	// Check if this and the previous text (in the same LOD level) overlap, if they do skip this text.
	float distance = static_cast<float>(glm::distance(labelData.coords, glm::dvec2(extent)));
	if (distance < (extent.z + halfExtent_x) && glm::abs(extent.z - halfExtent_x) < distance)
	{
		label.text.reset();
		return true;
	}

	// Update with fresh data - position (stored in x, y components) and half extent (stored in z component).
	extent = glm::vec3(labelData.coords, halfExtent_x);

	return false;
}

bool skipLabel(LabelData& labelData, Label& label, glm::dvec3& extent)
{
	// Check if labels overlap.
	// Almost half extent (dividing by 1.95 to leave some padding between text) of the scaled text.
	float halfExtent_x = label.text->getScaledDimension().x / 1.95f;

	// Check if this text crosses the tile boundary or the text overruns the end of the road segment.
	if (labelData.distToBoundary < halfExtent_x)
	{
		label.text.reset();
		return true;
	}

	// Check if the text overruns the end of the road segment.
	if (labelData.distToEndOfSegment < halfExtent_x)
	{
		label.text.reset();
		return true;
	}

	// Check if this and the previous text (in the same LOD level) overlap, if they do skip this text.
	float distance = static_cast<float>(glm::distance(labelData.coords, glm::dvec2(extent)));
	if (distance < (extent.z + halfExtent_x) && glm::abs(extent.z - halfExtent_x) < distance)
	{
		label.text.reset();
		return true;
	}

	// Update with fresh data - position (stored in x, y components) and half extent (stored in z component).
	extent = glm::vec3(labelData.coords, halfExtent_x);

	return false;
}

/// <summary>Record the primary command buffer.</summary>
void OGLESNavigation2D::createUIRendererItems()
{
	for (uint32_t col = 0; col < _numCols; ++col)
	{
		for (uint32_t row = 0; row < _numRows; row++)
		{ initializeRenderers(&_tileRenderingResources[col][row], &_tileRenderingResources[col][std::min(row + 1, _numRows - 1)], col, row); }
	}

	for (uint32_t col = 0; col < _numCols; ++col)
	{
		for (uint32_t row = 0; row < _numRows; ++row)
		{
			auto& tile = _OSMdata->getTiles()[col][row];
			auto& tileRes = _tileRenderingResources[col][row];
			for (uint32_t lod = 0; lod < LOD::Count; ++lod)
			{
				glm::dvec3 extent(0, 0, 0);
				if (!tile.icons[lod].empty() || !tile.labels[lod].empty() || !tile.amenityLabels[lod].empty())
				{
					tileRes.tileGroup[lod] = tileRes.renderer->createPixelGroup();
					auto& group = tileRes.tileGroup[lod];
					auto& camGroup = tileRes.cameraRotateGroup[lod] = tileRes.renderer->createPixelGroup();
					group->setAnchor(pvr::ui::Anchor::Center, 0.f, 0.f);

					for (auto&& icon : tile.icons[lod])
					{
						tileRes.amenityIcons[lod].push_back(AmenityIconGroup());
						auto& tileResIcon = tileRes.amenityIcons[lod].back();

						tileResIcon.iconData = icon;
						tileResIcon.group = tileRes.renderer->createPixelGroup();

						tileResIcon.group->add(tileRes.spriteImages[icon.buildingType]);

						// create the image - or at least take a copy that we'll work with from now on
						tileResIcon.icon.image = tileRes.spriteImages[icon.buildingType];
						tileResIcon.icon.image->setAnchor(pvr::ui::Anchor::Center, 0.f, 0.f);

						// flip the icon
						tileResIcon.icon.image->setRotation(glm::pi<float>());
						tileResIcon.icon.image->commitUpdates();

						// add the amenity icon to the group
						tileResIcon.group->add(tileResIcon.icon.image);
						tileResIcon.group->setAnchor(pvr::ui::Anchor::Center, 0.f, 0.f);
						tileResIcon.group->commitUpdates();

						group->add(tileResIcon.group);
					}

					for (auto&& amenityLabel : tile.amenityLabels[lod])
					{
						tileRes.amenityLabels[lod].push_back(AmenityLabelGroup());
						auto& tileResAmenityLabel = tileRes.amenityLabels[lod].back();

						tileResAmenityLabel.iconData = amenityLabel.iconData;

						tileResAmenityLabel.group = tileRes.renderer->createPixelGroup();

						tileResAmenityLabel.label.text = tileRes.renderer->createText(amenityLabel.name, tileRes.font);
						debug_assertion(tileResAmenityLabel.label.text != nullptr, "Amenity label must be a valid UIRenderer Text Element");
						tileResAmenityLabel.label.text->setColor(0.f, 0.f, 0.f, 1.f);
						tileResAmenityLabel.label.text->setAlphaRenderingMode(true);

						float txtScale = 1.0f / (scales[lod + 1] * 12.0f);

						tileResAmenityLabel.label.text->setScale(txtScale, txtScale);
						tileResAmenityLabel.label.text->setPixelOffset(-glm::abs(tileResAmenityLabel.iconData.coords - amenityLabel.coords));
						tileResAmenityLabel.label.text->commitUpdates();

						if (skipAmenityLabel(amenityLabel, tileResAmenityLabel.label, extent)) { continue; }

						// add the label to its corresponding amenity group
						tileResAmenityLabel.group->add(tileResAmenityLabel.label.text);
						tileResAmenityLabel.group->commitUpdates();

						group->add(tileResAmenityLabel.group);
					}

					for (auto&& label : tile.labels[lod])
					{
						tileRes.labels[lod].push_back(Label());
						auto& tileResLabel = tileRes.labels[lod].back();

						tileResLabel.text = tileRes.renderer->createText(label.name, tileRes.font);
						debug_assertion(tileResLabel.text != nullptr, "Label must be a valid UIRenderer Text Element");

						tileResLabel.text->setColor(0.f, 0.f, 0.f, 1.f);
						tileResLabel.text->setAlphaRenderingMode(true);

						float txtScale = label.scale * 2.0f;

						tileResLabel.text->setScale(txtScale, txtScale);
						tileResLabel.text->setPixelOffset(label.coords);
						tileResLabel.text->commitUpdates();

						if (skipLabel(label, tileResLabel, extent)) { continue; }

						group->add(tileResLabel.text);
					}

					group->commitUpdates();
					camGroup->add(group);
					camGroup->commitUpdates();
				}
			}
		}
	}
}

/// <summary>Find the tiles that need to be rendered.</summary>
void OGLESNavigation2D::render()
{
	_deviceResources->renderqueue.clear();

	for (uint32_t i = 0; i < _numCols; ++i)
	{
		for (uint32_t j = 0; j < _numRows; ++j)
		{
			auto& tile = _tileRenderingResources[i][j];
			if (inFrustum(_OSMdata->getTiles()[i][j].screenMin, _OSMdata->getTiles()[i][j].screenMax))
			{
				_deviceResources->renderqueue.push_back(&tile);

				// Update text elements
				updateLabels(i, j);

				// Update icons (points of interest)
				updateAmenities(i, j);

				// Update icons (points of interest)
				updateGroups(i, j);
			}
		}
	}

	bindAndClearFramebuffer();

	for (auto&& tile : _deviceResources->renderqueue)
	{
		if (tile->renderer) { renderTile(_OSMdata->getTiles()[tile->col][tile->row], *tile); }
		for (int lod = _currentScaleLevel; lod < LOD::Count; ++lod)
		{
			if (tile->cameraRotateGroup[lod])
			{
				tile->renderer->beginRendering(_stateTracker);
				tile->cameraRotateGroup[lod]->render();
				tile->renderer->endRendering(_stateTracker);
			}
		}
	}
}

/// <summary>Capture frustum planes from the current View Projection matrix.</summary>
void OGLESNavigation2D::calculateClipPlanes() { pvr::math::getFrustumPlanes(_deviceResources->context->getApiVersion(), _mapMVPMtx, _viewFrustum); }

/// <summary>Tests whether a 2D bounding box is intersected or enclosed by a view frustum.
/// Only the top, bottom, left and right planes of the view frustum are taken into consideration
/// to optimize the intersection test.</summary>
/// <param name="min">The minimum co-ordinates of the bounding box.</param>
/// <param name="max">The maximum co-ordinates of the bounding box.</param>
/// <returns>Return boolean True if inside the view frustum, false if outside.</returns>
bool OGLESNavigation2D::inFrustum(glm::vec2 min, glm::vec2 max)
{
	// Test the axis-aligned bounding box against each frustum plane,
	// cull if all points are outside of one the view frustum planes.
	pvr::math::AxisAlignedBox aabb;
	aabb.setMinMax(glm::vec3(min.x, min.y, 0.0f), glm::vec3(max.x, max.y, 1.0f));
	return pvr::math::aabbInFrustum(aabb, _viewFrustum);
}

void OGLESNavigation2D::updateGroups(uint32_t col, uint32_t row)
{
	const glm::vec2 pixelOffset = _translation * _scale;
	TileRenderingResources& tileRes = _tileRenderingResources[col][row];

	for (uint32_t lod = _currentScaleLevel; lod < LOD::Count; ++lod)
	{
		if (tileRes.tileGroup[lod])
		{
			tileRes.tileGroup[lod]->setAnchor(pvr::ui::Anchor::Center, 0, 0);
			tileRes.tileGroup[lod]->setPixelOffset(pixelOffset.x, pixelOffset.y);
			tileRes.tileGroup[lod]->setScale(_scale, _scale);
			tileRes.tileGroup[lod]->commitUpdates();
		}
		if (tileRes.cameraRotateGroup[lod])
		{
			tileRes.cameraRotateGroup[lod]->setRotation(glm::radians(_rotation + MapScreenAlignRotation));
			tileRes.cameraRotateGroup[lod]->setAnchor(pvr::ui::Anchor::Center, 0, 0);
			tileRes.cameraRotateGroup[lod]->commitUpdates();
		}
	}
}

/// <summary>Update the renderable text (dependant on LOD level) using the pre-processed
/// data (position, scale, _rotation, std::string) and UIRenderer.</summary>
/// <param name="col">Column index for tile.</param>
/// <param name="row">Row index for tile.</param>
void OGLESNavigation2D::updateLabels(uint32_t col, uint32_t row)
{
	Tile& tile = _OSMdata->getTiles()[col][row];
	TileRenderingResources& tileRes = _tileRenderingResources[col][row];

	for (uint32_t lod = _currentScaleLevel; lod < LOD::Count; ++lod)
	{
		for (uint32_t labelIdx = 0; labelIdx < tile.labels[lod].size(); ++labelIdx)
		{
			auto& tileResLabelLod = tileRes.labels[lod];

			if (tileResLabelLod.empty()) { continue; }

			auto& tileLabel = tile.labels[lod][labelIdx];
			auto& tileResLabel = tileRes.labels[lod][labelIdx];
			if (tileResLabel.text == nullptr) { continue; }

			glm::dvec2 offset(0, 0);

			float txtScale = tileLabel.scale * 2.0f;

			// Make sure road text is displayed upright (between 0 deg and 180 deg), otherwise flip it.
			float total_angle = tileLabel.rotation + _rotation + MapScreenAlignRotation; // Use that to calculate if the text is upright
			float angle = tileLabel.rotation;

			// check whether the label needs flipping
			// we add a small buffer onto the total angles to reduce the chance of parts of roads being flipped whilst other parts are not
			if ((total_angle - 2.f) <= -90.f) { angle += 180.f; }
			else if ((total_angle + 2.f) >= 90.f)
			{
				angle -= 180.f;
			}
			float aabbHeight = tileResLabel.text->getBoundingBox().getSize().y;

			offset.y += tileLabel.scale * aabbHeight * 0.6f; // CENTRE THE TEXT ON THE ROAD

			// rotate the label to align with the road rotation
			tileResLabel.text->setRotation(glm::radians(angle));
			tileResLabel.text->setScale(txtScale, txtScale);
			tileResLabel.text->commitUpdates();
		}
	}
}

/// <summary>Update renderable icon, dependant on LOD level (for buildings such as; cafe, pub, library etc.)
/// using the pre-processed data (position, type) and UIRenderer.</summary>
/// <param name="col">Column index for tile.</param>
/// <param name="row">Row index for tile.</param>
void OGLESNavigation2D::updateAmenities(uint32_t col, uint32_t row)
{
	TileRenderingResources& tileRes = _tileRenderingResources[col][row];
	const float rotation = -_rotation - MapScreenAlignRotation;
	for (uint32_t lod = _currentScaleLevel; lod < LOD::Count; ++lod)
	{
		for (uint32_t amenityIconIndex = 0; amenityIconIndex < tileRes.amenityIcons[lod].size(); ++amenityIconIndex)
		{
			AmenityIconGroup& amenityIcon = tileRes.amenityIcons[lod][amenityIconIndex];
			debug_assertion(amenityIcon.icon.image != nullptr, "Amenity Icon must be a valid UIRenderer Icon");

			float iconScale = (1.0f / (_scale * 20.0f));
			iconScale = glm::clamp(iconScale, amenityIcon.iconData.scale, amenityIcon.iconData.scale * 2.0f);

			amenityIcon.icon.image->setScale(glm::vec2(iconScale, iconScale));
			amenityIcon.icon.image->commitUpdates();

			// reverse the rotation applied by the camera rotation group
			amenityIcon.group->setRotation(glm::radians(rotation));
			amenityIcon.group->setPixelOffset(static_cast<float>(amenityIcon.iconData.coords.x), static_cast<float>(amenityIcon.iconData.coords.y));
			amenityIcon.group->commitUpdates();
		}

		for (uint32_t amenityLabelIndex = 0; amenityLabelIndex < tileRes.amenityLabels[lod].size(); ++amenityLabelIndex)
		{
			AmenityLabelGroup& amenityLabel = tileRes.amenityLabels[lod][amenityLabelIndex];
			if (amenityLabel.label.text == nullptr) { continue; }

			float txtScale = 1.0f / (_scale * 15.0f);

			amenityLabel.label.text->setScale(txtScale, txtScale);
			// move the label below the icon based on the size of the label
			amenityLabel.label.text->setPixelOffset(0.0f, -2.2f * amenityLabel.label.text->getBoundingBox().getHalfExtent().y * txtScale);
			amenityLabel.label.text->commitUpdates();

			// reverse the rotation applied by the camera rotation group
			amenityLabel.group->setRotation(glm::radians(rotation));
			amenityLabel.group->setPixelOffset(static_cast<float>(amenityLabel.iconData.coords.x), static_cast<float>(amenityLabel.iconData.coords.y));
			amenityLabel.group->commitUpdates();
		}
	}
}

/// <summary>This function must be implemented by the user of the shell. The user should return its pvr::Shell object defining the behaviour of the application.</summary>
/// <returns>Return a unique ptr to the demo supplied by the user.</returns>
std::unique_ptr<pvr::Shell> pvr::newDemo() { return std::make_unique<OGLESNavigation2D>(); }
