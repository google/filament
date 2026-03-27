#version 460

const vec4 constGlobal = vec4(1.0, 0.0, 0.0, 1.0);

out vec4 outColor;

void main() {
    const float constLocal = 3.14;
    outColor = constGlobal * constLocal;
}