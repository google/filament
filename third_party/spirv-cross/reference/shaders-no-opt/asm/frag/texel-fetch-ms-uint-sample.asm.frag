#version 450

layout(binding = 0) uniform sampler2DMS uSamp;

layout(location = 0) out vec4 FragColor;

void main()
{
    ivec2 _28 = ivec2(gl_FragCoord.xy);
    FragColor.x = texelFetch(uSamp, _28, int(0u)).x;
    FragColor.y = texelFetch(uSamp, _28, int(1u)).x;
    FragColor.z = texelFetch(uSamp, _28, int(2u)).x;
    FragColor.w = texelFetch(uSamp, _28, int(3u)).x;
}

