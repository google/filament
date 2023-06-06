#version 450
#extension GL_EXT_shader_explicit_arithmetic_types_int8 : require
#extension GL_EXT_shader_explicit_arithmetic_types_int16 : require

layout(location = 0) flat in ivec4 vColor;
layout(location = 0) out ivec4 FragColorInt;
layout(location = 1) out uvec4 FragColorUint;

layout(push_constant, std140) uniform Push
{
	int8_t i8;
	uint8_t u8;
} registers;

layout(binding = 1, std140) uniform UBO
{
	int8_t i8;
	uint8_t u8;
} ubo;

layout(binding = 2, std430) buffer SSBO
{
	int8_t i8[16];
	uint8_t u8[16];
} ssbo;

void packing_int8()
{
	int16_t i16 = 10s;
	int i32 = 20;

	i8vec2 i8_2 = unpack8(i16);
	i8vec4 i8_4 = unpack8(i32);
	i16 = pack16(i8_2);
	i32 = pack32(i8_4);
	ssbo.i8[0] = i8_4.x;
	ssbo.i8[1] = i8_4.y;
	ssbo.i8[2] = i8_4.z;
	ssbo.i8[3] = i8_4.w;
}

void packing_uint8()
{
	uint16_t u16 = 10us;
	uint u32 = 20u;

	u8vec2 u8_2 = unpack8(u16);
	u8vec4 u8_4 = unpack8(u32);
	u16 = pack16(u8_2);
	u32 = pack32(u8_4);

	ssbo.u8[0] = u8_4.x;
	ssbo.u8[1] = u8_4.y;
	ssbo.u8[2] = u8_4.z;
	ssbo.u8[3] = u8_4.w;
}

void compute_int8()
{
	i8vec4 tmp = i8vec4(vColor);
	tmp += registers.i8;
	tmp += int8_t(-40);
	tmp += i8vec4(-50);
	tmp += i8vec4(10, 20, 30, 40);
	tmp += ssbo.i8[4];
	tmp += ubo.i8;
	FragColorInt = ivec4(tmp);
}

void compute_uint8()
{
	u8vec4 tmp = u8vec4(vColor);
	tmp += registers.u8;
	tmp += uint8_t(-40);
	tmp += u8vec4(-50);
	tmp += u8vec4(10, 20, 30, 40);
	tmp += ssbo.u8[4];
	tmp += ubo.u8;
	FragColorUint = uvec4(tmp);
}

void main()
{
	packing_int8();
	packing_uint8();
	compute_int8();
	compute_uint8();
}
