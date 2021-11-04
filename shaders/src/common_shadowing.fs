//------------------------------------------------------------------------------
// Shadowing
//------------------------------------------------------------------------------

#if defined(HAS_SHADOWING)
/**
 * Computes the light space position of the specified world space point.
 * The returned point may contain a bias to attempt to eliminate common
 * shadowing artifacts such as "acne". To achieve this, the world space
 * normal at the point must also be passed to this function.
 */
highp vec4 computeLightSpacePosition(const highp vec3 p, const highp vec3 n, const highp vec3 l,
        const float b, const highp mat4 lightFromWorldMatrix) {
#if defined(HAS_VSM)
    // VSM don't apply the shadow bias
    highp vec4 lightSpacePosition = mulMat4x4Float3(lightFromWorldMatrix, p);
#else
    float NoL = saturate(dot(n, l));
    float sinTheta = sqrt(1.0 - NoL * NoL);
    highp vec3 offsetPosition = p + n * (sinTheta * b);
    highp vec4 lightSpacePosition = mulMat4x4Float3(lightFromWorldMatrix, offsetPosition);
#endif
    return lightSpacePosition;
}
#endif
