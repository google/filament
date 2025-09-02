#version 450
#extension GL_KHR_shader_subgroup_basic : require
#extension GL_KHR_shader_subgroup_ballot : require
#extension GL_KHR_shader_subgroup_vote : require
#extension GL_KHR_shader_subgroup_shuffle : require
#extension GL_KHR_shader_subgroup_shuffle_relative : require
#extension GL_KHR_shader_subgroup_arithmetic : require
#extension GL_KHR_shader_subgroup_clustered : require
#extension GL_KHR_shader_subgroup_quad : require

layout(location = 0) out float FragColor;

void main()
{
	// basic
	FragColor = float(gl_SubgroupSize);
	FragColor = float(gl_SubgroupInvocationID);
	subgroupBarrier();
	subgroupMemoryBarrier();
	subgroupMemoryBarrierBuffer();
	subgroupMemoryBarrierImage();
	bool elected = subgroupElect();

	// ballot
	FragColor = float(gl_SubgroupEqMask);
	FragColor = float(gl_SubgroupGeMask);
	FragColor = float(gl_SubgroupGtMask);
	FragColor = float(gl_SubgroupLeMask);
	FragColor = float(gl_SubgroupLtMask);
	vec4 broadcasted = subgroupBroadcast(vec4(10.0), 8u);
	bvec2 broadcasted_bool = subgroupBroadcast(bvec2(true), 8u);
	vec3 first = subgroupBroadcastFirst(vec3(20.0));
	bvec4 first_bool = subgroupBroadcastFirst(bvec4(false));
	uvec4 ballot_value = subgroupBallot(true);
	bool inverse_ballot_value = subgroupInverseBallot(ballot_value);
	bool bit_extracted = subgroupBallotBitExtract(uvec4(10u), 8u);
	uint bit_count = subgroupBallotBitCount(ballot_value);
	uint inclusive_bit_count = subgroupBallotInclusiveBitCount(ballot_value);
	uint exclusive_bit_count = subgroupBallotExclusiveBitCount(ballot_value);
	uint lsb = subgroupBallotFindLSB(ballot_value);
	uint msb = subgroupBallotFindMSB(ballot_value);

	// shuffle
	uint shuffled = subgroupShuffle(10u, 8u);
	bool shuffled_bool = subgroupShuffle(true, 9u);
	uint shuffled_xor = subgroupShuffleXor(30u, 8u);
	bool shuffled_xor_bool = subgroupShuffleXor(false, 9u);

	// shuffle relative 
	uint shuffled_up = subgroupShuffleUp(20u, 4u);
	bool shuffled_up_bool = subgroupShuffleUp(true, 4u);
	uint shuffled_down = subgroupShuffleDown(20u, 4u);
	bool shuffled_down_bool = subgroupShuffleDown(false, 4u);

	// vote
	bool has_all = subgroupAll(true);
	bool has_any = subgroupAny(true);
	bool has_equal = subgroupAllEqual(0);
	has_equal = subgroupAllEqual(true);
	has_equal = subgroupAllEqual(vec3(0.0, 1.0, 2.0));
	has_equal = subgroupAllEqual(bvec4(true, true, false, true));

	// arithmetic
	vec4 added = subgroupAdd(vec4(20.0));
	ivec4 iadded = subgroupAdd(ivec4(20));
	vec4 multiplied = subgroupMul(vec4(20.0));
	ivec4 imultiplied = subgroupMul(ivec4(20));
	vec4 lo = subgroupMin(vec4(20.0));
	vec4 hi = subgroupMax(vec4(20.0));
	ivec4 slo = subgroupMin(ivec4(20));
	ivec4 shi = subgroupMax(ivec4(20));
	uvec4 ulo = subgroupMin(uvec4(20));
	uvec4 uhi = subgroupMax(uvec4(20));
	uvec4 anded = subgroupAnd(ballot_value);
	uvec4 ored = subgroupOr(ballot_value);
	uvec4 xored = subgroupXor(ballot_value);

	added = subgroupInclusiveAdd(added);
	iadded = subgroupInclusiveAdd(iadded);
	multiplied = subgroupInclusiveMul(multiplied);
	imultiplied = subgroupInclusiveMul(imultiplied);
	//lo = subgroupInclusiveMin(lo);  // FIXME: Unsupported by Metal
	//hi = subgroupInclusiveMax(hi);
	//slo = subgroupInclusiveMin(slo);
	//shi = subgroupInclusiveMax(shi);
	//ulo = subgroupInclusiveMin(ulo);
	//uhi = subgroupInclusiveMax(uhi);
	//anded = subgroupInclusiveAnd(anded);
	//ored = subgroupInclusiveOr(ored);
	//xored = subgroupInclusiveXor(ored);
	//added = subgroupExclusiveAdd(lo);

	added = subgroupExclusiveAdd(multiplied);
	multiplied = subgroupExclusiveMul(multiplied);
	iadded = subgroupExclusiveAdd(imultiplied);
	imultiplied = subgroupExclusiveMul(imultiplied);
	//lo = subgroupExclusiveMin(lo);  // FIXME: Unsupported by Metal
	//hi = subgroupExclusiveMax(hi);
	//ulo = subgroupExclusiveMin(ulo);
	//uhi = subgroupExclusiveMax(uhi);
	//slo = subgroupExclusiveMin(slo);
	//shi = subgroupExclusiveMax(shi);
	//anded = subgroupExclusiveAnd(anded);
	//ored = subgroupExclusiveOr(ored);
	//xored = subgroupExclusiveXor(ored);

	// clustered
	added = subgroupClusteredAdd(added, 4u);
	multiplied = subgroupClusteredMul(multiplied, 4u);
	iadded = subgroupClusteredAdd(iadded, 4u);
	imultiplied = subgroupClusteredMul(imultiplied, 4u);
	lo = subgroupClusteredMin(lo, 4u);
	hi = subgroupClusteredMax(hi, 4u);
	ulo = subgroupClusteredMin(ulo, 4u);
	uhi = subgroupClusteredMax(uhi, 4u);
	slo = subgroupClusteredMin(slo, 4u);
	shi = subgroupClusteredMax(shi, 4u);
	anded = subgroupClusteredAnd(anded, 4u);
	ored = subgroupClusteredOr(ored, 4u);
	xored = subgroupClusteredXor(xored, 4u);

	// quad
	vec4 swap_horiz = subgroupQuadSwapHorizontal(vec4(20.0));
	bvec4 swap_horiz_bool = subgroupQuadSwapHorizontal(bvec4(true));
	vec4 swap_vertical = subgroupQuadSwapVertical(vec4(20.0));
	bvec4 swap_vertical_bool = subgroupQuadSwapVertical(bvec4(true));
	vec4 swap_diagonal = subgroupQuadSwapDiagonal(vec4(20.0));
	bvec4 swap_diagonal_bool = subgroupQuadSwapDiagonal(bvec4(true));
	vec4 quad_broadcast = subgroupQuadBroadcast(vec4(20.0), 3u);
	bvec4 quad_broadcast_bool = subgroupQuadBroadcast(bvec4(true), 3u);
}
