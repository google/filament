/*
 * Copyright © 2014 Advanced Micro Devices, Inc.
 * All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE COPYRIGHT HOLDER(S) OR AUTHOR(S) BE LIABLE FOR ANY CLAIM, DAMAGES OR
 * OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 *
 */

#include <errno.h>
#include <string.h>

#include "amdgpu.h"
#include "amdgpu_drm.h"
#include "amdgpu_internal.h"
#include "xf86drm.h"

drm_public int amdgpu_query_info(amdgpu_device_handle dev, unsigned info_id,
				 unsigned size, void *value)
{
	struct drm_amdgpu_info request;

	memset(&request, 0, sizeof(request));
	request.return_pointer = (uintptr_t)value;
	request.return_size = size;
	request.query = info_id;

	return drmCommandWrite(dev->fd, DRM_AMDGPU_INFO, &request,
			       sizeof(struct drm_amdgpu_info));
}

drm_public int amdgpu_query_crtc_from_id(amdgpu_device_handle dev, unsigned id,
					 int32_t *result)
{
	struct drm_amdgpu_info request;

	memset(&request, 0, sizeof(request));
	request.return_pointer = (uintptr_t)result;
	request.return_size = sizeof(*result);
	request.query = AMDGPU_INFO_CRTC_FROM_ID;
	request.mode_crtc.id = id;

	return drmCommandWrite(dev->fd, DRM_AMDGPU_INFO, &request,
			       sizeof(struct drm_amdgpu_info));
}

drm_public int amdgpu_read_mm_registers(amdgpu_device_handle dev,
		unsigned dword_offset, unsigned count, uint32_t instance,
		uint32_t flags, uint32_t *values)
{
	struct drm_amdgpu_info request;

	memset(&request, 0, sizeof(request));
	request.return_pointer = (uintptr_t)values;
	request.return_size = count * sizeof(uint32_t);
	request.query = AMDGPU_INFO_READ_MMR_REG;
	request.read_mmr_reg.dword_offset = dword_offset;
	request.read_mmr_reg.count = count;
	request.read_mmr_reg.instance = instance;
	request.read_mmr_reg.flags = flags;

	return drmCommandWrite(dev->fd, DRM_AMDGPU_INFO, &request,
			       sizeof(struct drm_amdgpu_info));
}

drm_public int amdgpu_query_hw_ip_count(amdgpu_device_handle dev,
					unsigned type,
					uint32_t *count)
{
	struct drm_amdgpu_info request;

	memset(&request, 0, sizeof(request));
	request.return_pointer = (uintptr_t)count;
	request.return_size = sizeof(*count);
	request.query = AMDGPU_INFO_HW_IP_COUNT;
	request.query_hw_ip.type = type;

	return drmCommandWrite(dev->fd, DRM_AMDGPU_INFO, &request,
			       sizeof(struct drm_amdgpu_info));
}

drm_public int amdgpu_query_hw_ip_info(amdgpu_device_handle dev, unsigned type,
				       unsigned ip_instance,
				       struct drm_amdgpu_info_hw_ip *info)
{
	struct drm_amdgpu_info request;

	memset(&request, 0, sizeof(request));
	request.return_pointer = (uintptr_t)info;
	request.return_size = sizeof(*info);
	request.query = AMDGPU_INFO_HW_IP_INFO;
	request.query_hw_ip.type = type;
	request.query_hw_ip.ip_instance = ip_instance;

	return drmCommandWrite(dev->fd, DRM_AMDGPU_INFO, &request,
			       sizeof(struct drm_amdgpu_info));
}

drm_public int amdgpu_query_firmware_version(amdgpu_device_handle dev,
		unsigned fw_type, unsigned ip_instance, unsigned index,
		uint32_t *version, uint32_t *feature)
{
	struct drm_amdgpu_info request;
	struct drm_amdgpu_info_firmware firmware = {};
	int r;

	memset(&request, 0, sizeof(request));
	request.return_pointer = (uintptr_t)&firmware;
	request.return_size = sizeof(firmware);
	request.query = AMDGPU_INFO_FW_VERSION;
	request.query_fw.fw_type = fw_type;
	request.query_fw.ip_instance = ip_instance;
	request.query_fw.index = index;

	r = drmCommandWrite(dev->fd, DRM_AMDGPU_INFO, &request,
			    sizeof(struct drm_amdgpu_info));
	if (r)
		return r;

	*version = firmware.ver;
	*feature = firmware.feature;
	return 0;
}

drm_private int amdgpu_query_gpu_info_init(amdgpu_device_handle dev)
{
	int r, i;

	r = amdgpu_query_info(dev, AMDGPU_INFO_DEV_INFO, sizeof(dev->dev_info),
			      &dev->dev_info);
	if (r)
		return r;

	dev->info.asic_id = dev->dev_info.device_id;
	dev->info.chip_rev = dev->dev_info.chip_rev;
	dev->info.chip_external_rev = dev->dev_info.external_rev;
	dev->info.family_id = dev->dev_info.family;
	dev->info.max_engine_clk = dev->dev_info.max_engine_clock;
	dev->info.max_memory_clk = dev->dev_info.max_memory_clock;
	dev->info.gpu_counter_freq = dev->dev_info.gpu_counter_freq;
	dev->info.enabled_rb_pipes_mask = dev->dev_info.enabled_rb_pipes_mask;
	dev->info.rb_pipes = dev->dev_info.num_rb_pipes;
	dev->info.ids_flags = dev->dev_info.ids_flags;
	dev->info.num_hw_gfx_contexts = dev->dev_info.num_hw_gfx_contexts;
	dev->info.num_shader_engines = dev->dev_info.num_shader_engines;
	dev->info.num_shader_arrays_per_engine =
		dev->dev_info.num_shader_arrays_per_engine;
	dev->info.vram_type = dev->dev_info.vram_type;
	dev->info.vram_bit_width = dev->dev_info.vram_bit_width;
	dev->info.ce_ram_size = dev->dev_info.ce_ram_size;
	dev->info.vce_harvest_config = dev->dev_info.vce_harvest_config;
	dev->info.pci_rev_id = dev->dev_info.pci_rev;

	if (dev->info.family_id < AMDGPU_FAMILY_AI) {
		for (i = 0; i < (int)dev->info.num_shader_engines; i++) {
			unsigned instance = (i << AMDGPU_INFO_MMR_SE_INDEX_SHIFT) |
					    (AMDGPU_INFO_MMR_SH_INDEX_MASK <<
					     AMDGPU_INFO_MMR_SH_INDEX_SHIFT);

			r = amdgpu_read_mm_registers(dev, 0x263d, 1, instance, 0,
						     &dev->info.backend_disable[i]);
			if (r)
				return r;
			/* extract bitfield CC_RB_BACKEND_DISABLE.BACKEND_DISABLE */
			dev->info.backend_disable[i] =
				(dev->info.backend_disable[i] >> 16) & 0xff;

			r = amdgpu_read_mm_registers(dev, 0xa0d4, 1, instance, 0,
						     &dev->info.pa_sc_raster_cfg[i]);
			if (r)
				return r;

			if (dev->info.family_id >= AMDGPU_FAMILY_CI) {
				r = amdgpu_read_mm_registers(dev, 0xa0d5, 1, instance, 0,
						     &dev->info.pa_sc_raster_cfg1[i]);
				if (r)
					return r;
			}
		}
	}

	r = amdgpu_read_mm_registers(dev, 0x263e, 1, 0xffffffff, 0,
					     &dev->info.gb_addr_cfg);
	if (r)
		return r;

	if (dev->info.family_id < AMDGPU_FAMILY_AI) {
		r = amdgpu_read_mm_registers(dev, 0x2644, 32, 0xffffffff, 0,
					     dev->info.gb_tile_mode);
		if (r)
			return r;

		if (dev->info.family_id >= AMDGPU_FAMILY_CI) {
			r = amdgpu_read_mm_registers(dev, 0x2664, 16, 0xffffffff, 0,
						     dev->info.gb_macro_tile_mode);
			if (r)
				return r;
		}

		r = amdgpu_read_mm_registers(dev, 0x9d8, 1, 0xffffffff, 0,
					     &dev->info.mc_arb_ramcfg);
		if (r)
			return r;
	}

	dev->info.cu_active_number = dev->dev_info.cu_active_number;
	dev->info.cu_ao_mask = dev->dev_info.cu_ao_mask;
	memcpy(&dev->info.cu_bitmap[0][0], &dev->dev_info.cu_bitmap[0][0], sizeof(dev->info.cu_bitmap));

	/* TODO: info->max_quad_shader_pipes is not set */
	/* TODO: info->avail_quad_shader_pipes is not set */
	/* TODO: info->cache_entries_per_quad_pipe is not set */
	return 0;
}

drm_public int amdgpu_query_gpu_info(amdgpu_device_handle dev,
				     struct amdgpu_gpu_info *info)
{
	if (!dev || !info)
		return -EINVAL;

	/* Get ASIC info*/
	*info = dev->info;

	return 0;
}

drm_public int amdgpu_query_heap_info(amdgpu_device_handle dev,
				      uint32_t heap,
				      uint32_t flags,
				      struct amdgpu_heap_info *info)
{
	struct drm_amdgpu_info_vram_gtt vram_gtt_info = {};
	int r;

	r = amdgpu_query_info(dev, AMDGPU_INFO_VRAM_GTT,
			      sizeof(vram_gtt_info), &vram_gtt_info);
	if (r)
		return r;

	/* Get heap information */
	switch (heap) {
	case AMDGPU_GEM_DOMAIN_VRAM:
		/* query visible only vram heap */
		if (flags & AMDGPU_GEM_CREATE_CPU_ACCESS_REQUIRED)
			info->heap_size = vram_gtt_info.vram_cpu_accessible_size;
		else /* query total vram heap */
			info->heap_size = vram_gtt_info.vram_size;

		info->max_allocation = vram_gtt_info.vram_cpu_accessible_size;

		if (flags & AMDGPU_GEM_CREATE_CPU_ACCESS_REQUIRED)
			r = amdgpu_query_info(dev, AMDGPU_INFO_VIS_VRAM_USAGE,
					      sizeof(info->heap_usage),
					      &info->heap_usage);
		else
			r = amdgpu_query_info(dev, AMDGPU_INFO_VRAM_USAGE,
					      sizeof(info->heap_usage),
					      &info->heap_usage);
		if (r)
			return r;
		break;
	case AMDGPU_GEM_DOMAIN_GTT:
		info->heap_size = vram_gtt_info.gtt_size;
		info->max_allocation = vram_gtt_info.vram_cpu_accessible_size;

		r = amdgpu_query_info(dev, AMDGPU_INFO_GTT_USAGE,
				      sizeof(info->heap_usage),
				      &info->heap_usage);
		if (r)
			return r;
		break;
	default:
		return -EINVAL;
	}

	return 0;
}

drm_public int amdgpu_query_gds_info(amdgpu_device_handle dev,
				     struct amdgpu_gds_resource_info *gds_info)
{
	struct drm_amdgpu_info_gds gds_config = {};
        int r;

	if (!gds_info)
		return -EINVAL;

        r = amdgpu_query_info(dev, AMDGPU_INFO_GDS_CONFIG,
                              sizeof(gds_config), &gds_config);
        if (r)
                return r;

	gds_info->gds_gfx_partition_size = gds_config.gds_gfx_partition_size;
	gds_info->compute_partition_size = gds_config.compute_partition_size;
	gds_info->gds_total_size = gds_config.gds_total_size;
	gds_info->gws_per_gfx_partition = gds_config.gws_per_gfx_partition;
	gds_info->gws_per_compute_partition = gds_config.gws_per_compute_partition;
	gds_info->oa_per_gfx_partition = gds_config.oa_per_gfx_partition;
	gds_info->oa_per_compute_partition = gds_config.oa_per_compute_partition;

	return 0;
}

drm_public int amdgpu_query_sensor_info(amdgpu_device_handle dev, unsigned sensor_type,
					unsigned size, void *value)
{
	struct drm_amdgpu_info request;

	memset(&request, 0, sizeof(request));
	request.return_pointer = (uintptr_t)value;
	request.return_size = size;
	request.query = AMDGPU_INFO_SENSOR;
	request.sensor_info.type = sensor_type;

	return drmCommandWrite(dev->fd, DRM_AMDGPU_INFO, &request,
			       sizeof(struct drm_amdgpu_info));
}

drm_public int amdgpu_query_video_caps_info(amdgpu_device_handle dev, unsigned cap_type,
                                            unsigned size, void *value)
{
	struct drm_amdgpu_info request;

	memset(&request, 0, sizeof(request));
	request.return_pointer = (uintptr_t)value;
	request.return_size = size;
	request.query = AMDGPU_INFO_VIDEO_CAPS;
	request.sensor_info.type = cap_type;

	return drmCommandWrite(dev->fd, DRM_AMDGPU_INFO, &request,
			       sizeof(struct drm_amdgpu_info));
}

drm_public int amdgpu_query_gpuvm_fault_info(amdgpu_device_handle dev,
					     unsigned size, void *value)
{
	struct drm_amdgpu_info request;

	memset(&request, 0, sizeof(request));
	request.return_pointer = (uintptr_t)value;
	request.return_size = size;
	request.query = AMDGPU_INFO_GPUVM_FAULT;

	return drmCommandWrite(dev->fd, DRM_AMDGPU_INFO, &request,
			       sizeof(struct drm_amdgpu_info));
}
