#pragma clang diagnostic ignored "-Wmissing-prototypes"
#pragma clang diagnostic ignored "-Wmissing-braces"

#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

template<typename T, size_t Num>
struct spvUnsafeArray
{
    T elements[Num ? Num : 1];
    
    thread T& operator [] (size_t pos) thread
    {
        return elements[pos];
    }
    constexpr const thread T& operator [] (size_t pos) const thread
    {
        return elements[pos];
    }
    
    device T& operator [] (size_t pos) device
    {
        return elements[pos];
    }
    constexpr const device T& operator [] (size_t pos) const device
    {
        return elements[pos];
    }
    
    constexpr const constant T& operator [] (size_t pos) const constant
    {
        return elements[pos];
    }
    
    threadgroup T& operator [] (size_t pos) threadgroup
    {
        return elements[pos];
    }
    constexpr const threadgroup T& operator [] (size_t pos) const threadgroup
    {
        return elements[pos];
    }
};

struct _13
{
    float4 x;
    float4 y;
    float4 z;
    spvUnsafeArray<float4, 2> u;
    spvUnsafeArray<float2, 2> v;
    spvUnsafeArray<float, 3> w;
};

struct main0_out
{
    float4 FragColor [[color(0)]];
};

struct main0_in
{
    interpolant<float4, interpolation::no_perspective> foo [[user(locn0)]];
    interpolant<float3, interpolation::perspective> bar [[user(locn1)]];
    interpolant<float2, interpolation::perspective> baz [[user(locn2)]];
    int sid [[user(locn3)]];
    interpolant<float2, interpolation::perspective> a_0 [[user(locn4)]];
    interpolant<float2, interpolation::perspective> a_1 [[user(locn5)]];
    interpolant<float2, interpolation::perspective> b_0 [[user(locn6)]];
    interpolant<float2, interpolation::perspective> b_1 [[user(locn7)]];
    interpolant<float2, interpolation::perspective> c_0 [[user(locn8)]];
    interpolant<float2, interpolation::perspective> c_1 [[user(locn9)]];
    interpolant<float4, interpolation::perspective> s_x [[user(locn10)]];
    interpolant<float4, interpolation::no_perspective> s_y [[user(locn11)]];
    interpolant<float4, interpolation::perspective> s_z [[user(locn12)]];
    interpolant<float4, interpolation::perspective> s_u_0 [[user(locn13)]];
    interpolant<float4, interpolation::perspective> s_u_1 [[user(locn14)]];
    interpolant<float2, interpolation::no_perspective> s_v_0 [[user(locn15)]];
    interpolant<float2, interpolation::no_perspective> s_v_1 [[user(locn16)]];
    interpolant<float, interpolation::perspective> s_w_0 [[user(locn17)]];
    interpolant<float, interpolation::perspective> s_w_1 [[user(locn18)]];
    interpolant<float, interpolation::perspective> s_w_2 [[user(locn19)]];
};

fragment main0_out main0(main0_in in [[stage_in]], uint gl_SampleID [[sample_id]])
{
    main0_out out = {};
    spvUnsafeArray<float2, 2> a = {};
    _13 s = {};
    spvUnsafeArray<float2, 2> b = {};
    spvUnsafeArray<float2, 2> c = {};
    a[0] = in.a_0.interpolate_at_center();
    a[1] = in.a_1.interpolate_at_center();
    s.x = in.s_x.interpolate_at_center();
    s.y = in.s_y.interpolate_at_centroid();
    s.z = in.s_z.interpolate_at_sample(gl_SampleID);
    s.u[0] = in.s_u_0.interpolate_at_centroid();
    s.u[1] = in.s_u_1.interpolate_at_centroid();
    s.v[0] = in.s_v_0.interpolate_at_sample(gl_SampleID);
    s.v[1] = in.s_v_1.interpolate_at_sample(gl_SampleID);
    s.w[0] = in.s_w_0.interpolate_at_center();
    s.w[1] = in.s_w_1.interpolate_at_center();
    s.w[2] = in.s_w_2.interpolate_at_center();
    b[0] = in.b_0.interpolate_at_centroid();
    b[1] = in.b_1.interpolate_at_centroid();
    c[0] = in.c_0.interpolate_at_sample(gl_SampleID);
    c[1] = in.c_1.interpolate_at_sample(gl_SampleID);
    out.FragColor = in.foo.interpolate_at_center();
    out.FragColor += in.foo.interpolate_at_centroid();
    out.FragColor += in.foo.interpolate_at_sample(in.sid);
    out.FragColor += in.foo.interpolate_at_offset(float2(0.100000001490116119384765625) + 0.4375);
    float3 _65 = out.FragColor.xyz + in.bar.interpolate_at_centroid();
    out.FragColor = float4(_65.x, _65.y, _65.z, out.FragColor.w);
    float3 _71 = out.FragColor.xyz + in.bar.interpolate_at_centroid();
    out.FragColor = float4(_71.x, _71.y, _71.z, out.FragColor.w);
    float3 _78 = out.FragColor.xyz + in.bar.interpolate_at_sample(in.sid);
    out.FragColor = float4(_78.x, _78.y, _78.z, out.FragColor.w);
    float3 _84 = out.FragColor.xyz + in.bar.interpolate_at_offset(float2(-0.100000001490116119384765625) + 0.4375);
    out.FragColor = float4(_84.x, _84.y, _84.z, out.FragColor.w);
    float2 _91 = out.FragColor.xy + b[0];
    out.FragColor = float4(_91.x, _91.y, out.FragColor.z, out.FragColor.w);
    float2 _98 = out.FragColor.xy + in.b_1.interpolate_at_centroid();
    out.FragColor = float4(_98.x, _98.y, out.FragColor.z, out.FragColor.w);
    float2 _105 = out.FragColor.xy + in.b_0.interpolate_at_sample(2);
    out.FragColor = float4(_105.x, _105.y, out.FragColor.z, out.FragColor.w);
    float2 _112 = out.FragColor.xy + in.b_1.interpolate_at_offset(float2(-0.100000001490116119384765625, 0.100000001490116119384765625) + 0.4375);
    out.FragColor = float4(_112.x, _112.y, out.FragColor.z, out.FragColor.w);
    float2 _119 = out.FragColor.xy + c[0];
    out.FragColor = float4(_119.x, _119.y, out.FragColor.z, out.FragColor.w);
    float2 _127 = out.FragColor.xy + in.c_1.interpolate_at_centroid().xy;
    out.FragColor = float4(_127.x, _127.y, out.FragColor.z, out.FragColor.w);
    float2 _135 = out.FragColor.xy + in.c_0.interpolate_at_sample(2).yx;
    out.FragColor = float4(_135.x, _135.y, out.FragColor.z, out.FragColor.w);
    float2 _143 = out.FragColor.xy + in.c_1.interpolate_at_offset(float2(-0.100000001490116119384765625, 0.100000001490116119384765625) + 0.4375).xx;
    out.FragColor = float4(_143.x, _143.y, out.FragColor.z, out.FragColor.w);
    out.FragColor += s.x;
    out.FragColor += in.s_x.interpolate_at_centroid();
    out.FragColor += in.s_x.interpolate_at_sample(in.sid);
    out.FragColor += in.s_x.interpolate_at_offset(float2(0.100000001490116119384765625) + 0.4375);
    out.FragColor += s.y;
    out.FragColor += in.s_y.interpolate_at_centroid();
    out.FragColor += in.s_y.interpolate_at_sample(in.sid);
    out.FragColor += in.s_y.interpolate_at_offset(float2(-0.100000001490116119384765625) + 0.4375);
    float2 _184 = out.FragColor.xy + s.v[0];
    out.FragColor = float4(_184.x, _184.y, out.FragColor.z, out.FragColor.w);
    float2 _191 = out.FragColor.xy + in.s_v_1.interpolate_at_centroid();
    out.FragColor = float4(_191.x, _191.y, out.FragColor.z, out.FragColor.w);
    float2 _198 = out.FragColor.xy + in.s_v_0.interpolate_at_sample(2);
    out.FragColor = float4(_198.x, _198.y, out.FragColor.z, out.FragColor.w);
    float2 _205 = out.FragColor.xy + in.s_v_1.interpolate_at_offset(float2(-0.100000001490116119384765625, 0.100000001490116119384765625) + 0.4375);
    out.FragColor = float4(_205.x, _205.y, out.FragColor.z, out.FragColor.w);
    out.FragColor.x += s.w[0];
    out.FragColor.x += in.s_w_1.interpolate_at_centroid();
    out.FragColor.x += in.s_w_0.interpolate_at_sample(2);
    out.FragColor.x += in.s_w_1.interpolate_at_offset(float2(-0.100000001490116119384765625, 0.100000001490116119384765625) + 0.4375);
    float2 _328 = out.FragColor.xy + in.baz.interpolate_at_sample(gl_SampleID);
    out.FragColor = float4(_328.x, _328.y, out.FragColor.z, out.FragColor.w);
    out.FragColor.x += in.baz.interpolate_at_centroid().x;
    out.FragColor.y += in.baz.interpolate_at_sample(3).y;
    out.FragColor.z += in.baz.interpolate_at_offset(float2(-0.100000001490116119384765625, 0.100000001490116119384765625) + 0.4375).y;
    float2 _353 = out.FragColor.xy + in.a_1.interpolate_at_centroid();
    out.FragColor = float4(_353.x, _353.y, out.FragColor.z, out.FragColor.w);
    float2 _360 = out.FragColor.xy + in.a_0.interpolate_at_sample(2);
    out.FragColor = float4(_360.x, _360.y, out.FragColor.z, out.FragColor.w);
    float2 _367 = out.FragColor.xy + in.a_1.interpolate_at_offset(float2(-0.100000001490116119384765625, 0.100000001490116119384765625) + 0.4375);
    out.FragColor = float4(_367.x, _367.y, out.FragColor.z, out.FragColor.w);
    out.FragColor += s.z;
    float2 _379 = out.FragColor.xy + in.s_z.interpolate_at_centroid().yy;
    out.FragColor = float4(_379.x, _379.y, out.FragColor.z, out.FragColor.w);
    float2 _387 = out.FragColor.yz + in.s_z.interpolate_at_sample(3).xy;
    out.FragColor = float4(out.FragColor.x, _387.x, _387.y, out.FragColor.w);
    float2 _395 = out.FragColor.zw + in.s_z.interpolate_at_offset(float2(-0.100000001490116119384765625, 0.100000001490116119384765625) + 0.4375).wx;
    out.FragColor = float4(out.FragColor.x, out.FragColor.y, _395.x, _395.y);
    out.FragColor += s.u[0];
    out.FragColor += in.s_u_1.interpolate_at_centroid();
    out.FragColor += in.s_u_0.interpolate_at_sample(2);
    out.FragColor += in.s_u_1.interpolate_at_offset(float2(-0.100000001490116119384765625, 0.100000001490116119384765625) + 0.4375);
    return out;
}

