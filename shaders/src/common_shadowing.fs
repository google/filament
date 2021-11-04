//------------------------------------------------------------------------------
// Shadowing
//------------------------------------------------------------------------------

#if defined(HAS_SHADOWING)
/**
 * Computes the light space position of the specified world space point.
 * The returned point may contain a bias to attempt to eliminate common
 * shadowing artifacts such as "acne". To achieve this, the world space
 * normal at the point must also be passed to this function.
 * Normal bias is not used for VSM.
 */

#if defined(HAS_VSM)
#if defined(HAS_DIRECTIONAL_LIGHTING)
highp vec4 computeLightSpacePositionDirectional(const highp vec3 p, const highp vec3 n,
        const highp vec3 l, const float b, const highp mat4 lightFromWorldMatrix) {
    return mulMat4x4Float3(lightFromWorldMatrix, p);
}
#endif // HAS_DIRECTIONAL_LIGHTING
#if defined(HAS_DYNAMIC_LIGHTING)
highp vec4 computeLightSpacePositionSpot(const highp vec3 p, const highp vec3 n,
        const highp vec3 l,  const float b, const highp mat4 lightFromWorldMatrix) {
    return mulMat4x4Float3(lightFromWorldMatrix, p);
}
#endif // HAS_DYNAMIC_LIGHTING
#else // HAS_VSM
highp vec4 computeLightSpacePositionDirectional(const highp vec3 p, const highp vec3 n,
        const highp vec3 l, const float b, const highp mat4 lightFromWorldMatrix) {
    float NoL = saturate(dot(n, l));
    highp float sinTheta = sqrt(1.0 - NoL * NoL);
    highp vec3 offsetPosition = p + n * (sinTheta * b);
    return mulMat4x4Float3(lightFromWorldMatrix, offsetPosition);
}
#if defined(HAS_DYNAMIC_LIGHTING)
highp vec4 computeLightSpacePositionSpot(const highp vec3 p, const highp vec3 n,
        const highp vec3 l, const float b, const highp mat4 lightFromWorldMatrix) {
    highp vec4 positionLs = mulMat4x4Float3(lightFromWorldMatrix, p);
    highp float oneOverZ = positionLs.w / positionLs.z;
    return computeLightSpacePositionDirectional(p, n, l, b * oneOverZ, lightFromWorldMatrix);
}
#endif // HAS_DIRECTIONAL_LIGHTING
#endif // HAS_VSM
#endif // HAS_SHADOWING
