// This file is part of gltfpack; see gltfpack.h for version/license details
#include "gltfpack.h"

#include <string.h>

static bool areTextureViewsEqual(const cgltf_texture_view& lhs, const cgltf_texture_view& rhs)
{
	if (lhs.has_transform != rhs.has_transform)
		return false;

	if (lhs.has_transform)
	{
		const cgltf_texture_transform& lt = lhs.transform;
		const cgltf_texture_transform& rt = rhs.transform;

		if (memcmp(lt.offset, rt.offset, sizeof(cgltf_float) * 2) != 0)
			return false;

		if (lt.rotation != rt.rotation)
			return false;

		if (memcmp(lt.scale, rt.scale, sizeof(cgltf_float) * 2) != 0)
			return false;

		if (lt.texcoord != rt.texcoord)
			return false;
	}

	if (lhs.texture != rhs.texture)
		return false;

	if (lhs.texcoord != rhs.texcoord)
		return false;

	if (lhs.scale != rhs.scale)
		return false;

	return true;
}

static bool areExtrasEqual(const cgltf_extras& lhs, const cgltf_extras& rhs)
{
	if (lhs.data && rhs.data)
		return strcmp(lhs.data, rhs.data) == 0;
	else
		return lhs.data == rhs.data;
}

static bool areMaterialComponentsEqual(const cgltf_pbr_metallic_roughness& lhs, const cgltf_pbr_metallic_roughness& rhs)
{
	if (!areTextureViewsEqual(lhs.base_color_texture, rhs.base_color_texture))
		return false;

	if (!areTextureViewsEqual(lhs.metallic_roughness_texture, rhs.metallic_roughness_texture))
		return false;

	if (memcmp(lhs.base_color_factor, rhs.base_color_factor, sizeof(cgltf_float) * 4) != 0)
		return false;

	if (lhs.metallic_factor != rhs.metallic_factor)
		return false;

	if (lhs.roughness_factor != rhs.roughness_factor)
		return false;

	return true;
}

static bool areMaterialComponentsEqual(const cgltf_pbr_specular_glossiness& lhs, const cgltf_pbr_specular_glossiness& rhs)
{
	if (!areTextureViewsEqual(lhs.diffuse_texture, rhs.diffuse_texture))
		return false;

	if (!areTextureViewsEqual(lhs.specular_glossiness_texture, rhs.specular_glossiness_texture))
		return false;

	if (memcmp(lhs.diffuse_factor, rhs.diffuse_factor, sizeof(cgltf_float) * 4) != 0)
		return false;

	if (memcmp(lhs.specular_factor, rhs.specular_factor, sizeof(cgltf_float) * 3) != 0)
		return false;

	if (lhs.glossiness_factor != rhs.glossiness_factor)
		return false;

	return true;
}

static bool areMaterialComponentsEqual(const cgltf_clearcoat& lhs, const cgltf_clearcoat& rhs)
{
	if (!areTextureViewsEqual(lhs.clearcoat_texture, rhs.clearcoat_texture))
		return false;

	if (!areTextureViewsEqual(lhs.clearcoat_roughness_texture, rhs.clearcoat_roughness_texture))
		return false;

	if (!areTextureViewsEqual(lhs.clearcoat_normal_texture, rhs.clearcoat_normal_texture))
		return false;

	if (lhs.clearcoat_factor != rhs.clearcoat_factor)
		return false;

	if (lhs.clearcoat_roughness_factor != rhs.clearcoat_roughness_factor)
		return false;

	return true;
}

static bool areMaterialComponentsEqual(const cgltf_transmission& lhs, const cgltf_transmission& rhs)
{
	if (!areTextureViewsEqual(lhs.transmission_texture, rhs.transmission_texture))
		return false;

	if (lhs.transmission_factor != rhs.transmission_factor)
		return false;

	return true;
}

static bool areMaterialComponentsEqual(const cgltf_ior& lhs, const cgltf_ior& rhs)
{
	if (lhs.ior != rhs.ior)
		return false;

	return true;
}

static bool areMaterialComponentsEqual(const cgltf_specular& lhs, const cgltf_specular& rhs)
{
	if (!areTextureViewsEqual(lhs.specular_texture, rhs.specular_texture))
		return false;

	if (!areTextureViewsEqual(lhs.specular_color_texture, rhs.specular_color_texture))
		return false;

	if (lhs.specular_factor != rhs.specular_factor)
		return false;

	if (memcmp(lhs.specular_color_factor, rhs.specular_color_factor, sizeof(cgltf_float) * 3) != 0)
		return false;

	return true;
}

static bool areMaterialComponentsEqual(const cgltf_sheen& lhs, const cgltf_sheen& rhs)
{
	if (!areTextureViewsEqual(lhs.sheen_color_texture, rhs.sheen_color_texture))
		return false;

	if (memcmp(lhs.sheen_color_factor, rhs.sheen_color_factor, sizeof(cgltf_float) * 3) != 0)
		return false;

	if (!areTextureViewsEqual(lhs.sheen_roughness_texture, rhs.sheen_roughness_texture))
		return false;

	if (lhs.sheen_roughness_factor != rhs.sheen_roughness_factor)
		return false;

	return true;
}

static bool areMaterialComponentsEqual(const cgltf_volume& lhs, const cgltf_volume& rhs)
{
	if (!areTextureViewsEqual(lhs.thickness_texture, rhs.thickness_texture))
		return false;

	if (lhs.thickness_factor != rhs.thickness_factor)
		return false;

	if (memcmp(lhs.attenuation_color, rhs.attenuation_color, sizeof(cgltf_float) * 3) != 0)
		return false;

	if (lhs.attenuation_distance != rhs.attenuation_distance)
		return false;

	return true;
}

static bool areMaterialComponentsEqual(const cgltf_emissive_strength& lhs, const cgltf_emissive_strength& rhs)
{
	if (lhs.emissive_strength != rhs.emissive_strength)
		return false;

	return true;
}

static bool areMaterialComponentsEqual(const cgltf_iridescence& lhs, const cgltf_iridescence& rhs)
{
	if (lhs.iridescence_factor != rhs.iridescence_factor)
		return false;

	if (!areTextureViewsEqual(lhs.iridescence_texture, rhs.iridescence_texture))
		return false;

	if (lhs.iridescence_ior != rhs.iridescence_ior)
		return false;

	if (lhs.iridescence_thickness_min != rhs.iridescence_thickness_min)
		return false;

	if (lhs.iridescence_thickness_max != rhs.iridescence_thickness_max)
		return false;

	if (!areTextureViewsEqual(lhs.iridescence_thickness_texture, rhs.iridescence_thickness_texture))
		return false;

	return true;
}

static bool areMaterialsEqual(const cgltf_material& lhs, const cgltf_material& rhs, const Settings& settings)
{
	if (lhs.has_pbr_metallic_roughness != rhs.has_pbr_metallic_roughness)
		return false;

	if (lhs.has_pbr_metallic_roughness && !areMaterialComponentsEqual(lhs.pbr_metallic_roughness, rhs.pbr_metallic_roughness))
		return false;

	if (lhs.has_pbr_specular_glossiness != rhs.has_pbr_specular_glossiness)
		return false;

	if (lhs.has_pbr_specular_glossiness && !areMaterialComponentsEqual(lhs.pbr_specular_glossiness, rhs.pbr_specular_glossiness))
		return false;

	if (lhs.has_clearcoat != rhs.has_clearcoat)
		return false;

	if (lhs.has_clearcoat && !areMaterialComponentsEqual(lhs.clearcoat, rhs.clearcoat))
		return false;

	if (lhs.has_transmission != rhs.has_transmission)
		return false;

	if (lhs.has_transmission && !areMaterialComponentsEqual(lhs.transmission, rhs.transmission))
		return false;

	if (lhs.has_ior != rhs.has_ior)
		return false;

	if (lhs.has_ior && !areMaterialComponentsEqual(lhs.ior, rhs.ior))
		return false;

	if (lhs.has_specular != rhs.has_specular)
		return false;

	if (lhs.has_specular && !areMaterialComponentsEqual(lhs.specular, rhs.specular))
		return false;

	if (lhs.has_sheen != rhs.has_sheen)
		return false;

	if (lhs.has_sheen && !areMaterialComponentsEqual(lhs.sheen, rhs.sheen))
		return false;

	if (lhs.has_volume != rhs.has_volume)
		return false;

	if (lhs.has_volume && !areMaterialComponentsEqual(lhs.volume, rhs.volume))
		return false;

	if (lhs.has_emissive_strength != rhs.has_emissive_strength)
		return false;

	if (lhs.has_emissive_strength && !areMaterialComponentsEqual(lhs.emissive_strength, rhs.emissive_strength))
		return false;

	if (lhs.has_iridescence != rhs.has_iridescence)
		return false;

	if (lhs.has_iridescence && !areMaterialComponentsEqual(lhs.iridescence, rhs.iridescence))
		return false;

	if (!areTextureViewsEqual(lhs.normal_texture, rhs.normal_texture))
		return false;

	if (!areTextureViewsEqual(lhs.occlusion_texture, rhs.occlusion_texture))
		return false;

	if (!areTextureViewsEqual(lhs.emissive_texture, rhs.emissive_texture))
		return false;

	if (memcmp(lhs.emissive_factor, rhs.emissive_factor, sizeof(cgltf_float) * 3) != 0)
		return false;

	if (lhs.alpha_mode != rhs.alpha_mode)
		return false;

	if (lhs.alpha_cutoff != rhs.alpha_cutoff)
		return false;

	if (lhs.double_sided != rhs.double_sided)
		return false;

	if (lhs.unlit != rhs.unlit)
		return false;

	if (settings.keep_extras && !areExtrasEqual(lhs.extras, rhs.extras))
		return false;

	return true;
}

void mergeMeshMaterials(cgltf_data* data, std::vector<Mesh>& meshes, const Settings& settings)
{
	std::vector<cgltf_material*> material_remap(data->materials_count);

	for (size_t i = 0; i < data->materials_count; ++i)
	{
		material_remap[i] = &data->materials[i];

		if (settings.keep_materials && data->materials[i].name && *data->materials[i].name)
			continue;

		for (size_t j = 0; j < i; ++j)
		{
			if (settings.keep_materials && data->materials[j].name && *data->materials[j].name)
				continue;

			if (areMaterialsEqual(data->materials[i], data->materials[j], settings))
			{
				material_remap[i] = &data->materials[j];
				break;
			}
		}
	}

	for (size_t i = 0; i < meshes.size(); ++i)
	{
		Mesh& mesh = meshes[i];

		if (mesh.material)
			mesh.material = material_remap[mesh.material - data->materials];

		for (size_t j = 0; j < mesh.variants.size(); ++j)
			mesh.variants[j].material = material_remap[mesh.variants[j].material - data->materials];
	}
}

void markNeededMaterials(cgltf_data* data, std::vector<MaterialInfo>& materials, const std::vector<Mesh>& meshes, const Settings& settings)
{
	// mark all used materials as kept
	for (size_t i = 0; i < meshes.size(); ++i)
	{
		const Mesh& mesh = meshes[i];

		if (mesh.material)
		{
			MaterialInfo& mi = materials[mesh.material - data->materials];

			mi.keep = true;
		}

		for (size_t j = 0; j < mesh.variants.size(); ++j)
		{
			MaterialInfo& mi = materials[mesh.variants[j].material - data->materials];

			mi.keep = true;
		}
	}

	// mark all named materials as kept if requested
	if (settings.keep_materials)
	{
		for (size_t i = 0; i < data->materials_count; ++i)
		{
			cgltf_material& material = data->materials[i];

			if (material.name && *material.name)
			{
				materials[i].keep = true;
			}
		}
	}
}

bool hasValidTransform(const cgltf_texture_view& view)
{
	if (view.has_transform)
	{
		if (view.transform.offset[0] != 0.0f || view.transform.offset[1] != 0.0f ||
		    view.transform.scale[0] != 1.0f || view.transform.scale[1] != 1.0f ||
		    view.transform.rotation != 0.0f)
			return true;

		if (view.transform.has_texcoord && view.transform.texcoord != view.texcoord)
			return true;
	}

	return false;
}

static void analyzeMaterialTexture(const cgltf_texture_view& view, TextureKind kind, MaterialInfo& mi, cgltf_data* data, std::vector<ImageInfo>& images)
{
	mi.usesTextureTransform |= hasValidTransform(view);

	if (view.texture && view.texture->image)
	{
		ImageInfo& info = images[view.texture->image - data->images];

		mi.textureSetMask |= 1u << view.texcoord;
		mi.needsTangents |= (kind == TextureKind_Normal);

		if (info.kind == TextureKind_Generic)
			info.kind = kind;
		else if (info.kind > kind) // this is useful to keep color textures that have attrib data in alpha tagged as color
			info.kind = kind;

		info.normal_map |= (kind == TextureKind_Normal);
		info.srgb |= (kind == TextureKind_Color);
	}
}

static void analyzeMaterial(const cgltf_material& material, MaterialInfo& mi, cgltf_data* data, std::vector<ImageInfo>& images)
{
	if (material.has_pbr_metallic_roughness)
	{
		analyzeMaterialTexture(material.pbr_metallic_roughness.base_color_texture, TextureKind_Color, mi, data, images);
		analyzeMaterialTexture(material.pbr_metallic_roughness.metallic_roughness_texture, TextureKind_Attrib, mi, data, images);
	}

	if (material.has_pbr_specular_glossiness)
	{
		analyzeMaterialTexture(material.pbr_specular_glossiness.diffuse_texture, TextureKind_Color, mi, data, images);
		analyzeMaterialTexture(material.pbr_specular_glossiness.specular_glossiness_texture, TextureKind_Attrib, mi, data, images);
	}

	if (material.has_clearcoat)
	{
		analyzeMaterialTexture(material.clearcoat.clearcoat_texture, TextureKind_Attrib, mi, data, images);
		analyzeMaterialTexture(material.clearcoat.clearcoat_roughness_texture, TextureKind_Attrib, mi, data, images);
		analyzeMaterialTexture(material.clearcoat.clearcoat_normal_texture, TextureKind_Normal, mi, data, images);
	}

	if (material.has_transmission)
	{
		analyzeMaterialTexture(material.transmission.transmission_texture, TextureKind_Attrib, mi, data, images);
	}

	if (material.has_specular)
	{
		analyzeMaterialTexture(material.specular.specular_texture, TextureKind_Attrib, mi, data, images);
		analyzeMaterialTexture(material.specular.specular_color_texture, TextureKind_Color, mi, data, images);
	}

	if (material.has_sheen)
	{
		analyzeMaterialTexture(material.sheen.sheen_color_texture, TextureKind_Color, mi, data, images);
		analyzeMaterialTexture(material.sheen.sheen_roughness_texture, TextureKind_Attrib, mi, data, images);
	}

	if (material.has_volume)
	{
		analyzeMaterialTexture(material.volume.thickness_texture, TextureKind_Attrib, mi, data, images);
	}

	if (material.has_iridescence)
	{
		analyzeMaterialTexture(material.iridescence.iridescence_texture, TextureKind_Attrib, mi, data, images);
		analyzeMaterialTexture(material.iridescence.iridescence_thickness_texture, TextureKind_Attrib, mi, data, images);
	}

	analyzeMaterialTexture(material.normal_texture, TextureKind_Normal, mi, data, images);
	analyzeMaterialTexture(material.occlusion_texture, TextureKind_Attrib, mi, data, images);
	analyzeMaterialTexture(material.emissive_texture, TextureKind_Color, mi, data, images);
}

void analyzeMaterials(cgltf_data* data, std::vector<MaterialInfo>& materials, std::vector<ImageInfo>& images)
{
	for (size_t i = 0; i < data->materials_count; ++i)
	{
		analyzeMaterial(data->materials[i], materials[i], data, images);
	}
}

static const cgltf_texture_view* getColorTexture(const cgltf_material& material)
{
	if (material.has_pbr_metallic_roughness)
		return &material.pbr_metallic_roughness.base_color_texture;

	if (material.has_pbr_specular_glossiness)
		return &material.pbr_specular_glossiness.diffuse_texture;

	return NULL;
}

static float getAlphaFactor(const cgltf_material& material)
{
	if (material.has_pbr_metallic_roughness)
		return material.pbr_metallic_roughness.base_color_factor[3];

	if (material.has_pbr_specular_glossiness)
		return material.pbr_specular_glossiness.diffuse_factor[3];

	return 1.f;
}

static int getChannels(const cgltf_image& image, ImageInfo& info, const char* input_path)
{
	if (info.channels)
		return info.channels;

	std::string img_data;
	std::string mime_type;
	if (readImage(image, input_path, img_data, mime_type))
		info.channels = hasAlpha(img_data, mime_type.c_str()) ? 4 : 3;
	else
		info.channels = -1;

	return info.channels;
}

void optimizeMaterials(cgltf_data* data, const char* input_path, std::vector<ImageInfo>& images)
{
	for (size_t i = 0; i < data->materials_count; ++i)
	{
		// remove BLEND/MASK from materials that don't have alpha information
		if (data->materials[i].alpha_mode != cgltf_alpha_mode_opaque)
		{
			const cgltf_texture_view* color = getColorTexture(data->materials[i]);
			float alpha = getAlphaFactor(data->materials[i]);

			if (alpha == 1.f && !(color && color->texture && color->texture->image && getChannels(*color->texture->image, images[color->texture->image - data->images], input_path) == 4))
			{
				data->materials[i].alpha_mode = cgltf_alpha_mode_opaque;
			}
		}
	}
}
