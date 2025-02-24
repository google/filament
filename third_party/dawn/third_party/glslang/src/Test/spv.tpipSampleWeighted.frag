#version 450
#extension GL_QCOM_image_processing : require

precision highp float;

// fragment shader inputs and outputs
layout (location = 0) in vec4 v_texcoord;

layout (location = 0) out vec4 fragColor;

// fragment shader resources
layout(set = 0, binding = 0) uniform texture2DArray tex2DArray_weights;
layout(set = 0, binding = 1) uniform texture2D tex2D_src1;
layout(set = 0, binding = 2) uniform texture2D tex2D_src2;
layout(set = 0, binding = 3) uniform sampler samp;
layout(set = 0, binding = 4) uniform sampler2D tex_samp;
layout(set = 0, binding = 5) uniform sampler2DArray tex_samp_array;

void main()
{

    fragColor = textureWeightedQCOM(
                    sampler2D(tex2D_src1, samp),                   // source texture
                    v_texcoord.xy,                                 // tex coords
                    sampler2DArray(tex2DArray_weights, samp));     // weight texture
    fragColor = textureWeightedQCOM(
                    tex_samp,                                      // combined source texture
                    v_texcoord.xy,                                 // tex coords
                    tex_samp_array);                               // combined weight texture

}

