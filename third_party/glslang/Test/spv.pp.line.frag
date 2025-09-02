#version 140
#extension GL_GOOGLE_cpp_style_line_directive : require

uniform sampler1D       texSampler1D;
uniform sampler2D       texSampler2D;

varying float blend;
varying vec4 u;

in  vec2 coords2D;

#line 0 "header.h"
float myAbs(float x) {
    if (x > 0) {
        return x;
    }
    else {
        return -x;
    }
}

#line 22 "spv.pp.line.frag"
void main()
{
    float blendscale = myAbs(1.789);
    float bias       = 2.0;
    float coords1D   = 1.789;
    vec4  color      = vec4(0.0, 0.0, 0.0, 0.0);
#line 53
    color += texture    (texSampler1D, coords1D);
    color += texture    (texSampler1D, coords1D, bias);
#line 102
    color += texture        (texSampler2D, coords2D);
    color += texture        (texSampler2D, coords2D, bias);

    gl_FragColor = mix(color, u, blend * blendscale);
}
