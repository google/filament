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
        const highp vec3 l, const float b, const highp mat4 lightFromWorldMatrix) {

#if !defined(VARIANT_HAS_VSM)
    highp float NoL = saturate(dot(n, l));
    highp float sinTheta = sqrt(1.0 - NoL * NoL);
    p += n * (sinTheta * b);
#endif

    return mulMat4x4Float3(lightFromWorldMatrix, p);
}

#endif // VARIANT_HAS_SHADOWING
