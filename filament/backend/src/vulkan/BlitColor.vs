#version 320 es

layout(set = 0, binding = 0, std140) uniform ParamsBlock {
    mat4 tmp;
} params;

layout(location = 0) in vec4 pos;

void main() {
    gl_Position = pos;
}
