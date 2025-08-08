#pragma clang diagnostic ignored "-Wmissing-prototypes"

#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

template<typename T>
inline T spvSubgroupBroadcast(T value, ushort lane)
{
    return simd_broadcast(value, lane);
}

template<>
inline bool spvSubgroupBroadcast(bool value, ushort lane)
{
    return !!simd_broadcast((ushort)value, lane);
}

template<uint N>
inline vec<bool, N> spvSubgroupBroadcast(vec<bool, N> value, ushort lane)
{
    return (vec<bool, N>)simd_broadcast((vec<ushort, N>)value, lane);
}

template<typename T>
inline T spvSubgroupBroadcastFirst(T value)
{
    return simd_broadcast_first(value);
}

template<>
inline bool spvSubgroupBroadcastFirst(bool value)
{
    return !!simd_broadcast_first((ushort)value);
}

template<uint N>
inline vec<bool, N> spvSubgroupBroadcastFirst(vec<bool, N> value)
{
    return (vec<bool, N>)simd_broadcast_first((vec<ushort, N>)value);
}

inline uint4 spvSubgroupBallot(bool value)
{
    simd_vote vote = simd_ballot(value);
    // simd_ballot() returns a 64-bit integer-like object, but
    // SPIR-V callers expect a uint4. We must convert.
    // FIXME: This won't include higher bits if Apple ever supports
    // 128 lanes in an SIMD-group.
    return uint4(as_type<uint2>((simd_vote::vote_t)vote), 0, 0);
}

inline bool spvSubgroupBallotBitExtract(uint4 ballot, uint bit)
{
    return !!extract_bits(ballot[bit / 32], bit % 32, 1);
}

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

template<typename T>
inline bool spvSubgroupAllEqual(T value)
{
    return simd_all(all(value == simd_broadcast_first(value)));
}

template<>
inline bool spvSubgroupAllEqual(bool value)
{
    return simd_all(value) || !simd_any(value);
}

template<uint N>
inline bool spvSubgroupAllEqual(vec<bool, N> value)
{
    return simd_all(all(value == (vec<bool, N>)simd_broadcast_first((vec<ushort, N>)value)));
}

template<typename T>
inline T spvSubgroupShuffle(T value, ushort lane)
{
    return simd_shuffle(value, lane);
}

template<>
inline bool spvSubgroupShuffle(bool value, ushort lane)
{
    return !!simd_shuffle((ushort)value, lane);
}

template<uint N>
inline vec<bool, N> spvSubgroupShuffle(vec<bool, N> value, ushort lane)
{
    return (vec<bool, N>)simd_shuffle((vec<ushort, N>)value, lane);
}

template<typename T>
inline T spvSubgroupShuffleXor(T value, ushort mask)
{
    return simd_shuffle_xor(value, mask);
}

template<>
inline bool spvSubgroupShuffleXor(bool value, ushort mask)
{
    return !!simd_shuffle_xor((ushort)value, mask);
}

template<uint N>
inline vec<bool, N> spvSubgroupShuffleXor(vec<bool, N> value, ushort mask)
{
    return (vec<bool, N>)simd_shuffle_xor((vec<ushort, N>)value, mask);
}

template<typename T>
inline T spvSubgroupShuffleUp(T value, ushort delta)
{
    return simd_shuffle_up(value, delta);
}

template<>
inline bool spvSubgroupShuffleUp(bool value, ushort delta)
{
    return !!simd_shuffle_up((ushort)value, delta);
}

template<uint N>
inline vec<bool, N> spvSubgroupShuffleUp(vec<bool, N> value, ushort delta)
{
    return (vec<bool, N>)simd_shuffle_up((vec<ushort, N>)value, delta);
}

template<typename T>
inline T spvSubgroupShuffleDown(T value, ushort delta)
{
    return simd_shuffle_down(value, delta);
}

template<>
inline bool spvSubgroupShuffleDown(bool value, ushort delta)
{
    return !!simd_shuffle_down((ushort)value, delta);
}

template<uint N>
inline vec<bool, N> spvSubgroupShuffleDown(vec<bool, N> value, ushort delta)
{
    return (vec<bool, N>)simd_shuffle_down((vec<ushort, N>)value, delta);
}

template<uint N, uint offset>
struct spvClusteredAddDetail;

// Base cases
template<>
struct spvClusteredAddDetail<1, 0>
{
    template<typename T>
    static T op(T value, uint)
    {
        return value;
    }
};

template<uint offset>
struct spvClusteredAddDetail<1, offset>
{
    template<typename T>
    static T op(T value, uint lid)
    {
        // If the target lane is inactive, then return identity.
        if (!extract_bits(as_type<uint2>((simd_vote::vote_t)simd_active_threads_mask())[(lid ^ offset) / 32], (lid ^ offset) % 32, 1))
            return 0;
        return simd_shuffle_xor(value, offset);
    }
};

template<>
struct spvClusteredAddDetail<4, 0>
{
    template<typename T>
    static T op(T value, uint)
    {
        return quad_sum(value);
    }
};

template<uint offset>
struct spvClusteredAddDetail<4, offset>
{
    template<typename T>
    static T op(T value, uint lid)
    {
        // Here, we care if any of the lanes in the quad are active.
        uint quad_mask = extract_bits(as_type<uint2>((simd_vote::vote_t)simd_active_threads_mask())[(lid ^ offset) / 32], ((lid ^ offset) % 32) & ~3, 4);
        if (!quad_mask)
            return 0;
        // But we need to make sure we shuffle from an active lane.
        return simd_shuffle(quad_sum(value), ((lid ^ offset) & ~3) | ctz(quad_mask));
    }
};

// General case
template<uint N, uint offset>
struct spvClusteredAddDetail
{
    template<typename T>
    static T op(T value, uint lid)
    {
        return spvClusteredAddDetail<N/2, offset>::op(value, lid) + spvClusteredAddDetail<N/2, offset + N/2>::op(value, lid);
    }
};

template<uint N, typename T>
T spvClustered_sum(T value, uint lid)
{
    return spvClusteredAddDetail<N, 0>::op(value, lid);
}

template<uint N, uint offset>
struct spvClusteredMulDetail;

// Base cases
template<>
struct spvClusteredMulDetail<1, 0>
{
    template<typename T>
    static T op(T value, uint)
    {
        return value;
    }
};

template<uint offset>
struct spvClusteredMulDetail<1, offset>
{
    template<typename T>
    static T op(T value, uint lid)
    {
        // If the target lane is inactive, then return identity.
        if (!extract_bits(as_type<uint2>((simd_vote::vote_t)simd_active_threads_mask())[(lid ^ offset) / 32], (lid ^ offset) % 32, 1))
            return 1;
        return simd_shuffle_xor(value, offset);
    }
};

template<>
struct spvClusteredMulDetail<4, 0>
{
    template<typename T>
    static T op(T value, uint)
    {
        return quad_product(value);
    }
};

template<uint offset>
struct spvClusteredMulDetail<4, offset>
{
    template<typename T>
    static T op(T value, uint lid)
    {
        // Here, we care if any of the lanes in the quad are active.
        uint quad_mask = extract_bits(as_type<uint2>((simd_vote::vote_t)simd_active_threads_mask())[(lid ^ offset) / 32], ((lid ^ offset) % 32) & ~3, 4);
        if (!quad_mask)
            return 1;
        // But we need to make sure we shuffle from an active lane.
        return simd_shuffle(quad_product(value), ((lid ^ offset) & ~3) | ctz(quad_mask));
    }
};

// General case
template<uint N, uint offset>
struct spvClusteredMulDetail
{
    template<typename T>
    static T op(T value, uint lid)
    {
        return spvClusteredMulDetail<N/2, offset>::op(value, lid) * spvClusteredMulDetail<N/2, offset + N/2>::op(value, lid);
    }
};

template<uint N, typename T>
T spvClustered_product(T value, uint lid)
{
    return spvClusteredMulDetail<N, 0>::op(value, lid);
}

template<uint N, uint offset>
struct spvClusteredMinDetail;

// Base cases
template<>
struct spvClusteredMinDetail<1, 0>
{
    template<typename T>
    static T op(T value, uint)
    {
        return value;
    }
};

template<uint offset>
struct spvClusteredMinDetail<1, offset>
{
    template<typename T>
    static T op(T value, uint lid)
    {
        // If the target lane is inactive, then return identity.
        if (!extract_bits(as_type<uint2>((simd_vote::vote_t)simd_active_threads_mask())[(lid ^ offset) / 32], (lid ^ offset) % 32, 1))
            return numeric_limits<T>::max();
        return simd_shuffle_xor(value, offset);
    }
};

template<>
struct spvClusteredMinDetail<4, 0>
{
    template<typename T>
    static T op(T value, uint)
    {
        return quad_min(value);
    }
};

template<uint offset>
struct spvClusteredMinDetail<4, offset>
{
    template<typename T>
    static T op(T value, uint lid)
    {
        // Here, we care if any of the lanes in the quad are active.
        uint quad_mask = extract_bits(as_type<uint2>((simd_vote::vote_t)simd_active_threads_mask())[(lid ^ offset) / 32], ((lid ^ offset) % 32) & ~3, 4);
        if (!quad_mask)
            return numeric_limits<T>::max();
        // But we need to make sure we shuffle from an active lane.
        return simd_shuffle(quad_min(value), ((lid ^ offset) & ~3) | ctz(quad_mask));
    }
};

// General case
template<uint N, uint offset>
struct spvClusteredMinDetail
{
    template<typename T>
    static T op(T value, uint lid)
    {
        return min(spvClusteredMinDetail<N/2, offset>::op(value, lid), spvClusteredMinDetail<N/2, offset + N/2>::op(value, lid));
    }
};

template<uint N, typename T>
T spvClustered_min(T value, uint lid)
{
    return spvClusteredMinDetail<N, 0>::op(value, lid);
}

template<uint N, uint offset>
struct spvClusteredMaxDetail;

// Base cases
template<>
struct spvClusteredMaxDetail<1, 0>
{
    template<typename T>
    static T op(T value, uint)
    {
        return value;
    }
};

template<uint offset>
struct spvClusteredMaxDetail<1, offset>
{
    template<typename T>
    static T op(T value, uint lid)
    {
        // If the target lane is inactive, then return identity.
        if (!extract_bits(as_type<uint2>((simd_vote::vote_t)simd_active_threads_mask())[(lid ^ offset) / 32], (lid ^ offset) % 32, 1))
            return numeric_limits<T>::min();
        return simd_shuffle_xor(value, offset);
    }
};

template<>
struct spvClusteredMaxDetail<4, 0>
{
    template<typename T>
    static T op(T value, uint)
    {
        return quad_max(value);
    }
};

template<uint offset>
struct spvClusteredMaxDetail<4, offset>
{
    template<typename T>
    static T op(T value, uint lid)
    {
        // Here, we care if any of the lanes in the quad are active.
        uint quad_mask = extract_bits(as_type<uint2>((simd_vote::vote_t)simd_active_threads_mask())[(lid ^ offset) / 32], ((lid ^ offset) % 32) & ~3, 4);
        if (!quad_mask)
            return numeric_limits<T>::min();
        // But we need to make sure we shuffle from an active lane.
        return simd_shuffle(quad_max(value), ((lid ^ offset) & ~3) | ctz(quad_mask));
    }
};

// General case
template<uint N, uint offset>
struct spvClusteredMaxDetail
{
    template<typename T>
    static T op(T value, uint lid)
    {
        return max(spvClusteredMaxDetail<N/2, offset>::op(value, lid), spvClusteredMaxDetail<N/2, offset + N/2>::op(value, lid));
    }
};

template<uint N, typename T>
T spvClustered_max(T value, uint lid)
{
    return spvClusteredMaxDetail<N, 0>::op(value, lid);
}

template<uint N, uint offset>
struct spvClusteredAndDetail;

// Base cases
template<>
struct spvClusteredAndDetail<1, 0>
{
    template<typename T>
    static T op(T value, uint)
    {
        return value;
    }
};

template<uint offset>
struct spvClusteredAndDetail<1, offset>
{
    template<typename T>
    static T op(T value, uint lid)
    {
        // If the target lane is inactive, then return identity.
        if (!extract_bits(as_type<uint2>((simd_vote::vote_t)simd_active_threads_mask())[(lid ^ offset) / 32], (lid ^ offset) % 32, 1))
            return ~T(0);
        return simd_shuffle_xor(value, offset);
    }
};

template<>
struct spvClusteredAndDetail<4, 0>
{
    template<typename T>
    static T op(T value, uint)
    {
        return quad_and(value);
    }
};

template<uint offset>
struct spvClusteredAndDetail<4, offset>
{
    template<typename T>
    static T op(T value, uint lid)
    {
        // Here, we care if any of the lanes in the quad are active.
        uint quad_mask = extract_bits(as_type<uint2>((simd_vote::vote_t)simd_active_threads_mask())[(lid ^ offset) / 32], ((lid ^ offset) % 32) & ~3, 4);
        if (!quad_mask)
            return ~T(0);
        // But we need to make sure we shuffle from an active lane.
        return simd_shuffle(quad_and(value), ((lid ^ offset) & ~3) | ctz(quad_mask));
    }
};

// General case
template<uint N, uint offset>
struct spvClusteredAndDetail
{
    template<typename T>
    static T op(T value, uint lid)
    {
        return spvClusteredAndDetail<N/2, offset>::op(value, lid) & spvClusteredAndDetail<N/2, offset + N/2>::op(value, lid);
    }
};

template<uint N, typename T>
T spvClustered_and(T value, uint lid)
{
    return spvClusteredAndDetail<N, 0>::op(value, lid);
}

template<uint N, uint offset>
struct spvClusteredOrDetail;

// Base cases
template<>
struct spvClusteredOrDetail<1, 0>
{
    template<typename T>
    static T op(T value, uint)
    {
        return value;
    }
};

template<uint offset>
struct spvClusteredOrDetail<1, offset>
{
    template<typename T>
    static T op(T value, uint lid)
    {
        // If the target lane is inactive, then return identity.
        if (!extract_bits(as_type<uint2>((simd_vote::vote_t)simd_active_threads_mask())[(lid ^ offset) / 32], (lid ^ offset) % 32, 1))
            return 0;
        return simd_shuffle_xor(value, offset);
    }
};

template<>
struct spvClusteredOrDetail<4, 0>
{
    template<typename T>
    static T op(T value, uint)
    {
        return quad_or(value);
    }
};

template<uint offset>
struct spvClusteredOrDetail<4, offset>
{
    template<typename T>
    static T op(T value, uint lid)
    {
        // Here, we care if any of the lanes in the quad are active.
        uint quad_mask = extract_bits(as_type<uint2>((simd_vote::vote_t)simd_active_threads_mask())[(lid ^ offset) / 32], ((lid ^ offset) % 32) & ~3, 4);
        if (!quad_mask)
            return 0;
        // But we need to make sure we shuffle from an active lane.
        return simd_shuffle(quad_or(value), ((lid ^ offset) & ~3) | ctz(quad_mask));
    }
};

// General case
template<uint N, uint offset>
struct spvClusteredOrDetail
{
    template<typename T>
    static T op(T value, uint lid)
    {
        return spvClusteredOrDetail<N/2, offset>::op(value, lid) | spvClusteredOrDetail<N/2, offset + N/2>::op(value, lid);
    }
};

template<uint N, typename T>
T spvClustered_or(T value, uint lid)
{
    return spvClusteredOrDetail<N, 0>::op(value, lid);
}

template<uint N, uint offset>
struct spvClusteredXorDetail;

// Base cases
template<>
struct spvClusteredXorDetail<1, 0>
{
    template<typename T>
    static T op(T value, uint)
    {
        return value;
    }
};

template<uint offset>
struct spvClusteredXorDetail<1, offset>
{
    template<typename T>
    static T op(T value, uint lid)
    {
        // If the target lane is inactive, then return identity.
        if (!extract_bits(as_type<uint2>((simd_vote::vote_t)simd_active_threads_mask())[(lid ^ offset) / 32], (lid ^ offset) % 32, 1))
            return 0;
        return simd_shuffle_xor(value, offset);
    }
};

template<>
struct spvClusteredXorDetail<4, 0>
{
    template<typename T>
    static T op(T value, uint)
    {
        return quad_xor(value);
    }
};

template<uint offset>
struct spvClusteredXorDetail<4, offset>
{
    template<typename T>
    static T op(T value, uint lid)
    {
        // Here, we care if any of the lanes in the quad are active.
        uint quad_mask = extract_bits(as_type<uint2>((simd_vote::vote_t)simd_active_threads_mask())[(lid ^ offset) / 32], ((lid ^ offset) % 32) & ~3, 4);
        if (!quad_mask)
            return 0;
        // But we need to make sure we shuffle from an active lane.
        return simd_shuffle(quad_xor(value), ((lid ^ offset) & ~3) | ctz(quad_mask));
    }
};

// General case
template<uint N, uint offset>
struct spvClusteredXorDetail
{
    template<typename T>
    static T op(T value, uint lid)
    {
        return spvClusteredXorDetail<N/2, offset>::op(value, lid) ^ spvClusteredXorDetail<N/2, offset + N/2>::op(value, lid);
    }
};

template<uint N, typename T>
T spvClustered_xor(T value, uint lid)
{
    return spvClusteredXorDetail<N, 0>::op(value, lid);
}

template<typename T>
inline T spvQuadBroadcast(T value, uint lane)
{
    return quad_broadcast(value, lane);
}

template<>
inline bool spvQuadBroadcast(bool value, uint lane)
{
    return !!quad_broadcast((ushort)value, lane);
}

template<uint N>
inline vec<bool, N> spvQuadBroadcast(vec<bool, N> value, uint lane)
{
    return (vec<bool, N>)quad_broadcast((vec<ushort, N>)value, lane);
}

template<typename T>
inline T spvQuadSwap(T value, uint dir)
{
    return quad_shuffle_xor(value, dir + 1);
}

template<>
inline bool spvQuadSwap(bool value, uint dir)
{
    return !!quad_shuffle_xor((ushort)value, dir + 1);
}

template<uint N>
inline vec<bool, N> spvQuadSwap(vec<bool, N> value, uint dir)
{
    return (vec<bool, N>)quad_shuffle_xor((vec<ushort, N>)value, dir + 1);
}

struct main0_out
{
    float FragColor [[color(0)]];
};

fragment main0_out main0(uint gl_SubgroupSize [[threads_per_simdgroup]], uint gl_SubgroupInvocationID [[thread_index_in_simdgroup]])
{
    main0_out out = {};
    uint4 gl_SubgroupEqMask = gl_SubgroupInvocationID >= 32 ? uint4(0, (1 << (gl_SubgroupInvocationID - 32)), uint2(0)) : uint4(1 << gl_SubgroupInvocationID, uint3(0));
    uint4 gl_SubgroupGeMask = uint4(insert_bits(0u, 0xFFFFFFFF, min(gl_SubgroupInvocationID, 32u), (uint)max(min((int)gl_SubgroupSize, 32) - (int)gl_SubgroupInvocationID, 0)), insert_bits(0u, 0xFFFFFFFF, (uint)max((int)gl_SubgroupInvocationID - 32, 0), (uint)max((int)gl_SubgroupSize - (int)max(gl_SubgroupInvocationID, 32u), 0)), uint2(0));
    uint4 gl_SubgroupGtMask = uint4(insert_bits(0u, 0xFFFFFFFF, min(gl_SubgroupInvocationID + 1, 32u), (uint)max(min((int)gl_SubgroupSize, 32) - (int)gl_SubgroupInvocationID - 1, 0)), insert_bits(0u, 0xFFFFFFFF, (uint)max((int)gl_SubgroupInvocationID + 1 - 32, 0), (uint)max((int)gl_SubgroupSize - (int)max(gl_SubgroupInvocationID + 1, 32u), 0)), uint2(0));
    uint4 gl_SubgroupLeMask = uint4(extract_bits(0xFFFFFFFF, 0, min(gl_SubgroupInvocationID + 1, 32u)), extract_bits(0xFFFFFFFF, 0, (uint)max((int)gl_SubgroupInvocationID + 1 - 32, 0)), uint2(0));
    uint4 gl_SubgroupLtMask = uint4(extract_bits(0xFFFFFFFF, 0, min(gl_SubgroupInvocationID, 32u)), extract_bits(0xFFFFFFFF, 0, (uint)max((int)gl_SubgroupInvocationID - 32, 0)), uint2(0));
    out.FragColor = float(gl_SubgroupSize);
    out.FragColor = float(gl_SubgroupInvocationID);
    bool _24 = simd_is_first();
    bool elected = _24;
    out.FragColor = float4(gl_SubgroupEqMask).x;
    out.FragColor = float4(gl_SubgroupGeMask).x;
    out.FragColor = float4(gl_SubgroupGtMask).x;
    out.FragColor = float4(gl_SubgroupLeMask).x;
    out.FragColor = float4(gl_SubgroupLtMask).x;
    float4 broadcasted = spvSubgroupBroadcast(float4(10.0), 8u);
    bool2 broadcasted_bool = spvSubgroupBroadcast(bool2(true), 8u);
    float3 first = spvSubgroupBroadcastFirst(float3(20.0));
    bool4 first_bool = spvSubgroupBroadcastFirst(bool4(false));
    uint4 ballot_value = spvSubgroupBallot(true);
    bool inverse_ballot_value = spvSubgroupBallotBitExtract(ballot_value, gl_SubgroupInvocationID);
    bool bit_extracted = spvSubgroupBallotBitExtract(uint4(10u), 8u);
    uint bit_count = spvSubgroupBallotBitCount(ballot_value, gl_SubgroupSize);
    uint inclusive_bit_count = spvSubgroupBallotInclusiveBitCount(ballot_value, gl_SubgroupInvocationID);
    uint exclusive_bit_count = spvSubgroupBallotExclusiveBitCount(ballot_value, gl_SubgroupInvocationID);
    uint lsb = spvSubgroupBallotFindLSB(ballot_value, gl_SubgroupSize);
    uint msb = spvSubgroupBallotFindMSB(ballot_value, gl_SubgroupSize);
    uint shuffled = spvSubgroupShuffle(10u, 8u);
    bool shuffled_bool = spvSubgroupShuffle(true, 9u);
    uint shuffled_xor = spvSubgroupShuffleXor(30u, 8u);
    bool shuffled_xor_bool = spvSubgroupShuffleXor(false, 9u);
    uint shuffled_up = spvSubgroupShuffleUp(20u, 4u);
    bool shuffled_up_bool = spvSubgroupShuffleUp(true, 4u);
    uint shuffled_down = spvSubgroupShuffleDown(20u, 4u);
    bool shuffled_down_bool = spvSubgroupShuffleDown(false, 4u);
    bool has_all = simd_all(true);
    bool has_any = simd_any(true);
    bool has_equal = spvSubgroupAllEqual(0);
    has_equal = spvSubgroupAllEqual(true);
    has_equal = spvSubgroupAllEqual(float3(0.0, 1.0, 2.0));
    has_equal = spvSubgroupAllEqual(bool4(true, true, false, true));
    float4 added = simd_sum(float4(20.0));
    int4 iadded = simd_sum(int4(20));
    float4 multiplied = simd_product(float4(20.0));
    int4 imultiplied = simd_product(int4(20));
    float4 lo = simd_min(float4(20.0));
    float4 hi = simd_max(float4(20.0));
    int4 slo = simd_min(int4(20));
    int4 shi = simd_max(int4(20));
    uint4 ulo = simd_min(uint4(20u));
    uint4 uhi = simd_max(uint4(20u));
    uint4 anded = simd_and(ballot_value);
    uint4 ored = simd_or(ballot_value);
    uint4 xored = simd_xor(ballot_value);
    added = simd_prefix_inclusive_sum(added);
    iadded = simd_prefix_inclusive_sum(iadded);
    multiplied = simd_prefix_inclusive_product(multiplied);
    imultiplied = simd_prefix_inclusive_product(imultiplied);
    added = simd_prefix_exclusive_sum(multiplied);
    multiplied = simd_prefix_exclusive_product(multiplied);
    iadded = simd_prefix_exclusive_sum(imultiplied);
    imultiplied = simd_prefix_exclusive_product(imultiplied);
    added = spvClustered_sum<4>(added, gl_SubgroupInvocationID);
    multiplied = spvClustered_product<4>(multiplied, gl_SubgroupInvocationID);
    iadded = spvClustered_sum<4>(iadded, gl_SubgroupInvocationID);
    imultiplied = spvClustered_product<4>(imultiplied, gl_SubgroupInvocationID);
    lo = spvClustered_min<4>(lo, gl_SubgroupInvocationID);
    hi = spvClustered_max<4>(hi, gl_SubgroupInvocationID);
    ulo = spvClustered_min<4>(ulo, gl_SubgroupInvocationID);
    uhi = spvClustered_max<4>(uhi, gl_SubgroupInvocationID);
    slo = spvClustered_min<4>(slo, gl_SubgroupInvocationID);
    shi = spvClustered_max<4>(shi, gl_SubgroupInvocationID);
    anded = spvClustered_and<4>(anded, gl_SubgroupInvocationID);
    ored = spvClustered_or<4>(ored, gl_SubgroupInvocationID);
    xored = spvClustered_xor<4>(xored, gl_SubgroupInvocationID);
    float4 swap_horiz = spvQuadSwap(float4(20.0), 0u);
    bool4 swap_horiz_bool = spvQuadSwap(bool4(true), 0u);
    float4 swap_vertical = spvQuadSwap(float4(20.0), 1u);
    bool4 swap_vertical_bool = spvQuadSwap(bool4(true), 1u);
    float4 swap_diagonal = spvQuadSwap(float4(20.0), 2u);
    bool4 swap_diagonal_bool = spvQuadSwap(bool4(true), 2u);
    float4 quad_broadcast0 = spvQuadBroadcast(float4(20.0), 3u);
    bool4 quad_broadcast_bool = spvQuadBroadcast(bool4(true), 3u);
    return out;
}

