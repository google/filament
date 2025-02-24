#version 450
#extension GL_EXT_shader_explicit_arithmetic_types: enable
#extension GL_EXT_integer_dot_product: enable

int32_t i32 = 0;
int16_t i16 = int16_t(0);
uint32_t ui32 = 0;
uint16_t ui16 = uint16_t(0);
int64_t i64 = int64_t(0);
uint64_t ui64 = uint64_t(0);

void main (void)
{
    // DotProductInput4x8BitPackedKHR
    dotPacked4x8EXT(i32,i32);
    dotPacked4x8EXT(ui32,ui32);
    dotPacked4x8EXT(ui32,i32);
    dotPacked4x8EXT(i32,ui32);
    dotPacked4x8AccSatEXT(i32,i32,i32);
    dotPacked4x8AccSatEXT(ui32,ui32,ui32);
    dotPacked4x8AccSatEXT(ui32,i32,i32);
    dotPacked4x8AccSatEXT(i32,ui32,i32);

    // 8bit vec4
    dotEXT(i8vec4(0),i8vec4(0));
    dotEXT(u8vec4(0),u8vec4(0));
    dotEXT(u8vec4(0),i8vec4(0));
    dotEXT(i8vec4(0),u8vec4(0));
    dotAccSatEXT(i8vec4(0),i8vec4(0),i32);
    dotAccSatEXT(u8vec4(0),u8vec4(0),ui32);
    dotAccSatEXT(u8vec4(0),i8vec4(0),i32);
    dotAccSatEXT(i8vec4(0),u8vec4(0),i32);

    // 8bit vec3
    dotEXT(i8vec3(0),i8vec3(0));
    dotEXT(u8vec3(0),u8vec3(0));
    dotEXT(u8vec3(0),i8vec3(0));
    dotEXT(i8vec3(0),u8vec3(0));
    dotAccSatEXT(i8vec3(0),i8vec3(0),i32);
    dotAccSatEXT(u8vec3(0),u8vec3(0),ui32);
    dotAccSatEXT(u8vec3(0),i8vec3(0),i32);
    dotAccSatEXT(i8vec3(0),u8vec3(0),i32);

    // 8bit vec2
    dotEXT(i8vec2(0),i8vec2(0));
    dotEXT(u8vec2(0),u8vec2(0));
    dotEXT(u8vec2(0),i8vec2(0));
    dotEXT(i8vec2(0),u8vec2(0));
    dotAccSatEXT(i8vec2(0),i8vec2(0),i32);
    dotAccSatEXT(u8vec2(0),u8vec2(0),ui32);
    dotAccSatEXT(u8vec2(0),i8vec2(0),i32);
    dotAccSatEXT(i8vec2(0),u8vec2(0),i32);

    // 16bit vec4
    dotEXT(i16vec4(0),i16vec4(0));
    dotEXT(u16vec4(0),u16vec4(0));
    dotEXT(i16vec4(0),u16vec4(0));
    dotEXT(u16vec4(0),i16vec4(0));
    dotAccSatEXT(i16vec4(0),i16vec4(0),i32);
    dotAccSatEXT(u16vec4(0),u16vec4(0),ui32);
    dotAccSatEXT(i16vec4(0),u16vec4(0),i32);
    dotAccSatEXT(u16vec4(0),i16vec4(0),i32);

    // 16bit vec3
    dotEXT(i16vec3(0),i16vec3(0));
    dotEXT(u16vec3(0),u16vec3(0));
    dotEXT(i16vec3(0),u16vec3(0));
    dotEXT(u16vec3(0),i16vec3(0));
    dotAccSatEXT(i16vec3(0),i16vec3(0),i32);
    dotAccSatEXT(u16vec3(0),u16vec3(0),ui32);
    dotAccSatEXT(i16vec3(0),u16vec3(0),i32);
    dotAccSatEXT(u16vec3(0),i16vec3(0),i32);

    // 16bit vec2
    dotEXT(i16vec2(0),i16vec2(0));
    dotEXT(u16vec2(0),u16vec2(0));
    dotEXT(i16vec2(0),u16vec2(0));
    dotEXT(u16vec2(0),i16vec2(0));
    dotAccSatEXT(i16vec2(0),i16vec2(0),i32);
    dotAccSatEXT(u16vec2(0),u16vec2(0),ui32);
    dotAccSatEXT(i16vec2(0),u16vec2(0),i32);
    dotAccSatEXT(u16vec2(0),i16vec2(0),i32);

    // 32bit vec4
    dotEXT(i32vec4(0),i32vec4(0));
    dotEXT(u32vec4(0),u32vec4(0));
    dotEXT(i32vec4(0),u32vec4(0));
    dotEXT(u32vec4(0),i32vec4(0));
    dotAccSatEXT(i32vec4(0),i32vec4(0),i32);
    dotAccSatEXT(u32vec4(0),u32vec4(0),ui32);
    dotAccSatEXT(i32vec4(0),u32vec4(0),i32);
    dotAccSatEXT(u32vec4(0),i32vec4(0),i32);

    // 32bit vec3
    dotEXT(i32vec3(0),i32vec3(0));
    dotEXT(u32vec3(0),u32vec3(0));
    dotEXT(i32vec3(0),u32vec3(0));
    dotEXT(u32vec3(0),i32vec3(0));
    dotAccSatEXT(i32vec3(0),i32vec3(0),i32);
    dotAccSatEXT(u32vec3(0),u32vec3(0),ui32);
    dotAccSatEXT(i32vec3(0),u32vec3(0),i32);
    dotAccSatEXT(u32vec3(0),i32vec3(0),i32);

    // 32bit vec2
    dotEXT(i32vec2(0),i32vec2(0));
    dotEXT(u32vec2(0),u32vec2(0));
    dotEXT(i32vec2(0),u32vec2(0));
    dotEXT(u32vec2(0),i32vec2(0));
    dotAccSatEXT(i32vec2(0),i32vec2(0),i32);
    dotAccSatEXT(u32vec2(0),u32vec2(0),ui32);
    dotAccSatEXT(i32vec2(0),u32vec2(0),i32);
    dotAccSatEXT(u32vec2(0),i32vec2(0),i32);

    // 64bit vec4
    dotEXT(i64vec4(0),i64vec4(0));
    dotEXT(u64vec4(0),u64vec4(0));
    dotEXT(i64vec4(0),u64vec4(0));
    dotEXT(u64vec4(0),i64vec4(0));
    dotAccSatEXT(i64vec4(0),i64vec4(0),i64);
    dotAccSatEXT(u64vec4(0),u64vec4(0),ui64);
    dotAccSatEXT(i64vec4(0),u64vec4(0),i64);
    dotAccSatEXT(u64vec4(0),i64vec4(0),i64);

    // 64bit vec3
    dotEXT(i64vec3(0),i64vec3(0));
    dotEXT(u64vec3(0),u64vec3(0));
    dotEXT(i64vec3(0),u64vec3(0));
    dotEXT(u64vec3(0),i64vec3(0));
    dotAccSatEXT(i64vec3(0),i64vec3(0),i64);
    dotAccSatEXT(u64vec3(0),u64vec3(0),ui64);
    dotAccSatEXT(i64vec3(0),u64vec3(0),i64);
    dotAccSatEXT(u64vec3(0),i64vec3(0),i64);

    // 64bit vec2
    dotEXT(i64vec2(0),i64vec2(0));
    dotEXT(u64vec2(0),u64vec2(0));
    dotEXT(i64vec2(0),u64vec2(0));
    dotEXT(u64vec2(0),i64vec2(0));
    dotAccSatEXT(i64vec2(0),i64vec2(0),i64);
    dotAccSatEXT(u64vec2(0),u64vec2(0),ui64);
    dotAccSatEXT(i64vec2(0),u64vec2(0),i64);
    dotAccSatEXT(u64vec2(0),i64vec2(0),i64);
}
