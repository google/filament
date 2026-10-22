#version 320 es
precision highp float;
out vec4 my_FragColor;
void main()
{
    // GLSL ES 3.00.6 section 4.1.4 Floats:
    // "A value with a magnitude too small to be represented as a mantissa and exponent is converted to zero."
    // 1.0e-50 is small enough that it can't even be stored as subnormal.
    float correct = (1.0e-50 == 0.0) ? 1.0 : 0.0;
    float correct1 = isinf(1.0e40) ? 1.0 : 0.0;
    vec4 foo = vec4(1.0e-50, -1.0e-50, 1.0e50, -1.0e50);
    my_FragColor = vec4(0.0, correct, correct1, 1.0);
}
