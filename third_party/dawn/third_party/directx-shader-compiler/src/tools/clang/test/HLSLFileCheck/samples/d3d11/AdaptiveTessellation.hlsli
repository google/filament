//--------------------------------------------------------------------------------------
// File: AdaptiveTessellation.hlsl
//
// These utility functions are typically called by the hull shader patch constant function
// in order to apply adaptive levels of tessellation, and cull patches altogether if
// back facing or outside the view frustum
//
// Contributed by the AMD Developer Relations Team
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//--------------------------------------------------------------------------------------


//--------------------------------------------------------------------------------------
// Returns the dot product between the viewing vector and the patch edge
//--------------------------------------------------------------------------------------
float GetEdgeDotProduct ( 
                        float3 f3EdgeNormal0,   // Normalized normal of the first control point of the given patch edge 
                        float3 f3EdgeNormal1,   // Normalized normal of the second control point of the given patch edge 
                        float3 f3ViewVector     // Normalized viewing vector
                        )
{
    float3 f3EdgeNormal = normalize( ( f3EdgeNormal0 + f3EdgeNormal1 ) * 0.5f );
    
    float fEdgeDotProduct = dot( f3EdgeNormal, f3ViewVector );

    return fEdgeDotProduct;
}


//--------------------------------------------------------------------------------------
// Returns the screen space position from the given world space patch control point
//--------------------------------------------------------------------------------------
float2 GetScreenSpacePosition   ( 
                                float3 f3Position,              // World space position of patch control point
                                float4x4 f4x4ViewProjection,    // View x Projection matrix
                                float fScreenWidth,             // Screen width
                                float fScreenHeight             // Screen height
                                )
{
    float4 f4ProjectedPosition = mul( float4( f3Position, 1.0f ), f4x4ViewProjection );
    
    float2 f2ScreenPosition = f4ProjectedPosition.xy / f4ProjectedPosition.ww;
    
    f2ScreenPosition = ( f2ScreenPosition + 1.0f ) * 0.5f * float2( fScreenWidth, -fScreenHeight );

    return f2ScreenPosition;
}


//--------------------------------------------------------------------------------------
// Returns the distance of a given point from a given plane
//--------------------------------------------------------------------------------------
float DistanceFromPlane ( 
                        float3 f3Position,      // World space position of the patch control point
                        float4 f4PlaneEquation  // Plane equation of a frustum plane
                        )
{
    float fDistance = dot( float4( f3Position, 1.0f ), f4PlaneEquation );
    
    return fDistance;
}


//--------------------------------------------------------------------------------------
// Returns a distance adaptive tessellation scale factor (0.0f -> 1.0f) 
//--------------------------------------------------------------------------------------
float GetDistanceAdaptiveScaleFactor(    
                                    float3 f3Eye,           // Position of the camera/eye
                                    float3 f3EdgePosition0, // Position of the first control point of the given patch edge
                                    float3 f3EdgePosition1, // Position of the second control point of the given patch edge
                                    float fMinDistance,     // Minimum distance that maximum tessellation factors should be applied at
                                    float fRange            // Range beyond the minimum distance where tessellation will scale down to the minimum scaling factor    
                                    )
{
    float3 f3MidPoint = ( f3EdgePosition0 + f3EdgePosition1 ) * 0.5f;

    float fDistance = distance( f3MidPoint, f3Eye ) - fMinDistance;
        
    float fScale = 1.0f - saturate( fDistance / fRange );

    return fScale;
}


//--------------------------------------------------------------------------------------
// Returns the orientation adaptive tessellation factor (0.0f -> 1.0f)
//--------------------------------------------------------------------------------------
float GetOrientationAdaptiveScaleFactor ( 
                                        float fEdgeDotProduct,      // Dot product of edge normal with view vector
                                        float fSilhouetteEpsilon    // Epsilon to determine the range of values considered to be silhoutte
                                        )
{
    float fScale = 1.0f - abs( fEdgeDotProduct );
        
    fScale = saturate( ( fScale - fSilhouetteEpsilon ) / ( 1.0f - fSilhouetteEpsilon ) );

    return fScale;
}


//--------------------------------------------------------------------------------------
// Returns the screen resolution adaptive tessellation scale factor (0.0f -> 1.0f)
//--------------------------------------------------------------------------------------
float GetScreenResolutionAdaptiveScaleFactor( 
                                            float fCurrentWidth,    // Current render window width 
                                            float fCurrentHeight,   // Current render window height 
                                            float fMaxWidth,        // Width considered to be max
                                            float fMaxHeight        // Height considered to be max
                                            )
{
    float fMaxArea = fMaxWidth * fMaxHeight;
    
    float fCurrentArea = fCurrentWidth * fCurrentHeight;

    float fScale = saturate( fCurrentArea / fMaxArea );

    return fScale;
}


//--------------------------------------------------------------------------------------
// Returns the screen space adaptive tessellation scale factor (0.0f -> 1.0f)
//--------------------------------------------------------------------------------------
float GetScreenSpaceAdaptiveScaleFactor (
                                        float2 f2EdgeScreenPosition0,   // Screen coordinate of the first patch edge control point
                                        float2 f2EdgeScreenPosition1,   // Screen coordinate of the second patch edge control point    
                                        float fMaxEdgeTessFactor,       // Maximum edge tessellation factor                            
                                        float fTargetEdgePrimitiveSize  // Desired primitive edge size in pixels
                                        )
{
    float fEdgeScreenLength = distance( f2EdgeScreenPosition0, f2EdgeScreenPosition1 );

    float fTargetTessFactor = fEdgeScreenLength / fTargetEdgePrimitiveSize;

    fTargetTessFactor /= fMaxEdgeTessFactor;
    
    float fScale = saturate( fTargetTessFactor );
    
    return fScale;
}


//--------------------------------------------------------------------------------------
// Returns back face culling test result (true / false)
//--------------------------------------------------------------------------------------
bool BackFaceCull    ( 
                    float fEdgeDotProduct0, // Dot product of edge 0 normal with view vector
                    float fEdgeDotProduct1, // Dot product of edge 1 normal with view vector
                    float fEdgeDotProduct2, // Dot product of edge 2 normal with view vector
                    float fBackFaceEpsilon  // Epsilon to determine cut off value for what is considered back facing
                    )
{
    float3 f3BackFaceCull;
    
    f3BackFaceCull.x = ( fEdgeDotProduct0 > -fBackFaceEpsilon ) ? ( 0.0f ) : ( 1.0f );
    f3BackFaceCull.y = ( fEdgeDotProduct1 > -fBackFaceEpsilon ) ? ( 0.0f ) : ( 1.0f );
    f3BackFaceCull.z = ( fEdgeDotProduct2 > -fBackFaceEpsilon ) ? ( 0.0f ) : ( 1.0f );
    
    return all( f3BackFaceCull );
}


//--------------------------------------------------------------------------------------
// Returns view frustum Culling test result (true / false)
//--------------------------------------------------------------------------------------
bool ViewFrustumCull(
                    float3 f3EdgePosition0,         // World space position of patch control point 0
                    float3 f3EdgePosition1,         // World space position of patch control point 1
                    float3 f3EdgePosition2,         // World space position of patch control point 2
                    float4 f4ViewFrustumPlanes[4],  // 4 plane equations (left, right, top, bottom)
                    float fCullEpsilon              // Epsilon to determine the distance outside the view frustum is still considered inside
                    )
{    
    float4 f4PlaneTest;
    float fPlaneTest;
    
    // Left clip plane
    f4PlaneTest.x = ( ( DistanceFromPlane( f3EdgePosition0, f4ViewFrustumPlanes[0]) > -fCullEpsilon ) ? 1.0f : 0.0f ) +
                    ( ( DistanceFromPlane( f3EdgePosition1, f4ViewFrustumPlanes[0]) > -fCullEpsilon ) ? 1.0f : 0.0f ) +
                    ( ( DistanceFromPlane( f3EdgePosition2, f4ViewFrustumPlanes[0]) > -fCullEpsilon ) ? 1.0f : 0.0f );
    // Right clip plane
    f4PlaneTest.y = ( ( DistanceFromPlane( f3EdgePosition0, f4ViewFrustumPlanes[1]) > -fCullEpsilon ) ? 1.0f : 0.0f ) +
                    ( ( DistanceFromPlane( f3EdgePosition1, f4ViewFrustumPlanes[1]) > -fCullEpsilon ) ? 1.0f : 0.0f ) +
                    ( ( DistanceFromPlane( f3EdgePosition2, f4ViewFrustumPlanes[1]) > -fCullEpsilon ) ? 1.0f : 0.0f );
    // Top clip plane
    f4PlaneTest.z = ( ( DistanceFromPlane( f3EdgePosition0, f4ViewFrustumPlanes[2]) > -fCullEpsilon ) ? 1.0f : 0.0f ) +
                    ( ( DistanceFromPlane( f3EdgePosition1, f4ViewFrustumPlanes[2]) > -fCullEpsilon ) ? 1.0f : 0.0f ) +
                    ( ( DistanceFromPlane( f3EdgePosition2, f4ViewFrustumPlanes[2]) > -fCullEpsilon ) ? 1.0f : 0.0f );
    // Bottom clip plane
    f4PlaneTest.w = ( ( DistanceFromPlane( f3EdgePosition0, f4ViewFrustumPlanes[3]) > -fCullEpsilon ) ? 1.0f : 0.0f ) +
                    ( ( DistanceFromPlane( f3EdgePosition1, f4ViewFrustumPlanes[3]) > -fCullEpsilon ) ? 1.0f : 0.0f ) +
                    ( ( DistanceFromPlane( f3EdgePosition2, f4ViewFrustumPlanes[3]) > -fCullEpsilon ) ? 1.0f : 0.0f );
        
    // Triangle has to pass all 4 plane tests to be visible
    return !all( f4PlaneTest );
}


//--------------------------------------------------------------------------------------
// EOF
//--------------------------------------------------------------------------------------
