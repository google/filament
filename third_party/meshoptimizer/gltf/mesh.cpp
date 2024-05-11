// This file is part of gltfpack; see gltfpack.h for version/license details
#include "gltfpack.h"

#include <algorithm>

#include <math.h>
#include <string.h>

#include "../src/meshoptimizer.h"

static float inverseTranspose(float* result, const float* transform)
{
	float m[4][4] = {};
	memcpy(m, transform, 16 * sizeof(float));

	float det =
	    m[0][0] * (m[1][1] * m[2][2] - m[2][1] * m[1][2]) -
	    m[0][1] * (m[1][0] * m[2][2] - m[1][2] * m[2][0]) +
	    m[0][2] * (m[1][0] * m[2][1] - m[1][1] * m[2][0]);

	float invdet = (det == 0.f) ? 0.f : 1.f / det;

	float r[4][4] = {};

	r[0][0] = (m[1][1] * m[2][2] - m[2][1] * m[1][2]) * invdet;
	r[1][0] = (m[0][2] * m[2][1] - m[0][1] * m[2][2]) * invdet;
	r[2][0] = (m[0][1] * m[1][2] - m[0][2] * m[1][1]) * invdet;
	r[0][1] = (m[1][2] * m[2][0] - m[1][0] * m[2][2]) * invdet;
	r[1][1] = (m[0][0] * m[2][2] - m[0][2] * m[2][0]) * invdet;
	r[2][1] = (m[1][0] * m[0][2] - m[0][0] * m[1][2]) * invdet;
	r[0][2] = (m[1][0] * m[2][1] - m[2][0] * m[1][1]) * invdet;
	r[1][2] = (m[2][0] * m[0][1] - m[0][0] * m[2][1]) * invdet;
	r[2][2] = (m[0][0] * m[1][1] - m[1][0] * m[0][1]) * invdet;

	r[3][3] = 1.f;

	memcpy(result, r, 16 * sizeof(float));

	return det;
}

static void transformPosition(float* res, const float* ptr, const float* transform)
{
	float x = ptr[0] * transform[0] + ptr[1] * transform[4] + ptr[2] * transform[8] + transform[12];
	float y = ptr[0] * transform[1] + ptr[1] * transform[5] + ptr[2] * transform[9] + transform[13];
	float z = ptr[0] * transform[2] + ptr[1] * transform[6] + ptr[2] * transform[10] + transform[14];

	res[0] = x;
	res[1] = y;
	res[2] = z;
}

static void transformNormal(float* res, const float* ptr, const float* transform)
{
	float x = ptr[0] * transform[0] + ptr[1] * transform[4] + ptr[2] * transform[8];
	float y = ptr[0] * transform[1] + ptr[1] * transform[5] + ptr[2] * transform[9];
	float z = ptr[0] * transform[2] + ptr[1] * transform[6] + ptr[2] * transform[10];

	float l = sqrtf(x * x + y * y + z * z);
	float s = (l == 0.f) ? 0.f : 1 / l;

	res[0] = x * s;
	res[1] = y * s;
	res[2] = z * s;
}

// assumes mesh & target are structurally identical
static void transformMesh(Mesh& target, const Mesh& mesh, const cgltf_node* node)
{
	assert(target.streams.size() == mesh.streams.size());
	assert(target.indices.size() == mesh.indices.size());

	float transform[16];
	cgltf_node_transform_world(node, transform);

	float transforminvt[16];
	float det = inverseTranspose(transforminvt, transform);

	for (size_t si = 0; si < mesh.streams.size(); ++si)
	{
		const Stream& source = mesh.streams[si];
		Stream& stream = target.streams[si];

		assert(source.type == stream.type);
		assert(source.data.size() == stream.data.size());

		if (stream.type == cgltf_attribute_type_position)
		{
			for (size_t i = 0; i < stream.data.size(); ++i)
				transformPosition(stream.data[i].f, source.data[i].f, transform);
		}
		else if (stream.type == cgltf_attribute_type_normal)
		{
			for (size_t i = 0; i < stream.data.size(); ++i)
				transformNormal(stream.data[i].f, source.data[i].f, transforminvt);
		}
		else if (stream.type == cgltf_attribute_type_tangent)
		{
			for (size_t i = 0; i < stream.data.size(); ++i)
				transformNormal(stream.data[i].f, source.data[i].f, transform);
		}
	}

	// copy indices so that we can modify them below
	target.indices = mesh.indices;

	if (det < 0 && mesh.type == cgltf_primitive_type_triangles)
	{
		// negative scale means we need to flip face winding
		for (size_t i = 0; i < target.indices.size(); i += 3)
			std::swap(target.indices[i + 0], target.indices[i + 1]);
	}
}

bool compareMeshTargets(const Mesh& lhs, const Mesh& rhs)
{
	if (lhs.targets != rhs.targets)
		return false;

	if (lhs.target_weights.size() != rhs.target_weights.size())
		return false;

	for (size_t i = 0; i < lhs.target_weights.size(); ++i)
		if (lhs.target_weights[i] != rhs.target_weights[i])
			return false;

	if (lhs.target_names.size() != rhs.target_names.size())
		return false;

	for (size_t i = 0; i < lhs.target_names.size(); ++i)
		if (strcmp(lhs.target_names[i], rhs.target_names[i]) != 0)
			return false;

	return true;
}

bool compareMeshVariants(const Mesh& lhs, const Mesh& rhs)
{
	if (lhs.variants.size() != rhs.variants.size())
		return false;

	for (size_t i = 0; i < lhs.variants.size(); ++i)
	{
		if (lhs.variants[i].variant != rhs.variants[i].variant)
			return false;

		if (lhs.variants[i].material != rhs.variants[i].material)
			return false;
	}

	return true;
}

bool compareMeshNodes(const Mesh& lhs, const Mesh& rhs)
{
	if (lhs.nodes.size() != rhs.nodes.size())
		return false;

	for (size_t i = 0; i < lhs.nodes.size(); ++i)
		if (lhs.nodes[i] != rhs.nodes[i])
			return false;

	return true;
}

static bool canMergeMeshNodes(cgltf_node* lhs, cgltf_node* rhs, const Settings& settings)
{
	if (lhs == rhs)
		return true;

	if (lhs->parent != rhs->parent)
		return false;

	bool lhs_transform = lhs->has_translation | lhs->has_rotation | lhs->has_scale | lhs->has_matrix | (!!lhs->weights);
	bool rhs_transform = rhs->has_translation | rhs->has_rotation | rhs->has_scale | rhs->has_matrix | (!!rhs->weights);

	if (lhs_transform || rhs_transform)
		return false;

	if (settings.keep_nodes)
	{
		if (lhs->name && *lhs->name)
			return false;

		if (rhs->name && *rhs->name)
			return false;
	}

	// we can merge nodes that don't have transforms of their own and have the same parent
	// this is helpful when instead of splitting mesh into primitives, DCCs split mesh into mesh nodes
	return true;
}

static bool canMergeMeshes(const Mesh& lhs, const Mesh& rhs, const Settings& settings)
{
	if (lhs.scene != rhs.scene)
		return false;

	if (lhs.nodes.size() != rhs.nodes.size())
		return false;

	for (size_t i = 0; i < lhs.nodes.size(); ++i)
		if (!canMergeMeshNodes(lhs.nodes[i], rhs.nodes[i], settings))
			return false;

	if (lhs.instances.size() || rhs.instances.size())
		return false;

	if (lhs.material != rhs.material)
		return false;

	if (lhs.skin != rhs.skin)
		return false;

	if (lhs.type != rhs.type)
		return false;

	if (!compareMeshTargets(lhs, rhs))
		return false;

	if (!compareMeshVariants(lhs, rhs))
		return false;

	if (lhs.indices.empty() != rhs.indices.empty())
		return false;

	if (lhs.streams.size() != rhs.streams.size())
		return false;

	for (size_t i = 0; i < lhs.streams.size(); ++i)
		if (lhs.streams[i].type != rhs.streams[i].type || lhs.streams[i].index != rhs.streams[i].index || lhs.streams[i].target != rhs.streams[i].target)
			return false;

	return true;
}

static void mergeMeshes(Mesh& target, const Mesh& mesh)
{
	assert(target.streams.size() == mesh.streams.size());

	size_t vertex_offset = target.streams[0].data.size();
	size_t index_offset = target.indices.size();

	for (size_t i = 0; i < target.streams.size(); ++i)
		target.streams[i].data.insert(target.streams[i].data.end(), mesh.streams[i].data.begin(), mesh.streams[i].data.end());

	target.indices.resize(target.indices.size() + mesh.indices.size());

	size_t index_count = mesh.indices.size();

	for (size_t i = 0; i < index_count; ++i)
		target.indices[index_offset + i] = unsigned(vertex_offset + mesh.indices[i]);
}

void mergeMeshInstances(Mesh& mesh)
{
	if (mesh.nodes.empty())
		return;

	// fast-path: for single instance meshes we transform in-place
	if (mesh.nodes.size() == 1)
	{
		transformMesh(mesh, mesh, mesh.nodes[0]);
		mesh.nodes.clear();
		return;
	}

	Mesh base = mesh;
	Mesh transformed = base;

	for (size_t i = 0; i < mesh.streams.size(); ++i)
	{
		mesh.streams[i].data.clear();
		mesh.streams[i].data.reserve(base.streams[i].data.size() * mesh.nodes.size());
	}

	mesh.indices.clear();
	mesh.indices.reserve(base.indices.size() * mesh.nodes.size());

	for (size_t i = 0; i < mesh.nodes.size(); ++i)
	{
		transformMesh(transformed, base, mesh.nodes[i]);
		mergeMeshes(mesh, transformed);
	}

	mesh.nodes.clear();
}

void mergeMeshes(std::vector<Mesh>& meshes, const Settings& settings)
{
	for (size_t i = 0; i < meshes.size(); ++i)
	{
		Mesh& target = meshes[i];

		if (target.streams.empty())
			continue;

		size_t target_vertices = target.streams[0].data.size();
		size_t target_indices = target.indices.size();

		size_t last_merged = i;

		for (size_t j = i + 1; j < meshes.size(); ++j)
		{
			Mesh& mesh = meshes[j];

			if (!mesh.streams.empty() && canMergeMeshes(target, mesh, settings))
			{
				target_vertices += mesh.streams[0].data.size();
				target_indices += mesh.indices.size();
				last_merged = j;
			}
		}

		for (size_t j = 0; j < target.streams.size(); ++j)
			target.streams[j].data.reserve(target_vertices);

		target.indices.reserve(target_indices);

		for (size_t j = i + 1; j <= last_merged; ++j)
		{
			Mesh& mesh = meshes[j];

			if (!mesh.streams.empty() && canMergeMeshes(target, mesh, settings))
			{
				mergeMeshes(target, mesh);

				mesh.streams.clear();
				mesh.indices.clear();
				mesh.nodes.clear();
				mesh.instances.clear();
			}
		}

		assert(target.streams[0].data.size() == target_vertices);
		assert(target.indices.size() == target_indices);
	}
}

void filterEmptyMeshes(std::vector<Mesh>& meshes)
{
	size_t write = 0;

	for (size_t i = 0; i < meshes.size(); ++i)
	{
		Mesh& mesh = meshes[i];

		if (mesh.streams.empty())
			continue;

		if (mesh.streams[0].data.empty())
			continue;

		if (mesh.type != cgltf_primitive_type_points && mesh.indices.empty())
			continue;

		// the following code is roughly equivalent to meshes[write] = std::move(mesh)
		std::vector<Stream> streams;
		streams.swap(mesh.streams);

		std::vector<unsigned int> indices;
		indices.swap(mesh.indices);

		meshes[write] = mesh;
		meshes[write].streams.swap(streams);
		meshes[write].indices.swap(indices);

		write++;
	}

	meshes.resize(write);
}

static bool hasColors(const std::vector<Attr>& data)
{
	const float threshold = 0.99f;

	for (size_t i = 0; i < data.size(); ++i)
	{
		const Attr& a = data[i];

		if (a.f[0] < threshold || a.f[1] < threshold || a.f[2] < threshold || a.f[3] < threshold)
			return true;
	}

	return false;
}

static bool hasDeltas(const std::vector<Attr>& data)
{
	const float threshold = 0.01f;

	for (size_t i = 0; i < data.size(); ++i)
	{
		const Attr& a = data[i];

		if (fabsf(a.f[0]) > threshold || fabsf(a.f[1]) > threshold || fabsf(a.f[2]) > threshold)
			return true;
	}

	return false;
}

void filterStreams(Mesh& mesh, const MaterialInfo& mi)
{
	bool morph_normal = false;
	bool morph_tangent = false;
	int keep_texture_set = -1;

	for (size_t i = 0; i < mesh.streams.size(); ++i)
	{
		Stream& stream = mesh.streams[i];

		if (stream.target)
		{
			morph_normal = morph_normal || (stream.type == cgltf_attribute_type_normal && hasDeltas(stream.data));
			morph_tangent = morph_tangent || (stream.type == cgltf_attribute_type_tangent && hasDeltas(stream.data));
		}

		if (stream.type == cgltf_attribute_type_texcoord && (mi.textureSetMask & (1u << stream.index)) != 0)
		{
			keep_texture_set = std::max(keep_texture_set, stream.index);
		}
	}

	size_t write = 0;

	for (size_t i = 0; i < mesh.streams.size(); ++i)
	{
		Stream& stream = mesh.streams[i];

		if (stream.type == cgltf_attribute_type_texcoord && stream.index > keep_texture_set)
			continue;

		if (stream.type == cgltf_attribute_type_tangent && !mi.needsTangents)
			continue;

		if ((stream.type == cgltf_attribute_type_joints || stream.type == cgltf_attribute_type_weights) && !mesh.skin)
			continue;

		if (stream.type == cgltf_attribute_type_color && !hasColors(stream.data))
			continue;

		if (stream.target && stream.type == cgltf_attribute_type_normal && !morph_normal)
			continue;

		if (stream.target && stream.type == cgltf_attribute_type_tangent && !morph_tangent)
			continue;

		// the following code is roughly equivalent to streams[write] = std::move(stream)
		std::vector<Attr> data;
		data.swap(stream.data);

		mesh.streams[write] = stream;
		mesh.streams[write].data.swap(data);

		write++;
	}

	mesh.streams.resize(write);
}

static void reindexMesh(Mesh& mesh)
{
	size_t total_vertices = mesh.streams[0].data.size();
	size_t total_indices = mesh.indices.size();

	std::vector<meshopt_Stream> streams;
	for (size_t i = 0; i < mesh.streams.size(); ++i)
	{
		if (mesh.streams[i].target)
			continue;

		assert(mesh.streams[i].data.size() == total_vertices);

		meshopt_Stream stream = {&mesh.streams[i].data[0], sizeof(Attr), sizeof(Attr)};
		streams.push_back(stream);
	}

	if (streams.empty())
		return;

	std::vector<unsigned int> remap(total_vertices);
	size_t unique_vertices = meshopt_generateVertexRemapMulti(&remap[0], &mesh.indices[0], total_indices, total_vertices, &streams[0], streams.size());
	assert(unique_vertices <= total_vertices);

	meshopt_remapIndexBuffer(&mesh.indices[0], &mesh.indices[0], total_indices, &remap[0]);

	for (size_t i = 0; i < mesh.streams.size(); ++i)
	{
		assert(mesh.streams[i].data.size() == total_vertices);

		meshopt_remapVertexBuffer(&mesh.streams[i].data[0], &mesh.streams[i].data[0], total_vertices, sizeof(Attr), &remap[0]);
		mesh.streams[i].data.resize(unique_vertices);
	}
}

static void filterTriangles(Mesh& mesh)
{
	assert(mesh.type == cgltf_primitive_type_triangles);

	unsigned int* indices = &mesh.indices[0];
	size_t total_indices = mesh.indices.size();

	size_t write = 0;

	for (size_t i = 0; i < total_indices; i += 3)
	{
		unsigned int a = indices[i + 0], b = indices[i + 1], c = indices[i + 2];

		if (a != b && a != c && b != c)
		{
			indices[write + 0] = a;
			indices[write + 1] = b;
			indices[write + 2] = c;
			write += 3;
		}
	}

	mesh.indices.resize(write);
}

static Stream* getStream(Mesh& mesh, cgltf_attribute_type type, int index = 0)
{
	for (size_t i = 0; i < mesh.streams.size(); ++i)
		if (mesh.streams[i].type == type && mesh.streams[i].index == index)
			return &mesh.streams[i];

	return 0;
}

static void simplifyMesh(Mesh& mesh, float threshold, bool aggressive)
{
	assert(mesh.type == cgltf_primitive_type_triangles);

	if (mesh.indices.empty())
		return;

	const Stream* positions = getStream(mesh, cgltf_attribute_type_position);
	if (!positions)
		return;

	size_t vertex_count = mesh.streams[0].data.size();

	size_t target_index_count = size_t(double(mesh.indices.size() / 3) * threshold) * 3;
	float target_error = 1e-2f;
	float target_error_aggressive = 1e-1f;

	if (target_index_count < 1)
		return;

	std::vector<unsigned int> indices(mesh.indices.size());
	indices.resize(meshopt_simplify(&indices[0], &mesh.indices[0], mesh.indices.size(), positions->data[0].f, vertex_count, sizeof(Attr), target_index_count, target_error));
	mesh.indices.swap(indices);

	// Note: if the simplifier got stuck, we can try to reindex without normals/tangents and retry
	// For now we simply fall back to aggressive simplifier instead

	// if the precise simplifier got "stuck", we'll try to simplify using the sloppy simplifier; this is only used when aggressive simplification is enabled as it breaks attribute discontinuities
	if (aggressive && mesh.indices.size() > target_index_count)
	{
		indices.resize(meshopt_simplifySloppy(&indices[0], &mesh.indices[0], mesh.indices.size(), positions->data[0].f, vertex_count, sizeof(Attr), target_index_count, target_error_aggressive));
		mesh.indices.swap(indices);
	}
}

static void optimizeMesh(Mesh& mesh, bool compressmore)
{
	assert(mesh.type == cgltf_primitive_type_triangles);

	if (mesh.indices.empty())
		return;

	size_t vertex_count = mesh.streams[0].data.size();

	if (compressmore)
		meshopt_optimizeVertexCacheStrip(&mesh.indices[0], &mesh.indices[0], mesh.indices.size(), vertex_count);
	else
		meshopt_optimizeVertexCache(&mesh.indices[0], &mesh.indices[0], mesh.indices.size(), vertex_count);

	std::vector<unsigned int> remap(vertex_count);
	size_t unique_vertices = meshopt_optimizeVertexFetchRemap(&remap[0], &mesh.indices[0], mesh.indices.size(), vertex_count);
	assert(unique_vertices <= vertex_count);

	meshopt_remapIndexBuffer(&mesh.indices[0], &mesh.indices[0], mesh.indices.size(), &remap[0]);

	for (size_t i = 0; i < mesh.streams.size(); ++i)
	{
		assert(mesh.streams[i].data.size() == vertex_count);

		meshopt_remapVertexBuffer(&mesh.streams[i].data[0], &mesh.streams[i].data[0], vertex_count, sizeof(Attr), &remap[0]);
		mesh.streams[i].data.resize(unique_vertices);
	}
}

struct BoneInfluence
{
	float i;
	float w;
};

struct BoneInfluenceWeightPredicate
{
	bool operator()(const BoneInfluence& lhs, const BoneInfluence& rhs) const
	{
		return lhs.w > rhs.w;
	}
};

static void filterBones(Mesh& mesh)
{
	const int kMaxGroups = 8;

	std::pair<Stream*, Stream*> groups[kMaxGroups];
	int group_count = 0;

	// gather all joint/weight groups; each group contains 4 bone influences
	for (int i = 0; i < kMaxGroups; ++i)
	{
		Stream* jg = getStream(mesh, cgltf_attribute_type_joints, int(i));
		Stream* wg = getStream(mesh, cgltf_attribute_type_weights, int(i));

		if (!jg || !wg)
			break;

		groups[group_count++] = std::make_pair(jg, wg);
	}

	if (group_count == 0)
		return;

	// weights below cutoff can't be represented in quantized 8-bit storage
	const float weight_cutoff = 0.5f / 255.f;

	size_t vertex_count = mesh.streams[0].data.size();

	BoneInfluence inf[kMaxGroups * 4] = {};

	for (size_t i = 0; i < vertex_count; ++i)
	{
		int count = 0;

		// gather all bone influences for this vertex
		for (int j = 0; j < group_count; ++j)
		{
			const Attr& ja = groups[j].first->data[i];
			const Attr& wa = groups[j].second->data[i];

			for (int k = 0; k < 4; ++k)
				if (wa.f[k] > weight_cutoff)
				{
					inf[count].i = ja.f[k];
					inf[count].w = wa.f[k];
					count++;
				}
		}

		// pick top 4 influences; this also sorts resulting influences by weight which helps renderers that use influence subset in shader LODs
		std::sort(inf, inf + count, BoneInfluenceWeightPredicate());

		// copy the top 4 influences back into stream 0 - we will remove other streams at the end
		Attr& ja = groups[0].first->data[i];
		Attr& wa = groups[0].second->data[i];

		for (int k = 0; k < 4; ++k)
		{
			if (k < count)
			{
				ja.f[k] = inf[k].i;
				wa.f[k] = inf[k].w;
			}
			else
			{
				ja.f[k] = 0.f;
				wa.f[k] = 0.f;
			}
		}
	}

	// remove redundant weight/joint streams
	for (size_t i = 0; i < mesh.streams.size();)
	{
		Stream& s = mesh.streams[i];

		if ((s.type == cgltf_attribute_type_joints || s.type == cgltf_attribute_type_weights) && s.index > 0)
			mesh.streams.erase(mesh.streams.begin() + i);
		else
			++i;
	}
}

static void simplifyPointMesh(Mesh& mesh, float threshold)
{
	assert(mesh.type == cgltf_primitive_type_points);

	if (threshold >= 1)
		return;

	const Stream* positions = getStream(mesh, cgltf_attribute_type_position);
	if (!positions)
		return;

	size_t vertex_count = mesh.streams[0].data.size();

	size_t target_vertex_count = size_t(double(vertex_count) * threshold);

	if (target_vertex_count < 1)
		return;

	std::vector<unsigned int> indices(target_vertex_count);
	indices.resize(meshopt_simplifyPoints(&indices[0], positions->data[0].f, vertex_count, sizeof(Attr), target_vertex_count));

	std::vector<Attr> scratch(indices.size());

	for (size_t i = 0; i < mesh.streams.size(); ++i)
	{
		std::vector<Attr>& data = mesh.streams[i].data;

		assert(data.size() == vertex_count);

		for (size_t j = 0; j < indices.size(); ++j)
			scratch[j] = data[indices[j]];

		data = scratch;
	}
}

static void sortPointMesh(Mesh& mesh)
{
	assert(mesh.type == cgltf_primitive_type_points);

	const Stream* positions = getStream(mesh, cgltf_attribute_type_position);
	if (!positions)
		return;

	size_t vertex_count = mesh.streams[0].data.size();

	std::vector<unsigned int> remap(vertex_count);
	meshopt_spatialSortRemap(&remap[0], positions->data[0].f, vertex_count, sizeof(Attr));

	for (size_t i = 0; i < mesh.streams.size(); ++i)
	{
		assert(mesh.streams[i].data.size() == vertex_count);

		meshopt_remapVertexBuffer(&mesh.streams[i].data[0], &mesh.streams[i].data[0], vertex_count, sizeof(Attr), &remap[0]);
	}
}

void processMesh(Mesh& mesh, const Settings& settings)
{
	switch (mesh.type)
	{
	case cgltf_primitive_type_points:
		assert(mesh.indices.empty());
		simplifyPointMesh(mesh, settings.simplify_threshold);
		sortPointMesh(mesh);
		break;

	case cgltf_primitive_type_lines:
		break;

	case cgltf_primitive_type_triangles:
		filterBones(mesh);
		reindexMesh(mesh);
		filterTriangles(mesh);
		if (settings.simplify_threshold < 1)
			simplifyMesh(mesh, settings.simplify_threshold, settings.simplify_aggressive);
		optimizeMesh(mesh, settings.compressmore);
		break;

	default:
		assert(!"Unknown primitive type");
	}
}

#ifndef NDEBUG
extern MESHOPTIMIZER_API unsigned char* meshopt_simplifyDebugKind;
extern MESHOPTIMIZER_API unsigned int* meshopt_simplifyDebugLoop;
extern MESHOPTIMIZER_API unsigned int* meshopt_simplifyDebugLoopBack;

void debugSimplify(const Mesh& source, Mesh& kinds, Mesh& loops, float ratio)
{
	Mesh mesh = source;
	assert(mesh.type == cgltf_primitive_type_triangles);

	// note: it's important to follow the same pipeline as processMesh
	// otherwise the result won't match
	filterBones(mesh);
	reindexMesh(mesh);
	filterTriangles(mesh);

	// before simplification we need to setup target kind/loop arrays
	size_t vertex_count = mesh.streams[0].data.size();

	std::vector<unsigned char> kind(vertex_count);
	std::vector<unsigned int> loop(vertex_count);
	std::vector<unsigned int> loopback(vertex_count);
	std::vector<unsigned char> live(vertex_count);

	meshopt_simplifyDebugKind = &kind[0];
	meshopt_simplifyDebugLoop = &loop[0];
	meshopt_simplifyDebugLoopBack = &loopback[0];

	simplifyMesh(mesh, ratio, /* aggressive= */ false);

	meshopt_simplifyDebugKind = 0;
	meshopt_simplifyDebugLoop = 0;
	meshopt_simplifyDebugLoopBack = 0;

	// fill out live info
	for (size_t i = 0; i < mesh.indices.size(); ++i)
		live[mesh.indices[i]] = true;

	// color palette for display
	static const Attr kPalette[] = {
	    {0.5f, 0.5f, 0.5f, 1.f}, // manifold
	    {0.f, 0.f, 1.f, 1.f},    // border
	    {0.f, 1.f, 0.f, 1.f},    // seam
	    {0.f, 1.f, 1.f, 1.f},    // complex
	    {1.f, 0.f, 0.f, 1.f},    // locked
	};

	// prepare meshes
	kinds.nodes = mesh.nodes;
	kinds.skin = mesh.skin;

	loops.nodes = mesh.nodes;
	loops.skin = mesh.skin;

	for (size_t i = 0; i < mesh.streams.size(); ++i)
	{
		const Stream& stream = mesh.streams[i];

		if (stream.target == 0 && (stream.type == cgltf_attribute_type_position || stream.type == cgltf_attribute_type_joints || stream.type == cgltf_attribute_type_weights))
		{
			kinds.streams.push_back(stream);
			loops.streams.push_back(stream);
		}
	}

	// transform kind/loop data into lines & points
	Stream colors = {cgltf_attribute_type_color};
	colors.data.resize(vertex_count);

	for (size_t i = 0; i < vertex_count; ++i)
		colors.data[i] = kPalette[kind[i]];

	kinds.type = cgltf_primitive_type_points;

	kinds.streams.push_back(colors);

	for (size_t i = 0; i < vertex_count; ++i)
		if (live[i] && kind[i] != 0)
			kinds.indices.push_back(unsigned(i));

	loops.type = cgltf_primitive_type_lines;

	loops.streams.push_back(colors);

	for (size_t i = 0; i < vertex_count; ++i)
		if (live[i] && (kind[i] == 1 || kind[i] == 2))
		{
			if (loop[i] != ~0u && live[loop[i]])
			{
				loops.indices.push_back(unsigned(i));
				loops.indices.push_back(loop[i]);
			}

			if (loopback[i] != ~0u && live[loopback[i]])
			{
				loops.indices.push_back(loopback[i]);
				loops.indices.push_back(unsigned(i));
			}
		}
}

void debugMeshlets(const Mesh& source, Mesh& meshlets, Mesh& bounds, int max_vertices, bool scan)
{
	Mesh mesh = source;
	assert(mesh.type == cgltf_primitive_type_triangles);

	reindexMesh(mesh);

	if (scan)
		optimizeMesh(mesh, false);

	const Stream* positions = getStream(mesh, cgltf_attribute_type_position);
	assert(positions);

	const float cone_weight = 0.f;

	size_t max_triangles = (max_vertices * 2 + 3) & ~3;
	size_t max_meshlets = meshopt_buildMeshletsBound(mesh.indices.size(), max_vertices, max_triangles);

	std::vector<meshopt_Meshlet> ml(max_meshlets);
	std::vector<unsigned int> mlv(max_meshlets * max_vertices);
	std::vector<unsigned char> mlt(max_meshlets * max_triangles * 3);

	if (scan)
		ml.resize(meshopt_buildMeshletsScan(&ml[0], &mlv[0], &mlt[0], &mesh.indices[0], mesh.indices.size(), positions->data.size(), max_vertices, max_triangles));
	else
		ml.resize(meshopt_buildMeshlets(&ml[0], &mlv[0], &mlt[0], &mesh.indices[0], mesh.indices.size(), positions->data[0].f, positions->data.size(), sizeof(Attr), max_vertices, max_triangles, cone_weight));

	// generate meshlet meshes, using unique colors
	meshlets.nodes = mesh.nodes;

	Stream mv = {cgltf_attribute_type_position};
	Stream mc = {cgltf_attribute_type_color};

	for (size_t i = 0; i < ml.size(); ++i)
	{
		const meshopt_Meshlet& m = ml[i];

		unsigned int h = unsigned(i);
		h ^= h >> 13;
		h *= 0x5bd1e995;
		h ^= h >> 15;

		Attr c = {{float(h & 0xff) / 255.f, float((h >> 8) & 0xff) / 255.f, float((h >> 16) & 0xff) / 255.f, 1.f}};

		unsigned int offset = unsigned(mv.data.size());

		for (size_t j = 0; j < m.vertex_count; ++j)
		{
			mv.data.push_back(positions->data[mlv[m.vertex_offset + j]]);
			mc.data.push_back(c);
		}

		for (size_t j = 0; j < m.triangle_count; ++j)
		{
			meshlets.indices.push_back(offset + mlt[m.triangle_offset + j * 3 + 0]);
			meshlets.indices.push_back(offset + mlt[m.triangle_offset + j * 3 + 1]);
			meshlets.indices.push_back(offset + mlt[m.triangle_offset + j * 3 + 2]);
		}
	}

	meshlets.type = cgltf_primitive_type_triangles;
	meshlets.streams.push_back(mv);
	meshlets.streams.push_back(mc);

	// generate bounds meshes, using a sphere per meshlet
	bounds.nodes = mesh.nodes;

	Stream bv = {cgltf_attribute_type_position};
	Stream bc = {cgltf_attribute_type_color};

	for (size_t i = 0; i < ml.size(); ++i)
	{
		const meshopt_Meshlet& m = ml[i];

		meshopt_Bounds mb = meshopt_computeMeshletBounds(&mlv[m.vertex_offset], &mlt[m.triangle_offset], m.triangle_count, positions->data[0].f, positions->data.size(), sizeof(Attr));

		unsigned int h = unsigned(i);
		h ^= h >> 13;
		h *= 0x5bd1e995;
		h ^= h >> 15;

		Attr c = {{float(h & 0xff) / 255.f, float((h >> 8) & 0xff) / 255.f, float((h >> 16) & 0xff) / 255.f, 0.1f}};

		unsigned int offset = unsigned(bv.data.size());

		const int N = 10;

		for (int y = 0; y <= N; ++y)
		{
			float u = (y == N) ? 0 : float(y) / N * 2 * 3.1415926f;
			float sinu = sinf(u), cosu = cosf(u);

			for (int x = 0; x <= N; ++x)
			{
				float v = float(x) / N * 3.1415926f;
				float sinv = sinf(v), cosv = cosf(v);

				float fx = sinv * cosu;
				float fy = sinv * sinu;
				float fz = cosv;

				Attr p = {{mb.center[0] + mb.radius * fx, mb.center[1] + mb.radius * fy, mb.center[2] + mb.radius * fz, 1.f}};

				bv.data.push_back(p);
				bc.data.push_back(c);
			}
		}

		for (int y = 0; y < N; ++y)
			for (int x = 0; x < N; ++x)
			{
				bounds.indices.push_back(offset + (N + 1) * (y + 0) + (x + 0));
				bounds.indices.push_back(offset + (N + 1) * (y + 0) + (x + 1));
				bounds.indices.push_back(offset + (N + 1) * (y + 1) + (x + 0));

				bounds.indices.push_back(offset + (N + 1) * (y + 1) + (x + 0));
				bounds.indices.push_back(offset + (N + 1) * (y + 0) + (x + 1));
				bounds.indices.push_back(offset + (N + 1) * (y + 1) + (x + 1));
			}
	}

	bounds.type = cgltf_primitive_type_triangles;
	bounds.streams.push_back(bv);
	bounds.streams.push_back(bc);
}
#endif
