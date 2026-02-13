#version 460

// explicitly sized in earlier stage, unsized and statically indexed with larger index in later stage
// error: array is indexed with index (6) greater than its size (3)
uniform sampler2D s[];

// unsized and statically indexed in earlier stage, explicitly sized with smaller index (but not indexed) in later stage
// error: array is indexed with index (20) greater than its size (2)
uniform U0 {
    vec4 a[2];
} u0;

// explicitly sized in earlier stage, unsized and statically indexed with larger index in later stage
// error: array is indexed with index (12) greater than its size (5)
uniform U1 {
    vec4 a[];
} u1;

// explicitly sized (3) and not indexed in earlier stage, explicitly sized with different size (5) and not indexed in later stage
// error: array size in earlier stage (3) does not array size in later stage (5)
buffer B0 {
    vec4 a[5];
} b0;

// explicitly sized in earlier stage, unsized and statically indexed with larger index in later stage
// error: array is indexed with index (4) greater than its size (3)
buffer B1 {
    vec4 a[];
} b1;

// explicitly sized in earlier stage, unsized and statically indexed with larger index in later stage
// error: array is indexed with index (4) greater than its size (2)
buffer B2 {
    vec4 a[];
} b2[2];

in vec4 out_VS;
out vec4 o;

void main() {
    o = texture(s[6], out_VS.xy);
    o = u1.a[12];
    o = b1.a[4];
}
