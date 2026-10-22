#version 450
#if defined(GL_ARB_gpu_shader_int64)
#extension GL_ARB_gpu_shader_int64 : require
#elif defined(GL_NV_gpu_shader5)
#extension GL_NV_gpu_shader5 : require
#else
#error No extension available for 64-bit integers.
#endif

layout(location = 0) out vec3 FragColor;
layout(location = 0) flat in double vTmp;

void main()
{
    FragColor = vec3(dvec3(uint64BitsToDouble(0x7ff0000000000000ul /* inf */), uint64BitsToDouble(0xfff0000000000000ul /* -inf */), uint64BitsToDouble(0x7ff8000000000000ul /* nan */)) + dvec3(vTmp));
}

