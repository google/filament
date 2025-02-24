/*!
\brief Contains a function to write TGA data to a file
\file PVRCore/textureio/TGAWriter.h
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
#pragma once
#include "PVRCore/stream/FileStream.h"
#include "PVRCore/Log.h"

namespace pvr {

/// <summary>Write out TGA data from an image.</summary>
/// <param name="filename">Stream to write the TGA into</param>
/// <param name="w">The width of the image</param>
/// <param name="h">The height of the image</param>
/// <param name="imageData">Pointer to the raw image data</param>
/// <param name="stride">Size in bytes of each pixel. (Equal to the number of channels)</param>
/// <param name="pixelReplicate">Upscale factor.</param>
/// <returns>True if successfuly completed, otherwise False. (log error on false)</returns>
inline void writeTGA(pvr::Stream& file, uint32_t w, uint32_t h, const unsigned char* imageData, const uint32_t stride, uint32_t pixelReplicate = 1)
{
	if (!file.isWritable()) { throw InvalidOperationError("[writeTGA]: Attempted to write to non-writable stream"); }
	if (pixelReplicate == 0 || w == 0 || h == 0) { throw InvalidArgumentError("writeTGA: Invalid size."); }

	if (!imageData) { throw InvalidArgumentError("writeTGA: Pointer to data was null"); }

	// header
	unsigned char lengthID(0);
	unsigned char colorMapType(0);
	unsigned char imageType(2);

	// header - color map specification (5 bytes)
	unsigned short firstEntryIndex(0);
	unsigned short colorMapLength(0);
	unsigned char colorMapEntrySize(0);

	// header - image specification (10 bytes)
	unsigned short originX(0);
	unsigned short originY(0);
	unsigned short imageWidth(static_cast<unsigned short>(w * pixelReplicate));
	unsigned short imageHeight(static_cast<unsigned short>(h * pixelReplicate));
	unsigned char bitsperpixel(static_cast<unsigned char>(stride * 8));
	unsigned char imageDescriptor(0);

	// Write header
	size_t dataWritten;
	file.write(sizeof(lengthID), 1, &lengthID, dataWritten);
	file.write(sizeof(colorMapType), 1, &colorMapType, dataWritten);
	file.write(sizeof(imageType), 1, &imageType, dataWritten);
	file.write(sizeof(firstEntryIndex), 1, &firstEntryIndex, dataWritten);
	file.write(sizeof(colorMapLength), 1, &colorMapLength, dataWritten);
	file.write(sizeof(colorMapEntrySize), 1, &colorMapEntrySize, dataWritten);
	file.write(sizeof(originX), 1, &originX, dataWritten);
	file.write(sizeof(originY), 1, &originY, dataWritten);
	file.write(sizeof(imageWidth), 1, &imageWidth, dataWritten);
	file.write(sizeof(imageHeight), 1, &imageHeight, dataWritten);
	file.write(sizeof(bitsperpixel), 1, &bitsperpixel, dataWritten);
	file.write(sizeof(imageDescriptor), 1, &imageDescriptor, dataWritten);

	// write out the data
	if (pixelReplicate == 1) { file.write((size_t)w * (size_t)h * (size_t)stride, 1, imageData, dataWritten); }
	else
	{
		for (uint32_t y = 0; y < h; ++y)
		{
			const unsigned char* row = &imageData[stride * w * y];

			for (uint32_t repY = 0; repY < pixelReplicate; ++repY)
			{
				for (uint32_t x = 0; x < w; ++x)
				{
					const unsigned char* pixel = &row[stride * x];

					for (uint32_t repX = 0; repX < pixelReplicate; ++repX) { file.write(stride, 1, pixel, dataWritten); }
				}
			}
		}
	}
}

/// <summary>Write out TGA data from an image.</summary>
/// <param name="filename">Stream to write the TGA into</param>
/// <param name="w">The width of the image</param>
/// <param name="h">The height of the image</param>
/// <param name="imageData">Pointer to the raw image data</param>
/// <param name="stride">Size in bytes of each pixel. (Equal to the number of channels)</param>
/// <param name="pixelReplicate">Upscale factor.</param>
/// <returns>True if successfuly completed, otherwise False. (log error on false)</returns>
inline void writeTGA(pvr::Stream&& file, uint32_t w, uint32_t h, const unsigned char* imageData, const unsigned char stride, uint32_t pixelReplicate = 1)
{
	writeTGA(file, w, h, imageData, stride, pixelReplicate);
}

/// <summary>Write out TGA data from an image.</summary>
/// <param name="filename">Filename for which a filestream will be created to write the TGA into</param>
/// <param name="w">The width of the image</param>
/// <param name="h">The height of the image</param>
/// <param name="imageData">Pointer to the raw image data</param>
/// <param name="stride">Size in bytes of each pixel. (Equal to the number of channels)</param>
/// <param name="pixelReplicate">Upscale factor.</param>
/// <returns>True if successfuly completed, otherwise False. (log error on false)</returns>
inline void writeTGA(const char* filename, uint32_t w, uint32_t h, const unsigned char* imageData, const unsigned char stride, uint32_t pixelReplicate = 1)
{
	FileStream fs(filename, "wb");
	writeTGA(fs, w, h, imageData, stride, pixelReplicate);
}

/// <summary>Write out TGA data from an image.</summary>
/// <param name="filename">Stream to write the TGA into</param>
/// <param name="w">The width of the image</param>
/// <param name="h">The height of the image</param>
/// <param name="imageDataR">Pointer to the RED channel image data</param>
/// <param name="imageDataG">Pointer to the GREEN channel image data</param>
/// <param name="imageDataB">Pointer to the BLUE channel image data</param>
/// <param name="stride">Size in bytes of each pixel</param>
/// <param name="pixelReplicate">Upscale factor.</param>
/// <returns>True if successfuly completed, otherwise False. (log error on false)</returns>
inline void writeTGAFromPlanar(pvr::Stream& stream, uint32_t w, uint32_t h, const unsigned char* imageDataR, const unsigned char* imageDataG, const unsigned char* imageDataB,
	const uint32_t stride, uint32_t pixelReplicate = 1)
{
	std::vector<unsigned char> data((size_t)w * (size_t)h * 3);
	for (size_t row = 0; row < h; ++row)
	{
		for (size_t col = 0; col < w; ++col)
		{
			size_t offset = row * w + col;
			data[offset * 3] = imageDataB[offset];
			data[offset * 3 + 1] = imageDataG[offset];
			data[offset * 3 + 2] = imageDataR[offset];
		}
	}
	writeTGA(stream, w, h, data.data(), stride, pixelReplicate);
}
inline void writeTGAFromPlanar(pvr::Stream&& stream, uint32_t w, uint32_t h, const unsigned char* imageDataR, const unsigned char* imageDataG, const unsigned char* imageDataB,
	const uint32_t stride, uint32_t pixelReplicate = 1)
{
	writeTGAFromPlanar(stream, w, h, imageDataR, imageDataG, imageDataB, stride, pixelReplicate);
}

/// <summary>Write out TGA data from an image.</summary>
/// <param name="filename">C-style string with the filename to write the TGA.</param>
/// <param name="w">The width of the image</param>
/// <param name="h">The height of the image</param>
/// <param name="imageDataR">Pointer to the RED channel image data</param>
/// <param name="imageDataG">Pointer to the GREEN channel image data</param>
/// <param name="imageDataB">Pointer to the BLUE channel image data</param>
/// <param name="stride">Size in bytes of each pixel</param>
/// <param name="pixelReplicate">Upscale factor.</param>
/// <returns>True if successfuly completed, otherwise False. (log error on false)</returns>
inline void writeTGAFromPlanar(const char* filename, uint32_t w, uint32_t h, const unsigned char* imageDataR, const unsigned char* imageDataG, const unsigned char* imageDataB,
	const uint32_t stride, uint32_t pixelReplicate = 1)
{
	writeTGAFromPlanar(FileStream(filename, "wb"), w, h, imageDataR, imageDataG, imageDataB, stride, pixelReplicate);
}

} // namespace pvr
