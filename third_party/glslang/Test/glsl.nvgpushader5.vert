#version 150
#extension GL_NV_gpu_shader5 : enable
#extension GL_ARB_gpu_shader_fp64 : enable

// Testing storage qualifiers sample in/ sample out
in vec4 colorSampIn; // interpolation qualifier are not allowed on input vertex data
sample out vec4 colorSampOut;


// Test explicitly sized input variables
in int8_t    var1;
in int16_t   var5;
in int32_t   var9;
in int64_t   var13;
in float16_t var33;
in float64_t var41;
in double var42;

out int8_t    var11;
out int16_t   var51;
out int32_t   var91;
out int64_t   var131;
out float16_t var331;
out float64_t var411;
out double var421;

void main()
{

}