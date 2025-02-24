/*!
\brief Particle animation system using Compute Shaders. Requires the PVRShell.
\file OpenGLESParticleSystem.cpp
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
#include "PVRShell/PVRShell.h"
#include "PVRUtils/PVRUtilsGles.h"

/// === Objects that have corresponding representations in Shader Code (Ubo, SSBO)
/// We are using PRAGMA PACK to remove all compiler-generated padding and hence only add our own explicit
/// padding, following the std140 rules (http://www.opengl.org/registry/doc/glspec45.core.pdf#page=159)
/// This is not strictly 100% necessary as std140 is actually "stricter" (has more padding) than all "common"
/// architectures (the example will still run fine without the pragma pack in x86/x64 and armvX architectures
/// compiled with any tested VS, GCC or Clang version) but it is the right thing to do.
#pragma pack(push)
#pragma pack(1)

/// <summary>The particle structure will be kept packed. We follow the STD140 spec to explicitly add the paddings
/// so that we can be sure of the layout.</summary>
struct Particle
{
	glm::vec3 vPosition; // vec3
	float _padding;
	glm::vec3 vVelocity; // vec4.xyz
	float fTimeToLive; // vec4/w
}; // SIZE:32 bytes

/// <summary>All the following will all be used in uniforms/ssbos, so we will mimic the alignment of std140 glsl
/// layout spec in order to make their use simpler.</summary>
struct Sphere
{
	glm::vec3 vPosition; // vec4: xyz
	float fRadius; // vec4: w
	Sphere(const glm::vec3& pos, float radius) : vPosition(pos), fRadius(radius) {}
};

struct Emitter
{
	glm::mat4 mTransformation; // mat4
	float fHeight; // float
	float fRadius; // float
	Emitter(const glm::mat4& trans, float height, float radius) : mTransformation(trans), fHeight(height), fRadius(radius) {}
	Emitter() {}
};

struct ParticleConfig
{
	Emitter emitter; // 18 floats //Emitter will need 2 floats padding to be a multiple of 16 (vec4 size).
	float _padding1_[2]; // 20 floats //These are non-reclaimable as emitter is a struct.
	glm::vec3 gravity; // 23 floats //vec3 will be aligned to 4 floats, but the last element can be filled with a float
	float dt; // 24 floats //simple float
	float totalTime; // 25 floats //simple float
	float dragCoeffLinear; // 26 floats //simple float
	float dragCoeffQuadratic; // 27 floats //simple float
	float inwardForceCoeff; // 28 floats //simple float
	float inwardForceRadius; // 29 floats //simple float
	float bounciness; // 30 floats //simple float
	float minLifespan; // 31 floats //simple float
	float maxLifespan; // 32 floats //simple float
	// float   _padding2_[2];	// Luckily this struct is a multiple of 16. Otherwise, we would have used padding defensively as std140
	// dictates that the size of the whole ubo will be aligned to the size of vec4. (i.e. 4floats/16 bytes : we would pad 30 to 32, 33 to 48 etc)
	ParticleConfig() {}
};
#pragma pack(pop)

namespace Files {
// Asset files
const char SphereModel[] = "sphere.pod";
const char FragShader[] = "FragShader.fsh";
const char VertShader[] = "VertShader.vsh";
const char ParticleFragShader[] = "ParticleFragShader.fsh";
const char ParticleVertShader[] = "ParticleVertShader.vsh";
const char ParticleComputeShader[] = "ParticleSolver.csh";
} // namespace Files

namespace Configuration {
enum
{
	MinNoParticles = 128,
	MaxNoParticles = 262144,
	InitialNoParticles = 4096,
	NumberOfSpheres = 8,
};

const float CameraNear = .1f;
const float CameraFar = 1000.0f;
const glm::vec3 LightPosition(0.0f, 10.0f, 0.0f);
const uint32_t workgroupSize = 32;
const Sphere SpheresData[] = {
	Sphere(glm::vec3(-20.0f, 6.0f, -20.0f), 5.f),
	Sphere(glm::vec3(-20.0f, 6.0f, 0.0f), 5.f),
	Sphere(glm::vec3(-20.0f, 6.0f, 20.0f), 5.f),
	Sphere(glm::vec3(0.0f, 6.0f, -20.0f), 5.f),
	Sphere(glm::vec3(0.0f, 6.0f, 20.0f), 5.f),
	Sphere(glm::vec3(20.0f, 6.0f, -20.0f), 5.f),
	Sphere(glm::vec3(20.0f, 6.0f, 0.0f), 5.f),
	Sphere(glm::vec3(20.0f, 6.0f, 20.0f), 5.f),
};
} // namespace Configuration

// Index to bind the attributes to vertex shaders
namespace Attributes {
enum Enum
{
	ParticlePoisitionArray = 0,
	ParticleLifespanArray = 1,
	VertexArray = 0,
	NormalArray = 1,
	TexCoordArray = 2,
	BindingIndex0 = 0
};
}

enum BufferBindingPoint
{
	SPHERES_UBO_BINDING_INDEX = 1,
	PARTICLE_CONFIG_UBO_BINDING_INDEX = 2,
	PARTICLES_SSBO_BINDING_INDEX_IN = 3,
	PARTICLES_SSBO_BINDING_INDEX_OUT = 4,
};

const uint32_t NumBuffers(2);

/// <summary>class implementing the PVRShell functions.</summary>
class OpenGLESParticleSystem : public pvr::Shell
{
private:
	struct DrawPass
	{
		glm::mat4 model;
		glm::mat4 modelView;
		glm::mat4 modelViewProj;
		glm::mat3 modelViewIT;
		glm::vec3 lightPos;
	};

	struct DeviceResources
	{
		pvr::EglContext context;

		GLuint sphereVbo;
		GLuint sphereIbo;
		GLuint sphereVao;

		GLuint floorVao;
		GLuint floorVbo;

		// Manually ghosted buffer objects
		GLuint particleBuffers[NumBuffers];
		GLuint particleVaos[NumBuffers];
		GLuint particleConfigUbo, spheresUbo;

		// UIRenderer used to display text
		pvr::ui::UIRenderer uiRenderer;

		struct ParticleProgram
		{
			GLuint program;
			GLint positionArrayLoc;
			GLint lifespanArrayLoc;
			GLint mvpMatrixLoc;
			ParticleProgram() : program(0), positionArrayLoc(-1), lifespanArrayLoc(-1), mvpMatrixLoc(-1) {}
		} programParticle;

		struct Program
		{
			GLuint program;
			GLint mvMatrixLoc;
			GLint mvITMatrixLoc;
			GLint mvpMatrixLoc;
			GLint lightPositionLoc;
			Program() : program(0), mvMatrixLoc(-1), mvITMatrixLoc(-1), mvpMatrixLoc(-1), lightPositionLoc(-1) {}
		};
		Program programSimple;
		Program programFloor;

		struct ComputeProgram
		{
			GLuint program;
			ComputeProgram() : program(0) {}
		} programParticlesCompute;

		DeviceResources() : sphereVbo(0), sphereIbo(0), floorVbo(0) { memset(particleBuffers, 0, sizeof(particleBuffers)); }
	};
	std::unique_ptr<DeviceResources> _deviceResources;

	pvr::assets::ModelHandle _scene;
	bool _isCameraPaused;
	uint8_t _currentBufferIdx;

	// View matrix
	glm::mat4 _viewMtx, _projMtx, _viewProjMtx;
	glm::mat3 _viewIT;
	glm::vec3 _lightPos;
	DrawPass _passSphere[Configuration::NumberOfSpheres];

	// SIMULATION DATA
	uint32_t _numParticles;

	ParticleConfig _particleConfigData;
	std::vector<Particle> _particleArrayData;

	bool _blendModeAdditive;

public:
	OpenGLESParticleSystem();

	virtual pvr::Result initApplication();
	virtual pvr::Result initView();
	virtual pvr::Result releaseView();
	virtual pvr::Result quitApplication();
	virtual pvr::Result renderFrame();
	virtual void eventMappedInput(pvr::SimplifiedInput key);

	bool createBuffers();
	bool createPrograms();
	void updateFloorProgramUniforms();
	void updateSphereProgramUniforms(const glm::mat4& proj, const glm::mat4& view);
	void updateParticleUniforms();
	void useSimplePipelineProgramAndSetState();
	void useFloorPipelineProgramAndSetState();
	void useParticleRenderingProgramAndSetState();
	void renderScene();
	void renderParticles();
	void initializeParticles();
};

/// <summary>Handles user input and updates live variables accordingly. </summary>
/// <param name="key" Input key to handle.</param>
void OpenGLESParticleSystem::eventMappedInput(pvr::SimplifiedInput key)
{
	switch (key)
	{
	case pvr::SimplifiedInput::Left:
	{
		if (_numParticles / 2 >= Configuration::MinNoParticles)
		{
			_numParticles /= 2;
			initializeParticles();
			_deviceResources->uiRenderer.getDefaultDescription()->setText(pvr::strings::createFormatted("No. of Particles: %d", _numParticles));
			_deviceResources->uiRenderer.getDefaultDescription()->commitUpdates();
		}
	}
	break;
	case pvr::SimplifiedInput::Right:
	{
		if (_numParticles * 2 <= Configuration::MaxNoParticles)
		{
			_numParticles *= 2;
			initializeParticles();
			_deviceResources->uiRenderer.getDefaultDescription()->setText(pvr::strings::createFormatted("No. of Particles: %d", _numParticles));
			_deviceResources->uiRenderer.getDefaultDescription()->commitUpdates();
		}
	}
	break;
	case pvr::SimplifiedInput::Action1: _isCameraPaused = !_isCameraPaused; break;
	case pvr::SimplifiedInput::Action2: _blendModeAdditive = !_blendModeAdditive; break;
	case pvr::SimplifiedInput::ActionClose: exitShell(); break;
	default: break;
	}
}

OpenGLESParticleSystem::OpenGLESParticleSystem() : _isCameraPaused(0), _numParticles(Configuration::InitialNoParticles), _particleArrayData(0), _blendModeAdditive(true)
{
	memset(&_particleConfigData, 0, sizeof(ParticleConfig));
}

/// <summary>Loads the mesh data required for this training course into vertex buffer objects.</summary>
/// <returns>Return true on success.</returns>
bool OpenGLESParticleSystem::createBuffers()
{
	// Create the VBO for the Sphere model
	pvr::utils::createSingleBuffersFromMesh(_scene->getMesh(0), _deviceResources->sphereVbo, _deviceResources->sphereIbo);

	gl::GenVertexArrays(1, &_deviceResources->sphereVao);
	gl::BindVertexArray(_deviceResources->sphereVao);
	gl::BindVertexBuffer(0, _deviceResources->sphereVbo, 0, _scene->getMesh(0).getStride(0));
	gl::BindBuffer(GL_ELEMENT_ARRAY_BUFFER, _deviceResources->sphereIbo);
	gl::EnableVertexAttribArray(0);
	gl::EnableVertexAttribArray(1);
	gl::VertexAttribBinding(0, 0);
	gl::VertexAttribBinding(1, 0);
	gl::VertexAttribFormat(0, 3, GL_FLOAT, GL_FALSE, _scene->getMesh(0).getVertexAttributeByName("POSITION")->getOffset());
	gl::VertexAttribFormat(1, 3, GL_FLOAT, GL_FALSE, _scene->getMesh(0).getVertexAttributeByName("NORMAL")->getOffset());

	// Initialize the vertex buffer data for the floor - 3*Position data, 3* normal data
	glm::vec2 maxCorner(40, 40);
	const float afVertexBufferData[] = { -maxCorner.x, 0.0f, -maxCorner.y, 0.0f, 1.0f, 0.0f, -maxCorner.x, 0.0f, maxCorner.y, 0.0f, 1.0f, 0.0f, maxCorner.x, 0.0f, -maxCorner.y,
		0.0f, 1.0f, 0.0f, maxCorner.x, 0.0f, maxCorner.y, 0.0f, 1.0f, 0.0f };
	gl::GenBuffers(1, &_deviceResources->floorVbo);
	gl::BindBuffer(GL_ARRAY_BUFFER, _deviceResources->floorVbo);
	gl::BufferData(GL_ARRAY_BUFFER, sizeof(afVertexBufferData), afVertexBufferData, GL_STATIC_DRAW);

	gl::GenVertexArrays(1, &_deviceResources->floorVao);
	gl::BindVertexArray(_deviceResources->floorVao);
	gl::BindVertexBuffer(0, _deviceResources->floorVbo, 0, 6 * sizeof(float));
	gl::EnableVertexAttribArray(0);
	gl::EnableVertexAttribArray(1);
	gl::VertexAttribBinding(0, 0);
	gl::VertexAttribBinding(1, 0);
	gl::VertexAttribFormat(0, 3, GL_FLOAT, GL_FALSE, 0);
	gl::VertexAttribFormat(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float));

	// Create the (VBO/SSBO) Particles buffer
	gl::GenBuffers(NumBuffers, _deviceResources->particleBuffers);
	gl::GenVertexArrays(NumBuffers, _deviceResources->particleVaos);
	for (uint32_t i = 0; i < NumBuffers; ++i)
	{
		gl::BindBuffer(GL_ARRAY_BUFFER, _deviceResources->particleBuffers[i]);
		gl::BufferData(GL_ARRAY_BUFFER, sizeof(Particle) * _numParticles, nullptr, GL_DYNAMIC_COPY);
		gl::BindVertexArray(_deviceResources->particleVaos[i]);
		gl::BindVertexBuffer(0, _deviceResources->particleBuffers[i], 0, sizeof(Particle));
		gl::EnableVertexAttribArray(0);
		gl::EnableVertexAttribArray(1);
		gl::VertexAttribBinding(0, 0);
		gl::VertexAttribBinding(1, 0);
		gl::VertexAttribFormat(0, 3, GL_FLOAT, GL_FALSE, 0);
		gl::VertexAttribFormat(1, 1, GL_FLOAT, GL_FALSE, sizeof(glm::vec4));
	}

	gl::BindVertexArray(0);

	// Create the "Physical" collision spheres UBO (Sphere centre and radius, used for the Compute collisions)

	gl::GenBuffers(1, &_deviceResources->spheresUbo);
	gl::BindBuffer(GL_UNIFORM_BUFFER, _deviceResources->spheresUbo);
	gl::BufferData(GL_UNIFORM_BUFFER, sizeof(Sphere) * Configuration::NumberOfSpheres, Configuration::SpheresData, GL_STATIC_DRAW);

	gl::GenBuffers(1, &_deviceResources->particleConfigUbo);
	gl::BindBuffer(GL_UNIFORM_BUFFER, _deviceResources->particleConfigUbo);
	gl::BufferData(GL_UNIFORM_BUFFER, sizeof(ParticleConfig), &_particleConfigData, GL_STATIC_DRAW);

	return true;
}

void OpenGLESParticleSystem::useSimplePipelineProgramAndSetState()
{
	gl::UseProgram(_deviceResources->programSimple.program);
	// NO BLENDING, BACK FACE CULLING, DEPTH TEST ENABLED, DEPTH WRITE ENABLED, TRIANGLE LIST.
	// SIMPLE_PIPE
	gl::Disable(GL_BLEND);
	gl::Enable(GL_CULL_FACE);
	gl::CullFace(GL_BACK);
	gl::FrontFace(GL_CCW);
	gl::Enable(GL_DEPTH_TEST);
	gl::DepthMask(GL_TRUE);
	// ENABLE
}

void OpenGLESParticleSystem::useFloorPipelineProgramAndSetState()
{
	gl::UseProgram(_deviceResources->programSimple.program);
	// NO BLENDING, BACK FACE CULLING, DEPTH TEST ENABLED, DEPTH WRITE ENABLED, TRIANGLE LIST.
	// SIMPLE_PIPE
	gl::Disable(GL_BLEND);
	// ENABLE
}

void OpenGLESParticleSystem::useParticleRenderingProgramAndSetState()
{
	gl::UseProgram(_deviceResources->programParticle.program);
	gl::Enable(GL_BLEND);
	gl::DepthMask(GL_FALSE);
	// Source alpha factor is GL_ZERO and destination alpha factor is GL_ONE to preserve the framebuffer alpha value,
	// in order to avoid artefacts in compositors that actually support framebuffer alpha for window transparency.
	gl::BlendFuncSeparate(GL_SRC_ALPHA, _blendModeAdditive ? GL_ONE : GL_ONE_MINUS_SRC_ALPHA, GL_ZERO, GL_ONE);
	gl::BlendEquation(GL_FUNC_ADD);
}

/// <summary>Loads and compiles the shaders and links the shader programs required for this training course.</summary>
/// <returns>Return pvr::Result::Success if no error occurred.</returns>
bool OpenGLESParticleSystem::createPrograms()
{
	// Enable or disable gamma correction based on if it is automatically performed on the framebuffer or we need to do it in the shader.
	const char* graphicsDefines[] = { "FRAMEBUFFER_SRGB" };
	uint32_t numDefines = 1;
	if (getBackBufferColorspace() != pvr::ColorSpace::sRGB) { numDefines = 0; }
	// Simple Pipeline
	{
		pvr::utils::VertexAttributeInfoGles attributes[2];
		//"inVertex"
		attributes[0].format = GL_FLOAT;
		attributes[0].index = 0;
		attributes[0].offset = 0;
		attributes[0].size = 3;
		attributes[0].stride = sizeof(float) * (3 + 3);
		attributes[0].vboIndex = 0;
		//"inNormal"
		attributes[1] = attributes[0];
		attributes[1].index = 1;
		attributes[1].offset = reinterpret_cast<void*>(sizeof(float) * 3);

		const char* simplePipeAttributes[] = { "inVertex", "inNormal" };
		const uint16_t simplePipeAttributeIndices[] = { Attributes::VertexArray, Attributes::NormalArray };

		_deviceResources->programSimple.program =
			pvr::utils::createShaderProgram(*this, Files::VertShader, Files::FragShader, simplePipeAttributes, simplePipeAttributeIndices, 2, graphicsDefines, numDefines);

		useSimplePipelineProgramAndSetState();

		_deviceResources->programSimple.mvMatrixLoc = gl::GetUniformLocation(_deviceResources->programSimple.program, "uModelViewMatrix");
		_deviceResources->programSimple.mvITMatrixLoc = gl::GetUniformLocation(_deviceResources->programSimple.program, "uModelViewITMatrix");
		_deviceResources->programSimple.mvpMatrixLoc = gl::GetUniformLocation(_deviceResources->programSimple.program, "uModelViewProjectionMatrix");
		_deviceResources->programSimple.lightPositionLoc = gl::GetUniformLocation(_deviceResources->programSimple.program, "uLightPosition");
	}

	//  Floor Pipeline
	{
		const char* floorPipeAttributes[] = { "inVertex", "inNormal" };
		const uint16_t floorPipeAttributeIndices[] = { Attributes::VertexArray, Attributes::NormalArray };

		_deviceResources->programFloor.program =
			pvr::utils::createShaderProgram(*this, Files::VertShader, Files::FragShader, floorPipeAttributes, floorPipeAttributeIndices, 2, graphicsDefines, numDefines);

		_deviceResources->programFloor.mvMatrixLoc = gl::GetUniformLocation(_deviceResources->programFloor.program, "uModelViewMatrix");
		_deviceResources->programFloor.mvITMatrixLoc = gl::GetUniformLocation(_deviceResources->programFloor.program, "uModelViewITMatrix");
		_deviceResources->programFloor.mvpMatrixLoc = gl::GetUniformLocation(_deviceResources->programFloor.program, "uModelViewProjectionMatrix");
		_deviceResources->programFloor.lightPositionLoc = gl::GetUniformLocation(_deviceResources->programFloor.program, "uLightPosition");
	}

	//  Particle Pipeline
	{
		const char* particleAttribs[] = { "inPosition", "inLifespan" };
		const uint16_t particleAttribIndices[] = { 0, 1 };

		_deviceResources->programParticle.program =
			pvr::utils::createShaderProgram(*this, Files::ParticleVertShader, Files::ParticleFragShader, particleAttribs, particleAttribIndices, 2, graphicsDefines, numDefines);
		_deviceResources->programParticle.mvpMatrixLoc = gl::GetUniformLocation(_deviceResources->programParticle.program, "uModelViewProjectionMatrix");
	}

	//  Particle Compute Pipeline
	{
		gl::BindBufferBase(GL_UNIFORM_BUFFER, PARTICLE_CONFIG_UBO_BINDING_INDEX, _deviceResources->particleConfigUbo);
		gl::BindBufferBase(GL_UNIFORM_BUFFER, SPHERES_UBO_BINDING_INDEX, _deviceResources->spheresUbo);

		bool pingpong = 0;
		gl::BindBufferBase(GL_SHADER_STORAGE_BUFFER, PARTICLES_SSBO_BINDING_INDEX_IN, _deviceResources->particleBuffers[pingpong]);
		gl::BindBufferBase(GL_SHADER_STORAGE_BUFFER, PARTICLES_SSBO_BINDING_INDEX_OUT, _deviceResources->particleBuffers[pingpong]);

		std::string defines("WORKGROUP_SIZE               ", 30);
		sprintf(&defines[16], "%d", Configuration::workgroupSize);
		const char* defines_buffer[1] = { &defines[0] };

		_deviceResources->programParticlesCompute.program = pvr::utils::createComputeShaderProgram(*this, Files::ParticleComputeShader, defines_buffer, 1);
	}

	// GLOBAL STATE ALL CONFIGS USE
	gl::Enable(GL_DEPTH_TEST);
	gl::DepthMask(GL_TRUE);
	gl::Enable(GL_CULL_FACE);
	gl::CullFace(GL_BACK);
	gl::FrontFace(GL_CCW);

	return true;
}

/// <summary>Code in initApplication() will be called by the Shell once per run, before the rendering context is created.
/// Used to initialize variables that are not dependent on it  (e.g. external modules, loading meshes, etc.)
/// If the rendering context is lost, InitApplication() will not be called again.</summary>
/// <returns>Return pvr::Result::Success if no error occurred.</returns>
pvr::Result OpenGLESParticleSystem::initApplication()
{
	// Load the _scene
	_scene = pvr::assets::loadModel(*this, Files::SphereModel);

	for (uint32_t i = 0; i < _scene->getNumMeshes(); ++i)
	{
		_scene->getMesh(i).setVertexAttributeIndex("POSITION0", Attributes::VertexArray);
		_scene->getMesh(i).setVertexAttributeIndex("NORMAL0", Attributes::NormalArray);
		_scene->getMesh(i).setVertexAttributeIndex("UV0", Attributes::TexCoordArray);
	}

	return pvr::Result::Success;
}

/// <summary>Code in quitApplication() will be called by the Shell once per run, just before exiting the program.
/// If the rendering context is lost, QuitApplication() will not be called.</summary>
/// <returns>Return pvr::Result::Success if no error occurred.</returns>
pvr::Result OpenGLESParticleSystem::quitApplication()
{
	_scene.reset();
	return pvr::Result::Success;
}

/// <summary>Code in initView() will be called by the Shell upon initialization or after a change in the rendering context.
/// Used to initialize variables that are dependent on the rendering context (e.g. textures, vertex buffers, etc.).</summary>
/// <returns>Return pvr::Result::Success if no error occurred.</returns>
pvr::Result OpenGLESParticleSystem::initView()
{
	if (this->getMinApi() < pvr::Api::OpenGLES31) { Log(LogLevel::Information, "This demo requires a minimum API of OpenGLES31."); }

	_deviceResources = std::make_unique<DeviceResources>();
	_deviceResources->context = pvr::createEglContext();
	_deviceResources->context->init(getWindow(), getDisplay(), getDisplayAttributes(), pvr::Api::OpenGLES31);

	// Initialize UIRenderer textures
	_deviceResources->uiRenderer.init(getWidth(), getHeight(), isFullScreen(), getBackBufferColorspace() == pvr::ColorSpace::sRGB);

	//  Create the Buffers
	if (!createBuffers()) { return pvr::Result::UnknownError; }

	//  Load and compile the shaders & link programs
	if (!createPrograms()) { return pvr::Result::UnknownError; }

	// Set the gravity
	_particleConfigData.gravity = glm::vec3(0.f, -9.81f, 0.f);
	_particleConfigData.dragCoeffLinear = 0.f;
	_particleConfigData.dragCoeffQuadratic = 0.f;
	_particleConfigData.inwardForceCoeff = 0.f;
	_particleConfigData.inwardForceRadius = .001f;
	_particleConfigData.bounciness = .9f;
	_particleConfigData.minLifespan = .5f;
	_particleConfigData.maxLifespan = 1.5f;

	initializeParticles();

	// Creates the projection matrix.
	_projMtx = glm::perspectiveFov(glm::pi<float>() / 3.0f, static_cast<float>(getWidth()), static_cast<float>(getHeight()), Configuration::CameraNear, Configuration::CameraFar);

	_deviceResources->uiRenderer.getDefaultTitle()->setText("ParticleSystem");
	_deviceResources->uiRenderer.getDefaultDescription()->setText(pvr::strings::createFormatted("No. of Particles: %d", _numParticles));
	_deviceResources->uiRenderer.getDefaultControls()->setText("Action1: Pause rotation\nLeft: Decrease particles\nRight: Increase particles");
	_deviceResources->uiRenderer.getDefaultTitle()->commitUpdates();
	_deviceResources->uiRenderer.getDefaultDescription()->commitUpdates();
	_deviceResources->uiRenderer.getDefaultControls()->commitUpdates();
	gl::ClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	return pvr::Result::Success;
}

/// <summary>Code in releaseView() will be called by pvr::Shell when the application quits or before a change in the rendering context. </summary>
/// <returns>Return pvr::Result::Success if no error occurred.</returns>
pvr::Result OpenGLESParticleSystem::releaseView()
{
	_deviceResources.reset();
	return pvr::Result::Success;
}

/// <summary>Main rendering loop function of the program. The shell will call this function every frame. </summary>
/// <returns>Return pvr::Result::Success if no error occurred.</returns>
pvr::Result OpenGLESParticleSystem::renderFrame()
{
	gl::DepthMask(GL_TRUE);
	gl::Clear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	_currentBufferIdx++;
	if (_currentBufferIdx >= NumBuffers) { _currentBufferIdx = 0; }
	debugThrowOnApiError("OpenGLESParticleSystem::renderFrame Enter");
	updateParticleUniforms();

	if (!_isCameraPaused)
	{
		static float angle = 0;
		angle += getFrameTime() / 5000.0f;
		glm::vec3 vFrom = glm::vec3(sinf(angle) * 50.0f, 30.0f, cosf(angle) * 50.0f);

		_viewMtx = glm::lookAt(vFrom, glm::vec3(0.0f, 15.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		_viewProjMtx = _projMtx * _viewMtx;
	}
	// Render floor
	updateFloorProgramUniforms();
	updateSphereProgramUniforms(_projMtx, _viewMtx);

	{
		gl::UseProgram(_deviceResources->programParticlesCompute.program);

		gl::BindBufferBase(GL_UNIFORM_BUFFER, PARTICLE_CONFIG_UBO_BINDING_INDEX, _deviceResources->particleConfigUbo);
		gl::BindBufferBase(GL_UNIFORM_BUFFER, SPHERES_UBO_BINDING_INDEX, _deviceResources->spheresUbo);

		int inputBuffer = _currentBufferIdx % NumBuffers;
		int outputBuffer = (_currentBufferIdx + 1) % NumBuffers;

		gl::BindBufferBase(GL_SHADER_STORAGE_BUFFER, PARTICLES_SSBO_BINDING_INDEX_IN, _deviceResources->particleBuffers[inputBuffer]);
		gl::BindBufferBase(GL_SHADER_STORAGE_BUFFER, PARTICLES_SSBO_BINDING_INDEX_OUT, _deviceResources->particleBuffers[outputBuffer]);

		// Accesses to shader storage blocks after this barrier will reflect writes prior to the barrier.
		gl::DispatchCompute(_numParticles / Configuration::workgroupSize, 1, 1);
	}

	// Vertex data sourced after this barrier will reflect data written by shaders prior to the barrier
	gl::MemoryBarrier(GL_VERTEX_ATTRIB_ARRAY_BARRIER_BIT | GL_SHADER_STORAGE_BARRIER_BIT);

	renderScene();
	renderParticles();

	gl::BindVertexArray(0);

	_deviceResources->uiRenderer.beginRendering();
	_deviceResources->uiRenderer.getDefaultTitle()->render();
	_deviceResources->uiRenderer.getDefaultDescription()->render();
	_deviceResources->uiRenderer.getDefaultControls()->render();
	_deviceResources->uiRenderer.getSdkLogo()->render();
	_deviceResources->uiRenderer.endRendering();
	debugThrowOnApiError("OpenGLESParticleSystem::renderFrame Exit");

	if (this->shouldTakeScreenshot()) { pvr::utils::takeScreenshot(this->getScreenshotFileName(), this->getWidth(), this->getHeight()); }

	_deviceResources->context->swapBuffers();

	return pvr::Result::Success;
}

/// <summary>Updates the memory from where the command buffer will read the values to update the uniforms for the spheres.</summary>
/// <param name="proj">Projection matrix.</param>
/// <param name="view">View matrix.</param>
void OpenGLESParticleSystem::updateSphereProgramUniforms(const glm::mat4& proj, const glm::mat4& view)
{
	for (uint32_t i = 0; i < Configuration::NumberOfSpheres; ++i)
	{
		const glm::vec3& position = Configuration::SpheresData[i].vPosition;
		float radius = Configuration::SpheresData[i].fRadius;
		DrawPass& pass = _passSphere[i];

		const glm::mat4 mModel = glm::translate(position) * glm::scale(glm::vec3(radius, radius, radius));
		pass.modelView = view * mModel;
		pass.modelViewProj = proj * pass.modelView;
		pass.modelViewIT = glm::inverseTranspose(glm::mat3(pass.modelView));
		pass.lightPos = glm::vec3(view * glm::vec4(Configuration::LightPosition, 1.0f));
	}
}

/// <summary>Updates the memory from where the commandbuffer will read the values to update the uniforms for the floor.</summary>
void OpenGLESParticleSystem::updateFloorProgramUniforms()
{
	_viewIT = glm::inverseTranspose(glm::mat3(_viewMtx));
	_lightPos = glm::vec3(_viewMtx * glm::vec4(Configuration::LightPosition, 1.0f));
	_viewProjMtx = _projMtx * _viewMtx;
}

/// <summary>Updates particle positions and attributes, e.g. lifespan, position, velocity etc.
/// Will update the buffer that was "just used" as the Input, as output, so that we can exploit more GPU parallelisation.</summary>
void OpenGLESParticleSystem::updateParticleUniforms()
{
	float dt = static_cast<float>(getFrameTime());

	static float rot_angle = 0.0f;
	rot_angle += dt / 500.0f;
	float el_angle = (sinf(rot_angle / 4.0f) + 1.0f) * 0.2f + 0.2f;

	glm::mat4 rot = glm::rotate(rot_angle, glm::vec3(0.0f, 1.0f, 0.0f));
	glm::mat4 skew = glm::rotate(el_angle, glm::vec3(0.0f, 0.0f, 1.0f));

	_particleConfigData.emitter = Emitter(rot * skew, 1.3f, 1.0f);

	dt *= 0.001f;

	_particleConfigData.dt = dt;
	_particleConfigData.totalTime += dt;

	debugThrowOnApiError("OpenGLESParticleSystem::updateParticleUniforms Enter");

	gl::BindBuffer(GL_UNIFORM_BUFFER, _deviceResources->particleConfigUbo);
	gl::BufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(_particleConfigData), &_particleConfigData);

	debugThrowOnApiError("OpenGLESParticleSystem::updateParticleUniforms Exit");
}

void OpenGLESParticleSystem::renderScene()
{
	debugThrowOnApiError("OpenGLESParticleSystem::renderScene Enter");
	static const pvr::assets::Mesh& mesh = _scene->getMesh(0);
	// Render Spheres
	useSimplePipelineProgramAndSetState();
	gl::BindVertexArray(_deviceResources->sphereVao);

	for (uint32_t i = 0; i < Configuration::NumberOfSpheres; i++)
	{
		gl::UniformMatrix4fv(_deviceResources->programSimple.mvpMatrixLoc, 1, GL_FALSE, glm::value_ptr(_passSphere[i].modelViewProj));
		gl::UniformMatrix4fv(_deviceResources->programSimple.mvMatrixLoc, 1, GL_FALSE, glm::value_ptr(_passSphere[i].modelView));
		gl::UniformMatrix3fv(_deviceResources->programSimple.mvITMatrixLoc, 1, GL_FALSE, glm::value_ptr(_passSphere[i].modelViewIT));
		gl::Uniform3fv(_deviceResources->programSimple.lightPositionLoc, 1, glm::value_ptr(_passSphere[i].lightPos));
		auto gltype = pvr::utils::convertToGles(_scene->getMesh(0).getFaces().getDataType());
		gl::DrawElements(GL_TRIANGLES, mesh.getNumFaces() * 3, gltype, nullptr);
	}

	// Enables depth testing
	useFloorPipelineProgramAndSetState();
	gl::BindVertexArray(_deviceResources->floorVao);
	gl::UniformMatrix4fv(_deviceResources->programFloor.mvpMatrixLoc, 1, GL_FALSE, glm::value_ptr(_viewProjMtx));
	gl::UniformMatrix4fv(_deviceResources->programFloor.mvMatrixLoc, 1, GL_FALSE, glm::value_ptr(_viewMtx));
	gl::UniformMatrix3fv(_deviceResources->programFloor.mvITMatrixLoc, 1, GL_FALSE, glm::value_ptr(_viewIT));
	gl::Uniform3fv(_deviceResources->programFloor.lightPositionLoc, 1, glm::value_ptr(_lightPos));

	gl::DrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	gl::BindVertexArray(0);
	debugThrowOnApiError("OpenGLESParticleSystem::renderScene Exit");
}

void OpenGLESParticleSystem::renderParticles()
{
	debugThrowOnApiError("OpenGLESParticleSystem::renderParticles Enter");
	useParticleRenderingProgramAndSetState();
	gl::BindVertexArray(_deviceResources->particleVaos[_currentBufferIdx]);
	gl::UniformMatrix4fv(_deviceResources->programParticle.mvpMatrixLoc, 1, GL_FALSE, glm::value_ptr(_viewProjMtx));
	gl::DrawArrays(GL_POINTS, 0, _numParticles);
	gl::BindVertexArray(0);
	debugThrowOnApiError("OpenGLESParticleSystem::renderParticles Exit");
}

void OpenGLESParticleSystem::initializeParticles()
{
	_particleArrayData.resize(_numParticles);

	for (uint32_t i = 0; i < _numParticles; ++i)
	{
		_particleArrayData[i].fTimeToLive = pvr::randomrange(0.0f, _particleConfigData.maxLifespan);
		_particleArrayData[i].vPosition.x = 0.0f;
		_particleArrayData[i].vPosition.y = 0.0f;
		_particleArrayData[i].vPosition.z = 1.0f;
		_particleArrayData[i].vVelocity = glm::vec3(0.0f);
	}

	for (uint32_t i = 0; i < NumBuffers; ++i)
	{
		gl::BindBuffer(GL_SHADER_STORAGE_BUFFER, _deviceResources->particleBuffers[i]);
		gl::BufferData(GL_SHADER_STORAGE_BUFFER, sizeof(Particle) * _numParticles, _particleArrayData.data(), GL_DYNAMIC_COPY);
	}
}

/// <summary>This function must be implemented by the user of the shell. It should return the Application class (a class inheriting from pvr::Shell.</summary>
/// <returns>Return a smart pointer to the application class.</returns>
std::unique_ptr<pvr::Shell> pvr::newDemo() { return std::make_unique<OpenGLESParticleSystem>(); }
