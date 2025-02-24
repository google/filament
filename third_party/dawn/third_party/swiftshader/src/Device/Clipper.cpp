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

#include "Clipper.hpp"

#include "Polygon.hpp"
#include "Renderer.hpp"

namespace {

inline void clipEdge(sw::float4 &Vo, const sw::float4 &Vi, const sw::float4 &Vj, float di, float dj)
{
	float D = 1.0f / (dj - di);

	Vo.x = (dj * Vi.x - di * Vj.x) * D;
	Vo.y = (dj * Vi.y - di * Vj.y) * D;
	Vo.z = (dj * Vi.z - di * Vj.z) * D;
	Vo.w = (dj * Vi.w - di * Vj.w) * D;
}

void clipNear(sw::Polygon &polygon, bool depthClipNegativeOneToOne)
{
	const sw::float4 **V = polygon.P[polygon.i];
	const sw::float4 **T = polygon.P[polygon.i + 1];

	int t = 0;

	for(int i = 0; i < polygon.n; i++)
	{
		int j = i == polygon.n - 1 ? 0 : i + 1;

		float di = V[i]->z;
		float dj = V[j]->z;

		// When depthClipNegativeOneToOne is enabled the near plane is at z=-w, otherwise it is at z=0.
		if(depthClipNegativeOneToOne)
		{
			di += V[i]->w;
			dj += V[j]->w;
		}

		if(di >= 0)
		{
			T[t++] = V[i];

			if(dj < 0)
			{
				clipEdge(polygon.B[polygon.b], *V[i], *V[j], di, dj);
				T[t++] = &polygon.B[polygon.b++];
			}
		}
		else
		{
			if(dj > 0)
			{
				clipEdge(polygon.B[polygon.b], *V[j], *V[i], dj, di);
				T[t++] = &polygon.B[polygon.b++];
			}
		}
	}

	polygon.n = t;
	polygon.i += 1;
}

void clipFar(sw::Polygon &polygon)
{
	const sw::float4 **V = polygon.P[polygon.i];
	const sw::float4 **T = polygon.P[polygon.i + 1];

	int t = 0;

	for(int i = 0; i < polygon.n; i++)
	{
		int j = i == polygon.n - 1 ? 0 : i + 1;

		float di = V[i]->w - V[i]->z;
		float dj = V[j]->w - V[j]->z;

		if(di >= 0)
		{
			T[t++] = V[i];

			if(dj < 0)
			{
				clipEdge(polygon.B[polygon.b], *V[i], *V[j], di, dj);
				T[t++] = &polygon.B[polygon.b++];
			}
		}
		else
		{
			if(dj > 0)
			{
				clipEdge(polygon.B[polygon.b], *V[j], *V[i], dj, di);
				T[t++] = &polygon.B[polygon.b++];
			}
		}
	}

	polygon.n = t;
	polygon.i += 1;
}

void clipLeft(sw::Polygon &polygon)
{
	const sw::float4 **V = polygon.P[polygon.i];
	const sw::float4 **T = polygon.P[polygon.i + 1];

	int t = 0;

	for(int i = 0; i < polygon.n; i++)
	{
		int j = i == polygon.n - 1 ? 0 : i + 1;

		float di = V[i]->w + V[i]->x;
		float dj = V[j]->w + V[j]->x;

		if(di >= 0)
		{
			T[t++] = V[i];

			if(dj < 0)
			{
				clipEdge(polygon.B[polygon.b], *V[i], *V[j], di, dj);
				T[t++] = &polygon.B[polygon.b++];
			}
		}
		else
		{
			if(dj > 0)
			{
				clipEdge(polygon.B[polygon.b], *V[j], *V[i], dj, di);
				T[t++] = &polygon.B[polygon.b++];
			}
		}
	}

	polygon.n = t;
	polygon.i += 1;
}

void clipRight(sw::Polygon &polygon)
{
	const sw::float4 **V = polygon.P[polygon.i];
	const sw::float4 **T = polygon.P[polygon.i + 1];

	int t = 0;

	for(int i = 0; i < polygon.n; i++)
	{
		int j = i == polygon.n - 1 ? 0 : i + 1;

		float di = V[i]->w - V[i]->x;
		float dj = V[j]->w - V[j]->x;

		if(di >= 0)
		{
			T[t++] = V[i];

			if(dj < 0)
			{
				clipEdge(polygon.B[polygon.b], *V[i], *V[j], di, dj);
				T[t++] = &polygon.B[polygon.b++];
			}
		}
		else
		{
			if(dj > 0)
			{
				clipEdge(polygon.B[polygon.b], *V[j], *V[i], dj, di);
				T[t++] = &polygon.B[polygon.b++];
			}
		}
	}

	polygon.n = t;
	polygon.i += 1;
}

void clipTop(sw::Polygon &polygon)
{
	const sw::float4 **V = polygon.P[polygon.i];
	const sw::float4 **T = polygon.P[polygon.i + 1];

	int t = 0;

	for(int i = 0; i < polygon.n; i++)
	{
		int j = i == polygon.n - 1 ? 0 : i + 1;

		float di = V[i]->w - V[i]->y;
		float dj = V[j]->w - V[j]->y;

		if(di >= 0)
		{
			T[t++] = V[i];

			if(dj < 0)
			{
				clipEdge(polygon.B[polygon.b], *V[i], *V[j], di, dj);
				T[t++] = &polygon.B[polygon.b++];
			}
		}
		else
		{
			if(dj > 0)
			{
				clipEdge(polygon.B[polygon.b], *V[j], *V[i], dj, di);
				T[t++] = &polygon.B[polygon.b++];
			}
		}
	}

	polygon.n = t;
	polygon.i += 1;
}

void clipBottom(sw::Polygon &polygon)
{
	const sw::float4 **V = polygon.P[polygon.i];
	const sw::float4 **T = polygon.P[polygon.i + 1];

	int t = 0;

	for(int i = 0; i < polygon.n; i++)
	{
		int j = i == polygon.n - 1 ? 0 : i + 1;

		float di = V[i]->w + V[i]->y;
		float dj = V[j]->w + V[j]->y;

		if(di >= 0)
		{
			T[t++] = V[i];

			if(dj < 0)
			{
				clipEdge(polygon.B[polygon.b], *V[i], *V[j], di, dj);
				T[t++] = &polygon.B[polygon.b++];
			}
		}
		else
		{
			if(dj > 0)
			{
				clipEdge(polygon.B[polygon.b], *V[j], *V[i], dj, di);
				T[t++] = &polygon.B[polygon.b++];
			}
		}
	}

	polygon.n = t;
	polygon.i += 1;
}

}  // anonymous namespace

namespace sw {

bool Clipper::Clip(Polygon &polygon, int clipFlagsOr, const DrawCall &draw)
{
	if(clipFlagsOr & CLIP_FRUSTUM)
	{
		if(clipFlagsOr & CLIP_NEAR) clipNear(polygon, draw.depthClipNegativeOneToOne);
		if(polygon.n >= 3)
		{
			if(clipFlagsOr & CLIP_FAR) clipFar(polygon);
			if(polygon.n >= 3)
			{
				if(clipFlagsOr & CLIP_LEFT) clipLeft(polygon);
				if(polygon.n >= 3)
				{
					if(clipFlagsOr & CLIP_RIGHT) clipRight(polygon);
					if(polygon.n >= 3)
					{
						if(clipFlagsOr & CLIP_TOP) clipTop(polygon);
						if(polygon.n >= 3)
						{
							if(clipFlagsOr & CLIP_BOTTOM) clipBottom(polygon);
						}
					}
				}
			}
		}
	}

	return polygon.n >= 3;
}

}  // namespace sw
