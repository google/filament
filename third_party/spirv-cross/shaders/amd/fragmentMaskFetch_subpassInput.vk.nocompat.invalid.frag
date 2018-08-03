#version 450
#extension GL_AMD_shader_fragment_mask : require

layout(input_attachment_index = 0, binding = 0) uniform subpassInputMS t;

void main ()
{
    vec4 test2 = fragmentFetchAMD(t, 4);
    uint testi2 = fragmentMaskFetchAMD(t);
}
