// File: basisu_wasm_api.cpp - Simplified compression API for WASM WASI modules and Python native support.
// Also useable by plain C callers.
#include "basisu_comp.h"
#include "basisu_wasm_api.h"

using namespace basisu;

static inline uint64_t wasm_offset(void* p)
{
    return (uint64_t)(uintptr_t)p;
}

static inline uint8_t* wasm_ptr(uint64_t offset)
{
	return (uint8_t*)(uintptr_t)offset;
}

BU_WASM_EXPORT("bu_get_version")
uint32_t bu_get_version()
{
	printf("Hello from basisu_wasm_api.cpp version %u\n", BASISU_LIB_VERSION);
		
	return BASISU_LIB_VERSION;
}

BU_WASM_EXPORT("bu_enable_debug_printf")
void bu_enable_debug_printf(uint32_t flag)
{
	enable_debug_printf(flag != 0);
}

BU_WASM_EXPORT("bu_init")
void bu_init()
{
	basisu_encoder_init(false, false);
}

// Memory alloc/free — stubs
BU_WASM_EXPORT("bu_alloc")
uint64_t bu_alloc(uint64_t size)
{
    void* p = malloc((size_t)size);
    return wasm_offset(p);
}

BU_WASM_EXPORT("bu_free")
void bu_free(uint64_t ofs)
{
    free(wasm_ptr(ofs));
}

const uint32_t COMP_PARAMS_MAGIC = 0x43504D50; // "CPMP"

struct comp_params
{
	uint32_t m_magic = COMP_PARAMS_MAGIC;

	comp_params() 
	{
		clear();
	}

	void clear()
	{
		assert(m_magic == COMP_PARAMS_MAGIC);

		m_comp_data.clear();
		m_images.clear();
		m_imagesf.clear();

		m_stats.clear();
	}

	uint8_vec m_comp_data;

	basisu::vector<image> m_images;
	basisu::vector<imagef> m_imagesf;

	image_stats m_stats;
};

BU_WASM_EXPORT("bu_new_comp_params")
uint64_t bu_new_comp_params()
{
	comp_params* p = new comp_params;
	return wasm_offset(p);
}

BU_WASM_EXPORT("bu_delete_comp_params")
wasm_bool_t bu_delete_comp_params(uint64_t params_ofs)
{
	comp_params* p = (comp_params*)wasm_ptr(params_ofs);
	if (!p)
		return false;

	assert(p->m_magic == COMP_PARAMS_MAGIC);
	if (p->m_magic != COMP_PARAMS_MAGIC)
		return false;

	delete p;

	return true;
}

BU_WASM_EXPORT("bu_comp_params_get_comp_data_size")
uint64_t bu_comp_params_get_comp_data_size(uint64_t params_ofs)
{
	comp_params* pParams = (comp_params*)wasm_ptr(params_ofs);
	if (!pParams)
		return 0;

	assert(pParams->m_magic == COMP_PARAMS_MAGIC);
	if (pParams->m_magic != COMP_PARAMS_MAGIC)
		return 0;

	return pParams->m_comp_data.size();
}

BU_WASM_EXPORT("bu_comp_params_get_comp_data_ofs")
uint64_t bu_comp_params_get_comp_data_ofs(uint64_t params_ofs)
{
	comp_params* pParams = (comp_params*)wasm_ptr(params_ofs);
	if (!pParams)
		return 0;

	assert(pParams->m_magic == COMP_PARAMS_MAGIC);
	if (pParams->m_magic != COMP_PARAMS_MAGIC)
		return 0;

	return wasm_offset(pParams->m_comp_data.get_ptr());
}

BU_WASM_EXPORT("bu_comp_params_clear")
wasm_bool_t bu_comp_params_clear(uint64_t params_ofs)
{
	comp_params* pParams = (comp_params*)wasm_ptr(params_ofs);
	if (!pParams)
		return false;

	assert(pParams->m_magic == COMP_PARAMS_MAGIC);
	if (pParams->m_magic != COMP_PARAMS_MAGIC)
		return false;

	pParams->clear();

	return true;
}

// Caller wants to give us a LDR/SDR 32bpp RGBA mipmap level (4 bytes per pixel)
BU_WASM_EXPORT("bu_comp_params_set_image_rgba32")
wasm_bool_t bu_comp_params_set_image_rgba32(
	uint64_t params_ofs,
	uint32_t image_index,
	uint64_t img_data_ofs,
	uint32_t width, uint32_t height,
	uint32_t pitch_in_bytes)
{
	if ((!width) || (!height) || (!pitch_in_bytes))
		return false;

	comp_params* pParams = (comp_params*)wasm_ptr(params_ofs);
	if (!pParams)
		return false;

	assert(pParams->m_magic == COMP_PARAMS_MAGIC);
	if (pParams->m_magic != COMP_PARAMS_MAGIC)
		return false;
		
	const uint8_t* pImage = wasm_ptr(img_data_ofs);
	if (!pImage)
		return false;

	const uint32_t bytes_per_pixel = sizeof(color_rgba);

	if (pitch_in_bytes < width * bytes_per_pixel)
		return false;

	if (image_index >= pParams->m_images.size())
	{
		if (!pParams->m_images.try_resize(image_index + 1))
			return false;
	}

	basisu::image& dst_img = pParams->m_images[image_index];

	dst_img.resize(width, height);
		
	if (pitch_in_bytes == width * bytes_per_pixel)
	{
		memcpy(dst_img.get_ptr(), pImage, pitch_in_bytes * height);
	}
	else
	{
		for (uint32_t y = 0; y < height; y++)
		{
			const uint8_t* pSrc_row = pImage + y * pitch_in_bytes;
			
			uint8_t* pDst_row = (uint8_t *)&dst_img(0, y);

			memcpy(pDst_row, pSrc_row, width * bytes_per_pixel);
		} // y
	}

	return true;
}

// Caller wants to give us a float RGBA mipmap level (4*4=16 bytes per pixel)
BU_WASM_EXPORT("bu_comp_params_set_image_float_rgba")
wasm_bool_t bu_comp_params_set_image_float_rgba(
	uint64_t params_ofs,
	uint32_t image_index,
	uint64_t img_data_ofs,
	uint32_t width, uint32_t height,
	uint32_t pitch_in_bytes)
{
	if ((!width) || (!height) || (!pitch_in_bytes))
		return false;

	comp_params* pParams = (comp_params*)wasm_ptr(params_ofs);
	if (!pParams)
		return false;

	assert(pParams->m_magic == COMP_PARAMS_MAGIC);
	if (pParams->m_magic != COMP_PARAMS_MAGIC)
		return false;

	const uint8_t* pImage = wasm_ptr(img_data_ofs);
	if (!pImage)
		return false;

	const uint32_t bytes_per_pixel = sizeof(float) * 4;

	if (pitch_in_bytes < width * bytes_per_pixel)
		return false;

	if (image_index >= pParams->m_images.size())
	{
		if (!pParams->m_imagesf.try_resize(image_index + 1))
			return false;
	}

	basisu::imagef& dst_img = pParams->m_imagesf[image_index];

	dst_img.resize(width, height);

	if (pitch_in_bytes == width * bytes_per_pixel)
	{
		memcpy((void *)dst_img.get_ptr(), (const void *)pImage, pitch_in_bytes * height);
	}
	else
	{
		for (uint32_t y = 0; y < height; y++)
		{
			const uint8_t* pSrc_row = pImage + y * pitch_in_bytes;

			uint8_t* pDst_row = (uint8_t*)&dst_img(0, y);

			memcpy(pDst_row, pSrc_row, width * bytes_per_pixel);
		} // y
	}

	return true;
}

BU_WASM_EXPORT("bu_compress_texture")
wasm_bool_t bu_compress_texture(
	uint64_t params_ofs,
	uint32_t desired_basis_tex_format, // basis_tex_format
	int quality_level, int effort_level,
	uint64_t flags_and_quality, float low_level_uastc_rdo_or_dct_quality)
{
	//enable_debug_printf((flags_and_quality & cFlagDebug) != 0);

	comp_params* pParams = (comp_params*)wasm_ptr(params_ofs);
	if (!pParams)
		return false;

	assert(pParams->m_magic == COMP_PARAMS_MAGIC);
	if (pParams->m_magic != COMP_PARAMS_MAGIC)
		return false;

	pParams->m_comp_data.clear();

	if (desired_basis_tex_format >= (uint32_t)basist::basis_tex_format::cTotalFormats)
		return false;

	if (!pParams->m_images.size() && !pParams->m_imagesf.size())
		return false;
	if (pParams->m_images.size() && pParams->m_imagesf.size())
		return false;

	size_t comp_size = 0;
	
	void* pComp_data = basis_compress_internal(
		(basist::basis_tex_format)desired_basis_tex_format,
		pParams->m_images.size() ? &pParams->m_images : nullptr,
		pParams->m_imagesf.size() ? &pParams->m_imagesf : nullptr,
		(uint32_t)flags_and_quality,
		low_level_uastc_rdo_or_dct_quality,
		&comp_size,
		&pParams->m_stats,
		quality_level,
		effort_level);

	if (!pComp_data)
		return false;

	if (!pParams->m_comp_data.try_resize(comp_size))
	{
		basis_free_data(pComp_data);
		return false;
	}
	
	memcpy(pParams->m_comp_data.get_ptr(), pComp_data, comp_size);

	basis_free_data(pComp_data);

	return true;
}
