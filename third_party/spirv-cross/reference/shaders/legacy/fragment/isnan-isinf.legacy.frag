#version 100
precision mediump float;
precision highp int;

varying highp float scalar;
varying highp vec4 vector;

void main()
{
    gl_FragData[0] = vec4(0.0);
    if (scalar != scalar)
    {
        gl_FragData[0].x = 1.0;
    }
    if (scalar != 0.0 && 2.0 * scalar == scalar)
    {
        gl_FragData[0].y = 1.0;
    }
    if (any(notEqual(vector, vector)))
    {
        gl_FragData[0].z = 1.0;
    }
    if (any(bvec4(vector.x != 0.0 && 2.0 * vector.x == vector.x, vector.y != 0.0 && 2.0 * vector.y == vector.y, vector.z != 0.0 && 2.0 * vector.z == vector.z, vector.w != 0.0 && 2.0 * vector.w == vector.w)))
    {
        gl_FragData[0].w = 1.0;
    }
}

