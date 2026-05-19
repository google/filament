// basisu_wasm_transcoder_api.cpp - Transcoding API support for WASM WASI modules and Python native support.
// Also useable by plain C callers.
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "../transcoder/basisu_transcoder.h"
#include "basisu_wasm_transcoder_api.h"

using namespace basisu;
using namespace basist;

static inline uint64_t wasm_offset(void* p)
{
	return (uint64_t)(uintptr_t)p;
}

static inline uint8_t* wasm_ptr(uint64_t offset)
{
	return (uint8_t*)(uintptr_t)offset;
}

// High-level functions

BU_WASM_EXPORT("bt_get_version")
uint32_t bt_get_version()
{
	printf("Hello from basisu_wasm_transcoder_api.cpp version %u\n", BASISD_LIB_VERSION);

	return BASISD_LIB_VERSION;
}

BU_WASM_EXPORT("bt_enable_debug_printf")
void bt_enable_debug_printf(uint32_t flag)
{
	enable_debug_printf(flag != 0);
}

BU_WASM_EXPORT("bt_init")
void bt_init()
{
	basisu_transcoder_init();
}

// Memory alloc/free — stubs
BU_WASM_EXPORT("bt_alloc")
uint64_t bt_alloc(uint64_t size)
{
	void* p = malloc((size_t)size);
	return wasm_offset(p);
}

BU_WASM_EXPORT("bt_free")
void bt_free(uint64_t mem_ofs)
{
	free(wasm_ptr(mem_ofs));
}

// basis_tex_format helpers

BU_WASM_EXPORT("bt_basis_tex_format_is_xuastc_ldr")
wasm_bool_t bt_basis_tex_format_is_xuastc_ldr(uint32_t basis_tex_fmt_u32)
{
	assert(basis_tex_fmt_u32 < (uint32_t)basis_tex_format::cTotalFormats);

	basis_tex_format tex_fmt = static_cast<basis_tex_format>(basis_tex_fmt_u32);
	
	return basis_tex_format_is_xuastc_ldr(tex_fmt);
}

BU_WASM_EXPORT("bt_basis_tex_format_is_astc_ldr")
wasm_bool_t bt_basis_tex_format_is_astc_ldr(uint32_t basis_tex_fmt_u32)
{
	assert(basis_tex_fmt_u32 < (uint32_t)basis_tex_format::cTotalFormats);

	basis_tex_format tex_fmt = static_cast<basis_tex_format>(basis_tex_fmt_u32);

	return basis_tex_format_is_astc_ldr(tex_fmt);
}

BU_WASM_EXPORT("bt_basis_tex_format_get_block_width")
uint32_t bt_basis_tex_format_get_block_width(uint32_t basis_tex_fmt_u32)
{
	assert(basis_tex_fmt_u32 < (uint32_t)basis_tex_format::cTotalFormats);

	basis_tex_format tex_fmt = static_cast<basis_tex_format>(basis_tex_fmt_u32);

	return basis_tex_format_get_block_width(tex_fmt);
}

BU_WASM_EXPORT("bt_basis_tex_format_get_block_height")
uint32_t bt_basis_tex_format_get_block_height(uint32_t basis_tex_fmt_u32)
{
	assert(basis_tex_fmt_u32 < (uint32_t)basis_tex_format::cTotalFormats);

	basis_tex_format tex_fmt = static_cast<basis_tex_format>(basis_tex_fmt_u32);

	return basis_tex_format_get_block_height(tex_fmt);
}

BU_WASM_EXPORT("bt_basis_tex_format_is_hdr")
wasm_bool_t bt_basis_tex_format_is_hdr(uint32_t basis_tex_fmt_u32)
{
	assert(basis_tex_fmt_u32 < (uint32_t)basis_tex_format::cTotalFormats);

	basis_tex_format tex_fmt = static_cast<basis_tex_format>(basis_tex_fmt_u32);

	return basis_tex_format_is_hdr(tex_fmt);
}

BU_WASM_EXPORT("bt_basis_tex_format_is_ldr")
wasm_bool_t bt_basis_tex_format_is_ldr(uint32_t basis_tex_fmt_u32)
{
	assert(basis_tex_fmt_u32 < (uint32_t)basis_tex_format::cTotalFormats);

	basis_tex_format tex_fmt = static_cast<basis_tex_format>(basis_tex_fmt_u32);

	return basis_tex_format_is_ldr(tex_fmt);
}

// transcoder_texture_format helpers

BU_WASM_EXPORT("bt_basis_get_bytes_per_block_or_pixel")
uint32_t bt_basis_get_bytes_per_block_or_pixel(uint32_t transcoder_texture_format_u32)
{
	assert(transcoder_texture_format_u32 < (uint32_t)transcoder_texture_format::cTFTotalTextureFormats);

	transcoder_texture_format fmt = static_cast<transcoder_texture_format>(transcoder_texture_format_u32);

	return basis_get_bytes_per_block_or_pixel(fmt);
}

BU_WASM_EXPORT("bt_basis_transcoder_format_has_alpha")
wasm_bool_t bt_basis_transcoder_format_has_alpha(uint32_t transcoder_texture_format_u32)
{
	assert(transcoder_texture_format_u32 < (uint32_t)transcoder_texture_format::cTFTotalTextureFormats);
	
	transcoder_texture_format fmt = static_cast<transcoder_texture_format>(transcoder_texture_format_u32);

	return basis_transcoder_format_has_alpha(fmt);
}

BU_WASM_EXPORT("bt_basis_transcoder_format_is_hdr")
wasm_bool_t bt_basis_transcoder_format_is_hdr(uint32_t transcoder_texture_format_u32)
{
	assert(transcoder_texture_format_u32 < (uint32_t)transcoder_texture_format::cTFTotalTextureFormats);

	transcoder_texture_format fmt = static_cast<transcoder_texture_format>(transcoder_texture_format_u32);

	return basis_transcoder_format_is_hdr(fmt);
}

BU_WASM_EXPORT("bt_basis_transcoder_format_is_ldr")
wasm_bool_t bt_basis_transcoder_format_is_ldr(uint32_t transcoder_texture_format_u32)
{
	assert(transcoder_texture_format_u32 < (uint32_t)transcoder_texture_format::cTFTotalTextureFormats);

	transcoder_texture_format fmt = static_cast<transcoder_texture_format>(transcoder_texture_format_u32);

	return basis_transcoder_format_is_ldr(fmt);
}

BU_WASM_EXPORT("bt_basis_transcoder_texture_format_is_astc")
wasm_bool_t bt_basis_transcoder_texture_format_is_astc(uint32_t transcoder_texture_format_u32)
{
	assert(transcoder_texture_format_u32 < (uint32_t)transcoder_texture_format::cTFTotalTextureFormats);

	transcoder_texture_format fmt = static_cast<transcoder_texture_format>(transcoder_texture_format_u32);

	return basis_is_transcoder_texture_format_astc(fmt);
}

BU_WASM_EXPORT("bt_basis_transcoder_format_is_uncompressed")
wasm_bool_t bt_basis_transcoder_format_is_uncompressed(uint32_t transcoder_texture_format_u32)
{
	assert(transcoder_texture_format_u32 < (uint32_t)transcoder_texture_format::cTFTotalTextureFormats);

	transcoder_texture_format fmt = static_cast<transcoder_texture_format>(transcoder_texture_format_u32);

	return basis_transcoder_format_is_uncompressed(fmt);
}

BU_WASM_EXPORT("bt_basis_get_uncompressed_bytes_per_pixel")
uint32_t bt_basis_get_uncompressed_bytes_per_pixel(uint32_t transcoder_texture_format_u32)
{
	assert(transcoder_texture_format_u32 < (uint32_t)transcoder_texture_format::cTFTotalTextureFormats);

	transcoder_texture_format fmt = static_cast<transcoder_texture_format>(transcoder_texture_format_u32);

	return basis_get_uncompressed_bytes_per_pixel(fmt);
}

BU_WASM_EXPORT("bt_basis_get_block_width")
uint32_t bt_basis_get_block_width(uint32_t transcoder_texture_format_u32)
{
	assert(transcoder_texture_format_u32 < (uint32_t)transcoder_texture_format::cTFTotalTextureFormats);

	transcoder_texture_format fmt = static_cast<transcoder_texture_format>(transcoder_texture_format_u32);

	return basis_get_block_width(fmt);
}

BU_WASM_EXPORT("bt_basis_get_block_height")
uint32_t bt_basis_get_block_height(uint32_t transcoder_texture_format_u32)
{
	assert(transcoder_texture_format_u32 < (uint32_t)transcoder_texture_format::cTFTotalTextureFormats);

	transcoder_texture_format fmt = static_cast<transcoder_texture_format>(transcoder_texture_format_u32);

	return basis_get_block_height(fmt);
}

BU_WASM_EXPORT("bt_basis_get_transcoder_texture_format_from_basis_tex_format")
uint32_t bt_basis_get_transcoder_texture_format_from_basis_tex_format(uint32_t basis_tex_format_u32)
{
	assert(basis_tex_format_u32 < (uint32_t)basis_tex_format::cTotalFormats);
	
	basis_tex_format fmt = static_cast<basis_tex_format>(basis_tex_format_u32);

	return (uint32_t)basis_get_transcoder_texture_format_from_xuastc_or_astc_ldr_basis_tex_format(fmt);
}

BU_WASM_EXPORT("bt_basis_is_format_supported")
wasm_bool_t bt_basis_is_format_supported(uint32_t transcoder_texture_format_u32, uint32_t basis_tex_format_u32)
{
	assert(transcoder_texture_format_u32 < (uint32_t)transcoder_texture_format::cTFTotalTextureFormats);
	assert(basis_tex_format_u32 < (uint32_t)basis_tex_format::cTotalFormats);

	transcoder_texture_format transcoder_tex_fmt = static_cast<transcoder_texture_format>(transcoder_texture_format_u32);
	basis_tex_format basis_tex_fmt = static_cast<basis_tex_format>(basis_tex_format_u32);

	return basis_is_format_supported(transcoder_tex_fmt, basis_tex_fmt);
}

BU_WASM_EXPORT("bt_basis_compute_transcoded_image_size_in_bytes")
uint32_t bt_basis_compute_transcoded_image_size_in_bytes(uint32_t transcoder_texture_format_u32, uint32_t orig_width, uint32_t orig_height)
{
	assert(transcoder_texture_format_u32 < (uint32_t)transcoder_texture_format::cTFTotalTextureFormats);

	transcoder_texture_format transcoder_tex_fmt = static_cast<transcoder_texture_format>(transcoder_texture_format_u32);

	return basis_compute_transcoded_image_size_in_bytes(transcoder_tex_fmt, orig_width, orig_height);
}

// KTX2 inspection and transcoding helpers

const uint32_t KTX2_HANDLE_MAGIC = 0xAB21EF20;

struct ktx2_handle_t
{
	uint32_t m_magic = KTX2_HANDLE_MAGIC;
	ktx2_transcoder m_transcoder;
};

BU_WASM_EXPORT("bt_ktx2_open")
uint64_t bt_ktx2_open(uint64_t data_mem_ofs, uint32_t data_len)
{
	if (!data_mem_ofs || (data_len < 4))
		return 0;

	ktx2_handle_t* pHandle = new ktx2_handle_t();
	
	if (!pHandle->m_transcoder.init(wasm_ptr(data_mem_ofs), data_len))
	{
		delete pHandle;
		return 0;
	}

	return wasm_offset(pHandle);
}

BU_WASM_EXPORT("bt_ktx2_close")
void bt_ktx2_close(uint64_t handle)
{
	if (!handle)
		return;
		
	ktx2_handle_t* pHandle = reinterpret_cast<ktx2_handle_t *>(wasm_ptr(handle));

	assert(pHandle->m_magic == KTX2_HANDLE_MAGIC);
	if (pHandle->m_magic != KTX2_HANDLE_MAGIC)
		return;

	delete pHandle;
}

BU_WASM_EXPORT("bt_ktx2_get_width")
uint32_t bt_ktx2_get_width(uint64_t handle)
{
	if (!handle)
	{
		assert(0);
		return 0;
	}

	ktx2_handle_t* pHandle = reinterpret_cast<ktx2_handle_t*>(wasm_ptr(handle));

	assert(pHandle->m_magic == KTX2_HANDLE_MAGIC);
	if (pHandle->m_magic != KTX2_HANDLE_MAGIC)
		return 0;

	return pHandle->m_transcoder.get_width();
}

BU_WASM_EXPORT("bt_ktx2_get_height")
uint32_t bt_ktx2_get_height(uint64_t handle)
{
	if (!handle)
	{
		assert(0);
		return 0;
	}

	ktx2_handle_t* pHandle = reinterpret_cast<ktx2_handle_t*>(wasm_ptr(handle));

	assert(pHandle->m_magic == KTX2_HANDLE_MAGIC);
	if (pHandle->m_magic != KTX2_HANDLE_MAGIC)
		return 0;

	return pHandle->m_transcoder.get_height();
}

BU_WASM_EXPORT("bt_ktx2_get_levels")
uint32_t bt_ktx2_get_levels(uint64_t handle)
{
	if (!handle)
	{
		assert(0);
		return 0;
	}

	ktx2_handle_t* pHandle = reinterpret_cast<ktx2_handle_t*>(wasm_ptr(handle));

	assert(pHandle->m_magic == KTX2_HANDLE_MAGIC);
	if (pHandle->m_magic != KTX2_HANDLE_MAGIC)
		return 0;

	return pHandle->m_transcoder.get_levels();
}

BU_WASM_EXPORT("bt_ktx2_get_faces")
uint32_t bt_ktx2_get_faces(uint64_t handle)
{
	if (!handle)
	{
		assert(0);
		return 0;
	}

	ktx2_handle_t* pHandle = reinterpret_cast<ktx2_handle_t*>(wasm_ptr(handle));

	assert(pHandle->m_magic == KTX2_HANDLE_MAGIC);
	if (pHandle->m_magic != KTX2_HANDLE_MAGIC)
		return 0;

	return pHandle->m_transcoder.get_faces();
}

BU_WASM_EXPORT("bt_ktx2_get_layers")
uint32_t bt_ktx2_get_layers(uint64_t handle)
{
	if (!handle)
	{
		assert(0);
		return 0;
	}

	ktx2_handle_t* pHandle = reinterpret_cast<ktx2_handle_t*>(wasm_ptr(handle));

	assert(pHandle->m_magic == KTX2_HANDLE_MAGIC);
	if (pHandle->m_magic != KTX2_HANDLE_MAGIC)
		return 0;

	return pHandle->m_transcoder.get_layers();
}

BU_WASM_EXPORT("bt_ktx2_get_basis_tex_format")
uint32_t bt_ktx2_get_basis_tex_format(uint64_t handle)
{
	if (!handle)
	{
		assert(0);
		return 0;
	}

	ktx2_handle_t* pHandle = reinterpret_cast<ktx2_handle_t*>(wasm_ptr(handle));

	assert(pHandle->m_magic == KTX2_HANDLE_MAGIC);
	if (pHandle->m_magic != KTX2_HANDLE_MAGIC)
		return 0;

	return (uint32_t)pHandle->m_transcoder.get_basis_tex_format();
}

BU_WASM_EXPORT("bt_ktx2_is_etc1s")
wasm_bool_t bt_ktx2_is_etc1s(uint64_t handle)
{
	if (!handle)
	{
		assert(0);
		return false;
	}

	ktx2_handle_t* pHandle = reinterpret_cast<ktx2_handle_t*>(wasm_ptr(handle));

	assert(pHandle->m_magic == KTX2_HANDLE_MAGIC);
	if (pHandle->m_magic != KTX2_HANDLE_MAGIC)
		return false;

	return pHandle->m_transcoder.is_etc1s();
}

BU_WASM_EXPORT("bt_ktx2_is_uastc_ldr_4x4")
wasm_bool_t bt_ktx2_is_uastc_ldr_4x4(uint64_t handle)
{
	if (!handle)
	{
		assert(0);
		return false;
	}

	ktx2_handle_t* pHandle = reinterpret_cast<ktx2_handle_t*>(wasm_ptr(handle));

	assert(pHandle->m_magic == KTX2_HANDLE_MAGIC);
	if (pHandle->m_magic != KTX2_HANDLE_MAGIC)
		return false;

	return pHandle->m_transcoder.is_uastc();
}

BU_WASM_EXPORT("bt_ktx2_is_hdr")
wasm_bool_t bt_ktx2_is_hdr(uint64_t handle)
{
	if (!handle)
	{
		assert(0);
		return false;
	}

	ktx2_handle_t* pHandle = reinterpret_cast<ktx2_handle_t*>(wasm_ptr(handle));

	assert(pHandle->m_magic == KTX2_HANDLE_MAGIC);
	if (pHandle->m_magic != KTX2_HANDLE_MAGIC)
		return false;

	return pHandle->m_transcoder.is_hdr();
}

BU_WASM_EXPORT("bt_ktx2_is_hdr_4x4")
wasm_bool_t bt_ktx2_is_hdr_4x4(uint64_t handle)
{
	if (!handle)
	{
		assert(0);
		return false;
	}

	ktx2_handle_t* pHandle = reinterpret_cast<ktx2_handle_t*>(wasm_ptr(handle));

	assert(pHandle->m_magic == KTX2_HANDLE_MAGIC);
	if (pHandle->m_magic != KTX2_HANDLE_MAGIC)
		return false;

	return pHandle->m_transcoder.is_hdr_4x4();
}

BU_WASM_EXPORT("bt_ktx2_is_hdr_6x6")
wasm_bool_t bt_ktx2_is_hdr_6x6(uint64_t handle)
{
	if (!handle)
	{
		assert(0);
		return false;
	}

	ktx2_handle_t* pHandle = reinterpret_cast<ktx2_handle_t*>(wasm_ptr(handle));

	assert(pHandle->m_magic == KTX2_HANDLE_MAGIC);
	if (pHandle->m_magic != KTX2_HANDLE_MAGIC)
		return false;

	return pHandle->m_transcoder.is_hdr_6x6();
}

BU_WASM_EXPORT("bt_ktx2_is_ldr")
wasm_bool_t bt_ktx2_is_ldr(uint64_t handle)
{
	if (!handle)
	{
		assert(0);
		return false;
	}

	ktx2_handle_t* pHandle = reinterpret_cast<ktx2_handle_t*>(wasm_ptr(handle));

	assert(pHandle->m_magic == KTX2_HANDLE_MAGIC);
	if (pHandle->m_magic != KTX2_HANDLE_MAGIC)
		return false;

	return pHandle->m_transcoder.is_ldr();
}

BU_WASM_EXPORT("bt_ktx2_is_astc_ldr")
wasm_bool_t bt_ktx2_is_astc_ldr(uint64_t handle)
{
	if (!handle)
	{
		assert(0);
		return false;
	}

	ktx2_handle_t* pHandle = reinterpret_cast<ktx2_handle_t*>(wasm_ptr(handle));

	assert(pHandle->m_magic == KTX2_HANDLE_MAGIC);
	if (pHandle->m_magic != KTX2_HANDLE_MAGIC)
		return false;

	return pHandle->m_transcoder.is_astc_ldr();
}

BU_WASM_EXPORT("bt_ktx2_is_xuastc_ldr")
wasm_bool_t bt_ktx2_is_xuastc_ldr(uint64_t handle)
{
	if (!handle)
	{
		assert(0);
		return false;
	}

	ktx2_handle_t* pHandle = reinterpret_cast<ktx2_handle_t*>(wasm_ptr(handle));

	assert(pHandle->m_magic == KTX2_HANDLE_MAGIC);
	if (pHandle->m_magic != KTX2_HANDLE_MAGIC)
		return false;

	return pHandle->m_transcoder.is_xuastc_ldr();
}

BU_WASM_EXPORT("bt_ktx2_get_block_width")
uint32_t bt_ktx2_get_block_width(uint64_t handle)
{
	if (!handle)
	{
		assert(0);
		return 0;
	}

	ktx2_handle_t* pHandle = reinterpret_cast<ktx2_handle_t*>(wasm_ptr(handle));

	assert(pHandle->m_magic == KTX2_HANDLE_MAGIC);
	if (pHandle->m_magic != KTX2_HANDLE_MAGIC)
		return 0;

	return pHandle->m_transcoder.get_block_width();
}

BU_WASM_EXPORT("bt_ktx2_get_block_height")
uint32_t bt_ktx2_get_block_height(uint64_t handle)
{
	if (!handle)
	{
		assert(0);
		return 0;
	}

	ktx2_handle_t* pHandle = reinterpret_cast<ktx2_handle_t*>(wasm_ptr(handle));

	assert(pHandle->m_magic == KTX2_HANDLE_MAGIC);
	if (pHandle->m_magic != KTX2_HANDLE_MAGIC)
		return 0;

	return pHandle->m_transcoder.get_block_height();
}

BU_WASM_EXPORT("bt_ktx2_has_alpha")
wasm_bool_t bt_ktx2_has_alpha(uint64_t handle)
{
	if (!handle)
	{
		assert(0);
		return false;
	}

	ktx2_handle_t* pHandle = reinterpret_cast<ktx2_handle_t*>(wasm_ptr(handle));

	assert(pHandle->m_magic == KTX2_HANDLE_MAGIC);
	if (pHandle->m_magic != KTX2_HANDLE_MAGIC)
		return false;

	return pHandle->m_transcoder.get_has_alpha();
}

BU_WASM_EXPORT("bt_ktx2_get_dfd_color_model")
uint32_t bt_ktx2_get_dfd_color_model(uint64_t handle)
{
	if (!handle)
	{
		assert(0);
		return 0;
	}

	ktx2_handle_t* pHandle = reinterpret_cast<ktx2_handle_t*>(wasm_ptr(handle));

	assert(pHandle->m_magic == KTX2_HANDLE_MAGIC);
	if (pHandle->m_magic != KTX2_HANDLE_MAGIC)
		return 0;

	return pHandle->m_transcoder.get_dfd_color_model();
}

BU_WASM_EXPORT("bt_ktx2_get_dfd_color_primaries")
uint32_t bt_ktx2_get_dfd_color_primaries(uint64_t handle)
{
	if (!handle)
	{
		assert(0);
		return 0;
	}

	ktx2_handle_t* pHandle = reinterpret_cast<ktx2_handle_t*>(wasm_ptr(handle));

	assert(pHandle->m_magic == KTX2_HANDLE_MAGIC);
	if (pHandle->m_magic != KTX2_HANDLE_MAGIC)
		return 0;

	return pHandle->m_transcoder.get_dfd_color_primaries();
}

BU_WASM_EXPORT("bt_ktx2_get_dfd_transfer_func")
uint32_t bt_ktx2_get_dfd_transfer_func(uint64_t handle)
{
	if (!handle)
	{
		assert(0);
		return 0;
	}

	ktx2_handle_t* pHandle = reinterpret_cast<ktx2_handle_t*>(wasm_ptr(handle));

	assert(pHandle->m_magic == KTX2_HANDLE_MAGIC);
	if (pHandle->m_magic != KTX2_HANDLE_MAGIC)
		return 0;

	return pHandle->m_transcoder.get_dfd_transfer_func();
}

BU_WASM_EXPORT("bt_ktx2_is_srgb")
wasm_bool_t bt_ktx2_is_srgb(uint64_t handle)
{
	if (!handle)
	{
		assert(0);
		return false;
	}

	ktx2_handle_t* pHandle = reinterpret_cast<ktx2_handle_t*>(wasm_ptr(handle));

	assert(pHandle->m_magic == KTX2_HANDLE_MAGIC);
	if (pHandle->m_magic != KTX2_HANDLE_MAGIC)
		return false;

	return pHandle->m_transcoder.is_srgb();
}

BU_WASM_EXPORT("bt_ktx2_get_dfd_flags")
uint32_t bt_ktx2_get_dfd_flags(uint64_t handle)
{
	if (!handle)
	{
		assert(0);
		return 0;
	}

	ktx2_handle_t* pHandle = reinterpret_cast<ktx2_handle_t*>(wasm_ptr(handle));

	assert(pHandle->m_magic == KTX2_HANDLE_MAGIC);
	if (pHandle->m_magic != KTX2_HANDLE_MAGIC)
		return 0;

	return pHandle->m_transcoder.get_dfd_flags();
}

BU_WASM_EXPORT("bt_ktx2_get_dfd_total_samples")
uint32_t bt_ktx2_get_dfd_total_samples(uint64_t handle)
{
	if (!handle)
	{
		assert(0);
		return 0;
	}

	ktx2_handle_t* pHandle = reinterpret_cast<ktx2_handle_t*>(wasm_ptr(handle));

	assert(pHandle->m_magic == KTX2_HANDLE_MAGIC);
	if (pHandle->m_magic != KTX2_HANDLE_MAGIC)
		return 0;

	return pHandle->m_transcoder.get_dfd_total_samples();
}

BU_WASM_EXPORT("bt_ktx2_get_dfd_channel_id0")
uint32_t bt_ktx2_get_dfd_channel_id0(uint64_t handle)
{
	if (!handle)
	{
		assert(0);
		return 0;
	}

	ktx2_handle_t* pHandle = reinterpret_cast<ktx2_handle_t*>(wasm_ptr(handle));

	assert(pHandle->m_magic == KTX2_HANDLE_MAGIC);
	if (pHandle->m_magic != KTX2_HANDLE_MAGIC)
		return 0;

	return pHandle->m_transcoder.get_dfd_channel_id0();
}

BU_WASM_EXPORT("bt_ktx2_get_dfd_channel_id1")
uint32_t bt_ktx2_get_dfd_channel_id1(uint64_t handle)
{
	if (!handle)
	{
		assert(0);
		return 0;
	}

	ktx2_handle_t* pHandle = reinterpret_cast<ktx2_handle_t*>(wasm_ptr(handle));

	assert(pHandle->m_magic == KTX2_HANDLE_MAGIC);
	if (pHandle->m_magic != KTX2_HANDLE_MAGIC)
		return 0;

	return pHandle->m_transcoder.get_dfd_channel_id1();
}

BU_WASM_EXPORT("bt_ktx2_is_video")
wasm_bool_t bt_ktx2_is_video(uint64_t handle)
{
	if (!handle)
	{
		assert(0);
		return false;
	}

	ktx2_handle_t* pHandle = reinterpret_cast<ktx2_handle_t*>(wasm_ptr(handle));

	assert(pHandle->m_magic == KTX2_HANDLE_MAGIC);
	if (pHandle->m_magic != KTX2_HANDLE_MAGIC)
		return false;

	return pHandle->m_transcoder.is_video();
}

BU_WASM_EXPORT("bt_ktx2_get_ldr_hdr_upconversion_nit_multiplier")
float bt_ktx2_get_ldr_hdr_upconversion_nit_multiplier(uint64_t handle)
{
	if (!handle)
	{
		assert(0);
		return 0.0f;
	}

	ktx2_handle_t* pHandle = reinterpret_cast<ktx2_handle_t*>(wasm_ptr(handle));

	assert(pHandle->m_magic == KTX2_HANDLE_MAGIC);
	if (pHandle->m_magic != KTX2_HANDLE_MAGIC)
		return 0.0f;

	return pHandle->m_transcoder.get_ldr_hdr_upconversion_nit_multiplier();
}

BU_WASM_EXPORT("bt_ktx2_get_level_orig_width")
uint32_t bt_ktx2_get_level_orig_width(uint64_t handle, uint32_t level_index, uint32_t layer_index, uint32_t face_index)
{
	if (!handle)
	{
		assert(0);
		return 0;
	}

	ktx2_handle_t* pHandle = reinterpret_cast<ktx2_handle_t*>(wasm_ptr(handle));

	assert(pHandle->m_magic == KTX2_HANDLE_MAGIC);
	if (pHandle->m_magic != KTX2_HANDLE_MAGIC)
		return 0;

	// FIXME slow - most info is thrown away.
	ktx2_image_level_info level_info;
	if (!pHandle->m_transcoder.get_image_level_info(level_info, level_index, layer_index, face_index))
		return 0;

	return level_info.m_orig_width;
}

BU_WASM_EXPORT("bt_ktx2_get_level_orig_height")
uint32_t bt_ktx2_get_level_orig_height(uint64_t handle, uint32_t level_index, uint32_t layer_index, uint32_t face_index)
{
	if (!handle)
	{
		assert(0);
		return 0;
	}

	ktx2_handle_t* pHandle = reinterpret_cast<ktx2_handle_t*>(wasm_ptr(handle));

	assert(pHandle->m_magic == KTX2_HANDLE_MAGIC);
	if (pHandle->m_magic != KTX2_HANDLE_MAGIC)
		return 0;

	// FIXME slow - most info is thrown away.
	ktx2_image_level_info level_info;
	if (!pHandle->m_transcoder.get_image_level_info(level_info, level_index, layer_index, face_index))
		return 0;

	return level_info.m_orig_height;
}

BU_WASM_EXPORT("bt_ktx2_get_level_actual_width")
uint32_t bt_ktx2_get_level_actual_width(uint64_t handle, uint32_t level_index, uint32_t layer_index, uint32_t face_index)
{
	if (!handle)
	{
		assert(0);
		return 0;
	}

	ktx2_handle_t* pHandle = reinterpret_cast<ktx2_handle_t*>(wasm_ptr(handle));

	assert(pHandle->m_magic == KTX2_HANDLE_MAGIC);
	if (pHandle->m_magic != KTX2_HANDLE_MAGIC)
		return 0;

	// FIXME slow - most info is thrown away.
	ktx2_image_level_info level_info;
	if (!pHandle->m_transcoder.get_image_level_info(level_info, level_index, layer_index, face_index))
		return 0;

	return level_info.m_width;
}

BU_WASM_EXPORT("bt_ktx2_get_level_actual_height")
uint32_t bt_ktx2_get_level_actual_height(uint64_t handle, uint32_t level_index, uint32_t layer_index, uint32_t face_index)
{
	if (!handle)
	{
		assert(0);
		return 0;
	}

	ktx2_handle_t* pHandle = reinterpret_cast<ktx2_handle_t*>(wasm_ptr(handle));

	assert(pHandle->m_magic == KTX2_HANDLE_MAGIC);
	if (pHandle->m_magic != KTX2_HANDLE_MAGIC)
		return 0;

	// FIXME slow - most info is thrown away.
	ktx2_image_level_info level_info;
	if (!pHandle->m_transcoder.get_image_level_info(level_info, level_index, layer_index, face_index))
		return 0;

	return level_info.m_height;
}

BU_WASM_EXPORT("bt_ktx2_get_level_num_blocks_x")
uint32_t bt_ktx2_get_level_num_blocks_x(uint64_t handle, uint32_t level_index, uint32_t layer_index, uint32_t face_index)
{
	if (!handle)
	{
		assert(0);
		return 0;
	}

	ktx2_handle_t* pHandle = reinterpret_cast<ktx2_handle_t*>(wasm_ptr(handle));

	assert(pHandle->m_magic == KTX2_HANDLE_MAGIC);
	if (pHandle->m_magic != KTX2_HANDLE_MAGIC)
		return 0;

	// FIXME slow - most info is thrown away.
	ktx2_image_level_info level_info;
	if (!pHandle->m_transcoder.get_image_level_info(level_info, level_index, layer_index, face_index))
		return 0;

	return level_info.m_num_blocks_x;
}

BU_WASM_EXPORT("bt_ktx2_get_level_num_blocks_y")
uint32_t bt_ktx2_get_level_num_blocks_y(uint64_t handle, uint32_t level_index, uint32_t layer_index, uint32_t face_index)
{
	if (!handle)
	{
		assert(0);
		return 0;
	}

	ktx2_handle_t* pHandle = reinterpret_cast<ktx2_handle_t*>(wasm_ptr(handle));

	assert(pHandle->m_magic == KTX2_HANDLE_MAGIC);
	if (pHandle->m_magic != KTX2_HANDLE_MAGIC)
		return 0;

	// FIXME slow - most info is thrown away.
	ktx2_image_level_info level_info;
	if (!pHandle->m_transcoder.get_image_level_info(level_info, level_index, layer_index, face_index))
		return 0;

	return level_info.m_num_blocks_y;
}

BU_WASM_EXPORT("bt_ktx2_get_level_total_blocks")
uint32_t bt_ktx2_get_level_total_blocks(uint64_t handle, uint32_t level_index, uint32_t layer_index, uint32_t face_index)
{
	if (!handle)
	{
		assert(0);
		return 0;
	}

	ktx2_handle_t* pHandle = reinterpret_cast<ktx2_handle_t*>(wasm_ptr(handle));

	assert(pHandle->m_magic == KTX2_HANDLE_MAGIC);
	if (pHandle->m_magic != KTX2_HANDLE_MAGIC)
		return 0;

	// FIXME slow - most info is thrown away.
	ktx2_image_level_info level_info;
	if (!pHandle->m_transcoder.get_image_level_info(level_info, level_index, layer_index, face_index))
		return 0;

	return level_info.m_total_blocks;
}

BU_WASM_EXPORT("bt_ktx2_get_level_alpha_flag")
wasm_bool_t bt_ktx2_get_level_alpha_flag(uint64_t handle, uint32_t level_index, uint32_t layer_index, uint32_t face_index)
{
	if (!handle)
	{
		assert(0);
		return false;
	}

	ktx2_handle_t* pHandle = reinterpret_cast<ktx2_handle_t*>(wasm_ptr(handle));

	assert(pHandle->m_magic == KTX2_HANDLE_MAGIC);
	if (pHandle->m_magic != KTX2_HANDLE_MAGIC)
		return false;

	// FIXME slow - most info is thrown away.
	ktx2_image_level_info level_info;
	if (!pHandle->m_transcoder.get_image_level_info(level_info, level_index, layer_index, face_index))
		return false;

	return level_info.m_alpha_flag;
}

BU_WASM_EXPORT("bt_ktx2_get_level_iframe_flag")
wasm_bool_t bt_ktx2_get_level_iframe_flag(uint64_t handle, uint32_t level_index, uint32_t layer_index, uint32_t face_index)
{
	if (!handle)
	{
		assert(0);
		return false;
	}

	ktx2_handle_t* pHandle = reinterpret_cast<ktx2_handle_t*>(wasm_ptr(handle));

	assert(pHandle->m_magic == KTX2_HANDLE_MAGIC);
	if (pHandle->m_magic != KTX2_HANDLE_MAGIC)
		return false;

	// FIXME slow - most info is thrown away.
	ktx2_image_level_info level_info;
	if (!pHandle->m_transcoder.get_image_level_info(level_info, level_index, layer_index, face_index))
		return false;

	return level_info.m_iframe_flag;
}

BU_WASM_EXPORT("bt_ktx2_start_transcoding")
wasm_bool_t bt_ktx2_start_transcoding(uint64_t handle)
{
	if (!handle)
	{
		assert(0);
		return false;
	}

	ktx2_handle_t* pHandle = reinterpret_cast<ktx2_handle_t*>(wasm_ptr(handle));

	assert(pHandle->m_magic == KTX2_HANDLE_MAGIC);
	if (pHandle->m_magic != KTX2_HANDLE_MAGIC)
		return false;

	return pHandle->m_transcoder.start_transcoding();
}

const uint32_t KTX2_TRANSCODE_STATE_MAGIC = 0x2B21CF21;

struct ktx2_transcode_state_t
{
	uint32_t m_magic = KTX2_TRANSCODE_STATE_MAGIC;

	ktx2_transcoder_state m_state;
};

BU_WASM_EXPORT("bt_ktx2_create_transcode_state")
uint64_t bt_ktx2_create_transcode_state()
{
	return wasm_offset(new ktx2_transcode_state_t());
}

BU_WASM_EXPORT("bt_ktx2_destroy_transcode_state")
void bt_ktx2_destroy_transcode_state(uint64_t handle)
{
	if (!handle)
		return;

	ktx2_transcode_state_t* pState = reinterpret_cast<ktx2_transcode_state_t*>(wasm_ptr(handle));

	assert(pState->m_magic == KTX2_TRANSCODE_STATE_MAGIC);
	if (pState->m_magic != KTX2_TRANSCODE_STATE_MAGIC)
		return;

	delete pState;
}

BU_WASM_EXPORT("bt_ktx2_transcode_image_level")
wasm_bool_t bt_ktx2_transcode_image_level(
	uint64_t ktx2_handle,
	uint32_t level_index, uint32_t layer_index, uint32_t face_index,
	uint64_t output_block_mem_ofs, uint32_t output_blocks_buf_size_in_blocks_or_pixels,
	uint32_t transcoder_texture_format_u32,
	uint32_t decode_flags,
	uint32_t output_row_pitch_in_blocks_or_pixels,
	uint32_t output_rows_in_pixels,
	int channel0, int channel1,
	uint64_t state_handle)
{
	if ((!ktx2_handle) || (!output_block_mem_ofs))
	{
		assert(0);
		return false;
	}

	ktx2_handle_t* pHandle = reinterpret_cast<ktx2_handle_t*>(wasm_ptr(ktx2_handle));

	assert(pHandle->m_magic == KTX2_HANDLE_MAGIC);
	if (pHandle->m_magic != KTX2_HANDLE_MAGIC)
		return false;

	assert(transcoder_texture_format_u32 < (uint32_t)transcoder_texture_format::cTFTotalTextureFormats);
	transcoder_texture_format tex_fmt = static_cast<transcoder_texture_format>(transcoder_texture_format_u32);

	ktx2_transcode_state_t* pTranscode_state = nullptr;

	if (state_handle)
	{
		pTranscode_state = reinterpret_cast<ktx2_transcode_state_t *>(wasm_ptr(state_handle));

		assert(pTranscode_state->m_magic == KTX2_TRANSCODE_STATE_MAGIC);
		if (pTranscode_state->m_magic != KTX2_TRANSCODE_STATE_MAGIC)
			return false;
	}

	return pHandle->m_transcoder.transcode_image_level(
		level_index, layer_index, face_index,
			wasm_ptr(output_block_mem_ofs), output_blocks_buf_size_in_blocks_or_pixels,
			tex_fmt,
			decode_flags, output_row_pitch_in_blocks_or_pixels, output_rows_in_pixels, channel0, channel1,
			pTranscode_state ? &pTranscode_state->m_state : nullptr);
}
