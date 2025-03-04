#version 300 es
precision highp float;
out vec4 my_FragColor;
void main()
{
    // Out-of-range floats should overflow to infinity
    // GLSL ES 3.00.6 section 4.1.4 Floats:
    // "If the value of the floating point number is too large (small) to be stored as a single precision value, it is converted to positive (negative) infinity"
    float correct = isinf(1.0e2147483649) ? 1.0 : 0.0;
    my_FragColor = vec4(0.0, correct, 0.0, 1.0);
}
