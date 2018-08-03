//------------------------------------------------------------------------------
// Shadowing
//------------------------------------------------------------------------------

#if defined(HAS_SHADOWING) && defined(HAS_DIRECTIONAL_LIGHTING)
/**
 * Computes the light space position of the specified world space point.
 * The returned point may contain a bias to attempt to eliminate common
 * shadowing artifacts such as "acne". To achieve this, the world space
 * normal at the point must also be passed to this function.
 */
vec4 getLightSpacePosition(const vec3 p, const vec3 n) {
    float NoL = saturate(dot(n, frameUniforms.lightDirection));

#ifdef TARGET_MOBILE
    float normalBias = 1.0 - NoL * NoL;
#else
    float normalBias = sqrt(1.0 - NoL * NoL);
#endif

    vec3 offsetPosition = p + n * (normalBias * frameUniforms.shadowBias.y);
    vec4 lightSpacePosition = (getLightFromWorldMatrix() * vec4(offsetPosition, 1.0));
    lightSpacePosition.z -= frameUniforms.shadowBias.x;

    return lightSpacePosition;
}
#endif
