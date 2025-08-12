#version 460

layout(location = 0) in BBlock0 {
    vec4 b0;
    vec4 b1;
    vec4 b2;
    vec4 b3;
};

layout(location = 4) in vec4 b4[2];

layout(location = 6) in BBlock1 {
    vec4 b5;
};

layout(location = 7) in vec4 b6[5];

layout(location = 12) in float b7[4];

layout(location = 21) in BBlock2 {
    vec2 b8;
    float b9;
    layout(location = 16) vec3 b10;
    float b11[4];
};

layout(location = 23) in BBlock3 {
    float b12[6];
};

layout(location = 29) in BBlock4 {
    float b13[]; // implicitly sized to 10
};

layout(location = 39) in BBlock5 {
    flat ivec2 b14;
};

layout(location = 40) in mat2 b15;

in flat uint b16;

layout(location = 45) in BBlock6 {
    vec4 b17;
    layout(location = 43) vec4 b18;
    vec4 b19;
};

layout(binding = 0) uniform UBO0 {
    int i;
} ubo0;

layout(location = 46) in vec4 b20[4];

layout(location = 50) in BBlock7 {
    vec4 b21[10];
};

layout(location = 60) in vec2 b22;

// automatically mapped by name
in Block0 {
    vec4 a20;
};

// automatically mapped by name
in vec2 a21[2];

out vec4 o;

uint uncalled() {
    return b16;
}

void main() {
    o = b1;
    o += b2;
    o += b3;
    o += b4[0];
    o += b4[1];
    o += b5;
    o += b6[0];
    o += b6[1];
    o += b6[2];
    o += b6[3];
    o += b6[4];
    o += vec4(b7[0], b7[1], b7[2], b7[3]);
    o += b8.xxyy;
    o += vec4(b9);
    o += b10.xyzx;
    o += vec4(b11[0], b11[1], b11[2], b11[3]);
    o += vec4(b12[0], b12[1], b12[2], b12[3] + b12[4] + b12[5]);
    o += vec4(b13[9], b13[7], b13[6], b13[0]);
    if (false) {
        o += (b15 * vec2(1.0)).xxyy;
        o += b18;
        o += b19;
    }
    o += b20[ubo0.i];
    o += b21[ubo0.i];
    o += b22.xxyy;
    o += a20;
    o += vec4(a21[0], a21[1]);
}
