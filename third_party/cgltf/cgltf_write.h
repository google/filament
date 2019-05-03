/**
 * cgltf_write - a single-file glTF 2.0 writer written in C99.
 *
 * Version: 1.0
 *
 * Website: https://github.com/jkuhlmann/cgltf
 *
 * Distributed under the MIT License, see notice at the end of this file.
 *
 * Building:
 * Include this file where you need the struct and function
 * declarations. Have exactly one source file where you define
 * `CGLTF_WRITE_IMPLEMENTATION` before including this file to get the
 * function definitions.
 *
 * Reference:
 * `cgltf_write_file` writes JSON to the given file path. Buffer files and external images are not
 * written out.
 *
 * `cgltf_write` writes JSON into the given memory buffer. Returns the number of bytes written to
 * "buffer". If buffer is null, returns the number of bytes that would have been written.
 */
#ifndef CGLTF_WRITE_H_INCLUDED__
#define CGLTF_WRITE_H_INCLUDED__

#include "cgltf.h"

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

cgltf_result cgltf_write_file(const cgltf_options* options, const char* path, const cgltf_data* data);
cgltf_size cgltf_write(const cgltf_options* options, char* buffer, cgltf_size size, const cgltf_data* data);

#ifdef __cplusplus
}
#endif

#endif /* #ifndef CGLTF_WRITE_H_INCLUDED__ */

/*
 *
 * Stop now, if you are only interested in the API.
 * Below, you find the implementation.
 *
 */

#ifdef __INTELLISENSE__
/* This makes MSVC intellisense work. */
#define CGLTF_WRITE_IMPLEMENTATION
#endif

#ifdef CGLTF_WRITE_IMPLEMENTATION

#include <stdio.h>

typedef struct {
	char* buffer;
	size_t buffer_size;
	size_t remaining;
	char* cursor;
	size_t tmp;
	size_t chars_written;
	const cgltf_data* data;
	int depth;
	const char* indent;
	int needs_comma;
} cgltf_write_context;

#define CGLTF_SPRINTF(fmt, ...) { \
		context->tmp = snprintf ( context->cursor, context->remaining, fmt, ## __VA_ARGS__ ); \
		context->chars_written += context->tmp; \
		if (context->cursor) { \
			context->cursor += context->tmp; \
			context->remaining -= context->tmp; \
		} }

#define CGLTF_WRITE_IDXPROP(label, val, start) if (val) { \
		cgltf_write_indent(context); \
		CGLTF_SPRINTF("\"%s\": %d", label, (int) (val - start)); \
		context->needs_comma = 1; }

#define CGLTF_WRITE_IDXARRPROP(label, dim, vals, start) if (vals) { \
		cgltf_write_indent(context); \
		CGLTF_SPRINTF("\"%s\": [", label); \
		for (int i = 0; i < dim; ++i) { \
			int idx = (int) (vals[i] - start); \
			if (i != 0) CGLTF_SPRINTF(","); \
			CGLTF_SPRINTF(" %d", idx); \
		} \
		CGLTF_SPRINTF(" ]"); \
		context->needs_comma = 1; }

// TODO: scale and transform
#define CGLTF_WRITE_TEXTURE_INFO(label, info) if (info.texture) { \
		cgltf_write_line(context, "\"" label "\": {"); \
		CGLTF_WRITE_IDXPROP("index", info.texture, context->data->textures); \
		cgltf_write_intprop(context, "texCoord", info.texcoord, 0); \
		cgltf_write_line(context, "}"); }

static void cgltf_write_indent(cgltf_write_context* context)
{
	if (context->needs_comma)
	{
		CGLTF_SPRINTF(",\n");
		context->needs_comma = 0;
	}
	else
	{
		CGLTF_SPRINTF("\n");
	}
	for (int i = 0; i < context->depth; ++i)
	{
		CGLTF_SPRINTF("%s", context->indent);
	}
}

static void cgltf_write_line(cgltf_write_context* context, const char* line)
{
	if (line[0] == ']' || line[0] == '}')
	{
		--context->depth;
		context->needs_comma = 0;
	}
	cgltf_write_indent(context);
	CGLTF_SPRINTF("%s", line);
	int last = strlen(line) - 1;
	if (line[0] == ']' || line[0] == '}')
	{
		context->needs_comma = 1;
	}
	if (line[last] == '[' || line[last] == '{')
	{
		++context->depth;
		context->needs_comma = 0;
	}
}

static void cgltf_write_strprop(cgltf_write_context* context, const char* label, const char* val)
{
	if (val)
	{
		cgltf_write_indent(context);
		CGLTF_SPRINTF("\"%s\": \"%s\"", label, val);
		context->needs_comma = 1;
	}
}

static void cgltf_write_intprop(cgltf_write_context* context, const char* label, int val, int def)
{
	if (val != def)
	{
		cgltf_write_indent(context);
		CGLTF_SPRINTF("\"%s\": %d", label, val);
		context->needs_comma = 1;
	}
}

static void cgltf_write_floatprop(cgltf_write_context* context, const char* label, float val, float def)
{
	if (val != def)
	{
		cgltf_write_indent(context);
		CGLTF_SPRINTF("\"%s\": %g", label, val);
		context->needs_comma = 1;
	}
}

static void cgltf_write_boolprop_optional(cgltf_write_context* context, const char* label, bool val, bool def)
{
	if (val != def)
	{
		cgltf_write_indent(context);
		CGLTF_SPRINTF("\"%s\": %s", label, val ? "true" : "false");
		context->needs_comma = 1;
	}
}

static void cgltf_write_floatarrayprop(cgltf_write_context* context, const char* label, const cgltf_float* vals, int dim)
{
	cgltf_write_indent(context);
	CGLTF_SPRINTF("\"%s\": [", label);
	for (int i = 0; i < dim; ++i)
	{
		if (i != 0)
		{
			CGLTF_SPRINTF(", %g", vals[i]);
		}
		else
		{
			CGLTF_SPRINTF("%g", vals[i]);
		}
	}
	CGLTF_SPRINTF("]");
	context->needs_comma = 1;
}

static bool cgltf_check_floatarray(const float* vals, int dim, float val) {
	while (dim--)
	{
		if (vals[dim] != val)
		{
			return true;
		}
	}
	return false;
}

static int cgltf_int_from_component_type(cgltf_component_type ctype)
{
	switch (ctype)
	{
		case cgltf_component_type_invalid: return 0;
		case cgltf_component_type_r_8: return 5120;
		case cgltf_component_type_r_8u: return 5121;
		case cgltf_component_type_r_16: return 5122;
		case cgltf_component_type_r_16u: return 5123;
		case cgltf_component_type_r_32u: return 5125;
		case cgltf_component_type_r_32f: return 5126;
	}
}

static const char* cgltf_str_from_alpha_mode(cgltf_alpha_mode alpha_mode)
{
	switch (alpha_mode)
	{
		case cgltf_alpha_mode_opaque: return 0;
		case cgltf_alpha_mode_mask: return "MASK";
		case cgltf_alpha_mode_blend: return "BLEND";
	}
}

static const char* cgltf_str_from_type(cgltf_type type)
{
	switch (type)
	{
		case cgltf_type_invalid: return 0;
		case cgltf_type_scalar: return "SCALAR";
		case cgltf_type_vec2: return "VEC2";
		case cgltf_type_vec3: return "VEC3";
		case cgltf_type_vec4: return "VEC4";
		case cgltf_type_mat2: return "MAT2";
		case cgltf_type_mat3: return "MAT3";
		case cgltf_type_mat4: return "MAT4";
	}
}

static int cgltf_dim_from_type(cgltf_type type)
{
	switch (type)
	{
		case cgltf_type_invalid: return 0;
		case cgltf_type_scalar: return 1;
		case cgltf_type_vec2: return 2;
		case cgltf_type_vec3: return 3;
		case cgltf_type_vec4: return 4;
		case cgltf_type_mat2: return 4;
		case cgltf_type_mat3: return 9;
		case cgltf_type_mat4: return 16;
	}
}

static void cgltf_write_asset(cgltf_write_context* context, const cgltf_asset* asset)
{
	cgltf_write_line(context, "\"asset\": {");
	cgltf_write_strprop(context, "copyright", asset->copyright);
	cgltf_write_strprop(context, "generator", asset->generator);
	cgltf_write_strprop(context, "version", asset->version);
	cgltf_write_strprop(context, "min_version", asset->min_version);
	cgltf_write_line(context, "}");
}

static void cgltf_write_primitive(cgltf_write_context* context, const cgltf_primitive* prim)
{
	cgltf_write_intprop(context, "mode", (int) prim->type, 4);
	CGLTF_WRITE_IDXPROP("indices", prim->indices, context->data->accessors);
	CGLTF_WRITE_IDXPROP("material", prim->material, context->data->materials);
	cgltf_write_line(context, "\"attributes\": {");
	for (cgltf_size i = 0; i < prim->attributes_count; ++i)
	{
		const cgltf_attribute* attr = prim->attributes + i;
		CGLTF_WRITE_IDXPROP(attr->name, attr->data, context->data->accessors);
	}
	cgltf_write_line(context, "}");

	// TODO: prim->targets
}

static void cgltf_write_mesh(cgltf_write_context* context, const cgltf_mesh* mesh)
{
	cgltf_write_line(context, "{");
	cgltf_write_strprop(context, "name", mesh->name);

	cgltf_write_line(context, "\"primitives\": [");
	for (cgltf_size i = 0; i < mesh->primitives_count; ++i)
	{
		cgltf_write_line(context, "{");
		cgltf_write_primitive(context, mesh->primitives + i);
		cgltf_write_line(context, "}");
	}
	cgltf_write_line(context, "]");

	// TODO: mesh->weights

	cgltf_write_line(context, "}");
}

static void cgltf_write_buffer_view(cgltf_write_context* context, const cgltf_buffer_view* view)
{
	cgltf_write_line(context, "{");
	CGLTF_WRITE_IDXPROP("buffer", view->buffer, context->data->buffers);
	cgltf_write_intprop(context, "byteLength", view->size, -1);
	cgltf_write_intprop(context, "byteOffset", view->offset, 0);
	cgltf_write_intprop(context, "byteStride", view->stride, 0);
	// NOTE: We skip writing "target" because the spec says its usage can be inferred.
	// NOTE: The spec defines name / extensions / extras but cgltf does not read these.
	cgltf_write_line(context, "}");
}


static void cgltf_write_buffer(cgltf_write_context* context, const cgltf_buffer* buffer)
{
	cgltf_write_line(context, "{");
	cgltf_write_strprop(context, "uri", buffer->uri);
	cgltf_write_intprop(context, "byteLength", buffer->size, -1);
	// NOTE: The spec defines name / extensions / extras but cgltf does not read these.
	cgltf_write_line(context, "}");
}

static void cgltf_write_material(cgltf_write_context* context, const cgltf_material* material)
{
	cgltf_write_line(context, "{");
	cgltf_write_strprop(context, "name", material->name);
	cgltf_write_floatprop(context, "alphaCutoff", material->alpha_cutoff, 0.5f);
	cgltf_write_boolprop_optional(context, "doubleSided", material->double_sided, false);
	cgltf_write_boolprop_optional(context, "unlit", material->unlit, false);

	if (material->has_pbr_metallic_roughness)
	{
		const auto& params = material->pbr_metallic_roughness;
		cgltf_write_line(context, "\"pbrMetallicRoughness\": {");
		CGLTF_WRITE_TEXTURE_INFO("baseColorTexture", params.base_color_texture);
		CGLTF_WRITE_TEXTURE_INFO("metallicRoughnessTexture", params.metallic_roughness_texture);
		cgltf_write_floatprop(context, "metallicFactor", params.metallic_factor, 1.0f);
		cgltf_write_floatprop(context, "roughnessFactor", params.roughness_factor, 1.0f);
		if (cgltf_check_floatarray(params.base_color_factor, 4, 1.0f))
		{
			cgltf_write_floatarrayprop(context, "baseColorFactor", params.base_color_factor, 4);
		}
		cgltf_write_line(context, "}");
	}

	if (material->has_pbr_specular_glossiness)
	{
		const auto& params = material->pbr_specular_glossiness;
		cgltf_write_line(context, "\"extensions\": {");
		cgltf_write_line(context, "\"KHR_materials_pbrSpecularGlossiness\": {");
		CGLTF_WRITE_TEXTURE_INFO("diffuseTexture", params.diffuse_texture);
		CGLTF_WRITE_TEXTURE_INFO("specularGlossinessTexture", params.specular_glossiness_texture);
		if (cgltf_check_floatarray(params.diffuse_factor, 4, 1.0f))
		{
			cgltf_write_floatarrayprop(context, "dffuseFactor", params.diffuse_factor, 4);
		}
		if (cgltf_check_floatarray(params.specular_factor, 3, 1.0f))
		{
			cgltf_write_floatarrayprop(context, "specularFactor", params.specular_factor, 3);
		}
		cgltf_write_floatprop(context, "glossinessFactor", params.glossiness_factor, 1.0f);
		cgltf_write_line(context, "}");
		cgltf_write_line(context, "}");
	}

	CGLTF_WRITE_TEXTURE_INFO("normalTexture", material->normal_texture);
	CGLTF_WRITE_TEXTURE_INFO("occlusionTexture", material->occlusion_texture);
	CGLTF_WRITE_TEXTURE_INFO("emissiveTexture", material->emissive_texture);
	if (cgltf_check_floatarray(material->emissive_factor, 3, 0.0f))
	{
		cgltf_write_floatarrayprop(context, "emissiveFactor", material->emissive_factor, 3);
	}
	cgltf_write_strprop(context, "alphaMode", cgltf_str_from_alpha_mode(material->alpha_mode));
	cgltf_write_line(context, "}");
}

static void cgltf_write_image(cgltf_write_context* context, const cgltf_image* image)
{
	cgltf_write_line(context, "{");
	cgltf_write_strprop(context, "name", image->name);
	cgltf_write_strprop(context, "uri", image->uri);
	CGLTF_WRITE_IDXPROP("bufferView", image->buffer_view, context->data->buffer_views);
	cgltf_write_strprop(context, "mime_type", image->mime_type);
	cgltf_write_line(context, "}");
}

static void cgltf_write_texture(cgltf_write_context* context, const cgltf_texture* texture)
{
	cgltf_write_line(context, "{");
	cgltf_write_strprop(context, "name", texture->name);
	CGLTF_WRITE_IDXPROP("source", texture->image, context->data->images);
	CGLTF_WRITE_IDXPROP("sampler", texture->sampler, context->data->samplers);
	cgltf_write_line(context, "}");
}

static void cgltf_write_sampler(cgltf_write_context* context, const cgltf_sampler* sampler)
{
	cgltf_write_line(context, "{");
	cgltf_write_intprop(context, "magFilter", sampler->mag_filter, 0);
	cgltf_write_intprop(context, "minFilter", sampler->min_filter, 0);
	cgltf_write_intprop(context, "wrapS", sampler->wrap_s, 10497);
	cgltf_write_intprop(context, "wrapT", sampler->wrap_t, 10497);
	cgltf_write_line(context, "}");
}

static void cgltf_write_node(cgltf_write_context* context, const cgltf_node* node)
{
	cgltf_write_line(context, "{");
	CGLTF_WRITE_IDXARRPROP("children", node->children_count, node->children, context->data->nodes);
	CGLTF_WRITE_IDXPROP("mesh", node->mesh, context->data->meshes);
	cgltf_write_strprop(context, "name", node->name);
	if (node->has_matrix)
	{
		cgltf_write_floatarrayprop(context, "matrix", node->matrix, 16);
	}
	if (node->has_translation)
	{
		cgltf_write_floatarrayprop(context, "translation", node->translation, 3);
	}
	if (node->has_rotation)
	{
		cgltf_write_floatarrayprop(context, "rotation", node->rotation, 4);
	}
	if (node->has_scale)
	{
		cgltf_write_floatarrayprop(context, "scale", node->scale, 3);
	}
	// TODO: skin, weights, light, camera
	cgltf_write_line(context, "}");
}

static void cgltf_write_scene(cgltf_write_context* context, const cgltf_scene* scene)
{
	cgltf_write_line(context, "{");
	cgltf_write_strprop(context, "name", scene->name);
	CGLTF_WRITE_IDXARRPROP("nodes", scene->nodes_count, scene->nodes, context->data->nodes);
	cgltf_write_line(context, "}");
}

static void cgltf_write_accessor(cgltf_write_context* context, const cgltf_accessor* accessor)
{
	cgltf_write_line(context, "{");
	CGLTF_WRITE_IDXPROP("bufferView", accessor->buffer_view, context->data->buffer_views);
	cgltf_write_intprop(context, "componentType", cgltf_int_from_component_type(accessor->component_type), 0);
	cgltf_write_strprop(context, "type", cgltf_str_from_type(accessor->type));
	int dim = cgltf_dim_from_type(accessor->type);
	cgltf_write_boolprop_optional(context, "normalized", accessor->normalized, false);
	cgltf_write_intprop(context, "byteOffset", accessor->offset, 0);
	cgltf_write_intprop(context, "count", accessor->count, -1);
	if (accessor->has_min)
	{
		cgltf_write_floatarrayprop(context, "min", accessor->min, dim);
	}
	if (accessor->has_max)
	{
		cgltf_write_floatarrayprop(context, "max", accessor->max, dim);
	}
	cgltf_write_line(context, "}");
	// Note that stride is just an annotation and should not be written out.
	// TODO: accessor->sparse
}

cgltf_result cgltf_write_file(const cgltf_options* options, const char* path, const cgltf_data* data)
{
	size_t size = cgltf_write(options, NULL, 0, data);
	char* buffer = (char*) malloc(size);
	size = cgltf_write(options, buffer, size, data);
	FILE* file = fopen(path, "wt");
	fwrite(buffer, size, 1, file);
	fclose(file);
	free(buffer);
	return cgltf_result_success;
}

cgltf_size cgltf_write(const cgltf_options* options, char* buffer, cgltf_size size, const cgltf_data* data)
{
	cgltf_write_context ctx;
	ctx.buffer = buffer;
	ctx.buffer_size = size;
	ctx.remaining = size;
	ctx.cursor = buffer;
	ctx.chars_written = 0;
	ctx.data = data;
	ctx.depth = 1;
	ctx.indent = "  ";
	ctx.needs_comma = 0;

	cgltf_write_context* context = &ctx;

	CGLTF_SPRINTF("{");

	if (data->accessors_count > 0)
	{
		cgltf_write_line(context, "\"accessors\": [");
		for (cgltf_size i = 0; i < data->accessors_count; ++i)
		{
			cgltf_write_accessor(context, data->accessors + i);
		}
		cgltf_write_line(context, "]");
	}

	cgltf_write_asset(context, &data->asset);

	if (data->buffer_views_count > 0)
	{
		cgltf_write_line(context, "\"bufferViews\": [");
		for (cgltf_size i = 0; i < data->buffer_views_count; ++i)
		{
			cgltf_write_buffer_view(context, data->buffer_views + i);
		}
		cgltf_write_line(context, "]");
	}

	if (data->buffers_count > 0)
	{
		cgltf_write_line(context, "\"buffers\": [");
		for (cgltf_size i = 0; i < data->buffers_count; ++i)
		{
			cgltf_write_buffer(context, data->buffers + i);
		}
		cgltf_write_line(context, "]");
	}

	if (data->images_count > 0)
	{
		cgltf_write_line(context, "\"images\": [");
		for (cgltf_size i = 0; i < data->images_count; ++i)
		{
			cgltf_write_image(context, data->images + i);
		}
		cgltf_write_line(context, "]");
	}

	if (data->meshes_count > 0)
	{
		cgltf_write_line(context, "\"meshes\": [");
		for (cgltf_size i = 0; i < data->meshes_count; ++i)
		{
			cgltf_write_mesh(context, data->meshes + i);
		}
		cgltf_write_line(context, "]");
	}

	if (data->materials_count > 0)
	{
		cgltf_write_line(context, "\"materials\": [");
		for (cgltf_size i = 0; i < data->materials_count; ++i)
		{
			cgltf_write_material(context, data->materials + i);
		}
		cgltf_write_line(context, "]");
	}

	if (data->nodes_count > 0)
	{
		cgltf_write_line(context, "\"nodes\": [");
		for (cgltf_size i = 0; i < data->nodes_count; ++i)
		{
			cgltf_write_node(context, data->nodes + i);
		}
		cgltf_write_line(context, "]");
	}

	if (data->samplers_count > 0)
	{
		cgltf_write_line(context, "\"samplers\": [");
		for (cgltf_size i = 0; i < data->samplers_count; ++i)
		{
			cgltf_write_sampler(context, data->samplers + i);
		}
		cgltf_write_line(context, "]");
	}

	cgltf_write_intprop(context, "scene", data->scene - data->scenes, -1);

	if (data->scenes_count > 0)
	{
		cgltf_write_line(context, "\"scenes\": [");
		for (cgltf_size i = 0; i < data->scenes_count; ++i)
		{
			cgltf_write_scene(context, data->scenes + i);
		}
		cgltf_write_line(context, "]");
	}

	if (data->textures_count > 0)
	{
		cgltf_write_line(context, "\"textures\": [");
		for (cgltf_size i = 0; i < data->textures_count; ++i)
		{
			cgltf_write_texture(context, data->textures + i);
		}
		cgltf_write_line(context, "]");
	}

	// TODO: skins, animations, cameras, extensions

	CGLTF_SPRINTF("\n}\n");

	// snprintf does not include the null terminator in its return value, so be sure to include it
	// in the returned byte count.
	return 1 + ctx.chars_written;
}

#endif /* #ifdef CGLTF_WRITE_IMPLEMENTATION */

/* cgltf is distributed under MIT license:
 *
 * Copyright (c) 2018 Johannes Kuhlmann

 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:

 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.

 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */
