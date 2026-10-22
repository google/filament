#pragma clang diagnostic ignored "-Wmissing-prototypes"

#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

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

struct main0_out
{
    uint FragColor [[color(0)]];
};

struct main0_in
{
    int index [[user(locn0)]];
};

fragment main0_out main0(main0_in in [[stage_in]], uint gl_SubgroupInvocationID [[thread_index_in_simdgroup]])
{
    main0_out out = {};
    uint _17 = uint(in.index);
    out.FragColor = uint(simd_min(in.index));
    out.FragColor = uint(simd_max(int(_17)));
    out.FragColor = simd_min(uint(in.index));
    out.FragColor = simd_max(_17);
    out.FragColor = uint(spvClustered_min<4>(in.index, gl_SubgroupInvocationID));
    out.FragColor = uint(spvClustered_max<4>(int(_17), gl_SubgroupInvocationID));
    out.FragColor = spvClustered_min<4>(uint(in.index), gl_SubgroupInvocationID);
    out.FragColor = spvClustered_max<4>(_17, gl_SubgroupInvocationID);
    return out;
}

