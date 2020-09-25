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
vec4 computeLightSpacePosition(const vec3 p, const vec3 n, const vec3 l,
        const float b, const mat4 lightFromWorldMatrix) {
    float NoL = saturate(dot(n, l));
    float sinTheta = sqrt(1.0 - NoL * NoL);
    vec3 offsetPosition = p + n * (sinTheta * b);
    vec4 lightSpacePosition = (lightFromWorldMatrix * vec4(offsetPosition, 1.0));
    return lightSpacePosition;
}
#endif
