#version 320 es

layout(set = 0, binding = 0) uniform StaticPerScene
{
	mediump float farClipDistance;
};

layout(set = 1, binding = 0) uniform StaticsPerPointLight
{
	mediump float fLightIntensity;
	mediump float fLightRadius;
	mediump vec4 vLightColor;
	mediump vec4 vLightSourceColor;
};

layout(location = 0) out mediump vec4 oColorFbo;

void main()
{
	oColorFbo = vLightSourceColor;
}
