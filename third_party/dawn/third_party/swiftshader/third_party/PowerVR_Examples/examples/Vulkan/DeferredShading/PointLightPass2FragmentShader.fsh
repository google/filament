#version 320 es

layout(set = 0, binding = 0) uniform StaticPerScene
{
	mediump float farClipDistance;
};

layout(set = 1, binding = 1) uniform DynamicsPerPointLight
{
	highp mat4 mWorldViewProjectionMatrix;
	highp vec4 vViewPosition;
	highp mat4 mProxyWorldViewProjectionMatrix;
	highp mat4 mProxyWorldViewMatrix;
};

layout(set = 1, binding = 0) uniform StaticsPerPointLight
{
	mediump float fLightIntensity;
	mediump float fLightRadius;
	mediump vec4 vLightColor;
	mediump vec4 vLightSourceColor;
};

layout(input_attachment_index = 0, set = 2, binding = 0) uniform mediump subpassInput localMemAlbedo;
layout(input_attachment_index = 1, set = 2, binding = 1) uniform mediump subpassInput localMemNormal;
layout(input_attachment_index = 2, set = 2, binding = 2) uniform highp subpassInput localMemDepth;

layout(location = 0) out mediump vec4 oColorFbo;

layout(location = 0) in highp vec3 vPositionVS;
layout(location = 1) in mediump vec3 vViewDirVS;

void main()
{		
	//
	// Read GBuffer attributes
	//
	mediump vec4 albedoSpec = subpassLoad(localMemAlbedo);
	mediump vec3 normal =  subpassLoad(localMemNormal).xyz;
	// reconstruct original depth value
	highp float depth = -subpassLoad(localMemDepth).x;

	//
	// Reconstruct view space position 
	//
	highp vec3 viewRay = vec3(vPositionVS.xy * (farClipDistance / vPositionVS.z), farClipDistance);
	highp vec3 positionVS = viewRay * depth;
	
	//
	// Calculate view space light direction
	//
	mediump vec3 lightDirection = vViewPosition.xyz - positionVS;
	mediump float lightDistance = length(lightDirection);
	lightDirection /= lightDistance;

	mediump vec3 color = vec3(0.);
	
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
		// parts of the image, but then avoiding a "sharp cut-off" at the light's max range by tapering off to zero.

		// calculate an attenuation factor
		highp float attenuation = 1. / (lightDistance * lightDistance);
		highp float attenuationSoftenEdge = (3. / (attenuation_point * attenuation_point)) - (2. * lightDistance / (attenuation_point * attenuation_point * attenuation_point));

		attenuation = mix(attenuation, attenuationSoftenEdge, lightDistance > attenuation_point);

		color = fLightIntensity * diffuse * vLightColor.rgb * attenuation;
	}
	oColorFbo = vec4(color, 1.0);
}
