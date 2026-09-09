// RUN: %dxc -E main -T hs_6_0 %s | FileCheck %s

// CHECK: Rsqrt
// CHECK: dot3
// CHECK: Saturate
// CHECK: FMax
// CHECK: dot3
// CHECK: dot3
// CHECK: dot3
// CHECK: dot3
// CHECK: dot3
// CHECK: dot3
// CHECK: dot3
// CHECK: dot3
// CHECK: dot3
// CHECK: dot3
// CHEKC: FAbs
// CHECK: dot3
// CHECK: dot3
// CHECK: storePatchConstant
// CHECK: main
// CHECK: outputControlPointID
// CHECK: storeOutput

//--------------------------------------------------------------------------------------
// File: DecalTessellation.hlsl
//
// HLSL file containing shader functions for decal tessellation.
//
// Contributed by the AMD Developer Relations Team
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//--------------------------------------------------------------------------------------

#include "AdaptiveTessellation.hlsli"

#define MAX_DECALS 50
#define MIN_PRIM_SIZE 16
#define BACKFACE_EPSILON 0.25
//--------------------------------------------------------------------------------------
// Textures
//--------------------------------------------------------------------------------------
Texture2D g_DisplacementMap : register( t0 );        // Displacement map for the rendered object
Texture2D g_NormalMap : register( t1 );              // Normal map for the rendered object
Texture2D g_BaseMap : register( t2 );                // Base color map

//--------------------------------------------------------------------------------------
// Samplers
//--------------------------------------------------------------------------------------
SamplerState g_sampleLinear : register( s0 );

//--------------------------------------------------------------------------------------
// Constant Buffers
//--------------------------------------------------------------------------------------
cbuffer cbInit : register( b0 )
{
    float4  g_vMaterialColor;            // mesh color
    float4  g_vAmbientColor;             // mesh ambient color
    float4  g_vSpecularColor;            // mesh specular color
    float4  g_vScreenSize;               // x = screen width, y = screen height
    float4  g_vFlags;                    // miscellaneous flags
};

cbuffer cbUpdate : register( b1 )
{
    matrix    g_mWorld;                    // World matrix
    matrix    g_mViewProjection;           // ViewProjection matrix
    matrix    g_mWorldViewProjection;      // WVP matrix
    float4    g_vTessellationFactor;       // x = tessellation factor, z = backface culling, w = adaptive
    float4    g_vDisplacementScaleBias;    // Scale and bias of displacement
    float4    g_vLightPosition;            // 3D light position
    float4    g_vEyePosition;              // 3D world space eye position
};

cbuffer cbDamage : register( b2 )
{
    float4  g_vNormal[MAX_DECALS];              // tangent space normal
    float4  g_vBinormal[MAX_DECALS];            // tangent space binormal
    float4  g_vTangent[MAX_DECALS];             // tangent space tangent
    float4  g_vDecalPositionSize[MAX_DECALS];   // position and size of this decal
}                                                 

//--------------------------------------------------------------------------------------
// Structures
//--------------------------------------------------------------------------------------

// Tessellation vertex shader input
struct VS_INPUT
{
    float3 vPos            : POSITION;
    float3 vNormal        : NORMAL0;
    float2 vTexCoord    : TEXCOORD0;
};

// Tessellation vertex shader output, hull shader input
struct VS_OUTPUT_HS_INPUT
{
    float3 vPosWS        : POSITION;
    float2 vTexCoord    : TEXCOORD0;
    float3 vNormal        : NORMAL0;
};

// Patch constant hull shader output
struct HS_CONSTANT_DATA_OUTPUT
{
    float Edges[3]        : SV_TessFactor;
    float Inside        : SV_InsideTessFactor;
};

// Control point hull shader output
struct HS_CONTROL_POINT_OUTPUT
{
    float3 vWorldPos    : WORLDPOS;
    float2 vTexCoord    : TEXCOORD0;
    float3 vNormal        : NORMAL0;
};

// Domain shader (or no-tessellation vertex shader) output, pixel shader input
struct DS_VS_OUTPUT_PS_INPUT
{
    float4 vPosCS        : SV_POSITION;
    float2 vTexCoord    : TEXCOORD0;
    float3 vNormal        : NORMAL0;
    float3 vNMTexCoord    : TEXCOORD1;
    float3 vLightTS        : LIGHTVECTORTS;
    float3 vLightWS        : LIGHTVECTORWS;    // world space light vector for non-normal mapped pixels
    float3 vViewTS        : VIEWVECTORTS;
    float3 vViewWS        : VIEWVECTORWS;
};



//--------------------------------------------------------------------------------------
// Hull shader (patch constant phase)
// This shader is used for controling the tessellation factor of patch edges
//--------------------------------------------------------------------------------------
HS_CONSTANT_DATA_OUTPUT ConstantsHS( InputPatch<VS_OUTPUT_HS_INPUT, 3> p, uint PatchID : SV_PrimitiveID )
{
    HS_CONSTANT_DATA_OUTPUT Out;

    // Assign tessellation factors

    // unpack the tessellation factor constant vector
    float tessellationFactor = g_vTessellationFactor.x;
    float ScreenSpaceAdaptive = g_vTessellationFactor.y;
    float backFaceCulling = g_vTessellationFactor.z;
    float DisplacementAdaptive = g_vTessellationFactor.w;
    float tessellationFactor0 = tessellationFactor;
    float tessellationFactor1 = tessellationFactor;
    float tessellationFactor2 = tessellationFactor;
    
    // Get the triangle vertices passed into the hull shader
    // The vertex ordering needs to be consistant with the edge order.
    // In this case 0 is for the first edge, 1 for the second, 2 for the third.
    float3 vPos0 = p[1].vPosWS;
    float3 vPos1 = p[2].vPosWS;
    float3 vPos2 = p[0].vPosWS;
    float3 vEdge0 = vPos1 - vPos0;
    float3 vEdge2 = vPos2 - vPos0;
    
    if (backFaceCulling)
    {
        // calculate the triangle face normal with a cross product of the edges
        float3 vFaceNormal = normalize( cross(vEdge2,vEdge0) );
        float3 vView = normalize( vPos0 - g_vEyePosition.xyz );
        
        // A negative dot product means facing away from view direction
        // Compare with a small negative number so that displacements near
        // the edge are still rendered, since they may still be visible.
        if ( BackFaceCull( dot( vView, vFaceNormal), -1, -1, BACKFACE_EPSILON ) )
        {
            // back facing
            // Cull the triangle by setting the tessellation factors to 0.
            Out.Edges[0] = 0;
            Out.Edges[1] = 0;
            Out.Edges[2] = 0;
            Out.Inside   = 0;
            return Out; // exit early since this patch won't be rendered
        }
    }
    
    // Screen Space Adaptive Tessellation
    if (ScreenSpaceAdaptive)
    {
        float2 vScreenPos0 = GetScreenSpacePosition( vPos0, g_mViewProjection, g_vScreenSize.x, g_vScreenSize.y );
        float2 vScreenPos1 = GetScreenSpacePosition( vPos1, g_mViewProjection, g_vScreenSize.x, g_vScreenSize.y );
        float2 vScreenPos2 = GetScreenSpacePosition( vPos2, g_mViewProjection, g_vScreenSize.x, g_vScreenSize.y );
        float scale;
        scale = GetScreenSpaceAdaptiveScaleFactor(vScreenPos0, vScreenPos1, g_vTessellationFactor.x, MIN_PRIM_SIZE);
        tessellationFactor0 = g_vTessellationFactor.x * scale;
        scale = GetScreenSpaceAdaptiveScaleFactor(vScreenPos1, vScreenPos2, g_vTessellationFactor.x, MIN_PRIM_SIZE);
        tessellationFactor1 = g_vTessellationFactor.x * scale;
        scale = GetScreenSpaceAdaptiveScaleFactor(vScreenPos2, vScreenPos0, g_vTessellationFactor.x, MIN_PRIM_SIZE);
        tessellationFactor2 = g_vTessellationFactor.x * scale;
        tessellationFactor = max(tessellationFactor0, max(tessellationFactor1, tessellationFactor2));
    }

    if (!DisplacementAdaptive)
    {
        // For non-adaptive tessellation, just use a global tessellation factor
        Out.Edges[0] = tessellationFactor0;
        Out.Edges[1] = tessellationFactor1;
        Out.Edges[2] = tessellationFactor2;
        Out.Inside   = tessellationFactor;
        return Out; //exit early since all edges will have the same tessellation factor
    }
    
    // Displacement Adaptive Tessellation
    // The adaptive algroithm uses a distance to the mesh intersection to determine if tessellation
    // is needed. If the distance to the intersection is greater than the decal radius, then
    // render the patch but don't use tessellation (set tessellation factor to 1).
    
    // default tessellation factors
    Out.Edges[0] = 1;
    Out.Edges[1] = 1;
    Out.Edges[2] = 1;
    Out.Inside   = 1;
            
    // The distance calculation is based on the vector formula for the distance from a point to a line.
    // Given a line from point A to point B, and a point in space P, the perpendicular distance to the
    // line is found by projecting the vector from A to P onto the line A to B, or more accurately onto 
    // the vector formed by the line from A to B. A right triangle can be constructed by taking the line
    // from P to A as the hypotenuse, the projection of the hypotenuse onto the line AB as the second edge,
    // and the perpendicular line from the point P to the line AB as the third edge. The distance from the
    // point to the line is the distance of this third edge. The pythagorean theorm gives us the distance.
    // We can use the squared distance to avoid taking epensive square roots. Therefore,
    // 
    // distanceSquared = (squared length of PA) - (squared length of projection of PA onto AB)
    
    // pre-calculate some edges and squared magnitude of edges for later use
    float magSquaredEdge0 = (vEdge0.x * vEdge0.x) + (vEdge0.y * vEdge0.y) + (vEdge0.z * vEdge0.z);
    float3 vEdge1 = vPos2 - vPos1;
    float magSquaredEdge1 = (vEdge1.x * vEdge1.x) + (vEdge1.y * vEdge1.y) + (vEdge1.z * vEdge1.z);
    vEdge2 = -vEdge2; // reverse direction
    float magSquaredEdge2 = (vEdge2.x * vEdge2.x) + (vEdge2.y * vEdge2.y) + (vEdge2.z * vEdge2.z);
    float3 vEdgeA = vEdge0;
    float3 vEdgeB = -vEdge2;
    float dotAA = dot(vEdgeA, vEdgeA);
    float dotAB = dot(vEdgeA, vEdgeB);
    float dotBB = dot(vEdgeB, vEdgeB);
    float invDenom = 1.0 / (dotAA * dotBB - dotAB * dotAB);
    float3 vPlaneNormal = normalize( cross(vEdgeA, vEdgeB) );
    
    // Iterate over all of the decals to see if any are close enough to one of the 3 edges to tessellate
    for (int i = 0; i < MAX_DECALS; i++)
    {
        if (g_vNormal[i].x == 0.0 && g_vNormal[i].y == 0.0 && g_vNormal[i].z == 0.0)
            break;    // the rest of the list is empty
            
        float distanceSquared;
        bool edgeTessellated = false;
        float3 vProjected;
        float magSquaredProj;
        
        float3 vHitLocation;
        vHitLocation.x = g_vDecalPositionSize[i].x;
        vHitLocation.y = g_vDecalPositionSize[i].y;
        vHitLocation.z = g_vDecalPositionSize[i].z;
        
        float decalRadius = g_vDecalPositionSize[i].w;
        float decalRadiusSquared = decalRadius * decalRadius;
        
        // Create vectors from the hit location to all 3 vertices, then compute the squared distance (i.e. magnitude)
        float3 vHitEdge0 = vHitLocation - vPos0;
        float magSquaredHitEdge0 = (vHitEdge0.x * vHitEdge0.x) + (vHitEdge0.y * vHitEdge0.y) 
            + (vHitEdge0.z * vHitEdge0.z);
        float3 vHitEdge1 = vHitLocation - vPos1;
        float magSquaredHitEdge1 = (vHitEdge1.x * vHitEdge1.x) + (vHitEdge1.y * vHitEdge1.y) 
            + (vHitEdge1.z * vHitEdge1.z);
        float3 vHitEdge2 = vHitLocation - vPos2;
        float magSquaredHitEdge2 = (vHitEdge2.x * vHitEdge2.x) + (vHitEdge2.y * vHitEdge2.y) 
            + (vHitEdge2.z * vHitEdge2.z);
            
        
        // Edge 0
        // Check if the distance of the hit location to the edge endpoints is within the radius
        if ((magSquaredHitEdge0 <= decalRadiusSquared) || (magSquaredHitEdge1 <= decalRadiusSquared))
        {
            Out.Edges[0] = tessellationFactor0;
            edgeTessellated = true;
        }
        else
        {
            // If the distance from the hit location either of the endpoints is greater than the radius,
            // then part of the edge may still be within the a radius distance from the hit location. To
            // determine this we need to calculate the distance from the hit location to the edge.
            vProjected = (dot(vHitEdge0, vEdge0)/magSquaredEdge0) * vEdge0;        // create one edge of the right triangle
            // calculate the squared length of the edge
            magSquaredProj = (vProjected.x * vProjected.x) + (vProjected.y * vProjected.y)
                + (vProjected.z * vProjected.z);
                
            // Use the Pythagorean theorm to find the squared distance.
            distanceSquared = magSquaredHitEdge0 - magSquaredProj;
            
            // See if the distance sqared is less than or equal to the radius squared. Also
            // check to see if the the perpendicular distance is within the line segment. This
            // is done by testing the direction of the projection with the edge direction (negative
            // means it's on the line in the opposite direction. Also if the lenght of the projection
            // is greater than the edge, then the distance is measured to a point beyond the segment
            // in either case we don't want to tessellate.
            if ((distanceSquared <= decalRadiusSquared) && (dot(vProjected,vEdge0) >= 0)
                && (magSquaredProj <= magSquaredEdge0))
            {
                Out.Edges[0] = tessellationFactor0;
                edgeTessellated = true;
            }
        }

        // Edge 1
        // Same as Edge 0 - see comments above ...
        if ((magSquaredHitEdge1 <= decalRadiusSquared) || (magSquaredHitEdge2 <= decalRadiusSquared))
        {
            Out.Edges[1] = tessellationFactor1;
            edgeTessellated = true;
        }
        else
        {
            vProjected = (dot(vHitEdge1, vEdge1)/magSquaredEdge1) * vEdge1;
            magSquaredProj = (vProjected.x * vProjected.x) + (vProjected.y * vProjected.y)
                + (vProjected.z * vProjected.z);
            distanceSquared = magSquaredHitEdge1 - magSquaredProj;
            if ((distanceSquared <= decalRadiusSquared) && (dot(vProjected,vEdge1) >= 0)
                && (magSquaredProj <= magSquaredEdge1))
            {
                Out.Edges[1] = tessellationFactor1;
                edgeTessellated = true;
            }
        }
        
        // Edge 2
        // Same as Edge 0 - see comments above ...
        if ((magSquaredHitEdge2 <= decalRadiusSquared) || (magSquaredHitEdge0 <= decalRadiusSquared))
        {
            Out.Edges[2] = tessellationFactor2;
            edgeTessellated = true;
        }
        else
        {
            vProjected = (dot(vHitEdge2, vEdge2)/magSquaredEdge2) * vEdge2;
            magSquaredProj = (vProjected.x * vProjected.x) + (vProjected.y * vProjected.y)
                + (vProjected.z * vProjected.z);
            distanceSquared = magSquaredHitEdge2 - magSquaredProj;
            if ((distanceSquared <= decalRadiusSquared) && (dot(vProjected,vEdge2) >= 0)
                && (magSquaredProj <= magSquaredEdge2))
            {
                Out.Edges[2] = tessellationFactor2;
                edgeTessellated = true;
            }
        }
            
        // Inside
        //        There are 2 reasons to enable tessellation for the inside. One
        // reason is if any of the edges of the patch need tessellation. The
        // other reason is if the hit location is on the triangle, but the
        // radius is too small to touch any of the edges.
        //        First check to see if the distance from the hit location
        // to the plane formed by the triangle is within the decal radius.
        // Use the dot product to find the distance between a point and a plane
        float distanceToPlane = abs (dot(vPlaneNormal, vHitEdge0));
        
        // If the distance to the triangle plane is within the decal radius,
        // check to see if the intersection point is inside the triangle
        if (distanceToPlane <= decalRadius)
        {
            float dotAHit = dot(vEdgeA, vHitEdge0);
            float dotBHit = dot(vEdgeB, vHitEdge0);
            // Calculate barycentric coordinates to determine if the point is in the triangle
            float u = (dotBB * dotAHit - dotAB * dotBHit) * invDenom;
            float v = (dotAA * dotBHit - dotAB * dotAHit) * invDenom;
            if ( ((u > 0) && (v > 0) && ((u + v) < 1)) || edgeTessellated )
            {
                Out.Inside = tessellationFactor;
            }
        }
    }        


    return Out;
}


//--------------------------------------------------------------------------------------
// Hull shader (control point phase)
//--------------------------------------------------------------------------------------
[domain("tri")]
[partitioning("fractional_odd")]
[outputtopology("triangle_cw")]
[outputcontrolpoints(3)]
[patchconstantfunc("ConstantsHS")]
[maxtessfactor(64.0)]
HS_CONTROL_POINT_OUTPUT main( InputPatch<VS_OUTPUT_HS_INPUT, 3> inputPatch, 
                            uint uCPID : SV_OutputControlPointID )
{
    HS_CONTROL_POINT_OUTPUT Out;
    
    // Copy inputs to outputs
    Out.vWorldPos = inputPatch[uCPID].vPosWS.xyz;
    Out.vTexCoord = inputPatch[uCPID].vTexCoord;
    Out.vNormal = inputPatch[uCPID].vNormal;
    
    return Out;
}

