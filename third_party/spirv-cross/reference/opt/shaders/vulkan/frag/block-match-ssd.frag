#version 450
#extension GL_QCOM_image_processing : require

layout(binding = 4) uniform sampler2D target_samp;
layout(binding = 5) uniform sampler2D ref_samp;
uniform sampler2D SPIRV_Cross_Combinedtex2D_src1samp;
uniform sampler2D SPIRV_Cross_Combinedtex2D_src2samp;

layout(location = 0) in vec4 v_texcoord;
layout(location = 0) out vec4 fragColor;

void main()
{
    uvec2 tgt_coords;
    tgt_coords.x = uint(v_texcoord.x);
    tgt_coords.x = uint(v_texcoord.y);
    uvec2 ref_coords;
    ref_coords.x = uint(v_texcoord.z);
    ref_coords.y = uint(v_texcoord.w);
    uvec2 blockSize = uvec2(4u);
    vec4 _59 = textureBlockMatchSSDQCOM(SPIRV_Cross_Combinedtex2D_src1samp, tgt_coords, SPIRV_Cross_Combinedtex2D_src2samp, ref_coords, uvec2(4u));
    fragColor = _59;
    vec4 _68 = textureBlockMatchSSDQCOM(target_samp, tgt_coords, ref_samp, ref_coords, uvec2(4u));
    fragColor = _68;
}

