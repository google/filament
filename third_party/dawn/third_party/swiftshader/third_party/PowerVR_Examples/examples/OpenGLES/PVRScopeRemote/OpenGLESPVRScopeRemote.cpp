/*!
\brief Shows how to use our example PVRScope graph code.
\file OpenGLESPVRScopeRemote.cpp
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
#include "PVRShell/PVRShell.h"
#include "PVRUtils/PVRUtilsGles.h"
#include "PVRScopeComms.h"
#include "PVRAssets/Helper.h"

// Source shaders
const char FragShaderEs2SrcFile[] = "FragShader_ES2.fsh";
const char FragShaderEs3SrcFile[] = "FragShader_ES3.fsh";
const char VertShaderEs2SrcFile[] = "VertShader_ES2.vsh";
const char VertShaderEs3SrcFile[] = "VertShader_ES3.vsh";

// PVR texture files
const char TextureFile[] = "Marble.pvr";

// POD _scene files
const char SceneFile[] = "Satyr.pod";
namespace CounterDefs {
enum Enum
{
	Counter,
	Counter10,
	NumCounter
};
}
const char* FrameDefs[CounterDefs::NumCounter] = { "Frames", "Frames10" };

/// <summary>Class implementing the PVRShell functions.</summary>
class OpenGLESPVRScopeRemote : public pvr::Shell
{
	glm::vec3 ClearColor;
	struct DeviceResources
	{
		GLuint program;
		GLuint texture;
		std::vector<GLuint> vbos;
		std::vector<GLuint> ibos;
		GLuint onScreenFbo;

		GLuint shaders[2];

		pvr::EglContext context;

		// UIRenderer used to display text
		pvr::ui::UIRenderer uiRenderer;

		DeviceResources() : program(0), texture(0), vbos(0), ibos(0) {}

		~DeviceResources()
		{
			gl::DeleteProgram(program);
			gl::DeleteTextures(1, &texture);
			gl::DeleteBuffers((GLsizei)vbos.size(), vbos.data());
			gl::DeleteBuffers((GLsizei)ibos.size(), ibos.data());
		}
	};

	std::unique_ptr<DeviceResources> _deviceResources;
	// 3D Model
	pvr::assets::ModelHandle _scene;
	// Projection and view matrices

	// Group shader programs and their uniform locations together
	struct
	{
		int32_t mvpMtx;
		int32_t mvITMtx;
		int32_t lightDirView;
		int32_t albedo;
		int32_t specularExponent;
		int32_t metallicity;
		int32_t reflectivity;
	} _uniformLocations;

	struct Uniforms
	{
		glm::mat4 projectionMtx;
		glm::mat4 viewMtx;
		glm::mat4 mvpMatrix;
		glm::mat4 mvMatrix;
		glm::mat3 mvITMatrix;
		glm::vec3 lightDirView;
		float specularExponent;
		float metallicity;
		float reflectivity;
		glm::vec3 albedo;
	} _progUniforms;

	// The translation and Rotate parameter of Model
	float _angleY;

	// Data connection to PVRPerfServer
	bool _hasCommunicationError;
	SSPSCommsData* _spsCommsData;
	SSPSCommsLibraryTypeFloat _commsLibSpecularExponent;
	SSPSCommsLibraryTypeFloat _commsLibMetallicity;
	SSPSCommsLibraryTypeFloat _commsLibReflectivity;
	SSPSCommsLibraryTypeFloat _commsLibAlbedoR;
	SSPSCommsLibraryTypeFloat _commsLibAlbedoG;
	SSPSCommsLibraryTypeFloat _commsLibAlbedoB;

	std::vector<char> _vertShaderSrc;
	std::vector<char> _fragShaderSrc;
	uint32_t _frameCounter;
	uint32_t _frame10Counter;
	uint32_t _counterReadings[CounterDefs::NumCounter];
	pvr::utils::VertexConfiguration _vertexConfiguration;

	std::string _vertShaderSrcFile;
	std::string _fragShaderSrcFile;

public:
	virtual pvr::Result initApplication();
	virtual pvr::Result initView();
	virtual pvr::Result releaseView();
	virtual pvr::Result quitApplication();
	virtual pvr::Result renderFrame();
	void createSamplerTexture();
	void createProgram(const char* const pszFrag, const char* const pszVert, bool recompile = false);
	void loadVbos();
	void drawMesh(int i32NodeIndex);
};

/// <summary>Loads the textures required for this training course.</summary>
/// <returns>Return true if no error occurred.</returns>
void OpenGLESPVRScopeRemote::createSamplerTexture()
{
	CPPLProcessingScoped PPLProcessingScoped(_spsCommsData, __FUNCTION__, static_cast<uint32_t>(strlen(__FUNCTION__)), _frameCounter);
	auto texStream = getAssetStream(TextureFile);
	pvr::Texture tex = pvr::textureLoad(*texStream, pvr::TextureFileFormat::PVR);
	pvr::utils::TextureUploadResults uploadResults = pvr::utils::textureUpload(tex, _deviceResources->context->getApiVersion() == pvr::Api::OpenGLES2, true);
	_deviceResources->texture = uploadResults.image;
	gl::BindTexture(GL_TEXTURE_2D, _deviceResources->texture);
	gl::TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_LINEAR);
	gl::TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	gl::BindTexture(GL_TEXTURE_2D, 0);
	pvr::utils::throwOnGlError("[OpenGLESPVRScopeRemote::createSamplerTexture] - Failed to create texture and sampler");
}

/// <summary>Loads and compiles the shaders and links the shader programs required for this training course.</summary>
/// <returns>Return true if no error occurred.</returns>
void OpenGLESPVRScopeRemote::createProgram(const char* const fragShaderSource, const char* const vertShaderSource, bool recompile)
{
	// Enable or disable gamma correction based on if it is automatically performed on the framebuffer or we need to do it in the shader.
	const char* defines[] = { "FRAMEBUFFER_SRGB" };
	uint32_t numDefines = 1;
	glm::vec3 clearColorLinearSpace(0.0f, 0.40f, .39f);
	ClearColor = clearColorLinearSpace;
	if (getBackBufferColorspace() != pvr::ColorSpace::sRGB)
	{
		ClearColor = pvr::utils::convertLRGBtoSRGB(clearColorLinearSpace); // Gamma correct the clear colour...
		numDefines = 0;
	}

	gl::GetError();
	// Mapping of mesh semantic names to shader variables
	const char* vertexBindings[] = { "inVertex", "inNormal", "inTexCoord" };
	const uint16_t attribIndices[3] = { 0, 1, 2 };
	CPPLProcessingScoped PPLProcessingScoped(_spsCommsData, __FUNCTION__, static_cast<uint32_t>(strlen(__FUNCTION__)), _frameCounter);

	/* Load and compile the shaders from files. */
	pvr::BufferStream vertexShaderStream("", vertShaderSource, strlen(vertShaderSource));
	pvr::BufferStream fragShaderStream("", fragShaderSource, strlen(fragShaderSource));
	if (recompile)
	{
		if (_deviceResources->shaders[0])
		{
			gl::DetachShader(_deviceResources->program, _deviceResources->shaders[0]);
			gl::DeleteShader(_deviceResources->shaders[0]);
			_deviceResources->shaders[0] = 0;
		}
		if (_deviceResources->shaders[1])
		{
			gl::DetachShader(_deviceResources->program, _deviceResources->shaders[1]);
			gl::DeleteShader(_deviceResources->shaders[1]);
			_deviceResources->shaders[1] = 0;
		}
		gl::GetError(); // Don't really care if we succeeded or failed...
	}
	_deviceResources->shaders[0] = pvr::utils::loadShader(vertexShaderStream, pvr::ShaderType::VertexShader, defines, numDefines);
	_deviceResources->shaders[1] = pvr::utils::loadShader(fragShaderStream, pvr::ShaderType::FragmentShader, defines, numDefines);

	_deviceResources->program =
		pvr::utils::createShaderProgram(_deviceResources->shaders, ARRAY_SIZE(_deviceResources->shaders), vertexBindings, attribIndices, ARRAY_SIZE(attribIndices));

	// Set the sampler2D variable to the first texture unit
	gl::UseProgram(_deviceResources->program);
	gl::Uniform1i(gl::GetUniformLocation(_deviceResources->program, "sTexture"), 0);
	gl::UseProgram(0);

	// Store the location of uniforms for later use
	_uniformLocations.mvpMtx = gl::GetUniformLocation(_deviceResources->program, "mVPMatrix");
	_uniformLocations.mvITMtx = gl::GetUniformLocation(_deviceResources->program, "mVITMatrix");
	_uniformLocations.lightDirView = gl::GetUniformLocation(_deviceResources->program, "viewLightDirection");

	_uniformLocations.specularExponent = gl::GetUniformLocation(_deviceResources->program, "specularExponent");
	_uniformLocations.metallicity = gl::GetUniformLocation(_deviceResources->program, "metallicity");
	_uniformLocations.reflectivity = gl::GetUniformLocation(_deviceResources->program, "reflectivity");
	_uniformLocations.albedo = gl::GetUniformLocation(_deviceResources->program, "albedoModulation");
}

/// <summary>Loads the mesh data required for this training course into vertex buffer objects.</summary>
void OpenGLESPVRScopeRemote::loadVbos()
{
	CPPLProcessingScoped PPLProcessingScoped(_spsCommsData, __FUNCTION__, static_cast<uint32_t>(strlen(__FUNCTION__)), _frameCounter);

	//  Load vertex data of all meshes in the _scene into VBOs
	//  The meshes have been exported with the "Interleave Vectors" option,
	//  so all data is interleaved in the buffer at pMesh->pInterleaved.
	//  Interleaving data improves the memory access pattern and cache efficiency,
	//  thus it can be read faster by the hardware.
	pvr::utils::appendSingleBuffersFromModel(*_scene, _deviceResources->vbos, _deviceResources->ibos);
}

/// <summary>Code in initApplication() will be called by Shell once per run, before the rendering context is created.
/// Used to initialize variables that are not dependent on it (e.g. external modules, loading meshes, etc.)
/// If the rendering context is lost, initApplication() will not be called again.</summary>
/// <returns>Return pvr::Result::Success if no error occurred.</returns>
pvr::Result OpenGLESPVRScopeRemote::initApplication()
{
	// Load the scene
	_scene = pvr::assets::loadModel(*this, SceneFile);
	pvr::utils::VertexBindings_Name vertexBindings[] = { { "POSITION", "inVertex" }, { "NORMAL", "inNormal" }, { "UV0", "inTexCoord" } };
	_vertexConfiguration = createInputAssemblyFromMesh(_scene->getMesh(0), vertexBindings, 3);

	_progUniforms.specularExponent = 5.f; // Width of the specular highlights (using low exponent for a brushed metal look)
	_progUniforms.albedo = glm::vec3(1.0f, 0.563f, 0.087f); // Overall colour
	_progUniforms.metallicity = 1.f; // Is the colour of the specular white (non-metallic), or coloured by the object(metallic)
	_progUniforms.reflectivity = 0.9f; // Percentage of contribution of diffuse / specular
	_frameCounter = 0;
	_frame10Counter = 0;

	// set angle of rotation
	_angleY = 0.0f;

	// We want a data connection to PVRPerfServer
	{
		_spsCommsData = pplInitialise("PVRScopeRemote", 14);
		_hasCommunicationError = false;

		// Demonstrate that there is a good chance of the initial data being
		// lost - the connection is normally completed asynchronously.
		pplSendMark(_spsCommsData, "lost", static_cast<uint32_t>(strlen("lost")));

		// This is entirely optional. Wait for the connection to succeed, it will
		// timeout if e.g. PVRPerfServer is not running.
		int isConnected;
		pplWaitForConnection(_spsCommsData, &isConnected, 1, 200);
	}
	CPPLProcessingScoped PPLProcessingScoped(_spsCommsData, __FUNCTION__, static_cast<uint32_t>(strlen(__FUNCTION__)), _frameCounter);

	return pvr::Result::Success;
}

/// <summary>Code in quitApplication() will be called by Shell once per run, just before exiting the program.
///	If the rendering context is lost, QuitApplication() will not be called.</summary>
/// <returns>Return pvr::Result::Success if no error occurred.</returns>
pvr::Result OpenGLESPVRScopeRemote::quitApplication()
{
	if (_spsCommsData)
	{
		_hasCommunicationError |= !pplSendProcessingBegin(_spsCommsData, __FUNCTION__, static_cast<uint32_t>(strlen(__FUNCTION__)), _frameCounter);

		// Close the data connection to PVRPerfServer
		for (uint32_t i = 0; i < 40; ++i)
		{
			char buf[128];
			const int nLen = sprintf(buf, "test %u", i);
			_hasCommunicationError |= !pplSendMark(_spsCommsData, buf, nLen);
		}
		_hasCommunicationError |= !pplSendProcessingEnd(_spsCommsData);
		pplShutdown(_spsCommsData);
	}

	_scene.reset();

	return pvr::Result::Success;
}

/// <summary>Code in initView() will be called by Shell upon initialization or after a change in the rendering context.</summary>
/// Used to initialize variables that are dependent on the rendering context (e.g. textures, vertex buffers, etc.)
/// <returns>Return pvr::Result::Success if no error occurred.</returns>
pvr::Result OpenGLESPVRScopeRemote::initView()
{
	_deviceResources = std::make_unique<DeviceResources>();

	_deviceResources->context = pvr::createEglContext();
	_deviceResources->context->init(getWindow(), getDisplay(), getDisplayAttributes(), this->getMinApi(), this->getMaxApi());

	_deviceResources->shaders[0] = 0;
	_deviceResources->shaders[1] = 0;

	//  Remotely editable library items
	if (_spsCommsData)
	{
		std::vector<SSPSCommsLibraryItem> communicableItems;
		size_t dataRead;

		// choose the correct shader version for the API type
		if (_deviceResources->context->getApiVersion() < pvr::Api::OpenGLES3)
		{
			_vertShaderSrcFile = VertShaderEs2SrcFile;
			_fragShaderSrcFile = FragShaderEs2SrcFile;
		}
		else
		{
			_vertShaderSrcFile = VertShaderEs3SrcFile;
			_fragShaderSrcFile = FragShaderEs3SrcFile;
		}

		//  Editable shaders
		const std::pair<std::string, std::unique_ptr<pvr::Stream>> aShaders[2] = { { _vertShaderSrcFile, getAssetStream(_vertShaderSrcFile) },
			{ _fragShaderSrcFile, getAssetStream(_fragShaderSrcFile) } };

		std::vector<char> data[sizeof(aShaders) / sizeof(*aShaders)];
		for (uint32_t i = 0; i < sizeof(aShaders) / sizeof(*aShaders); ++i)
		{
			communicableItems.push_back(SSPSCommsLibraryItem());
			communicableItems.back().pszName = aShaders[i].first.c_str();
			communicableItems.back().nNameLength = static_cast<uint32_t>(aShaders[i].first.length());
			communicableItems.back().eType = eSPSCommsLibTypeString;
			data[i].resize(aShaders[i].second->getSize());
			aShaders[i].second->read(aShaders[i].second->getSize(), 1, &data[i][0], dataRead);
			communicableItems.back().pData = &data[i][0];
			communicableItems.back().nDataLength = static_cast<uint32_t>(aShaders[i].second->getSize());
		}

		// Editable: Specular Exponent
		communicableItems.push_back(SSPSCommsLibraryItem());
		_commsLibSpecularExponent.fCurrent = _progUniforms.specularExponent;
		_commsLibSpecularExponent.fMin = 1.1f;
		_commsLibSpecularExponent.fMax = 300.0f;
		communicableItems.back().pszName = "Specular Exponent";
		communicableItems.back().nNameLength = static_cast<uint32_t>(strlen(communicableItems.back().pszName));
		communicableItems.back().eType = eSPSCommsLibTypeFloat;
		communicableItems.back().pData = (const char*)&_commsLibSpecularExponent;
		communicableItems.back().nDataLength = sizeof(_commsLibSpecularExponent);

		communicableItems.push_back(SSPSCommsLibraryItem());
		// Editable: Metallicity
		_commsLibMetallicity.fCurrent = _progUniforms.metallicity;
		_commsLibMetallicity.fMin = 0.0f;
		_commsLibMetallicity.fMax = 1.0f;
		communicableItems.back().pszName = "Metallicity";
		communicableItems.back().nNameLength = static_cast<uint32_t>(strlen(communicableItems.back().pszName));
		communicableItems.back().eType = eSPSCommsLibTypeFloat;
		communicableItems.back().pData = (const char*)&_commsLibMetallicity;
		communicableItems.back().nDataLength = sizeof(_commsLibMetallicity);

		// Editable: Reflectivity
		communicableItems.push_back(SSPSCommsLibraryItem());
		_commsLibReflectivity.fCurrent = _progUniforms.reflectivity;
		_commsLibReflectivity.fMin = 0.;
		_commsLibReflectivity.fMax = 1.;
		communicableItems.back().pszName = "Reflectivity";
		communicableItems.back().nNameLength = static_cast<uint32_t>(strlen(communicableItems.back().pszName));
		communicableItems.back().eType = eSPSCommsLibTypeFloat;
		communicableItems.back().pData = (const char*)&_commsLibReflectivity;
		communicableItems.back().nDataLength = sizeof(_commsLibReflectivity);

		// Editable: Albedo R channel
		communicableItems.push_back(SSPSCommsLibraryItem());
		_commsLibAlbedoR.fCurrent = _progUniforms.albedo.r;
		_commsLibAlbedoR.fMin = 0.0f;
		_commsLibAlbedoR.fMax = 1.0f;
		communicableItems.back().pszName = "Albedo R";
		communicableItems.back().nNameLength = static_cast<uint32_t>(strlen(communicableItems.back().pszName));
		communicableItems.back().eType = eSPSCommsLibTypeFloat;
		communicableItems.back().pData = (const char*)&_commsLibAlbedoR;
		communicableItems.back().nDataLength = sizeof(_commsLibAlbedoR);

		// Editable: Albedo R channel
		communicableItems.push_back(SSPSCommsLibraryItem());
		_commsLibAlbedoG.fCurrent = _progUniforms.albedo.g;
		_commsLibAlbedoG.fMin = 0.0f;
		_commsLibAlbedoG.fMax = 1.0f;
		communicableItems.back().pszName = "Albedo G";
		communicableItems.back().nNameLength = static_cast<uint32_t>(strlen(communicableItems.back().pszName));
		communicableItems.back().eType = eSPSCommsLibTypeFloat;
		communicableItems.back().pData = (const char*)&_commsLibAlbedoG;
		communicableItems.back().nDataLength = sizeof(_commsLibAlbedoG);

		// Editable: Albedo R channel
		communicableItems.push_back(SSPSCommsLibraryItem());
		_commsLibAlbedoB.fCurrent = _progUniforms.albedo.b;
		_commsLibAlbedoB.fMin = 0.0f;
		_commsLibAlbedoB.fMax = 1.0f;
		communicableItems.back().pszName = "Albedo B";
		communicableItems.back().nNameLength = static_cast<uint32_t>(strlen(communicableItems.back().pszName));
		communicableItems.back().eType = eSPSCommsLibTypeFloat;
		communicableItems.back().pData = (const char*)&_commsLibAlbedoB;
		communicableItems.back().nDataLength = sizeof(_commsLibAlbedoB);

		// OK, submit our library
		if (!pplLibraryCreate(_spsCommsData, communicableItems.data(), static_cast<uint32_t>(communicableItems.size())))
		{ Log(LogLevel::Debug, "PVRScopeRemote: pplLibraryCreate() failed\n"); } // User defined counters

		SSPSCommsCounterDef counterDefines[CounterDefs::NumCounter];
		for (uint32_t i = 0; i < CounterDefs::NumCounter; ++i)
		{
			counterDefines[i].pszName = FrameDefs[i];
			counterDefines[i].nNameLength = static_cast<uint32_t>(strlen(FrameDefs[i]));
		}

		if (!pplCountersCreate(_spsCommsData, counterDefines, CounterDefs::NumCounter)) { Log(LogLevel::Debug, "PVRScopeRemote: pplCountersCreate() failed\n"); }
	}
	_deviceResources->onScreenFbo = _deviceResources->context->getOnScreenFbo();

	//  Initialize VBO data
	loadVbos();
	//  Load textures
	createSamplerTexture();
	size_t dataRead;
	// Take our initial vertex shader source
	{
		std::unique_ptr<pvr::Stream> vertShader = getAssetStream(_vertShaderSrcFile);
		_vertShaderSrc.resize(vertShader->getSize() + 1, 0);
		vertShader->read(vertShader->getSize(), 1, &_vertShaderSrc[0], dataRead);
	}
	// Take our initial fragment shader source
	{
		std::unique_ptr<pvr::Stream> fragShader = getAssetStream(_fragShaderSrcFile);
		_fragShaderSrc.resize(fragShader->getSize() + 1, 0);
		fragShader->read(fragShader->getSize(), 1, &_fragShaderSrc[0], dataRead);
	}

	// create the pipeline
	createProgram(&_fragShaderSrc[0], &_vertShaderSrc[0], false);
	debugThrowOnApiError("createProgram");
	//  Initialize the UI Renderer
	_deviceResources->uiRenderer.init(getWidth(), getHeight(), isFullScreen(), getBackBufferColorspace() == pvr::ColorSpace::sRGB);

	// create the PVRScope connection pass and fail text
	_deviceResources->uiRenderer.getDefaultTitle()->setText("PVRScopeRemote");
	_deviceResources->uiRenderer.getDefaultTitle()->commitUpdates();

	_deviceResources->uiRenderer.getDefaultDescription()->setScale(glm::vec2(.5, .5));
	_deviceResources->uiRenderer.getDefaultDescription()->setText("Use PVRTune to remotely control the parameters of this application.");
	_deviceResources->uiRenderer.getDefaultDescription()->commitUpdates();

	// Calculate the projection and view matrices
	// Is the screen rotated?
	bool isRotated = this->isScreenRotated();
	if (isRotated)
	{
		_progUniforms.projectionMtx = pvr::math::perspectiveFov(_deviceResources->context->getApiVersion(), glm::pi<float>() / 6, static_cast<float>(getHeight()),
			static_cast<float>(getWidth()), _scene->getCamera(0).getNear(), _scene->getCamera(0).getFar(), glm::pi<float>() * .5f);
	}
	else
	{
		_progUniforms.projectionMtx = pvr::math::perspectiveFov(_deviceResources->context->getApiVersion(), glm::pi<float>() / 6, static_cast<float>(getWidth()),
			static_cast<float>(getHeight()), _scene->getCamera(0).getNear(), _scene->getCamera(0).getFar());
	}
	gl::BindFramebuffer(GL_FRAMEBUFFER, _deviceResources->onScreenFbo);
	gl::ClearColor(ClearColor.r, ClearColor.g, ClearColor.b, 1.0f);
	return pvr::Result::Success;
}

/// <summary>Code in releaseView() will be called by Shell when the application quits or before a change in the rendering context. </summary>
/// <returns>Return pvr::Result::Success if no error occurred </returns>
pvr::Result OpenGLESPVRScopeRemote::releaseView()
{
	CPPLProcessingScoped PPLProcessingScoped(_spsCommsData, __FUNCTION__, static_cast<uint32_t>(strlen(__FUNCTION__)), _frameCounter);
	// Release UIRenderer
	_deviceResources.reset();
	return pvr::Result::Success;
}

/// <summary>Main rendering loop function of the program. The shell will call this function every frame.</summary>
/// <returns>Return Result::Success if no error occurred.</returns>
pvr::Result OpenGLESPVRScopeRemote::renderFrame()
{
	if (_spsCommsData)
	{
		_hasCommunicationError |= !pplSendProcessingBegin(_spsCommsData, __FUNCTION__, static_cast<uint32_t>(strlen(__FUNCTION__)), _frameCounter);

		if (!_hasCommunicationError)
		{
			// mark every N frames
			if (!(_frameCounter % 100))
			{
				char buf[128];
				const int nLen = sprintf(buf, "frame %u", _frameCounter);
				_hasCommunicationError |= !pplSendMark(_spsCommsData, buf, nLen);
			}

			// Check for dirty items
			_hasCommunicationError |= !pplSendProcessingBegin(_spsCommsData, "dirty", static_cast<uint32_t>(strlen("dirty")), _frameCounter);
			{
				uint32_t nItem, nNewDataLen;
				const char* pData;
				bool recompile = false;
				while (pplLibraryDirtyGetFirst(_spsCommsData, &nItem, &nNewDataLen, &pData))
				{
					Log(LogLevel::Debug, "dirty item %u %u 0x%08x\n", nItem, nNewDataLen, pData);
					switch (nItem)
					{
					case 0:
						_vertShaderSrc.assign(pData, pData + nNewDataLen);
						_vertShaderSrc.push_back(0);
						recompile = true;
						break;

					case 1:
						_fragShaderSrc.assign(pData, pData + nNewDataLen);
						_fragShaderSrc.push_back(0);
						recompile = true;
						break;

					case 2:
						if (nNewDataLen == sizeof(SSPSCommsLibraryTypeFloat))
						{
							const SSPSCommsLibraryTypeFloat* const psData = (SSPSCommsLibraryTypeFloat*)pData;
							_progUniforms.specularExponent = psData->fCurrent;
							Log(LogLevel::Information, "Setting Specular Exponent to value [%6.2f]", _progUniforms.specularExponent);
						}
						break;
					case 3:
						if (nNewDataLen == sizeof(SSPSCommsLibraryTypeFloat))
						{
							const SSPSCommsLibraryTypeFloat* const psData = (SSPSCommsLibraryTypeFloat*)pData;
							_progUniforms.metallicity = psData->fCurrent;
							Log(LogLevel::Information, "Setting Metallicity to value [%3.2f]", _progUniforms.metallicity);
						}
						break;
					case 4:
						if (nNewDataLen == sizeof(SSPSCommsLibraryTypeFloat))
						{
							const SSPSCommsLibraryTypeFloat* const psData = (SSPSCommsLibraryTypeFloat*)pData;
							_progUniforms.reflectivity = psData->fCurrent;
							Log(LogLevel::Information, "Setting Reflectivity to value [%3.2f]", _progUniforms.reflectivity);
						}
						break;
					case 5:
						if (nNewDataLen == sizeof(SSPSCommsLibraryTypeFloat))
						{
							const SSPSCommsLibraryTypeFloat* const psData = (SSPSCommsLibraryTypeFloat*)pData;
							_progUniforms.albedo.r = psData->fCurrent;
							Log(LogLevel::Information, "Setting Albedo Red channel to value [%3.2f]", _progUniforms.albedo.r);
						}
						break;
					case 6:
						if (nNewDataLen == sizeof(SSPSCommsLibraryTypeFloat))
						{
							const SSPSCommsLibraryTypeFloat* const psData = (SSPSCommsLibraryTypeFloat*)pData;
							_progUniforms.albedo.g = psData->fCurrent;
							Log(LogLevel::Information, "Setting Albedo Green channel to value [%3.2f]", _progUniforms.albedo.g);
						}
						break;
					case 7:
						if (nNewDataLen == sizeof(SSPSCommsLibraryTypeFloat))
						{
							const SSPSCommsLibraryTypeFloat* const psData = (SSPSCommsLibraryTypeFloat*)pData;
							_progUniforms.albedo.b = psData->fCurrent;
							Log(LogLevel::Information, "Setting Albedo Blue channel to value [%3.2f]", _progUniforms.albedo.b);
						}
						break;
					}
				}

				if (recompile)
				{
					try
					{
						createProgram(&_fragShaderSrc[0], &_vertShaderSrc[0], true);
					}
					catch (std::runtime_error& e)
					{
						_deviceResources->uiRenderer.getDefaultControls()->setText(std::string("*** Could not recompile the shaders passed from PVRScopeComms **** ") + e.what());
						Log("*** Could not recompile the shaders passed from PVRScopeComms **** %s", e.what());
					}
				}
			}
			_hasCommunicationError |= !pplSendProcessingEnd(_spsCommsData);
		}
	}

	if (_spsCommsData) { _hasCommunicationError |= !pplSendProcessingBegin(_spsCommsData, "draw", static_cast<uint32_t>(strlen("draw")), _frameCounter); }

	gl::CullFace(GL_BACK);
	gl::FrontFace(GL_CCW);
	gl::Enable(GL_DEPTH_TEST);
	gl::Clear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	gl::UseProgram(_deviceResources->program);

	// Rotate and Translation the model matrix
	glm::mat4 modelMtx = glm::rotate(_angleY, glm::vec3(0, 1, 0)) * glm::scale(glm::vec3(0.6f)) * _scene->getWorldMatrix(0);
	_angleY += (2 * glm::pi<float>() * getFrameTime() / 1000) / 10;

	_progUniforms.viewMtx = glm::lookAt(glm::vec3(0.f, 0.f, 75.f), glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));

	// Set model view projection matrix
	_progUniforms.mvMatrix = _progUniforms.viewMtx * modelMtx;
	_progUniforms.mvpMatrix = _progUniforms.projectionMtx * _progUniforms.mvMatrix;

	_progUniforms.mvITMatrix = glm::inverseTranspose(glm::mat3(_progUniforms.mvMatrix));

	// Set light direction in model space
	_progUniforms.lightDirView = glm::normalize(glm::vec3(1., 1., -1.));

	// Now that the uniforms are set, draw the mesh.
	if (_hasCommunicationError)
	{
		_deviceResources->uiRenderer.getDefaultControls()->setText("Communication Error:\nPVRScopeComms failed\n"
																   "Is PVRPerfServer connected?");
		_deviceResources->uiRenderer.getDefaultControls()->setColor(glm::vec4(.8f, .3f, .3f, 1.0f));
		_hasCommunicationError = false;
	}
	else
	{
		_deviceResources->uiRenderer.getDefaultControls()->setText("PVRScope Communication established.");
		_deviceResources->uiRenderer.getDefaultControls()->setColor(glm::vec4(1.f));
	}

	_deviceResources->uiRenderer.getDefaultControls()->commitUpdates();

	gl::BindTexture(GL_TEXTURE_2D, _deviceResources->texture);

	gl::Uniform3fv(_uniformLocations.lightDirView, 1, glm::value_ptr(_progUniforms.lightDirView));
	gl::UniformMatrix4fv(_uniformLocations.mvpMtx, 1, false, glm::value_ptr(_progUniforms.mvpMatrix));
	gl::UniformMatrix3fv(_uniformLocations.mvITMtx, 1, false, glm::value_ptr(_progUniforms.mvITMatrix));
	gl::Uniform1fv(_uniformLocations.specularExponent, 1, &_progUniforms.specularExponent);
	gl::Uniform1fv(_uniformLocations.metallicity, 1, &_progUniforms.metallicity);
	gl::Uniform1fv(_uniformLocations.reflectivity, 1, &_progUniforms.reflectivity);
	gl::Uniform3fv(_uniformLocations.albedo, 1, glm::value_ptr(_progUniforms.albedo));

	drawMesh(0);

	if (_spsCommsData) { _hasCommunicationError |= !pplSendProcessingEnd(_spsCommsData); }

	if (_spsCommsData) { _hasCommunicationError |= !pplSendProcessingBegin(_spsCommsData, "UIRenderer", static_cast<uint32_t>(strlen("UIRenderer")), _frameCounter); }
	// Displays the demo name using the tools. For a detailed explanation, see the example
	// IntroducingUIRenderer
	_deviceResources->uiRenderer.beginRendering();
	_deviceResources->uiRenderer.getDefaultTitle()->render();
	_deviceResources->uiRenderer.getDefaultDescription()->render();
	_deviceResources->uiRenderer.getSdkLogo()->render();
	_deviceResources->uiRenderer.getDefaultControls()->render();
	_deviceResources->uiRenderer.endRendering();
	if (_spsCommsData) { _hasCommunicationError |= !pplSendProcessingEnd(_spsCommsData); }

	// send counters
	_counterReadings[CounterDefs::Counter] = _frameCounter;
	_counterReadings[CounterDefs::Counter10] = _frame10Counter;
	if (_spsCommsData) { _hasCommunicationError |= !pplCountersUpdate(_spsCommsData, _counterReadings); }

	// update some counters
	++_frameCounter;
	if (0 == (_frameCounter / 10) % 10) { _frame10Counter += 10; }

	if (this->shouldTakeScreenshot()) { pvr::utils::takeScreenshot(this->getScreenshotFileName(), this->getWidth(), this->getHeight()); }

	_deviceResources->context->swapBuffers();

	if (_spsCommsData)
	{
		_hasCommunicationError |= !pplSendProcessingEnd(_spsCommsData);
		_hasCommunicationError |= !pplSendFlush(_spsCommsData);
	}

	return pvr::Result::Success;
}

/// <summary>Draws a pvr::assets::Mesh after the model view matrix has been set and the material prepared.</summary>
/// <param name="nodeIndex">Node index of the mesh to draw </param>
void OpenGLESPVRScopeRemote::drawMesh(int nodeIndex)
{
	debugThrowOnApiError("draw mesh");
	CPPLProcessingScoped PPLProcessingScoped(_spsCommsData, __FUNCTION__, static_cast<uint32_t>(strlen(__FUNCTION__)), _frameCounter);

	uint32_t meshIndex = _scene->getNode(nodeIndex).getObjectId();
	const pvr::assets::Mesh& mesh = _scene->getMesh(meshIndex);
	// bind the VBO for the mesh
	gl::BindBuffer(GL_ARRAY_BUFFER, _deviceResources->vbos[meshIndex]);
	debugThrowOnApiError("draw mesh");
	for (uint32_t i = 0; i < 3; ++i)
	{
		auto& attrib = _vertexConfiguration.attributes[i];
		auto& binding = _vertexConfiguration.bindings[0];
		gl::EnableVertexAttribArray(attrib.index);
		gl::VertexAttribPointer(attrib.index, attrib.width, pvr::utils::convertToGles(attrib.format), dataTypeIsNormalised(attrib.format), binding.strideInBytes,
			reinterpret_cast<const void*>(static_cast<uintptr_t>(attrib.offsetInBytes)));
		debugThrowOnApiError("draw mesh");
	}

	debugThrowOnApiError("draw mesh");

	//  The geometry can be exported in 4 ways:
	//  - Indexed Triangle list
	//  - Non-Indexed Triangle list
	//  - Indexed Triangle strips
	//  - Non-Indexed Triangle strips
	if (mesh.getNumStrips() == 0)
	{
		if (_deviceResources->ibos[meshIndex])
		{
			// Indexed Triangle list
			gl::BindBuffer(GL_ELEMENT_ARRAY_BUFFER, _deviceResources->ibos[meshIndex]);
			gl::DrawElements(GL_TRIANGLES, mesh.getNumFaces() * 3, GL_UNSIGNED_SHORT, nullptr);
		}
		else
		{
			// Non-Indexed Triangle list
			gl::DrawArrays(GL_TRIANGLES, 0, mesh.getNumFaces());
		}
	}
	else
	{
		for (uint32_t i = 0; i < mesh.getNumStrips(); ++i)
		{
			int offset = 0;
			if (_deviceResources->ibos[meshIndex])
			{
				// Indexed Triangle strips
				gl::BindBuffer(GL_ELEMENT_ARRAY_BUFFER, _deviceResources->ibos[meshIndex]);
				gl::DrawElements(GL_TRIANGLE_STRIP, mesh.getStripLength(i) + 2, GL_UNSIGNED_SHORT, 0);
			}
			else
			{
				// Non-Indexed Triangle strips
				gl::DrawArrays(GL_TRIANGLE_STRIP, 0, mesh.getStripLength(i) + 2);
			}
			offset += mesh.getStripLength(i) + 2;
		}
	}
	for (uint32_t i = 0; i < 3; ++i)
	{
		auto& attrib = _vertexConfiguration.attributes[i];
		gl::DisableVertexAttribArray(attrib.index);
	}
}

/// <summary>This function must be implemented by the user of the shell. The user should return its pvr::Shell object defining the behaviour of the application.</summary>
/// <returns>Return a unique ptr to the demo supplied by the user.</returns>
std::unique_ptr<pvr::Shell> pvr::newDemo() { return std::make_unique<OpenGLESPVRScopeRemote>(); }
