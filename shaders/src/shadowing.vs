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
    vec3 l = frameUniforms.lightDirection;
    float NoL = saturate(dot(n, l));
    float sinTheta = sqrt(1.0 - NoL * NoL);

    vec3 offsetPosition = p;
    // normal bias
    offsetPosition += n * (sinTheta * frameUniforms.shadowBias.y);
    // constant bias (note this could be a uniform). We apply the constant bias in
    // world space (as opposed to light-space) to account for perspective and lispsm shadow maps.
    offsetPosition += l * frameUniforms.shadowBias.x;

    vec4 lightSpacePosition = (getLightFromWorldMatrix() * vec4(offsetPosition, 1.0));

    return lightSpacePosition;
}
#endif
