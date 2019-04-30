/*
xatlas
https://github.com/jpcy/xatlas
Copyright (c) 2018 Jonathan Young

thekla_atlas
https://github.com/Thekla/thekla_atlas
Copyright (c) 2013 Thekla, Inc
Copyright NVIDIA Corporation 2006 -- Ignacio Castano <icastano@nvidia.com>

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/
#pragma once
#ifndef XATLAS_H
#define XATLAS_H
#include <stdint.h>

namespace xatlas {

struct Chart
{
	uint32_t atlasIndex;
	uint32_t *indexArray;
	uint32_t indexCount;
};

struct Vertex
{
	int32_t atlasIndex; // -1 if the vertex doesn't exist in any atlas.
	float uv[2]; // Not normalized, values are in Atlas width and height range.
	uint32_t xref; // Index of input vertex from which this output vertex originated.
};

struct Mesh
{
	Chart *chartArray;
	uint32_t chartCount;
	uint32_t *indexArray;
	uint32_t indexCount;
	Vertex *vertexArray;
	uint32_t vertexCount;
};

struct Atlas
{
	uint32_t width;
	uint32_t height;
	uint32_t atlasCount;
	uint32_t chartCount;
	uint32_t meshCount;
	Mesh *meshes;

	// Normalized atlas texel utilization. atlasCount in length.
	float *utilization;

	// Equal to PackOptions texelsPerUnit if texelsPerUnit > 0, otherwise an estimated value to try and match PackOptions resolution.
	float texelsPerUnit;
};

Atlas *Create();
void Destroy(Atlas *atlas);

struct AddMeshError
{
	enum Enum
	{
		Success,
		IndexOutOfRange,
		InvalidIndexCount // Not evenly divisible by 3 - expecting triangles.
	};
};

struct IndexFormat
{
	enum Enum
	{
		UInt16,
		UInt32
	};
};

struct MeshDecl
{
	uint32_t vertexCount;
	const void *vertexPositionData;
	uint32_t vertexPositionStride;
	const void *vertexNormalData; // optional
	uint32_t vertexNormalStride; // optional
	const void *vertexUvData; // optional. The input UVs are provided as a hint to the chart generator.
	uint32_t vertexUvStride; // optional
	uint32_t indexCount;
	const void *indexData;
	int32_t indexOffset; // optional. Add this offset to all indices.
	IndexFormat::Enum indexFormat;
	
	// optional. indexCount / 3 in length.
	// Don't atlas faces set to true. Faces will still exist in the output meshes, Vertex uv will be (0, 0) and Vertex atlasIndex will be -1.
	const bool *faceIgnoreData;

	MeshDecl()
	{
		vertexCount = 0;
		vertexPositionData = NULL;
		vertexPositionStride = 0;
		vertexNormalData = NULL;
		vertexNormalStride = 0;
		vertexUvData = NULL;
		vertexUvStride = 0;
		indexCount = 0;
		indexData = NULL;
		indexOffset = 0;
		indexFormat = IndexFormat::UInt16;
		faceIgnoreData = NULL;
	}
};

AddMeshError::Enum AddMesh(Atlas *atlas, const MeshDecl &meshDecl);

struct ProgressCategory
{
	enum Enum
	{
		ComputeCharts,
		ParameterizeCharts,
		PackCharts,
		BuildOutputMeshes
	};
};

typedef void (*ProgressFunc)(ProgressCategory::Enum category, int progress, void *userData);

struct ChartOptions
{
	float proxyFitMetricWeight;
	float roundnessMetricWeight;
	float straightnessMetricWeight;
	float normalSeamMetricWeight;
	float textureSeamMetricWeight;
	float maxChartArea;
	float maxBoundaryLength;
	float maxThreshold;
	uint32_t growFaceCount;
	uint32_t maxIterations;

	ChartOptions()
	{
		proxyFitMetricWeight = 2.0f;
		roundnessMetricWeight = 0.01f;
		straightnessMetricWeight = 6.0f;
		normalSeamMetricWeight = 4.0f;
		textureSeamMetricWeight = 0.5f;
		maxChartArea = 0.0f;
		maxBoundaryLength = 0.0f;
		maxThreshold = 2.0f;
		growFaceCount = 32;
		maxIterations = 1;
	}
};

typedef void (*ParameterizeFunc)(const float *positions, float *texcoords, uint32_t vertexCount, const uint32_t *indices, uint32_t indexCount, bool isPlanar);

struct PackOptions
{
	// The number of attempts to find a suitable random chart location.
	// 0 is brute force - very slow, but best results. Faster if blockAlign is true;
	int attempts;

	// Unit to texel scale. e.g. a 1x1 quad with texelsPerUnit of 32 will take up approximately 32x32 texels in the atlas.
	// If 0, an estimated value will be calculated to try and match the given resolution.
	// If resolution is also 0, the estimated value will try to match a 1024x1024 atlas.
	float texelsPerUnit;

	// If 0, generate a single atlas with texelsPerUnit determining the final resolution.
	// If not 0, generate 1 or more atlases with that exact resolution.
	uint32_t resolution;

	// Charts larger than this will be scaled down.
	uint32_t maxChartSize;

	// Align charts to 4x4 blocks. 
	bool blockAlign;

	// Pack charts with extra padding.
	bool conservative;

	// Number of pixels to pad. conservative must be true.
	int padding;

	PackOptions()
	{
		attempts = 4096;
		texelsPerUnit = 0.0f;
		resolution = 0;
		maxChartSize = 1024;
		blockAlign = false;
		conservative = false;
		padding = 0;
	}
};

// Equivalent to calling ComputeCharts, ParameterizeCharts and PackCharts in sequence.
void Generate(Atlas *atlas, ChartOptions chartOptions = ChartOptions(), ParameterizeFunc paramFunc = NULL, PackOptions packOptions = PackOptions(), ProgressFunc progressFunc = NULL, void *progressUserData = NULL);

// Call after AddMesh.
void ComputeCharts(Atlas *atlas, ChartOptions chartOptions = ChartOptions(), ProgressFunc progressFunc = NULL, void *progressUserData = NULL);

// Call after ComputeCharts. Can be called multiple times to re-parameterize charts with a different ParameterizeFunc.
void ParameterizeCharts(Atlas *atlas, ParameterizeFunc func = NULL, ProgressFunc progressFunc = NULL, void *progressUserData = NULL);

// Call after ParameterizeCharts. Can be called multiple times to re-pack charts with different options.
void PackCharts(Atlas *atlas, PackOptions packOptions = PackOptions(), ProgressFunc progressFunc = NULL, void *progressUserData = NULL);

typedef void *(*ReallocFunc)(void *, size_t);
void SetRealloc(ReallocFunc reallocFunc);

typedef int (*PrintFunc)(const char *, ...);
void SetPrint(PrintFunc print, bool verbose);

const char *StringForEnum(AddMeshError::Enum error);
const char *StringForEnum(ProgressCategory::Enum category);

} // namespace xatlas

#endif // XATLAS_H
