/*!
\brief Contains implementations of functions for the UIRendererGles class.
\file PVRUtils/OpenGLES/UIRendererGles.cpp
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
//!\cond NO_DOXYGEN
#include "UIRendererGles.h"
#include "ErrorsGles.h"
#include "PVRCore/stream/BufferStream.h"
#include "PVRUtils/OpenGLES/HelperGles.h"
#include "PVRUtils/ArialBoldFont.h"
#include "PVRUtils/PowerVRLogo.h"
#include "PVRUtils/OpenGLES/UIRendererShaders_ES.h"

using std::map;
using std::vector;
namespace pvr {
namespace ui {
const glm::vec2 BaseScreenDim(640, 480);

// store the current GL state
void GLState::storeCurrentGlState(Api api)
{
	debugThrowOnApiError("glState::storeCurrentGlState Enter");

	gl::GetIntegerv(GL_CURRENT_PROGRAM, &activeProgram);
	gl::GetIntegerv(GL_ACTIVE_TEXTURE, &activeTextureUnit);
	gl::GetIntegerv(GL_TEXTURE_BINDING_2D, &boundTexture);
	gl::GetIntegerv(GL_BLEND, &blendEnabled);
	gl::GetIntegerv(GL_BLEND_SRC_RGB, &blendSrcRgb);
	gl::GetIntegerv(GL_BLEND_SRC_ALPHA, &blendSrcAlpha);
	gl::GetIntegerv(GL_BLEND_DST_RGB, &blendDstRgb);
	gl::GetIntegerv(GL_BLEND_DST_ALPHA, &blendDstAlpha);
	gl::GetIntegerv(GL_BLEND_EQUATION_RGB, &blendEqationRgb);
	gl::GetIntegerv(GL_BLEND_EQUATION_ALPHA, &blendEqationAlpha);
	gl::GetBooleanv(GL_COLOR_WRITEMASK, colorMask);
	gl::GetIntegerv(GL_DEPTH_TEST, &depthTest);
	gl::GetIntegerv(GL_DEPTH_WRITEMASK, &depthMask);
	gl::GetIntegerv(GL_STENCIL_TEST, &stencilTest);
	gl::GetIntegerv(GL_CULL_FACE, &cullingEnabled);
	gl::GetIntegerv(GL_CULL_FACE_MODE, &culling);
	gl::GetIntegerv(GL_FRONT_FACE, &windingOrder);
	gl::GetIntegerv(GL_ARRAY_BUFFER_BINDING, &vbo);
	gl::GetIntegerv(GL_ELEMENT_ARRAY_BUFFER_BINDING, &ibo);
	debugThrowOnApiError("glState::storeCurrentGlState: 1");
	if (api > Api::OpenGLES2)
	{
		gl::GetIntegerv(GL_VERTEX_ARRAY_BINDING, &vao);
		if (activeTextureUnit != 7) { gl::ActiveTexture(GL_TEXTURE7); }
		gl::GetIntegerv(GL_SAMPLER_BINDING, &sampler7);
		if (activeTextureUnit != 7) { gl::ActiveTexture(activeTextureUnit); }
	}
	else
	{
		gl::GetIntegerv(GL_VERTEX_ARRAY_BINDING_OES, &vao);
	}

	if (vao != 0)
	{
		if (api > Api::OpenGLES2) { gl::BindVertexArray(0); }
		else
		{
			gl::ext::BindVertexArrayOES(0);
		}
	}
	debugThrowOnApiError("glState::storeCurrentGlState: 2");

	for (uint32_t i = 0; i < 8; i++)
	{
		GLint enabled;
		gl::GetVertexAttribiv(i, GL_VERTEX_ATTRIB_ARRAY_ENABLED, &enabled);
		vertexAttribArray[i] = enabled == 1 ? GL_TRUE : GL_FALSE;
		debugThrowOnApiError("glState::storeCurrentGlState: 3");

		if (vertexAttribArray[i])
		{
			vertexAttribBindings[i] = static_cast<uint32_t>(-1);
			vertexAttribArray[i] = false;
#ifdef GL_VERTEX_ATTRIB_BINDING
			if (api > pvr::Api::OpenGLES3)
			{
				GLint attribBinding;
				gl::GetVertexAttribiv(i, GL_VERTEX_ATTRIB_BINDING, &attribBinding);
				vertexAttribBindings[i] = attribBinding;
				vertexAttribArray[i] = true; // enable it back.
				debugThrowOnApiError("glState::storeCurrentGlState:4");
			}
#endif

			GLint attribSize;
			gl::GetVertexAttribiv(i, GL_VERTEX_ATTRIB_ARRAY_SIZE, &attribSize);
			vertexAttribSizes[i] = attribSize;

			GLint attribType;
			gl::GetVertexAttribiv(i, GL_VERTEX_ATTRIB_ARRAY_TYPE, &attribType);
			vertexAttribTypes[i] = attribType;

			GLint attribNormalized;
			gl::GetVertexAttribiv(i, GL_VERTEX_ATTRIB_ARRAY_NORMALIZED, &attribNormalized);
			vertexAttribNormalized[i] = attribNormalized;

			GLint attribStride;
			gl::GetVertexAttribiv(i, GL_VERTEX_ATTRIB_ARRAY_STRIDE, &attribStride);
			vertexAttribStride[i] = attribStride;

			GLvoid* attribOffset;
			gl::GetVertexAttribPointerv(i, GL_VERTEX_ATTRIB_ARRAY_POINTER, &attribOffset);
			vertexAttribOffset[i] = attribOffset;
		}
	}
	debugThrowOnApiError("glState::storeCurrentGlState Exit");
}

void GLStateTracker::checkStateChanged(const GLState& currentGlState)
{
	activeProgramChanged = activeProgram != currentGlState.activeProgram;
	activeTextureUnitChanged = activeTextureUnit != currentGlState.activeTextureUnit;
	boundTextureChanged = boundTexture != currentGlState.boundTexture;

	// blending states
	blendEnabledChanged = blendEnabled != currentGlState.blendEnabled;
	blendSrcRgbChanged = blendDstRgb != currentGlState.blendSrcRgb;
	blendDstRgbChanged = blendDstRgb != currentGlState.blendDstRgb;
	blendSrcAlphaChanged = blendSrcAlpha != currentGlState.blendSrcAlpha;
	blendDstAlphaChanged = blendDstAlpha != currentGlState.blendDstAlpha;
	blendEqationRgbChanged = blendEqationRgb != currentGlState.blendEqationRgb;
	blendEqationAlphaChanged = blendEqationAlpha != currentGlState.blendEqationAlpha;

	// depth states
	depthTestChanged = depthTest != currentGlState.depthTest;
	depthMaskChanged = depthMask != currentGlState.depthMask;

	stencilTestChanged = stencilTest != currentGlState.stencilTest;

	cullingEnabledChanged = cullingEnabled != currentGlState.cullingEnabled;
	cullingChanged = culling != currentGlState.culling;
	windingOrderChanged = windingOrder != currentGlState.windingOrder;

	sampler7Changed = sampler7 != currentGlState.sampler7;
	if (vbo != -1) { vboChanged = vbo != currentGlState.vbo; }
	if (ibo != -1) { iboChanged = ibo != currentGlState.ibo; }
	if (vao != -1) { vaoChanged = vao != currentGlState.vao; }

	if (currentGlState.vao != 0) { vaoChanged = true; }

	colorMaskChanged = ((colorMask[0] != currentGlState.colorMask[0]) || (colorMask[1] != currentGlState.colorMask[1]) || (colorMask[2] != currentGlState.colorMask[2]) ||
		(colorMask[3] != currentGlState.colorMask[3]));

	for (uint32_t i = 0; i < 8; i++)
	{
		vertexAttribArrayChanged[i] = vertexAttribArray[i] != currentGlState.vertexAttribArray[i];

		vertexAttribPointerChanged[i] = vertexAttribBindings[i] != currentGlState.vertexAttribBindings[i] || vertexAttribSizes[i] != currentGlState.vertexAttribSizes[i] ||
			vertexAttribTypes[i] != currentGlState.vertexAttribTypes[i] || vertexAttribNormalized[i] != currentGlState.vertexAttribNormalized[i] ||
			vertexAttribStride[i] != currentGlState.vertexAttribStride[i] || vertexAttribOffset[i] != currentGlState.vertexAttribOffset[i];
	}
}

void GLStateTracker::checkStateChanged(const GLStateTracker& stateTracker)
{
	activeProgramChanged = stateTracker.activeProgramChanged;
	activeTextureUnitChanged = stateTracker.activeTextureUnitChanged;
	boundTextureChanged = stateTracker.boundTextureChanged;

	// blending states
	blendEnabledChanged = stateTracker.blendEnabledChanged;
	blendSrcRgbChanged = stateTracker.blendSrcRgbChanged;
	blendDstRgbChanged = stateTracker.blendDstRgbChanged;
	blendSrcAlphaChanged = stateTracker.blendSrcAlphaChanged;
	blendDstAlphaChanged = stateTracker.blendDstAlphaChanged;
	blendEqationRgbChanged = stateTracker.blendEqationRgbChanged;
	blendEqationAlphaChanged = stateTracker.blendEqationAlphaChanged;

	// depth states
	depthTestChanged = stateTracker.depthTestChanged;
	depthMaskChanged = stateTracker.depthMaskChanged;

	stencilTestChanged = stateTracker.stencilTestChanged;

	cullingEnabledChanged = stateTracker.cullingEnabledChanged;
	cullingChanged = stateTracker.cullingChanged;
	windingOrderChanged = stateTracker.windingOrderChanged;

	sampler7Changed = stateTracker.sampler7Changed;
	if (vbo != -1) { vboChanged = stateTracker.vboChanged; }
	if (ibo != -1) { iboChanged = stateTracker.iboChanged; }
	if (vao != -1) { vaoChanged = stateTracker.vaoChanged; }

	if (stateTracker.vao != 0) { vaoChanged = true; }

	colorMaskChanged = stateTracker.colorMaskChanged;

	for (uint32_t i = 0; i < 8; i++)
	{
		vertexAttribArrayChanged[i] = stateTracker.vertexAttribArrayChanged[i];
		vertexAttribPointerChanged[i] = stateTracker.vertexAttribPointerChanged[i];
	}
}

void GLStateTracker::setUiState(Api api)
{
	debugThrowOnApiError("GLStateTracker::setState Enter");
	if (activeProgramChanged) { gl::UseProgram(static_cast<GLuint>(activeProgram)); }
	if (activeTextureUnitChanged) { gl::ActiveTexture(static_cast<GLenum>(activeTextureUnit)); }
	if (boundTextureChanged) { gl::BindTexture(GL_TEXTURE_2D, static_cast<GLuint>(boundTexture)); }
	if (blendEnabledChanged) { blendEnabled ? gl::Enable(GL_BLEND) : gl::Disable(GL_BLEND); }
	if (blendSrcRgbChanged || blendSrcAlphaChanged || blendDstRgbChanged || blendDstAlphaChanged) { gl::BlendFuncSeparate(blendSrcRgb, blendDstRgb, blendSrcAlpha, blendDstAlpha); }
	if (blendEqationRgbChanged || blendEqationAlphaChanged) { gl::BlendEquationSeparate(blendEqationRgb, blendEqationAlpha); }

	if (colorMaskChanged) { gl::ColorMask(colorMask[0], colorMask[1], colorMask[2], colorMask[3]); }
	if (depthTestChanged) { depthTest ? gl::Enable(GL_DEPTH_TEST) : gl::Disable(GL_DEPTH_TEST); }
	if (depthMaskChanged) { gl::DepthMask((GLboolean)depthMask); }
	if (stencilTestChanged) { stencilTest ? gl::Enable(GL_STENCIL_TEST) : gl::Disable(GL_STENCIL_TEST); }
	if (cullingEnabledChanged) { cullingEnabled ? gl::Enable(GL_CULL_FACE) : gl::Disable(GL_CULL_FACE); }
	if (cullingChanged) { gl::CullFace(culling); }
	if (windingOrderChanged) { gl::FrontFace(windingOrder); }
	if (sampler7Changed) { gl::BindSampler(7, sampler7); }
	if (vaoChanged)
	{
		if (api > Api::OpenGLES2) { gl::BindVertexArray(0); }
		else
		{
			gl::ext::BindVertexArrayOES(0);
		}
	}
	if (vboChanged) { gl::BindBuffer(GL_ARRAY_BUFFER, static_cast<GLuint>(vbo)); }
	if (iboChanged) { gl::BindBuffer(GL_ELEMENT_ARRAY_BUFFER, static_cast<GLuint>(ibo)); }

	for (uint32_t i = 0; i < 8; i++)
	{
		if (vertexAttribArrayChanged[i])
		{
			if (vertexAttribArray[i])
			{
				gl::EnableVertexAttribArray(i);

				if (vertexAttribPointerChanged[i])
				{
					gl::VertexAttribPointer(
						vertexAttribBindings[i], vertexAttribSizes[i], vertexAttribTypes[i], (GLboolean)vertexAttribNormalized[i], vertexAttribStride[i], vertexAttribOffset[i]);
				}
			}
			else
			{
				gl::DisableVertexAttribArray(i);
			}
		}
	}

	debugThrowOnApiError("GLStateTracker::setState Exit");
}

void GLStateTracker::restoreState(const GLState& currentGlState, Api api)
{
	debugThrowOnApiError("glState::restoreState Enter");
	if (activeProgramChanged)
	{
		gl::UseProgram(static_cast<GLuint>(currentGlState.activeProgram));
		debugThrowOnApiError("glState::restoreState Exit");
	}
	if (activeTextureUnitChanged)
	{
		gl::ActiveTexture(static_cast<GLenum>(currentGlState.activeTextureUnit));
		debugThrowOnApiError("glState::restoreState Exit");
	}
	if (boundTextureChanged)
	{
		gl::BindTexture(GL_TEXTURE_2D, static_cast<GLuint>(currentGlState.boundTexture));
		debugThrowOnApiError("glState::restoreState Exit");
	}
	if (blendEnabledChanged)
	{
		currentGlState.blendEnabled ? gl::Enable(GL_BLEND) : gl::Disable(GL_BLEND);
		debugThrowOnApiError("glState::restoreState Exit");
	}
	if (blendSrcRgbChanged || blendSrcAlphaChanged || blendDstRgbChanged || blendDstAlphaChanged)
	{
		gl::BlendFuncSeparate(currentGlState.blendSrcRgb, currentGlState.blendDstRgb, currentGlState.blendSrcAlpha, currentGlState.blendDstAlpha);
		debugThrowOnApiError("glState::restoreState Exit");
	}
	if (blendEqationRgbChanged || blendEqationAlphaChanged)
	{
		gl::BlendEquationSeparate(currentGlState.blendEqationRgb, currentGlState.blendEqationAlpha);
		debugThrowOnApiError("glState::restoreState Exit");
	}

	if (colorMaskChanged)
	{
		gl::ColorMask(currentGlState.colorMask[0], currentGlState.colorMask[1], currentGlState.colorMask[2], currentGlState.colorMask[3]);
		debugThrowOnApiError("glState::restoreState Exit");
	}
	if (depthTestChanged)
	{
		currentGlState.depthTest ? gl::Enable(GL_DEPTH_TEST) : gl::Disable(GL_DEPTH_TEST);
		debugThrowOnApiError("glState::restoreState Exit");
	}
	if (depthMaskChanged)
	{
		gl::DepthMask((GLboolean)currentGlState.depthMask);
		debugThrowOnApiError("glState::restoreState Exit");
	}
	if (stencilTestChanged)
	{
		currentGlState.stencilTest ? gl::Enable(GL_STENCIL_TEST) : gl::Disable(GL_STENCIL_TEST);
		debugThrowOnApiError("glState::restoreState Exit");
	}
	if (cullingEnabledChanged)
	{
		currentGlState.cullingEnabled ? gl::Enable(GL_CULL_FACE) : gl::Disable(GL_CULL_FACE);
		debugThrowOnApiError("glState::restoreState Exit");
	}
	if (cullingChanged)
	{
		gl::CullFace(static_cast<GLenum>(currentGlState.culling));
		debugThrowOnApiError("glState::restoreState Exit");
	}

	if (windingOrder)
	{
		gl::FrontFace(static_cast<GLenum>(currentGlState.windingOrder));
		debugThrowOnApiError("glState::restoreState Exit");
	}
	if (sampler7Changed)
	{
		gl::BindSampler(7, static_cast<GLuint>(currentGlState.sampler7));
		debugThrowOnApiError("glState::restoreState Exit");
	}
	if (vboChanged)
	{
		gl::BindBuffer(GL_ARRAY_BUFFER, static_cast<GLuint>(currentGlState.vbo));
		debugThrowOnApiError("glState::restoreState Exit");
	}
	if (iboChanged)
	{
		gl::BindBuffer(GL_ELEMENT_ARRAY_BUFFER, static_cast<GLuint>(currentGlState.ibo));
		debugThrowOnApiError("glState::restoreState Exit");
	}

	for (uint32_t i = 0; i < 8; i++)
	{
		if (vertexAttribArrayChanged[i])
		{
			if (currentGlState.vertexAttribArray[i])
			{
				gl::EnableVertexAttribArray(i);

				if (vertexAttribPointerChanged[i])
				{
					gl::VertexAttribPointer(currentGlState.vertexAttribBindings[i], currentGlState.vertexAttribSizes[i], currentGlState.vertexAttribTypes[i],
						(GLboolean)currentGlState.vertexAttribNormalized[i], currentGlState.vertexAttribStride[i], vertexAttribOffset[i]);
					debugThrowOnApiError("glState::restoreState Exit");
				}
			}
			else
			{
				gl::DisableVertexAttribArray(i);
				debugThrowOnApiError("glState::restoreState Exit");
			}
		}
	}

	if (vaoChanged)
	{
		if (api > Api::OpenGLES2)
		{
			gl::BindVertexArray(static_cast<GLuint>(currentGlState.vao));
			debugThrowOnApiError("glState::restoreState Exit");
		}
		else
		{
			gl::ext::BindVertexArrayOES(static_cast<GLuint>(currentGlState.vao));
			debugThrowOnApiError("glState::restoreState Exit");
		}
	}

	debugThrowOnApiError("glState::restoreState Exit");
}

void UIRenderer::checkStateChanged(const GLStateTracker& stateTracker) { _uiStateTracker.checkStateChanged(stateTracker); }

void UIRenderer::checkStateChanged() { _uiStateTracker.checkStateChanged(_currentState); }

void UIRenderer::restoreState() { _uiStateTracker.restoreState(_currentState, _api); }

void UIRenderer::storeCurrentGlState() { _currentState.storeCurrentGlState(_api); }

void UIRenderer::setUiState() { _uiStateTracker.setUiState(_api); }

void UIRenderer::init_CreateShaders(bool framebufferSRGB)
{
	debugThrowOnApiError("UIRenderer::init_CreateShaders entry");
	// Text_ pipe
	GLuint shaders[] = { 0, 0 };

	shaders[0] = pvr::utils::loadShader(BufferStream("", _print3DShader_glsles200_vsh, _print3DShader_glsles200_vsh_size), ShaderType::VertexShader, nullptr, 0);

	// fragment shader
	if (framebufferSRGB)
	{
		const char* fragShaderDefines[] = { "FRAMEBUFFER_SRGB" };
		shaders[1] = pvr::utils::loadShader(BufferStream("", _print3DShader_glsles200_fsh, _print3DShader_glsles200_fsh_size), ShaderType::FragmentShader, fragShaderDefines, 1);
	}
	else
	{
		shaders[1] = pvr::utils::loadShader(BufferStream("", _print3DShader_glsles200_fsh, _print3DShader_glsles200_fsh_size), ShaderType::FragmentShader, nullptr, 0);
	}

	const char* attributes[] = { "myVertex", "myUV" };
	const uint16_t attribIndices[] = { 0, 1 };

	_program = pvr::utils::createShaderProgram(shaders, 2, attributes, attribIndices, 2);

	_uiStateTracker.activeProgram = _program;

	GLint prev_program;
	gl::GetIntegerv(GL_CURRENT_PROGRAM, &prev_program);

	gl::UseProgram(_program);
	_programData.uniforms[UIRenderer::ProgramData::UniformMVPmtx] = gl::GetUniformLocation(_program, "myMVPMatrix");
	_programData.uniforms[UIRenderer::ProgramData::UniformFontTexture] = gl::GetUniformLocation(_program, "fontTexture");
	_programData.uniforms[UIRenderer::ProgramData::UniformColor] = gl::GetUniformLocation(_program, "varColor");
	_programData.uniforms[UIRenderer::ProgramData::UniformAlphaMode] = gl::GetUniformLocation(_program, "alphaMode");
	_programData.uniforms[UIRenderer::ProgramData::UniformUVmtx] = gl::GetUniformLocation(_program, "myUVMatrix");

	gl::Uniform1i(_programData.uniforms[UIRenderer::ProgramData::UniformFontTexture], 7);
	debugThrowOnApiError("UIRenderer::init_CreateShaders exit");
}

Font UIRenderer::createFont(const Texture& tex, GLuint sampler)
{
	auto results = utils::textureUpload(tex, _api == Api::OpenGLES2, true);
	return createFont(results.image, tex, sampler);
}

Font UIRenderer::createFont(GLuint texture, const TextureHeader& texHeader, GLuint sampler)
{
	Font font = pvr::ui::impl::Font_::constructShared(*this, texture, texHeader, sampler);
	_fonts.emplace_back(font);
	return font;
}

Image UIRenderer::createImage(const Texture& texture, GLuint sampler)
{
	utils::TextureUploadResults res = utils::textureUpload(texture, _api == Api::OpenGLES2, true);

	gl::BindTexture(GL_TEXTURE_2D, res.image);
	if (texture.getLayersSize().numMipLevels > 1)
	{
		gl::TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		gl::TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	}
	else
	{
		gl::TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		gl::TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	}
	gl::TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	gl::TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	return createImage(res.image, texture.getWidth(), texture.getHeight(), texture.getLayersSize().numMipLevels > 1, sampler);
}

MatrixGroup UIRenderer::createMatrixGroup()
{
	MatrixGroup group = pvr::ui::impl::MatrixGroup_::constructShared(*this, generateGroupId());
	_sprites.emplace_back(group);
	group->commitUpdates();
	return group;
}

PixelGroup UIRenderer::createPixelGroup()
{
	PixelGroup group = pvr::ui::impl::PixelGroup_::constructShared(*this, generateGroupId());
	_sprites.emplace_back(group);
	group->commitUpdates();
	return group;
}

Image UIRenderer::createImage(GLuint tex, int32_t width, int32_t height, bool useMipmaps, GLuint sampler)
{
	return createImageFromAtlas(tex, Rectanglef(0.0f, 0.0f, 1.0f, 1.0f), width, height, useMipmaps, sampler);
}

pvr::ui::Image UIRenderer::createImageFromAtlas(GLuint texture, const Rectanglef& uv, uint32_t atlasWidth, uint32_t atlasHeight, bool useMipmaps, GLuint sampler)
{
	Image image = pvr::ui::impl::Image_::constructShared(*this, texture, atlasWidth, atlasHeight, useMipmaps, sampler);
	_sprites.emplace_back(image);
	// construct the scaling matrix
	// calculate the scale factor
	// convert from texel to normalize coord
	image->setUV(uv);
	image->commitUpdates();
	return image;
}

TextElement UIRenderer::createTextElement(const std::wstring& text, const Font& font)
{
	TextElement spriteText = pvr::ui::impl::TextElement_::constructShared(*this, text, font);
	_textElements.emplace_back(spriteText);
	return spriteText;
}

TextElement UIRenderer::createTextElement(const std::string& text, const Font& font)
{
	TextElement spriteText = pvr::ui::impl::TextElement_::constructShared(*this, text, font);
	_textElements.emplace_back(spriteText);
	return spriteText;
}

Text UIRenderer::createText(const TextElement& textElement)
{
	Text text = pvr::ui::impl::Text_::constructShared(*this, textElement);
	_sprites.emplace_back(text);
	text->commitUpdates();
	return text;
}

namespace {
inline Api getCurrentGlesVersion()
{
	const char* apistring = (const char*)gl::GetString(GL_VERSION);
	int major, minor;
	sscanf(apistring, "OpenGL ES %d.%d", &major, &minor);

	if (major == 2) return Api::OpenGLES2;
	if (major == 3)
	{
		if (minor == 0) return Api::OpenGLES3;
		return Api::OpenGLES31;
	}
	throw "";
}

} // namespace

void UIRenderer::init(uint32_t width, uint32_t height, bool fullscreen, bool isFrameBufferSRGB)
{
	_api = pvr::utils::getCurrentGlesVersion();

	debugThrowOnApiError("");
	release();
	_screenDimensions = glm::vec2(width, height);
	// screen rotated?
	if (_screenDimensions.y > _screenDimensions.x && fullscreen) { rotateScreen90degreeCCW(); }

	debugThrowOnApiError("UIRenderer::init 1");
	storeCurrentGlState();
	debugThrowOnApiError("UIRenderer::init 2");
	init_CreateShaders(isFrameBufferSRGB);

	if (_api != Api::OpenGLES2)
	{
		init_CreateDefaultSampler();
		debugThrowOnApiError("UIRenderer::init CreateDefaultSampler");
	}
	init_CreateDefaultSdkLogo();
	debugThrowOnApiError("UIRenderer::init CreateDefaultSdkLogo");
	init_CreateDefaultFont();
	debugThrowOnApiError("UIRenderer::init CreateDefaultFont");
	init_CreateDefaultTitle();
	debugThrowOnApiError("UIRenderer::init CreateDefaultTitle");

	// set some initial ui state tracking state
	_uiStateTracker.vertexAttribArray[0] = GL_TRUE;
	_uiStateTracker.vertexAttribArray[1] = GL_TRUE;

	_uiStateTracker.vertexAttribBindings[0] = 0;
	_uiStateTracker.vertexAttribSizes[0] = 4;
	_uiStateTracker.vertexAttribTypes[0] = GL_FLOAT;
	_uiStateTracker.vertexAttribNormalized[0] = GL_FALSE;
	_uiStateTracker.vertexAttribStride[0] = sizeof(float) * 6;
	_uiStateTracker.vertexAttribOffset[0] = nullptr;

	_uiStateTracker.vertexAttribBindings[1] = 1;
	_uiStateTracker.vertexAttribSizes[1] = 2;
	_uiStateTracker.vertexAttribTypes[1] = GL_FLOAT;
	_uiStateTracker.vertexAttribNormalized[1] = GL_FALSE;
	_uiStateTracker.vertexAttribStride[1] = sizeof(float) * 6;
	_uiStateTracker.vertexAttribOffset[1] = reinterpret_cast<GLvoid*>(sizeof(float) * 4);

	checkStateChanged();
	restoreState();
	debugThrowOnApiError("UIRenderer::init RestoreState");
}

void UIRenderer::init_CreateDefaultSampler()
{
	if (_api != Api::OpenGLES2)
	{
		debugThrowOnApiError("UIRenderer::init_CreateDefaultSampler Enter");
		debugThrowOnApiError("UIRenderer::init_CreateDefaultSampler 1");
		gl::GenSamplers(1, &_samplerBilinear);
		gl::GenSamplers(1, &_samplerTrilinear);
		debugThrowOnApiError("UIRenderer::init_CreateDefaultSampler 1.1");
		gl::SamplerParameteri(_samplerBilinear, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
		debugThrowOnApiError("UIRenderer::init_CreateDefaultSampler 1.2");
		gl::SamplerParameteri(_samplerBilinear, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		debugThrowOnApiError("UIRenderer::init_CreateDefaultSampler 1.3");
		gl::SamplerParameteri(_samplerBilinear, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
		debugThrowOnApiError("UIRenderer::init_CreateDefaultSampler 1.4");
		gl::SamplerParameteri(_samplerBilinear, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		debugThrowOnApiError("UIRenderer::init_CreateDefaultSampler 1.5");
		gl::SamplerParameteri(_samplerBilinear, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

		debugThrowOnApiError("UIRenderer::init_CreateDefaultSampler 2");

		gl::SamplerParameteri(_samplerTrilinear, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		debugThrowOnApiError("UIRenderer::init_CreateDefaultSampler 2.1");
		gl::SamplerParameteri(_samplerTrilinear, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		debugThrowOnApiError("UIRenderer::init_CreateDefaultSampler 2.2");
		gl::SamplerParameteri(_samplerTrilinear, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
		debugThrowOnApiError("UIRenderer::init_CreateDefaultSampler 2.3");
		gl::SamplerParameteri(_samplerTrilinear, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		debugThrowOnApiError("UIRenderer::init_CreateDefaultSampler 2.4");
		gl::SamplerParameteri(_samplerTrilinear, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		debugThrowOnApiError("UIRenderer::init_CreateDefaultSampler Exit");

		_uiStateTracker.sampler7 = _samplerBilinear;
	}
}

void UIRenderer::init_CreateDefaultSdkLogo()
{
	std::unique_ptr<Stream> sdkLogo = std::make_unique<BufferStream>("", _PowerVR_Logo_RGBA_pvr, static_cast<size_t>(_PowerVR_Logo_RGBA_pvr_size));
	Texture sdkTex;
	sdkTex = textureLoad(*sdkLogo, TextureFileFormat::PVR);

	_sdkLogo = createImage(sdkTex);

	_sdkLogo->setAnchor(Anchor::BottomRight, glm::vec2(.98f, -.98f));
	float scalefactor = .3f * getRenderingDim().x / BaseScreenDim.x;

	if (scalefactor > 1.f) { scalefactor = 1.f; }
	else if (scalefactor > .5f)
	{
		scalefactor = .5f;
	}
	else if (scalefactor > .25f)
	{
		scalefactor = .25f;
	}
	else if (scalefactor > .125f)
	{
		scalefactor = .125f;
	}
	else
	{
		scalefactor = .0625;
	}

	_sdkLogo->setScale(glm::vec2(scalefactor));
	_sdkLogo->commitUpdates();
}

void UIRenderer::init_CreateDefaultTitle()
{
	_defaultTitle = createText(createTextElement("DefaultTitle", _defaultFont));
	debugThrowOnApiError("UIRenderer::init_CreateDefaultTitle createText0");
	_defaultDescription = createText(createTextElement("", _defaultFont));
	debugThrowOnApiError("UIRenderer::init_CreateDefaultTitle createText1");
	_defaultControls = createText(createTextElement("", _defaultFont));
	debugThrowOnApiError("UIRenderer::init_CreateDefaultTitle createText2");

	_defaultTitle->setAnchor(Anchor::TopLeft, glm::vec2(-.98f, .98f))->setScale(glm::vec2(.8, .8));
	_defaultTitle->commitUpdates();

	_defaultDescription->setAnchor(Anchor::TopLeft, glm::vec2(-.98f, .98f - _defaultTitle->getFont()->getFontLineSpacing() / static_cast<float>(getRenderingDimY()) * 1.5f))
		->setScale(glm::vec2(.60, .60));
	_defaultDescription->commitUpdates();

	_defaultControls->setAnchor(Anchor::BottomLeft, glm::vec2(-.98f, -.98f))->setScale(glm::vec2(.5, .5));
	_defaultControls->commitUpdates();
	debugThrowOnApiError("UIRenderer::init_CreateDefaultTitle Exit");
}

void UIRenderer::init_CreateDefaultFont()
{
	Texture fontTex;
	std::unique_ptr<Stream> arialFontTex;
	float maxRenderDim = glm::max<float>(getRenderingDimX(), getRenderingDimY());
	// pick the right font size of this resolution.
	if (maxRenderDim <= 800) { arialFontTex = std::make_unique<BufferStream>("", _arialbd_36_a8_pvr, _arialbd_36_a8_pvr_size); }
	else if (maxRenderDim <= 1000)
	{
		arialFontTex = std::make_unique<BufferStream>("", _arialbd_46_a8_pvr, _arialbd_46_a8_pvr_size);
	}
	else
	{
		arialFontTex = std::make_unique<BufferStream>("", _arialbd_56_a8_pvr, _arialbd_56_a8_pvr_size);
	}

	fontTex = textureLoad(*arialFontTex, TextureFileFormat::PVR);

	_defaultFont = createFont(fontTex);

	if (_api > Api::OpenGLES2)
	{
		gl::BindTexture(GL_TEXTURE_2D, _defaultFont->getTexture());
		gl::TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_R, GL_RED);
		gl::TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_G, GL_RED);
		gl::TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_B, GL_RED);
		gl::TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_A, GL_RED);
	}
	gl::BindTexture(GL_TEXTURE_2D, 0);
}
} // namespace ui
} // namespace pvr
//!\endcond
