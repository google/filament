uniform mediump vec4 fColor;
void main()
{
    gl_FragColor = vec4(fColor.r, fColor.g, fColor.b, fColor.a);
}
