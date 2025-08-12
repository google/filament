#version 450
#extension GL_ARM_tensors : require
#extension GL_EXT_shader_explicit_arithmetic_types : require

layout(location = 0) out vec4 outColor;

layout(set=0, binding=0) uniform tensorARM<uint8_t, 3> tens;

void main() {
    const uint size_d1 = tensorSizeARM(tens, 1);
    const uint size_d2 = tensorSizeARM(tens, 2);
    const uint coord_x = uint(gl_FragCoord.x) % size_d1;
    const uint coord_y = uint(gl_FragCoord.y) % size_d2;

    uint8_t tensorValue = uint8_t(0);
    tensorReadARM(tens, uint[](coord_y, coord_x, 1), tensorValue);
    outColor = vec4(0.0, tensorValue, 0.0, 255.0);
}
