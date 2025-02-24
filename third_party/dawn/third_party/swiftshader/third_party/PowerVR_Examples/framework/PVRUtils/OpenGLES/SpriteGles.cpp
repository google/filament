/*!
\brief Contains implementations of functions for the Sprite_ class and subclasses (Sprite_, Text_, Image_, Font_,
MatrixGroup_)
\file PVRUtils/OpenGLES/SpriteGles.cpp
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
//!\cond NO_DOXYGEN
#include "SpriteGles.h"
#include "UIRendererGles.h"
#include "PVRCore/strings/StringHash.h"
#include "PVRUtils/OpenGLES/ErrorsGles.h"
#include "PVRCore/strings/UnicodeConverter.h"
#include "PVRCore/types/GpuDataTypes.h"

using glm::vec2;
using glm::vec3;
using glm::vec4;
using glm::ivec2;
using glm::ivec3;
using glm::ivec4;
using glm::uvec2;
using glm::uvec3;
using glm::uvec4;
using glm::mat3;
using glm::mat3x4;
using glm::mat4;

namespace pvr {
namespace ui {
namespace impl {
struct UboData
{
	mat4 mvp;
	mat4 uv;
	vec4 color;
	bool alphaMode;
};

Sprite_::Sprite_(UIRenderer& uiRenderer) : _color(1.f, 1.f, 1.f, 1.f), _alphaMode(false), _uiRenderer(&uiRenderer) { _boundingRect.clear(); }

void Sprite_::commitUpdates() const { calculateMvp(0, glm::mat4(1.f), _uiRenderer->getScreenRotation() * _uiRenderer->getProjection(), _uiRenderer->getViewport()); }

void Sprite_::render() const { onRender(0); }

void Image_::calculateMvp(uint64_t parentIds, glm::mat4 const& srt, const glm::mat4& viewProj, pvr::Rectanglei const& viewport) const
{
	if (_isPositioningDirty)
	{
		vec2 offset(0.0f); // offset the default center anchor point.

		switch (_anchor)
		{
		case Anchor::Center: break;
		case Anchor::TopLeft: offset = vec2(-1.f, 1.f); break;
		case Anchor::TopCenter: offset = vec2(0.0f, 1.f); break;
		case Anchor::TopRight: offset = vec2(1.f, 1.f); break;
		case Anchor::BottomLeft: offset = vec2(-1.f, -1.f); break;
		case Anchor::BottomCenter: offset = vec2(0.0f, -1.f); break;
		case Anchor::BottomRight: offset = vec2(1.f, -1.f); break;
		case Anchor::CenterLeft: offset = vec2(-1.f, 0.0f); break;
		case Anchor::CenterRight: offset = vec2(1.f, 0.0f); break;
		}

		memset(glm::value_ptr(_cachedMatrix), 0, sizeof(_cachedMatrix));
		//_matrix[0][0] = Will be set in Scale
		//_matrix[1][1] = Will be set in Scale
		_cachedMatrix[2][2] = 1.f; // Does not really matter - we don't have width...
		_cachedMatrix[3][3] = 1.f;
		// READ THIS BOTTOM TO TOP DUE TO THE WAY THE OPTIMISED GLM FUNCTIONS WORK

		// 4: Transform to SCREEN coordinates...
		// BECAUSE _cachedMatrix IS A PURE ROTATION, WE CAN OPTIMISE THE 1st SCALING OP.
		// THIS IS : _matrix = scale(_matrix, toScreenCoordinates);
		_cachedMatrix[0][0] = 1.f;
		_cachedMatrix[1][1] = 1.f;

		// 3: Rotate...
		_cachedMatrix = glm::rotate(_cachedMatrix, _rotation, vec3(0.f, 0.f, 1.f));

		// 2: Scale...
		_cachedMatrix = glm::scale(_cachedMatrix, vec3(_scale.x * getWidth() * .5f, _scale.y * getHeight() * .5f, 1.f));

		// 1: Apply the offsetting (i.e. place the center at its correct spot. THIS IS NOT THE SCREEN POSITIONING, only the anchor.)
		_cachedMatrix = glm::translate(_cachedMatrix, vec3(-offset, 0.0f));
		_isPositioningDirty = false;
	}

	glm::vec2 tmpPos;
	// 5: Translate (screen coords)
	tmpPos.x = _position.x * viewport.width * .5f + viewport.width * .5f + viewport.x + _pixelOffset.x;
	tmpPos.y = _position.y * viewport.height * .5f + viewport.height * .5f + viewport.y + _pixelOffset.y;
	_mvpData[parentIds].mvp = viewProj * srt * glm::translate(glm::vec3(tmpPos, 0.0f)) * _cachedMatrix;
}

void Image_::onRender(uint64_t parentId) const
{
	debugThrowOnApiError("Image_::onRender Enter");

	if (_uiRenderer->_uiStateTracker.activeTextureUnit != GL_TEXTURE7)
	{
		gl::ActiveTexture(GL_TEXTURE7);
		_uiRenderer->_uiStateTracker.activeTextureUnitChanged = true;
		_uiRenderer->_uiStateTracker.activeTextureUnit = GL_TEXTURE7;
	}

	if (static_cast<GLuint>(_uiRenderer->_uiStateTracker.boundTexture) != getTexture() || _uiRenderer->_uiStateTracker.activeTextureUnitChanged)
	{
		debugThrowOnApiError("Image_::onRender bind texture");
		gl::BindTexture(GL_TEXTURE_2D, getTexture());
		_uiRenderer->_uiStateTracker.boundTexture = getTexture();
		_uiRenderer->_uiStateTracker.boundTextureChanged = true;
	}

	if (_uiRenderer->getApiVersion() > Api::OpenGLES2)
	{
		_uiRenderer->_uiStateTracker.sampler7 = getSampler();
		gl::BindSampler(7, getSampler());
		debugThrowOnApiError("Image_::onRender bind sampler");
	}

	GLuint vbo = _uiRenderer->getImageVbo();
	debugThrowOnApiError("Image_::onRender getImageVbo");
	_uiRenderer->_uiStateTracker.vbo = vbo;
	gl::BindBuffer(GL_ARRAY_BUFFER, vbo);
	debugThrowOnApiError("Image_::onRender bind vbo");

	_uiRenderer->_uiStateTracker.vertexAttribBindings[0] = 0;
	_uiRenderer->_uiStateTracker.vertexAttribSizes[0] = 4;
	_uiRenderer->_uiStateTracker.vertexAttribTypes[0] = GL_FLOAT;
	_uiRenderer->_uiStateTracker.vertexAttribNormalized[0] = GL_FALSE;
	_uiRenderer->_uiStateTracker.vertexAttribStride[0] = sizeof(float) * 6;
	_uiRenderer->_uiStateTracker.vertexAttribOffset[0] = NULL;

	gl::VertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(float) * 6, NULL); // myVertex
	debugThrowOnApiError("Image_::onRender vertexattribptr 0");

	_uiRenderer->_uiStateTracker.vertexAttribBindings[1] = 1;
	_uiRenderer->_uiStateTracker.vertexAttribSizes[1] = 2;
	_uiRenderer->_uiStateTracker.vertexAttribTypes[1] = GL_FLOAT;
	_uiRenderer->_uiStateTracker.vertexAttribNormalized[1] = GL_FALSE;
	_uiRenderer->_uiStateTracker.vertexAttribStride[1] = sizeof(float) * 6;
	_uiRenderer->_uiStateTracker.vertexAttribOffset[1] = reinterpret_cast<GLvoid*>(sizeof(float) * 4);

	gl::VertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 6, reinterpret_cast<const void*>(sizeof(float) * 4)); // myUv
	debugThrowOnApiError("Image_::onRender vertexattribptr 1");

	gl::UniformMatrix4fv(_uiRenderer->getProgramData().uniforms[UIRenderer::ProgramData::UniformMVPmtx], 1, GL_FALSE, glm::value_ptr(_mvpData[parentId].mvp));
	debugThrowOnApiError("Image_::onRender uniform 0");
	gl::Uniform4fv(_uiRenderer->getProgramData().uniforms[UIRenderer::ProgramData::UniformColor], 1, glm::value_ptr(_color));
	debugThrowOnApiError("Image_::onRender uniform 1");
	gl::Uniform1i(_uiRenderer->getProgramData().uniforms[UIRenderer::ProgramData::UniformAlphaMode], _alphaMode);
	debugThrowOnApiError("Image_::onRender uniform 2");
	gl::UniformMatrix4fv(_uiRenderer->getProgramData().uniforms[UIRenderer::ProgramData::UniformUVmtx], 1, GL_FALSE,
		glm::value_ptr(glm::translate(glm::vec3(_uv.x, _uv.y, 0.0f)) * glm::scale(glm::vec3(_uv.width, _uv.height, 1.0f))));
	debugThrowOnApiError("Image_::onRender uniform 3");

	gl::DrawArrays(GL_TRIANGLES, 0, 6);
	debugThrowOnApiError("Image_::onRender draw");
}

Image_::Image_(make_shared_enabler, UIRenderer& uiRenderer, const GLuint& texture, uint32_t width, uint32_t height, bool useMipmaps, const GLuint& sampler)
	: Sprite_(uiRenderer), _texW(width), _texH(height), _texture(texture), _sampler(sampler), _isTextureDirty(true)
{
	if (_uiRenderer->_uiStateTracker.activeTextureUnit != GL_TEXTURE7)
	{
		gl::ActiveTexture(GL_TEXTURE7);
		_uiRenderer->_uiStateTracker.activeTextureUnitChanged = true;
		_uiRenderer->_uiStateTracker.activeTextureUnit = GL_TEXTURE7;
	}

	if (static_cast<GLuint>(_uiRenderer->_uiStateTracker.boundTexture) != _texture || _uiRenderer->_uiStateTracker.activeTextureUnitChanged)
	{
		_uiRenderer->_uiStateTracker.boundTexture = _texture;
		gl::BindTexture(GL_TEXTURE_2D, _texture);
		_uiRenderer->_uiStateTracker.boundTextureChanged = true;
	}

	if (_uiRenderer->getApiVersion() > Api::OpenGLES2 && _sampler != 0)
	{
		_sampler = useMipmaps ? uiRenderer.getSamplerTrilinear() : uiRenderer.getSamplerBilinear();
		gl::BindSampler(7, _sampler);
		_uiRenderer->_uiStateTracker.sampler7 = _sampler;
	}
	_boundingRect.setMinMax(glm::vec3(width * -.5f, height * -.5f, 0.0f), glm::vec3(width * .5f, height * .5f, 0.0f));
}

void Font_::loadFontData(const Texture& texture)
{
	const TextureHeader& texHeader = texture;
	_dim.x = texHeader.getWidth();
	_dim.y = texHeader.getHeight();

	const Header* header = reinterpret_cast<const Header*>(texture.getMetaDataMap()->at(TextureHeader::PVRv3).at(FontHeader).getData());

	_header = *header;
	_header.numCharacters = _header.numCharacters & 0xFFFF;
	_header.numKerningPairs = _header.numKerningPairs & 0xFFFF;

	const std::map<uint32_t, TextureMetaData>& metaDataMap = texture.getMetaDataMap()->at(TextureHeader::PVRv3);
	std::map<uint32_t, TextureMetaData>::const_iterator found;

	if (_header.numCharacters)
	{
		_characters.resize(_header.numCharacters);
		found = metaDataMap.find(FontCharList);

		if (found != metaDataMap.end()) { memcpy(&_characters[0], found->second.getData(), found->second.getDataSize()); }

		_yOffsets.resize(_header.numCharacters);
		found = metaDataMap.find(FontYoffset);

		if (found != metaDataMap.end()) { memcpy(&_yOffsets[0], found->second.getData(), found->second.getDataSize()); }

		_charMetrics.resize(_header.numCharacters);
		found = metaDataMap.find(FontMetrics);

		if (found != metaDataMap.end()) { memcpy(&_charMetrics[0], found->second.getData(), found->second.getDataSize()); }

		_rects.resize(_header.numCharacters);
		found = metaDataMap.find(FontRects);

		if (found != metaDataMap.end()) { memcpy(&_rects[0], found->second.getData(), found->second.getDataSize()); }

		// Build UVs
		_characterUVs.resize(_header.numCharacters);
		for (int16_t uiChar = 0; uiChar < _header.numCharacters; uiChar++)
		{
			_characterUVs[uiChar].ul = _rects[uiChar].x / static_cast<float>(_dim.x);
			_characterUVs[uiChar].ur = _characterUVs[uiChar].ul + _rects[uiChar].width / static_cast<float>(_dim.x);
			_characterUVs[uiChar].vt = _rects[uiChar].y / static_cast<float>(_dim.y);
			_characterUVs[uiChar].vb = _characterUVs[uiChar].vt + _rects[uiChar].height / static_cast<float>(_dim.y);
		}
	}

	if (_header.numKerningPairs)
	{
		found = metaDataMap.find(FontKerning);
		_kerningPairs.resize(_header.numKerningPairs);

		if (found != metaDataMap.end()) { memcpy(&_kerningPairs[0], found->second.getData(), found->second.getDataSize()); }
	}
}

uint32_t Font_::findCharacter(uint32_t character) const
{
	uint32_t* item = reinterpret_cast<uint32_t*>(bsearch(&character, &_characters[0], _characters.size(), sizeof(uint32_t), characterCompFunc));

	if (!item) { return InvalidChar; }

	uint32_t index = static_cast<uint32_t>(item - &_characters[0]);
	return index;
}

void Font_::applyKerning(uint32_t charA, uint32_t charB, float& offset)
{
	if (_kerningPairs.size())
	{
		uint64_t uiPairToSearch = (static_cast<uint64_t>(charA) << 32) | static_cast<uint64_t>(charB);
		KerningPair* pItem = (KerningPair*)bsearch(&uiPairToSearch, &_kerningPairs[0], _kerningPairs.size(), sizeof(KerningPair), kerningCompFunc);

		if (pItem) { offset += static_cast<float>(pItem->offset); }
	}
}

int32_t Font_::characterCompFunc(const void* a, const void* b) { return (*static_cast<const int32_t*>(a) - *static_cast<const int32_t*>(b)); }

int32_t Font_::kerningCompFunc(const void* a, const void* b)
{
	KerningPair* pPairA = (KerningPair*)a;
	KerningPair* pPairB = (KerningPair*)b;

	if (pPairA->pair > pPairB->pair) { return 1; }

	if (pPairA->pair < pPairB->pair) { return -1; }

	return 0;
}

Font_::Font_(make_shared_enabler, UIRenderer& uiRenderer, const GLuint& tex2D, const Texture& tex, const GLuint& sampler)
{
	setUIRenderer(&uiRenderer);
	_texture = tex2D;
	if (uiRenderer.getApiVersion() > Api::OpenGLES2)
	{
		_sampler = (sampler != 0) ? sampler : uiRenderer.getSamplerBilinear();
		gl::BindSampler(7, _sampler);
		_uiRenderer->_uiStateTracker.sampler7 = static_cast<GLint>(_sampler);
	}
	else
	{
		gl::BindTexture(GL_TEXTURE_2D, tex2D);
		gl::TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		gl::TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		gl::TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	}

	if ((tex.getPixelFormat().getNumChannels() == 1 && tex.getPixelFormat().getChannelContent(0) == 'a') ||
		(tex.getPixelFormat().getNumChannels() == 4 && tex.getPixelFormat().getChannelContent(3) == 'a'))
	{ _alphaRenderingMode = true; }
	loadFontData(tex);
}

uint32_t TextElement_::updateVertices(float fZPos, float xPos, float yPos, const std::vector<uint32_t>& text, Vertex* const pVertices) const
{
	if (pVertices == NULL || text.empty()) { return 0; }
	_boundingRect.clear();
	/* Nothing to update */

	Font tmp = _font;
	Font_& font = *tmp;

	yPos -= font.getAscent();

	yPos = glm::round(yPos);

	float preXPos = xPos; // The original offset (after screen scale modification) of the X coordinate.

	float kernOffset;
	float fAOff;
	float fYOffset;
	uint32_t vertexCount = 0;
	int32_t nextChar;

	size_t numCharsInString = text.size();

	for (size_t index = 0; index < numCharsInString; index++)
	{
		if (index > MaxLetters) { break; }

		// Newline
		if (text[index] == 0x0A)
		{
			xPos = preXPos;
			yPos -= glm::round(static_cast<float>(font.getFontLineSpacing()));
			continue;
		}

		// Get the character
		uint32_t charIndex = font.findCharacter(text[index]);

		// No character found. Add a space.
		if (charIndex == Font_::InvalidChar)
		{
			xPos += glm::round(static_cast<float>(font.getSpaceWidth()));
			continue;
		}

		kernOffset = 0;
		fYOffset = static_cast<float>(font.getYOffset(charIndex));
		// The A offset. Could include overhang or underhang.
		fAOff = glm::round(static_cast<float>(font.getCharMetrics(charIndex).xOff));

		if (index < numCharsInString - 1)
		{
			nextChar = text[index + 1];
			font.applyKerning(text[index], nextChar, kernOffset);
		}

		const Font_::CharacterUV& charUV = font.getCharacterUV(charIndex);
		/* Filling vertex data */
		pVertices[vertexCount + 0].x = (xPos + fAOff);
		pVertices[vertexCount + 0].y = (yPos + fYOffset);
		pVertices[vertexCount + 0].z = (fZPos);
		pVertices[vertexCount + 0].rhw = (1.0f);
		pVertices[vertexCount + 0].tu = (charUV.ul);
		pVertices[vertexCount + 0].tv = (charUV.vt);
		_boundingRect.add(pVertices[vertexCount + 0].x, pVertices[vertexCount + 0].y, 0.0f);

		pVertices[vertexCount + 1].x = (xPos + fAOff + glm::round(static_cast<float>(font.getRectangle(charIndex).width)));
		pVertices[vertexCount + 1].y = (yPos + fYOffset);
		pVertices[vertexCount + 1].z = (fZPos);
		pVertices[vertexCount + 1].rhw = (1.0f);
		pVertices[vertexCount + 1].tu = (charUV.ur);
		pVertices[vertexCount + 1].tv = (charUV.vt);
		_boundingRect.add(pVertices[vertexCount + 1].x, pVertices[vertexCount + 1].y, 0.0f);

		pVertices[vertexCount + 2].x = (xPos + fAOff);
		pVertices[vertexCount + 2].y = (yPos + fYOffset - glm::round(static_cast<float>(font.getRectangle(charIndex).height)));
		pVertices[vertexCount + 2].z = (fZPos);
		pVertices[vertexCount + 2].rhw = (1.0f);
		pVertices[vertexCount + 2].tu = (charUV.ul);
		pVertices[vertexCount + 2].tv = (charUV.vb);
		_boundingRect.add(pVertices[vertexCount + 2].x, pVertices[vertexCount + 2].y, 0.0f);

		pVertices[vertexCount + 3].x = (xPos + fAOff + glm::round(static_cast<float>(font.getRectangle(charIndex).width)));
		pVertices[vertexCount + 3].y = (yPos + fYOffset - glm::round(static_cast<float>(font.getRectangle(charIndex).height)));
		pVertices[vertexCount + 3].z = (fZPos);
		pVertices[vertexCount + 3].rhw = (1.0f);
		pVertices[vertexCount + 3].tu = (charUV.ur);
		pVertices[vertexCount + 3].tv = (charUV.vb);
		_boundingRect.add(pVertices[vertexCount + 3].x, pVertices[vertexCount + 3].y, 0.0f);

		// Add on this characters width
		xPos = xPos + glm::round(static_cast<float>(font.getCharMetrics(charIndex).characterWidth + kernOffset) /** renderParam.scale.x*/);
		vertexCount += 4;
	}
	return vertexCount;
}

void TextElement_::regenerateText() const
{
	debugThrowOnApiError("TextElement_::regenerateText enter");
	_utf32.clear();
	if (_isUtf8) { utils::UnicodeConverter::convertUTF8ToUTF32(reinterpret_cast<const utf8*>(_textStr.c_str()), _utf32); }
	else
	{
		if (sizeof(wchar_t) == 2 && _textWStr.length()) { utils::UnicodeConverter::convertUTF16ToUTF32((const utf16*)_textWStr.c_str(), _utf32); }
		else if (_textWStr.length()) // if (sizeof(wchar_t) == 4)
		{
			_utf32.resize(_textWStr.size());
			memcpy(&_utf32[0], &_textWStr[0], _textWStr.size() * sizeof(_textWStr[0]));
		}
	}

	_vertices.clear();
	if (_vertices.size() < (_utf32.size() * 4)) { _vertices.resize(_utf32.size() * 4); }

	_numCachedVerts = updateVertices(0.0f, 0.f, 0.f, _utf32, _vertices.size() ? &_vertices[0] : 0);
	assertion((_numCachedVerts % 4) == 0);
	assertion((_numCachedVerts / 4) < MaxLetters);
	_isTextDirty = false;
	debugThrowOnApiError("TextElement_::regenerateText exit");
}

void TextElement_::updateVbo() const
{
	debugThrowOnApiError("TextElement_::updateVbo enter");
	if (_vertices.size())
	{
		if (!_vboCreated)
		{
			gl::GenBuffers(1, &_vbo);
			_vboCreated = true;
		}
		_uiRenderer->_uiStateTracker.vbo = _vbo;
		gl::BindBuffer(GL_ARRAY_BUFFER, _vbo);
		gl::BufferData(GL_ARRAY_BUFFER, (static_cast<uint32_t>(sizeof(Vertex) * _vertices.size())), _vertices.data(), GL_STATIC_DRAW);

		// rebind previous buffer so state remains the same
		if (_uiRenderer->_uiStateTracker.vbo != _uiRenderer->_currentState.vbo) { gl::BindBuffer(GL_ARRAY_BUFFER, _uiRenderer->_currentState.vbo); }
	}
	debugThrowOnApiError("TextElement_::updateVbo exit");
}

void TextElement_::onRender() const
{
	if (_vboCreated)
	{
		_uiRenderer->_uiStateTracker.vbo = _vbo;
		_uiRenderer->_uiStateTracker.ibo = _uiRenderer->getFontIbo();
		gl::BindBuffer(GL_ARRAY_BUFFER, _vbo);
		gl::BindBuffer(GL_ELEMENT_ARRAY_BUFFER, _uiRenderer->getFontIbo());

		_uiRenderer->_uiStateTracker.vertexAttribBindings[0] = 0;
		_uiRenderer->_uiStateTracker.vertexAttribSizes[0] = 4;
		_uiRenderer->_uiStateTracker.vertexAttribTypes[0] = GL_FLOAT;
		_uiRenderer->_uiStateTracker.vertexAttribNormalized[0] = GL_FALSE;
		_uiRenderer->_uiStateTracker.vertexAttribStride[0] = sizeof(float) * 6;
		_uiRenderer->_uiStateTracker.vertexAttribOffset[0] = NULL;

		gl::VertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(float) * 6, NULL); // myVertex

		_uiRenderer->_uiStateTracker.vertexAttribBindings[1] = 1;
		_uiRenderer->_uiStateTracker.vertexAttribSizes[1] = 2;
		_uiRenderer->_uiStateTracker.vertexAttribTypes[1] = GL_FLOAT;
		_uiRenderer->_uiStateTracker.vertexAttribNormalized[1] = GL_FALSE;
		_uiRenderer->_uiStateTracker.vertexAttribStride[1] = sizeof(float) * 6;
		_uiRenderer->_uiStateTracker.vertexAttribOffset[1] = reinterpret_cast<GLvoid*>(sizeof(float) * 4);

		gl::VertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 6, reinterpret_cast<const void*>(sizeof(float) * 4)); // myUv

		gl::DrawElements(GL_TRIANGLES, (glm::min<int32_t>(_numCachedVerts, 0xFFFC) >> 1) * 3, GL_UNSIGNED_SHORT, 0);
	}
}

void Text_::calculateMvp(uint64_t parentIds, glm::mat4 const& srt, const glm::mat4& viewProj, pvr::Rectanglei const& viewport) const
{
	_textElement->updateText();
	if (_isPositioningDirty || _boundingRect != _textElement->getBoundingBox())
	{
		_boundingRect = _textElement->getBoundingBox();
		vec2 offset;

		switch (_anchor)
		{
		case Anchor::Center: offset = vec2(_boundingRect.center()); break;
		case Anchor::TopLeft: offset = vec2(_boundingRect.topLeftNear()); break;
		case Anchor::TopCenter: offset = vec2(_boundingRect.topCenterNear()); break;
		case Anchor::TopRight: offset = vec2(_boundingRect.topRightNear()); break;
		case Anchor::BottomLeft: offset = vec2(_boundingRect.bottomLeftNear()); break;
		case Anchor::BottomCenter: offset = vec2(_boundingRect.bottomCenterNear()); break;
		case Anchor::BottomRight: offset = vec2(_boundingRect.bottomRightNear()); break;
		case Anchor::CenterLeft: offset = vec2(_boundingRect.centerLeftNear()); break;
		case Anchor::CenterRight: offset = vec2(_boundingRect.centerRightNear()); break;
		}

		// Read the following bottom to top - this is because of how the optimised GLM functions work

		// We are assuming the identity matrix so we can optimise out the operations
		_cachedMatrix = glm::mat4(1.f);

		// 5: Translate - move it to its position - we are assuming the identity matrix so we can optimise out the operations
		//_cachedMatrix = translate(vec3(_position.x, _position.y, 0.f));

		// 4: Bring to Pixel (screen) coordinates. Because the matrix is a pure rotation we can optimise our the 1st scaling operation
		// glm::vec3 toScreenCoordinates(1.f / _uiRenderer->getRenderingDimX(), 1.f / _uiRenderer->getRenderingDimY(), 1.f);
		// _cachedMatrix = scale(_cachedMatrix, toScreenCoordinates);

		// 3: Rotate - do the rotation around the anchor
		_cachedMatrix = glm::rotate(_cachedMatrix, _rotation, vec3(0.f, 0.f, 1.f));

		// 2: Scale - do the scale around the anchor
		_cachedMatrix = glm::scale(_cachedMatrix, vec3(_scale, 1.f));

		// 1: Anchor the text properly - translate the anchor to the origin
		_cachedMatrix = glm::translate(_cachedMatrix, vec3(-offset, 0.f));
		_isPositioningDirty = false;
	}

	glm::vec2 tmpPos;
	tmpPos.x = _position.x * viewport.width * .5f + viewport.width * .5f;
	tmpPos.y = _position.y * viewport.height * .5f + viewport.height * .5f;

	tmpPos.x += viewport.x + _pixelOffset.x;
	tmpPos.y += viewport.y + _pixelOffset.y;

	_mvpData[parentIds].mvp = viewProj * srt * glm::translate(glm::vec3(tmpPos, 0.0f)) * _cachedMatrix;
}

void Text_::onRender(uint64_t parentId) const
{
	if (_uiRenderer->getApiVersion() > Api::OpenGLES2)
	{
		if (static_cast<GLuint>(_uiRenderer->_uiStateTracker.sampler7) != getFont()->getSampler())
		{
			gl::BindSampler(7, getFont()->getSampler());
			_uiRenderer->_uiStateTracker.sampler7 = getFont()->getSampler();
			_uiRenderer->_uiStateTracker.sampler7Changed = true;
		}
	}

	if (_uiRenderer->_uiStateTracker.activeTextureUnit != GL_TEXTURE7)
	{
		gl::ActiveTexture(GL_TEXTURE7);
		_uiRenderer->_uiStateTracker.activeTextureUnitChanged = true;
		_uiRenderer->_uiStateTracker.activeTextureUnit = GL_TEXTURE7;
	}

	if (static_cast<GLuint>(_uiRenderer->_uiStateTracker.boundTexture) != getFont()->getTexture() || _uiRenderer->_uiStateTracker.activeTextureUnitChanged)
	{
		gl::BindTexture(GL_TEXTURE_2D, getFont()->getTexture());
		_uiRenderer->_uiStateTracker.boundTexture = getFont()->getTexture();
		_uiRenderer->_uiStateTracker.boundTextureChanged = true;
	}

	gl::UniformMatrix4fv(_uiRenderer->getProgramData().uniforms[UIRenderer::ProgramData::UniformMVPmtx], 1, GL_FALSE, glm::value_ptr(_mvpData[parentId].mvp));
	gl::Uniform4fv(_uiRenderer->getProgramData().uniforms[UIRenderer::ProgramData::UniformColor], 1, glm::value_ptr(_color));
	gl::Uniform1i(_uiRenderer->getProgramData().uniforms[UIRenderer::ProgramData::UniformAlphaMode], _alphaMode);
	gl::UniformMatrix4fv(_uiRenderer->getProgramData().uniforms[UIRenderer::ProgramData::UniformUVmtx], 1, GL_FALSE, glm::value_ptr(mat4(1.)));

	_textElement->onRender();
}

Text_::Text_(make_shared_enabler, UIRenderer& uiRenderer, const TextElement& textElement) : Sprite_(uiRenderer), _textElement(textElement)
{
	_alphaMode = _textElement->getFont()->isAlphaRendering();
}

/// <summary>You must always submit your outstanding operations to a texture before calling setText. Because
/// setText will edit the content of VBOs and similar, these must be submitted before changing the text. To avoid
/// that, prefer using more Text objects.</summary>
TextElement_& TextElement_::setText(const std::string& str)
{
	_isTextDirty = true;
	_isUtf8 = true;
	_textStr.assign(str);
	// check if need reallocation
	return *this;
}

TextElement_& TextElement_::setText(const std::wstring& str)
{
	_isTextDirty = true;
	_isUtf8 = false;
	_textStr.clear();
	_textWStr = str;
	return *this;
	// check if need reallocation
}

TextElement_& TextElement_::setText(std::string&& str)
{
	_isTextDirty = true;
	_isUtf8 = true;
	_textWStr.clear();
	_textStr = std::move(str);
	return *this;
}

TextElement_& TextElement_::setText(std::wstring&& str)
{
	_isTextDirty = true;
	_isUtf8 = false;
	_textStr.clear();
	_textWStr = std::move(str);
	return *this;
}

MatrixGroup_::MatrixGroup_(make_shared_enabler, UIRenderer& uiRenderer, uint64_t id) : Group_(uiRenderer, id) {}

void MatrixGroup_::commitUpdates() const { calculateMvp(0, glm::mat4(1.f), _uiRenderer->getScreenRotation() * _viewProj, _uiRenderer->getViewport()); }

void PixelGroup_::calculateMvp(uint64_t parentIds, const glm::mat4& srt, const glm::mat4& viewProj, pvr::Rectanglei const& viewport) const
{
	vec2 offset(_boundingRect.center()); // offset the default center anchor point.

	switch (_anchor)
	{
	case Anchor::Center: break;
	case Anchor::TopLeft: offset = glm::vec2(_boundingRect.topLeftNear()); break;
	case Anchor::TopCenter: offset = glm::vec2(_boundingRect.topCenterNear()); break;
	case Anchor::TopRight: offset = glm::vec2(_boundingRect.topRightNear()); break;
	case Anchor::BottomLeft: offset = glm::vec2(_boundingRect.bottomLeftNear()); break;
	case Anchor::BottomCenter: offset = glm::vec2(_boundingRect.bottomCenterNear()); break;
	case Anchor::BottomRight: offset = glm::vec2(_boundingRect.bottomRightNear()); break;
	case Anchor::CenterLeft: offset = glm::vec2(_boundingRect.centerLeftNear()); break;
	case Anchor::CenterRight: offset = glm::vec2(_boundingRect.centerRightNear()); break;
	}

	memset(glm::value_ptr(_cachedMatrix), 0, sizeof(_cachedMatrix));
	_cachedMatrix[0][0] = 1.f;
	_cachedMatrix[1][1] = 1.f;
	_cachedMatrix[2][2] = 1.f; // Does not really matter - we don't have width...
	_cachedMatrix[3][3] = 1.f;

	//*** READ THIS BOTTOM TO TOP DUE TO THE WAY THE OPTIMISED GLM FUNCTIONS WORK
	//- translate the anchor to the origin
	//- do the scale and then the rotation around the anchor
	//- do the final translation
	glm::vec2 tmpPos;
	// tranform from ndc to screen space
	tmpPos.x = static_cast<float>(math::ndcToPixel(_position.x, viewport.width));
	tmpPos.y = static_cast<float>(math::ndcToPixel(_position.y, viewport.height));
	// add the final pixel offset
	tmpPos.x += static_cast<float>(_pixelOffset.x) + static_cast<float>(viewport.x);
	tmpPos.y += static_cast<float>(_pixelOffset.y) + static_cast<float>(viewport.y);

	_cachedMatrix[3][0] = tmpPos.x;
	_cachedMatrix[3][1] = tmpPos.y;

	_cachedMatrix = glm::rotate(_cachedMatrix, _rotation, glm::vec3(0.f, 0.f, 1.f));
	_cachedMatrix = glm::scale(_cachedMatrix, glm::vec3(_scale, 1.f));
	_cachedMatrix = glm::translate(_cachedMatrix, glm::vec3(-offset, 0.0f));

	glm::mat4 tmpMatrix = srt * _cachedMatrix;
	// My cached matrix should always be up-to-date unless overridden. No effect.
	for (ChildContainer::iterator it = _children.begin(); it != _children.end(); ++it)
	{
		(*it)->calculateMvp(
			packId(parentIds, _id), tmpMatrix, viewProj, Rectanglei(0, 0, static_cast<int32_t>(_boundingRect.getSize().x), static_cast<int32_t>(_boundingRect.getSize().y)));
	}
}

Group_* Group_::add(const Sprite& sprite)
{
	_children.emplace_back(sprite);
	_boundingRect.add(sprite->getDimensions().x, sprite->getDimensions().y, 0.0f);
	return this;
}
} // namespace impl
} // namespace ui
} // namespace pvr
//!\endcond
