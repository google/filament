#version 310 es
#extension GL_EXT_shader_pixel_local_storage2 : enable

#ifndef GL_EXT_shader_pixel_local_storage2
#extension GL_EXT_shader_pixel_local_storage : require
#endif

layout(rgba8)  __pixel_localEXT FragDataLocal {
	layout(rgba8) mediump vec4 albedo;
	layout(rgb10_a2) mediump vec4 normal;
	layout(r32f) highp float depth;
	layout(r11f_g11f_b10f) mediump vec3 color;
} pls;

layout(binding = 0) uniform StaticPerDirectionalLight
{
	mediump vec4 vLightIntensity;
	mediump vec4 vAmbientLight;
};

layout(binding = 1) uniform DynamicPerDirectionalLight
{	
	mediump vec4 vViewSpaceLightDirection;
};

layout(location = 0) in mediump vec2 vViewDirVS;

void main()
{
	highp vec3 viewDir = normalize(vec3(vViewDirVS, pls.depth));

	// Fetch required gbuffer attributes
	mediump vec4 albedoSpec = pls.albedo;
	mediump vec3 normalTex = pls.normal.rgb;
	mediump vec3 normal = normalize(normalTex.xyz * 2.0 - 1.0);

	mediump vec3 lightdir = normalize(-vViewSpaceLightDirection.xyz);

	mediump vec3 tmpcolor = vec3(0.f);
	highp float depth1 = pls.depth;
	// Calculate simple diffuse lighting
	mediump float n_dot_l = max(dot(lightdir, normal.xyz), 0.0);
	tmpcolor = albedoSpec.rgb * (n_dot_l * vLightIntensity.rgb + vAmbientLight.rgb);

	if (false && n_dot_l > 0.)
	{

		mediump vec3 reflectedLightDirection = reflect(lightdir, normal);
		mediump float v_dot_r = max(dot(viewDir, reflectedLightDirection), 0.0);
		tmpcolor += vec3(pow(v_dot_r, 5.0)) * vLightIntensity.rgb * albedoSpec.a;
	}

	pls.color = tmpcolor;
}
