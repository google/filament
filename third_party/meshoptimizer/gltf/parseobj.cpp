// This file is part of gltfpack; see gltfpack.h for version/license details
#include "gltfpack.h"

#include "../extern/fast_obj.h"
#include "../src/meshoptimizer.h"

#include <stdlib.h>
#include <string.h>

static void defaultFree(void*, void* p)
{
	free(p);
}

static int textureIndex(const std::vector<std::string>& textures, const char* name)
{
	for (size_t i = 0; i < textures.size(); ++i)
		if (textures[i] == name)
			return int(i);

	return -1;
}

static cgltf_data* parseSceneObj(fastObjMesh* obj)
{
	cgltf_data* data = (cgltf_data*)calloc(1, sizeof(cgltf_data));
	data->memory.free_func = defaultFree;

	std::vector<std::string> textures;

	for (unsigned int mi = 0; mi < obj->material_count; ++mi)
	{
		fastObjMaterial& om = obj->materials[mi];

		if (om.map_Kd.name && textureIndex(textures, om.map_Kd.name) < 0)
			textures.push_back(om.map_Kd.name);
	}

	data->images = (cgltf_image*)calloc(textures.size(), sizeof(cgltf_image));
	data->images_count = textures.size();

	for (size_t i = 0; i < textures.size(); ++i)
	{
		data->images[i].uri = (char*)malloc(textures[i].size() + 1);
		strcpy(data->images[i].uri, textures[i].c_str());
	}

	data->textures = (cgltf_texture*)calloc(textures.size(), sizeof(cgltf_texture));
	data->textures_count = textures.size();

	for (size_t i = 0; i < textures.size(); ++i)
	{
		data->textures[i].image = &data->images[i];
	}

	data->materials = (cgltf_material*)calloc(obj->material_count, sizeof(cgltf_material));
	data->materials_count = obj->material_count;

	for (unsigned int mi = 0; mi < obj->material_count; ++mi)
	{
		cgltf_material& gm = data->materials[mi];
		fastObjMaterial& om = obj->materials[mi];

		gm.has_pbr_metallic_roughness = true;
		gm.pbr_metallic_roughness.base_color_factor[0] = 1.0f;
		gm.pbr_metallic_roughness.base_color_factor[1] = 1.0f;
		gm.pbr_metallic_roughness.base_color_factor[2] = 1.0f;
		gm.pbr_metallic_roughness.base_color_factor[3] = 1.0f;
		gm.pbr_metallic_roughness.metallic_factor = 0.0f;
		gm.pbr_metallic_roughness.roughness_factor = 1.0f;

		gm.alpha_cutoff = 0.5f;

		if (om.map_Kd.name)
		{
			gm.pbr_metallic_roughness.base_color_texture.texture = &data->textures[textureIndex(textures, om.map_Kd.name)];
			gm.pbr_metallic_roughness.base_color_texture.scale = 1.0f;

			gm.alpha_mode = (om.illum == 4 || om.illum == 6 || om.illum == 7 || om.illum == 9) ? cgltf_alpha_mode_mask : cgltf_alpha_mode_opaque;
		}

		if (om.map_d.name)
		{
			gm.alpha_mode = cgltf_alpha_mode_blend;
		}
	}

	data->scenes = (cgltf_scene*)calloc(1, sizeof(cgltf_scene));
	data->scenes_count = 1;

	return data;
}

static void parseMeshObj(fastObjMesh* obj, unsigned int face_offset, unsigned int face_vertex_offset, unsigned int face_count, unsigned int face_vertex_count, unsigned int index_count, Mesh& mesh)
{
	std::vector<unsigned int> remap(face_vertex_count);
	size_t unique_vertices = meshopt_generateVertexRemap(remap.data(), nullptr, face_vertex_count, &obj->indices[face_vertex_offset], face_vertex_count, sizeof(fastObjIndex));

	int pos_stream = 0;
	int nrm_stream = obj->normal_count > 1 ? 1 : -1;
	int tex_stream = obj->texcoord_count > 1 ? 1 + (nrm_stream >= 0) : -1;

	mesh.streams.resize(1 + (nrm_stream >= 0) + (tex_stream >= 0));

	mesh.streams[pos_stream].type = cgltf_attribute_type_position;
	mesh.streams[pos_stream].data.resize(unique_vertices);

	if (nrm_stream >= 0)
	{
		mesh.streams[nrm_stream].type = cgltf_attribute_type_normal;
		mesh.streams[nrm_stream].data.resize(unique_vertices);
	}

	if (tex_stream >= 0)
	{
		mesh.streams[tex_stream].type = cgltf_attribute_type_texcoord;
		mesh.streams[tex_stream].data.resize(unique_vertices);
	}

	mesh.indices.resize(index_count);

	for (unsigned int vi = 0; vi < face_vertex_count; ++vi)
	{
		unsigned int target = remap[vi];
		// TODO: this fills every target vertex multiple times

		fastObjIndex ii = obj->indices[face_vertex_offset + vi];

		Attr p = {{obj->positions[ii.p * 3 + 0], obj->positions[ii.p * 3 + 1], obj->positions[ii.p * 3 + 2]}};
		mesh.streams[pos_stream].data[target] = p;

		if (nrm_stream >= 0)
		{
			Attr n = {{obj->normals[ii.n * 3 + 0], obj->normals[ii.n * 3 + 1], obj->normals[ii.n * 3 + 2]}};
			mesh.streams[nrm_stream].data[target] = n;
		}

		if (tex_stream >= 0)
		{
			Attr t = {{obj->texcoords[ii.t * 2 + 0], 1.f - obj->texcoords[ii.t * 2 + 1]}};
			mesh.streams[tex_stream].data[target] = t;
		}
	}

	unsigned int vertex_offset = 0;
	unsigned int index_offset = 0;

	for (unsigned int fi = 0; fi < face_count; ++fi)
	{
		unsigned int face_vertices = obj->face_vertices[face_offset + fi];

		for (unsigned int vi = 2; vi < face_vertices; ++vi)
		{
			size_t to = index_offset + (vi - 2) * 3;

			mesh.indices[to + 0] = remap[vertex_offset];
			mesh.indices[to + 1] = remap[vertex_offset + vi - 1];
			mesh.indices[to + 2] = remap[vertex_offset + vi];
		}

		vertex_offset += face_vertices;
		index_offset += (face_vertices - 2) * 3;
	}

	assert(vertex_offset == face_vertex_count);
	assert(index_offset == index_count);
}

static void parseMeshesObj(fastObjMesh* obj, cgltf_data* data, std::vector<Mesh>& meshes)
{
	unsigned int face_vertex_offset = 0;

	for (unsigned int face_offset = 0; face_offset < obj->face_count; )
	{
		unsigned int mi = obj->face_materials[face_offset];

		unsigned int face_count = 0;
		unsigned int face_vertex_count = 0;
		unsigned int index_count = 0;

		for (unsigned int fj = face_offset; fj < obj->face_count && obj->face_materials[fj] == mi; ++fj)
		{
			face_count += 1;
			face_vertex_count += obj->face_vertices[fj];
			index_count += (obj->face_vertices[fj] - 2) * 3;
		}

		meshes.push_back(Mesh());
		Mesh& mesh = meshes.back();

		if (data->materials_count)
		{
			assert(mi < data->materials_count);
			mesh.material = &data->materials[mi];
		}

		mesh.type = cgltf_primitive_type_triangles;
		mesh.targets = 0;

		parseMeshObj(obj, face_offset, face_vertex_offset, face_count, face_vertex_count, index_count, mesh);

		face_offset += face_count;
		face_vertex_offset += face_vertex_count;
	}
}

cgltf_data* parseObj(const char* path, std::vector<Mesh>& meshes, const char** error)
{
	fastObjMesh* obj = fast_obj_read(path);

	if (!obj)
	{
		*error = "file not found";
		return 0;
	}

	cgltf_data* data = parseSceneObj(obj);

	parseMeshesObj(obj, data, meshes);

	fast_obj_destroy(obj);

	return data;
}
