/*!
\brief The definition of the class used to represent Texture metadata.
\file PVRCore/texture/MetaData.h
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
#pragma once
#include <cstdint>
#include <cstring>

namespace pvr {
/// <summary>The TextureMetaData class contains metadata of a texture. Metadata is any information that a texture
/// could be correctly loaded from file without. In most cases, metadata may still be necessary to actually USE the
/// texture, such as winding orders, paddings, atlas information and others.</summary>
class TextureMetaData
{
public:
	/// <summary>Values for each meta data type that we know about. Texture arrays hinge on each surface being identical in all but content,
	///  including meta data. If the meta data varies even slightly then a new texture should be used. It is possible to write your own
	///  extension to get around this however.</summary>
	enum Identifier
	{
		IdentifierTextureAtlasCoords = 0,
		IdentifierBumpData,
		IdentifierCubeMapOrder,
		IdentifierTextureOrientation,
		IdentifierBorderData,
		IdentifierPadding,
		IdentifierNumMetaDataTypes
	};

	/// <summary>Axes, used to query orientations.</summary>
	enum Axis
	{
		AxisAxisX = 0,
		AxisAxisY = 1,
		AxisAxisZ = 2
	};

	/// <summary>Orientations of various axes.</summary>
	enum AxisOrientation
	{
		AxisOrientationLeft = 1 << AxisAxisX,
		AxisOrientationRight = 0,
		AxisOrientationUp = 1 << AxisAxisY,
		AxisOrientationDown = 0,
		AxisOrientationOut = 1 << AxisAxisZ,
		AxisOrientationIn = 0
	};

public:
	/// <summary>Constructor</summary>
	TextureMetaData() : _fourCC(0), _key(0), _dataSize(0), _data(NULL) {}

	/// <summary>Constructor</summary>
	/// <param name="fourCC">FourCC of the metadata</param>
	/// <param name="key">Key of the metadata</param>
	/// <param name="dataSize">The total size of the payload of the metadata</param>
	/// <param name="data">A pointer to the actual data</param>
	TextureMetaData(uint32_t fourCC, uint32_t key, uint32_t dataSize, const char* data) : _fourCC(0), _key(0), _dataSize(0), _data(NULL)
	{
		// Copy the data across.
		if (dataSize)
		{
			_data = new unsigned char[dataSize];
			memset(_data, 0, sizeof(unsigned char) * dataSize);
		}
		if (_data)
		{
			_fourCC = fourCC;
			_key = key;
			_dataSize = dataSize;
			if (data) { memcpy(_data, data, _dataSize); }
		}
	}

	/// <summary>Copy Constructor</summary>
	/// <param name="rhs">Copy from this object</param>
	TextureMetaData(const TextureMetaData& rhs) : _fourCC(0), _key(0), _dataSize(0), _data(NULL)
	{
		// Copy the data across.
		if (rhs._dataSize)
		{
			_data = new unsigned char[rhs._dataSize];
			memset(_data, 0, sizeof(unsigned char) * rhs._dataSize);
		}
		if (_data)
		{
			_fourCC = rhs._fourCC;
			_key = rhs._key;
			_dataSize = rhs._dataSize;
			if (rhs._data) { memcpy(_data, rhs._data, _dataSize); }
		}
	}
	/// <summary>Copy Constructor</summary>
	/// <param name="rhs">Copy from this object</param>
	TextureMetaData(const TextureMetaData&& rhs) : _fourCC(0), _key(0), _dataSize(0), _data(NULL)
	{
		// Copy the data across.
		if (rhs._dataSize)
		{
			_data = new unsigned char[rhs._dataSize];
			memset(_data, 0, sizeof(unsigned char) * rhs._dataSize);
		}
		if (_data)
		{
			_fourCC = rhs._fourCC;
			_key = rhs._key;
			_dataSize = rhs._dataSize;
			if (rhs._data) { memcpy(_data, rhs._data, _dataSize); }
		}
	}

	/// <summary>Destructor</summary>
	~TextureMetaData()
	{
		if (_data)
		{
			delete[] _data;
			_data = NULL;
		}
	}

	/// <summary>Copy assignment operator</summary>
	/// <param name="rhs">Copy from this object</param>
	/// <returns>This object</returns>
	TextureMetaData& operator=(const TextureMetaData& rhs)
	{
		// If it equals itself, return early.
		if (&rhs == this) { return *this; }

		// Initialize
		_fourCC = _key = _dataSize = 0;

		// Delete any old data
		if (_data)
		{
			delete[] _data;
			_data = NULL;
		}

		// Copy the data across.
		_data = new unsigned char[rhs._dataSize];
		if (_data)
		{
			_fourCC = rhs._fourCC;
			_key = rhs._key;
			_dataSize = rhs._dataSize;
			if (rhs._data) { memcpy(_data, rhs._data, _dataSize); }
		}

		return *this;
	}

	/// <summary>Get the 4cc descriptor of the data type's creator. Values equating to values between 'P' 'V' 'R' 0
	/// and 'P' 'V' 'R' 255 will be used by our headers.</summary>
	/// <returns>Return 4cc descriptor of the data type's creator.</returns>
	uint32_t getFourCC() const { return _fourCC; }

	/// <summary>Get the data size of this meta data</summary>
	/// <returns>Return the size of the meta data</returns>
	uint32_t getDataSize() const { return _dataSize; }

	/// <summary>Get the enumeration key identifying the data type.</summary>
	/// <returns>Return the enumeration key.</returns>
	uint32_t getKey() const { return _key; }

	/// <summary>Get the data, can be absolutely anything, the loader needs to know how to handle it based on fourCC
	/// and key.</summary>
	/// <returns>Return the data</returns>
	const unsigned char* getData() const { return _data; }
	/// <summary>Get the data, can be absolutely anything, the loader needs to know how to handle it based on fourCC
	/// and key.</summary>
	/// <returns>Return the data</returns>
	unsigned char* getData() { return _data; }

	/// <summary>Get the data total size in memory</summary>
	/// <returns>Return the data total size in memory</returns>
	uint32_t getTotalSizeInMemory() const { return sizeof(_fourCC) + sizeof(_key) + sizeof(_dataSize) + _dataSize; }

private:
	uint32_t _fourCC; // A 4cc descriptor of the data type's creator.
	// Values equating to values between 'P' 'V' 'R' 0 and 'P' 'V' 'R' 255 will be used by our headers.
	uint32_t _key; // Enumeration key identifying the data type.
	uint32_t _dataSize; // Size of attached data.
	unsigned char* _data; // Data array, can be absolutely anything, the loader needs to know how to handle it based on fourCC and key.
};
} // namespace pvr
