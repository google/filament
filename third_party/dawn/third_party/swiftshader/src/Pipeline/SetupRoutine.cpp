// Copyright 2016 The SwiftShader Authors. All Rights Reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//    http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "SetupRoutine.hpp"

#include "Constants.hpp"
#include "Device/Polygon.hpp"
#include "Device/Primitive.hpp"
#include "Device/Renderer.hpp"
#include "Device/Vertex.hpp"
#include "Reactor/Reactor.hpp"
#include "Vulkan/VkDevice.hpp"

namespace sw {

SetupRoutine::SetupRoutine(const SetupProcessor::State &state)
    : state(state)
{
}

SetupRoutine::~SetupRoutine()
{
}

void SetupRoutine::generate()
{
	SetupFunction function;
	{
		Pointer<Byte> device(function.Arg<0>());
		Pointer<Byte> primitive(function.Arg<1>());
		Pointer<Byte> tri(function.Arg<2>());
		Pointer<Byte> polygon(function.Arg<3>());
		Pointer<Byte> data(function.Arg<4>());

		Pointer<Byte> constants = device + OFFSET(vk::Device, constants);

		const bool point = state.isDrawPoint;
		const bool line = state.isDrawLine;
		const bool triangle = state.isDrawTriangle;

		const int V0 = OFFSET(Triangle, v0);
		const int V1 = (triangle || line) ? OFFSET(Triangle, v1) : OFFSET(Triangle, v0);
		const int V2 = triangle ? OFFSET(Triangle, v2) : (line ? OFFSET(Triangle, v1) : OFFSET(Triangle, v0));

		Pointer<Byte> v0 = tri + V0;
		Pointer<Byte> v1 = tri + V1;
		Pointer<Byte> v2 = tri + V2;

		Array<Int> X(16);
		Array<Int> Y(16);

		X[0] = *Pointer<Int>(v0 + OFFSET(Vertex, projected.x));
		X[1] = *Pointer<Int>(v1 + OFFSET(Vertex, projected.x));
		X[2] = *Pointer<Int>(v2 + OFFSET(Vertex, projected.x));

		Y[0] = *Pointer<Int>(v0 + OFFSET(Vertex, projected.y));
		Y[1] = *Pointer<Int>(v1 + OFFSET(Vertex, projected.y));
		Y[2] = *Pointer<Int>(v2 + OFFSET(Vertex, projected.y));

		Int d = 1;  // Winding direction

		// Culling
		if(triangle)
		{
			Float x0 = Float(X[0]);
			Float x1 = Float(X[1]);
			Float x2 = Float(X[2]);

			Float y0 = Float(Y[0]);
			Float y1 = Float(Y[1]);
			Float y2 = Float(Y[2]);

			Float A = (y0 - y2) * x1 + (y2 - y1) * x0 + (y1 - y0) * x2;  // Area

			Int w0w1w2 = *Pointer<Int>(v0 + OFFSET(Vertex, w)) ^
			             *Pointer<Int>(v1 + OFFSET(Vertex, w)) ^
			             *Pointer<Int>(v2 + OFFSET(Vertex, w));

			A = IfThenElse(w0w1w2 < 0, -A, A);

			Bool frontFacing = (state.frontFace == VK_FRONT_FACE_COUNTER_CLOCKWISE) ? (A >= 0.0f) : (A <= 0.0f);

			if(state.cullMode & VK_CULL_MODE_FRONT_BIT)
			{
				If(frontFacing) Return(0);
			}
			if(state.cullMode & VK_CULL_MODE_BACK_BIT)
			{
				If(!frontFacing) Return(0);
			}

			d = IfThenElse(A > 0.0f, d, Int(0));

			If(frontFacing)
			{
				*Pointer<Byte8>(primitive + OFFSET(Primitive, clockwiseMask)) = Byte8(0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF);
				*Pointer<Byte8>(primitive + OFFSET(Primitive, invClockwiseMask)) = Byte8(0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);
			}
			Else
			{
				*Pointer<Byte8>(primitive + OFFSET(Primitive, clockwiseMask)) = Byte8(0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);
				*Pointer<Byte8>(primitive + OFFSET(Primitive, invClockwiseMask)) = Byte8(0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF);
			}
		}
		else
		{
			*Pointer<Byte8>(primitive + OFFSET(Primitive, clockwiseMask)) = Byte8(0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF);
			*Pointer<Byte8>(primitive + OFFSET(Primitive, invClockwiseMask)) = Byte8(0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);
		}

		Int n = *Pointer<Int>(polygon + OFFSET(Polygon, n));
		Int m = *Pointer<Int>(polygon + OFFSET(Polygon, i));

		If(m != 0 || Bool(!triangle))  // Clipped triangle; reproject
		{
			Pointer<Byte> V = polygon + OFFSET(Polygon, P) + m * sizeof(void *) * 16;

			Int i = 0;

			Do
			{
				Pointer<Float4> p = *Pointer<Pointer<Float4> >(V + i * sizeof(void *));
				Float4 v = *Pointer<Float4>(p, 16);

				Float w = v.w;
				Float rhw = IfThenElse(w != 0.0f, 1.0f / w, Float(1.0f));

				X[i] = RoundInt(*Pointer<Float>(data + OFFSET(DrawData, X0xF)) + v.x * rhw * *Pointer<Float>(data + OFFSET(DrawData, WxF)));
				Y[i] = RoundInt(*Pointer<Float>(data + OFFSET(DrawData, Y0xF)) + v.y * rhw * *Pointer<Float>(data + OFFSET(DrawData, HxF)));

				i++;
			}
			Until(i >= n);
		}

		// Vertical range
		Int yMin = Y[0];
		Int yMax = Y[0];

		Int i = 1;

		Do
		{
			yMin = Min(Y[i], yMin);
			yMax = Max(Y[i], yMax);

			i++;
		}
		Until(i >= n);

		constexpr int subPixB = vk::SUBPIXEL_PRECISION_BITS;
		constexpr int subPixM = vk::SUBPIXEL_PRECISION_MASK;
		constexpr float subPixF = vk::SUBPIXEL_PRECISION_FACTOR;

		if(state.enableMultiSampling)
		{
			yMin = (yMin + yMinMultiSampleOffset) >> subPixB;
			yMax = (yMax + yMaxMultiSampleOffset) >> subPixB;
		}
		else
		{
			yMin = (yMin + subPixM) >> subPixB;
			yMax = (yMax + subPixM) >> subPixB;
		}

		yMin = Max(yMin, *Pointer<Int>(data + OFFSET(DrawData, scissorY0)));
		yMax = Min(yMax, *Pointer<Int>(data + OFFSET(DrawData, scissorY1)));

		// If yMin and yMax are initially negative, the scissor clamping above will typically result
		// in yMin == 0 and yMax unchanged. We bail as we don't need to rasterize this primitive, and
		// code below assumes yMin < yMax.
		If(yMin >= yMax)
		{
			Return(0);
		}

		For(Int q = 0, q < state.multiSampleCount, q++)
		{
			Array<Int> Xq(16);
			Array<Int> Yq(16);

			Int i = 0;

			Do
			{
				Xq[i] = X[i];
				Yq[i] = Y[i];

				if(state.enableMultiSampling)
				{
					// The subtraction here is because we're not moving the point, we're testing the edge against it
					Xq[i] = Xq[i] - *Pointer<Int>(constants + OFFSET(Constants, Xf) + q * sizeof(int));
					Yq[i] = Yq[i] - *Pointer<Int>(constants + OFFSET(Constants, Yf) + q * sizeof(int));
				}

				i++;
			}
			Until(i >= n);

			Pointer<Byte> leftEdge = Pointer<Byte>(primitive + OFFSET(Primitive, outline->left)) + q * sizeof(Primitive);
			Pointer<Byte> rightEdge = Pointer<Byte>(primitive + OFFSET(Primitive, outline->right)) + q * sizeof(Primitive);

			if(state.enableMultiSampling)
			{
				Int xMin = *Pointer<Int>(data + OFFSET(DrawData, scissorX0));
				Int xMax = *Pointer<Int>(data + OFFSET(DrawData, scissorX1));
				Short x = Short(Clamp((X[0] + subPixM) >> subPixB, xMin, xMax));

				For(Int y = yMin - 1, y < yMax + 1, y++)
				{
					*Pointer<Short>(leftEdge + y * sizeof(Primitive::Span)) = x;
					*Pointer<Short>(rightEdge + y * sizeof(Primitive::Span)) = x;
				}
			}

			Xq[n] = Xq[0];
			Yq[n] = Yq[0];

			// Rasterize
			{
				Int i = 0;

				Do
				{
					edge(primitive, data, Xq[i + 1 - d], Yq[i + 1 - d], Xq[i + d], Yq[i + d], q);

					i++;
				}
				Until(i >= n);
			}

			if(!state.enableMultiSampling)
			{
				For(, yMin < yMax && *Pointer<Short>(leftEdge + yMin * sizeof(Primitive::Span)) == *Pointer<Short>(rightEdge + yMin * sizeof(Primitive::Span)), yMin++)
				{
					// Increments yMin
				}

				For(, yMax > yMin && *Pointer<Short>(leftEdge + (yMax - 1) * sizeof(Primitive::Span)) == *Pointer<Short>(rightEdge + (yMax - 1) * sizeof(Primitive::Span)), yMax--)
				{
					// Decrements yMax
				}

				If(yMin == yMax)
				{
					Return(0);
				}

				*Pointer<Short>(leftEdge + (yMin - 1) * sizeof(Primitive::Span)) = *Pointer<Short>(leftEdge + yMin * sizeof(Primitive::Span));
				*Pointer<Short>(rightEdge + (yMin - 1) * sizeof(Primitive::Span)) = *Pointer<Short>(leftEdge + yMin * sizeof(Primitive::Span));
				*Pointer<Short>(leftEdge + yMax * sizeof(Primitive::Span)) = *Pointer<Short>(leftEdge + (yMax - 1) * sizeof(Primitive::Span));
				*Pointer<Short>(rightEdge + yMax * sizeof(Primitive::Span)) = *Pointer<Short>(leftEdge + (yMax - 1) * sizeof(Primitive::Span));
			}
		}

		*Pointer<Int>(primitive + OFFSET(Primitive, yMin)) = yMin;
		*Pointer<Int>(primitive + OFFSET(Primitive, yMax)) = yMax;

		// Sort by minimum y
		if(triangle)
		{
			Float y0 = *Pointer<Float>(v0 + OFFSET(Vertex, y));
			Float y1 = *Pointer<Float>(v1 + OFFSET(Vertex, y));
			Float y2 = *Pointer<Float>(v2 + OFFSET(Vertex, y));

			Float yMin = Min(Min(y0, y1), y2);

			conditionalRotate1(yMin == y1, v0, v1, v2);
			conditionalRotate2(yMin == y2, v0, v1, v2);
		}

		// Sort by maximum w
		if(triangle)
		{
			Float w0 = *Pointer<Float>(v0 + OFFSET(Vertex, w));
			Float w1 = *Pointer<Float>(v1 + OFFSET(Vertex, w));
			Float w2 = *Pointer<Float>(v2 + OFFSET(Vertex, w));

			Float wMax = Max(Max(w0, w1), w2);

			conditionalRotate1(wMax == w1, v0, v1, v2);
			conditionalRotate2(wMax == w2, v0, v1, v2);
		}

		Float w0 = *Pointer<Float>(v0 + OFFSET(Vertex, w));
		Float w1 = *Pointer<Float>(v1 + OFFSET(Vertex, w));
		Float w2 = *Pointer<Float>(v2 + OFFSET(Vertex, w));

		Float4 w012;

		w012.x = w0;
		w012.y = w1;
		w012.z = w2;
		w012.w = 1;

		Float rhw0 = *Pointer<Float>(v0 + OFFSET(Vertex, projected.w));

		Int X0 = *Pointer<Int>(v0 + OFFSET(Vertex, projected.x));
		Int X1 = *Pointer<Int>(v1 + OFFSET(Vertex, projected.x));
		Int X2 = *Pointer<Int>(v2 + OFFSET(Vertex, projected.x));

		Int Y0 = *Pointer<Int>(v0 + OFFSET(Vertex, projected.y));
		Int Y1 = *Pointer<Int>(v1 + OFFSET(Vertex, projected.y));
		Int Y2 = *Pointer<Int>(v2 + OFFSET(Vertex, projected.y));

		if(line)
		{
			X2 = X1 + Y1 - Y0;
			Y2 = Y1 + X0 - X1;
		}

		Float x0 = Float(X0) * (1.0f / subPixF);
		Float y0 = Float(Y0) * (1.0f / subPixF);
		*Pointer<Float>(primitive + OFFSET(Primitive, x0)) = x0;
		*Pointer<Float>(primitive + OFFSET(Primitive, y0)) = y0;

		X1 -= X0;
		Y1 -= Y0;

		X2 -= X0;
		Y2 -= Y0;

		Float x1 = w1 * (1.0f / subPixF) * Float(X1);
		Float y1 = w1 * (1.0f / subPixF) * Float(Y1);

		Float x2 = w2 * (1.0f / subPixF) * Float(X2);
		Float y2 = w2 * (1.0f / subPixF) * Float(Y2);

		Float a = x1 * y2 - x2 * y1;

		Float4 M[3];

		M[0] = Float4(0, 0, 0, 0);
		M[1] = Float4(0, 0, 0, 0);
		M[2] = Float4(0, 0, 0, 0);

		M[0].z = rhw0;

		If(a != 0.0f)
		{
			Float A = 1.0f / a;
			Float D = A * rhw0;

			M[0].x = (y1 * w2 - y2 * w1) * D;
			M[0].y = (x2 * w1 - x1 * w2) * D;
			//	M[0].z = rhw0;
			//	M[0].w = 0;

			M[1].x = y2 * A;
			M[1].y = -x2 * A;
			//	M[1].z = 0;
			//	M[1].w = 0;

			M[2].x = -y1 * A;
			M[2].y = x1 * A;
			//	M[2].z = 0;
			//	M[2].w = 0;
		}

		if(state.interpolateW)
		{
			Float4 ABC = M[0] + M[1] + M[2];

			*Pointer<Float>(primitive + OFFSET(Primitive, w.A)) = ABC.x;
			*Pointer<Float>(primitive + OFFSET(Primitive, w.B)) = ABC.y;
			*Pointer<Float>(primitive + OFFSET(Primitive, w.C)) = ABC.z;
		}

		if(state.interpolateZ)
		{
			Float z0 = *Pointer<Float>(v0 + OFFSET(Vertex, projected.z));
			Float z1 = *Pointer<Float>(v1 + OFFSET(Vertex, projected.z));
			Float z2 = *Pointer<Float>(v2 + OFFSET(Vertex, projected.z));

			z1 -= z0;
			z2 -= z0;

			Float A;
			Float B;
			Float C;

			if(!point)
			{
				Float x1 = Float(X1) * (1.0f / subPixF);
				Float y1 = Float(Y1) * (1.0f / subPixF);
				Float x2 = Float(X2) * (1.0f / subPixF);
				Float y2 = Float(Y2) * (1.0f / subPixF);

				Float D = *Pointer<Float>(data + OFFSET(DrawData, depthRange)) / (x1 * y2 - x2 * y1);

				A = (y2 * z1 - y1 * z2) * D;
				B = (x1 * z2 - x2 * z1) * D;
			}
			else
			{
				A = 0.0f;
				B = 0.0f;
			}

			C = z0 * *Pointer<Float>(data + OFFSET(DrawData, depthRange)) + *Pointer<Float>(data + OFFSET(DrawData, depthNear));

			*Pointer<Float>(primitive + OFFSET(Primitive, z.A)) = A;
			*Pointer<Float>(primitive + OFFSET(Primitive, z.B)) = B;
			*Pointer<Float>(primitive + OFFSET(Primitive, z.C)) = C;

			Float bias = 0.0f;

			if(state.applyConstantDepthBias)
			{
				Float r;  // Minimum resolvable difference

				if(state.fixedPointDepthBuffer)
				{
					// TODO(b/139341727): Pre-multiply the constant depth bias factor by the minimum
					// resolvable difference.

					// TODO(b/139341727): When there's a fixed-point depth buffer and no depth bias clamp,
					// the constant depth bias factor could be added to 'depthNear', eliminating the per-
					// polygon addition.

					r = *Pointer<Float>(data + OFFSET(DrawData, minimumResolvableDepthDifference));
				}
				else  // Floating-point depth buffer
				{
					// "For floating-point depth buffers, there is no single minimum resolvable difference.
					//  In this case, the minimum resolvable difference for a given polygon is dependent on
					//  the maximum exponent, e, in the range of z values spanned by the primitive. If n is
					//  the number of bits in the floating-point mantissa, the minimum resolvable difference,
					//  r, for the given primitive is defined as r = 2^(e-n)."

					Float Z0 = C;
					Float Z1 = z1 * *Pointer<Float>(data + OFFSET(DrawData, depthRange)) + *Pointer<Float>(data + OFFSET(DrawData, depthNear));
					Float Z2 = z2 * *Pointer<Float>(data + OFFSET(DrawData, depthRange)) + *Pointer<Float>(data + OFFSET(DrawData, depthNear));

					Int e0 = As<Int>(Z0) & 0x7F800000;
					Int e1 = As<Int>(Z1) & 0x7F800000;
					Int e2 = As<Int>(Z2) & 0x7F800000;

					Int e = Max(Max(e0, e1), e2);

					r = As<Float>(e) * Float(1.0f / (1 << 23));
				}

				bias = r * *Pointer<Float>(data + OFFSET(DrawData, constantDepthBias));
			}

			if(state.applySlopeDepthBias)
			{
				Float m = Max(Abs(A), Abs(B));

				bias += m * *Pointer<Float>(data + OFFSET(DrawData, slopeDepthBias));  // TODO(b/155302798): Optimize 0 += x;
			}

			if(state.applyConstantDepthBias || state.applySlopeDepthBias)
			{
				if(state.applyDepthBiasClamp)
				{
					Float clamp = *Pointer<Float>(data + OFFSET(DrawData, depthBiasClamp));

					bias = IfThenElse(clamp > 0.0f, Min(bias, clamp), Max(bias, clamp));
				}

				*Pointer<Float>(primitive + OFFSET(Primitive, zBias)) = bias;
			}
		}

		int packedInterpolant = 0;
		for(int interfaceInterpolant = 0; interfaceInterpolant < MAX_INTERFACE_COMPONENTS; interfaceInterpolant++)
		{
			if(state.gradient[interfaceInterpolant].Type != SpirvShader::ATTRIBTYPE_UNUSED)
			{
				setupGradient(primitive, tri, w012, M, v0, v1, v2,
				              OFFSET(Vertex, v[interfaceInterpolant]),
				              OFFSET(Primitive, V[packedInterpolant]),
				              state.gradient[interfaceInterpolant].Flat,
				              !state.gradient[interfaceInterpolant].NoPerspective);
				packedInterpolant++;
			}
		}

		for(unsigned int i = 0; i < state.numClipDistances; i++)
		{
			setupGradient(primitive, tri, w012, M, v0, v1, v2,
			              OFFSET(Vertex, clipDistance[i]),
			              OFFSET(Primitive, clipDistance[i]),
			              false, true);
		}

		for(unsigned int i = 0; i < state.numCullDistances; i++)
		{
			setupGradient(primitive, tri, w012, M, v0, v1, v2,
			              OFFSET(Vertex, cullDistance[i]),
			              OFFSET(Primitive, cullDistance[i]),
			              false, true);
		}

		Return(1);
	}

	routine = function("SetupRoutine");
}

void SetupRoutine::setupGradient(Pointer<Byte> &primitive, Pointer<Byte> &triangle, Float4 &w012, Float4 (&m)[3], Pointer<Byte> &v0, Pointer<Byte> &v1, Pointer<Byte> &v2, int attribute, int planeEquation, bool flat, bool perspective)
{
	if(!flat)
	{
		Float4 i;
		i.x = *Pointer<Float>(v0 + attribute);
		i.y = *Pointer<Float>(v1 + attribute);
		i.z = *Pointer<Float>(v2 + attribute);
		i.w = 0;

		if(!perspective)
		{
			i *= w012;
		}

		Float4 A = i.xxxx * m[0];
		Float4 B = i.yyyy * m[1];
		Float4 C = i.zzzz * m[2];

		Float4 P = A + B + C;

		*Pointer<Float>(primitive + planeEquation + 0) = P.x;
		*Pointer<Float>(primitive + planeEquation + 4) = P.y;
		*Pointer<Float>(primitive + planeEquation + 8) = P.z;
	}
	else
	{
		int leadingVertex = OFFSET(Triangle, v0);
		Float C = *Pointer<Float>(triangle + leadingVertex + attribute);

		*Pointer<Float>(primitive + planeEquation + 0) = 0;
		*Pointer<Float>(primitive + planeEquation + 4) = 0;
		*Pointer<Float>(primitive + planeEquation + 8) = C;
	}
}

void SetupRoutine::edge(Pointer<Byte> &primitive, Pointer<Byte> &data, const Int &Xa, const Int &Ya, const Int &Xb, const Int &Yb, Int &q)
{
	If(Ya != Yb)
	{
		Bool swap = Yb < Ya;

		Int X1 = IfThenElse(swap, Xb, Xa);
		Int X2 = IfThenElse(swap, Xa, Xb);
		Int Y1 = IfThenElse(swap, Yb, Ya);
		Int Y2 = IfThenElse(swap, Ya, Yb);

		constexpr int subPixB = vk::SUBPIXEL_PRECISION_BITS;
		constexpr int subPixM = vk::SUBPIXEL_PRECISION_MASK;

		Int y1 = (Y1 + subPixM) >> subPixB;
		Int y2 = (Y2 + subPixM) >> subPixB;
		Int yMin = Max(y1, *Pointer<Int>(data + OFFSET(DrawData, scissorY0)));
		Int yMax = Min(y2, *Pointer<Int>(data + OFFSET(DrawData, scissorY1)));

		If(yMin < yMax)
		{
			Int xMin = *Pointer<Int>(data + OFFSET(DrawData, scissorX0));
			Int xMax = *Pointer<Int>(data + OFFSET(DrawData, scissorX1));

			Pointer<Byte> leftEdge = primitive + q * sizeof(Primitive) + OFFSET(Primitive, outline->left);
			Pointer<Byte> rightEdge = primitive + q * sizeof(Primitive) + OFFSET(Primitive, outline->right);
			Pointer<Byte> edge = IfThenElse(swap, rightEdge, leftEdge);

			// Deltas
			Int DX12 = X2 - X1;
			Int DY12 = Y2 - Y1;

			Int FDX12 = DX12 << subPixB;
			Int FDY12 = DY12 << subPixB;

			Int X = DX12 * ((y1 << subPixB) - Y1) + (X1 & subPixM) * DY12;
			Int x = (X1 >> subPixB) + X / FDY12;  // Edge
			Int d = X % FDY12;                    // Error-term
			Int ceil = -d >> 31;                  // Ceiling division: remainder <= 0
			x -= ceil;
			d -= ceil & FDY12;

			Int Q = FDX12 / FDY12;  // Edge-step
			Int R = FDX12 % FDY12;  // Error-step
			Int floor = R >> 31;    // Flooring division: remainder >= 0
			Q += floor;
			R += floor & FDY12;

			Int D = FDY12;  // Error-overflow
			Int y = y1;

			Do
			{
				If(y >= yMin)
				{
					*Pointer<Short>(edge + y * sizeof(Primitive::Span)) = Short(Clamp(x, xMin, xMax));
				}

				x += Q;
				d += R;

				Int overflow = -d >> 31;

				d -= D & overflow;
				x -= overflow;

				y++;
			}
			Until(y >= yMax);
		}
	}
}

void SetupRoutine::conditionalRotate1(Bool condition, Pointer<Byte> &v0, Pointer<Byte> &v1, Pointer<Byte> &v2)
{
#if 0  // Rely on LLVM optimization
		If(condition)
		{
			Pointer<Byte> vX;

			vX = v0;
			v0 = v1;
			v1 = v2;
			v2 = vX;
		}
#else
	Pointer<Byte> vX = v0;
	v0 = IfThenElse(condition, v1, v0);
	v1 = IfThenElse(condition, v2, v1);
	v2 = IfThenElse(condition, vX, v2);
#endif
}

void SetupRoutine::conditionalRotate2(Bool condition, Pointer<Byte> &v0, Pointer<Byte> &v1, Pointer<Byte> &v2)
{
#if 0  // Rely on LLVM optimization
		If(condition)
		{
			Pointer<Byte> vX;

			vX = v2;
			v2 = v1;
			v1 = v0;
			v0 = vX;
		}
#else
	Pointer<Byte> vX = v2;
	v2 = IfThenElse(condition, v1, v2);
	v1 = IfThenElse(condition, v0, v1);
	v0 = IfThenElse(condition, vX, v0);
#endif
}

SetupFunction::RoutineType SetupRoutine::getRoutine()
{
	return routine;
}

}  // namespace sw
