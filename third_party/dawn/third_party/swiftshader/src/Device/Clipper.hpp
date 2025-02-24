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

#ifndef sw_Clipper_hpp
#define sw_Clipper_hpp

#include "System/Types.hpp"

namespace sw {

struct DrawCall;
struct Polygon;

struct Clipper
{
	enum ClipFlags
	{
		// Indicates the vertex is outside the respective frustum plane
		CLIP_RIGHT = 1 << 0,
		CLIP_TOP = 1 << 1,
		CLIP_FAR = 1 << 2,
		CLIP_LEFT = 1 << 3,
		CLIP_BOTTOM = 1 << 4,
		CLIP_NEAR = 1 << 5,

		CLIP_SIDES = CLIP_LEFT | CLIP_RIGHT | CLIP_BOTTOM | CLIP_TOP,
		CLIP_FRUSTUM = CLIP_SIDES | CLIP_NEAR | CLIP_FAR,

		CLIP_FINITE = 1 << 7,  // All position coordinates are finite
	};

	static bool Clip(Polygon &polygon, int clipFlagsOr, const DrawCall &draw);
};

}  // namespace sw

#endif  // sw_Clipper_hpp
