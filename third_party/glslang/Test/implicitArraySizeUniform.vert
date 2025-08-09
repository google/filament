#version 460

#extension GL_EXT_nonuniform_qualifier : require

// unsized and not indexed in either stage
// both should become implicitly sized to 1
uniform float f0[];

// unsized and not indexed in earlier stage, unsized and dynamically indexed in later stage
// both should become runtime sized
uniform sampler2D s0[];

// unsized and dynamically indexed in earlier stage, unsized and not indexed in later stage
// both should become runtime sized
uniform sampler2D s1[];

// unsized and statically indexed in earlier stage, unsized and not indexed in later stage
// both should become implicitly sized to highest index + 1 (11)
uniform U0 {
    vec4 a[];
} u0;

// unsized and not indexed in earlier stage, unsized and statically indexed in later stage
// both should become implicitly sized to highest index + 1 (7)
uniform U1 {
    vec4 a[];
} u1;

// unsized buffer array, statically indexed in earlier stage, not indexed in later stage
// both should become implicitly sized to highest index + 1 (10)
buffer B0 {
    vec4 a;
} b0[];

// unsized and statically indexed in earlier stage, explicitly sized in later stage
// should adopt explicit size (11)
buffer B1 {
    vec4 a[];
} b1;

// unsized and not indexed in earlier stage, unsized and dynamically indexed in later stage
// both should become runtime sized
buffer B2 {
    vec4 a[];
} b2;

out vec4 out_VS;

void main() {
    out_VS = texture(s1[nonuniformEXT(0)], vec2(0));
    out_VS = u0.a[10];
    out_VS = b1.a[5];
}
