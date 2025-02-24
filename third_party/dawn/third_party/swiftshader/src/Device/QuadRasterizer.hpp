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

#ifndef sw_QuadRasterizer_hpp
#define sw_QuadRasterizer_hpp

#include "Rasterizer.hpp"
#include "Pipeline/ShaderCore.hpp"
#include "Pipeline/SpirvShader.hpp"
#include "System/Types.hpp"

namespace sw {

class QuadRasterizer : public Rasterizer
{
public:
	QuadRasterizer(const PixelProcessor::State &state, const SpirvShader *spirvShader);
	virtual ~QuadRasterizer();

	void generate();

protected:
	Pointer<Byte> constants;

	// Fragment coordinates relative to the polygon's origin
	// TODO(b/236162233): Use SIMD::Float2
	SIMD::Float xFragment;
	SIMD::Float yFragment;

	// B * y + C term of interpolants plane equations
	SIMD::Float Dz[4];
	SIMD::Float Dw;
	SIMD::Float Dv[MAX_INTERFACE_COMPONENTS];
	SIMD::Float Df;
	SIMD::Float DclipDistance[MAX_CLIP_DISTANCES];
	SIMD::Float DcullDistance[MAX_CULL_DISTANCES];

	UInt occlusion;

	virtual void quad(Pointer<Byte> cBuffer[4], Pointer<Byte> &zBuffer, Pointer<Byte> &sBuffer, Int cMask[4], Int &x, Int &y) = 0;

	bool interpolateZ() const;
	bool interpolateW() const;
	SIMD::Float interpolate(SIMD::Float &x, SIMD::Float &D, SIMD::Float &rhw, Pointer<Byte> planeEquation, bool flat, bool perspective);

	const PixelProcessor::State &state;
	const SpirvShader *const spirvShader;

private:
	void rasterize(Int &yMin, Int &yMax);
};

}  // namespace sw

#endif  // sw_QuadRasterizer_hpp
