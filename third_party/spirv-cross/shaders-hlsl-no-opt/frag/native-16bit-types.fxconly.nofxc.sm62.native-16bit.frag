#version 450
#extension GL_EXT_shader_explicit_arithmetic_types : require

layout(location = 0) out f16vec4 Output;
layout(location = 0) in f16vec4 Input;
layout(location = 1) out i16vec4 OutputI;
layout(location = 1) flat in i16vec4 InputI;
layout(location = 2) out u16vec4 OutputU;
layout(location = 2) flat in u16vec4 InputU;

layout(set = 0, binding = 0) buffer Buf
{
    float16_t foo0[4];
    int16_t foo1[4];
    uint16_t foo2[4];

    f16vec4 foo3[4];
    i16vec4 foo4[4];
    u16vec4 foo5[4];

    f16mat2x3 foo6[4];
    layout(row_major) f16mat2x3 foo7[4];
};

void main()
{
    int index = int(gl_FragCoord.x);
    Output = Input + float16_t(20.0);
    OutputI = InputI + int16_t(-40);
    OutputU = InputU + uint16_t(20);

    // Load 16-bit scalar.
    Output += foo0[index];
    OutputI += foo1[index];
    OutputU += foo2[index];

    // Load 16-bit vector.
    Output += foo3[index];
    OutputI += foo4[index];
    OutputU += foo5[index];

    // Load 16-bit vector from ColMajor matrix.
    Output += foo6[index][1].xyzz;

    // Load 16-bit vector from RowMajor matrix.
    Output += foo7[index][1].xyzz;

    // Load 16-bit matrix from ColMajor.
    f16mat2x3 m0 = foo6[index];
    // Load 16-bit matrix from RowMajor.
    f16mat2x3 m1 = foo7[index];

    // Store 16-bit scalar
    foo0[index] = Output.x;
    foo1[index] = OutputI.y;
    foo2[index] = OutputU.z;

    // Store 16-bit vector
    foo3[index] = Output;
    foo4[index] = OutputI;
    foo5[index] = OutputU;

    // Store 16-bit vector to ColMajor matrix.
    foo6[index][1] = Output.xyz;
    // Store 16-bit vector to RowMajor matrix.
    foo7[index][1] = Output.xyz;

    // Store 16-bit matrix to ColMajor.
    foo6[index] = f16mat2x3(Output.xyz, Output.wzy);
    // Store 16-bit matrix to RowMajor.
    foo7[index] = f16mat2x3(Output.xyz, Output.wzy);
}
