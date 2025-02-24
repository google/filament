#version 310 es
#extension GL_EXT_shader_pixel_local_storage2 : enable

#ifndef GL_EXT_shader_pixel_local_storage2
#extension GL_EXT_shader_pixel_local_storage : require
#endif

uniform mediump float fFarClipDistance;

layout(std140, binding = 0) uniform DynamicsPerPointLight
{
	highp mat4 mWorldViewProjectionMatrix;
	highp vec4 vViewPosition;
	highp mat4 mProxyWorldViewProjectionMatrix;
	highp mat4 mProxyWorldViewMatrix;
};

layout(std140, binding = 1) uniform StaticsPerPointLight
{
	mediump float fLightIntensity;
	mediump float fLightRadius;
	mediump vec4 vLightColor;
	mediump vec4 vLightSourceColor;
};

layout(rgba8)  __pixel_localEXT FragDataLocal {
	layout(rgba8) mediump vec4 albedo;
	layout(rgb10_a2) mediump vec4 normal; 
	layout(r32f) highp float depth;
	layout(r11f_g11f_b10f) mediump vec3 color;
} pls;

layout(location = 0) in highp vec3 vPositionVS;
layout(location = 1) in mediump vec3 vViewDirVS;

void main()
{
	// Fetch required gbuffer attributes
	mediump vec3 normalTex = pls.normal.rgb;
	mediump vec4 albedoSpec = pls.albedo;
	mediump vec3 normal = normalize(normalTex.xyz * 2.0 - 1.0);
	mediump float depth = pls.depth;

	//
	// Reconstruct view space position 
	//
	mediump vec3 viewRay = vec3(vPositionVS.xy * (fFarClipDistance / vPositionVS.z), fFarClipDistance);
	highp vec3 positionVS = viewRay * depth;
		
	//
	// Calculate view space light direction
	//
	mediump vec3 lightDirection = vViewPosition.xyz - positionVS;
	mediump float lightDistance = length(lightDirection);
	lightDirection /= lightDistance;

	if (lightDistance <= fLightRadius)
	{
		//
		// Calculate lighting terms
		//
		mediump float n_dot_l = max(dot(lightDirection, normal), 0.0);
		mediump vec3 diffuse = n_dot_l * albedoSpec.rgb;

		mediump vec3 viewDirection = normalize(vViewDirVS);
		mediump vec3 reflectedLightDirection = reflect(lightDirection, normal);
		mediump float v_dot_r = max(dot(viewDirection, reflectedLightDirection), 0.0);
		diffuse += vec3(pow(v_dot_r, 16.0) * albedoSpec.a);

		highp float attenuation_point = fLightRadius * .667; // as described in the .cpp file, the attenuation equation
		// we are using switches from "physically based" quadratic attenuation to a softening linear mode at 2/3 of the
		// total light radius, 1) keeping the "physically based" quadratic equation as much as possible at the brighter
		// parts of the image, but then avoiding a "sharp cutoff" at the light's max range by tapering off to zero.

		// calculate an attenuation factor
		mediump float attenuation = 1. / (lightDistance * lightDistance);
		mediump float attenuationSoftenEdge = (3. / (attenuation_point * attenuation_point)) - (2. * lightDistance / (attenuation_point * attenuation_point * attenuation_point));

		attenuation = mix(attenuation, attenuationSoftenEdge, lightDistance > attenuation_point);

		mediump vec3 lightColor = fLightIntensity * diffuse * vLightColor.rgb * attenuation;
		pls.color = pls.color + lightColor;
	}
}