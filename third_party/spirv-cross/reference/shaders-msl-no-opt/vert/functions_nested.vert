#pragma clang diagnostic ignored "-Wmissing-prototypes"

#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

struct attr_desc
{
    int type;
    int attribute_size;
    int starting_offset;
    int stride;
    int swap_bytes;
    int is_volatile;
};

struct VertexBuffer
{
    float4x4 scale_offset_mat;
    uint vertex_base_index;
    int4 input_attributes[16];
};

struct VertexConstantsBuffer
{
    float4 vc[16];
};

constant float4 _295 = {};

struct main0_out
{
    float4 tc0 [[user(locn0)]];
    float4 back_color [[user(locn10)]];
    float4 gl_Position [[position]];
};

// Returns 2D texture coords corresponding to 1D texel buffer coords
uint2 spvTexelBufferCoord(uint tc)
{
    return uint2(tc % 4096, tc / 4096);
}

attr_desc fetch_desc(thread const int& location, constant VertexBuffer& v_227)
{
    int attribute_flags = v_227.input_attributes[location].w;
    attr_desc result;
    result.type = v_227.input_attributes[location].x;
    result.attribute_size = v_227.input_attributes[location].y;
    result.starting_offset = v_227.input_attributes[location].z;
    result.stride = attribute_flags & 255;
    result.swap_bytes = (attribute_flags >> 8) & 1;
    result.is_volatile = (attribute_flags >> 9) & 1;
    return result;
}

uint get_bits(thread const uint4& v, thread const int& swap)
{
    if (swap != 0)
    {
        return ((v.w | (v.z << uint(8))) | (v.y << uint(16))) | (v.x << uint(24));
    }
    return ((v.x | (v.y << uint(8))) | (v.z << uint(16))) | (v.w << uint(24));
}

float4 fetch_attr(thread const attr_desc& desc, thread const int& vertex_id, thread const texture2d<uint> input_stream)
{
    float4 result = float4(0.0, 0.0, 0.0, 1.0);
    bool reverse_order = false;
    int first_byte = (vertex_id * desc.stride) + desc.starting_offset;
    uint4 tmp;
    for (int n = 0; n < 4; n++)
    {
        if (n == desc.attribute_size)
        {
            break;
        }
        switch (desc.type)
        {
            case 0:
            {
                int _131 = first_byte;
                first_byte = _131 + 1;
                tmp.x = input_stream.read(spvTexelBufferCoord(_131)).x;
                int _138 = first_byte;
                first_byte = _138 + 1;
                tmp.y = input_stream.read(spvTexelBufferCoord(_138)).x;
                uint4 param = tmp;
                int param_1 = desc.swap_bytes;
                result[n] = float(get_bits(param, param_1));
                break;
            }
            case 1:
            {
                int _156 = first_byte;
                first_byte = _156 + 1;
                tmp.x = input_stream.read(spvTexelBufferCoord(_156)).x;
                int _163 = first_byte;
                first_byte = _163 + 1;
                tmp.y = input_stream.read(spvTexelBufferCoord(_163)).x;
                int _170 = first_byte;
                first_byte = _170 + 1;
                tmp.z = input_stream.read(spvTexelBufferCoord(_170)).x;
                int _177 = first_byte;
                first_byte = _177 + 1;
                tmp.w = input_stream.read(spvTexelBufferCoord(_177)).x;
                uint4 param_2 = tmp;
                int param_3 = desc.swap_bytes;
                result[n] = as_type<float>(get_bits(param_2, param_3));
                break;
            }
            case 2:
            {
                int _195 = first_byte;
                first_byte = _195 + 1;
                result[n] = float(input_stream.read(spvTexelBufferCoord(_195)).x);
                reverse_order = desc.swap_bytes != 0;
                break;
            }
        }
    }
    float4 _210;
    if (reverse_order)
    {
        _210 = result.wzyx;
    }
    else
    {
        _210 = result;
    }
    return _210;
}

float4 read_location(thread const int& location, constant VertexBuffer& v_227, thread uint& gl_VertexIndex, thread texture2d<uint> buff_in_2, thread texture2d<uint> buff_in_1)
{
    int param = location;
    attr_desc desc = fetch_desc(param, v_227);
    int vertex_id = gl_VertexIndex - int(v_227.vertex_base_index);
    if (desc.is_volatile != 0)
    {
        attr_desc param_1 = desc;
        int param_2 = vertex_id;
        return fetch_attr(param_1, param_2, buff_in_2);
    }
    else
    {
        attr_desc param_3 = desc;
        int param_4 = vertex_id;
        return fetch_attr(param_3, param_4, buff_in_1);
    }
}

void vs_adjust(thread float4& dst_reg0, thread float4& dst_reg1, thread float4& dst_reg7, constant VertexBuffer& v_227, thread uint& gl_VertexIndex, thread texture2d<uint> buff_in_2, thread texture2d<uint> buff_in_1, constant VertexConstantsBuffer& v_309)
{
    int param = 3;
    float4 in_diff_color = read_location(param, v_227, gl_VertexIndex, buff_in_2, buff_in_1);
    int param_1 = 0;
    float4 in_pos = read_location(param_1, v_227, gl_VertexIndex, buff_in_2, buff_in_1);
    int param_2 = 8;
    float4 in_tc0 = read_location(param_2, v_227, gl_VertexIndex, buff_in_2, buff_in_1);
    dst_reg1 = in_diff_color * v_309.vc[13];
    float4 tmp0;
    tmp0.x = float4(dot(float4(in_pos.xyz, 1.0), v_309.vc[4])).x;
    tmp0.y = float4(dot(float4(in_pos.xyz, 1.0), v_309.vc[5])).y;
    tmp0.z = float4(dot(float4(in_pos.xyz, 1.0), v_309.vc[6])).z;
    float4 tmp1;
    tmp1 = float4(in_tc0.xy.x, in_tc0.xy.y, tmp1.z, tmp1.w);
    tmp1.z = v_309.vc[15].x;
    dst_reg7.y = float4(dot(float4(tmp1.xyz, 1.0), v_309.vc[8])).y;
    dst_reg7.x = float4(dot(float4(tmp1.xyz, 1.0), v_309.vc[7])).x;
    dst_reg0.y = float4(dot(float4(tmp0.xyz, 1.0), v_309.vc[1])).y;
    dst_reg0.x = float4(dot(float4(tmp0.xyz, 1.0), v_309.vc[0])).x;
}

vertex main0_out main0(constant VertexBuffer& v_227 [[buffer(0)]], constant VertexConstantsBuffer& v_309 [[buffer(1)]], texture2d<uint> buff_in_2 [[texture(0)]], texture2d<uint> buff_in_1 [[texture(1)]], uint gl_VertexIndex [[vertex_id]])
{
    main0_out out = {};
    float4 dst_reg0 = float4(0.0, 0.0, 0.0, 1.0);
    float4 dst_reg1 = float4(0.0);
    float4 dst_reg7 = float4(0.0);
    float4 param = dst_reg0;
    float4 param_1 = dst_reg1;
    float4 param_2 = dst_reg7;
    vs_adjust(param, param_1, param_2, v_227, gl_VertexIndex, buff_in_2, buff_in_1, v_309);
    dst_reg0 = param;
    dst_reg1 = param_1;
    dst_reg7 = param_2;
    out.gl_Position = dst_reg0;
    out.back_color = dst_reg1;
    out.tc0 = dst_reg7;
    out.gl_Position *= v_227.scale_offset_mat;
    return out;
}

