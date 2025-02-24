/*!
\brief Implementations of methods from the Mesh class.
\file PVRAssets/model/Mesh.cpp
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
//!\cond NO_DOXYGEN
#include <cstring>
#include "PVRAssets/model/Mesh.h"
#include <algorithm>
using std::map;
using std::pair;
using pvr::hash;
using std::vector;

namespace pvr {
namespace assets {
class SemanticLessThan
{
	inline bool operator()(const Mesh::VertexAttributeData& lhs, const Mesh::VertexAttributeData rhs) { return lhs.getSemantic() < rhs.getSemantic(); }
};

void Mesh::VertexAttributeData::setDataType(DataType type) { _layout.dataType = type; }

void Mesh::VertexAttributeData::setOffset(uint32_t offset) { _layout.offset = static_cast<uint16_t>(offset); }

void Mesh::VertexAttributeData::setN(uint8_t n) { _layout.width = n; }

void Mesh::VertexAttributeData::setDataIndex(uint16_t dataIndex) { _dataIndex = dataIndex; }

// CFaceData
Mesh::FaceData::FaceData() : _indexType(IndexType::IndexType16Bit) {}

void Mesh::FaceData::setData(const uint8_t* data, uint32_t size, const IndexType indexType)
{
	_indexType = indexType;
	_data.resize(size);
	memcpy(_data.data(), data, size);
}

int32_t Mesh::addData(const uint8_t* data, uint32_t size, uint32_t stride)
{
	_data.vertexAttributeDataBlocks.emplace_back(StridedBuffer());
	_data.vertexAttributeDataBlocks.back().stride = static_cast<uint16_t>(stride);
	UInt8Buffer& last_element = _data.vertexAttributeDataBlocks.back();
	last_element.resize(size);
	if (data) { memcpy(last_element.data(), data, size); }
	return static_cast<int32_t>(_data.vertexAttributeDataBlocks.size()) - 1;
}

int32_t Mesh::addData(const uint8_t* data, uint32_t size, uint32_t stride, uint32_t index)
{
	if (_data.vertexAttributeDataBlocks.size() <= index) { _data.vertexAttributeDataBlocks.resize(index + 1); }
	StridedBuffer& last_element = _data.vertexAttributeDataBlocks[index];
	last_element.stride = static_cast<uint16_t>(stride);
	last_element.resize(size);
	if (data) { memcpy(last_element.data(), data, size); }
	return static_cast<int32_t>(_data.vertexAttributeDataBlocks.size()) - 1;
}

void Mesh::setStride(uint32_t index, uint32_t stride)
{
	if (_data.vertexAttributeDataBlocks.size() <= index) { _data.vertexAttributeDataBlocks.resize(index + 1); }
	_data.vertexAttributeDataBlocks[index].stride = static_cast<uint16_t>(stride);
}

void Mesh::removeData(uint32_t index)
{
	// Remove element
	_data.vertexAttributeDataBlocks.erase(_data.vertexAttributeDataBlocks.begin() + index);

	VertexAttributeContainer::iterator walk = _data.vertexAttributes.begin();

	// Update the indices stored by the Vertex Attributes
	for (; walk != _data.vertexAttributes.end(); ++walk)
	{
		uint32_t idx = walk->value.getDataIndex();

		if (idx > index) { walk->value.setDataIndex(static_cast<uint16_t>(idx--)); }
		else if (idx == index)
		{
			walk->value.setDataIndex(static_cast<uint16_t>(-1));
		}
	}
}

// Should this take all the parameters for an element along with data or just pass in an already complete class? or both?
int32_t Mesh::addVertexAttribute(const VertexAttributeData& element, bool forceReplace)
{
	VertexAttributeContainer::index_iterator it = _data.vertexAttributes.indexed_find(element.getSemantic());
	if (it == _data.vertexAttributes.indexed_end()) { return static_cast<int32_t>(_data.vertexAttributes.insert(element.getSemantic(), element)); }
	else
	{
		if (forceReplace) { _data.vertexAttributes[it->first] = element; }
		else
		{
			return -1;
		}
		return static_cast<int32_t>(it->second);
	}
}

int32_t Mesh::addVertexAttribute(const StringHash& semanticName, const DataType& type, uint32_t n, uint32_t offset, uint32_t dataIndex, bool forceReplace)
{
	int32_t index = static_cast<int32_t>(_data.vertexAttributes.getIndex(semanticName));

	if (index == -1)
	{
		index = static_cast<int32_t>(_data.vertexAttributes.insert(
			semanticName, VertexAttributeData(semanticName, type, static_cast<uint8_t>(n), static_cast<uint16_t>(offset), static_cast<uint16_t>(dataIndex))));
	}
	else
	{
		if (forceReplace)
		{ _data.vertexAttributes[index] = VertexAttributeData(semanticName, type, static_cast<uint8_t>(n), static_cast<uint16_t>(offset), static_cast<uint16_t>(dataIndex)); }
		else
		{
			return -1;
		}
	}
	return index;
}

void Mesh::addFaces(const uint8_t* data, uint32_t size, IndexType indexType)
{
	_data.faces.setData(data, size, indexType);

	if (size) { _data.primitiveData.numFaces = size / (indexType == IndexType::IndexType32Bit ? 4 : 2) / 3; }
	else
	{
		_data.primitiveData.numFaces = 0;
	}
}

void Mesh::removeVertexAttribute(const StringHash& semantic) { _data.vertexAttributes.erase(semantic); }

void Mesh::removeAllVertexAttributes(void) { _data.vertexAttributes.clear(); }

namespace {
struct DataCarrier
{
	uint8_t* indexData;
	uint8_t* vertexData;
	size_t vboStride;
	size_t attribOffset;
	size_t indexDataSize;
	size_t valueToAddToVertices;
};
} // namespace

} // namespace assets
} // namespace pvr
//!\endcond
