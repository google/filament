#version 450

layout(binding = 0) uniform sampler1DShadow uShadow1D;
layout(binding = 1) uniform sampler2DShadow uShadow2D;
layout(binding = 2) uniform sampler1D uSampler1D;
layout(binding = 3) uniform sampler2D uSampler2D;
layout(binding = 4) uniform sampler3D uSampler3D;

layout(location = 0) out float FragColor;
layout(location = 1) in vec4 vClip4;
layout(location = 2) in vec2 vClip2;
layout(location = 0) in vec3 vClip3;

void main()
{
    vec4 _20 = vClip4;
    _20.y = vClip4.w;
    FragColor = textureProj(uShadow1D, vec4(_20.x, 0.0, vClip4.z, _20.y));
    vec4 _30 = vClip4;
    _30.z = vClip4.w;
    FragColor = textureProj(uShadow2D, vec4(_30.xy, vClip4.z, _30.z));
    FragColor = textureProj(uSampler1D, vClip2).x;
    FragColor = textureProj(uSampler2D, vClip3).x;
    FragColor = textureProj(uSampler3D, vClip4).x;
}

