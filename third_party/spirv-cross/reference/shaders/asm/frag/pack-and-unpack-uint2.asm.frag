#version 450
#if defined(GL_ARB_gpu_shader_int64)
#extension GL_ARB_gpu_shader_int64 : require
#elif defined(GL_NV_gpu_shader5)
#extension GL_NV_gpu_shader5 : require
#else
#error No extension available for 64-bit integers.
#endif

layout(location = 0) out vec4 FragColor;

void main()
{
    uint64_t _packed = packUint2x32(uvec2(18u, 52u));
    uvec2 unpacked = unpackUint2x32(_packed);
    FragColor = vec4(float(unpacked.x), float(unpacked.y), 1.0, 1.0);
}

