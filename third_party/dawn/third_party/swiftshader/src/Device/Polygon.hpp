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

#ifndef sw_Polygon_hpp
#define sw_Polygon_hpp

namespace sw {

struct Polygon
{
	Polygon(const float4 *P0, const float4 *P1, const float4 *P2)
	{
		P[0][0] = P0;
		P[0][1] = P1;
		P[0][2] = P2;

		n = 3;
		i = 0;
		b = 0;
	}

	Polygon(const float4 *P, int n)
	{
		for(int i = 0; i < n; i++)
		{
			this->P[0][i] = &P[i];
		}

		this->n = n;
		this->i = 0;
		this->b = 0;
	}

	float4 B[16];             // Buffer for clipped vertices
	const float4 *P[16][16];  // Pointers to clipped polygon's vertices

	int n;  // Number of vertices
	int i;  // Level of P to use
	int b;  // Next available new vertex
};

}  // namespace sw

#endif  // sw_Polygon_hpp
