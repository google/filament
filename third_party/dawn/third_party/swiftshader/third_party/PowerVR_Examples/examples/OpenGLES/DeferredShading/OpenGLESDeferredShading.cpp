/*!
\brief This demo demonstrates how to implement Deferred Shading efficiently OpenGL ES.
\file OpenGLESDeferredShading.cpp
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/

#include "PVRShell/PVRShell.h"
#include "PVRAssets/PVRAssets.h"
#include "PVRUtils/PVRUtilsGles.h"

/// <summary>Shader vertex Bindings </summary>
const pvr::utils::VertexBindings_Name vertexBindings[] = { { "POSITION", "inVertex" }, { "NORMAL", "inNormal" }, { "UV0", "inTexCoords" }, { "TANGENT", "inTangent" } };

namespace AttributeIndices {
enum Enum
{
	VertexArray = 0,
	NormalArray = 1,
	TexCoordArray = 2,
	TangentArray = 3
};
}

const pvr::utils::VertexBindings_Name floorVertexBindings[] = { { "POSITION", "inVertex" }, { "NORMAL", "inNormal" }, { "UV0", "inTexCoords" } };

namespace FloorAttributeIndices {
enum Enum
{
	VertexArray = 0,
	NormalArray = 1,
	TexCoordArray = 2
};
}

const pvr::utils::VertexBindings_Name pointLightVertexBindings[] = { { "POSITION", "inVertex" } };

enum class pointLightAttributeIndices
{
	VertexArray = 0
};

namespace BufferBindings {
enum Enum
{
	Matrices = 0,
	Materials = 1,
	DirectionalLightStaticData = 0,
	DirectionalLightDynamicData = 1,
	PointLightDynamicData = 0,
	PointLightStaticData = 1,
};
}

/// <summary>Light mesh nodes.</summary>
enum class LightNodes
{
	PointLightMeshNode = 0,
	NumberOfPointLightMeshNodes
};

enum class MeshNodes
{
	Satyr = 0,
	Floor = 1,
	NumberOfMeshNodes
};

/// <summary>Structures used for storing the shared point light data for the point light passes.</summary>
struct PointLightPasses
{
	struct PointLightProperties
	{
		glm::mat4 worldViewProjectionMatrix;
		glm::mat4 proxyWorldViewMatrix;
		glm::mat4 proxyWorldViewProjectionMatrix;
		glm::vec4 proxyViewSpaceLightPosition;
		glm::vec4 lightColor;
		glm::vec4 lightSourceColor;
		float lightIntensity;
		float lightRadius;
	};

	std::vector<PointLightProperties> lightProperties;

	struct InitialData
	{
		float radial_vel;
		float axial_vel;
		float vertical_vel;
		float angle;
		float distance;
		float height;
	};

	std::vector<InitialData> initialData;
};

/// <summary>structure used to draw the point light sources.</summary>
struct DrawPointLightSources
{
	GLuint program;
};

/// <summary>structure used to draw the proxy point light.</summary>
struct DrawPointLightProxy
{
	GLuint program;
	GLuint farClipDistanceLocation;
};

/// <summary>structure used to fill the stencil buffer used for optimising the proxy point light pass.</summary>
struct PointLightGeometryStencil
{
	GLuint program;
};

/// <summary>structure used to render directional lighting.</summary>
struct DrawDirectionalLight
{
	GLuint program;

	struct DirectionalLightProperties
	{
		glm::vec4 lightIntensity;
		glm::vec4 viewSpaceLightDirection;
	};
	std::vector<DirectionalLightProperties> lightProperties;
};

/// <summary>structure used to blit the contents of pls.color to the main framebuffer.</summary>
struct BlitPlsToFbo
{
	GLuint program;
};

/// <summary>structure used to fill the GBuffer.</summary>
struct DrawGBuffer
{
	struct Objects
	{
		GLuint program;
		glm::mat4 world;
		glm::mat4 worldView;
		glm::mat4 worldViewProj;
		glm::mat4 worldViewIT4x4;
		GLuint farClipDistanceLocation;
	};
	std::vector<Objects> objects;
};

/// <summary>structure used to hold the rendering information for the demo.</summary>
struct RenderData
{
	DrawGBuffer renderGBuffer; // pass 0
	DrawDirectionalLight directionalLightPass; // pass 1
	PointLightGeometryStencil pointLightGeometryStencilPass; // pass 1
	DrawPointLightProxy pointLightProxyPass; // pass 1
	DrawPointLightSources pointLightSourcesPass; // pass 1
	PointLightPasses pointLightPasses; // holds point light data
	BlitPlsToFbo writePlsToFbo; // blits the contents of pls.color to the main framebuffer
};

namespace UniformNames {
const std::string FarClipDistance = "fFarClipDistance";
const std::string DiffuseTexture = "sTexture";
const std::string BumpmapTexture = "sBumpMap";
} // namespace UniformNames

namespace TextureIndices {
uint32_t DiffuseTexture = 0;
uint32_t BumpmapTexture = 1;
} // namespace TextureIndices

namespace BufferIndices {
uint32_t Matrices = 0;
uint32_t Material = 1;
uint32_t PointLightProperties = 1;
uint32_t PointLightMatrices = 0;
uint32_t DirectionalLightStatic = 0;
uint32_t DirectionalLightDynamic = 1;
} // namespace BufferIndices

/// <summary>Shader names for all of the demo passes.</summary>
namespace Files {
const std::string PointLightModelFile = "pointlight.pod";
const std::string SceneFile = "SatyrAndTable.pod";

const std::string GBufferVertexShader = "GBufferVertexShader.vsh";
const std::string GBufferFragmentShader = "GBufferFragmentShader.fsh";

const std::string GBufferFloorVertexShader = "GBufferFloorVertexShader.vsh";
const std::string GBufferFloorFragmentShader = "GBufferFloorFragmentShader.fsh";

const std::string AttributelessVertexShader = "AttributelessVertexShader.vsh";

const std::string WritePlsToFboShader = "WritePlsToFbo.fsh";

const std::string DirectionalLightingFragmentShader = "DirectionalLightFragmentShader.fsh";

const std::string PointLightPass1FragmentShader = "PointLightPass1FragmentShader.fsh";
const std::string PointLightPass1VertexShader = "PointLightPass1VertexShader.vsh";

const std::string PointLightPass2FragmentShader = "PointLightPass2FragmentShader.fsh";
const std::string PointLightPass2VertexShader = "PointLightPass2VertexShader.vsh";

const std::string PointLightPass3FragmentShader = "PointLightPass3FragmentShader.fsh";
const std::string PointLightPass3VertexShader = "PointLightPass3VertexShader.vsh";
} // namespace Files

namespace BufferEntryNames {
namespace PerModelMaterial {
const std::string SpecularStrength = "fSpecularStrength";
const std::string DiffuseColor = "vDiffuseColor";
} // namespace PerModelMaterial

namespace PerModel {
const std::string WorldViewProjectionMatrix = "mWorldViewProjectionMatrix";
const std::string WorldViewMatrix = "mWorldViewMatrix";
const std::string WorldViewITMatrix = "mWorldViewITMatrix";
} // namespace PerModel

namespace StaticDirectionalLight {
const std::string LightIntensity = "vLightIntensity";
const std::string AmbientLight = "vAmbientLight";
} // namespace StaticDirectionalLight

namespace DynamicDirectionalLight {
const std::string ViewSpaceLightDirection = "vViewSpaceLightDirection";
}

namespace StaticPointLight {
const std::string LightIntensity = "fLightIntensity";
const std::string LightRadius = "fLightRadius";
const std::string LightColor = "vLightColor";
const std::string LightSourceColor = "vLightSourceColor";
} // namespace StaticPointLight

namespace DynamicPointLight {
const std::string WorldViewProjectionMatrix = "mWorldViewProjectionMatrix";
const std::string ViewPosition = "vViewPosition";
const std::string ProxyWorldViewProjectionMatrix = "mProxyWorldViewProjectionMatrix";
const std::string ProxyWorldViewMatrix = "mProxyWorldViewMatrix";
} // namespace DynamicPointLight
} // namespace BufferEntryNames

/// <summary>Application wide configuration data.</summary>
namespace ApplicationConfiguration {
const float FrameRate = 1.0f / 120.0f;
}

/// <summary>Directional lighting configuration data.</summary>
namespace DirectionalLightConfiguration {
static bool AdditionalDirectionalLight = true;
const float DirectionalLightIntensity = .1f;
const glm::vec4 AmbientLightColor = glm::vec4(.005f, .005f, .005f, 0.0f);
} // namespace DirectionalLightConfiguration

/// <summary>Point lighting configuration data.</summary>
namespace PointLightConfiguration {
float LightMaxDistance = 40.f;
float LightMinDistance = 20.f;
float LightMinHeight = -30.f;
float LightMaxHeight = 40.f;
float LightAxialVelocityChange = .01f;
float LightRadialVelocityChange = .003f;
float LightVerticalVelocityChange = .01f;
float LightMaxAxialVelocity = 5.f;
float LightMaxRadialVelocity = 1.5f;
float LightMaxVerticalVelocity = 5.f;

uint32_t MaxScenePointLights = 5;
int32_t NumProceduralPointLights = 10;
float PointlightIntensity = 20.f;
float PointLightMinIntensityForCuttoff = 10.f / 255.f;
float PointLightMaxRadius = 1.5f * glm::sqrt(PointLightConfiguration::PointlightIntensity / PointLightConfiguration::PointLightMinIntensityForCuttoff);
// The "Max radius" value we find is 50% more than the radius where we reach a specific light value.
// Light attenuation is quadratic: Light value = Intensity / Distance ^2
// The problem is that with this equation, light has infinite radius, as it asymptotically goes to zero as distance increases
// Very big radius is in general undesirable for deferred shading where you wish to have a lot of small lights, and where there
// contribution will be small to none, but a sharp cut off is usually quite visible on dark scenes.
// For that reason, we have implemented an attenuation equation which begins close to the light following this value,
// but then after a predetermined value, switches to linear falloff and continues to zero following the same slope.
// This can be tweaked through this vale: It basically says "At which light intensity should the quadratic equation
// be switched to a linear one and trail to zero?".
// Following the numbers, if we follow the slope of 1/x^2 linearly, the value becomes exactly zero at 1.5 x distance.
// Good guide values here are around 5.f/255.f for a sharp falloff (but hence better performance as less pixels are shaded
// up to ~1.f/255.f for almost undetectably soft falloff in pitch-black scenes (hence more correct, but shading a lot
// of pixels that have a miniscule lighting contribution).
// Additionally, if there is a strong ambient or directional, this value can be increased (hence reducing the number of pixels
// shaded) as the ambient light will completely hide the small contributions of the edges of the point lights. Reversely,
// a completely dark scene would only be acceptable with values less than 2.f as otherwise the cut off of the lights would be
// quite visible.
// NUMBERS: ( Symbols: Light Value: LV, Differential of LV: LV' Intensity: I, Distance: D, Distance of switch quadratic->linear:A)
// After doing some number-crunching, starting with LV = I / D^2
// LV = I * (3 * A^2 - 2 * D / A^3). See the PointLightPass2FragmentShader.
// Finally, crunching more numbers you will find that LV drops to zero when D = 1.5 * A, so we need to render the lights
// with a radius of 1.5 * A. In the shader, this is reversed to precisely find the point where we switch from quadratic to linear.

} // namespace PointLightConfiguration

/// <summary>Class implementing the Shell functions.</summary>
class OpenGLESDeferredShading : public pvr::Shell
{
public:
	struct Material
	{
		GLuint diffuseTexture;
		GLuint bumpmapTexture;

		float specularStrength;
		glm::vec4 diffuseColor;
		Material() : diffuseTexture(-1), bumpmapTexture(-1) {}
	};
	struct DeviceResources
	{
		pvr::EglContext context;

		GLuint modelMaterialUbo;
		GLuint modelMatrixUbo;
		pvr::utils::StructuredBufferView modelMatrixBufferView;

		pvr::utils::StructuredBufferView modelMaterialBufferView;

		GLuint pointLightPropertiesUbo;
		GLuint pointLightMatrixUbo;
		pvr::utils::StructuredBufferView staticDirectionalLightBufferView;

		pvr::utils::StructuredBufferView dynamicDirectionalLightBufferView;

		GLuint directionalLightStaticDataUbo;
		GLuint directionalLightDynamicDataUbo;
		pvr::utils::StructuredBufferView staticPointLightBufferView;

		pvr::utils::StructuredBufferView dynamicPointLightBufferView;

		// Samplers
		GLuint samplerTrilinear;

		pvr::utils::VertexConfiguration sceneVertexConfigurations[static_cast<uint32_t>(MeshNodes::NumberOfMeshNodes)];
		std::vector<GLuint> sceneVaos;
		std::vector<GLuint> sceneVbos;
		std::vector<GLuint> sceneIbos;

		pvr::utils::VertexConfiguration pointLightVertexConfiguration;
		GLuint pointLightVao;
		GLuint pointLightVbo;
		GLuint pointLightIbo;

		std::vector<Material> materials;

		RenderData renderInfo;

		GLint defaultFbo;

		// UIRenderer used to display text
		pvr::ui::UIRenderer uiRenderer;
	};

	// Putting all API objects into a pointer just makes it easier to release them all together with RAII
	std::unique_ptr<DeviceResources> _deviceResources;

	// 3D Model
	pvr::assets::ModelHandle _mainScene;
	pvr::assets::ModelHandle _pointLightScene;

	// Frame counters for animation
	float _frameNumber;
	bool _isPaused;
	uint32_t _cameraId;
	bool _animateCamera;

	uint32_t _numberOfPointLights;
	uint32_t _numberOfDirectionalLights;

	// Projection and Model View matrices
	glm::vec3 _cameraPosition;
	glm::mat4 _viewMatrix;
	glm::mat4 _projectionMatrix;
	glm::mat4 _viewProjectionMatrix;
	glm::mat4 _inverseViewMatrix;
	float _farClipDistance;

	int32_t _windowWidth;
	int32_t _windowHeight;

	bool _simpleGammaFunction;
	bool _pixelLocalStorageSupported;
	bool _pixelLocalStorage2Supported;
	bool _bufferStorageExtSupported;

	GLuint _sizeOfPixelLocationStorage;

	GLint _uniformAlignment;

	glm::vec4 _clearColor;

	OpenGLESDeferredShading()
	{
		_animateCamera = false;
		_isPaused = false;
	}

	//  Overridden from pvr::Shell
	virtual pvr::Result initApplication();
	virtual pvr::Result initView();
	virtual pvr::Result releaseView();
	virtual pvr::Result quitApplication();
	virtual pvr::Result renderFrame();

	void createPrograms();
	void uploadStaticData();
	void uploadStaticModelData();
	void uploadStaticDirectionalLightData();
	void uploadStaticPointLightData();
	void createBuffers();
	void createGeometryBuffers();
	void bindVertexSpecification(const pvr::assets::Mesh& mesh, const pvr::utils::VertexBindings_Name* const vertexBindingsName, const uint32_t numVertexBindings,
		pvr::utils::VertexConfiguration& vertexConfiguration, GLuint& vao, GLuint& vbo, GLuint& ibo);
	void createModelBuffers();
	void createPointLightBuffers();
	void createDirectionalLightBuffers();
	void initialiseStaticLightProperties();
	void allocateLights();
	void updateDynamicSceneData();
	void updateProceduralPointLight(PointLightPasses::InitialData& data, PointLightPasses::PointLightProperties& pointLightProperties, bool initial);
	void createSamplers();
	void createMaterialTextures();
	void updateAnimation();

	void setDefaultStates();
	void bindAndClearFramebuffer();
	void endFramebuffer();
	void renderGBuffer();
	void renderDirectionalLights();
	void renderPointLights();
	void renderPointLightProxyGeometryIntoStencilBuffer();
	void renderPointLightProxy();
	void renderPointLightSources();
	void renderPlsToFbo();
	void renderUi();

	void eventMappedInput(pvr::SimplifiedInput key)
	{
		switch (key)
		{
		// Handle input
		case pvr::SimplifiedInput::ActionClose: exitShell(); break;
		case pvr::SimplifiedInput::Action1: _isPaused = !_isPaused; break;
		case pvr::SimplifiedInput::Action2: _animateCamera = !_animateCamera; break;
		default: break;
		}
	}
};

/// <summary>This class is added as the Debug Callback. Redirects the debug output to the Log object.</summary>
inline void GL_APIENTRY debugCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* userParam)
{
	(void)length;
	(void)severity;
	(void)userParam;
	Log(LogLevel::Debug, "[%d|%d|%d] %s", source, type, id, message);
}

/// <summary>Code in initApplication() will be called by Shell once per run, before the rendering context is created.
/// Used to initialize variables that are not dependent on it(e.g.external modules, loading meshes, etc.).If the rendering
/// context is lost, initApplication() will not be called again.</summary>
/// <returns> Result::Success if no error occurred. </returns>
pvr::Result OpenGLESDeferredShading::initApplication()
{
	setStencilBitsPerPixel(8);

	_frameNumber = 0.0f;
	_isPaused = false;
	_cameraId = 0;

	_clearColor = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);

	//  Load the _mainScene and the light
	_mainScene = pvr::assets::loadModel(*this, Files::SceneFile.c_str());

	if (_mainScene->getNumCameras() == 0)
	{
		setExitMessage("ERROR: The main _mainScene to display must contain a camera.\n");
		return pvr::Result::UnknownError;
	}

	//  Load light proxy geometry
	_pointLightScene = pvr::assets::loadModel(*this, Files::PointLightModelFile.c_str());

	return pvr::Result::Success;
}

/// <summary>Code in initView() will be called by Shell upon initialization or after a change
/// in the rendering context. Used to initialize variables that are dependent on the
/// rendering context(e.g.textures, vertex buffers, etc.).</summary>
/// <returns> Result::Success if no error occurred. </returns>
pvr::Result OpenGLESDeferredShading::initView()
{
	_deviceResources = std::make_unique<DeviceResources>();
	_deviceResources->context = pvr::createEglContext();

	_deviceResources->context->init(getWindow(), getDisplay(), getDisplayAttributes(), pvr::Api::OpenGLES31);

	// Check if pixel local storage extensions are supported
	if (gl::isGlExtensionSupported("GL_KHR_debug")) { gl::ext::DebugMessageCallbackKHR(&debugCallback, NULL); }

	_pixelLocalStorageSupported = gl::isGlExtensionSupported("GL_EXT_shader_pixel_local_storage");
	_pixelLocalStorage2Supported = gl::isGlExtensionSupported("GL_EXT_shader_pixel_local_storage2");

	if (!gl::isGlExtensionSupported("GL_EXT_color_buffer_float"))
	{
		setExitMessage("Floating point framebuffer targets are not supported.");
		return pvr::Result::UnknownError;
	}

	const pvr::CommandLine& commandOptions = getCommandLine();

	commandOptions.getIntOption("-numlights", PointLightConfiguration::NumProceduralPointLights);
	commandOptions.getFloatOption("-lightintensity", PointLightConfiguration::PointlightIntensity);
	_simpleGammaFunction = commandOptions.hasOption("-simpleGamma");

	if (!_pixelLocalStorageSupported && !_pixelLocalStorage2Supported)
	{
		setExitMessage("Pixel local storage is not supported.");
		return pvr::Result::UnknownError;
	}
	else
	{
		if (_pixelLocalStorage2Supported) { Log(LogLevel::Information, "GL_EXT_shader_pixel_local_storage2 is supported."); }
		else
		{
			Log(LogLevel::Information, "GL_EXT_shader_pixel_local_storage is supported (GL_EXT_shader_pixel_local_storage2 is not supported).");
		}
	}

	// setup UI renderer
	_deviceResources->uiRenderer.init(getWidth(), getHeight(), isFullScreen(), getBackBufferColorspace() == pvr::ColorSpace::sRGB);
	_deviceResources->uiRenderer.getDefaultTitle()->setText("DeferredShading");
	_deviceResources->uiRenderer.getDefaultTitle()->commitUpdates();
	_deviceResources->uiRenderer.getDefaultControls()->setText("Action1: Pause\n"
															   "Action2: Orbit Camera\n");
	_deviceResources->uiRenderer.getDefaultControls()->commitUpdates();

	_windowWidth = getWidth();
	_windowHeight = getHeight();

	// initialise the gbuffer renderpass list
	_deviceResources->renderInfo.renderGBuffer.objects.resize(_mainScene->getNumMeshNodes());

	Log(LogLevel::Information, "Onscreen Framebuffer dimensions: %d x %d\n", _windowWidth, _windowHeight);

	createSamplers();
	createMaterialTextures();

	if (isScreenRotated())
	{
		_projectionMatrix = pvr::math::perspectiveFov(pvr::Api::OpenGLES31, _mainScene->getCamera(0).getFOV(), static_cast<float>(_windowHeight), static_cast<float>(_windowWidth),
			_mainScene->getCamera(0).getNear(), _mainScene->getCamera(0).getFar(), glm::pi<float>() * .5f);
	}
	else
	{
		_projectionMatrix = glm::perspectiveFov(_mainScene->getCamera(0).getFOV(), static_cast<float>(_windowWidth), static_cast<float>(_windowHeight),
			_mainScene->getCamera(0).getNear(), _mainScene->getCamera(0).getFar());
	}

	createPrograms();

	// Initialise lighting structures
	allocateLights();

	// Create buffers used in the demo
	createBuffers();

	// Initialise the static light properties
	initialiseStaticLightProperties();

	// Upload static data
	uploadStaticData();

	setDefaultStates();

	gl::GetIntegerv(GL_FRAMEBUFFER_BINDING, &_deviceResources->defaultFbo);

	gl::Viewport(0, 0, _windowWidth, _windowHeight);

	if (_pixelLocalStorage2Supported)
	{
		gl::BindFramebuffer(GL_DRAW_FRAMEBUFFER, _deviceResources->defaultFbo);

		// calculate the size in bytes of pixel local storage
		_sizeOfPixelLocationStorage = 0;
		_sizeOfPixelLocationStorage += 4; // albedo
		_sizeOfPixelLocationStorage += 4; // normals
		_sizeOfPixelLocationStorage += 4; // depth
		_sizeOfPixelLocationStorage += 4; // colour

		// specifies the amount of storage required for pixel local variables whilst pls is enabled
		gl::ext::FramebufferPixelLocalStorageSizeEXT(GL_DRAW_FRAMEBUFFER, _sizeOfPixelLocationStorage);
	}

	gl::ClearColor(_clearColor.r, _clearColor.g, _clearColor.b, _clearColor.a);
	gl::ClearDepthf(1.0f);
	gl::ClearStencil(0);

	return pvr::Result::Success;
}

/// <summary>Main rendering loop function of the program. The shell will call this function every frame.</summary>
/// <returns> Result::Success if no error occurred.</returns>
pvr::Result OpenGLESDeferredShading::renderFrame()
{
	debugThrowOnApiError("Frame begin");

	//  Handle user input and update object animations
	updateAnimation();
	updateDynamicSceneData();

	gl::Enable(GL_DEPTH_TEST);

	bindAndClearFramebuffer();

	// enable pixel local storage
	gl::Enable(GL_SHADER_PIXEL_LOCAL_STORAGE_EXT);

	if (_pixelLocalStorage2Supported)
	{
		// clears pixel local storage to 0
		gl::ext::ClearPixelLocalStorageuiEXT(0, _sizeOfPixelLocationStorage / 4, NULL);
	}

	// render the gbuffer
	{
		renderGBuffer();
	}

	// render directional light
	{
		renderDirectionalLights();
	}

	// render point light
	{
		renderPointLights();
	}

	// out PLS to Fbo
	{
		renderPlsToFbo();
	}

	// disable pixel local storage
	gl::Disable(GL_SHADER_PIXEL_LOCAL_STORAGE_EXT);

	{
		renderUi();
	}

	endFramebuffer();

	debugThrowOnApiError("Frame end");

	if (this->shouldTakeScreenshot()) { pvr::utils::takeScreenshot(this->getScreenshotFileName(), this->getWidth(), this->getHeight()); }

	_deviceResources->context->swapBuffers();

	return pvr::Result::Success;
}

/// <summary>Code in releaseView() will be called by Shell when the application quits.</summary>
/// <returns> Result::Success if no error occurred.</returns>
pvr::Result OpenGLESDeferredShading::releaseView()
{
	_deviceResources.reset();
	return pvr::Result::Success;
}

/// <summary>Code in quitApplication() will be called by pvr::Shell once per run, just before exiting the program.</summary>
/// <returns> Result::Success if no error occurred</returns>.
pvr::Result OpenGLESDeferredShading::quitApplication()
{
	_mainScene.reset();
	_pointLightScene.reset();

	return pvr::Result::Success;
}

void OpenGLESDeferredShading::bindAndClearFramebuffer()
{
	// disable depth writes
	gl::DepthMask(GL_TRUE);

	gl::BindFramebuffer(GL_DRAW_FRAMEBUFFER, _deviceResources->defaultFbo);
	gl::Clear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
}

void OpenGLESDeferredShading::endFramebuffer()
{
	std::vector<GLenum> invalidateAttachments;
	invalidateAttachments.push_back(GL_DEPTH);
	invalidateAttachments.push_back(GL_STENCIL);

	gl::InvalidateFramebuffer(GL_FRAMEBUFFER, (GLsizei)invalidateAttachments.size(), &invalidateAttachments[0]);
}

void OpenGLESDeferredShading::setDefaultStates()
{
	gl::BindFramebuffer(GL_FRAMEBUFFER, _deviceResources->context->getOnScreenFbo());
	gl::UseProgram(0);

	gl::Disable(GL_BLEND);

	gl::Enable(GL_DEPTH_TEST); // depth test
	gl::DepthMask(GL_TRUE); // depth write enabled
	gl::DepthFunc(GL_LESS);

	gl::Enable(GL_CULL_FACE);
	gl::CullFace(GL_BACK);
	gl::FrontFace(GL_CCW);

	gl::Enable(GL_STENCIL_TEST);
	gl::StencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
	gl::StencilFunc(GL_ALWAYS, 0, 255);
	gl::StencilMask(255);
}

void OpenGLESDeferredShading::renderGBuffer()
{
	DrawGBuffer& pass = _deviceResources->renderInfo.renderGBuffer;

	gl::StencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
	gl::StencilFunc(GL_ALWAYS, 1, 255);
	gl::StencilMask(255);

	for (uint32_t i = 0; i < _mainScene->getNumMeshNodes(); ++i)
	{
		gl::UseProgram(pass.objects[i].program);

		const pvr::assets::Model::Node& node = _mainScene->getNode(i);
		const pvr::assets::Mesh& mesh = _mainScene->getMesh(node.getObjectId());

		const Material& material = _deviceResources->materials[node.getMaterialIndex()];

		gl::BindBufferRange(GL_UNIFORM_BUFFER, BufferBindings::Matrices, _deviceResources->modelMatrixUbo, _deviceResources->modelMatrixBufferView.getDynamicSliceOffset(i),
			(size_t)_deviceResources->modelMatrixBufferView.getDynamicSliceSize());

		gl::BindBufferRange(GL_UNIFORM_BUFFER, BufferBindings::Materials, _deviceResources->modelMaterialUbo, _deviceResources->modelMaterialBufferView.getDynamicSliceOffset(i),
			(size_t)_deviceResources->modelMaterialBufferView.getDynamicSliceSize());

		if (material.diffuseTexture != static_cast<uint32_t>(-1))
		{
			gl::ActiveTexture(GL_TEXTURE0);
			gl::BindSampler(0, _deviceResources->samplerTrilinear);
			gl::BindTexture(GL_TEXTURE_2D, material.diffuseTexture);
		}
		if (material.bumpmapTexture != static_cast<uint32_t>(-1))
		{
			gl::ActiveTexture(GL_TEXTURE1);
			gl::BindSampler(1, _deviceResources->samplerTrilinear);
			gl::BindTexture(GL_TEXTURE_2D, material.bumpmapTexture);
		}

		gl::BindVertexArray(_deviceResources->sceneVaos[i]);

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
}

void OpenGLESDeferredShading::renderDirectionalLights()
{
	// DIRECTIONAL LIGHTING - A full-screen quad that will apply any global (ambient/directional) lighting
	// disable the depth write as we do not want to modify the depth buffer while rendering directional lights

	// set winding order
	gl::FrontFace(GL_CW);

	// enable front face culling
	gl::CullFace(GL_FRONT);

	// disable depth testing and depth writing
	gl::Disable(GL_DEPTH_TEST);
	gl::DepthMask(GL_FALSE);

	// pass if the stencil equals 1 i.e. there is some geometry present
	gl::StencilFunc(GL_EQUAL, 1, 255);
	// disable stencil writes
	gl::StencilMask(0);

	// if for the current fragment the stencil has been filled then there is geometry present
	// and directional lighting calculations should be carried out
	gl::UseProgram(_deviceResources->renderInfo.directionalLightPass.program);

	// Make use of the stencil buffer contents to only shade pixels where actual geometry is located.
	for (uint32_t i = 0; i < _numberOfDirectionalLights; i++)
	{
		gl::BindBufferRange(GL_UNIFORM_BUFFER, BufferBindings::DirectionalLightStaticData, _deviceResources->directionalLightStaticDataUbo,
			_deviceResources->staticDirectionalLightBufferView.getDynamicSliceOffset(i), (size_t)_deviceResources->staticDirectionalLightBufferView.getDynamicSliceSize());

		gl::BindBufferRange(GL_UNIFORM_BUFFER, BufferBindings::DirectionalLightDynamicData, _deviceResources->directionalLightDynamicDataUbo,
			_deviceResources->dynamicDirectionalLightBufferView.getDynamicSliceOffset(i), (size_t)_deviceResources->dynamicDirectionalLightBufferView.getDynamicSliceSize());

		gl::DrawArrays(GL_TRIANGLE_STRIP, 0, 3);
	}

	// reset winding order
	gl::FrontFace(GL_CCW);
	gl::StencilMask(255);
}

void OpenGLESDeferredShading::renderPointLights()
{
	gl::BindVertexArray(_deviceResources->pointLightVao);

	for (uint32_t i = 0; i < _numberOfPointLights; i++)
	{
		// clear the stencil buffer so that the point light passes can make use of it
		gl::Clear(GL_STENCIL_BUFFER_BIT);

		gl::BindBufferRange(GL_UNIFORM_BUFFER, BufferBindings::PointLightDynamicData, _deviceResources->pointLightMatrixUbo,
			_deviceResources->dynamicPointLightBufferView.getDynamicSliceOffset(i), (size_t)_deviceResources->dynamicPointLightBufferView.getDynamicSliceSize());

		renderPointLightProxyGeometryIntoStencilBuffer();
		gl::BindBufferRange(GL_UNIFORM_BUFFER, BufferBindings::PointLightStaticData, _deviceResources->pointLightPropertiesUbo,
			_deviceResources->staticPointLightBufferView.getDynamicSliceOffset(i), (size_t)_deviceResources->staticPointLightBufferView.getDynamicSliceSize());

		renderPointLightProxy();
	}
	renderPointLightSources();

	gl::BindVertexArray(0);
}

void OpenGLESDeferredShading::renderPlsToFbo()
{
	// Output the contents of PLS to the main framebuffer - A full-screen quad will simply blit the contents of pls.color to the screen

	// set winding order
	gl::FrontFace(GL_CW);

	// enable front face culling
	gl::CullFace(GL_FRONT);

	// disable depth testing and depth writing
	gl::Disable(GL_DEPTH_TEST);
	gl::DepthMask(GL_FALSE);

	// disable stencil testing and stencil writing
	gl::Disable(GL_STENCIL_TEST);
	gl::StencilMask(0);

	gl::UseProgram(_deviceResources->renderInfo.writePlsToFbo.program);

	gl::DrawArrays(GL_TRIANGLE_STRIP, 0, 3);
}

void OpenGLESDeferredShading::renderPointLightProxyGeometryIntoStencilBuffer()
{
	// POINT LIGHTS GEOMETRY STENCIL PASS
	// Render the front face of each light volume
	// Z function is set as Less/Equal
	// Z test passes will leave the stencil as 0 i.e. the front of the light is in front of all geometry in the current pixel
	//    This is the condition we want for determining whether the geometry can be affected by the point lights
	// Z test fails will increment the stencil to 1. i.e. the front of the light is behind all of the geometry in the current pixel
	//    Under this condition the current pixel cannot be affected by the current point light as the geometry is in front of the front of the point light

	// disable colour writing
	gl::ColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);

	// enable back face culling
	gl::CullFace(GL_BACK);

	// enable depth testing
	gl::Enable(GL_DEPTH_TEST);
	// disable depth writing
	gl::DepthMask(GL_FALSE);
	// change the depth test function to less than or equal
	gl::DepthFunc(GL_LEQUAL);

	gl::StencilOp(GL_KEEP, GL_INCR, GL_KEEP);
	gl::StencilFunc(GL_ALWAYS, 0, 255);

	PointLightGeometryStencil& pointGeometryStencilPass = _deviceResources->renderInfo.pointLightGeometryStencilPass;

	const pvr::assets::Mesh& mesh = _pointLightScene->getMesh(static_cast<uint32_t>(LightNodes::PointLightMeshNode));

	gl::UseProgram(pointGeometryStencilPass.program);

	GLenum primitiveType = pvr::utils::convertToGles(mesh.getPrimitiveType());
	auto indextype = mesh.getFaces().getDataType();
	GLenum indexgltype = (indextype == pvr::IndexType::IndexType16Bit ? GL_UNSIGNED_SHORT : GL_UNSIGNED_INT);
	// Indexed Triangle list
	gl::DrawElements(primitiveType, mesh.getNumFaces() * 3, indexgltype, 0);
}

void OpenGLESDeferredShading::renderPointLightProxy()
{
	// POINT LIGHTS PROXIES - Actually light the pixels touched by a point light.
	// Render the back faces of the light volumes
	// Z function is set as Greater/Equal
	// Z test passes signify that there is geometry in front of the back face of the light volume i.e. for the current pixel there is
	// some geometry in front of the back face of the light volume
	// Stencil function is Equal i.e. the stencil reference is set to 0
	// Stencil passes signify that for the current pixel there exists a front face of a light volume in front of the current geometry
	// Point light calculations occur every time a pixel passes both the stencil AND Z test

	gl::ColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);

	// enable front face culling - cull the front faces of the light sources
	gl::CullFace(GL_FRONT);

	// change depth function to greater than or equal
	gl::DepthFunc(GL_GEQUAL);

	// if stencil state equals 0 then the lighting should take place as there is geometry inside the point lights area
	gl::StencilFunc(GL_EQUAL, 0, 255);
	gl::StencilOp(GL_ZERO, GL_ZERO, GL_ZERO);

	DrawPointLightProxy& pointLightProxyPass = _deviceResources->renderInfo.pointLightProxyPass;

	const pvr::assets::Mesh& mesh = _pointLightScene->getMesh(static_cast<uint32_t>(LightNodes::PointLightMeshNode));

	gl::UseProgram(pointLightProxyPass.program);

	GLenum primitiveType = pvr::utils::convertToGles(mesh.getPrimitiveType());
	auto indextype = mesh.getFaces().getDataType();
	GLenum indexgltype = (indextype == pvr::IndexType::IndexType16Bit ? GL_UNSIGNED_SHORT : GL_UNSIGNED_INT);
	// Indexed Triangle list
	gl::DrawElements(primitiveType, mesh.getNumFaces() * 3, indexgltype, 0);
}

void OpenGLESDeferredShading::renderPointLightSources()
{
	// LIGHT SOURCES : Rendering the "will-o-wisps" that are the sources of the light

	// enable back face culling
	gl::CullFace(GL_BACK);

	// disable stencil testing
	gl::Disable(GL_STENCIL_TEST);

	gl::Enable(GL_DEPTH_TEST);
	gl::DepthFunc(GL_LEQUAL);
	// enable depth writes
	gl::DepthMask(GL_TRUE);

	DrawPointLightSources& pointLightSourcePass = _deviceResources->renderInfo.pointLightSourcesPass;

	const pvr::assets::Mesh& mesh = _pointLightScene->getMesh(static_cast<uint32_t>(LightNodes::PointLightMeshNode));

	gl::UseProgram(pointLightSourcePass.program);

	for (uint32_t i = 0; i < _numberOfPointLights; i++)
	{
		gl::BindBufferRange(GL_UNIFORM_BUFFER, BufferBindings::PointLightStaticData, _deviceResources->pointLightPropertiesUbo,
			_deviceResources->staticPointLightBufferView.getDynamicSliceOffset(i), (size_t)_deviceResources->staticPointLightBufferView.getDynamicSliceSize());

		gl::BindBufferRange(GL_UNIFORM_BUFFER, BufferBindings::PointLightDynamicData, _deviceResources->pointLightMatrixUbo,
			_deviceResources->dynamicPointLightBufferView.getDynamicSliceOffset(i), (size_t)_deviceResources->dynamicPointLightBufferView.getDynamicSliceSize());

		GLenum primitiveType = pvr::utils::convertToGles(mesh.getPrimitiveType());
		auto indextype = mesh.getFaces().getDataType();
		GLenum indexgltype = (indextype == pvr::IndexType::IndexType16Bit ? GL_UNSIGNED_SHORT : GL_UNSIGNED_INT);
		// Indexed Triangle list
		gl::DrawElements(primitiveType, mesh.getNumFaces() * 3, indexgltype, 0);
	}
}

void OpenGLESDeferredShading::renderUi()
{
	_deviceResources->uiRenderer.beginRendering();
	_deviceResources->uiRenderer.getSdkLogo()->render();
	_deviceResources->uiRenderer.getDefaultTitle()->render();
	_deviceResources->uiRenderer.getDefaultControls()->render();
	_deviceResources->uiRenderer.endRendering();
}

void OpenGLESDeferredShading::createSamplers()
{
	// create trilinear sampler
	gl::GenSamplers(1, &_deviceResources->samplerTrilinear);

	gl::SamplerParameteri(_deviceResources->samplerTrilinear, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	gl::SamplerParameteri(_deviceResources->samplerTrilinear, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	gl::SamplerParameteri(_deviceResources->samplerTrilinear, GL_TEXTURE_WRAP_R, GL_REPEAT);
	gl::SamplerParameteri(_deviceResources->samplerTrilinear, GL_TEXTURE_WRAP_S, GL_REPEAT);
	gl::SamplerParameteri(_deviceResources->samplerTrilinear, GL_TEXTURE_WRAP_T, GL_REPEAT);
	pvr::utils::throwOnGlError("[OpenGLESDeferredShading::createSamplers] Sampler creation failed");
}

/// <summary>Loads the textures required for this example and sets up descriptorSets.</summary>
/// <returns> Return true if no error occurred </returns>
void OpenGLESDeferredShading::createMaterialTextures()
{
	if (_mainScene->getNumMaterials() == 0) { throw pvr::InvalidDataError("[OpenGLESDeferredShading::createMaterialTextures]Number of scene materials cannot be zero"); }

	// Load textures for each material
	_deviceResources->materials.resize(_mainScene->getNumMaterials());
	for (uint32_t i = 0; i < _mainScene->getNumMaterials(); ++i)
	{
		// get the current material
		const pvr::assets::Model::Material& material = _mainScene->getMaterial(i);

		// get material properties
		_deviceResources->materials[i].specularStrength = material.defaultSemantics().getShininess();
		_deviceResources->materials[i].diffuseColor = glm::vec4(material.defaultSemantics().getDiffuse(), 1.0f);

		int numTextures = 0;

		if (material.defaultSemantics().getDiffuseTextureIndex() != static_cast<uint32_t>(-1))
		{
			// load the diffuse texture
			_deviceResources->materials[i].diffuseTexture =
				pvr::utils::textureUpload(*this, _mainScene->getTexture(material.defaultSemantics().getDiffuseTextureIndex()).getName().c_str());

			++numTextures;
		}
		if (material.defaultSemantics().getBumpMapTextureIndex() != static_cast<uint32_t>(-1))
		{
			// Load the bump map
			_deviceResources->materials[i].bumpmapTexture =
				pvr::utils::textureUpload(*this, _mainScene->getTexture(material.defaultSemantics().getBumpMapTextureIndex()).getName().c_str());
		}
	}
}

/// <summary>Create the pipelines for this example.</summary>
/// <returns>Return true if no error occurred </returns>
void OpenGLESDeferredShading::createPrograms()
{
	{ // GBuffer program - bump mapped
		const char* attributeNames[] = { vertexBindings[0].variableName.c_str(), vertexBindings[1].variableName.c_str(), vertexBindings[2].variableName.c_str(),
			vertexBindings[3].variableName.c_str() };
		const uint16_t attributeIndices[] = { static_cast<uint16_t>(AttributeIndices::VertexArray), static_cast<uint16_t>(AttributeIndices::NormalArray),
			static_cast<uint16_t>(AttributeIndices::TexCoordArray), static_cast<uint16_t>(AttributeIndices::TangentArray) };
		const uint32_t numAttributes = 4;

		_deviceResources->renderInfo.renderGBuffer.objects[static_cast<uint32_t>(MeshNodes::Satyr)].program =
			pvr::utils::createShaderProgram(*this, Files::GBufferVertexShader.c_str(), Files::GBufferFragmentShader.c_str(), attributeNames, attributeIndices, numAttributes);

		gl::ProgramUniform1i(_deviceResources->renderInfo.renderGBuffer.objects[static_cast<uint32_t>(MeshNodes::Satyr)].program,
			gl::GetUniformLocation(_deviceResources->renderInfo.renderGBuffer.objects[static_cast<uint32_t>(MeshNodes::Satyr)].program, UniformNames::DiffuseTexture.c_str()),
			TextureIndices::DiffuseTexture);
		gl::ProgramUniform1i(_deviceResources->renderInfo.renderGBuffer.objects[static_cast<uint32_t>(MeshNodes::Satyr)].program,
			gl::GetUniformLocation(_deviceResources->renderInfo.renderGBuffer.objects[static_cast<uint32_t>(MeshNodes::Satyr)].program, UniformNames::BumpmapTexture.c_str()),
			TextureIndices::BumpmapTexture);

		// Store the location of uniforms for later use
		_deviceResources->renderInfo.renderGBuffer.objects[static_cast<uint32_t>(MeshNodes::Satyr)].farClipDistanceLocation =
			gl::GetUniformLocation(_deviceResources->renderInfo.renderGBuffer.objects[static_cast<uint32_t>(MeshNodes::Satyr)].program, UniformNames::FarClipDistance.c_str());
	}

	{ // GBuffer program - flat
		const char* attributeNames[] = { floorVertexBindings[0].variableName.c_str(), floorVertexBindings[1].variableName.c_str(), floorVertexBindings[2].variableName.c_str() };
		const uint16_t attributeIndices[] = { static_cast<uint16_t>(FloorAttributeIndices::VertexArray), static_cast<uint16_t>(FloorAttributeIndices::NormalArray),
			static_cast<uint16_t>(FloorAttributeIndices::TexCoordArray) };
		const uint32_t numAttributes = 3;

		_deviceResources->renderInfo.renderGBuffer.objects[static_cast<uint32_t>(MeshNodes::Floor)].program = pvr::utils::createShaderProgram(
			*this, Files::GBufferFloorVertexShader.c_str(), Files::GBufferFloorFragmentShader.c_str(), attributeNames, attributeIndices, numAttributes);
		gl::ProgramUniform1i(_deviceResources->renderInfo.renderGBuffer.objects[static_cast<uint32_t>(MeshNodes::Floor)].program,
			gl::GetUniformLocation(_deviceResources->renderInfo.renderGBuffer.objects[static_cast<uint32_t>(MeshNodes::Floor)].program, UniformNames::DiffuseTexture.c_str()),
			TextureIndices::DiffuseTexture);

		// Store the location of uniforms for later use
		_deviceResources->renderInfo.renderGBuffer.objects[static_cast<uint32_t>(MeshNodes::Floor)].farClipDistanceLocation =
			gl::GetUniformLocation(_deviceResources->renderInfo.renderGBuffer.objects[static_cast<uint32_t>(MeshNodes::Floor)].program, UniformNames::FarClipDistance.c_str());
	}

	{ // Directional Light program
		_deviceResources->renderInfo.directionalLightPass.program =
			pvr::utils::createShaderProgram(*this, Files::AttributelessVertexShader.c_str(), Files::DirectionalLightingFragmentShader.c_str(), 0, 0, 0);
	}

	{ // Point Light Stencil program
		const char* attributeNames[] = { pointLightVertexBindings[0].variableName.c_str() };
		const uint16_t attributeIndices[] = { static_cast<uint16_t>(pointLightAttributeIndices::VertexArray) };
		const uint32_t numAttributes = 1;

		_deviceResources->renderInfo.pointLightGeometryStencilPass.program = pvr::utils::createShaderProgram(
			*this, Files::PointLightPass1VertexShader.c_str(), Files::PointLightPass1FragmentShader.c_str(), attributeNames, attributeIndices, numAttributes);
	}

	{ // Point Light Proxy program
		const char* attributeNames[] = { pointLightVertexBindings[0].variableName.c_str() };
		const uint16_t attributeIndices[] = { static_cast<uint16_t>(pointLightAttributeIndices::VertexArray) };
		const uint32_t numAttributes = 1;

		_deviceResources->renderInfo.pointLightProxyPass.program = pvr::utils::createShaderProgram(
			*this, Files::PointLightPass2VertexShader.c_str(), Files::PointLightPass2FragmentShader.c_str(), attributeNames, attributeIndices, numAttributes);

		gl::UseProgram(_deviceResources->renderInfo.pointLightProxyPass.program);

		// Store the location of uniforms for later use
		_deviceResources->renderInfo.pointLightProxyPass.farClipDistanceLocation =
			gl::GetUniformLocation(_deviceResources->renderInfo.pointLightProxyPass.program, UniformNames::FarClipDistance.c_str());
	}
	{ // Point light source program
		const char* attributeNames[] = { pointLightVertexBindings[0].variableName.c_str() };
		const uint16_t attributeIndices[] = { static_cast<uint16_t>(pointLightAttributeIndices::VertexArray) };
		const uint32_t numAttributes = 1;

		_deviceResources->renderInfo.pointLightSourcesPass.program = pvr::utils::createShaderProgram(
			*this, Files::PointLightPass3VertexShader.c_str(), Files::PointLightPass3FragmentShader.c_str(), attributeNames, attributeIndices, numAttributes);
	}
	{ // Blit program
		// Enable or disable gamma correction based on if it is automatically performed on the framebuffer or we need to do it in the shader.
		const char* defines[] = { NULL, NULL };
		uint32_t numDefines = 0;
		if (getBackBufferColorspace() == pvr::ColorSpace::sRGB) { defines[numDefines++] = "FRAMEBUFFER_SRGB"; }
		if (_simpleGammaFunction) { defines[numDefines++] = "SIMPLE_GAMMA_FUNCTION"; }

		_deviceResources->renderInfo.writePlsToFbo.program =
			pvr::utils::createShaderProgram(*this, Files::AttributelessVertexShader.c_str(), Files::WritePlsToFboShader.c_str(), 0, 0, 0, defines, numDefines);
	}
}

/// <summary>Updates animation variables and camera matrices.</summary>
void OpenGLESDeferredShading::updateAnimation()
{
	glm::vec3 vTo, vUp;
	float fov;
	_mainScene->getCameraProperties(_cameraId, fov, _cameraPosition, vTo, vUp);

	// Update camera matrices
	static float angle = 0;
	if (_animateCamera) { angle += getFrameTime() / 5000.f; }
	_viewMatrix = glm::lookAt(glm::vec3(sin(angle) * 100.f + vTo.x, vTo.y + 30., cos(angle) * 100.f + vTo.z), vTo, vUp);
	_viewProjectionMatrix = _projectionMatrix * _viewMatrix;
	_inverseViewMatrix = glm::inverse(_viewMatrix);
}

void OpenGLESDeferredShading::createBuffers()
{
	// create the vaos, vbos and ibos
	createGeometryBuffers();

	_bufferStorageExtSupported = gl::isGlExtensionSupported("GL_EXT_buffer_storage");

	// get the uniform buffer offset alignment value
	gl::GetIntegerv(GL_UNIFORM_BUFFER_OFFSET_ALIGNMENT, &_uniformAlignment);

	// create model buffers
	createModelBuffers();

	// create lighting buffers
	createDirectionalLightBuffers();
	createPointLightBuffers();
}

/// <summary>Creates the buffers used for rendering the models.</summary>
void OpenGLESDeferredShading::createModelBuffers()
{
	{
		pvr::utils::StructuredMemoryDescription description;
		description.addElement(BufferEntryNames::PerModelMaterial::SpecularStrength, pvr::GpuDatatypes::Float);
		description.addElement(BufferEntryNames::PerModelMaterial::DiffuseColor, pvr::GpuDatatypes::vec4);
		_deviceResources->modelMaterialBufferView.initDynamic(description, _mainScene->getNumMeshNodes(), pvr::BufferUsageFlags::UniformBuffer, _uniformAlignment);

		gl::GenBuffers(1, &_deviceResources->modelMaterialUbo);
		gl::BindBuffer(GL_UNIFORM_BUFFER, _deviceResources->modelMaterialUbo);
		gl::BufferData(GL_UNIFORM_BUFFER, (size_t)_deviceResources->modelMaterialBufferView.getSize(), nullptr, GL_DYNAMIC_DRAW);

		// if GL_EXT_buffer_storage is supported then map the buffer upfront and never unmap it
		if (_bufferStorageExtSupported)
		{
			gl::BindBuffer(GL_COPY_READ_BUFFER, _deviceResources->modelMaterialUbo);
			gl::ext::BufferStorageEXT(
				GL_COPY_READ_BUFFER, (GLsizei)_deviceResources->modelMaterialBufferView.getSize(), 0, GL_MAP_WRITE_BIT_EXT | GL_MAP_PERSISTENT_BIT_EXT | GL_MAP_COHERENT_BIT_EXT);

			void* memory = gl::MapBufferRange(GL_COPY_READ_BUFFER, 0, static_cast<GLsizeiptr>(_deviceResources->modelMaterialBufferView.getSize()),
				GL_MAP_WRITE_BIT_EXT | GL_MAP_PERSISTENT_BIT_EXT | GL_MAP_COHERENT_BIT_EXT);
			_deviceResources->modelMaterialBufferView.pointToMappedMemory(memory);
		}
	}
	{
		pvr::utils::StructuredMemoryDescription description;
		description.addElement(BufferEntryNames::PerModel::WorldViewProjectionMatrix, pvr::GpuDatatypes::mat4x4);
		description.addElement(BufferEntryNames::PerModel::WorldViewMatrix, pvr::GpuDatatypes::mat4x4);
		description.addElement(BufferEntryNames::PerModel::WorldViewITMatrix, pvr::GpuDatatypes::mat4x4);
		_deviceResources->modelMatrixBufferView.initDynamic(description, _mainScene->getNumMeshNodes(), pvr::BufferUsageFlags::UniformBuffer, static_cast<uint64_t>(_uniformAlignment));

		gl::GenBuffers(1, &_deviceResources->modelMatrixUbo);
		gl::BindBuffer(GL_UNIFORM_BUFFER, _deviceResources->modelMatrixUbo);
		gl::BufferData(GL_UNIFORM_BUFFER, static_cast<GLsizeiptr>(_deviceResources->modelMatrixBufferView.getSize()), nullptr, GL_DYNAMIC_DRAW);

		if (_bufferStorageExtSupported)
		{
			gl::BindBuffer(GL_COPY_READ_BUFFER, _deviceResources->modelMatrixUbo);
			gl::ext::BufferStorageEXT(GL_COPY_READ_BUFFER, static_cast<GLsizei>(_deviceResources->modelMatrixBufferView.getSize()), 0,
				GL_MAP_WRITE_BIT_EXT | GL_MAP_PERSISTENT_BIT_EXT | GL_MAP_COHERENT_BIT_EXT);

			void* memory = gl::MapBufferRange(GL_COPY_READ_BUFFER, 0, static_cast<GLsizeiptr>(_deviceResources->modelMatrixBufferView.getSize()),
				GL_MAP_WRITE_BIT_EXT | GL_MAP_PERSISTENT_BIT_EXT | GL_MAP_COHERENT_BIT_EXT);
			_deviceResources->modelMatrixBufferView.pointToMappedMemory(memory);
		}
	}
}

/// <summary>Upload the static data to the buffers which do not change per frame.</summary>
void OpenGLESDeferredShading::uploadStaticModelData()
{
	// static model buffer
	if (!_bufferStorageExtSupported)
	{
		gl::BindBuffer(GL_UNIFORM_BUFFER, _deviceResources->modelMaterialUbo);
		void* memory = gl::MapBufferRange(GL_UNIFORM_BUFFER, 0, (size_t)_deviceResources->modelMaterialBufferView.getSize(), GL_MAP_WRITE_BIT);

		_deviceResources->modelMaterialBufferView.pointToMappedMemory(memory);
	}
	for (uint32_t i = 0; i < _mainScene->getNumMeshNodes(); ++i)
	{
		_deviceResources->modelMaterialBufferView.getElementByName(BufferEntryNames::PerModelMaterial::SpecularStrength, 0, i).setValue(_deviceResources->materials[i].specularStrength);

		_deviceResources->modelMaterialBufferView.getElementByName(BufferEntryNames::PerModelMaterial::DiffuseColor, 0, i).setValue(_deviceResources->materials[i].diffuseColor);
	}
	if (!_bufferStorageExtSupported) { gl::UnmapBuffer(GL_UNIFORM_BUFFER); }

	_farClipDistance = _mainScene->getCamera(0).getFar();

	gl::ProgramUniform1f(_deviceResources->renderInfo.renderGBuffer.objects[static_cast<uint32_t>(MeshNodes::Satyr)].program,
		_deviceResources->renderInfo.renderGBuffer.objects[static_cast<uint32_t>(MeshNodes::Satyr)].farClipDistanceLocation, _farClipDistance);

	gl::ProgramUniform1f(_deviceResources->renderInfo.renderGBuffer.objects[static_cast<uint32_t>(MeshNodes::Floor)].program,
		_deviceResources->renderInfo.renderGBuffer.objects[static_cast<uint32_t>(MeshNodes::Floor)].farClipDistanceLocation, _farClipDistance);

	gl::ProgramUniform1f(_deviceResources->renderInfo.pointLightProxyPass.program, _deviceResources->renderInfo.pointLightProxyPass.farClipDistanceLocation, _farClipDistance);
}

void OpenGLESDeferredShading::uploadStaticDirectionalLightData()
{
	// static directional light buffer
	if (!_bufferStorageExtSupported)
	{
		gl::BindBuffer(GL_UNIFORM_BUFFER, _deviceResources->directionalLightStaticDataUbo);
		void* memory = gl::MapBufferRange(GL_UNIFORM_BUFFER, 0, (size_t)_deviceResources->staticDirectionalLightBufferView.getSize(), GL_MAP_WRITE_BIT);
		_deviceResources->staticDirectionalLightBufferView.pointToMappedMemory(memory);
	}

	for (uint32_t i = 0; i < _numberOfDirectionalLights; ++i)
	{
		_deviceResources->staticDirectionalLightBufferView.getElementByName(BufferEntryNames::StaticDirectionalLight::LightIntensity, 0, i)
			.setValue(_deviceResources->renderInfo.directionalLightPass.lightProperties[i].lightIntensity);

		_deviceResources->staticDirectionalLightBufferView.getElementByName(BufferEntryNames::StaticDirectionalLight::AmbientLight, 0, i).setValue(DirectionalLightConfiguration::AmbientLightColor);
	}
	if (!_bufferStorageExtSupported) { gl::UnmapBuffer(GL_UNIFORM_BUFFER); }
}

void OpenGLESDeferredShading::uploadStaticPointLightData()
{
	// static directional light buffer
	if (!_bufferStorageExtSupported)
	{
		gl::BindBuffer(GL_UNIFORM_BUFFER, _deviceResources->pointLightPropertiesUbo);
		void* memory = gl::MapBufferRange(GL_UNIFORM_BUFFER, 0, static_cast<GLsizeiptr>(_deviceResources->staticPointLightBufferView.getSize()), GL_MAP_WRITE_BIT);
		_deviceResources->staticPointLightBufferView.pointToMappedMemory(memory);
	}

	for (uint32_t i = 0; i < _numberOfPointLights; ++i)
	{
		_deviceResources->staticPointLightBufferView.getElementByName(BufferEntryNames::StaticPointLight::LightIntensity, 0, i)
			.setValue(_deviceResources->renderInfo.pointLightPasses.lightProperties[i].lightIntensity);

		_deviceResources->staticPointLightBufferView.getElementByName(BufferEntryNames::StaticPointLight::LightRadius, 0, i)
			.setValue(_deviceResources->renderInfo.pointLightPasses.lightProperties[i].lightRadius);

		_deviceResources->staticPointLightBufferView.getElementByName(BufferEntryNames::StaticPointLight::LightColor, 0, i)
			.setValue(_deviceResources->renderInfo.pointLightPasses.lightProperties[i].lightColor);

		_deviceResources->staticPointLightBufferView.getElementByName(BufferEntryNames::StaticPointLight::LightSourceColor, 0, i)
			.setValue(_deviceResources->renderInfo.pointLightPasses.lightProperties[i].lightSourceColor);
	}
	if (!_bufferStorageExtSupported) { gl::UnmapBuffer(GL_UNIFORM_BUFFER); }
}

/// <summary>Upload the static data to the buffers which do not change per frame.</summary>
void OpenGLESDeferredShading::uploadStaticData()
{
	uploadStaticModelData();
	uploadStaticDirectionalLightData();
	uploadStaticPointLightData();

	gl::UseProgram(0);
}

/// <summary>Creates the buffers used for rendering the point lighting.</summary>
void OpenGLESDeferredShading::createPointLightBuffers()
{
	{
		pvr::utils::StructuredMemoryDescription description;
		description.addElement(BufferEntryNames::StaticPointLight::LightIntensity, pvr::GpuDatatypes::Float);
		description.addElement(BufferEntryNames::StaticPointLight::LightRadius, pvr::GpuDatatypes::Float);
		description.addElement(BufferEntryNames::StaticPointLight::LightColor, pvr::GpuDatatypes::vec4);
		description.addElement(BufferEntryNames::StaticPointLight::LightSourceColor, pvr::GpuDatatypes::vec4);
		_deviceResources->staticPointLightBufferView.initDynamic(description, _numberOfPointLights, pvr::BufferUsageFlags::UniformBuffer, _uniformAlignment);

		gl::GenBuffers(1, &_deviceResources->pointLightPropertiesUbo);
		gl::BindBuffer(GL_UNIFORM_BUFFER, _deviceResources->pointLightPropertiesUbo);
		gl::BufferData(GL_UNIFORM_BUFFER, (size_t)_deviceResources->staticPointLightBufferView.getSize(), nullptr, GL_DYNAMIC_DRAW);

		// if GL_EXT_buffer_storage is supported then map the buffer upfront and never unmap it
		if (_bufferStorageExtSupported)
		{
			gl::BindBuffer(GL_COPY_READ_BUFFER, _deviceResources->pointLightPropertiesUbo);
			gl::ext::BufferStorageEXT(GL_COPY_READ_BUFFER, static_cast<GLsizei>(_deviceResources->staticPointLightBufferView.getSize()), 0,
				GL_MAP_WRITE_BIT_EXT | GL_MAP_PERSISTENT_BIT_EXT | GL_MAP_COHERENT_BIT_EXT);

			void* memory = gl::MapBufferRange(
				GL_COPY_READ_BUFFER, 0, (size_t)_deviceResources->staticPointLightBufferView.getSize(), GL_MAP_WRITE_BIT_EXT | GL_MAP_PERSISTENT_BIT_EXT | GL_MAP_COHERENT_BIT_EXT);
			_deviceResources->staticPointLightBufferView.pointToMappedMemory(memory);
		}
	}

	{
		pvr::utils::StructuredMemoryDescription description;
		description.addElement(BufferEntryNames::DynamicPointLight::WorldViewProjectionMatrix, pvr::GpuDatatypes::mat4x4);
		description.addElement(BufferEntryNames::DynamicPointLight::ViewPosition, pvr::GpuDatatypes::vec4);
		description.addElement(BufferEntryNames::DynamicPointLight::ProxyWorldViewProjectionMatrix, pvr::GpuDatatypes::mat4x4);
		description.addElement(BufferEntryNames::DynamicPointLight::ProxyWorldViewMatrix, pvr::GpuDatatypes::mat4x4);
		_deviceResources->dynamicPointLightBufferView.initDynamic(description, _numberOfPointLights, pvr::BufferUsageFlags::UniformBuffer, _uniformAlignment);

		gl::GenBuffers(1, &_deviceResources->pointLightMatrixUbo);
		gl::BindBuffer(GL_UNIFORM_BUFFER, _deviceResources->pointLightMatrixUbo);
		gl::BufferData(GL_UNIFORM_BUFFER, (size_t)_deviceResources->dynamicPointLightBufferView.getSize(), nullptr, GL_DYNAMIC_DRAW);

		// if GL_EXT_buffer_storage is supported then map the buffer upfront and never unmap it
		if (_bufferStorageExtSupported)
		{
			gl::BindBuffer(GL_COPY_READ_BUFFER, _deviceResources->pointLightMatrixUbo);
			gl::ext::BufferStorageEXT(GL_COPY_READ_BUFFER, static_cast<GLsizei>(_deviceResources->dynamicPointLightBufferView.getSize()), 0,
				GL_MAP_WRITE_BIT_EXT | GL_MAP_PERSISTENT_BIT_EXT | GL_MAP_COHERENT_BIT_EXT);

			void* memory = gl::MapBufferRange(GL_COPY_READ_BUFFER, 0, (size_t)_deviceResources->dynamicPointLightBufferView.getSize(),
				GL_MAP_WRITE_BIT_EXT | GL_MAP_PERSISTENT_BIT_EXT | GL_MAP_COHERENT_BIT_EXT);
			_deviceResources->dynamicPointLightBufferView.pointToMappedMemory(memory);
		}
	}
}

void OpenGLESDeferredShading::createDirectionalLightBuffers()
{
	{
		pvr::utils::StructuredMemoryDescription description;
		description.addElement(BufferEntryNames::StaticDirectionalLight::LightIntensity, pvr::GpuDatatypes::vec4);
		description.addElement(BufferEntryNames::StaticDirectionalLight::AmbientLight, pvr::GpuDatatypes::vec4);
		_deviceResources->staticDirectionalLightBufferView.initDynamic(description, _numberOfDirectionalLights, pvr::BufferUsageFlags::UniformBuffer, _uniformAlignment);

		gl::GenBuffers(1, &_deviceResources->directionalLightStaticDataUbo);
		gl::BindBuffer(GL_UNIFORM_BUFFER, _deviceResources->directionalLightStaticDataUbo);
		gl::BufferData(GL_UNIFORM_BUFFER, (size_t)_deviceResources->staticDirectionalLightBufferView.getSize(), nullptr, GL_DYNAMIC_DRAW);

		// if GL_EXT_buffer_storage is supported then map the buffer upfront and never unmap it
		if (_bufferStorageExtSupported)
		{
			gl::BindBuffer(GL_COPY_READ_BUFFER, _deviceResources->directionalLightStaticDataUbo);
			gl::ext::BufferStorageEXT(GL_COPY_READ_BUFFER, static_cast<GLsizei>(_deviceResources->staticDirectionalLightBufferView.getSize()), 0,
				GL_MAP_WRITE_BIT_EXT | GL_MAP_PERSISTENT_BIT_EXT | GL_MAP_COHERENT_BIT_EXT);

			void* memory = gl::MapBufferRange(GL_COPY_READ_BUFFER, 0, (size_t)_deviceResources->staticDirectionalLightBufferView.getSize(),
				GL_MAP_WRITE_BIT_EXT | GL_MAP_PERSISTENT_BIT_EXT | GL_MAP_COHERENT_BIT_EXT);
			_deviceResources->staticDirectionalLightBufferView.pointToMappedMemory(memory);
		}
	}

	{
		pvr::utils::StructuredMemoryDescription description;
		description.addElement(BufferEntryNames::DynamicDirectionalLight::ViewSpaceLightDirection, pvr::GpuDatatypes::vec4);
		_deviceResources->dynamicDirectionalLightBufferView.initDynamic(description, _numberOfDirectionalLights, pvr::BufferUsageFlags::UniformBuffer, _uniformAlignment);

		gl::GenBuffers(1, &_deviceResources->directionalLightDynamicDataUbo);
		gl::BindBuffer(GL_UNIFORM_BUFFER, _deviceResources->directionalLightDynamicDataUbo);
		gl::BufferData(GL_UNIFORM_BUFFER, (size_t)_deviceResources->dynamicDirectionalLightBufferView.getSize(), nullptr, GL_DYNAMIC_DRAW);

		// if GL_EXT_buffer_storage is supported then map the buffer upfront and never unmap it
		if (_bufferStorageExtSupported)
		{
			gl::BindBuffer(GL_COPY_READ_BUFFER, _deviceResources->directionalLightDynamicDataUbo);
			gl::ext::BufferStorageEXT(GL_COPY_READ_BUFFER, static_cast<GLsizei>(_deviceResources->dynamicDirectionalLightBufferView.getSize()), 0,
				GL_MAP_WRITE_BIT_EXT | GL_MAP_PERSISTENT_BIT_EXT | GL_MAP_COHERENT_BIT_EXT);

			void* memory = gl::MapBufferRange(GL_COPY_READ_BUFFER, 0, (size_t)_deviceResources->dynamicDirectionalLightBufferView.getSize(),
				GL_MAP_WRITE_BIT_EXT | GL_MAP_PERSISTENT_BIT_EXT | GL_MAP_COHERENT_BIT_EXT);
			_deviceResources->dynamicDirectionalLightBufferView.pointToMappedMemory(memory);
		}
	}
}

void OpenGLESDeferredShading::bindVertexSpecification(const pvr::assets::Mesh& mesh, const pvr::utils::VertexBindings_Name* const vertexBindingsName,
	const uint32_t numVertexBindings, pvr::utils::VertexConfiguration& vertexConfiguration, GLuint& vao, GLuint& vbo, GLuint& ibo)
{
	vertexConfiguration = pvr::utils::createInputAssemblyFromMesh(mesh, vertexBindingsName, (uint16_t)numVertexBindings);

	gl::GenVertexArrays(1, &vao);
	gl::BindVertexArray(vao);
	gl::BindVertexBuffer(0, vbo, 0, mesh.getStride(0));
	gl::BindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);

	for (auto it = vertexConfiguration.attributes.begin(), end = vertexConfiguration.attributes.end(); it != end; ++it)
	{
		gl::EnableVertexAttribArray(it->index);
		gl::VertexAttribBinding(it->index, 0);
		gl::VertexAttribFormat(it->index, it->width, pvr::utils::convertToGles(it->format), pvr::dataTypeIsNormalised(it->format), static_cast<intptr_t>(it->offsetInBytes));
	}

	gl::BindVertexArray(0);
}

void OpenGLESDeferredShading::createGeometryBuffers()
{
	// create the vbos and ibos for the objects in the main scene
	pvr::utils::appendSingleBuffersFromModel(*_mainScene, _deviceResources->sceneVbos, _deviceResources->sceneIbos);
	_deviceResources->sceneVaos.resize(_mainScene->getNumMeshNodes());

	bindVertexSpecification(_mainScene->getMesh(static_cast<uint32_t>(MeshNodes::Satyr)), vertexBindings, 4,
		_deviceResources->sceneVertexConfigurations[static_cast<uint32_t>(MeshNodes::Satyr)], _deviceResources->sceneVaos[static_cast<uint32_t>(MeshNodes::Satyr)],
		_deviceResources->sceneVbos[static_cast<uint32_t>(MeshNodes::Satyr)], _deviceResources->sceneIbos[static_cast<uint32_t>(MeshNodes::Satyr)]);

	bindVertexSpecification(_mainScene->getMesh(static_cast<uint32_t>(MeshNodes::Floor)), floorVertexBindings, 3,
		_deviceResources->sceneVertexConfigurations[static_cast<uint32_t>(MeshNodes::Floor)], _deviceResources->sceneVaos[static_cast<uint32_t>(MeshNodes::Floor)],
		_deviceResources->sceneVbos[static_cast<uint32_t>(MeshNodes::Floor)], _deviceResources->sceneIbos[static_cast<uint32_t>(MeshNodes::Floor)]);

	// create the vbos and ibos for the point light sources
	pvr::utils::createSingleBuffersFromModel(*_pointLightScene, &_deviceResources->pointLightVbo, &_deviceResources->pointLightIbo);

	bindVertexSpecification(_pointLightScene->getMesh(static_cast<uint32_t>(LightNodes::PointLightMeshNode)), pointLightVertexBindings, 1,
		_deviceResources->pointLightVertexConfiguration, _deviceResources->pointLightVao, _deviceResources->pointLightVbo, _deviceResources->pointLightIbo);
}

/// <summary>Allocate memory for Uniforms.</summary>
void OpenGLESDeferredShading::allocateLights()
{
	uint32_t countPoint = 0;
	uint32_t countDirectional = 0;
	for (uint32_t i = 0; i < _mainScene->getNumLightNodes(); ++i)
	{
		switch (_mainScene->getLight(_mainScene->getLightNode(i).getObjectId()).getType())
		{
		case pvr::assets::Light::Directional: ++countDirectional; break;
		case pvr::assets::Light::Point: ++countPoint; break;
		default: break;
		}
	}

	if (DirectionalLightConfiguration::AdditionalDirectionalLight) { ++countDirectional; }

	if (countPoint >= PointLightConfiguration::MaxScenePointLights) { countPoint = PointLightConfiguration::MaxScenePointLights; }

	countPoint += static_cast<uint32_t>(PointLightConfiguration::NumProceduralPointLights);

	_numberOfPointLights = countPoint;

	_numberOfDirectionalLights = countDirectional;

	_deviceResources->renderInfo.directionalLightPass.lightProperties.resize(countDirectional);
	_deviceResources->renderInfo.pointLightPasses.lightProperties.resize(countPoint);
	_deviceResources->renderInfo.pointLightPasses.initialData.resize(countPoint);

	for (uint32_t i = countPoint - static_cast<uint32_t>(PointLightConfiguration::NumProceduralPointLights); i < countPoint; ++i)
	{ updateProceduralPointLight(_deviceResources->renderInfo.pointLightPasses.initialData[i], _deviceResources->renderInfo.pointLightPasses.lightProperties[i], true); }
}

/// <summary>Initialise the static light properties.</summary>
void OpenGLESDeferredShading::initialiseStaticLightProperties()
{
	RenderData& pass = _deviceResources->renderInfo;

	uint32_t pointLight = 0;
	uint32_t directionalLight = 0;
	for (uint32_t i = 0; i < _mainScene->getNumLightNodes(); ++i)
	{
		const pvr::assets::Node& lightNode = _mainScene->getLightNode(i);
		const pvr::assets::Light& light = _mainScene->getLight(lightNode.getObjectId());
		switch (light.getType())
		{
		case pvr::assets::Light::Point: {
			if (pointLight >= PointLightConfiguration::MaxScenePointLights) { continue; }

			// POINT LIGHT GEOMETRY : The spheres that will be used for the stencil pass
			pass.pointLightPasses.lightProperties[pointLight].lightColor = glm::vec4(light.getColor(), 1.f);

			// POINT LIGHT PROXIES : The "draw calls" that will perform the actual rendering
			pass.pointLightPasses.lightProperties[pointLight].lightIntensity = PointLightConfiguration::PointlightIntensity;

			// POINT LIGHT PROXIES : The "draw calls" that will perform the actual rendering
			pass.pointLightPasses.lightProperties[pointLight].lightRadius = PointLightConfiguration::PointLightMaxRadius;

			// POINT LIGHT SOURCES : The little balls that we render to show the lights
			pass.pointLightPasses.lightProperties[pointLight].lightSourceColor = glm::vec4(light.getColor(), .8f);
			++pointLight;
		}
		break;
		case pvr::assets::Light::Directional: {
			pass.directionalLightPass.lightProperties[directionalLight].lightIntensity = glm::vec4(light.getColor(), 1.0f) * DirectionalLightConfiguration::DirectionalLightIntensity;
			++directionalLight;
		}
		break;
		default: break;
		}
	}
	if (DirectionalLightConfiguration::AdditionalDirectionalLight)
	{
		pass.directionalLightPass.lightProperties[directionalLight].lightIntensity = glm::vec4(1, 1, 1, 1) * DirectionalLightConfiguration::DirectionalLightIntensity;
		++directionalLight;
	}
}

/// <summary>Update the procedural point lights.</summary>
void OpenGLESDeferredShading::updateProceduralPointLight(PointLightPasses::InitialData& data, PointLightPasses::PointLightProperties& pointLightProperties, bool initial)
{
	if (initial)
	{
		data.distance = pvr::randomrange(PointLightConfiguration::LightMinDistance, PointLightConfiguration::LightMaxDistance);
		data.angle = pvr::randomrange(-glm::pi<float>(), glm::pi<float>());
		data.height = pvr::randomrange(PointLightConfiguration::LightMinHeight, PointLightConfiguration::LightMaxHeight);
		data.axial_vel = pvr::randomrange(-PointLightConfiguration::LightMaxAxialVelocity, PointLightConfiguration::LightMaxAxialVelocity);
		data.radial_vel = pvr::randomrange(-PointLightConfiguration::LightMaxRadialVelocity, PointLightConfiguration::LightMaxRadialVelocity);
		data.vertical_vel = pvr::randomrange(-PointLightConfiguration::LightMaxVerticalVelocity, PointLightConfiguration::LightMaxVerticalVelocity);

		glm::vec3 lightColor = glm::vec3(pvr::randomrange(0, 1), pvr::randomrange(0, 1), pvr::randomrange(0, 1));
		lightColor / glm::max(glm::max(lightColor.x, lightColor.y), lightColor.z); // Have at least one component equal to 1... We want them bright-ish...
		pointLightProperties.lightColor = glm::vec4(lightColor, 1.); // random-looking
		pointLightProperties.lightSourceColor = glm::vec4(lightColor, 0.8); // random-looking
		pointLightProperties.lightIntensity = PointLightConfiguration::PointlightIntensity;
		pointLightProperties.lightRadius = PointLightConfiguration::PointLightMaxRadius;
	}

	if (!initial && !_isPaused) // Skip for the first frameNumber, as sometimes this moves the light too far...
	{
		uint64_t maxFrameTime = 30;
		float dt = static_cast<float>(std::min(getFrameTime(), maxFrameTime));
		if (data.distance < PointLightConfiguration::LightMinDistance)
		{ data.axial_vel = glm::abs(data.axial_vel) + (PointLightConfiguration::LightMaxAxialVelocity * dt * .001f); }
		if (data.distance > PointLightConfiguration::LightMaxDistance)
		{ data.axial_vel = -glm::abs(data.axial_vel) - (PointLightConfiguration::LightMaxAxialVelocity * dt * .001f); }
		if (data.height < PointLightConfiguration::LightMinHeight)
		{ data.vertical_vel = glm::abs(data.vertical_vel) + (PointLightConfiguration::LightMaxAxialVelocity * dt * .001f); }
		if (data.height > PointLightConfiguration::LightMaxHeight)
		{ data.vertical_vel = -glm::abs(data.vertical_vel) - (PointLightConfiguration::LightMaxAxialVelocity * dt * .001f); }

		data.axial_vel += pvr::randomrange(-PointLightConfiguration::LightAxialVelocityChange, PointLightConfiguration::LightAxialVelocityChange) * dt;

		data.radial_vel += pvr::randomrange(-PointLightConfiguration::LightRadialVelocityChange, PointLightConfiguration::LightRadialVelocityChange) * dt;

		data.vertical_vel += pvr::randomrange(-PointLightConfiguration::LightVerticalVelocityChange, PointLightConfiguration::LightVerticalVelocityChange) * dt;

		if (glm::abs(data.axial_vel) > PointLightConfiguration::LightMaxAxialVelocity) { data.axial_vel *= .8f; }
		if (glm::abs(data.radial_vel) > PointLightConfiguration::LightMaxRadialVelocity) { data.radial_vel *= .8f; }
		if (glm::abs(data.vertical_vel) > PointLightConfiguration::LightMaxVerticalVelocity) { data.vertical_vel *= .8f; }

		data.distance += data.axial_vel * dt * 0.001f;
		data.angle += data.radial_vel * dt * 0.001f;
		data.height += data.vertical_vel * dt * 0.001f;
	}

	float x = sin(data.angle) * data.distance;
	float z = cos(data.angle) * data.distance;
	float y = data.height;

	const glm::mat4& transMtx = glm::translate(glm::vec3(x, y, z));
	const glm::mat4& proxyScale = glm::scale(glm::vec3(PointLightConfiguration::PointLightMaxRadius));

	const glm::mat4 mWorldScale = transMtx * proxyScale;

	// POINT LIGHT GEOMETRY : The spheres that will be used for the stencil pass
	pointLightProperties.proxyWorldViewProjectionMatrix = _viewProjectionMatrix * mWorldScale;

	// POINT LIGHT PROXIES : The "draw calls" that will perform the actual rendering
	pointLightProperties.proxyWorldViewMatrix = _viewMatrix * mWorldScale;
	pointLightProperties.proxyViewSpaceLightPosition = glm::vec4((_viewMatrix * transMtx)[3]); // Translation component of the view matrix

	// POINT LIGHT SOURCES : The little balls that we render to show the lights
	pointLightProperties.worldViewProjectionMatrix = _viewProjectionMatrix * transMtx;
}

/// <summary>Update the CPU visible buffers containing dynamic data.</summary>
void OpenGLESDeferredShading::updateDynamicSceneData()
{
	RenderData& pass = _deviceResources->renderInfo;

	{
		// dynamic model buffer
		if (!_bufferStorageExtSupported)
		{
			gl::BindBuffer(GL_UNIFORM_BUFFER, _deviceResources->modelMatrixUbo);
			void* memory = gl::MapBufferRange(GL_UNIFORM_BUFFER, 0, static_cast<GLsizeiptr>(_deviceResources->modelMatrixBufferView.getSize()), GL_MAP_WRITE_BIT);
			_deviceResources->modelMatrixBufferView.pointToMappedMemory(memory);
		}

		for (uint32_t i = 0; i < _mainScene->getNumMeshNodes(); ++i)
		{
			const pvr::assets::Model::Node& node = _mainScene->getNode(i);
			pass.renderGBuffer.objects[i].world = _mainScene->getWorldMatrix(node.getObjectId());
			pass.renderGBuffer.objects[i].worldView = _viewMatrix * pass.renderGBuffer.objects[i].world;
			pass.renderGBuffer.objects[i].worldViewProj = _viewProjectionMatrix * pass.renderGBuffer.objects[i].world;
			pass.renderGBuffer.objects[i].worldViewIT4x4 = glm::inverseTranspose(pass.renderGBuffer.objects[i].worldView);

			_deviceResources->modelMatrixBufferView.getElementByName(BufferEntryNames::PerModel::WorldViewMatrix, 0, i).setValue(pass.renderGBuffer.objects[i].worldView);
			_deviceResources->modelMatrixBufferView.getElementByName(BufferEntryNames::PerModel::WorldViewProjectionMatrix, 0, i).setValue(pass.renderGBuffer.objects[i].worldViewProj);
			_deviceResources->modelMatrixBufferView.getElementByName(BufferEntryNames::PerModel::WorldViewITMatrix, 0, i).setValue(pass.renderGBuffer.objects[i].worldViewIT4x4);
		}
		if (!_bufferStorageExtSupported) { gl::UnmapBuffer(GL_UNIFORM_BUFFER); }
	}

	uint32_t pointLight = 0;
	uint32_t directionalLight = 0;

	// update the lighting data
	for (uint32_t i = 0; i < _mainScene->getNumLightNodes(); ++i)
	{
		const pvr::assets::Node& lightNode = _mainScene->getLightNode(i);
		const pvr::assets::Light& light = _mainScene->getLight(lightNode.getObjectId());
		switch (light.getType())
		{
		case pvr::assets::Light::Point: {
			if (pointLight >= PointLightConfiguration::MaxScenePointLights) { continue; }

			const glm::mat4& transMtx = _mainScene->getWorldMatrix(_mainScene->getNodeIdFromLightNodeId(i));
			const glm::mat4& proxyScale = glm::scale(glm::vec3(PointLightConfiguration::PointLightMaxRadius));
			const glm::mat4 mWorldScale = transMtx * proxyScale;

			// POINT LIGHT GEOMETRY : The spheres that will be used for the stencil pass
			pass.pointLightPasses.lightProperties[pointLight].proxyWorldViewProjectionMatrix = _viewProjectionMatrix * mWorldScale;

			// POINT LIGHT PROXIES : The "draw calls" that will perform the actual rendering
			pass.pointLightPasses.lightProperties[pointLight].proxyWorldViewMatrix = _viewMatrix * mWorldScale;
			pass.pointLightPasses.lightProperties[pointLight].proxyViewSpaceLightPosition = glm::vec4((_viewMatrix * transMtx)[3]); // Translation component of the view matrix

			// POINT LIGHT SOURCES : The little balls that we render to show the lights
			pass.pointLightPasses.lightProperties[pointLight].worldViewProjectionMatrix = _viewProjectionMatrix * transMtx;
			++pointLight;
		}
		break;
		case pvr::assets::Light::Directional: {
			const glm::mat4& transMtx = _mainScene->getWorldMatrix(_mainScene->getNodeIdFromLightNodeId(i));
			pass.directionalLightPass.lightProperties[directionalLight].viewSpaceLightDirection = _viewMatrix * transMtx * glm::vec4(0.f, -1.f, 0.f, 0.f);
			++directionalLight;
		}
		break;
		default: break;
		}
	}
	int numSceneLights = pointLight;
	if (DirectionalLightConfiguration::AdditionalDirectionalLight)
	{
		pass.directionalLightPass.lightProperties[directionalLight].viewSpaceLightDirection = _viewMatrix * glm::normalize(glm::vec4(1.f, -1.f, -.5f, 0.f));
		++directionalLight;
	}

	for (; pointLight < numSceneLights + static_cast<uint32_t>(PointLightConfiguration::NumProceduralPointLights); ++pointLight)
	{ updateProceduralPointLight(pass.pointLightPasses.initialData[pointLight], _deviceResources->renderInfo.pointLightPasses.lightProperties[pointLight], false); }
	{
		// dynamic directional light buffer
		if (!_bufferStorageExtSupported)
		{
			gl::BindBuffer(GL_UNIFORM_BUFFER, _deviceResources->directionalLightDynamicDataUbo);
			void* memory = gl::MapBufferRange(GL_UNIFORM_BUFFER, 0, (size_t)_deviceResources->dynamicDirectionalLightBufferView.getSize(), GL_MAP_WRITE_BIT);

			_deviceResources->dynamicDirectionalLightBufferView.pointToMappedMemory(memory);
		}
		for (uint32_t i = 0; i < _numberOfDirectionalLights; ++i)
		{
			_deviceResources->dynamicDirectionalLightBufferView.getElementByName(BufferEntryNames::DynamicDirectionalLight::ViewSpaceLightDirection, 0, i)
				.setValue(_deviceResources->renderInfo.directionalLightPass.lightProperties[i].viewSpaceLightDirection);
		}
		if (!_bufferStorageExtSupported) { gl::UnmapBuffer(GL_UNIFORM_BUFFER); }
	}

	// dynamic point light buffer
	if (!_bufferStorageExtSupported)
	{
		gl::BindBuffer(GL_UNIFORM_BUFFER, _deviceResources->pointLightMatrixUbo);
		void* memory = gl::MapBufferRange(GL_UNIFORM_BUFFER, 0, (size_t)_deviceResources->dynamicPointLightBufferView.getSize(), GL_MAP_WRITE_BIT);
		_deviceResources->dynamicPointLightBufferView.pointToMappedMemory(memory);
	}

	for (uint32_t i = 0; i < _numberOfPointLights; ++i)
	{
		_deviceResources->dynamicPointLightBufferView.getElementByName(BufferEntryNames::DynamicPointLight::WorldViewProjectionMatrix, 0, i)
			.setValue(_deviceResources->renderInfo.pointLightPasses.lightProperties[i].worldViewProjectionMatrix);
		_deviceResources->dynamicPointLightBufferView.getElementByName(BufferEntryNames::DynamicPointLight::ViewPosition, 0, i)
			.setValue(_deviceResources->renderInfo.pointLightPasses.lightProperties[i].proxyViewSpaceLightPosition);
		_deviceResources->dynamicPointLightBufferView.getElementByName(BufferEntryNames::DynamicPointLight::ProxyWorldViewProjectionMatrix, 0, i)
			.setValue(_deviceResources->renderInfo.pointLightPasses.lightProperties[i].proxyWorldViewProjectionMatrix);
		_deviceResources->dynamicPointLightBufferView.getElementByName(BufferEntryNames::DynamicPointLight::ProxyWorldViewMatrix, 0, i)
			.setValue(_deviceResources->renderInfo.pointLightPasses.lightProperties[i].proxyWorldViewMatrix);
	}
	if (!_bufferStorageExtSupported) { gl::UnmapBuffer(GL_UNIFORM_BUFFER); }
}

/// <summary>This function must be implemented by the user of the shell. The user should return its pvr::Shell object defining the behaviour of the application. </summary>
/// <returns> Return a unique ptr to the demo supplied by the user. </returns>
std::unique_ptr<pvr::Shell> pvr::newDemo() { return std::make_unique<OpenGLESDeferredShading>(); }
