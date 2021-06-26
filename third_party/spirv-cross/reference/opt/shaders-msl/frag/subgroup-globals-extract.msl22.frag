#pragma clang diagnostic ignored "-Wmissing-prototypes"

#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

struct main0_out
{
    uint2 FragColor [[color(0)]];
};

inline uint spvSubgroupBallotFindLSB(uint4 ballot, uint gl_SubgroupSize)
{
    uint4 mask = uint4(extract_bits(0xFFFFFFFF, 0, min(gl_SubgroupSize, 32u)), extract_bits(0xFFFFFFFF, 0, (uint)max((int)gl_SubgroupSize - 32, 0)), uint2(0));
    ballot &= mask;
    return select(ctz(ballot.x), select(32 + ctz(ballot.y), select(64 + ctz(ballot.z), select(96 + ctz(ballot.w), uint(-1), ballot.w == 0), ballot.z == 0), ballot.y == 0), ballot.x == 0);
}

inline uint spvSubgroupBallotFindMSB(uint4 ballot, uint gl_SubgroupSize)
{
    uint4 mask = uint4(extract_bits(0xFFFFFFFF, 0, min(gl_SubgroupSize, 32u)), extract_bits(0xFFFFFFFF, 0, (uint)max((int)gl_SubgroupSize - 32, 0)), uint2(0));
    ballot &= mask;
    return select(128 - (clz(ballot.w) + 1), select(96 - (clz(ballot.z) + 1), select(64 - (clz(ballot.y) + 1), select(32 - (clz(ballot.x) + 1), uint(-1), ballot.x == 0), ballot.y == 0), ballot.z == 0), ballot.w == 0);
}

inline uint spvPopCount4(uint4 ballot)
{
    return popcount(ballot.x) + popcount(ballot.y) + popcount(ballot.z) + popcount(ballot.w);
}

inline uint spvSubgroupBallotBitCount(uint4 ballot, uint gl_SubgroupSize)
{
    uint4 mask = uint4(extract_bits(0xFFFFFFFF, 0, min(gl_SubgroupSize, 32u)), extract_bits(0xFFFFFFFF, 0, (uint)max((int)gl_SubgroupSize - 32, 0)), uint2(0));
    return spvPopCount4(ballot & mask);
}

inline uint spvSubgroupBallotInclusiveBitCount(uint4 ballot, uint gl_SubgroupInvocationID)
{
    uint4 mask = uint4(extract_bits(0xFFFFFFFF, 0, min(gl_SubgroupInvocationID + 1, 32u)), extract_bits(0xFFFFFFFF, 0, (uint)max((int)gl_SubgroupInvocationID + 1 - 32, 0)), uint2(0));
    return spvPopCount4(ballot & mask);
}

inline uint spvSubgroupBallotExclusiveBitCount(uint4 ballot, uint gl_SubgroupInvocationID)
{
    uint4 mask = uint4(extract_bits(0xFFFFFFFF, 0, min(gl_SubgroupInvocationID, 32u)), extract_bits(0xFFFFFFFF, 0, (uint)max((int)gl_SubgroupInvocationID - 32, 0)), uint2(0));
    return spvPopCount4(ballot & mask);
}

fragment main0_out main0(uint gl_SubgroupInvocationID [[thread_index_in_simdgroup]], uint gl_SubgroupSize [[threads_per_simdgroup]])
{
    main0_out out = {};
    out.FragColor.x = (((spvSubgroupBallotFindLSB(uint4(1u, 2u, 3u, 4u), gl_SubgroupSize) + spvSubgroupBallotFindMSB(uint4(1u, 2u, 3u, 4u), gl_SubgroupSize)) + spvSubgroupBallotBitCount(uint4(1u, 2u, 3u, 4u), gl_SubgroupSize)) + spvSubgroupBallotInclusiveBitCount(uint4(1u, 2u, 3u, 4u), gl_SubgroupInvocationID)) + spvSubgroupBallotExclusiveBitCount(uint4(1u, 2u, 3u, 4u), gl_SubgroupInvocationID);
    return out;
}

