#version 450
layout(quads, ccw, equal_spacing) in;

void main()
{
    gl_Position = vec4(float(gl_PatchVerticesIn), 0.0, 0.0, 1.0);
}

