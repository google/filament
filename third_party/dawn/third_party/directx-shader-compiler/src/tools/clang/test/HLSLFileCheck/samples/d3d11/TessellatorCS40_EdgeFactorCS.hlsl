// RUN: %dxc -E main -T cs_6_0 %s | FileCheck %s

// CHECK: threadId
// CHECK: bufferLoad
// CHECK: dot2
// CHECK: dot2
// CHECK: FAbs
// CHECK: FMin
// CHECK: FMax
// CHECK: dot3
// CHECK: dot3
// CHECK: dot3
// CHECK: Sqrt
// CHECK: bufferStore

//--------------------------------------------------------------------------------------
// File: TessellatorCS40_EdgeFactorCS.hlsl
//
// The CS to compute edge tessellation factor acoording to current world, view, projection matrix
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//--------------------------------------------------------------------------------------

// http://jgt.akpeters.com/papers/akeninemoller01/tribox.html
bool planeBoxOverlap(float3 normal, float d, float3 maxbox)
{
    float3 vmin = maxbox, vmax = maxbox;
    [unroll]
    for (int q = 0;q <= 2; ++ q)
    {
        if (normal[q] > 0.0f)
        {
            vmin[q] *= -1;
        }
        else
        {
            vmax[q] *= -1;
        }
    }
    if (dot(normal, vmin) + d > 0.0f)
    {
        return false;
    }
    if (dot(normal, vmax) + d >= 0.0f)
    {
        return true;
    }

    return false;
}

/*======================== X-tests ========================*/
bool AXISTEST_X01(float3 v0, float3 v2, float3 boxhalfsize, float2 ab, float2 fab)
{
    float p0 = ab.x * v0.y - ab.y * v0.z;
    float p2 = ab.x * v2.y - ab.y * v2.z;
    float min_v = min(p0, p2);
    float max_v = max(p0, p2);
    float rad = dot(fab, boxhalfsize.yz);
    return (min_v < rad) && (max_v > -rad);
}

bool AXISTEST_X2(float3 v0, float3 v1, float3 boxhalfsize, float2 ab, float2 fab)
{
    float p0 = ab.x * v0.y - ab.y * v0.z;
    float p1 = ab.x * v1.y - ab.y * v1.z;
    float min_v = min(p0, p1);
    float max_v = max(p0, p1);
    float rad = dot(fab, boxhalfsize.yz);
    return (min_v < rad) && (max_v > -rad);
}

/*======================== Y-tests ========================*/
bool AXISTEST_Y02(float3 v0, float3 v2, float3 boxhalfsize, float2 ab, float2 fab)
{
    float p0 = -ab.x * v0.x + ab.y * v0.z;
    float p2 = -ab.x * v2.x + ab.y * v2.z;
    float min_v = min(p0, p2);
    float max_v = max(p0, p2);
    float rad = dot(fab, boxhalfsize.xz);
    return (min_v < rad) && (max_v > -rad);
}

bool AXISTEST_Y1(float3 v0, float3 v1, float3 boxhalfsize, float2 ab, float2 fab)
{
    float p0 = -ab.x * v0.x + ab.y * v0.z;
    float p1 = -ab.x * v1.x + ab.y * v1.z;
    float min_v = min(p0, p1);
    float max_v = max(p0, p1);
    float rad = dot(fab, boxhalfsize.xz);
    return (min_v < rad) && (max_v > -rad);
}

/*======================== Z-tests ========================*/
bool AXISTEST_Z12(float3 v1, float3 v2, float3 boxhalfsize, float2 ab, float2 fab)
{
    float p1 = ab.x * v1.x - ab.y * v1.y;
    float p2 = ab.x * v2.x - ab.y * v2.y;
    float min_v = min(p1, p2);
    float max_v = max(p1, p2);
    float rad = dot(fab, boxhalfsize.xy);
    return (min_v < rad) && (max_v > -rad);
}

bool AXISTEST_Z0(float3 v0, float3 v1, float3 boxhalfsize, float2 ab, float2 fab)
{
    float p0 = ab.x * v0.x - ab.y * v0.y;
    float p1 = ab.x * v1.x - ab.y * v1.y;
    float min_v = min(p0, p1);
    float max_v = max(p0, p1);
    float rad = dot(fab, boxhalfsize.xy);
    return (min_v < rad) && (max_v > -rad);
}

bool triBoxOverlap(float3 boxcenter,float3 boxhalfsize,float3 triverts0, float3 triverts1, float3 triverts2)
{
    /*    use separating axis theorem to test overlap between triangle and box */
    /*    need to test for overlap in these directions: */
    /*    1) the {x,y,z}-directions (actually, since we use the AABB of the triangle */
    /*       we do not even need to test these) */
    /*    2) normal of the triangle */
    /*    3) crossproduct(edge from tri, {x,y,z}-directin) */
    /*       this gives 3x3=9 more tests */

    /* This is the fastest branch on Sun */
    /* move everything so that the boxcenter is in (0,0,0) */
    float3 v0 = triverts0 - boxcenter;
    float3 v1 = triverts1 - boxcenter;
    float3 v2 = triverts2 - boxcenter;

    /* compute triangle edges */
    float3 e0 = v1 - v0;      /* tri edge 0 */
    float3 e1 = v2 - v1;      /* tri edge 1 */
    float3 e2 = v0 - v2;      /* tri edge 2 */

    /* Bullet 3:  */
    /*  test the 9 tests first (this was faster) */
    float3 fe = abs(e0);
    if (!AXISTEST_X01(v0, v2, boxhalfsize, e0.zy, fe.zy)
        || !AXISTEST_Y02(v0, v2, boxhalfsize, e0.zx, fe.zx)
        || !AXISTEST_Z12(v1, v2, boxhalfsize, e0.yx, fe.yx))
    {
        return false;
    }

    fe = abs(e1);
    if (!AXISTEST_X01(v0, v2, boxhalfsize, e1.zy, fe.zy)
        || !AXISTEST_Y02(v0, v2, boxhalfsize, e1.zx, fe.zx)
        || !AXISTEST_Z0(v0, v1, boxhalfsize, e1.yx, fe.yx))
    {
        return false;
    }

    fe = abs(e2);
    if (!AXISTEST_X2(v0, v1, boxhalfsize, e2.zy, fe.zy)
        || !AXISTEST_Y1(v0, v1, boxhalfsize, e2.zx, fe.zx)
        || !AXISTEST_Z12(v1, v2, boxhalfsize, e2.yx, fe.yx))
    {
        return false;
    }

    /* Bullet 1: */
    /*  first test overlap in the {x,y,z}-directions */
    /*  find min, max of the triangle each direction, and test for overlap in */
    /*  that direction -- this is equivalent to testing a minimal AABB around */
    /*  the triangle against the AABB */

    float3 min_v = min(min(v0, v1), v2);
    float3 max_v = max(max(v0, v1), v2);
    if ((min_v.x > boxhalfsize.x || max_v.x < -boxhalfsize.x)
        || (min_v.y > boxhalfsize.y || max_v.y < -boxhalfsize.y)
        || (min_v.z > boxhalfsize.z || max_v.z < -boxhalfsize.z))
    {
        return false;
    }

    /* Bullet 2: */
    /*  test if the box intersects the plane of the triangle */
    /*  compute plane equation of triangle: normal*x+d=0 */
    float3 normal = cross(e0, e1);
    float d = -dot(normal, v0);  /* plane eq: normal.x+d=0 */
    if (!planeBoxOverlap(normal, d, boxhalfsize))
    {
        return false;
    }

    return true;   /* box and triangle overlaps */
}


Buffer<float4> InputVertices : register(t0);
RWStructuredBuffer<float4> EdgeFactorBufOut : register(u0);

cbuffer cb
{
    row_major matrix    g_matWVP;
    float2              g_tess_edge_length_scale;
    int                 num_triangles;
    float               dummy;
}

[numthreads(128, 1, 1)]
void main( uint3 DTid : SV_DispatchThreadID )
{
    if (DTid.x < num_triangles)
    {
        float4 p0 = mul(InputVertices[DTid.x*3+0], g_matWVP);
        float4 p1 = mul(InputVertices[DTid.x*3+1], g_matWVP);
        float4 p2 = mul(InputVertices[DTid.x*3+2], g_matWVP);
        p0 = p0 / p0.w;
        p1 = p1 / p1.w;
        p2 = p2 / p2.w;

        float4 factor;
        // Only triangles which are completely inside or intersect with the view frustum are taken into account 
        if ( triBoxOverlap( float3(0, 0, 0.5), float3(1.02, 1.02, 0.52), p0.xyz, p1.xyz, p2.xyz ) )
        {
            factor.x = length((p0.xy - p2.xy) * g_tess_edge_length_scale);
            factor.y = length((p1.xy - p0.xy) * g_tess_edge_length_scale);
            factor.z = length((p2.xy - p1.xy) * g_tess_edge_length_scale);
            factor.w = min(min(factor.x, factor.y), factor.z);
            factor = clamp(factor, 0, 9);
        } else
        {
            factor = 0;
        }

        EdgeFactorBufOut[DTid.x] = factor;
    }
}
