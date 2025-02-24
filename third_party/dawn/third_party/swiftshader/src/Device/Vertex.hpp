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

#ifndef Vertex_hpp
#define Vertex_hpp

#include "Device/Config.hpp"
#include "System/Types.hpp"

namespace sw {

struct alignas(16) Vertex
{
	union
	{
		struct
		{
			float x;
			float y;
			float z;
			float w;
		};

		float4 position;
	};

	float pointSize;

	int clipFlags;
	int cullMask;
	float clipDistance[MAX_CLIP_DISTANCES];
	float cullDistance[MAX_CLIP_DISTANCES];

	alignas(16) struct
	{
		int x;
		int y;
		float z;
		float w;
	} projected;

	alignas(16) float v[MAX_INTERFACE_COMPONENTS];
};

static_assert((sizeof(Vertex) & 0x0000000F) == 0, "Vertex size not a multiple of 16 bytes (alignment requirement)");

}  // namespace sw

#endif  // Vertex_hpp
