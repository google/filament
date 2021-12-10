//------------------------------------------------------------------------------
// Screen-space reflections
//------------------------------------------------------------------------------

#if defined(HAS_REFLECTIONS) && REFLECTIONS_MODE == REFLECTIONS_MODE_SCREEN_SPACE

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

// Copied from depthUtils.fs
highp float linearizeDepth(highp float depth) {
    // Our far plane is at infinity, which causes a division by zero below, which in turn
    // causes some issues on some GPU. We workaround it by replacing "infinity" by the closest
    // value representable in  a 24 bit depth buffer.
    const float preventDiv0 = 1.0 / 16777216.0;
    mat4 p = getViewFromClipMatrix();
    // this works with perspective and ortho projections, for a perspective projection
    // this resolves to -near/depth, for an ortho projection this resolves to depth*(far - near) - far
    return (depth * p[2].z + p[3].z) / max(depth * p[2].w + p[3].w, preventDiv0);
}

// Here "cs" stands for "camera space", which is equivalent to Filament's "view space".
bool traceScreenSpaceRay
   (Point3          csOrigin,
    Vector3         csDirection,
    mat4x4          projectToPixelMatrix,
    sampler2D       csZBuffer,
    float           csZThickness,
    const in bool   csZBufferIsHyperbolic,
    float           nearPlaneZ,
    float           stride,
    float           jitterFraction,
    float           maxSteps,
    in float        maxRayTraceDistance,
    out Point2      hitPixel,
    out Point3      csHitPoint) {

    // Clip ray to a near plane in 3D (doesn't have to be *the* near plane, although that would be a good idea)
    float rayLength = ((csOrigin.z + csDirection.z * maxRayTraceDistance) > nearPlaneZ) ?
                        (nearPlaneZ - csOrigin.z) / csDirection.z :
                        maxRayTraceDistance;
    //float rayLength = 3.0f;
    Point3 csEndPoint = csDirection * rayLength + csOrigin;

    //debugLineView(csOrigin, csEndPoint, vec3(1.0, 1.0, 1.0));

    // Project into screen space
    Vector4 H0 = projectToPixelMatrix * Vector4(csOrigin, 1.0);
    Vector4 H1 = projectToPixelMatrix * Vector4(csEndPoint, 1.0);

//     highp mat4 viewToClip = getClipFromViewMatrix();
//     Vector4 H0 = viewToClip * vec4(csOrigin, 1.0);
//     Vector4 H1 = viewToClip * vec4(csEndPoint, 1.0);
//     H0.xy = (H0.xy / H0.w) * 0.5 + 0.5;
//     H1.xy = (H1.xy / H1.w) * 0.5 + 0.5;
//     H0.xy *= getResolution().xy;
//     H1.xy *= getResolution().xy;
//     H0.xy *= H0.w;
//     H1.xy *= H1.w;
//
//     H0.z = dot(H0.zw, vec2(.5, .5));
//     H1.z = dot(H1.zw, vec2(.5, .5));

    // H0, H1
    // xy are in unnormalized screen space
    // z, w are in homogeneous coordinates

    // debugLinePixels(uvec2(H0.xy), uvec2(H1.xy), vec3(0.0, 0.0, 1.0));

    // There are a lot of divisions by w that can be turned into multiplications
    // at some minor precision loss...and we need to interpolate these 1/w values
    // anyway.
    //
    // Because the caller was required to clip to the near plane,
    // this homogeneous division (projecting from 4D to 2D) is guaranteed
    // to succeed.
    float k0 = 1.0 / H0.w;
    float k1 = 1.0 / H1.w;

    // Switch the original points to values that interpolate linearly in 2D
    Point3 Q0 = csOrigin * k0;
    Point3 Q1 = csEndPoint * k1;

    // Screen-space endpoints
    Point2 P0 = H0.xy * k0;
    Point2 P1 = H1.xy * k1;

    //debugCrossPixels(uvec2(P0.xy), vec3(0.0, 1.0, 0.0));
    //debugCrossPixels(uvec2(P1.xy), vec3(0.0, 0.0, 1.0));

    // [Optional clipping to frustum sides here]

    // Initialize to off screen
    hitPixel = Point2(-1.0, -1.0);

    // If the line is degenerate, make it cover at least one pixel
    // to avoid handling zero-pixel extent as a special case later
    P1 += vec2((distanceSquared(P0, P1) < 0.0001) ? 0.01 : 0.0);

    Vector2 delta = P1 - P0;

    // Permute so that the primary iteration is in x to reduce
    // large branches later
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

    // We track the ray depth at +/- 1/2 pixel to treat pixels as clip-space solid
    // voxels. Because the depth at -1/2 for a given pixel will be the same as at
    // +1/2 for the previous iteration, we actually only have to compute one value
    // per iteration.
    float prevZMaxEstimate = csOrigin.z;
    float stepCount = 0.0;
    float rayZMax = prevZMaxEstimate, rayZMin = prevZMaxEstimate;
    float sceneZMax = rayZMax + 1e4;

    // P1.x is never modified after this point, so pre-scale it by
    // the step direction for a signed comparison
    float end = P1.x * stepDirection;

    // We only advance the z field of Q in the inner loop, since
    // Q.xy is never used until after the loop terminates.

    for (Point2 P = P0;
        ((P.x * stepDirection) <= end) &&
        (stepCount < maxSteps) &&
        ((rayZMax < sceneZMax - csZThickness) ||
            (rayZMin > sceneZMax)) &&
        (sceneZMax != 0.0);
        P += dP, Q.z += dQ.z, k += dk, stepCount += 1.0) {

        hitPixel = permute ? P.yx : P;

        // The depth range that the ray covers within this loop
        // iteration.  Assume that the ray is moving in increasing z
        // and swap if backwards.  Because one end of the interval is
        // shared between adjacent iterations, we track the previous
        // value and then swap as needed to ensure correct ordering
        rayZMin = prevZMaxEstimate;

        // Compute the value at 1/2 pixel into the future
        rayZMax = (dQ.z * 0.5 + Q.z) / (dk * 0.5 + k);
        prevZMaxEstimate = rayZMax;
        if (rayZMin > rayZMax) {
            swap(rayZMin, rayZMax);
        }

        // Camera-space z of the background
        sceneZMax = texelFetch(csZBuffer, int2(hitPixel.x, hitPixel.y), 0).r;
        sceneZMax = linearizeDepth(sceneZMax);
    } // pixel on ray

    Q.xy += dQ.xy * stepCount;
    csHitPoint = Q * (1.0 / k);
    // Matches the new loop condition:
    return (rayZMax >= sceneZMax - csZThickness) && (rayZMin <= sceneZMax);
}

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

void evaluateScreenSpaceReflections(highp vec3 r, inout vec3 Fr) {
    highp vec3 wsRayDirection = r;
    highp vec3 wsRayStart = shading_position + frameUniforms.ssrBias * wsRayDirection;

    // ray start/end in camera space
    // "cs" stands for camera space
    highp mat4 worldToView = getViewFromWorldMatrix();
    highp vec4 csRayStart = worldToView * vec4(wsRayStart, 1.0);
    highp vec4 csRayDirection = worldToView * vec4(wsRayDirection, 0.0);

    highp vec3 csOrigin = csRayStart.xyz;
    highp vec3 csDirection = csRayDirection.xyz;
    highp mat4x4 projectToPixelMatrix;
    highp float csZThickness = frameUniforms.ssrThickness;
    const bool csZBufferIsHyperbolic = true;
    highp float nearPlaneZ = -0.1f;
    highp float stride = 1.0;
    highp float jitterFraction = 1.0f; //random(gl_FragCoord.xy);
    highp float maxSteps = 1000.0;
    highp float maxRayTraceDistance = frameUniforms.ssrDistance;

    // <-- outputs -->
    vec2 hitPixel;
    vec3 csHitPoint;
    // <------------->

    vec2 res = vec2(textureSize(light_structure, 0).xy);
    projectToPixelMatrix =
        scaleMatrix(res.x, res.y) *
        translateMatrix(0.5f, 0.5f) *
#if defined(TARGET_METAL_ENVIRONMENT) || defined(TARGET_VULKAN_ENVIRONMENT)
        scaleMatrix(0.5f, -0.5f) *
#else
        scaleMatrix(0.5f, 0.5f) *
#endif
        getClipFromViewMatrix();

    if (traceScreenSpaceRay(csOrigin, csDirection, projectToPixelMatrix, light_structure,
            csZThickness, csZBufferIsHyperbolic, nearPlaneZ, stride, jitterFraction, maxSteps,
            maxRayTraceDistance, hitPixel, csHitPoint)) {
        vec2 ssrRes = vec2(textureSize(light_ssr, 0).xy);
        vec4 reprojected = scaleMatrix(ssrRes.x, ssrRes.y) *
                translateMatrix(0.5f, 0.5f) *
#if defined(TARGET_METAL_ENVIRONMENT) || defined(TARGET_VULKAN_ENVIRONMENT)
                scaleMatrix(0.5f, -0.5f) *
#else
                scaleMatrix(0.5f, 0.5f) *
#endif
                frameUniforms.ssrReprojection *
                vec4(csHitPoint, 1.0f);
        reprojected *= (1.0 / reprojected.w);
        Fr = texelFetch(light_ssr, int2(reprojected.x, reprojected.y), 0).rgb;
    }
}

#endif // screen-space reflections
