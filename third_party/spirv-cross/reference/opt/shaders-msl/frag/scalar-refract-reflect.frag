#pragma clang diagnostic ignored "-Wmissing-prototypes"

#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

struct main0_out
{
    float FragColor [[color(0)]];
};

struct main0_in
{
    float3 vRefract [[user(locn0)]];
};

template<typename T>
inline T spvReflect(T i, T n)
{
    return i - T(2) * i * n * n;
}

template<typename T>
inline T spvRefract(T i, T n, T eta)
{
    T NoI = n * i;
    T NoI2 = NoI * NoI;
    T k = T(1) - eta * eta * (T(1) - NoI2);
    if (k < T(0))
    {
        return T(0);
    }
    else
    {
        return eta * i - (eta * NoI + sqrt(k)) * n;
    }
}

fragment main0_out main0(main0_in in [[stage_in]])
{
    main0_out out = {};
    out.FragColor = spvRefract(in.vRefract.x, in.vRefract.y, in.vRefract.z);
    out.FragColor += spvReflect(in.vRefract.x, in.vRefract.y);
    out.FragColor += refract(in.vRefract.xy, in.vRefract.yz, in.vRefract.z).y;
    out.FragColor += reflect(in.vRefract.xy, in.vRefract.zy).y;
    return out;
}

