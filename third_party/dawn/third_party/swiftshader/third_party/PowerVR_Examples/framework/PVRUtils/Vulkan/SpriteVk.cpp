/*!
\brief Contains implementations of functions for the Sprite_ class and subclasses (Sprite_, Text_, Image_, Font_,
MatrixGroup_)
\file PVRUtils/Vulkan/SpriteVk.cpp
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
//!\cond NO_DOXYGEN

#include "PVRUtils/Vulkan/SpriteVk.h"
#include "PVRUtils/Vulkan/UIRendererVk.h"
#include "PVRVk/DescriptorSetVk.h"
#include "PVRVk/SamplerVk.h"
#include "PVRVk/ImageVk.h"
#include "PVRCore/strings/UnicodeConverter.h"

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
using namespace ::pvrvk;
namespace impl {
struct UboData
{
	glm::mat4 mvp;
	glm::mat4 uv;
	glm::vec4 color;
	bool alphaMode;
};

Sprite_::Sprite_(UIRenderer& uiRenderer) : _color(1.f, 1.f, 1.f, 1.f), _alphaMode(false), _uiRenderer(&uiRenderer), _spriteName("") { _boundingRect.clear(); }

void Sprite_::commitUpdates() const { calculateMvp(0, glm::mat4(1.f), _uiRenderer->getScreenRotation() * _uiRenderer->getProjection(), _uiRenderer->getViewport()); }

void Sprite_::render()
{
	if (!_uiRenderer->isRendering()) { throw UIRendererError("Sprite: Render called without first calling uiRenderer::begin to set up the commandbuffer."); }
	onRender(_uiRenderer->getActiveCommandBuffer(), 0);
}

void Image_::updateUbo(uint64_t parentIds) const
{
	glm::vec3 scale(_uv.getExtent().getWidth(), _uv.getExtent().getHeight(), 1.0f);
	glm::mat4 uvTrans = glm::translate(glm::vec3(_uv.getOffset().getX(), _uv.getOffset().getY(), 0.0f)) * glm::scale(scale);

	debug_assertion(_mvpData[parentIds].bufferArrayId != -1, "Invalid MVP Buffer ID");
	debug_assertion(_materialData.bufferArrayId != -1, "Invalid Material Buffer ID");
	// update the ubo
	_uiRenderer->getUbo().updateMvp(static_cast<uint32_t>(_mvpData[parentIds].bufferArrayId), _mvpData[parentIds].mvp);
	_uiRenderer->getMaterial().updateMaterial(static_cast<uint32_t>(_materialData.bufferArrayId), _color, _alphaMode, uvTrans);
}

void Image_::updateTextureDescriptorSet() const
{
	// update the texture descriptor set
	if (_isTextureDirty)
	{
		WriteDescriptorSet writeDescSet(pvrvk::DescriptorType::e_COMBINED_IMAGE_SAMPLER, _texDescSet, 0);
		writeDescSet.setImageInfo(0, DescriptorImageInfo(getImageView(), getSampler(), ImageLayout::e_SHADER_READ_ONLY_OPTIMAL));

		_uiRenderer->getDevice().lock()->updateDescriptorSets(&writeDescSet, 1, nullptr, 0);
		_isTextureDirty = false;
	}
}

void Image_::calculateMvp(uint64_t parentIds, const glm::mat4& srt, const glm::mat4& viewProj, const Rect2D& viewport) const
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
	tmpPos.x = _position.x * viewport.getExtent().getWidth() * .5f + viewport.getExtent().getWidth() * .5f + viewport.getOffset().getX() + _pixelOffset.x;
	tmpPos.y = _position.y * viewport.getExtent().getHeight() * .5f + viewport.getExtent().getHeight() * .5f + viewport.getOffset().getY() + _pixelOffset.y;
	_mvpData[parentIds].mvp = viewProj * srt * glm::translate(glm::vec3(tmpPos, 0.0f)) * _cachedMatrix;
	updateUbo(parentIds);
}

void Image_::onRender(CommandBufferBase& commandBuffer, uint64_t parentId)
{
	pvr::utils::beginCommandBufferDebugLabel(commandBuffer, pvrvk::DebugUtilsLabel("Rendering: (" + getSpriteName() + ")"));
	commandBuffer->bindDescriptorSet(PipelineBindPoint::e_GRAPHICS, _uiRenderer->getPipelineLayout(), 0, getTexDescriptorSet(), nullptr, 0);
	_uiRenderer->getUbo().bindUboDynamic(commandBuffer, _uiRenderer->getPipelineLayout(), static_cast<uint32_t>(_mvpData[parentId].bufferArrayId));
	_uiRenderer->getMaterial().bindUboDynamic(commandBuffer, _uiRenderer->getPipelineLayout(), static_cast<uint32_t>(_materialData.bufferArrayId));
	commandBuffer->bindVertexBuffer(_uiRenderer->getImageVbo(), 0, 0);
	commandBuffer->draw(0, 6);
	pvr::utils::endCommandBufferDebugLabel(commandBuffer);
}

void Image_::onAddInstance(uint64_t parentId)
{
	if (_mvpData[parentId].bufferArrayId == -1)
	{
		int32_t id = _uiRenderer->getUbo().getNewBufferSlice();
		if (id == -1) { throw UIRendererInstanceMaxError("Failed to create Image (UBO instance)."); }
		_mvpData[parentId].bufferArrayId = id;
	}
}

void Image_::onRemoveInstance(uint64_t parentId)
{
	if (_mvpData[parentId].bufferArrayId != -1)
	{
		_uiRenderer->getUbo().releaseBufferSlice(_mvpData[parentId].bufferArrayId);
		_uiRenderer->getMaterial().releaseBufferArray(_materialData.bufferArrayId);
		_mvpData[parentId].bufferArrayId = -1;
		_materialData.bufferArrayId = -1;
	}
}

Image_::Image_(make_shared_enabler, UIRenderer& uiRenderer, const ImageView& imageView, uint32_t width, uint32_t height, const Sampler& sampler)
	: Sprite_(uiRenderer), _texW(width), _texH(height), _imageView(imageView), _sampler(sampler), _isTextureDirty(true)
{
	if (!_sampler) { _sampler = imageView->getImage()->getNumMipLevels() > 1 ? uiRenderer.getSamplerTrilinear() : uiRenderer.getSamplerBilinear(); }
	_boundingRect.setMinMax(glm::vec3(width * -.5f, height * -.5f, 0.0f), glm::vec3(width * .5f, height * .5f, 0.0f));
	_texDescSet = uiRenderer.getDescriptorPool()->allocateDescriptorSet(uiRenderer.getTexDescriptorSetLayout());
	if (_materialData.bufferArrayId == -1 && (_materialData.bufferArrayId = _uiRenderer->getMaterial().getNewBufferArray()) == -1)
	{ throw std::runtime_error("Failed to create Image (Material instance)"); }
	onAddInstance(0);
}

void Font_::loadFontData(const Texture& texture)
{
	const TextureHeader& texHeader = texture;
	_dim.x = texHeader.getWidth();
	_dim.y = texHeader.getHeight();

	const Header* header = reinterpret_cast<const Header*>(texture.getMetaDataMap()->at(TextureHeader::PVRv3).at(static_cast<uint32_t>(FontHeader)).getData());
	assertion(header != NULL);

	_header = *header;
	_header.numCharacters = _header.numCharacters & 0xFFFF;
	_header.numKerningPairs = _header.numKerningPairs & 0xFFFF;

	const std::map<uint32_t, TextureMetaData>& metaDataMap = texture.getMetaDataMap()->at(TextureHeader::PVRv3);
	std::map<uint32_t, TextureMetaData>::const_iterator found;

	if (_header.numCharacters)
	{
		_characters.resize(_header.numCharacters);
		found = metaDataMap.find(static_cast<uint32_t>(FontCharList));

		if (found != metaDataMap.end()) { memcpy(&_characters[0], found->second.getData(), found->second.getDataSize()); }

		_yOffsets.resize(_header.numCharacters);
		found = metaDataMap.find(static_cast<uint32_t>(FontYoffset));

		if (found != metaDataMap.end()) { memcpy(&_yOffsets[0], found->second.getData(), found->second.getDataSize()); }

		_charMetrics.resize(_header.numCharacters);
		found = metaDataMap.find(static_cast<uint32_t>(FontMetrics));

		if (found != metaDataMap.end()) { memcpy(&_charMetrics[0], found->second.getData(), found->second.getDataSize()); }

		_rects.resize(_header.numCharacters);
		found = metaDataMap.find(static_cast<uint32_t>(FontRects));

		if (found != metaDataMap.end()) { memcpy(&_rects[0], found->second.getData(), found->second.getDataSize()); }

		// Build UVs
		_characterUVs.resize(_header.numCharacters);
		for (int16_t uiChar = 0; uiChar < _header.numCharacters; uiChar++)
		{
			_characterUVs[uiChar].ul = _rects[uiChar].getOffset().getX() / static_cast<float>(_dim.x);
			_characterUVs[uiChar].ur = _characterUVs[uiChar].ul + _rects[uiChar].getExtent().getWidth() / static_cast<float>(_dim.x);
			_characterUVs[uiChar].vt = _rects[uiChar].getOffset().getY() / static_cast<float>(_dim.y);
			_characterUVs[uiChar].vb = _characterUVs[uiChar].vt + _rects[uiChar].getExtent().getHeight() / static_cast<float>(_dim.y);
		}
	}

	if (_header.numKerningPairs)
	{
		found = metaDataMap.find(static_cast<uint32_t>(FontKerning));
		_kerningPairs.resize(_header.numKerningPairs);

		if (found != metaDataMap.end()) { memcpy(&_kerningPairs[0], found->second.getData(), found->second.getDataSize()); }
	}
}

uint32_t Font_::findCharacter(uint32_t character) const
{
	uint32_t* item = reinterpret_cast<uint32_t*>(bsearch(&character, &_characters[0], _characters.size(), sizeof(uint32_t), characterCompFunc));

	if (!item) { return static_cast<uint32_t>(InvalidChar); }

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

Font_::Font_(make_shared_enabler, UIRenderer& uiRenderer, const pvrvk::ImageView& tex2D, const TextureHeader& textureHeader, const pvrvk::Sampler& sampler)
{
	setUIRenderer(&uiRenderer);
	_imageView = tex2D;
	loadFontData(textureHeader);
	if (textureHeader.getPixelFormat().getNumChannels() == 1 && textureHeader.getPixelFormat().getChannelContent(0) == 'a') { _alphaRenderingMode = true; }

	_texDescSet = uiRenderer.getDescriptorPool()->allocateDescriptorSet(uiRenderer.getTexDescriptorSetLayout());
	// update the texture descriptor set
	WriteDescriptorSet writeDescSet(pvrvk::DescriptorType::e_COMBINED_IMAGE_SAMPLER, _texDescSet, 0, 0);
	writeDescSet.setImageInfo(0, DescriptorImageInfo(_imageView, (sampler ? sampler : uiRenderer.getSamplerBilinear()), pvrvk::ImageLayout::e_SHADER_READ_ONLY_OPTIMAL));
	uiRenderer.getDevice().lock()->updateDescriptorSets(&writeDescSet, 1, nullptr, 0);
}

uint32_t TextElement_::updateVertices(float fZPos, float xPos, float yPos, const std::vector<uint32_t>& text, Vertex* const pVertices) const
{
	if (pVertices == NULL || text.empty()) { return 0; }
	_boundingRect.clear();

	Font tmp = _font;
	Font_& font = *tmp;

	yPos -= font.getAscent();

	yPos = glm::round(yPos);

	// The original offset (after screen scale modification) of the X coordinate.
	float preXPos = xPos;

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
		// Filling vertex data
		pVertices[vertexCount + 0].x = (xPos + fAOff);
		pVertices[vertexCount + 0].y = (yPos + fYOffset);
		pVertices[vertexCount + 0].z = (fZPos);
		pVertices[vertexCount + 0].rhw = (1.0f);
		pVertices[vertexCount + 0].tu = (charUV.ul);
		pVertices[vertexCount + 0].tv = (charUV.vt);
		_boundingRect.add(pVertices[vertexCount + 0].x, pVertices[vertexCount + 0].y, 0.0f);

		pVertices[vertexCount + 1].x = (xPos + fAOff + glm::round(static_cast<float>(font.getRectangle(charIndex).getExtent().getWidth())));
		pVertices[vertexCount + 1].y = (yPos + fYOffset);
		pVertices[vertexCount + 1].z = (fZPos);
		pVertices[vertexCount + 1].rhw = (1.0f);
		pVertices[vertexCount + 1].tu = (charUV.ur);
		pVertices[vertexCount + 1].tv = (charUV.vt);
		_boundingRect.add(pVertices[vertexCount + 1].x, pVertices[vertexCount + 1].y, 0.0f);

		pVertices[vertexCount + 2].x = (xPos + fAOff);
		pVertices[vertexCount + 2].y = (yPos + fYOffset - glm::round(static_cast<float>(font.getRectangle(charIndex).getExtent().getHeight())));
		pVertices[vertexCount + 2].z = (fZPos);
		pVertices[vertexCount + 2].rhw = (1.0f);
		pVertices[vertexCount + 2].tu = (charUV.ul);
		pVertices[vertexCount + 2].tv = (charUV.vb);
		_boundingRect.add(pVertices[vertexCount + 2].x, pVertices[vertexCount + 2].y, 0.0f);

		pVertices[vertexCount + 3].x = (xPos + fAOff + glm::round(static_cast<float>(font.getRectangle(charIndex).getExtent().getWidth())));
		pVertices[vertexCount + 3].y = (yPos + fYOffset - round(static_cast<float>(font.getRectangle(charIndex).getExtent().getHeight())));
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

void Text_::onAddInstance(uint64_t parentId)
{
	if (_mvpData[parentId].bufferArrayId == -1)
	{
		if ((_mvpData[parentId].bufferArrayId = _uiRenderer->getUbo().getNewBufferSlice()) == -1)
		{ throw UIRendererInstanceMaxError("Failed to create Text (Material instances)"); }
	}
}

void TextElement_::createBuffers()
{
	_vbo = utils::createBuffer(_uiRenderer->getDevice().lock(),
		pvrvk::BufferCreateInfo(static_cast<uint32_t>(sizeof(Vertex) * _maxLength * 4), pvrvk::BufferUsageFlags::e_VERTEX_BUFFER_BIT), pvrvk::MemoryPropertyFlags::e_HOST_VISIBLE_BIT,
		pvrvk::MemoryPropertyFlags::e_HOST_VISIBLE_BIT, _uiRenderer->getMemoryAllocator(), pvr::utils::vma::AllocationCreateFlags::e_MAPPED_BIT);

	_drawIndirectBuffer = utils::createBuffer(_uiRenderer->getDevice().lock(),
		pvrvk::BufferCreateInfo(sizeof(VkDrawIndexedIndirectCommand), pvrvk::BufferUsageFlags::e_INDIRECT_BUFFER_BIT), pvrvk::MemoryPropertyFlags::e_HOST_VISIBLE_BIT,
		pvrvk::MemoryPropertyFlags::e_HOST_VISIBLE_BIT | pvrvk::MemoryPropertyFlags::e_HOST_COHERENT_BIT | pvrvk::MemoryPropertyFlags::e_DEVICE_LOCAL_BIT |
			pvrvk::MemoryPropertyFlags::e_HOST_CACHED_BIT,
		_uiRenderer->getMemoryAllocator(), pvr::utils::vma::AllocationCreateFlags::e_MAPPED_BIT);
}

void TextElement_::regenerateText() const
{
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
}

void TextElement_::updateVbo() const
{
	if (_vertices.size()) { pvr::utils::updateHostVisibleBuffer(_vbo, _vertices.data(), 0, static_cast<uint32_t>(sizeof(Vertex) * _vertices.size()), true); }
	VkDrawIndexedIndirectCommand cmd;
	cmd.firstInstance = 0;
	cmd.firstIndex = 0;
	cmd.instanceCount = 1;
	cmd.indexCount = (glm::min<int32_t>(_numCachedVerts, 0xFFFC) >> 1) * 3;
	cmd.vertexOffset = 0;

	pvr::utils::updateHostVisibleBuffer(_drawIndirectBuffer, &cmd, 0, static_cast<VkDeviceSize>(sizeof(VkDrawIndexedIndirectCommand)), true);
}

void TextElement_::onRender(CommandBufferBase& commands)
{
	if (_vbo)
	{
		commands->bindVertexBuffer(_vbo, 0, 0);
		commands->bindIndexBuffer(_uiRenderer->getFontIbo(), 0, pvrvk::IndexType::e_UINT16);
		commands->drawIndexedIndirect(_drawIndirectBuffer, 0, 1, 0);
	}
}

void Text_::calculateMvp(uint64_t parentIds, glm::mat4 const& srt, const glm::mat4& viewProj, Rect2D const& viewport) const
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
	tmpPos.x = _position.x * viewport.getExtent().getWidth() * .5f + viewport.getExtent().getWidth() * .5f;
	tmpPos.y = _position.y * viewport.getExtent().getHeight() * .5f + viewport.getExtent().getHeight() * .5f;

	tmpPos.x += viewport.getOffset().getX() + _pixelOffset.x;
	tmpPos.y += viewport.getOffset().getY() + _pixelOffset.y;

	_mvpData[parentIds].mvp = viewProj * srt * glm::translate(glm::vec3(tmpPos, 0.0f)) * _cachedMatrix;
	updateUbo(parentIds);
}

void Text_::updateUbo(uint64_t parentIds) const
{
	// update the descriptor set once
	debug_assertion(_mvpData[parentIds].bufferArrayId != -1, "Invalid MVP Buffer ID");
	debug_assertion(_materialData.bufferArrayId != -1, "Invalid Material Buffer ID");
	_uiRenderer->getUbo().updateMvp(_mvpData[parentIds].bufferArrayId, _mvpData[parentIds].mvp);
	_uiRenderer->getMaterial().updateMaterial(_materialData.bufferArrayId, _color, 1, glm::mat4(1.f));
}

void Text_::onRender(CommandBufferBase& commandBuffer, uint64_t parentId)
{
	updateUbo(parentId);
	pvr::utils::beginCommandBufferDebugLabel(commandBuffer, pvrvk::DebugUtilsLabel("Rendering: (" + getSpriteName() + ")"));
	commandBuffer->bindDescriptorSet(pvrvk::PipelineBindPoint::e_GRAPHICS, _uiRenderer->getPipelineLayout(), 0, getTexDescriptorSet(), nullptr, 0);

	_uiRenderer->getUbo().bindUboDynamic(commandBuffer, _uiRenderer->getPipelineLayout(), _mvpData[parentId].bufferArrayId);
	_uiRenderer->getMaterial().bindUboDynamic(commandBuffer, _uiRenderer->getPipelineLayout(), _materialData.bufferArrayId);

	_textElement->onRender(commandBuffer);
	pvr::utils::endCommandBufferDebugLabel(commandBuffer);
}

void Text_::onRemoveInstance(uint64_t parentId)
{
	if (_mvpData[parentId].bufferArrayId != -1)
	{
		_uiRenderer->getUbo().releaseBufferSlice(_mvpData[parentId].bufferArrayId);
		_uiRenderer->getMaterial().releaseBufferArray(_materialData.bufferArrayId);
		_mvpData[parentId].bufferArrayId = -1;
		_materialData.bufferArrayId = -1;
	}
}

Text_::Text_(make_shared_enabler, UIRenderer& uiRenderer, const TextElement& textElement)
	: Sprite_(uiRenderer), _textElement(textElement), _imageViewObjectName(""), _vboObjectName("")
{
	_alphaMode = textElement->getFont()->isAlphaRendering();
	if (_materialData.bufferArrayId == -1)
	{
		_materialData.bufferArrayId = _uiRenderer->getMaterial().getNewBufferArray();
		if (_materialData.bufferArrayId == -1) { throw UIRendererInstanceMaxError("Failed to create Text (Material instances)"); }
	}

	onAddInstance(0);
}

/// <summary>You must always submit your outstanding operations to a texture before calling setText. Because
/// setText will edit the content of VBOs and similar, these must be submitted before changing the text. To avoid
/// that, prefer using more Text objects.</summary>
TextElement_& TextElement_::setText(const std::string& str)
{
	_isTextDirty = true;
	_isUtf8 = true;
	_textWStr.clear();
	if (str.length() > _maxLength) { _textStr.assign(str.begin(), str.begin() + _maxLength); }
	else
	{
		_textStr = str;
	}
	// check if need reallocation
	return *this;
}

TextElement_& TextElement_::setText(const std::wstring& str)
{
	_isTextDirty = true;
	_isUtf8 = false;
	_textStr.clear();
	if (str.length() > _maxLength) { _textWStr.assign(str.begin(), str.begin() + _maxLength); }
	else
	{
		_textWStr = str;
	}
	return *this;
	// check if need reallocation
}

TextElement_& TextElement_::setText(std::string&& str)
{
	if (str.length() > _maxLength) { str.erase(str.begin() + _maxLength, str.end()); }
	_isTextDirty = true;
	_isUtf8 = true;
	_textWStr.clear();
	if (str.length() > _maxLength) { _textStr.erase(str.begin() + _maxLength, str.end()); }
	_textStr = std::move(str);
	return *this;
}

TextElement_& TextElement_::setText(std::wstring&& str)
{
	if (str.length() > _maxLength) { str.erase(str.begin() + _maxLength, str.end()); }
	_isTextDirty = true;
	_isUtf8 = false;
	_textStr.clear();
	if (str.length() > _maxLength) { str.erase(str.begin() + _maxLength, str.end()); }
	_textWStr = std::move(str);
	return *this;
}

MatrixGroup_::MatrixGroup_(make_shared_enabler, UIRenderer& uiRenderer, uint64_t id) : Group_(uiRenderer, id) {}

void MatrixGroup_::commitUpdates() const { calculateMvp(0, glm::mat4(1.f), _uiRenderer->getScreenRotation() * _viewProj, _uiRenderer->getViewport()); }

void PixelGroup_::calculateMvp(uint64_t parentIds, const glm::mat4& srt, const glm::mat4& viewProj, Rect2D const& viewport) const
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
	tmpPos.x = static_cast<float>(math::ndcToPixel(_position.x, viewport.getExtent().getWidth()));
	tmpPos.y = static_cast<float>(math::ndcToPixel(_position.y, viewport.getExtent().getHeight()));
	// add the final pixel offset
	tmpPos.x += static_cast<float>(_pixelOffset.x) + static_cast<float>(viewport.getOffset().getX());
	tmpPos.y += static_cast<float>(_pixelOffset.y) + static_cast<float>(viewport.getOffset().getY());

	_cachedMatrix[3][0] = tmpPos.x;
	_cachedMatrix[3][1] = tmpPos.y;

	_cachedMatrix = glm::rotate(_cachedMatrix, _rotation, glm::vec3(0.f, 0.f, 1.f));
	_cachedMatrix = glm::scale(_cachedMatrix, glm::vec3(_scale, 1.f));
	_cachedMatrix = glm::translate(_cachedMatrix, glm::vec3(-offset, 0.0f));

	glm::mat4 tmpMatrix = srt * _cachedMatrix;
	// My cached matrix should always be up-to-date unless overridden. No effect.
	for (ChildContainer::iterator it = _children.begin(); it != _children.end(); ++it)
	{
		(*it)->calculateMvp(packId(parentIds, _id), tmpMatrix, viewProj,
			Rect2D(pvrvk::Offset2D(0, 0), pvrvk::Extent2D(static_cast<int32_t>(_boundingRect.getSize().x), static_cast<int32_t>(_boundingRect.getSize().y))));
	}
}

Group_* Group_::add(const Sprite& sprite)
{
	_children.emplace_back(sprite);
	_boundingRect.add(sprite->getDimensions().x, sprite->getDimensions().y, 0.0f);
	try
	{
		_children.back()->onAddInstance(_id);
	}
	catch (...)
	{
		_children.pop_back();
		throw;
	}
	return this;
}

} // namespace impl
} // namespace ui
} // namespace pvr
//!\endcond
