#version 150
#extension GL_NV_gpu_shader5 : enable
#extension GL_ARB_gpu_shader_fp64 : enable

// Testing storage interpolation qualifiers sample in/ sample out
sample in vec4 colorSampIn[3];
sample out vec4 colorSampOut;

// Testing geometry layout qualifiers 
// input qualifier: invocations
// output qualifier: stream
layout(triangles, invocations=6) in;
layout(points, stream=0) out;

in vec4 color[3];
out vec3 Ocolor;

layout(stream=1) out;             // default is now stream 1

// Test explicitly sized input variables
in int8_t    var1[3];
in int16_t   var5[3];
in int32_t   var9[3];
in int64_t   var13[3];
in float16_t var33[3];
in float64_t var41[3];
in double var42[3];

out int8_t    var11;
out int16_t   var51;
out int32_t   var91;
out int64_t   var131;
out float16_t var331;
out float64_t var411;
out double var421;

// This test case verifies builtins enabled for geometry shader by NV_gpu_shader5
void testGeomBuiltins() 
{
    EmitStreamVertex(1);      // Geometry-only
    EndStreamPrimitive(0);    // Geometry-only

    EmitVertex();                      // Geometry-only
    EndPrimitive();                    // Geometry-only

    int i = gl_InvocationID;

    // Note: "patch" feature is not enabled as it's not used, below builtins are
    // related to patch
    int j = gl_PatchVerticesIn;
    float k = gl_TessLevelOuter[3];
    k = gl_TessLevelInner[1];

}
void main()
{
}