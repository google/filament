#version 100
#extension GL_EXT_shader_texture_lod : require
precision mediump float;
precision highp int;

uniform mediump sampler2D tex;

void main()
{
    gl_FragData[0] = texture2DLodEXT(tex, vec2(0.4000000059604644775390625, 0.60000002384185791015625), 0.0);
}

