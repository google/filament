#version 450

layout(points) in;
layout(triangle_strip, max_vertices = 3) out;

layout(location = 0) in vec4 vPositionIn[1];
layout(location = 1) in vec3 vColorIn[1];

layout(location = 0) out vec3 gColor;

void main() {
    vec4 center = vPositionIn[0];
    vec3 color = vColorIn[0];

    gColor = color;
    gl_Position = center + vec4(-0.1, -0.1, 0.0, 0.0);
    EmitVertex();

    gColor = color;
    gl_Position = center + vec4(0.1, -0.1, 0.0, 0.0);
    EmitVertex();

    gColor = color;
    gl_Position = center + vec4(0.0, 0.1, 0.0, 0.0);
    EmitVertex();

    EndPrimitive();
}