#version 460

// GLSL spec: Hence, the types, initializers, and any location specifiers of all statically used uniform
// variables with the same name must match across all shaders that are linked into a single program
uniform crossStageBlock {
    float a;
    // 2nd member from vert absent in frag
} blockname;

void main() {}