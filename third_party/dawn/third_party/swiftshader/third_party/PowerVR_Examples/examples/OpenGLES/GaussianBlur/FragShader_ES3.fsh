#version 310 es

uniform mediump sampler2D sOriginalTexture;
uniform mediump sampler2D sTexture;

const int GaussianKernelSize = 19;
const int NumGaussianWeightsAndOffsets = (GaussianKernelSize - 1) / 2 + 1;
const int HalfSetGaussianWeightsAndOffsets = NumGaussianWeightsAndOffsets / 2;

layout(std140, binding = 0) uniform blurConfig
{
	mediump vec2 config; // (windowWidth, inverseImageHeight)
};

const mediump float Weights[] = float[]( 
0.000072479248047, 0.003696441650391, 0.044357299804688, 0.192214965820313, 0.259658813476563, 0.259658813476563, 0.192214965820313, 0.044357299804688, 0.003696441650391, 0.000072479248047
);

layout(location = 0) in mediump vec2 vTexCoords[NumGaussianWeightsAndOffsets + 1];

layout(location = 0) out mediump vec4 oColor;

void main()
{
	mediump float imageCoordX = gl_FragCoord.x - 0.5;
	mediump float windowWidth = config.x;
	mediump float xPosition = imageCoordX / windowWidth;

	mediump vec3 col = vec3(0.0);
	
	if(xPosition < 0.5)
	{
		col = texture(sOriginalTexture, vTexCoords[NumGaussianWeightsAndOffsets]).rgb;
	}
	else if(xPosition > 0.497 && xPosition < 0.503)
	{
		col = vec3(1.0);
	}
	else
	{
		col = Weights[0] * texture(sTexture, vTexCoords[0]).rgb + 
			  Weights[1] * texture(sTexture, vTexCoords[1]).rgb + 
			  Weights[2] * texture(sTexture, vTexCoords[2]).rgb + 
			  Weights[3] * texture(sTexture, vTexCoords[3]).rgb + 
			  Weights[4] * texture(sTexture, vTexCoords[4]).rgb + 
			  Weights[4] * texture(sTexture, vTexCoords[5]).rgb + 
			  Weights[3] * texture(sTexture, vTexCoords[6]).rgb + 
			  Weights[2] * texture(sTexture, vTexCoords[7]).rgb + 
			  Weights[1] * texture(sTexture, vTexCoords[8]).rgb + 
			  Weights[0] * texture(sTexture, vTexCoords[9]).rgb;
	}

#ifndef FRAMEBUFFER_SRGB
	col = pow(col, vec3(0.454545));
#endif

	oColor = vec4(col , 1);
}

