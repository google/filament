//============================================================================================================
//
//
//                  Copyright (c) 2023, Qualcomm Innovation Center, Inc. All rights reserved.
//                              SPDX-License-Identifier: BSD-3-Clause
//
//============================================================================================================

////////////////////////
// USER CONFIGURATION //
////////////////////////

float fastLanczos2(float x)
{
	float wA = x-4.0;
	float wB = x*wA-wA;
	wA *= wA;
	return wB*wA;
}

#if defined(UseEdgeDirection)
vec2 weightY(float dx, float dy, float c, vec3 data)
#else
vec2 weightY(float dx, float dy, float c, float data)
#endif
{
#if defined(UseEdgeDirection)
	float stdDeviation = data.x;
	vec2 dir = data.yz;

	float edgeDis = ((dx*dir.y)+(dy*dir.x));
	float x = (((dx*dx)+(dy*dy))+((edgeDis*edgeDis)*((clamp(((c*c)*stdDeviation),0.0,1.0)*0.7)+-1.0)));
#else
	float stdDeviation = data;
	float x = ((dx*dx)+(dy*dy))* 0.55 + clamp(abs(c)*stdDeviation, 0.0, 1.0);
#endif

	float w = fastLanczos2(x);
	return vec2(w, w * c);
}

vec2 edgeDirection(vec4 left, vec4 right)
{
	vec2 dir;
	float RxLz = (right.x + (-left.z));
	float RwLy = (right.w + (-left.y));
	vec2 delta;
	delta.x = (RxLz + RwLy);
	delta.y = (RxLz + (-RwLy));
	float lengthInv = inversesqrt((delta.x * delta.x+ 3.075740e-05) + (delta.y * delta.y));
	dir.x = (delta.x * lengthInv);
	dir.y = (delta.y * lengthInv);
	return dir;
}

vec4 sgsr(mediump sampler2D ps0, highp vec2 in_TEXCOORD0, highp vec4 ViewportInfo[1])
{
	vec4 color;
	if(OperationMode == 1)
		color.xyz = textureLod(ps0,in_TEXCOORD0.xy,0.0).xyz;
	else
		color.xyzw = textureLod(ps0,in_TEXCOORD0.xy,0.0).xyzw;

#if OperationMode != 4
	if (OperationMode!=4)
	{
		highp vec2 imgCoord = ((in_TEXCOORD0.xy*ViewportInfo[0].zw)+vec2(-0.5,0.5));
		highp vec2 imgCoordPixel = floor(imgCoord);
		highp vec2 coord = (imgCoordPixel*ViewportInfo[0].xy);
		vec2 pl = (imgCoord+(-imgCoordPixel));
		vec4  left = gather(ps0,coord, OperationMode);

		float edgeVote = abs(left.z - left.y) + abs(color[OperationMode] - left.y)  + abs(color[OperationMode] - left.z) ;
		if(edgeVote > EdgeThreshold)
		{
			coord.x += ViewportInfo[0].x;

			vec4 right = gather(ps0,coord + vec2(ViewportInfo[0].x, 0.0), OperationMode);
			vec4 upDown;
			upDown.xy = gather(ps0,coord + vec2(0.0, -ViewportInfo[0].y),OperationMode).wz;
			upDown.zw  = gather(ps0,coord+ vec2(0.0, ViewportInfo[0].y), OperationMode).yx;

			float mean = (left.y+left.z+right.x+right.w)*0.25;
			left = left - vec4(mean);
			right = right - vec4(mean);
			upDown = upDown - vec4(mean);
			color.w = color[OperationMode] - mean;

			float sum = (((((abs(left.x)+abs(left.y))+abs(left.z))+abs(left.w)) +
					(((abs(right.x)+abs(right.y))+abs(right.z))+abs(right.w))) +
					(((abs(upDown.x)+abs(upDown.y))+abs(upDown.z))+abs(upDown.w)));

#if defined(UseEdgeDirection)
			float sumMean = 1.014185e+01 / sum;
			float stdDeviation = (sumMean * sumMean);
			vec3 data = vec3(stdDeviation, edgeDirection(left, right));
#else
			// taken from sgsr sample
			float data = 2.181818 / sum;
#endif

			vec2 aWY = weightY(pl.x, pl.y+1.0, upDown.x,data);
			aWY += weightY(pl.x-1.0, pl.y+1.0, upDown.y,data);
			aWY += weightY(pl.x-1.0, pl.y-2.0, upDown.z,data);
			aWY += weightY(pl.x, pl.y-2.0, upDown.w,data);
			aWY += weightY(pl.x+1.0, pl.y-1.0, left.x,data);
			aWY += weightY(pl.x, pl.y-1.0, left.y,data);
			aWY += weightY(pl.x, pl.y, left.z,data);
			aWY += weightY(pl.x+1.0, pl.y, left.w,data);
			aWY += weightY(pl.x-1.0, pl.y-1.0, right.x,data);
			aWY += weightY(pl.x-2.0, pl.y-1.0, right.y,data);
			aWY += weightY(pl.x-2.0, pl.y, right.z,data);
			aWY += weightY(pl.x-1.0, pl.y, right.w,data);

			float finalY = aWY.y/aWY.x;
			float maxY = max(max(left.y,left.z),max(right.x,right.w));
			float minY = min(min(left.y,left.z),min(right.x,right.w));
			float deltaY = clamp(EdgeSharpness*finalY, minY, maxY) -color.w;

			//smooth high contrast input
			deltaY = clamp(deltaY, -23.0 / 255.0, 23.0 / 255.0);

			color.x = clamp(color.x+deltaY, 0.0, 1.0);
			color.y = clamp(color.y+deltaY, 0.0, 1.0);
			color.z = clamp(color.z+deltaY, 0.0, 1.0);
		}
	}
#endif

	color.w = 1.0;  //assume alpha channel is not used
	return color;
}