//------------------------------------------------------------------------------
// Screen-space reflections
//------------------------------------------------------------------------------

#if defined(HAS_REFLECTIONS) && REFLECTION_MODE == REFLECTION_MODE_SCREEN_SPACE

// Copied from depthUtils.fs
highp float linearizeDepth(highp float depth) {
    // Our far plane is at infinity, which causes a division by zero below, which in turn
    // causes some issues on some GPU. We workaround it by replacing "infinity" by the closest
    // value representable in  a 24 bit depth buffer.
    const highp float preventDiv0 = 1.0 / 16777216.0;
    mat4 p = getViewFromClipMatrix();
    // this works with perspective and ortho projections, for a perspective projection
    // this resolves to -near/depth, for an ortho projection this resolves to depth*(far - near) - far
    return (depth * p[2].z + p[3].z) / max(depth * p[2].w + p[3].w, preventDiv0);
}

// Code adapted from McGuire and Mara, Efficient GPU Screen-Space Ray Tracing, Journal of Computer
// Graphics Techniques, 2014
//
// Copyright (c) 2014, Morgan McGuire and Michael Mara
// All rights reserved.
//
// This software is open source under the "BSD 2-clause license":
//
//    Redistribution and use in source and binary forms, with or without modification, are permitted
//    provided that the following conditions are met:
//
//    1. Redistributions of source code must retain the above copyright notice, this list of
//    conditions and the following disclaimer.
//
//    2. Redistributions in binary form must reproduce the above copyright notice, this list of
//    conditions and the following disclaimer in the documentation and/or other materials provided
//    with the distribution.
//
//    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR
//    IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY
//    AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR
//    CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
//    CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
//    SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
//    THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
//    OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
//    POSSIBILITY OF SUCH DAMAGE.

#define Point3 vec3
#define Point2 vec2
#define Vector2 vec2
#define Vector3 vec3
#define Vector4 vec4

void swap(inout float a, inout float b) {
     float temp = a;
     a = b;
     b = temp;
}

float distanceSquared(vec2 a, vec2 b) {
    a -= b;
    return dot(a, a);
}

// Note: McGuire and Mara use the "cs" prefix to stand for "camera space", equivalent to Filament's
// "view space". "cs" has been replaced with "vs" to avoid confusion.
bool traceScreenSpaceRay(Point3 vsOrigin, Vector3 vsDirection, mat4x4 projectToPixelMatrix,
        sampler2D vsZBuffer, float vsZThickness, float nearPlaneZ, float stride,
        float jitterFraction, float maxSteps, in float maxRayTraceDistance, out Point2 hitPixel,
        out Point3 vsHitPoint) {

    // Clip ray to a near plane in 3D (doesn't have to be *the* near plane, although that would be a
    // good idea)
    float rayLength = ((vsOrigin.z + vsDirection.z * maxRayTraceDistance) > nearPlaneZ) ?
                        (nearPlaneZ - vsOrigin.z) / vsDirection.z :
                        maxRayTraceDistance;
    Point3 vsEndPoint = vsDirection * rayLength + vsOrigin;

    // Project into screen space
    Vector4 H0 = projectToPixelMatrix * Vector4(vsOrigin, 1.0);
    Vector4 H1 = projectToPixelMatrix * Vector4(vsEndPoint, 1.0);

    // There are a lot of divisions by w that can be turned into multiplications at some minor
    // precision loss...and we need to interpolate these 1/w values anyway.
    //
    // Because the caller was required to clip to the near plane, this homogeneous division
    // (projecting from 4D to 2D) is guaranteed to succeed.
    float k0 = 1.0 / H0.w;
    float k1 = 1.0 / H1.w;

    // Switch the original points to values that interpolate linearly in 2D
    Point3 Q0 = vsOrigin * k0;
    Point3 Q1 = vsEndPoint * k1;

    // Screen-space endpoints
    Point2 P0 = H0.xy * k0;
    Point2 P1 = H1.xy * k1;

    // TODO:
    // [Optional clipping to frustum sides here]

    // Initialize to off screen
    hitPixel = Point2(-1.0, -1.0);

    // If the line is degenerate, make it cover at least one pixel to avoid handling zero-pixel
    // extent as a special case later
    P1 += vec2((distanceSquared(P0, P1) < 0.0001) ? 0.01 : 0.0);

    Vector2 delta = P1 - P0;

    // Permute so that the primary iteration is in x to reduce large branches later
    bool permute = false;
    if (abs(delta.x) < abs(delta.y)) {
        // More-vertical line. Create a permutation that swaps x and y in the output
        permute = true;
        // Directly swizzle the inputs
        delta = delta.yx;
        P1 = P1.yx;
        P0 = P0.yx;
    }

    // From now on, "x" is the primary iteration direction and "y" is the secondary one

    float stepDirection = sign(delta.x);
    float invdx = stepDirection / delta.x;
    Vector2 dP = Vector2(stepDirection, invdx * delta.y);

    // Track the derivatives of Q and k
    Vector3 dQ = (Q1 - Q0) * invdx;
    float   dk = (k1 - k0) * invdx;

    // Scale derivatives by the desired pixel stride
    dP *= stride; dQ *= stride; dk *= stride;

    // Offset the starting values by the jitter fraction
    P0 += dP * jitterFraction; Q0 += dQ * jitterFraction; k0 += dk * jitterFraction;

    // Slide P from P0 to P1, (now-homogeneous) Q from Q0 to Q1, and k from k0 to k1
    Point3 Q = Q0;
    float  k = k0;

    // We track the ray depth at +/- 1/2 pixel to treat pixels as clip-space solid voxels. Because
    // the depth at -1/2 for a given pixel will be the same as at +1/2 for the previous iteration,
    // we actually only have to compute one value per iteration.
    float prevZMaxEstimate = vsOrigin.z;
    float stepCount = 0.0;
    float rayZMax = prevZMaxEstimate, rayZMin = prevZMaxEstimate;
    float sceneZMax = rayZMax + 1e4;

    // P1.x is never modified after this point, so pre-scale it by the step direction for a signed
    // comparison
    float end = P1.x * stepDirection;

    // We only advance the z field of Q in the inner loop, since Q.xy is never used until after the
    // loop terminates.

    for (Point2 P = P0;
        ((P.x * stepDirection) <= end) &&
        (stepCount < maxSteps) &&
        ((rayZMax < sceneZMax - vsZThickness) ||
            (rayZMin > sceneZMax)) &&
        (sceneZMax != 0.0);
        P += dP, Q.z += dQ.z, k += dk, stepCount += 1.0) {

        hitPixel = permute ? P.yx : P;

        // The depth range that the ray covers within this loop iteration.  Assume that the ray is
        // moving in increasing z and swap if backwards.  Because one end of the interval is shared
        // between adjacent iterations, we track the previous value and then swap as needed to
        // ensure correct ordering
        rayZMin = prevZMaxEstimate;

        // Compute the value at 1/2 pixel into the future
        rayZMax = (dQ.z * 0.5 + Q.z) / (dk * 0.5 + k);
        prevZMaxEstimate = rayZMax;
        if (rayZMin > rayZMax) { swap(rayZMin, rayZMax); }

        // View-space z of the background
        sceneZMax = linearizeDepth(texelFetch(vsZBuffer, int2(hitPixel), 0).r);
    } // pixel on ray

    Q.xy += dQ.xy * stepCount;
    vsHitPoint = Q * (1.0 / k);
    // Matches the new loop condition:
    return (rayZMax >= sceneZMax - vsZThickness) && (rayZMin <= sceneZMax);
}

#undef Point3
#undef Point2
#undef Vector2
#undef Vector3
#undef Vector4

// -- end "BSD 2-clause license" -------------------------------------------------------------------

mat4x4 scaleMatrix(float x, float y) {
    mat4x4 m = mat4(1.0f);
    m[0].x = x;
    m[1].y = y;
    m[2].z = 1.0f;
    m[3].w = 1.0f;
    return m;
}

mat4x4 translateMatrix(float x, float y) {
    mat4x4 m = mat4(1.0f);
    m[3].xy = vec2(x, y);
    return m;
}

// random number between 0 and 1, using interleaved gradient noise
float ssr_random(const highp vec2 w) {
    const vec3 m = vec3(0.06711056, 0.00583715, 52.9829189);
    return fract(m.z * fract(dot(w, m.xy)));
}

/**
 * Evaluates screen-space reflections, storing the color in Fr if there's a hit.
 * r is the desired reflected vector.
 * Returns true if there's a hit, false otherwise.
 */
bool evaluateScreenSpaceReflections(highp vec3 r, inout vec3 Fr) {
    highp vec3 wsRayDirection = r;
    highp vec3 wsRayStart = shading_position + frameUniforms.ssrBias * wsRayDirection;

    // ray start/end in view space
    highp vec4 vsRayStart = getViewFromWorldMatrix() * vec4(wsRayStart, 1.0);
    highp vec4 vsRayDirection = getViewFromWorldMatrix() * vec4(wsRayDirection, 0.0);

    highp vec3 vsOrigin = vsRayStart.xyz;
    highp vec3 vsDirection = vsRayDirection.xyz;
    highp float vsZThickness = frameUniforms.ssrThickness;
    // TODO: use the actual near plane.
    highp float nearPlaneZ = -0.1f;
    highp float stride = 1.0;
    // TODO: jitterFraction should be a between 0 and 1, but anything < 1 gives banding artifacts.
    highp float jitterFraction = 1.0f;
    // TODO: set this to the larger of the viewport dimensions.
    highp float maxSteps = 1000.0;
    highp float maxRayTraceDistance = frameUniforms.ssrDistance;

    vec2 res = vec2(textureSize(light_structure, 0).xy);
    highp mat4x4 projectToPixelMatrix =
        scaleMatrix(res.x, res.y) *
        translateMatrix(0.5f, 0.5f) *
#if defined(TARGET_METAL_ENVIRONMENT) || defined(TARGET_VULKAN_ENVIRONMENT)
        scaleMatrix(0.5f, -0.5f) *  // Metal and Vulkan's texture space is y-inverted
#else
        scaleMatrix(0.5f, 0.5f) *
#endif
        getClipFromViewMatrix();

    // Outputs from the traceScreenSpaceRay function.
    vec2 hitPixel;  // not currently used
    vec3 vsHitPoint;

    if (traceScreenSpaceRay(vsOrigin, vsDirection, projectToPixelMatrix, light_structure,
            vsZThickness, nearPlaneZ, stride, jitterFraction, maxSteps,
            maxRayTraceDistance, hitPixel, vsHitPoint)) {
        vec2 ssrRes = vec2(textureSize(light_ssr, 0).xy);
        vec4 reprojected = scaleMatrix(ssrRes.x, ssrRes.y) *
                translateMatrix(0.5f, 0.5f) *
#if defined(TARGET_METAL_ENVIRONMENT) || defined(TARGET_VULKAN_ENVIRONMENT)
                scaleMatrix(0.5f, -0.5f) *
#else
                scaleMatrix(0.5f, 0.5f) *
#endif
                frameUniforms.ssrReprojection *
                vec4(vsHitPoint, 1.0f);
        reprojected *= (1.0 / reprojected.w);
        Fr = texelFetch(light_ssr, int2(reprojected.x, reprojected.y), 0).rgb;
        return true;
    }
    return false;
}

#endif // screen-space reflections
