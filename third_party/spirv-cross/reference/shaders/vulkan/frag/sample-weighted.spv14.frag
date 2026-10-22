#version 450
#extension GL_QCOM_image_processing : require

layout(binding = 4) uniform sampler2D tex_samp;
layout(binding = 5) uniform sampler2DArray tex_samp_array;
uniform sampler2D SPIRV_Cross_Combinedtex2D_src1samp;
uniform sampler2DArray SPIRV_Cross_Combinedtex2DArray_weightssamp;

layout(location = 0) out vec4 fragColor;
layout(location = 0) in vec4 v_texcoord;

void main()
{
    vec4 _32 = textureWeightedQCOM(SPIRV_Cross_Combinedtex2D_src1samp, v_texcoord.xy, SPIRV_Cross_Combinedtex2DArray_weightssamp);
    fragColor = _32;
    vec4 _41 = textureWeightedQCOM(tex_samp, v_texcoord.xy, tex_samp_array);
    fragColor = _41;
}

