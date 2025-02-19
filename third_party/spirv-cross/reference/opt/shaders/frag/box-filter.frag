#version 450
#extension GL_QCOM_image_processing : require

layout(binding = 4) uniform sampler2D tex_samp;
uniform sampler2D SPIRV_Cross_Combinedtex2D_src1samp;

layout(location = 0) out vec4 fragColor;
layout(location = 0) in vec4 v_texcoord;

void main()
{
    vec2 boxSize = vec2(2.5, 4.5);
    vec4 _31 = textureBoxFilterQCOM(SPIRV_Cross_Combinedtex2D_src1samp, v_texcoord.xy, vec2(2.5, 4.5));
    fragColor = _31;
    vec4 _38 = textureBoxFilterQCOM(tex_samp, v_texcoord.xy, vec2(2.5, 4.5));
    fragColor = _38;
}

