/*!
\brief Contains functions to decompress PVRTC or ETC formats into RGBA8888.
\file PVRCore/texture/PVRTDecompress.h
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
#pragma once
#include <stdint.h>
namespace pvr {

/// <summary>Decompresses PVRTC to RGBA 8888.</summary>
/// <param name="compressedData">The PVRTC texture data to decompress</param>
/// <param name="do2bitMode">Signifies whether the data is PVRTC2 or PVRTC4</param>
/// <param name="xDim">X dimension of the texture</param>
/// <param name="yDim">Y dimension of the texture</param>
/// <param name="outResultImage">The decompressed texture data</param>
/// <returns>Return the amount of data that was decompressed.</returns>
uint32_t PVRTDecompressPVRTC(const void* compressedData, uint32_t do2bitMode, uint32_t xDim, uint32_t yDim, uint8_t* outResultImage);

/// <summary>Decompresses ETC to RGBA 8888.</summary>
/// <param name="srcData">The ETC texture data to decompress</param>
/// <param name="xDim">X dimension of the texture</param>
/// <param name="yDim">Y dimension of the texture</param>
/// <param name="dstData">The decompressed texture data</param>
/// <param name="mode">The format of the data</param>
/// <returns>Return The number of bytes of ETC data decompressed</returns>
uint32_t PVRTDecompressETC(const void* srcData, uint32_t xDim, uint32_t yDim, void* dstData, uint32_t mode);
} // namespace pvr
