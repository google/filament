/*!
\brief Shows how to use the UIRenderer class to draw ASCII/UTF-8 or wide-charUnicode-compliant text in 3D.
\file OpenGLESIntroducingUIRenderer.cpp
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
#include "PVRShell/PVRShell.h"
#include "PVRUtils/PVRUtilsGles.h"

// PVR font files.
const char CentralTextFontFile[] = "arial_36.pvr";
const char CentralTitleFontFile[] = "starjout_60.pvr";
const char CentralTextFile[] = "Text.txt";

namespace FontSize {
enum Enum
{
	n_36,
	n_46,
	n_56,
	Count
};
}

const char* SubTitleFontFiles[FontSize::Count] = {
	"title_36.pvr",
	"title_46.pvr",
	"title_56.pvr",
};

const uint32_t IntroTime = 4000;
const uint32_t IntroFadeTime = 1000;
const uint32_t TitleTime = 4000;
const uint32_t TitleFadeTime = 1000;
const uint32_t TextFadeStart = 300;
const uint32_t TextFadeEnd = 500;

namespace Language {
enum Enum
{
	English,
	German,
	Norwegian,
	Bulgarian,
	Count
};
}

const wchar_t* Titles[Language::Count] = {
	L"IntroducingUIRenderer",
	L"Einf\u00FChrungUIRenderer",
	L"Innf\u00F8ringUIRenderer",
	L"\u0432\u044A\u0432\u0435\u0436\u0434\u0430\u043D\u0435UIRenderer",
};

/// <summary>Class implementing the Shell functions.</summary>
class OpenGLESIntroducingUIRenderer : public pvr::Shell
{
	pvr::EglContext _context;
	// UIRenderer class used to display text
	pvr::ui::UIRenderer _uiRenderer;
	pvr::ui::MatrixGroup _centralTextGroup;
	pvr::ui::Text _centralTitleLine1;
	pvr::ui::Text _centralTitleLine2;
	pvr::ui::Text _titleText1;
	pvr::ui::Text _titleText2;
	std::vector<pvr::ui::Text> _centralTextLines;
	pvr::ui::Image _background;

	glm::mat4 _mvp;

	float _textOffset;
	std::vector<char> _text;
	std::vector<const char*> _textLines;
	Language::Enum _titleLang;
	int32_t _textStartY, _textEndY;

public:
	virtual pvr::Result initApplication();
	virtual pvr::Result initView();
	virtual pvr::Result releaseView();
	virtual pvr::Result quitApplication();
	virtual pvr::Result renderFrame();

	void generateBackgroundTexture(uint32_t screenWidth, uint32_t screenHeight);
	void updateCentralTitle(uint64_t currentTime);
	void updateSubTitle(uint64_t currentTime);
	void updateCentralText();
};

/// <summary>Code in initApplication() will be called by Shell once per run, before the rendering context is created.
/// Used to initialize variables that are not dependent on it(e.g.external modules, loading meshes, etc.).If the rendering
/// context is lost, initApplication() will not be called again.</summary>
/// <returns>Result::Success if no error occurred.</returns>
pvr::Result OpenGLESIntroducingUIRenderer::initApplication()
{
	// Because the C++ standard states that only ASCII characters are valid in compiled code,
	// we are instead using an external resource file which contains all of the _text to be
	// rendered. This allows complete control over the encoding of the resource file which
	// in this case is encoded as UTF-8.
	std::unique_ptr<pvr::Stream> textStream = getAssetStream(CentralTextFile);

	// The following code simply pulls out each line in the resource file and adds it
	// to an array so we can render each line separately. ReadIntoCharBuffer null-terminates the std::string
	// so it is safe to check for null character.
	textStream->readIntoCharBuffer(_text);
	size_t current = 0;
	while (current < _text.size())
	{
		const char* start = _text.data() + current;

		_textLines.push_back(start);
		while (current < _text.size() && _text[current] != '\0' && _text[current] != '\n' && _text[current] != '\r') { ++current; }

		if (current < _text.size() && (_text[current] == '\r')) { _text[current++] = '\0'; }
		// null-term the strings!!!
		if (current < _text.size() && (_text[current] == '\n' || _text[current] == '\0')) { _text[current++] = '\0'; }
	}

	_titleLang = Language::English;
	return pvr::Result::Success;
}

/// <summary>Code in quitApplication() will be called by pvr::Shell once per run, just before exiting the program.</summary>
/// <returns>Result::Success if no error occurred</returns>.
pvr::Result OpenGLESIntroducingUIRenderer::quitApplication() { return pvr::Result::Success; }

/// <summary>Generates a simple background texture procedurally.</summary>
/// <param name="screenWidth ">Screen dimension's width.</param>
/// <param name="screenHeight">Screen dimension's height.</param>
void OpenGLESIntroducingUIRenderer::generateBackgroundTexture(uint32_t screenWidth, uint32_t screenHeight)
{
	// Generate star texture
	uint32_t width = pvr::math::makePowerOfTwoHigh(screenWidth);
	uint32_t height = pvr::math::makePowerOfTwoHigh(screenHeight);

	pvr::TextureHeader hd;
	hd.channelType = pvr::VariableType::UnsignedByteNorm;
	hd.pixelFormat = pvr::GeneratePixelType1<'l', 8>::ID;
	hd.colorSpace = pvr::ColorSpace::lRGB;
	hd.width = width;
	hd.height = height;
	pvr::Texture myTexture(hd);
	unsigned char* textureData = myTexture.getDataPointer();
	memset(textureData, 0, width * height);
	for (uint32_t j = 0; j < height; ++j)
	{
		for (uint32_t i = 0; i < width; ++i)
		{
			if (!(rand() % 200))
			{
				int brightness = rand() % 255;
				textureData[width * j + i] = (unsigned char)glm::clamp(textureData[width * j + i] + brightness, 0, 255);
			}
		}
	}
	_background = _uiRenderer.createImage(myTexture);
}

/// <summary>Load font from the resource used for this example.</summary>
/// <param name="streamManager"> asset provider.</param>
/// <param name="filename"> name of the font file.</param>
/// <param name="uirenderer"> ui::Font creator.</param>
/// <param name="font"> returned font.</param>
inline void loadFontFromResources(pvr::Shell& streamManager, const char* filename, pvr::ui::UIRenderer& uirenderer, pvr::ui::Font& font)
{
	// the AssetStore is unsuitable for loading the font, because it does not keep the actual texture data that we need.
	// The assetStore immediately releases the texture data as soon as it creates the API objects and the texture header.
	// Hence we use texture load.
	std::unique_ptr<pvr::Stream> fontFile = streamManager.getAssetStream(filename);
	pvr::Texture tmpTexture;
	tmpTexture = pvr::textureLoad(*fontFile, pvr::getTextureFormatFromFilename(filename));
	font = uirenderer.createFont(tmpTexture);
}

/// <summary>Code in initView() will be called by Shell upon initialization or after a change
/// in the rendering context. Used to initialize variables that are dependent on the
/// rendering context(e.g.textures, vertex buffers, etc.).</summary>
/// <returns>Result::Success if no error occurred.</returns>
pvr::Result OpenGLESIntroducingUIRenderer::initView()
{
	_context = pvr::createEglContext();
	_context->init(getWindow(), getDisplay(), getDisplayAttributes());
	_uiRenderer.init(getWidth(), getHeight(), isFullScreen(), getBackBufferColorspace() == pvr::ColorSpace::sRGB);

	// The fonts are loaded here using a PVRTool's ResourceFile wrapper. However,
	// it is possible to load the textures in any way that provides access to a pointer
	// to memory, and the size of the file.
	pvr::ui::Font subTitleFont, centralTitleFont, centralTextFont;
	{
		loadFontFromResources(*this, CentralTitleFontFile, _uiRenderer, centralTitleFont);
		loadFontFromResources(*this, CentralTextFontFile, _uiRenderer, centralTextFont);

		// Determine which size title font to use.
		uint32_t screenShortDimension = std::min(getWidth(), getHeight());
		const char* titleFontFileName = NULL;
		if (screenShortDimension >= 720) { titleFontFileName = SubTitleFontFiles[FontSize::n_56]; }
		else if (screenShortDimension >= 640)
		{
			titleFontFileName = SubTitleFontFiles[FontSize::n_46];
		}
		else
		{
			titleFontFileName = SubTitleFontFiles[FontSize::n_36];
		}
		loadFontFromResources(*this, titleFontFileName, _uiRenderer, subTitleFont);
	}

	_centralTextGroup = _uiRenderer.createMatrixGroup();
	_titleText1 = _uiRenderer.createText(subTitleFont);
	_titleText2 = _uiRenderer.createText(subTitleFont);
	_titleText1->setAnchor(pvr::ui::Anchor::TopLeft, -.98f, .98f);
	_titleText2->setAnchor(pvr::ui::Anchor::TopLeft, -.98f, .98f);

	for (std::vector<const char*>::const_iterator it = _textLines.begin(); it != _textLines.end(); ++it)
	{
		_centralTextLines.push_back(_uiRenderer.createText(*it, centralTextFont));
		_centralTextGroup->add(_centralTextLines.back());
	}

	_centralTitleLine1 = _uiRenderer.createText("introducing", centralTitleFont);
	_centralTitleLine2 = _uiRenderer.createText("uirenderer", centralTitleFont);

	_centralTitleLine1->setAnchor(pvr::ui::Anchor::BottomCenter, glm::vec2(.0f, .0f));
	_centralTitleLine2->setAnchor(pvr::ui::Anchor::TopCenter, glm::vec2(.0f, .0f));

	// Generate background texture
	generateBackgroundTexture(getWidth(), getHeight());
	_textStartY = static_cast<int32_t>(-_uiRenderer.getRenderingDimY() - _centralTextGroup->getDimensions().y);
	float linesSize = _centralTextLines.size() * _centralTextLines[0]->getDimensions().y;
	_textEndY = static_cast<int32_t>(_uiRenderer.getRenderingDimY() + linesSize * .5f);
	_textOffset = static_cast<float>(_textStartY);

	gl::ClearColor(0.f, 0.f, 0.f, 1.f);
	return pvr::Result::Success;
}

/// <summary>Code in releaseView() will be called by Shell when the application quits.</summary>
/// <returns>Result::Success if no error occurred.</returns>
pvr::Result OpenGLESIntroducingUIRenderer::releaseView()
{
	// Release uiRenderer Textures
	_centralTextLines.clear();
	_centralTitleLine1.reset();
	_centralTitleLine2.reset();
	_titleText1.reset();
	_titleText2.reset();
	_centralTextGroup.reset();
	_background.reset();
	_uiRenderer.release();
	_context.reset();

	return pvr::Result::Success;
}

/// <summary>Main rendering loop function of the program. The shell will call this function every frame.</summary>
/// <returns>Result::Success if no error occurred.</returns>
pvr::Result OpenGLESIntroducingUIRenderer::renderFrame()
{
	gl::Clear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	// Clears the colour and depth buffer
	uint64_t currentTime = this->getTime() - this->getTimeAtInitApplication();

	updateSubTitle(currentTime);

	_uiRenderer.beginRendering();
	_background->render();
	// Render the 'IntroducingUIRenderer' title for the first n seconds.
	if (currentTime < IntroTime)
	{
		updateCentralTitle(currentTime);

		// This is the difference
		_centralTitleLine1->render();
		_centralTitleLine2->render();
	}
	// Render the 3D text.
	else
	{
		updateCentralText();
		// Tells uiRenderer to do all the pending text rendering now
		_centralTextGroup->render();
		// Tells uiRenderer to do all the pending text rendering now
	}

	if (_titleText1->getColor().a > 0.0f) { _titleText1->render(); }
	if (_titleText2->getColor().a > 0.0f) { _titleText2->render(); }
	_uiRenderer.getSdkLogo()->render();
	_uiRenderer.endRendering();

	if (this->shouldTakeScreenshot()) { pvr::utils::takeScreenshot(this->getScreenshotFileName(), this->getWidth(), this->getHeight()); }

	_context->swapBuffers();
	return pvr::Result::Success;
}

/// <summary>Update the description sprite.</summary>
/// <param name="currentTime">Current Time.</returns>
void OpenGLESIntroducingUIRenderer::updateSubTitle(uint64_t currentTime)
{
	// Fade effect
	static uint32_t prevLang = -1;
	uint32_t titleLang = static_cast<uint32_t>((currentTime / 1000) / (TitleTime / 1000)) % Language::Count;

	uint32_t nextLang = (titleLang + 1) % Language::Count;
	uint32_t modTime = static_cast<uint32_t>(currentTime) % TitleTime;
	float titlePerc = 1.0f;
	float nextPerc = 0.0f;
	if (modTime > TitleTime - TitleFadeTime)
	{
		titlePerc = 1.0f - ((modTime - (TitleTime - TitleFadeTime)) / static_cast<float>(TitleFadeTime));
		nextPerc = 1.0f - titlePerc;
	}

	const glm::vec4 titleCol = glm::vec4(1.0f, 1.f, 1.f, titlePerc);
	const glm::vec4 nextCol = glm::vec4(1.0f, 1.f, 1.f, nextPerc);

	// Here we are passing in a wide-character std::string to uiRenderer function. This allows
	// Unicode to be compiled in to std::string-constants, which this code snippet demonstrates.
	// Because we are not setting a projection or a model-view matrix the default projection
	// matrix is used.
	if (titleLang != prevLang)
	{
		_titleText1->setText(Titles[titleLang]);
		_titleText2->setText(Titles[nextLang]);
		prevLang = titleLang;
	}
	_titleText1->setColor(titleCol);
	_titleText2->setColor(nextCol);

	_titleText1->commitUpdates();
	_titleText2->commitUpdates();
}

/// <summary>Draws the title _text.</summary>
/// <param name="fadeAmount">Amount of fade.</param>
void OpenGLESIntroducingUIRenderer::updateCentralTitle(uint64_t currentTime)
{
	// Using the MeasureText() method provided by uiRenderer, we can determine the bounding-box
	// size of a std::string of text. This can be useful for justify text centrally, as we are
	// doing here.
	float fadeAmount = 1.0f;

	// Fade in
	if (currentTime < IntroFadeTime) { fadeAmount = currentTime / (float)IntroFadeTime; }
	// Fade out
	else if (currentTime > IntroTime - IntroFadeTime)
	{
		fadeAmount = 1.0f - ((currentTime - (IntroTime - IntroFadeTime)) / (float)IntroFadeTime);
	}
	// Editing the _text's alpha based on the fade amount.
	_centralTitleLine1->setColor(1.f, 1.f, 0.f, fadeAmount);
	_centralTitleLine2->setColor(1.f, 1.f, 0.f, fadeAmount);
	_centralTitleLine1->commitUpdates();
	_centralTitleLine2->commitUpdates();
}

/// <summary>Draws the 3D _text and scrolls in to the screen.</summary>
void OpenGLESIntroducingUIRenderer::updateCentralText()
{
	glm::mat4 mProjection = glm::mat4(1.0f);

	mProjection =
		pvr::math::perspectiveFov(pvr::Api::OpenGLES31, 0.7f, static_cast<float>(_uiRenderer.getRenderingDimX()), static_cast<float>(_uiRenderer.getRenderingDimY()), 1.0f, 2000.0f);

	const glm::mat4 mCamera = glm::lookAt(glm::vec3(_uiRenderer.getRenderingDimX() * .5f, -_uiRenderer.getRenderingDimY(), 700.0f),
		glm::vec3(_uiRenderer.getRenderingDimX() * .5f, 0, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	_mvp = mProjection * mCamera;

	float lineSpacingNDC = 1.6f * _centralTextLines[0]->getFont()->getFontLineSpacing() / static_cast<float>(getHeight());

	// Calculate the FPS scale.
	float fFPSScale = float(getFrameTime()) * 60 / 1000;

	// Move the _text. Progressively speed up.
	float fSpeedInc = 0.0f;
	if (_textOffset > 0.0f) { fSpeedInc = _textOffset / _textEndY; }
	_textOffset += (0.75f + (1.0f * fSpeedInc)) * fFPSScale;
	if (_textOffset > static_cast<float>(_textEndY)) { _textOffset = static_cast<float>(_textStartY); }

	glm::mat4 trans = glm::translate(glm::vec3(0.0f, _textOffset, 0.0f));

	// uiRenderer can optionally be provided with user-defined projection and model-view matrices
	// which allow custom layout of text. Here we are proving both a projection and model-view
	// matrix. The projection matrix specified here uses perspective projection which will
	// provide the 3D effect. The model-view matrix positions the text in world space
	// providing the 'camera' position and the scrolling of the text.

	_centralTextGroup->setScaleRotateTranslate(trans);
	_centralTextGroup->setViewProjection(_mvp);

	// The previous method (renderTitle()) explains the following functions in more detail
	// however put simply, we are looping the entire array of loaded text which is encoded
	// in UTF-8. uiRenderer batches this internally and the call to Flush() will render the
	// _text to the frame buffer. We are also fading out the text over a certain distance.
	float pos, fade;
	for (uint32_t uiIndex = 0; uiIndex < _textLines.size(); ++uiIndex)
	{
		pos = (_textOffset - (uiIndex * 36.0f));
		fade = 1.0f;
		glm::vec4 color(1.0f, 1.0f, 0.0f, 1.0f);
		if (pos > TextFadeStart)
		{
			fade = glm::clamp(1.0f - ((pos - TextFadeStart) / (TextFadeEnd - TextFadeStart)), 0.0f, 1.0f);
			color.a *= fade;
		}
		_centralTextLines[uiIndex]->setColor(color);
		_centralTextLines[uiIndex]->setAnchor(pvr::ui::Anchor::Center, glm::vec2(0.f, -(uiIndex * lineSpacingNDC)));
	}
	_centralTextGroup->commitUpdates();
}

/// <summary>This function must be implemented by the user of the shell. The user should return its pvr::Shell object defining the behaviour of the application.</summary>
/// <returns>Return a unique ptr to the demo supplied by the user.</returns>
std::unique_ptr<pvr::Shell> pvr::newDemo() { return std::make_unique<OpenGLESIntroducingUIRenderer>(); }
