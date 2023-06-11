// Ad-hoc test that the wave op masks work as expected.
#include <glm/glm.hpp>
#include <assert.h>

using namespace glm;

static uvec4 gl_SubgroupEqMask;
static uvec4 gl_SubgroupGeMask;
static uvec4 gl_SubgroupGtMask;
static uvec4 gl_SubgroupLeMask;
static uvec4 gl_SubgroupLtMask;
using uint4 = uvec4;

static void test_main(unsigned wave_index)
{    
	const auto WaveGetLaneIndex = [&]() { return wave_index; };

	gl_SubgroupEqMask = 1u << (WaveGetLaneIndex() - uint4(0, 32, 64, 96));
	if (WaveGetLaneIndex() >= 32) gl_SubgroupEqMask.x = 0;
	if (WaveGetLaneIndex() >= 64 || WaveGetLaneIndex() < 32) gl_SubgroupEqMask.y = 0;
	if (WaveGetLaneIndex() >= 96 || WaveGetLaneIndex() < 64) gl_SubgroupEqMask.z = 0;
	if (WaveGetLaneIndex() < 96) gl_SubgroupEqMask.w = 0;
	gl_SubgroupGeMask = ~((1u << (WaveGetLaneIndex() - uint4(0, 32, 64, 96))) - 1u);
	if (WaveGetLaneIndex() >= 32) gl_SubgroupGeMask.x = 0u;
	if (WaveGetLaneIndex() >= 64) gl_SubgroupGeMask.y = 0u;
	if (WaveGetLaneIndex() >= 96) gl_SubgroupGeMask.z = 0u;
	if (WaveGetLaneIndex() < 32) gl_SubgroupGeMask.y = ~0u;
	if (WaveGetLaneIndex() < 64) gl_SubgroupGeMask.z = ~0u;
	if (WaveGetLaneIndex() < 96) gl_SubgroupGeMask.w = ~0u;
	uint gt_lane_index = WaveGetLaneIndex() + 1;
	gl_SubgroupGtMask = ~((1u << (gt_lane_index - uint4(0, 32, 64, 96))) - 1u);
	if (gt_lane_index >= 32) gl_SubgroupGtMask.x = 0u;
	if (gt_lane_index >= 64) gl_SubgroupGtMask.y = 0u;
	if (gt_lane_index >= 96) gl_SubgroupGtMask.z = 0u;
	if (gt_lane_index >= 128) gl_SubgroupGtMask.w = 0u;
	if (gt_lane_index < 32) gl_SubgroupGtMask.y = ~0u;
	if (gt_lane_index < 64) gl_SubgroupGtMask.z = ~0u;
	if (gt_lane_index < 96) gl_SubgroupGtMask.w = ~0u;
	uint le_lane_index = WaveGetLaneIndex() + 1;
	gl_SubgroupLeMask = (1u << (le_lane_index - uint4(0, 32, 64, 96))) - 1u;
	if (le_lane_index >= 32) gl_SubgroupLeMask.x = ~0u;
	if (le_lane_index >= 64) gl_SubgroupLeMask.y = ~0u;
	if (le_lane_index >= 96) gl_SubgroupLeMask.z = ~0u;
	if (le_lane_index >= 128) gl_SubgroupLeMask.w = ~0u;
	if (le_lane_index < 32) gl_SubgroupLeMask.y = 0u;
	if (le_lane_index < 64) gl_SubgroupLeMask.z = 0u;
	if (le_lane_index < 96) gl_SubgroupLeMask.w = 0u;
	gl_SubgroupLtMask = (1u << (WaveGetLaneIndex() - uint4(0, 32, 64, 96))) - 1u;
	if (WaveGetLaneIndex() >= 32) gl_SubgroupLtMask.x = ~0u;
	if (WaveGetLaneIndex() >= 64) gl_SubgroupLtMask.y = ~0u;
	if (WaveGetLaneIndex() >= 96) gl_SubgroupLtMask.z = ~0u;
	if (WaveGetLaneIndex() < 32) gl_SubgroupLtMask.y = 0u;
	if (WaveGetLaneIndex() < 64) gl_SubgroupLtMask.z = 0u;
	if (WaveGetLaneIndex() < 96) gl_SubgroupLtMask.w = 0u;
}

int main()
{
	for (unsigned subgroup_id = 0; subgroup_id < 128; subgroup_id++)
	{
		test_main(subgroup_id);

		for (unsigned bit = 0; bit < 128; bit++)
		{
			assert(bool(gl_SubgroupEqMask[bit / 32] & (1u << (bit & 31))) == (bit == subgroup_id));
			assert(bool(gl_SubgroupGtMask[bit / 32] & (1u << (bit & 31))) == (bit > subgroup_id));
			assert(bool(gl_SubgroupGeMask[bit / 32] & (1u << (bit & 31))) == (bit >= subgroup_id));
			assert(bool(gl_SubgroupLtMask[bit / 32] & (1u << (bit & 31))) == (bit < subgroup_id));
			assert(bool(gl_SubgroupLeMask[bit / 32] & (1u << (bit & 31))) == (bit <= subgroup_id));
		}
	}
}

