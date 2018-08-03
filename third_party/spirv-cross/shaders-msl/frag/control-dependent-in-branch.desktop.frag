#version 450

layout(location = 0) out vec4 FragColor;
layout(binding = 0) uniform sampler2D uSampler;
layout(location = 0) in vec4 vInput;

void main()
{
	FragColor = vInput;
	vec4 t = texture(uSampler, vInput.xy);
	vec4 d0 = dFdx(vInput);
	vec4 d1 = dFdy(vInput);
	vec4 d2 = fwidth(vInput);
	vec4 d3 = dFdxCoarse(vInput);
	vec4 d4 = dFdyCoarse(vInput);
	vec4 d5 = fwidthCoarse(vInput);
	vec4 d6 = dFdxFine(vInput);
	vec4 d7 = dFdyFine(vInput);
	vec4 d8 = fwidthFine(vInput);
	if (vInput.y > 10.0)
	{
		FragColor += t;
		FragColor += d0;
		FragColor += d1;
		FragColor += d2;
		FragColor += d3;
		FragColor += d4;
		FragColor += d5;
		FragColor += d6;
		FragColor += d7;
		FragColor += d8;
	}
}

