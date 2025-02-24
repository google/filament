/*!
\brief Contains implementations of functions for the classes in UIRendererGles.h
\file PVRUtils/OpenGLES/UIRendererGles.h
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
#pragma once
#include "PVRUtils/OpenGLES/SpriteGles.h"
#include "PVRCore/texture/Texture.h"
#include "PVRCore/math/MathUtils.h"

namespace pvr {
namespace ui {

/// <summary>The GLState structure is designed and used to hold the current OpenGL ES state. The UIRenderer then makes use of this structure
/// to efficiently and as optimally as possibly only change the state which is necessary to be changed when carrying out rendering commands.</summary>
struct GLState
{
	/// <summary>The active program.</summary>
	GLint activeProgram;
	/// <summary>The active texture unit.</summary>
	GLint activeTextureUnit;
	/// <summary>The currently bound texture.</summary>
	GLint boundTexture;
	/// <summary>Specifies whether blending is enabled.</summary>
	GLint blendEnabled;
	/// <summary>Specifies the source rgb blending mode.</summary>
	GLint blendSrcRgb;
	/// <summary>Specifies the source alpha blending mode.</summary>
	GLint blendSrcAlpha;
	/// <summary>Specifies the destination rgb blending mode.</summary>
	GLint blendDstRgb;
	/// <summary>Specifies the destination alpha blending mode.</summary>
	GLint blendDstAlpha;
	/// <summary>Specifies the blending equation to use for rgb blending.</summary>
	GLint blendEqationRgb;
	/// <summary>Specifies the blending equation to use for alpha blending.</summary>
	GLint blendEqationAlpha;
	/// <summary>Specifies the color mask.</summary>
	GLboolean colorMask[4];
	/// <summary>Specifies whether depth testing is enabled.</summary>
	GLint depthTest;
	/// <summary>Specifies the depth mask.</summary>
	GLint depthMask;
	/// <summary>Specifies whether stencil testing is enabled.</summary>
	GLint stencilTest;
	/// <summary>Specifies whether culling is enabled.</summary>
	GLint cullingEnabled;
	/// <summary>Specifies the culling mode.</summary>
	GLint culling;
	/// <summary>Specifies the winding order.</summary>
	GLint windingOrder;
	/// <summary>Specifies the sampler used at binding index 7.</summary>
	GLint sampler7;
	/// <summary>Specifies the vertex buffer object.</summary>
	GLint vbo;
	/// <summary>Specifies the index buffer object.</summary>
	GLint ibo;
	/// <summary>Specifies the vertex array object.</summary>
	GLint vao;
	/// <summary>Specifies a list of vertex array bindings.</summary>
	std::vector<bool> vertexAttribArray;
	/// <summary>Specifies a list of vertex attribute bindings.</summary>
	std::vector<GLint> vertexAttribBindings;
	/// <summary>Specifies a list of vertex attribute binding sizes.</summary>
	std::vector<GLint> vertexAttribSizes;
	/// <summary>Specifies a list of vertex attribute binding types.</summary>
	std::vector<GLint> vertexAttribTypes;
	/// <summary>Specifies a list of vertex attribute bindings and whether they are normalized.</summary>
	std::vector<GLint> vertexAttribNormalized;
	/// <summary>Specifies a list of vertex attribute binding strides.</summary>
	std::vector<GLint> vertexAttribStride;
	/// <summary>Specifies a list of vertex attribute binding offsets.</summary>
	std::vector<GLvoid*> vertexAttribOffset;

	/// <summary>Stores the current OpenGL ES state so that it can be modified and reset without issues.</summary>
	/// <param name="api">Specifies the OpenGL ES version supported.</param>
	void storeCurrentGlState(Api api);

	/// <summary>Default constructor for GLState which initialises the OpenGL ES states for the GLState structure.</summary>
	GLState()
		: activeProgram(-1), activeTextureUnit(-1), boundTexture(-1), blendDstAlpha(GL_ONE), blendDstRgb(GL_ONE_MINUS_SRC_ALPHA), blendEnabled(GL_TRUE),
		  blendEqationAlpha(GL_FUNC_ADD), blendEqationRgb(GL_FUNC_ADD), blendSrcAlpha(GL_ZERO), blendSrcRgb(GL_SRC_ALPHA), depthMask(GL_FALSE), depthTest(GL_FALSE),
		  stencilTest(GL_FALSE), cullingEnabled(GL_FALSE), culling(GL_BACK), windingOrder(GL_CCW), ibo(-1), sampler7(0), vbo(-1), vao(-1), vertexAttribArray(8, GL_FALSE),
		  vertexAttribBindings(8, -1), vertexAttribSizes(8, -1), vertexAttribTypes(8, -1), vertexAttribNormalized(8, -1), vertexAttribStride(8, -1),
		  vertexAttribOffset(8, static_cast<GLvoid*>(nullptr))
	{
		colorMask[0] = GL_TRUE;
		colorMask[1] = GL_TRUE;
		colorMask[2] = GL_TRUE;
		colorMask[3] = GL_TRUE;
	}
};

/// <summary>The GLStateTracker structure extends the functionality set out by the GLState structure to additionally help in determining what state has been changed.
/// The UIRenderer then makes use of this structure to check what state must be changed, set UIRenderer state appropriately and reset the state after the UIRenderer
/// has finished its rendering.</summary>
struct GLStateTracker : public GLState
{
	/// <summary>Specifies whther the current program has changed.</summary>
	bool activeProgramChanged;
	/// <summary>Specifies whther the active texture unit has changed.</summary>
	bool activeTextureUnitChanged;
	/// <summary>Specifies whther the bound texture has changed.</summary>
	bool boundTextureChanged;
	/// <summary>Specifies whether the enabling/disabling of blending has changed.</summary>
	bool blendEnabledChanged;
	/// <summary>Specifies whether the source rgb blending mode has changed.</summary>
	bool blendSrcRgbChanged;
	/// <summary>Specifies whether the source alpha blending mode has changed.</summary>
	bool blendSrcAlphaChanged;
	/// <summary>Specifies whether the destination rgb blending mode has changed.</summary>
	bool blendDstRgbChanged;
	/// <summary>Specifies whether the destination alpha blending mode has changed.</summary>
	bool blendDstAlphaChanged;
	/// <summary>Specifies whether the blending equation for rgb blending has changed.</summary>
	bool blendEqationRgbChanged;
	/// <summary>Specifies whether the blending equation for alpha blending has changed.</summary>
	bool blendEqationAlphaChanged;
	/// <summary>Specifies whether the color mask has changed.</summary>
	bool colorMaskChanged;
	/// <summary>Specifies whether depth testing has changed.</summary>
	bool depthTestChanged;
	/// <summary>Specifies whether the depth mask has changed.</summary>
	bool depthMaskChanged;
	/// <summary>Specifies whether stencil testing state has changed.</summary>
	bool stencilTestChanged;
	/// <summary>Specifies whether culling state has changed.</summary>
	bool cullingEnabledChanged;
	/// <summary>Specifies whether the culling mode has changed.</summary>
	bool cullingChanged;
	/// <summary>Specifies whether the winding order has changed.</summary>
	bool windingOrderChanged;
	/// <summary>Specifies whether the sampler at binding index 7 has changed.</summary>
	bool sampler7Changed;
	/// <summary>Specifies whether the vertex buffer object has changed.</summary>
	bool vboChanged;
	/// <summary>Specifies whether the index buffer object has changed.</summary>
	bool iboChanged;
	/// <summary>Specifies whether the vertex array object has changed.</summary>
	bool vaoChanged;
	/// <summary>Specifies whether the vertex attribute array states have changed.</summary>
	std::vector<bool> vertexAttribArrayChanged;
	/// <summary>Specifies whether the vertex attribute pointer states have changed.</summary>
	std::vector<bool> vertexAttribPointerChanged;

	/// <summary>Sets the OpenGL ES state based on what state must be changed from the current OpenGL ES state to the states required for rendering the UIRenderer and its
	/// sprites.</summary>
	/// <param name="api">Specifies the API type/version used.</param>
	void setUiState(Api api);

	/// <summary>Checks what OpenGL ES state has been changed based on differences between the current GLState object and the currentGlState structure.</summary>
	/// <param name="currentGlState">Specifies whether OpenGL ES state which should be checked for differences with the current GLState object to determine what state has been
	/// modified.</param>
	void checkStateChanged(const GLState& currentGlState);

	/// <summary>Uses the provided GLStateTracker to blindly determine what state has been changed. Usage of this function requires full tracking of the OpenGL ES state as no state
	/// will be checked but will be taken as fact from stateTracker.</summary> <param name="stateTracker">Specifies what OpenGL ES state has been changed.</param>
	void checkStateChanged(const GLStateTracker& stateTracker);

	/// <summary>Restores the OpenGL ES state back to what was stored prior to UIRenderer rendering.</summary>
	/// <param name="currentGlState">The OpenGL ES state to use for restoring.</param>
	/// <param name="api">Specifies the OpenGL ES version used.</param>
	void restoreState(const GLState& currentGlState, Api api);

	/// <summary>Default constructor for GLStateTracker which initialises the OpenGL ES state tracking.</summary>
	GLStateTracker()
		: activeProgramChanged(GL_FALSE), activeTextureUnitChanged(GL_FALSE), boundTextureChanged(GL_FALSE), blendEnabledChanged(GL_FALSE), blendSrcRgbChanged(GL_FALSE),
		  blendDstRgbChanged(GL_FALSE), blendDstAlphaChanged(GL_FALSE), blendEqationRgbChanged(GL_FALSE), blendEqationAlphaChanged(GL_FALSE), colorMaskChanged(GL_FALSE),
		  depthTestChanged(GL_FALSE), depthMaskChanged(GL_FALSE), stencilTestChanged(GL_FALSE), cullingEnabledChanged(GL_FALSE), cullingChanged(GL_FALSE),
		  windingOrderChanged(GL_FALSE), sampler7Changed(GL_FALSE), vboChanged(GL_FALSE), iboChanged(GL_FALSE), vaoChanged(GL_FALSE), vertexAttribArrayChanged(8, GL_FALSE),
		  vertexAttribPointerChanged(8, GL_FALSE)
	{}
};

/// <summary>Manages and render the sprites.</summary>
class UIRenderer
{
public:
	/// <summary>Information used for uploading required info to the shaders (matrices, attributes etc).</summary>
	struct ProgramData
	{
		/// <summary>Uniform index information.</summary>
		enum Uniform
		{
			UniformMVPmtx,
			UniformFontTexture,
			UniformColor,
			UniformAlphaMode,
			UniformUVmtx,
			NumUniform
		};
		/// <summary>Attribute index information.</summary>
		enum Attribute
		{
			AttributeVertex,
			AttributeUV,
			NumAttribute
		};
		/// <summary>An array of uniforms used by the UIRenderer.</summary>
		int32_t uniforms[NumUniform];
	};

	/// <summary>Retrieves the Font index buffer.</summary>
	/// <returns>The OpenGL ES handle for the Font index buffer.</returns>
	const GLuint& getFontIbo()
	{
		if (!_fontIboCreated)
		{
			// create the FontIBO
			std::vector<uint16_t> fontFaces;
			fontFaces.resize(impl::Font_::FontElement);

			for (uint32_t i = 0; i < impl::Font_::MaxRenderableLetters; ++i)
			{
				fontFaces[i * 6u] = static_cast<uint16_t>(0u + i * 4u);
				fontFaces[i * 6u + 1u] = static_cast<uint16_t>(3u + i * 4u);
				fontFaces[i * 6u + 2u] = static_cast<uint16_t>(1u + i * 4u);

				fontFaces[i * 6u + 3u] = static_cast<uint16_t>(3u + i * 4u);
				fontFaces[i * 6u + 4u] = static_cast<uint16_t>(0u + i * 4u);
				fontFaces[i * 6u + 5u] = static_cast<uint16_t>(2u + i * 4u);
			}
			GLint binding;
			gl::GetIntegerv(GL_ELEMENT_ARRAY_BUFFER_BINDING, &binding);

			gl::GenBuffers(1, &_fontIbo);
			gl::BindBuffer(GL_ELEMENT_ARRAY_BUFFER, _fontIbo);
			gl::BufferData(GL_ELEMENT_ARRAY_BUFFER, static_cast<uint32_t>(sizeof(fontFaces[0]) * fontFaces.size()), &fontFaces[0], GL_STATIC_DRAW);
			gl::BindBuffer(GL_ELEMENT_ARRAY_BUFFER, binding);

			_uiStateTracker.ibo = _fontIbo;
			_fontIboCreated = true;
		}
		return _fontIbo;
	}

	/// <summary>Retrieves the Image vertex buffer.</summary>
	/// <returns>The OpenGL ES handle for the Image vertex buffer.</returns>
	const GLuint& getImageVbo()
	{
		if (!_imageVboCreated)
		{
			// create the image vbo
			const float verts[] = {
				/*    Position  */
				-1.f, 1.f, 0.f, 1.0f, 0.0f, 1.0f, // upper left
				-1.f, -1.f, 0.f, 1.0f, 0.f, 0.0f, // lower left
				1.f, 1.f, 0.f, 1.0f, 1.f, 1.f, // upper right
				-1.f, -1.f, 0.f, 1.0f, 0.f, 0.0f, // lower left
				1.f, -1.f, 0.f, 1.0f, 1.f, 0.0f, // lower right
				1.f, 1.f, 0.f, 1.0f, 1.f, 1.f, // upper right
			};
			GLint binding;
			gl::GetIntegerv(GL_ARRAY_BUFFER_BINDING, &binding);
			gl::GenBuffers(1, &_imageVbo);
			gl::BindBuffer(GL_ARRAY_BUFFER, _imageVbo);
			gl::BufferData(GL_ARRAY_BUFFER, static_cast<uint32_t>(sizeof(verts)), static_cast<const void*>(verts), GL_STATIC_DRAW);
			gl::BindBuffer(GL_ARRAY_BUFFER, binding);

			_uiStateTracker.vbo = _imageVbo;
			_imageVboCreated = true;
		}
		return _imageVbo;
	}

	/// <summary>Constructor. Does not produce a ready-to-use object, use the init function before use.</summary>
	UIRenderer() : _screenRotation(.0f), _program(0), _fontIboCreated(false), _imageVboCreated(false), _samplerBilinear(0), _samplerTrilinear(0), _fontIbo(-1), _imageVbo(-1) {}

	/// <summary>Move Constructor. Does not produce a ready-to-use object, use the init function before use.</summary>
	/// <param name="rhs">Another UIRenderer to initiialise from.</param>
	UIRenderer(UIRenderer&& rhs)
		: _program(std::move(rhs._program)), _programData(std::move(rhs._programData)), _defaultFont(std::move(rhs._defaultFont)), _sdkLogo(std::move(rhs._sdkLogo)),
		  _defaultTitle(std::move(rhs._defaultTitle)), _defaultDescription(std::move(rhs._defaultDescription)), _defaultControls(std::move(rhs._defaultControls)),
		  _samplerBilinear(std::move(rhs._samplerBilinear)), _samplerTrilinear(std::move(rhs._samplerTrilinear)), _fontIbo(std::move(rhs._fontIbo)),
		  _imageVbo(std::move(rhs._imageVbo)), _screenDimensions(std::move(rhs._screenDimensions)), _screenRotation(std::move(rhs._screenRotation)),
		  _groupId(std::move(rhs._groupId)), _sprites(std::move(rhs._sprites)), _textElements(std::move(rhs._textElements)), _fonts(std::move(rhs._fonts)),
		  _fontIboCreated(std::move(rhs._fontIboCreated)), _imageVboCreated(std::move(rhs._imageVboCreated))
	{
		_program = std::move(rhs._program);
		rhs._program = 0;
		updateResourceOwnership();
	}

	/// <summary>Assignment Operator overload. Does not produce a ready-to-use object, use the init function before use.</summary>
	/// <param name="rhs">Another UIRenderer to initiialise from.</param>
	/// <returns>this object (allow chaining commands with ->)</returns>
	UIRenderer& operator=(UIRenderer&& rhs)
	{
		if (this == &rhs) { return *this; }
		_program = std::move(rhs._program);
		rhs._program = 0;
		_programData = std::move(rhs._programData);
		_defaultFont = std::move(rhs._defaultFont);
		_sdkLogo = std::move(rhs._sdkLogo);
		_defaultTitle = std::move(rhs._defaultTitle);
		_defaultDescription = std::move(rhs._defaultDescription);
		_defaultControls = std::move(rhs._defaultControls);
		_samplerBilinear = std::move(rhs._samplerBilinear);
		_samplerTrilinear = std::move(rhs._samplerTrilinear);
		_fontIbo = std::move(rhs._fontIbo);
		_imageVbo = std::move(rhs._imageVbo);
		_screenDimensions = std::move(rhs._screenDimensions);
		_screenRotation = std::move(rhs._screenRotation);
		_groupId = std::move(rhs._groupId);
		_fontIboCreated = std::move(rhs._fontIboCreated);
		_imageVboCreated = std::move(rhs._imageVboCreated);
		updateResourceOwnership();
		return *this;
	}

	//!\cond NO_DOXYGEN
	UIRenderer& operator=(const UIRenderer& rhs) = delete;
	UIRenderer(UIRenderer& rhs) = delete;
	//!\endcond

	/// <summary>Destructor for the UIRenderer which will release all resources currently in use.</summary>
	~UIRenderer() { release(); }

	/// <summary>Returns the ProgramData used by this UIRenderer.</summary>
	/// <returns>The ProgramData structure used by the UIRenderer.</returns>
	const ProgramData& getProgramData() { return _programData; }

	/// <summary>Initialize the UIRenderer with window dimensions. MUST BE called exactly once before use, after a valid
	/// graphics context is available (usually, during initView).</summary>
	/// <param name="width">The width of the screen used for rendering.</param>
	/// <param name="height">The height of the screen used for rendering</param>
	/// <param name="fullscreen">Indicates whether the rendering is occuring in full screen mode.</param>
	/// <param name="isFrameBufferSRGB">Indicates whether the rendering in to SRGB</param>
	/// <returns>True indicating the result of initialising the UIRenderer was successful otherwise False.</returns>
	void init(uint32_t width, uint32_t height, bool fullscreen, bool isFrameBufferSRGB);

	/// <summary>Release the UIRenderer and its resources. Must be called once after we are done with the UIRenderer.
	/// (usually, during releaseView).</summary>
	void release()
	{
		_defaultFont.reset();
		_defaultTitle.reset();
		_defaultDescription.reset();
		_defaultControls.reset();
		_sdkLogo.reset();

		_sprites.clear();
		_fonts.clear();
		_textElements.clear();

		if (_fontIboCreated && _fontIbo != static_cast<uint32_t>(-1)) { gl::DeleteBuffers(1, &_fontIbo); }
		if (_imageVboCreated && _imageVbo != static_cast<uint32_t>(-1)) { gl::DeleteBuffers(1, &_imageVbo); }
		if (_api != Api::OpenGLES2)
		{
			if (_samplerBilinear) { gl::DeleteSamplers(1, &_samplerBilinear); }
			if (_samplerTrilinear) { gl::DeleteSamplers(1, &_samplerTrilinear); }
		}

		_screenRotation = .0f;
		_program = 0;
		_fontIboCreated = false;
		_imageVboCreated = false;
		_samplerBilinear = static_cast<GLuint>(0);
		_samplerTrilinear = static_cast<GLuint>(0);
		_fontIbo = static_cast<GLuint>(-1);
		_imageVbo = static_cast<GLuint>(-1);
	}

	/// <summary>Create a Text sprite. Initialize with std::string. Uses default font.</summary>
	/// <param name="text">std::string object that this Text object will be initialized with</param>
	/// <returns>Text Element framework object, Null framework object if failed.</returns>
	TextElement createTextElement(const std::string& text = "") { return createTextElement(text, _defaultFont); }

	/// <summary>Create Text sprite from std::string and pvr::ui::Font.</summary>
	/// <param name="text">String object that this Text object will be initialized with</param>
	/// <param name="font">The font that the text will be using. The font must belong to the same UIrenderer object.</param>
	/// <returns>Text framework object, Null framework object if failed.</returns>
	TextElement createTextElement(const std::string& text, const Font& font);

	/// <summary>Create a Text Element sprite from a pvr::ui::Font. A default string will be used</summary>
	/// <param name="font">The font that the text element will be using. The font must belong to the same UIrenderer object.</param>
	/// <returns>Text framework object, Null framework object if failed.</returns>
	TextElement createTextElement(const Font& font) { return createTextElement(std::string(""), font); }

	/// <summary>Create Text sprite from wide std::string. Uses the Default Font.</summary>
	/// <param name="text">Wide std::string that this Text object will be initialized with. Will use the Default Font.</param>
	/// <returns>Text framework object, Null framework object if failed.</returns>
	TextElement createTextElement(const std::wstring& text) { return createTextElement(text, _defaultFont); }

	/// <summary>Create Text sprite from wide std::wstring and a pvr::ui::Font.</summary>
	/// <param name="text">text to be rendered.</param>
	/// <param name="font">The font that the text will be using. The font must belong to the same UIrenderer object.</param>
	/// <returns>Text framework object, Null framework object if failed.</returns>
	TextElement createTextElement(const std::wstring& text, const Font& font);

	/// <summary>Create a Text sprite from a TextElement.</summary>
	/// <param name="textElement">text element to initialise a Text framework object from.</param>
	/// <returns>Text framework object, Null framework object if failed.</returns>
	Text createText(const TextElement& textElement);

	/// <summary>Create a Text sprite. Initialize with std::string. Uses default font.</summary>
	/// <param name="text">std::string object that this Text object will be initialized with</param>
	/// <returns>Text framework object, Null framework object if failed.</returns>
	Text createText(const std::string& text = "") { return createText(createTextElement(text)); }

	/// <summary>Create Text sprite from std::string.</summary>
	/// <param name="text">String object that this Text object will be initialized with</param>
	/// <param name="font">The font that the text will be using. The font must belong to the same UIrenderer object.</param>
	/// <returns>Text framework object, Null framework object if failed.</returns>
	Text createText(const std::string& text, const Font& font) { return createText(createTextElement(text, font)); }

	/// <summary>Create a Text sprite from a pvr::ui::Font. Uses a default text string</summary>
	/// <param name="font">The font that the text will be using. The font must belong to the same UIrenderer object.</param>
	/// <returns>Text framework object, Null framework object if failed.</returns>
	Text createText(const Font& font) { return createText(createTextElement(font)); }

	/// <summary>Create Text sprite from wide std::wstring. Uses the Default Font.</summary>
	/// <param name="text">Wide std::string that this Text object will be initialized with. Will use the Default Font.</param>
	/// <returns>Text framework object, Null framework object if failed.</returns>
	Text createText(const std::wstring& text) { return createText(createTextElement(text)); }

	/// <summary>Create Text sprite from wide std::string.</summary>
	/// <param name="text">text to be rendered.</param>
	/// <param name="font">The font that the text will be using. The font must belong to the same UIrenderer object.</param>
	/// <returns>Text framework object, Null framework object if failed.</returns>
	Text createText(const std::wstring& text, const Font& font) { return createText(createTextElement(text, font)); }

	/// <summary>Get the X dimension of the rectangle the UIRenderer is rendering to in order to scale UI elements.
	/// Initial value is the Screen Width.</summary>
	/// <returns>Render width of the rectangle the UIRenderer is using for rendering.</returns>
	float getRenderingDimX() const { return _screenDimensions.x; }

	/// <summary>Get the Y dimension of the rectangle the UIRenderer is rendering to in order to scale UI elements.
	/// Initial value is the Screen Height.</summary>
	/// <returns>Render height of the rectangle the UIRenderer is using for rendering.</returns>
	float getRenderingDimY() const { return _screenDimensions.y; }

	/// <summary>Get the rendering dimensions of the rectangle the UIRenderer is rendering to in order to scale UI elements.
	/// Initial value is the Screen Width and Screen Height.</summary>
	/// <returns>Render width and height of the rectangle the UIRenderer is using for rendering.</returns>
	glm::vec2 getRenderingDim() const { return _screenDimensions; }

	/// <summary>Get the viewport of the rectangle the UIRenderer is rendering to. Initial value is the Screen Width and Screen Height.</summary>
	/// <returns>Viewport width and height of the rectangle the UIRenderer is using for rendering.</returns>
	Rectanglei getViewport() const { return Rectanglei(0, 0, static_cast<int32_t>(getRenderingDimX()), static_cast<int32_t>(getRenderingDimY())); }

	/// <summary>Set the X dimension of the rectangle the UIRenderer is rendering to in order to scale UI elements.
	/// Initial value is the Screen Width.</summary>
	/// <param name="value">The new rendering width.</param>
	void setRenderingDimX(float value) { _screenDimensions.x = value; }

	/// <summary>Set the Y dimension of the rectangle the UIRenderer is rendering to in order to scale UI elements.
	/// Initial value is the Screen Height.</summary>
	/// <param name="value">The new rendering height.</param>
	void setRenderingDimY(float value) { _screenDimensions.y = value; }

	/// <summary>Create a font from a given texture (use PVRTexTool to create the font texture from a font file).</summary>
	/// <param name="texture">An OpenGL ES texture handle. Must be 2D. It will be used directly.</param>
	/// <param name="textureHeader">A pvr::TextureHeader of the same object. Necessary for the texture metadata.</param>
	/// <param name="sampler">(Optional) A specific sampler object to use for this font.</param>
	/// <returns>A new Font object. Null object if failed.</returns>
	/// <remarks>Use PVRTexTool to create textures suitable for Font use. You can then use any of the createFont
	/// function overloads to create a pvr::ui::Font object to render your sprites. This overload requires that you
	/// have already created a Texture2D object from the file and is suitable for sharing a texture between different
	/// UIRenderer objects.</remarks>
	Font createFont(GLuint texture, const TextureHeader& textureHeader, GLuint sampler = 0);

	/// <summary>Create a font from a given texture (use PVRTexTool to create the font texture from a font file).</summary>
	/// <param name="texture">An OpenGL ES texture handle. Must be 2D. It will be used directly.</param>
	/// <param name="sampler">(Optional) A specific sampler object to use for this font.</param>
	/// <returns>A new Font object. Null object if failed.</returns>
	/// <remarks>Use PVRTexTool to create textures suitable for Font use. You can then use any of the createFont
	/// function overloads to create a pvr::ui::Font object to render your sprites. This overload directly uses a
	/// pvr::Texture for both data and metadata, but will create a new Texture2D so if using multiple
	/// UIRenderes using the same font, might not be the most efficient version.</remarks>
	Font createFont(const Texture& texture, GLuint sampler = 0);

	/// <summary>Create a pvr::ui::Image from an API texture.
	/// NOTE: Creating new image requires re-recording the commandbuffer</summary>
	/// <param name="texture">An OpenGL ES texture handle. Must be 2D. It will be used directly.</param>
	/// <param name="width">The width of the texture.</param>
	/// <param name="height">The height of the texture.</param>
	/// <param name="useMipmaps">Specifies whether the image should use mip maps</param>
	/// <param name="sampler">(Optional) A specific sampler object to use for this font.</param>
	/// <returns>A new Image object of the specified texture. Null object if failed.</returns>
	Image createImage(GLuint texture, int32_t width, int32_t height, bool useMipmaps, GLuint sampler = 0);

	/// <summary>Create a pvr::ui::Image from a Texture asset.
	/// NOTE: Creating new image requires re-recording the commandbuffer</summary>
	/// <param name="texture">A pvr::Texture object. Will be internally used to create an api::Texture2D to
	/// use.</param>
	/// <param name="sampler">(Optional) A specific sampler object to use for this font.</param>
	/// <returns>A new Image object of the specified texture. Null object if failed.</returns>
	Image createImage(const Texture& texture, GLuint sampler = 0);

	/// <summary>Create a pvr::ui::Image from a Texture Atlas asset.
	/// NOTE: Creating new image requires re-recording the commandbuffer</summary>
	/// <param name="texture">An OpenGL ES texture handle. Must be 2D. It will be used directly.</param>
	/// <param name="uv">Texture UV coordinate</param>
	/// <param name="width"> Texture Atlas width.</param>
	/// <param name="height"> Texture Atlas height</param>
	/// <param name="useMipmaps">Specifies whether the image should use mip maps</param>
	/// <param name="sampler">A sampler used for this Image (Optional)</param>
	/// <returns>A new Image object of the specified texture. Null object if failed.</returns>
	Image createImageFromAtlas(GLuint texture, const Rectanglef& uv, uint32_t width, uint32_t height, bool useMipmaps = false, GLuint sampler = 0);

	/// <summary>Create a pvr::ui::MatrixGroup.</summary>
	/// <returns>A new Group to display different sprites together. Null object if failed.</returns>
	MatrixGroup createMatrixGroup();

	/// <summary>Create a pvr::ui::PixelGroup.</summary>
	/// <returns>A new Group to display different sprites together. Null object if failed.</returns>
	PixelGroup createPixelGroup();

	/// <summary>Begins direct rendering for the UIRenderer.</summary>
	void beginRendering()
	{
		storeCurrentGlState();
		checkStateChanged();
		setUiState();
	}

	/// <summary>Begins direct rendering for the UIRenderer. A GLStateTracker structure is used for more efficient state tracking management.
	/// Note that for correct state tracking and management this structure must have been fully managed and updated by the caller. If certain states
	/// and their tracker are not updated appropriately then certain states may not be tracked or set correctly resulting in incorrect rendering.</summary>
	/// <param name="stateTracker">A GLStateTracker structure storing the current OpenGL ES state as well as indicating which states have been changed
	/// since the last time UIRenderer was used.</param>
	void beginRendering(const GLStateTracker& stateTracker)
	{
		checkStateChanged(stateTracker);
		setUiState();
	}

	/// <summary>Ends rendering and resets the state</summary>
	void endRendering()
	{
		checkStateChanged();
		restoreState();
	}

	/// <summary>Ends rendering and fills out the GLStateTracker structure which can be used by the caller for more efficient state tracking management.</summary>
	/// <param name="stateTracker">A GLStateTracker structure which stores the current OpenGL ES state as well as indicating which states have been changed
	/// since the last time the caller called beginRendering. The Caller has the responsibility of restoring and managing the OpenGL ES state.</param>
	void endRendering(GLStateTracker& stateTracker) { stateTracker = _uiStateTracker; }

	/// <summary>The UIRenderer has a built-in default pvr::ui::Font that can always be used when the UIRenderer is
	/// initialized. Used throughout the PowerVR SDK Examples.</summary>
	/// <returns>The default font. Constant overload.</returns>
	const Font& getDefaultFont() const { return _defaultFont; }

	/// <summary>The UIRenderer has a built-in default pvr::ui::Font that can always be used when the UIRenderer is
	/// initialized. Used throughout the PowerVR SDK Examples.</summary>
	/// <returns>A pvr::ui::Font object of the default font.</returns>
	Font& getDefaultFont() { return _defaultFont; }

	/// <summary>The UIRenderer has a built-in pvr::ui::Image of the PowerVR SDK logo that can always be used when the
	/// UIRenderer is initialized. Used throughout the PowerVR SDK Examples.</summary>
	/// <returns>The PowerVR SDK pvr::ui::Image. Constant overload.</returns>
	const Image& getSdkLogo() const { return _sdkLogo; }

	/// <summary>The UIRenderer has a built-in pvr::ui::Image of the PowerVR SDK logo that can always be used when the
	/// UIRenderer is initialized. Used throughout the PowerVR SDK Examples.</summary>
	/// <returns>The PowerVR SDK pvr::ui::Image.</returns>
	Image& getSdkLogo() { return _sdkLogo; }

	/// <summary>The UIRenderer has a built-in pvr::ui::Text positioned and sized for used as a title (top-left,
	/// large) for convenience. Set the text of this sprite and use it as normal. Can be resized and repositioned at
	/// will. Used throughout the PowerVR SDK Examples.</summary>
	/// <returns>The Default title pvr::ui::Text. Originally empty (use setText on it). Constant overload.</returns>
	const Text& getDefaultTitle() const { return _defaultTitle; }

	/// <summary>The UIRenderer has a built-in pvr::ui::Text positioned and sized for used as a title (top-left,
	/// large) for convenience. Set the text of this sprite and use it as normal. Can be resized and repositioned at
	/// will. Used throughout the PowerVR SDK Examples.</summary>
	/// <returns>The Default title pvr::ui::Text. Originally empty (use setText on it).</returns>
	Text& getDefaultTitle() { return _defaultTitle; }

	/// <summary>The UIRenderer has a built-in pvr::ui::Text positioned and sized for use as a description (subtitle)
	/// (top-left, below DefaultTitle, small)) for convenience. Set the text of this sprite and use it as normal. Can
	/// be resized and repositioned at will. Used throughout the PowerVR SDK Examples.</summary>
	/// <returns>The Default descritption pvr::ui::Text. Originally empty (use setText on it). Constant overload.</returns>
	const Text& getDefaultDescription() const { return _defaultDescription; }

	/// <summary>The UIRenderer has a built-in pvr::ui::Text positioned and sized for used as a description (subtitle
	/// (top-left, below DefaultTitle, small)) for convenience. Set the text of this sprite and use it as normal. Can
	/// be resized and repositioned at will. Used throughout the PowerVR SDK Examples.</summary>
	/// <returns>The Default descritption pvr::ui::Text. Originally empty (use setText on it).</returns>
	Text& getDefaultDescription() { return _defaultDescription; }

	/// <summary>The UIRenderer has a built-in pvr::ui::Text positioned and sized for use as a controls display
	/// (bottom-left, small text) for convenience. You can set the text of this sprite and use it as normal. Can be
	/// resized and repositioned at will. Used throughout the PowerVR SDK Examples.</summary>
	/// <returns>The Default Controls pvr::ui::Text. Originally empty (use setText on it). Constant overload.</returns>
	const Text& getDefaultControls() const { return _defaultControls; }

	/// <summary>The UIRenderer has a built-in pvr::ui::Text positioned and sized for use as a controls display
	/// (bottom-left, small text) for convenience. You can set the text of this sprite and use it as normal. Can be
	/// resized and repositioned at will. Used throughout the PowerVR SDK Examples.</summary>
	/// <returns>The Default Controls pvr::ui::Text. Originally empty (use setText on it).</returns>
	Text& getDefaultControls() { return _defaultControls; }

	/// <summary>Returns the projection matrix</summary>
	/// <returns>The UIRenderer projection matrix</returns>
	glm::mat4 getProjection() const { return pvr::math::ortho(Api::OpenGLES2, 0.0, getRenderingDimX(), 0.0f, getRenderingDimY()); }

	/// <summary>return the default DescriptorSetLayout. ONLY to be used by the Sprites</summary>
	/// <returns>const api::DescriptorSetLayout&</returns>
	void rotateScreen90degreeCCW()
	{
		_screenRotation += glm::pi<float>() * .5f;
		std::swap(_screenDimensions.x, _screenDimensions.y);
	}

	/// <summary>return the default DescriptorSetLayout. ONLY to be used by the Sprites</summary>
	/// <returns>const api::DescriptorSetLayout&</returns>
	void rotateScreen90degreeCW()
	{
		_screenRotation -= glm::pi<float>() * .5f;
		std::swap(_screenDimensions.x, _screenDimensions.y);
	}

	/// <summary>Return the default DescriptorSetLayout. ONLY to be used by the Sprites</summary>
	/// <returns>const api::DescriptorSetLayout&</returns>
	glm::mat4 getScreenRotation() const { return glm::rotate(_screenRotation, glm::vec3(0.0f, 0.0f, 1.f)); }

	/// <summary>Return the state tracker used by the ui renderer for tracking changed states</summary>
	/// <returns>pvr::ui::GLStateTracker</returns>
	GLStateTracker getStateTracker() const { return _uiStateTracker; }

	/// <summary>Return the OpenGL ES version assumed by the UIRenderer (the bound context version from when it was created)</summary>
	/// <returns>The API version assumed by the UIRenderer.</returns>
	Api getApiVersion() { return _api; }

private:
	void storeCurrentGlState();
	void setUiState();
	void checkStateChanged();
	void checkStateChanged(const GLStateTracker& stateTracker);
	void restoreState();

	void updateResourceOwnership()
	{
		std::for_each(_sprites.begin(), _sprites.end(), [this](SpriteWeakRef& sprite) { sprite.lock()->setUIRenderer(this); });

		std::for_each(_fonts.begin(), _fonts.end(), [this](FontWeakRef& font) { font.lock()->setUIRenderer(this); });

		std::for_each(_textElements.begin(), _textElements.end(), [this](TextElementWeakRef& textElement) { textElement.lock()->setUIRenderer(this); });
	}

	friend class ::pvr::ui::impl::Image_;
	friend class ::pvr::ui::impl::Text_;
	friend class ::pvr::ui::impl::Group_;
	friend class ::pvr::ui::impl::Sprite_;
	friend class ::pvr::ui::impl::Font_;
	friend class ::pvr::ui::impl::TextElement_;
	friend class ::pvr::ui::impl::MatrixGroup_;

	/// <summary>return the default DescriptorSetLayout. ONLY to be used by the Sprites</summary>
	/// <returns>const api::DescriptorSetLayout&</returns>
	uint64_t generateGroupId() { return _groupId++; }

	GLuint getSamplerBilinear() const { return _samplerBilinear; }

	GLuint getSamplerTrilinear() const { return _samplerTrilinear; }

	void init_CreateDefaultFont();
	void init_CreateDefaultSampler();
	void init_CreateDefaultSdkLogo();
	void init_CreateDefaultTitle();
	void init_CreateShaders(bool framebufferSRGB);

	std::vector<SpriteWeakRef> _sprites;
	std::vector<TextElementWeakRef> _textElements;
	std::vector<FontWeakRef> _fonts;

	ProgramData _programData;
	Font _defaultFont;
	Image _sdkLogo;
	Text _defaultTitle;
	Text _defaultDescription;
	Text _defaultControls;
	GLuint _program;

	GLuint _samplerBilinear;
	GLuint _samplerTrilinear;
	GLuint _fontIbo;
	bool _fontIboCreated;
	GLuint _imageVbo;
	bool _imageVboCreated;
	glm::vec2 _screenDimensions;
	float _screenRotation;
	uint64_t _groupId = 1;

	GLStateTracker _uiStateTracker;
	GLState _currentState;

	Api _api;
};
} // namespace ui
} // namespace pvr
