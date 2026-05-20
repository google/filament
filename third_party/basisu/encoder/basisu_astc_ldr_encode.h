// File: basisu_astc_ldr_encode.cpp
#pragma once
#include "basisu_enc.h"
#include "../transcoder/basisu_astc_helpers.h"

namespace basisu {
namespace astc_ldr {
			
	void encoder_init();

	const int EFFORT_LEVEL_MIN = 0, EFFORT_LEVEL_MAX = 10, EFFORT_LEVEL_DEF = 3;
	const int DCT_QUALITY_MIN = 1, DCT_QUALITY_MAX = 100;
		
	struct astc_ldr_encode_config
	{
		astc_ldr_encode_config()
		{
		}

		void clear()
		{
			*this = astc_ldr_encode_config();
		}
				
		// ASTC LDR block dimensions. Must be a valid ASTC block dimension. Any supported from 4x4-12x12, including unequal dimensions.
		uint32_t m_astc_block_width = 6;
		uint32_t m_astc_block_height = 6;

		// If true, the encoder assumes all ASTC blocks will be decompressed using sRGB vs. LDR8 mode. This corresponds to astcenc's -cs vs. cl color profiles.
		// This should match how the texture is later decoded by the GPU for maximum quality. This bit is stored into the output file.
		bool m_astc_decode_mode_srgb = true;

		// If true, trade off some compression (3-10%) for faster decompression.
		// If false, favor highest compression, but slower decompression.
		//bool m_use_faster_format = true;
		
		basist::astc_ldr_t::xuastc_ldr_syntax m_compressed_syntax = basist::astc_ldr_t::xuastc_ldr_syntax::cFullArith;

		// Encoder CPU effort vs. quality. [0,10], higher=better.
		// 0=extremely fast but very brittle (no subsets)
		// 1=first 2 subset effort level
		// 10=extremely high CPU requirements. 
		uint32_t m_effort_level = 3;

		// Weight grid DCT quality [1,100] - higher=better quality (JPEG-style).
		float m_dct_quality = 85;
		
		// true=use weight grid DCT, false=always use DPCM
		bool m_use_dct = false;
		
		// true=use lossy supercompression, false=supercompression stage is always lossless.
		bool m_lossy_supercompression = false;

		// Channel weights used to compute RGBA colorspace L2 errors. Must be >= 1.
		uint32_t m_comp_weights[4] = { 1, 1, 1, 1 };

		// Lossy supercompression stage parameters for RGB vs. RGBA image inputs.
		// (Bounded RDO - explictly not Lagrangian.)
		float m_replacement_min_psnr = 35.0f; // if the block's base PSNR is less than this, it cannot be changed
		float m_psnr_trial_diff_thresh = 1.5f; // reject candidates if their PSNR is lower than m_replacement_min_psnr-m_psnr_trial_diff_thresh
		float m_psnr_trial_diff_thresh_edge = 1.0f; // edge variant

		// Lossy supercompression settings - alpha texture variants
		float m_replacement_min_psnr_alpha = 38.0f;
		float m_psnr_trial_diff_thresh_alpha = .75f;
		float m_psnr_trial_diff_thresh_edge_alpha = .5f;

		// If true, try encoding blurred blocks, in addition to unblurred, for superpass 1 and 2. 
		// Higher quality, but massively slower and not yet tuned/refined.
		bool m_block_blurring_p1 = false, m_block_blurring_p2 = false;

		// If true, no matter what effort level subset usage will be disabled.
		bool m_force_disable_subsets = false;
		
		// If true, no matter what effort level RGB dual plane usage will be disabled.
		bool m_force_disable_rgb_dual_plane = false;
				
		bool m_debug_images = false;
		bool m_debug_output = false;

		std::string m_debug_file_prefix;

		void debug_print() const
		{
			fmt_debug_printf("ASTC block dimensions: {}x{}\n", m_astc_block_width, m_astc_block_height);
			fmt_debug_printf("ASTC decode profile mode sRGB: {}\n", m_astc_decode_mode_srgb);
			fmt_debug_printf("Syntax: {}\n", (uint32_t)m_compressed_syntax);
			fmt_debug_printf("Effort level: {}\n", m_effort_level);
			fmt_debug_printf("Use DCT: {}\n", m_use_dct);
			fmt_debug_printf("DCT quality level (1-100): {}\n", m_dct_quality);
			fmt_debug_printf("Comp weights: {} {} {} {}\n", m_comp_weights[0], m_comp_weights[1], m_comp_weights[2], m_comp_weights[3]);
			fmt_debug_printf("Block blurring: {} {}\n", m_block_blurring_p1, m_block_blurring_p2);
			fmt_debug_printf("Force disable subsets: {}\n", m_force_disable_subsets);
			fmt_debug_printf("Force disable RGB dual plane: {}\n", m_force_disable_rgb_dual_plane);

			fmt_debug_printf("\nLossy supercompression: {}\n", m_lossy_supercompression);
			fmt_debug_printf("m_replacement_min_psnr: {}\n", m_replacement_min_psnr);
			fmt_debug_printf("m_psnr_trial_diff_thresh: {}\n", m_psnr_trial_diff_thresh);
			fmt_debug_printf("m_psnr_trial_diff_thresh_edge: {}\n", m_psnr_trial_diff_thresh_edge);
			fmt_debug_printf("m_replacement_min_psnr_alpha: {}\n", m_replacement_min_psnr_alpha);
			fmt_debug_printf("m_psnr_trial_diff_thresh_alpha: {}\n", m_psnr_trial_diff_thresh_alpha);
			fmt_debug_printf("m_psnr_trial_diff_thresh_edge_alpha: {}\n", m_psnr_trial_diff_thresh_edge_alpha);

			fmt_debug_printf("m_debug_images: {}\n", m_debug_images);
		}
	};

	bool compress_image(
		const image& orig_img, uint8_vec &comp_data, vector2D<astc_helpers::log_astc_block>& coded_blocks,
		const astc_ldr_encode_config& global_cfg,
		job_pool& job_pool);
	
	void deblock_filter(uint32_t filter_block_width, uint32_t filter_block_height, const image& src_img, image& dst_img, bool stronger_filtering = false, int SKIP_THRESH = 24);

} // namespace astc_ldr
} // namespace basisu


