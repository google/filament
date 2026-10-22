// File: basisu_astc_ldr_common.h
#pragma once
#include "basisu_enc.h"
#include "basisu_gpu_texture.h"
#include <array>

namespace basisu
{

namespace astc_ldr
{
	const uint32_t ASTC_LDR_MAX_BLOCK_WIDTH = astc_helpers::MAX_BLOCK_DIM; // 12
	const uint32_t ASTC_LDR_MAX_BLOCK_HEIGHT = astc_helpers::MAX_BLOCK_DIM; // 12
	const uint32_t ASTC_LDR_MAX_BLOCK_PIXELS = astc_helpers::MAX_BLOCK_PIXELS; // 144
	const uint32_t ASTC_LDR_MAX_RAW_WEIGHTS = astc_helpers::MAX_WEIGHT_INTERPOLANT_VALUE + 1; // 65
	
	const uint32_t WEIGHT_REFINER_MAX_PASSES = 17;

	inline basist::color_rgba convert_to_basist_color_rgba(const color_rgba& c)
	{
		return basist::color_rgba(c.r, c.g, c.b, c.a);
	}

	struct cem_encode_params
	{
		uint32_t m_comp_weights[4];
		bool m_decode_mode_srgb; // todo: store astc_helpers::cDecodeModeSRGB8 : astc_helpers::cDecodeModeLDR8 instead, also the alpha mode for srgb because the decoders are broken

		const uint8_t* m_pForced_weight_vals0;
		const uint8_t* m_pForced_weight_vals1;

		uint32_t m_max_ls_passes, m_total_weight_refine_passes;
		bool m_worst_weight_nudging_flag;
		bool m_endpoint_refinement_flag;
				
		cem_encode_params() 
		{
			init();
		}

		void init()
		{
			m_comp_weights[0] = 1;
			m_comp_weights[1] = 1;
			m_comp_weights[2] = 1;
			m_comp_weights[3] = 1;

			m_decode_mode_srgb = true;

			m_pForced_weight_vals0 = nullptr;
			m_pForced_weight_vals1 = nullptr;

			m_max_ls_passes = 3;
			m_total_weight_refine_passes = 0;
			m_worst_weight_nudging_flag = false;
			m_endpoint_refinement_flag = false;
		}

		float get_total_comp_weights() const
		{
			return (float)(m_comp_weights[0] + m_comp_weights[1] + m_comp_weights[2] + m_comp_weights[3]);
		}
	};

	struct pixel_stats_t
	{
		uint32_t m_num_pixels;

		color_rgba m_pixels[ASTC_LDR_MAX_BLOCK_PIXELS];
		vec4F m_pixels_f[ASTC_LDR_MAX_BLOCK_PIXELS];

		color_rgba m_min, m_max;

		vec4F m_min_f, m_max_f;
		vec4F m_mean_f;
				
		// Always 3D, ignoring alpha
		vec3F m_mean_rel_axis3;
		vec3F m_zero_rel_axis3;
		
		// Always 4D
		vec4F m_mean_rel_axis4;
								
		bool m_has_alpha;
		
		stats<float> m_rgba_stats[4];

		void clear()
		{
			clear_obj(*this);
		}

		void init(uint32_t num_pixels, const color_rgba* pPixels);
		
	}; // struct struct pixel_stats

	void global_init();

	void bit_transfer_signed_enc(int& a, int& b);
	void bit_transfer_signed_dec(int& a, int& b); // transfers MSB from a to b, a is then [-32,31]
	color_rgba blue_contract_enc(color_rgba orig, bool& did_clamp, int encoded_b);
	int quant_preserve2(uint32_t ise_range, uint32_t v);

	uint32_t get_colors(const color_rgba& l, const color_rgba& h, uint32_t weight_ise_index, color_rgba* pColors, bool decode_mode_srgb);
	uint32_t get_colors_raw_weights(const color_rgba& l, const color_rgba& h, color_rgba* pColors, bool decode_mode_srgb);
	void decode_endpoints_ise20(uint32_t cem_index, const uint8_t* pEndpoint_vals, color_rgba& l, color_rgba& h); // assume BISE 20
	void decode_endpoints(uint32_t cem_index, const uint8_t* pEndpoint_vals, uint32_t endpoint_ise_index, color_rgba& l, color_rgba& h, float* pScale = nullptr);
	uint32_t get_colors(uint32_t cem_index, const uint8_t* pEndpoint_vals, uint32_t endpoint_ise_index, uint32_t weight_ise_index, color_rgba* pColors, bool decode_mode_srgb);
	uint32_t get_colors_raw_weights(uint32_t cem_index, const uint8_t* pEndpoint_vals, uint32_t endpoint_ise_index, color_rgba* pColors, bool decode_mode_srgb);

	//int apply_delta_to_bise_endpoint_val(uint32_t endpoint_ise_range, int ise_val, int delta);
	int apply_delta_to_bise_weight_val(uint32_t weight_ise_range, int ise_val, int delta);

	uint64_t eval_solution(
		const pixel_stats_t& pixel_stats,
		uint32_t total_weights, const color_rgba* pWeight_colors,
		uint8_t* pWeight_vals, uint32_t weight_ise_index,
		const cem_encode_params& params);

	uint64_t eval_solution(
		const pixel_stats_t& pixel_stats,
		uint32_t cem_index,
		const uint8_t* pEndpoint_vals, uint32_t endpoint_ise_index,
		uint8_t* pWeight_vals, uint32_t weight_ise_index,
		const cem_encode_params& params);

	uint64_t eval_solution_dp(
		uint32_t ccs_index,
		const pixel_stats_t& pixel_stats,
		uint32_t total_weights, const color_rgba* pWeight_colors,
		uint8_t* pWeight_vals0, uint8_t* pWeight_vals1, uint32_t weight_ise_index,
		const cem_encode_params& params);

	uint64_t eval_solution_dp(
		const pixel_stats_t& pixel_stats,
		uint32_t cem_index, uint32_t ccs_index,
		const uint8_t* pEndpoint_vals, uint32_t endpoint_ise_index,
		uint8_t* pWeight_vals0, uint8_t* pWeight_vals1, uint32_t weight_ise_index,
		const cem_encode_params& params);

	//bool cem8_or_12_used_blue_contraction(uint32_t cem_index, const uint8_t* pEndpoint_vals, uint32_t endpoint_ise_index);
	//bool cem9_or_13_used_blue_contraction(uint32_t cem_index, const uint8_t* pEndpoint_vals, uint32_t endpoint_ise_index);
	//bool used_blue_contraction(uint32_t cem_index, const uint8_t* pEndpoint_vals, uint32_t endpoint_ise_index);

	uint64_t cem_encode_pixels(
		uint32_t cem_index, int ccs_index,
		const pixel_stats_t& pixel_stats, const cem_encode_params& enc_params,
		uint32_t endpoint_ise_range, uint32_t weight_ise_range,
		uint8_t* pEndpoint_vals, uint8_t* pWeight_vals0, uint8_t* pWeight_vals1, uint64_t cur_blk_error,
		bool use_blue_contraction, bool* pBase_ofs_clamped_flag);

	// TODO: Rename, confusing vs. std::vector or basisu::vector or vec4F etc.
	struct partition_pattern_vec
	{
		uint32_t m_width, m_height;
		uint8_t m_parts[ASTC_LDR_MAX_BLOCK_PIXELS];
		
		partition_pattern_vec();

		partition_pattern_vec(const partition_pattern_vec& other);

		partition_pattern_vec(uint32_t width, uint32_t height, const uint8_t* pParts = nullptr);

		void init(uint32_t width, uint32_t height, const uint8_t* pParts = nullptr);

		void init_part_hist();

		void clear();

		partition_pattern_vec& operator= (const partition_pattern_vec& rhs);

		uint32_t get_width() const { return m_width; }
		uint32_t get_height() const { return m_height; }
		uint32_t get_total() const { return m_width * m_height; }

		uint8_t operator[] (uint32_t i) const { assert(i < get_total()); return m_parts[i]; }
		uint8_t& operator[] (uint32_t i) { assert(i < get_total()); return m_parts[i]; }

		uint8_t operator() (uint32_t x, uint32_t y) const { assert((x < m_width) && (y < m_height)); return m_parts[x + y * m_width]; }
		uint8_t& operator() (uint32_t x, uint32_t y) { assert((x < m_width) && (y < m_height)); return m_parts[x + y * m_width]; }

		int get_squared_distance(const partition_pattern_vec& other) const;
		
		float get_distance(const partition_pattern_vec& other) const
		{
			return sqrtf((float)get_squared_distance(other));
		}

		enum { cMaxPermute2Index = 1 };
		partition_pattern_vec get_permuted2(uint32_t permute_index) const;

		enum { cMaxPermute3Index = 5 };
		partition_pattern_vec get_permuted3(uint32_t permute_index) const;

		partition_pattern_vec get_canonicalized() const;

		bool operator== (const partition_pattern_vec& rhs) const
		{
			if ((m_width != rhs.m_width) || (m_height != rhs.m_height))
				return false;

			return memcmp(m_parts, rhs.m_parts, get_total()) == 0;
		}

		operator size_t() const
		{
			return basist::hash_hsieh(m_parts, get_total());
		}
	};

	struct vp_tree_node
	{
		partition_pattern_vec m_vantage_point;
		uint32_t m_point_index;
		float m_dist;

		int m_inner_node, m_outer_node;
	};

	const uint32_t NUM_PART3_MAPPINGS = 6;
	extern uint8_t g_part3_mapping[NUM_PART3_MAPPINGS][3];

	class vp_tree
	{
	public:
		vp_tree()
		{
		}

		void clear()
		{
			m_nodes.clear();
		}

		// This requires no redundant patterns, i.e. all must be unique.
		bool init(uint32_t n, const partition_pattern_vec* pUnique_pats);

		struct result
		{
			uint32_t m_pat_index;
			uint32_t m_mapping_index;
			float m_dist;

			bool operator< (const result& rhs) const { return m_dist < rhs.m_dist; }
			bool operator> (const result& rhs) const { return m_dist > rhs.m_dist; }
		};

		class result_queue
		{
			enum { MaxSupportedSize = 512 + 1 };

		public:
			result_queue() :
				m_cur_size(0)
			{
			}

			size_t get_size() const
			{
				return m_cur_size;
			}

			bool empty() const
			{
				return !m_cur_size;
			}

			typedef std::array<result, MaxSupportedSize + 1> result_array_type;

			const result_array_type& get_elements() const { return m_elements; }
			result_array_type& get_elements() { return m_elements; }

			void clear()
			{
				m_cur_size = 0;
			}

			void reserve(uint32_t n)
			{
				BASISU_NOTE_UNUSED(n);
			}

			const result& top() const
			{
				assert(m_cur_size);
				return m_elements[1];
			}

			bool insert(const result& val, uint32_t max_size)
			{
				assert(max_size < MaxSupportedSize);

				if (m_cur_size >= MaxSupportedSize)
					return false;

				m_elements[++m_cur_size] = val;
				up_heap(m_cur_size);

				if (m_cur_size > max_size)
					pop();

				return true;
			}

			bool pop()
			{
				if (m_cur_size == 0)
					return false;

				m_elements[1] = m_elements[m_cur_size--];
				down_heap(1);
				return true;
			}

			float get_highest_dist() const
			{
				if (!m_cur_size)
					return 0.0f;

				return top().m_dist;
			}

		private:
			result_array_type m_elements;
			size_t m_cur_size;

			void up_heap(size_t index)
			{
				while ((index > 1) && (m_elements[index] > m_elements[index >> 1]))
				{
					std::swap(m_elements[index], m_elements[index >> 1]);
					index >>= 1;
				}
			}

			void down_heap(size_t index)
			{
				for (; ; )
				{
					size_t largest = index, left_child = 2 * index, right_child = 2 * index + 1;

					if ((left_child <= m_cur_size) && (m_elements[left_child] > m_elements[largest]))
						largest = left_child;

					if ((right_child <= m_cur_size) && (m_elements[right_child] > m_elements[largest]))
						largest = right_child;

					if (largest == index)
						break;

					std::swap(m_elements[index], m_elements[largest]);
					index = largest;
				}
			}
		};

		void find_nearest(uint32_t num_subsets, const partition_pattern_vec& desired_pat, result_queue& results, uint32_t max_results) const;

	private:
		basisu::vector<vp_tree_node> m_nodes;

		void find_nearest_at_node(int node_index, uint32_t num_desired_pats, const partition_pattern_vec* pDesired_pats, result_queue& results, uint32_t max_results) const;
		
		void find_nearest_at_node_non_recursive(int init_node_index, uint32_t num_desired_pats, const partition_pattern_vec* pDesired_pats, result_queue& results, uint32_t max_results) const;
		
		// returns the index of the new node, or -1 on error
		int create_node(uint32_t n, const partition_pattern_vec* pUnique_pats, const uint_vec& pat_indices);
		
		// returns the pattern index of the vantage point (-1 on error), and the optimal split distance
		std::pair<int, float> find_best_vantage_point(uint32_t num_unique_pats, const partition_pattern_vec* pUnique_pats, const uint_vec& pat_indices);
	};

	typedef basisu::hash_map<partition_pattern_vec, std::pair<uint32_t, uint32_t > > partition_hash_map;

	struct partition_pattern_hist
	{
		uint8_t m_hist[4];

		partition_pattern_hist() { clear(); }

		void clear() { clear_obj(m_hist); }
	};

	struct partitions_data
	{
		uint32_t m_width, m_height, m_num_partitions;
		partition_pattern_vec m_partition_pats[astc_helpers::NUM_PARTITION_PATTERNS]; // indexed by unique index, NOT the 10-bit ASTC seed/pattern index
		
		partition_pattern_hist m_partition_pat_histograms[astc_helpers::NUM_PARTITION_PATTERNS]; // indexed by unique index, histograms of each pattern

		// ASTC seed to unique index and vice versa
		int16_t m_part_seed_to_unique_index[astc_helpers::NUM_PARTITION_PATTERNS];
		int16_t m_unique_index_to_part_seed[astc_helpers::NUM_PARTITION_PATTERNS];

		// Total number of unique patterns
		uint32_t m_total_unique_patterns;
		
		// VP tree used to rapidly find nearby/similar patterns.
		vp_tree m_part_vp_tree;
		
		void init(uint32_t num_partitions, uint32_t block_width, uint32_t block_height, bool init_vp_tree = true);
	};

	float surrogate_quant_endpoint_val(float e, uint32_t num_endpoint_levels, uint32_t flags);
	vec4F surrogate_quant_endpoint(const vec4F& e, uint32_t num_endpoint_levels, uint32_t flags);

	float surrogate_evaluate_rgba_sp(const pixel_stats_t& ps, const vec4F& l, const vec4F& h, float* pWeights0, uint32_t num_weight_levels, const cem_encode_params& enc_params, uint32_t flags);
	float surrogate_evaluate_rgba_dp(uint32_t ccs_index, const pixel_stats_t& ps, const vec4F& l, const vec4F& h, float* pWeights0, float* pWeights1, uint32_t num_weight_levels, const cem_encode_params& enc_params, uint32_t flags);

	enum
	{
		cFlagDisableQuant = 1,
		cFlagNoError = 2
	}
	;
	float cem_surrogate_encode_pixels(
		uint32_t cem_index, int ccs_index,
		const pixel_stats_t& pixel_stats, const cem_encode_params& enc_params,
		uint32_t endpoint_ise_range, uint32_t weight_ise_range,
		vec4F& low_endpoint, vec4F& high_endpoint, float& s, float* pWeights0, float* pWeights1, uint32_t flags = 0);

#if 0
	bool requantize_ise_endpoints(uint32_t cem,
		uint32_t src_ise_endpoint_range, const uint8_t* pSrc_endpoints,
		uint32_t dst_ise_endpoint_range, uint8_t* pDst_endpoints);

	uint32_t get_base_cem_without_alpha(uint32_t cem);

	bool pack_base_offset(
		uint32_t cem_index, uint32_t dst_ise_endpoint_range, uint8_t* pPacked_endpoints,
		const color_rgba& l, const color_rgba& h,
		bool use_blue_contraction, bool auto_disable_blue_contraction_if_clamped,
		bool& blue_contraction_clamped_flag, bool& base_ofs_clamped_flag, bool& endpoints_swapped);

	bool convert_endpoints_across_cems(
		uint32_t prev_cem, uint32_t prev_endpoint_ise_range, const uint8_t* pPrev_endpoints,
		uint32_t dst_cem, uint32_t dst_endpoint_ise_range, uint8_t* pDst_endpoints,
		bool always_repack,
		bool use_blue_contraction, bool auto_disable_blue_contraction_if_clamped,
		bool& blue_contraction_clamped_flag, bool& base_ofs_clamped_flag);
#endif

} //  namespace astc_ldr

} // namespace basisu
