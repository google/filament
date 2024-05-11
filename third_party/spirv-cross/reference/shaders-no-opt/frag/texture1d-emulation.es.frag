#version 310 es
precision mediump float;
precision highp int;

layout(binding = 0) uniform highp sampler2D uSamp;
layout(binding = 1) uniform highp sampler2DShadow uSampShadow;
layout(binding = 2) uniform highp sampler2DArray uSampArray;
layout(binding = 3) uniform highp sampler2DArrayShadow uSampArrayShadow;
layout(binding = 4, r32f) uniform highp image2D uImage;

layout(location = 0) out highp vec4 FragColor;
layout(location = 0) in highp vec4 vUV;

void main()
{
    FragColor = texture(uSamp, vec2(vUV.x, 0.0));
    FragColor += textureProj(uSamp, vec3(vUV.xy.x, 0.0, vUV.xy.y));
    FragColor += texelFetch(uSamp, ivec2(int(vUV.x), 0), 0);
    FragColor += vec4(texture(uSampShadow, vec3(vUV.xyz.x, 0.0, vUV.xyz.z)));
    highp vec4 _54 = vUV;
    highp vec4 _57 = _54;
    _57.y = _54.w;
    FragColor += vec4(textureProj(uSampShadow, vec4(_57.x, 0.0, _54.z, _57.y)));
    FragColor = texture(uSampArray, vec3(vUV.xy.x, 0.0, vUV.xy.y));
    FragColor += texelFetch(uSampArray, ivec3(ivec2(vUV.xy).x, 0, ivec2(vUV.xy).y), 0);
    FragColor += vec4(texture(uSampArrayShadow, vec4(vUV.xyz.xy.x, 0.0, vUV.xyz.xy.y, vUV.xyz.z)));
    FragColor += imageLoad(uImage, ivec2(int(vUV.x), 0));
    imageStore(uImage, ivec2(int(vUV.x), 0), FragColor);
}

