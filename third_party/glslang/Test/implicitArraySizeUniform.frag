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
    vec4 a[11];
} b1;

// unsized and not indexed in earlier stage, unsized and dynamically indexed in later stage
// both should become runtime sized
buffer B2 {
    vec4 a[];
} b2;

in vec4 out_VS;
out vec4 o;

void main() {
    o = texture(s0[nonuniformEXT(1)], gl_FragCoord.xy);
    o += u1.a[6];
    o += b0[9].a;
    o += b2.a[nonuniformEXT(1)];
}
