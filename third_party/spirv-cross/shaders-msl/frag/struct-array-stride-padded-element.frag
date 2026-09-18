#version 450

// std140 gives SpotLight an ArrayStride of 64 while its packed MSL size is 52.
// The array must be indexed dynamically so the padded array element path is taken;
// every member following the array must keep its SPIR-V offset.
struct SpotLight
{
	vec3 position;
	float range;
	vec3 direction;
	float angle;
	vec3 color;
	float intensity;
	float penumbra;
};

layout(std140, set = 0, binding = 0) uniform UBO
{
	SpotLight spot_lights[4];
	int spot_light_count;
	vec3 albedo;
	float roughness;
	float alpha;
} ubo;

layout(location = 0) out vec4 FragColor;

void main()
{
	vec3 acc = vec3(0.0);
	int n = min(ubo.spot_light_count, 4);
	for (int i = 0; i < n; i++)
		acc += ubo.spot_lights[i].color * ubo.spot_lights[i].intensity * ubo.spot_lights[i].penumbra;
	FragColor = vec4(ubo.albedo.r, ubo.roughness, ubo.alpha * acc.b, 1.0);
}
