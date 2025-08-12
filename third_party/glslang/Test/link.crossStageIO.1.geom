#version 460

layout(triangles) in;
layout(triangle_strip, max_vertices=3) out;

// automatically mapped by name
in Block0 {
    vec4 a20;
} gs_in0[3];

// automatically mapped by name
in vec2 a21[3][2];

// automatically mapped by name
out GBlock0 {
    vec4 a20;
} gs_out0;

// automatically mapped by name
out vec2 g_a21[2];

in GBlock1 {
    layout(location = 0) in vec4 a0[64];
} gs_in1[3];

layout(location = 0) out vec4 o0[64];

void main() {
    for (int i = 0; i < 32; ++i) {
        o0[i] = gs_in1[0].a0[i];
    }

    gs_out0.a20 = gs_in0[0].a20;
    g_a21[0] = a21[0][0];
    g_a21[0] = a21[0][1];

    gl_Position = vec4(0);

    EmitVertex();
    EmitVertex();
    EmitVertex();
}
