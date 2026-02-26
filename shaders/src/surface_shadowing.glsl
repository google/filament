//------------------------------------------------------------------------------
// Shadowing
//------------------------------------------------------------------------------

#if defined(VARIANT_HAS_SHADOWING)
/**
 * Computes the light space position of the specified world space point.
 * The returned point may contain a bias to attempt to eliminate common
 * shadowing artifacts such as "acne". To achieve this, the world space
 * normal at the point must also be passed to this function.
 * Normal bias is not used for VSM.
 */

highp vec4 computeLightSpacePosition(highp vec3 p, const highp vec3 n,
        const highp vec3 dir, const highp vec2 b, highp_mat4 lightFromWorldMatrix) {

#if !defined(VARIANT_HAS_VSM)
    // --------------------------------------------------------------------------------------
    // Anisotropic Normal Bias for Shadow Mapping
    // --------------------------------------------------------------------------------------
    // To prevent shadow acne, we must push the geometry along its normal to clear the
    // quantization steps of the shadow map's discrete depth grid. The exact physical depth
    // error we must clear is proportional to the shadow texel's world-space dimensions.
    //
    // This implementation computes the exact geometric projection of the rectangular
    // shadow map texel onto the surface normal.
    //
    // 1. Coordinate Space Transition:
    //    We project the world-space normal onto the light's X and Y basis vectors (L_right,
    //    L_up). This gives us the lateral components of the normal in Light Space (n_Lx, n_Ly).
    //
    // 2. The Implicit sin(theta) Slope Scale:
    //    Because the normal is a unit vector, the magnitude of its lateral components in
    //    light space inherently equals sin(theta), where theta is the angle of incidence.
    //    This perfectly and automatically scales the bias from 0.0 (top-down, flat surface)
    //    to maximum (grazing angle).
    //
    // 3. Exact Anisotropic Footprint (The L1 Norm):
    //    Shadow texels are rarely perfectly square due to Cascaded Shadow Maps (CSM) or
    //    Light Space Perspective Shadow Maps (LiSPSM). Jx and Jy are the physical world-space
    //    dimensions of the texel.
    //    By evaluating `abs(n_Lx * Jx) + abs(n_Ly * Jy)`, we compute the exact scalar
    //    projection of the rectangular texel footprint.
    //      - It is superior to `max(Jx, Jy)` which assumes a massive square and causes Peter Panning.
    //      - It is superior to `length()` which assumes an ellipse and under-biases the corners.
    // --------------------------------------------------------------------------------------

    // Extract the first row (Light's Right vector in World Space)
    highp vec3 L_right = vec3(lightFromWorldMatrix[0][0], lightFromWorldMatrix[1][0], lightFromWorldMatrix[2][0]);

    // Extract the second row (Light's Up vector in World Space)
    highp vec3 L_up    = vec3(lightFromWorldMatrix[0][1], lightFromWorldMatrix[1][1], lightFromWorldMatrix[2][1]);

    // Project the world normal onto the shadow map's 2D grid
    highp float n_Lx = dot(n, L_right);
    highp float n_Ly = dot(n, L_up);

    // Apply the anisotropic normal bias
    p += n * (abs(n_Lx * b.x) + abs(n_Ly * b.y));
#endif

    return mulMat4x4Float3(lightFromWorldMatrix, p);
}

#endif // VARIANT_HAS_SHADOWING
