#version 450

#extension GL_EXT_shader_explicit_arithmetic_types: enable
#extension GL_EXT_integer_dot_product: enable

int32_t i32 = 0;
int16_t i16 = int16_t(0);
uint32_t ui32 = 0;
uint16_t ui16 = uint16_t(0);
int64_t i64 = int64_t(0);
uint64_t ui64 = uint64_t(0);


void overload_errors()
{
    dotPacked4x8EXT(u8vec4(0),i32);
    dotPacked4x8EXT(i32,i64);
    dotPacked4x8EXT(i64,i32);
    dotPacked4x8AccSatEXT(u8vec4(0),i32,i32);
    dotPacked4x8AccSatEXT(i32,i32,i64);
    dotEXT(i32,i32);
    dotAccSatEXT(i32,i32,i32);

}

void main (void)
{
}
