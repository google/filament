#version 450
#extension GL_ARB_gpu_shader_int64 : require

layout(location = 0) out vec3 FragColor;
layout(location = 0) flat in double vTmp;

void main()
{
    FragColor = vec3(dvec3(uint64BitsToDouble(0x7ff0000000000000ul), uint64BitsToDouble(0xfff0000000000000ul), uint64BitsToDouble(0x7ff8000000000000ul)) + dvec3(vTmp));
}

