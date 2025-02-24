/*!
\brief Internal helper classes
\file PVRAssets/Helper.h
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
#pragma once

#include "PVRAssets/model/Mesh.h"
#include "PVRAssets/Model.h"
#include "PVRCore/IAssetProvider.h"

namespace pvr {
namespace assets {
namespace helper {
/// <summary>Read vertex data into float buffer.</summary>
/// <param name="data">Data to read from</param>
/// <param name="type">Data type of the vertex to read</param>
/// <param name="count">Number of vertices to read</param>
/// <param name="out">Array of vertex read</param>
void VertexRead(const uint8_t* data, const DataType type, uint32_t count, float* out);

/// <summary>Read vertex index data into uin32 buffer.</summary>
/// <param name="data">Data to read from</param>
/// <param name="type">Index type to read</param>
/// <param name="out">of index data read</param>
void VertexIndexRead(const uint8_t* data, const IndexType type, uint32_t* const out);

/// <summary>Retrieves the model definition type using the extension of the given filename.</summary>
/// <param name="modelFile">The name of the model file to use for determining its model file format</param>
pvr::assets::ModelFileFormat getModelFormatFromFilename(const std::string& modelFile);

} // namespace helper
/// <summary>Load a model file using the provided scene file name.</summary>
/// <param name="app">An asset provider used to load the model file</param>
/// <param name="modelFile"></param>
/// <returns>Returns a successfully created pvr::assets::ModelHandle object otherwise will throw</returns>
pvr::assets::ModelHandle loadModel(const IAssetProvider& app, const std::string& modelFile);

/// <summary>Load a model file using the provided scene file name.</summary>
/// <param name="app">An asset provider used to load the model file</param>
/// <param name="modelFile"></param>
/// <returns>Returns a successfully created pvr::assets::ModelHandle object otherwise will throw</returns>
pvr::assets::ModelHandle loadModel(const IAssetProvider& app, const pvr::Stream& model);
} // namespace assets
} // namespace pvr
