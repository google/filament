#extension GL_EXT_draw_buffers : enable
precision mediump float;
varying vec2 v_texCoord;
uniform sampler2D s_texture;
void main()
{
    vec4 color = texture2D(s_texture, v_texCoord);
    gl_FragData[0] = color;
    gl_FragData[1] = vec4(1.0, 1.0, 1.0, 1.0) - color.brga;
    gl_FragData[2] = vec4(0.2, 1.0, 0.5, 1.0) * color.gbra;
    gl_FragData[3] = color.rrra;
}
