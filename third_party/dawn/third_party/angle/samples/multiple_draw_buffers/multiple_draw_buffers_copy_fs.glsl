precision mediump float;
varying vec2 v_texCoord;
uniform sampler2D s_texture;
void main()
{
    vec4 color = texture2D(s_texture, v_texCoord);
    gl_FragColor = color;
}
