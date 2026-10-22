#version 460

// explicitly sized in earlier stage, unsized and statically indexed with larger index in later stage
// error: array is indexed with index (6) greater than its size (3)
uniform sampler2D s[3];

// unsized and statically indexed in earlier stage, explicitly sized with smaller index (but not indexed) in later stage
// error: array is indexed with index (20) greater than its size (2)
uniform U0 {
    vec4 a[];
} u0;

// explicitly sized in earlier stage, unsized and statically indexed with larger index in later stage
// error: array is indexed with index (12) greater than its size (5)
uniform U1 {
    vec4 a[5];
} u1;

// explicitly sized (3) and not indexed in earlier stage, explicitly sized with different size (5) and not indexed in later stage
// error: array size in earlier stage (3) does not array size in later stage (5)
buffer B0 {
    vec4 a[3];
} b0;

// explicitly sized in earlier stage, unsized and statically indexed with larger index in later stage
// error: array is indexed with index (4) greater than its size (3)
buffer B1 {
    vec4 a[3];
} b1;

// explicitly sized in earlier stage, unsized and statically indexed with larger index in later stage
// error: array is indexed with index (4) greater than its size (2)
buffer B2 {
    vec4 a[];
} b2[];

out vec4 out_VS;

void main() {
    out_VS = u0.a[20];
    out_VS = b2[4].a[0];
}
