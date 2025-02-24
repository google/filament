#version 320 es

layout(input_attachment_index = 0, set = 0, binding = 2) uniform mediump subpassInput localMemAlbedo;
layout(input_attachment_index = 1, set = 0, binding = 3) uniform mediump subpassInput localMemNormal;
layout(input_attachment_index = 2, set = 0, binding = 4) uniform highp subpassInput localMemDepth;

layout(location = 0) out mediump vec4 oColorFbo;

layout(set = 0, binding = 0) uniform StaticPerDirectionalLight
{
	mediump vec4 fLightIntensity;
	mediump vec4 fAmbientLight;
};

layout(set = 0, binding = 1) uniform DynamicPerDirectionalLight
{	
	mediump vec4 vViewDirection;
};

layout(location = 0) in mediump vec2 vViewDirVS;

void main()
{
	mediump vec3 viewDir = normalize(vec3(vViewDirVS, -subpassLoad(localMemDepth).r));
	
	// Fetch required gbuffer attributes
	mediump vec4 albedoSpec = subpassLoad(localMemAlbedo);
	mediump vec3 normal = subpassLoad(localMemNormal).rgb;
	
	mediump vec3 lightdir = normalize(-vViewDirection.xyz);

	mediump vec3 color = vec3(0.f);
	// Calculate simple diffuse lighting
	mediump float n_dot_l = max(dot(lightdir, normal.xyz), 0.0);
	color = albedoSpec.rgb * (n_dot_l * fLightIntensity.rgb + fAmbientLight.rgb);
	
	if (false && n_dot_l > 0.)
	{
		mediump vec3 reflectedLightDirection = reflect(lightdir, normal);
		mediump float v_dot_r = max(dot(viewDir, reflectedLightDirection), 0.0);
		color += vec3(pow(v_dot_r, 5.0) * fLightIntensity * albedoSpec.a);
	}

	oColorFbo = vec4(color, 1.); 
}
