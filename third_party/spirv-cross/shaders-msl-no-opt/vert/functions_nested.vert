#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(std140, set = 0, binding = 0) uniform VertexBuffer
{
	mat4 scale_offset_mat;
	uint vertex_base_index;
    ivec4 input_attributes[16];
};
layout(set=0, binding=3) uniform usamplerBuffer buff_in_1;
layout(set=0, binding=4) uniform usamplerBuffer buff_in_2;

layout(location=10) out vec4 back_color;
layout(location=0) out vec4 tc0;

layout(std140, set=0, binding = 1) uniform VertexConstantsBuffer
{
	vec4 vc[16];
};

struct attr_desc
{
	int type;
	int attribute_size;
	int starting_offset;
	int stride;
    int swap_bytes;
    int is_volatile;
};

uint get_bits(uvec4 v, int swap)
{
	if (swap != 0) return (v.w | v.z << 8 | v.y << 16 | v.x << 24);
	return (v.x | v.y << 8 | v.z << 16 | v.w << 24);
}

vec4 fetch_attr(attr_desc desc, int vertex_id, usamplerBuffer input_stream)
{
	vec4 result = vec4(0.0f, 0.0f, 0.0f, 1.0f);
	uvec4 tmp;
	uint bits;
	bool reverse_order = false;

	int first_byte = (vertex_id * desc.stride) + desc.starting_offset;
	for (int n = 0; n < 4; n++)
	{
		if (n == desc.attribute_size) break;

		switch (desc.type)
		{
		case 0:
			//signed normalized 16-bit
			tmp.x = texelFetch(input_stream, first_byte++).x;
			tmp.y = texelFetch(input_stream, first_byte++).x;
			result[n] = get_bits(tmp, desc.swap_bytes);
			break;
		case 1:
			//float
			tmp.x = texelFetch(input_stream, first_byte++).x;
			tmp.y = texelFetch(input_stream, first_byte++).x;
			tmp.z = texelFetch(input_stream, first_byte++).x;
			tmp.w = texelFetch(input_stream, first_byte++).x;
			result[n] = uintBitsToFloat(get_bits(tmp, desc.swap_bytes));
			break;
		case 2:
			//unsigned byte
			result[n] = texelFetch(input_stream, first_byte++).x;
			reverse_order = (desc.swap_bytes != 0);
			break;
		}
	}

	return (reverse_order)? result.wzyx: result;
}

attr_desc fetch_desc(int location)
{
	attr_desc result;
	int attribute_flags = input_attributes[location].w;
	result.type = input_attributes[location].x;
	result.attribute_size = input_attributes[location].y;
	result.starting_offset = input_attributes[location].z;
	result.stride = attribute_flags & 0xFF;
    result.swap_bytes = (attribute_flags >> 8) & 0x1;
    result.is_volatile = (attribute_flags >> 9) & 0x1;
	return result;
}

vec4 read_location(int location)
{
	attr_desc desc = fetch_desc(location);

	int vertex_id = gl_VertexIndex - int(vertex_base_index);
	if (desc.is_volatile != 0)
		return fetch_attr(desc, vertex_id, buff_in_2);
	else
		return fetch_attr(desc, vertex_id, buff_in_1);
}

void vs_adjust(inout vec4 dst_reg0, inout vec4 dst_reg1, inout vec4 dst_reg7)
{
	vec4 tmp0;
	vec4 tmp1;
	vec4 in_diff_color= read_location(3);
	vec4 in_pos= read_location(0);
	vec4 in_tc0= read_location(8);
	dst_reg1 = (in_diff_color * vc[13]);
	tmp0.x = vec4(dot(vec4(in_pos.xyzx.xyz, 1.0), vc[4])).x;
	tmp0.y = vec4(dot(vec4(in_pos.xyzx.xyz, 1.0), vc[5])).y;
	tmp0.z = vec4(dot(vec4(in_pos.xyzx.xyz, 1.0), vc[6])).z;
	tmp1.xy = in_tc0.xyxx.xy;
	tmp1.z = vc[15].xxxx.z;
	dst_reg7.y = vec4(dot(vec4(tmp1.xyzx.xyz, 1.0), vc[8])).y;
	dst_reg7.x = vec4(dot(vec4(tmp1.xyzx.xyz, 1.0), vc[7])).x;
	dst_reg0.y = vec4(dot(vec4(tmp0.xyzx.xyz, 1.0), vc[1])).y;
	dst_reg0.x = vec4(dot(vec4(tmp0.xyzx.xyz, 1.0), vc[0])).x;
}

void main ()
{
	vec4 dst_reg0= vec4(0.0f, 0.0f, 0.0f, 1.0f);
	vec4 dst_reg1= vec4(0.0, 0.0, 0.0, 0.0);
	vec4 dst_reg7= vec4(0.0, 0.0, 0.0, 0.0);

	vs_adjust(dst_reg0, dst_reg1, dst_reg7);

	gl_Position = dst_reg0;
	back_color = dst_reg1;
	tc0 = dst_reg7;
	gl_Position = gl_Position * scale_offset_mat;
}

