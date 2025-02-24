/*!
\brief Shows how to perform a separated Gaussian Blur using a Compute shader and Fragment shader for carrying out the horizontal and vertical passes respectively.
\file OpenGLESGamOfLife.cpp
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/

#include "PVRShell/PVRShell.h"
#include "PVRAssets/PVRAssets.h"
#include "PVRUtils/PVRUtilsGles.h"

// Source and binary shaders
const char FragShaderSrcFile[] = "FragShader_ES3.fsh";
const char VertShaderSrcFile[] = "VertShader_ES3.vsh";
const char CompShaderSrcFile[] = "CompShader_ES3.csh";

const char* attribNames[] = {
	"inVertex",
	"inTexCoord",
};

pvr::utils::VertexBindings_Name vertexBindings[] = {
	{ "POSITION", "inPosition" },
	{ "UV0", "TexCoord" },
};

const uint16_t attribIndices[] = {
	0,
	1,
};

enum BoardConfig
{
	Random = 0,
	Checkerboard,
	SpaceShips,
	NumBoards
};

const char* boardConfigs[BoardConfig::NumBoards] = { "Random", "CheckerBoard", "SpaceShips" };

/// <summary>Class implementing the Shell functions.</summary>
class OpenGLESGameOfLife : public pvr::Shell
{
private:
	struct Framebuffer
	{
		GLuint fbo;
		GLuint renderTex;
		pvr::Rectanglei renderArea;

		Framebuffer() : fbo(0), renderTex(0) {}

		~Framebuffer()
		{
			if (fbo)
			{
				gl::DeleteFramebuffers(1, &fbo);
				fbo = 0;
			}
		}
	};

	struct DeviceResources
	{
		pvr::EglContext context;
		// Fbo
		Framebuffer fbo;

		GLuint maskTexture;

		GLuint boardTextures[2];

		GLuint samplerNearest;
		GLuint samplerLinear;

		GLuint computeProgram;
		GLuint graphicsProgram;

		GLuint graphicsBuffer;

		// UIRenderer used to display text
		pvr::ui::UIRenderer uiRenderer;

		DeviceResources(){};
	};

	std::unique_ptr<DeviceResources> _deviceResources;

	std::vector<unsigned char> board;
	std::vector<unsigned char> petriDish;
	int boardWidth = 0;
	int boardHeight = 0;
	int currentTextureIndex = 0;

	float zoomRatio = 1;
	int zoomLevel = 1;
	std::string zoomRatioUI;
	std::string boardConfigUI;
	int currBoardConfig = 0;
	int generation = 0;

	int boardOffSetX = 0;
	int boardOffSetY = 0;

public:
	OpenGLESGameOfLife() {}

	virtual pvr::Result initApplication();
	virtual pvr::Result initView();
	virtual pvr::Result releaseView();
	virtual pvr::Result quitApplication();
	virtual pvr::Result renderFrame();
	virtual void eventMappedInput(pvr::SimplifiedInput key);

	void createResources();
	void render();
	void renderUI();
	void generateBoardData();
	void createPetriDishEffect();
	unsigned int getPetriDishSize() { return std::max(getHeight(), getWidth()) / 4; }

	const GLuint getCurrentInputTexture() const { return _deviceResources->boardTextures[currentTextureIndex]; };
	const GLuint getCurrentOutputTexture() const { return _deviceResources->boardTextures[currentTextureIndex ^ 1]; };
	void CreateBoardTextures();
	void UpdateBoardTextures();

	void setZoomLevel(int zoomLevel);
	void refreshBoard(bool regenData = false);

	// Pick a bit on the board and set it to either full(default) or empty.
	void setBoardBit(int x, int y, bool bit = true)
	{
		if ((x + boardOffSetX < boardWidth) && (x + boardOffSetX >= 0) && (y + boardOffSetY < boardHeight) && (y + boardOffSetY >= 0))
		{
			int idx = (y + boardOffSetY) * boardWidth + x + boardOffSetX;
			board[idx * 4] = bit * 255;
		}
	}
	// Set an offset for the setBoardBit operation.
	void setBoardBitOffset(int x, int y)
	{
		boardOffSetX = x;
		boardOffSetY = y;
	}
};

/// <summary>Sets the ZoomLevel of the board by calculating the ZoomRatio.</summary>
/// <param name="zoomLvl"> desired level of zoom.</param>
void OpenGLESGameOfLife::setZoomLevel(int zoomLvl)
{
	// Updating the Zoom Ratio based on the current level.
	zoomLevel = zoomLvl;
	zoomRatio = zoomLevel > 0 ? zoomLevel : 1.0f / (-zoomLevel + 2.0f);

	boardWidth = static_cast<int>(getWidth() / zoomRatio);
	boardHeight = static_cast<int>(getHeight() / zoomRatio);
	board.resize(static_cast<size_t>(boardWidth * boardHeight * 4));

	// Updating the Zoom UI Label and Value
	zoomRatioUI = "\nZoom Level : ";

	std::ostringstream oss;
	oss << std::setprecision(2) << zoomRatio;

	zoomRatioUI += oss.str();
}

/// <summary>Resets board texture data and restarts the simulation.</summary>
void OpenGLESGameOfLife::refreshBoard(bool regenData)
{
	generateBoardData();
	if (regenData) { CreateBoardTextures(); }
	else
	{
		UpdateBoardTextures();
	}
}

/// <summary>Creates the textures for the board.</summary>
void OpenGLESGameOfLife::CreateBoardTextures()
{
	pvr::TextureHeader textureheader(pvr::PixelFormat::RGBA_8888(), boardWidth, boardHeight);
	pvr::Texture boardTexture(textureheader, board.data());

	_deviceResources->boardTextures[0] = pvr::utils::textureUpload(boardTexture, false, true).image;
	_deviceResources->boardTextures[1] = pvr::utils::textureUpload(boardTexture, false, true).image;
}

/// <summary>Updates the board texture's data.</summary>
void OpenGLESGameOfLife::UpdateBoardTextures()
{
	pvr::TextureHeader textureheader(pvr::PixelFormat::RGBA_8888(), boardWidth, boardHeight);
	pvr::Texture boardTexture(textureheader, board.data());

	gl::BindTexture(GL_TEXTURE_2D, _deviceResources->boardTextures[0]);
	gl::TexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, boardTexture.getWidth(), boardTexture.getHeight(), GL_RGBA, GL_UNSIGNED_BYTE, boardTexture.getDataPointer());

	gl::BindTexture(GL_TEXTURE_2D, _deviceResources->boardTextures[1]);
	gl::TexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, boardTexture.getWidth(), boardTexture.getHeight(), GL_RGBA, GL_UNSIGNED_BYTE, boardTexture.getDataPointer());
}

/// <summary>Create the Petri dish masking effect.</summary>
void OpenGLESGameOfLife::createPetriDishEffect()
{
	unsigned int petriDishSize = getPetriDishSize();
	// Creating the Petri dish effect.
	petriDish.resize(static_cast<size_t>(petriDishSize * petriDishSize));

	float radius = petriDishSize * .5f;
	for (unsigned int y = 0; y < petriDishSize; y++)
	{
		for (unsigned int x = 0; x < petriDishSize; x++)
		{
			glm::vec2 r((float)(x - radius), (float)(y - radius));

			int i = (y * petriDishSize + x);
			petriDish[i] = (unsigned char)(std::max(0.f, std::min(255.f, (1.2f - glm::length(r) / radius) * 255.f)));
		}
	}

	pvr::TextureHeader textureheaderPetri(pvr::PixelFormat::R_8(), petriDishSize, petriDishSize);
	pvr::Texture petriDishTexture(textureheaderPetri, petriDish.data());

	// Load the texture from disk
	_deviceResources->maskTexture = pvr::utils::textureUpload(petriDishTexture, false, true).image;
}

/// <summary>Generates data as a starting state for the Game Of Life board.</summary>
void OpenGLESGameOfLife::generateBoardData()
{
	generation = 0;

	switch (currBoardConfig)
	{
	// Generates a board with random data.
	default:
	case BoardConfig::Random: {
		// Randomly Fill the board to create a starting state for simulation.
		for (unsigned int i = 0; i < board.size(); i += 4)
		{
			if (pvr::randomrange(0, 1) > .75f) { board[i] = static_cast<unsigned char>(255); }
			else
			{
				board[i] = static_cast<unsigned char>(0);
			}
		}
	}
	break;

	// Generates a Checker board.
	case BoardConfig::Checkerboard: {
		int checkerSize = 5;
		// int checkerSize2 = checkerSize * 2;

		for (unsigned int i = 0; i < board.size(); i += 4)
		{
			int row = (i / 4) / (boardWidth);

			bool rowblack = (row / checkerSize) % 2;
			bool colblack = (((i / 4) % (boardWidth)) / checkerSize) % 2;

			int r = rowblack ^ colblack ? 255 : 0;
			board[i] = static_cast<unsigned char>(r);
		}
	}
	break;

	// Generates a board with Heavyweight spaceship at random positions.
	case BoardConfig::SpaceShips: {
		memset(board.data(), 0, board.size());

		for (int i = 0; i < 200 / zoomRatio; i += 4)
		{
			setBoardBitOffset((int)pvr::randomrange(0.f, (float)boardWidth), (int)pvr::randomrange(0.f, (float)boardHeight));

			if (rand() % 2 == 0)
			{ // HWSS
				setBoardBit(0, 0);
				setBoardBit(0, 1);
				setBoardBit(0, 2);
				setBoardBit(1, 0);
				setBoardBit(1, 3);
				setBoardBit(2, 0);
				setBoardBit(3, 0);
				setBoardBit(3, 4);
				setBoardBit(4, 0);
				setBoardBit(4, 4);
				setBoardBit(5, 0);
				setBoardBit(6, 1);
				setBoardBit(6, 3);
			}
			else
			{
				// HWSS
				setBoardBit(6, 0);
				setBoardBit(6, 1);
				setBoardBit(6, 2);
				setBoardBit(5, 0);
				setBoardBit(5, 3);
				setBoardBit(4, 0);
				setBoardBit(3, 0);
				setBoardBit(3, 4);
				setBoardBit(2, 0);
				setBoardBit(2, 4);
				setBoardBit(1, 0);
				setBoardBit(0, 1);
				setBoardBit(0, 3);
			}
		}

		// Acorn:
		// setBoardBit(0, 0);
		// setBoardBit(1, 0);
		// setBoardBit(1, 2);
		// setBoardBit(3, 1);
		// setBoardBit(4, 0);
		// setBoardBit(5, 0);
		// setBoardBit(6, 0);

		// ?
		// setBoardBit(0, 0, true);
		// setBoardBit(0, 1, true);
		// setBoardBit(0, 2, true);
		// setBoardBit(2, 0, true);
		// setBoardBit(2, 1, true);
		// setBoardBit(3, -1, true);
		// setBoardBit(3, 1, true);
		// setBoardBit(4, -1, true);
		// setBoardBit(4, 1, true);
		// setBoardBit(5, 0, true);

		// R-pentomino
		// setBoardBit(0, 1);
		// setBoardBit(1, 0);
		// setBoardBit(1, 1);
		// setBoardBit(1, 2);
		// setBoardBit(2, 2);

		// B-heptomino
		// setBoardBit(0, 1);
		// setBoardBit(0, 2);
		// setBoardBit(1, 0);
		// setBoardBit(1, 1);
		// setBoardBit(2, 1);
		// setBoardBit(2, 2);
		// setBoardBit(3, 2);

		// Pi-heptomino
		// setBoardBit(0, 0);
		// setBoardBit(0, 1);
		// setBoardBit(0, 2);
		// setBoardBit(1, 2);
		// setBoardBit(2, 0);
		// setBoardBit(2, 1);
		// setBoardBit(2, 2);
	}
	break;
	}
}

/// <summary>Code in  createResources() loads the compute, fragment and vertex shaders and associated buffers used by them. It loads the input texture on which
/// we'll perform the Gaussian blur. It also generates the output texture that will be filled by the compute shader and used by the fragment shader.</ summary>
void OpenGLESGameOfLife::createResources()
{
	// Load the compute shader and create the associated program.
	_deviceResources->computeProgram = pvr::utils::createComputeShaderProgram(*this, CompShaderSrcFile);
	pvr::utils::throwOnGlError("Failed to create compute program.");

	// Enable or disable gamma correction based on if it is automatically performed on the framebuffer or we need to do it in the shader.
	const char* defines[] = { "FRAMEBUFFER_SRGB" };
	uint32_t numDefines = 1;

	if (getBackBufferColorspace() != pvr::ColorSpace::sRGB) { numDefines = 0; }
	// Load the fragment and vertex shaders and create the associated programs.
	_deviceResources->graphicsProgram = pvr::utils::createShaderProgram(*this, VertShaderSrcFile, FragShaderSrcFile, attribNames, attribIndices, 2, defines, numDefines);
	pvr::utils::throwOnGlError("Failed to create graphics program.");

	////////////////////BOARD
	generateBoardData();
	CreateBoardTextures();

	////////////////////PETRI DISH
	createPetriDishEffect();

	gl::BindTexture(GL_TEXTURE_2D, 0);

	gl::GenSamplers(1, &_deviceResources->samplerNearest);
	gl::SamplerParameteri(_deviceResources->samplerNearest, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	gl::SamplerParameteri(_deviceResources->samplerNearest, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	gl::SamplerParameteri(_deviceResources->samplerNearest, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	gl::SamplerParameteri(_deviceResources->samplerNearest, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	gl::SamplerParameteri(_deviceResources->samplerNearest, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	gl::GenSamplers(1, &_deviceResources->samplerLinear);
	gl::SamplerParameteri(_deviceResources->samplerLinear, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	gl::SamplerParameteri(_deviceResources->samplerLinear, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	gl::SamplerParameteri(_deviceResources->samplerLinear, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	gl::SamplerParameteri(_deviceResources->samplerLinear, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	gl::SamplerParameteri(_deviceResources->samplerLinear, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	pvr::utils::throwOnGlError("[OpenGLESGaussianBlur::createResources] Failed to create textures");
}

/// <summary>Performs the actual rendering each frame, first a compute shader is used to perform a horizontal compute based Gaussian blur, after this a
/// fragment shader based vertical Gaussian blur is used.</summary>
void OpenGLESGameOfLife::render()
{
	// We Execute the Compute shader, we bind the input and output texture.

	gl::UseProgram(_deviceResources->computeProgram);

	gl::ActiveTexture(GL_TEXTURE0);
	gl::BindSampler(0, _deviceResources->samplerNearest);
	gl::BindTexture(GL_TEXTURE_2D, getCurrentInputTexture());
	// gl::Uniform1i(0, 0);

	gl::BindImageTexture(1, getCurrentOutputTexture(), 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA8);
	gl::DispatchCompute((boardWidth * 4) / 8, (boardHeight * 4) / 4, 1);

	// Use a memory barrier to ensure memory accesses using shader image load store
	gl::MemoryBarrier(GL_TEXTURE_FETCH_BARRIER_BIT);

	currentTextureIndex = currentTextureIndex == 1 ? 0 : 1;

	// Execute the Graphic program (Vertex and Fragment) and pass the output texture
	gl::UseProgram(_deviceResources->graphicsProgram);

	gl::ActiveTexture(GL_TEXTURE1);
	gl::BindSampler(1, _deviceResources->samplerNearest);
	gl::BindTexture(GL_TEXTURE_2D, _deviceResources->maskTexture);

	gl::ActiveTexture(GL_TEXTURE2);
	gl::BindSampler(2, _deviceResources->samplerLinear);
	gl::BindTexture(GL_TEXTURE_2D, getCurrentInputTexture());

	gl::DrawArrays(GL_TRIANGLE_STRIP, 0, 3);
}

/// <summary>Renders the UI.</summary>
void OpenGLESGameOfLife::renderUI()
{
	std::string uiDescription = "Generation: " + std::to_string(generation);
	uiDescription += boardConfigUI;
	uiDescription += zoomRatioUI;

	_deviceResources->uiRenderer.getDefaultDescription()->setText(uiDescription);
	_deviceResources->uiRenderer.getDefaultDescription()->commitUpdates();

	_deviceResources->uiRenderer.beginRendering();
	_deviceResources->uiRenderer.getSdkLogo()->render();
	_deviceResources->uiRenderer.getDefaultTitle()->render();
	_deviceResources->uiRenderer.getDefaultDescription()->render();
	_deviceResources->uiRenderer.getDefaultControls()->render();
	_deviceResources->uiRenderer.endRendering();
}

/// <summary>Code in initApplication() will be called by Shell once per run, before the rendering context is created.
/// Used to initialize variables that are not dependent on it (e.g. external modules, loading meshes, etc.) If the rendering
/// context is lost, initApplication() will not be called again.</summary>
/// <returns>Result::Success if no error occurred.</returns>
pvr::Result OpenGLESGameOfLife::initApplication()
{
	setZoomLevel(1);
	boardHeight = getHeight();
	boardWidth = getWidth();

	boardConfigUI = "\nBoard Config : ";
	boardConfigUI += boardConfigs[currBoardConfig];

	this->setDepthBitsPerPixel(0);
	this->setStencilBitsPerPixel(0);

	return pvr::Result::Success;
}

/// <summary>Code in initView() will be called by Shell upon initialization or after a change in the rendering context.
/// Used to initialize variables that are dependent on the rendering context(e.g.textures, vertex buffers, etc.)</summary>
/// <returns>Result::Success if no error occurred.</returns>
pvr::Result OpenGLESGameOfLife::initView()
{
	// initialize the device resources object
	_deviceResources = std::make_unique<DeviceResources>();

	// create an OpenGLES context
	_deviceResources->context = pvr::createEglContext();
	_deviceResources->context->init(getWindow(), getDisplay(), getDisplayAttributes(), pvr::Api::OpenGLES31);

	// set up the application for rendering.
	createResources();

	// Set up the FBO to render to screen.
	_deviceResources->fbo.fbo = _deviceResources->context->getOnScreenFbo();
	_deviceResources->fbo.renderArea = pvr::Rectanglei(0, 0, getWidth(), getHeight());

	_deviceResources->uiRenderer.init(getWidth(), getHeight(), isFullScreen(), getBackBufferColorspace() == pvr::ColorSpace::sRGB);

	_deviceResources->uiRenderer.getDefaultTitle()->setText("Game of Life");
	_deviceResources->uiRenderer.getDefaultTitle()->commitUpdates();
	_deviceResources->uiRenderer.getDefaultDescription()->setText("Action 1: Reset Simulation\n"
																  "Up / Down: Zoom In/Out\n"
																  "Left / Right: Change Board Config");
	_deviceResources->uiRenderer.getDefaultDescription()->commitUpdates();

	gl::Disable(GL_DEPTH_TEST);
	gl::CullFace(GL_BACK);
	gl::FrontFace(GL_CCW);

	return pvr::Result::Success;
}

/// <summary>Code in releaseView() will be called by Shell when the application quits or before a change in the rendering context.</summary>
/// <returns>Result::Success if no error occurred.</returns>
pvr::Result OpenGLESGameOfLife::releaseView()
{
	_deviceResources.reset();
	return pvr::Result::Success;
}

/// <summary>Code in quitApplication() will be called by Shell once per run, just before exiting the program.
/// quitApplication() will not be called every time the rendering context is lost, only before application exit.</summary>
/// <returns>Result::Success if no error occurred.</returns>
pvr::Result OpenGLESGameOfLife::quitApplication() { return pvr::Result::Success; }

/// <summary>Main rendering loop function of the program. The shell will call this function every frame</summary>
/// <returns>Result::Success if no error occurred.</summary>
pvr::Result OpenGLESGameOfLife::renderFrame()
{
	debugThrowOnApiError("Frame begin");

	// Setup the Framebuffer for rendering
	gl::BindFramebuffer(GL_DRAW_FRAMEBUFFER, _deviceResources->fbo.fbo);
	gl::Viewport(0, 0, _deviceResources->fbo.renderArea.width, _deviceResources->fbo.renderArea.height);
	gl::Clear(GL_COLOR_BUFFER_BIT);

	render();
	renderUI();
	debugThrowOnApiError("Frame end");

	if (this->shouldTakeScreenshot()) { pvr::utils::takeScreenshot(this->getScreenshotFileName(), this->getWidth(), this->getHeight()); }

	_deviceResources->context->swapBuffers();
	generation++;
	return pvr::Result::Success;
}

/// <summary>Handles user input and updates live variables accordingly.</summary>
/// <param name="key">Input key to handle.</param>
void OpenGLESGameOfLife::eventMappedInput(pvr::SimplifiedInput key)
{
	switch (key)
	{
	case pvr::SimplifiedInput::NONE: break;

	// Switch between Board Configurations.
	case pvr::SimplifiedInput::Left:
	case pvr::SimplifiedInput::Right:
		currBoardConfig += (key == pvr::SimplifiedInput::Right ? 1 : -1);
		if (currBoardConfig >= BoardConfig::NumBoards) { currBoardConfig = 0; }
		else if (currBoardConfig < 0)
		{
			currBoardConfig = BoardConfig::NumBoards - 1;
		}

		boardConfigUI = "\nBoard Config : ";
		boardConfigUI += boardConfigs[currBoardConfig];

		refreshBoard();
		break;

	// Zoom In or Out of the Board.
	case pvr::SimplifiedInput::Up:
	case pvr::SimplifiedInput::Down:
		setZoomLevel(zoomLevel + (key == pvr::SimplifiedInput::Up ? 1 : -1));
		refreshBoard(true);
		break;
	case pvr::SimplifiedInput::ActionClose: exitShell(); break;

	// Refresh the board.
	case pvr::SimplifiedInput::Action1: refreshBoard(); break;
	case pvr::SimplifiedInput::Action2: break;
	case pvr::SimplifiedInput::Action3: break;
	default: break;
	}
}

/// <summary>This function must be implemented by the user of the shell. The user should return its pvr::Shell object defining the behaviour of the application.</summary>
/// <returns>Return a unique ptr to the demo supplied by the user.</returns>
std::unique_ptr<pvr::Shell> pvr::newDemo() { return std::make_unique<OpenGLESGameOfLife>(); }
