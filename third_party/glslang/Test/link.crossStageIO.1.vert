#version 460

layout(location = 40) out vec4 a0;

layout(location = 1) out ABlock0 {
    vec4 a1;
};

out ABlock1 {
    layout(location = 3) vec4 a2;
    layout(location = 2) vec4 a3;
};

layout(location = 5) out vec4 a4[3];

layout(location = 4) out ABlock2 {
    vec4 a5;
    layout(location = 8) vec4 a6[2];
    vec4 a7[2];
};

struct S {
    float a[3];
    float b;
};

layout(location = 12) out ABlock3 {
    float a8;
    layout(location = 15) vec2 a9[2];
    vec4 a10[2];
    layout(location = 19) S a11[2]; // consumes 8 locations
};

layout(location = 27) out float a12[3];

layout(location = 30) out struct AStruct1 {
    vec2 a13;
    S a14; // consumes 4 locations
    mat4 a15;
} s0;

layout(location = 39) out float a16;

layout(binding = 0) uniform UBO0 {
    int i;
} ubo0;

layout(location = 46) out vec4 a17[4];

layout(location = 50) out ABlock4 {
    vec4 a18[10];
};

layout(location = 59) out ABlock5 {
    layout(location = 60) vec2 a19;
};

// automatically mapped by name
out Block0 {
    vec4 a20;
};

// automatically mapped by name
out vec2 a21[2];

void main() {
    if (false) {
        a0 = vec4(1.0);
    }
    a1 = vec4(0.0);
    a2 = vec4(0.0);
    a3 = vec4(0.0);
    a4[0] = vec4(0.0);
    a4[1] = vec4(0.0);
    a4[2] = vec4(0.0);
    a5 = vec4(0.0);
    a6[0] = vec4(0.0);
    a6[1] = vec4(0.0);
    a7[0] = vec4(0.0);
    a7[1] = vec4(0.0);
    a8 = 0.0;
    a9[0].x = 0.0;
    a9[1].y = 0.0;
    a10[0] = vec4(0.0);
    a10[1] = vec4(0.0);
    a11[0].a[0] = 0.0;
    a11[0].a[1] = 0.0;
    a11[0].a[2] = 0.0;
    a11[0].b = 0.0;
    a11[1].a[0] = 0.0;
    a11[1].a[1] = 0.0;
    a11[1].a[2] = 0.0;
    a11[1].b = 0.0;
    a12[0] = 0.0;
    a12[1] = 0.0;
    a12[2] = 0.0;
    s0.a13.y = 0.0;
    s0.a14.a[0] = 0.0;
    s0.a14.a[1] = 0.0;
    s0.a14.a[2] = 0.0;
    s0.a14.b = 0.0;
    s0.a15 = mat4(1.0);
    // a16 intentionally not written to
    a17[ubo0.i] = vec4(1.0);
    a18[5] = vec4(1.0);
    a19 = vec2(0.0);
    a20 = vec4(1.0);
    a21[0] = vec2(0.0);
    a21[1] = vec2(0.0);
}
