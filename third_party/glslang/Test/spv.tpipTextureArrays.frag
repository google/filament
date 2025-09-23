#version 450
#extension GL_QCOM_image_processing : require

precision highp float;

// fragment shader inputs and outputs
layout (location = 0) in vec4 v_texcoord;

layout (location = 0) out vec4 fragColor;

// fragment shader resources
layout(set = 0, binding = 3) uniform sampler samp;


layout(set = 0, binding = 4) uniform texture2D tex2D_srcs[8];
layout(set = 0, binding = 5) uniform sampler2D samplers[3];

void main()
{

    uvec2 tgt_coords; tgt_coords.x = uint(v_texcoord.x); tgt_coords.x = uint(v_texcoord.y);
    uvec2 ref_coords; ref_coords.x = uint(v_texcoord.z); ref_coords.y = uint(v_texcoord.w);
    uvec2 blockSize = uvec2(4, 4);
    uint  ii = tgt_coords.x % 8;
    fragColor = textureBlockMatchSSDQCOM(
                    samplers[0],                                   // target texture
                    tgt_coords,                                    // target coords
                    sampler2D(tex2D_srcs[ii], samp),               // reference texture
                    ref_coords,                                    // reference coords
                    blockSize);                                    // block size

    fragColor = textureBlockMatchSADQCOM(
                    sampler2D(tex2D_srcs[1], samp),                // target texture
                    tgt_coords,                                    // target coords
                    samplers[1],                                   // reference texture
                    ref_coords,                                    // reference coords
                    blockSize);                                    // block size

}

