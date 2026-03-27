#version 450 core
#extension GL_KHR_memory_scope_semantics : enable
#extension GL_EXT_long_vector : enable
#extension GL_EXT_shader_explicit_arithmetic_types : enable
#extension GL_EXT_buffer_reference : enable

void main()
{
    vector<float, 5> vf;
    float f;
    vector<uint32_t, 5> vu;
    uint32_t u;
    vector<int32_t, 5> vi;
    int32_t i;
    vector<float16_t, 5> vf16;
    float16_t f16;
    vector<bool, 5> vb;
    bool b;

    // 8.14
    vf = dFdx(vf);
    vf = dFdy(vf);
    vf = fwidth(vf);
    vf = dFdxFine(vf);
    vf = dFdyFine(vf);
    vf = fwidthFine(vf);
    vf = dFdxCoarse(vf);
    vf = dFdyCoarse(vf);
    vf = fwidthCoarse(vf);
}