#version 450
uniform params {
    mat4 mvp;
    float psize;
};

layout(location = 0) in vec4 position;
layout(location = 1) in vec4 color0;
layout(location = 0) out vec4 color;

void main() {
    gl_Position = mvp * position;
    gl_PointSize = psize;
    color = color0;
}
