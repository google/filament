#version 450
layout(quads, ccw, equal_spacing) in;

vec4 read_patch_vertices()
{
    return vec4(float(gl_PatchVerticesIn), 0.0, 0.0, 1.0);
}

void main()
{
    gl_Position = read_patch_vertices();
}

