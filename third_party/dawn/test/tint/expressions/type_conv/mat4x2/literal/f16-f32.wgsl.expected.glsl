#version 310 es

mat4x2 u = mat4x2(vec2(1.0f, 2.0f), vec2(3.0f, 4.0f), vec2(5.0f, 6.0f), vec2(7.0f, 8.0f));
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
}
