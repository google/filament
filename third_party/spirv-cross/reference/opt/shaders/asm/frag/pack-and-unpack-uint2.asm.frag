#version 450
#extension GL_ARB_gpu_shader_int64 : require

layout(location = 0) out vec4 FragColor;

void main()
{
    uvec2 unpacked = unpackUint2x32(packUint2x32(uvec2(18u, 52u)));
    FragColor = vec4(float(unpacked.x), float(unpacked.y), 1.0, 1.0);
}

