#version 450
#extension GL_QCOM_image_processing  : require
#extension GL_QCOM_image_processing2 : require

precision highp float;

// fragment shader inputs and outputs
layout (location = 0) in vec4 v_texcoord;

layout (location = 0) out vec4 fragColor;

// fragment shader resources
layout(set = 0, binding = 0) uniform texture2DArray tex2DArray_weights;
layout(set = 0, binding = 1) uniform texture2D tex2D_src1;
layout(set = 0, binding = 2) uniform texture2D tex2D_src2;
layout(set = 0, binding = 3) uniform sampler samp;
layout(set = 0, binding = 4) uniform sampler2D target_samp;
layout(set = 0, binding = 5) uniform sampler2D ref_samp;

void main()
{

    uvec2 tgt_coords; tgt_coords.x = uint(v_texcoord.x); tgt_coords.x = uint(v_texcoord.y);
    uvec2 ref_coords; ref_coords.x = uint(v_texcoord.z); ref_coords.y = uint(v_texcoord.w);
    uvec2 blockSize = uvec2(4, 4);
    fragColor = textureBlockMatchWindowSSDQCOM(
                    sampler2D(tex2D_src1, samp),                   // target texture
                    tgt_coords,                                    // target coords
                    sampler2D(tex2D_src2, samp),                   // reference texture
                    ref_coords,                                    // reference coords
                    blockSize);                                    // block size
    fragColor = textureBlockMatchWindowSSDQCOM(
                    target_samp,                                   // target texture
                    tgt_coords,                                    // target coords
                    ref_samp,                                      // reference texture
                    ref_coords,                                    // reference coords
                    blockSize);                                    // block size
}


