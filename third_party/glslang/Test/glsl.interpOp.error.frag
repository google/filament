#version 320 es

struct S
{
    highp float a;
    highp float b;
};
layout(location = 0) in S v_var;

layout(location = 2) in highp float v;

struct S0 {
    highp vec4 s_v;
};

layout(location = 3) in FIn {
    highp float x;
    highp vec4 xyz[1];
    S0 s0;
};

layout(location = 7) in highp float z[1];

layout(location = 8) in highp vec4 w;

layout(location = 0) out mediump vec4 fragColor;
void main (void)
{
    // Centroid
    {
        // valid
        fragColor = vec4(interpolateAtCentroid(v));
        fragColor = vec4(interpolateAtCentroid(x));
        fragColor = vec4(interpolateAtCentroid(z[0]));
        fragColor = interpolateAtCentroid(w);
        fragColor = interpolateAtCentroid(xyz[0]);

        //// invalid
        fragColor = vec4(interpolateAtCentroid(v_var.a));
        fragColor = vec4(interpolateAtCentroid(w.x));
        fragColor = vec4(interpolateAtCentroid(s0.s_v));
    }

    // Sample
    {
        // valid
        fragColor = vec4(interpolateAtSample(v, 0));
        fragColor = vec4(interpolateAtSample(x, 0));
        fragColor = vec4(interpolateAtSample(z[0], 0));
        fragColor = interpolateAtSample(w, 0);
        fragColor = interpolateAtSample(xyz[0], 0);

        // invalid
        fragColor = vec4(interpolateAtSample(v_var.a, 0));
        fragColor = vec4(interpolateAtSample(w.x, 0));
        fragColor = vec4(interpolateAtSample(s0.s_v, 0));
    }

    // Offset
    {
        // valid
        fragColor = vec4(interpolateAtOffset(v,    vec2(0)));
        fragColor = vec4(interpolateAtOffset(x,    vec2(0)));
        fragColor = vec4(interpolateAtOffset(z[0], vec2(0)));
        fragColor = interpolateAtOffset(w,         vec2(0));
        fragColor = interpolateAtOffset(xyz[0],    vec2(0));

        // invalid
        fragColor = vec4(interpolateAtOffset(v_var.a, vec2(0)));
        fragColor = vec4(interpolateAtOffset(w.x,     vec2(0)));
        fragColor = vec4(interpolateAtOffset(s0.s_v, vec2(0)));
    }
}
