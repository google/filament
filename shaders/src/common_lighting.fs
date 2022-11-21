#define IBL_TECHNIQUE_INFINITE      0u
#define IBL_TECHNIQUE_FINITE_SPHERE 1u
#define IBL_TECHNIQUE_FINITE_BOX    2u

struct Light {
    vec4 colorIntensity;  // rgb, pre-exposed intensity
    vec3 l;
    float attenuation;
    highp vec3 worldPosition;
    float NoL;
    highp vec3 direction;
    float zLight;
    bool castsShadows;
    bool contactShadows;
    uint type;
    uint shadowIndex;
    uint channels;
};

struct PixelParams {
    vec3  diffuseColor;
    float perceptualRoughness;
    float perceptualRoughnessUnclamped;
    vec3  f0;
    float roughness;
    vec3  dfg;
    vec3  energyCompensation;

#if defined(MATERIAL_HAS_CLEAR_COAT)
    float clearCoat;
    float clearCoatPerceptualRoughness;
    float clearCoatRoughness;
#endif

#if defined(MATERIAL_HAS_SHEEN_COLOR)
    vec3  sheenColor;
#if !defined(SHADING_MODEL_CLOTH)
    float sheenRoughness;
    float sheenPerceptualRoughness;
    float sheenScaling;
    float sheenDFG;
#endif
#endif

#if defined(MATERIAL_HAS_ANISOTROPY)
    vec3  anisotropicT;
    vec3  anisotropicB;
    float anisotropy;
#endif

#if defined(SHADING_MODEL_SUBSURFACE) || defined(MATERIAL_HAS_REFRACTION)
    float thickness;
#endif
#if defined(SHADING_MODEL_SUBSURFACE)
    vec3  subsurfaceColor;
    float subsurfacePower;
#endif

#if defined(SHADING_MODEL_CLOTH) && defined(MATERIAL_HAS_SUBSURFACE_COLOR)
    vec3  subsurfaceColor;
#endif

#if defined(MATERIAL_HAS_REFRACTION)
    float etaRI;
    float etaIR;
    float transmission;
    float uThickness;
    vec3  absorption;
#endif
};

float computeMicroShadowing(float NoL, float visibility) {
    // Chan 2018, "Material Advances in Call of Duty: WWII"
    float aperture = inversesqrt(1.0 - min(visibility, 0.9999));
    float microShadow = saturate(NoL * aperture);
    return microShadow * microShadow;
}

/**
 * IBL utilities
 */

vec2 IntersectAABB(vec3 rayOrigin, vec3 rayDir, vec3 boxMin, vec3 boxMax) {
    vec3 tMin = (boxMin - rayOrigin) / rayDir;
    vec3 tMax = (boxMax - rayOrigin) / rayDir;
    vec3 t1 = min(tMin, tMax);
    vec3 t2 = max(tMin, tMax);
    float tNear = max3(t1);
    float tFar = min3(t2);
    return vec2(tNear, tFar);
}

// Assume: a <= b
float GetSmallestPositive(float a, float b) {
    return a >= 0.0 ? a : b;
}

/**
 * This function returns an IBL lookup direction, taking into account the current IBL type (e.g. infinite spherical, 
 * finite/local sphere/box), an initial intended lookup direction (baseDir) and the particular normal we compute
 * reflections against (e.g. either the interpolated surface or clearcoat normal).
 */
vec3 GetAdjustedReflectedDirection(const vec3 baseDir, const vec3 normal) {
    vec3 defaultReflected = reflect(-baseDir, normal);    

    if (frameUniforms.iblTechnique == IBL_TECHNIQUE_INFINITE) return defaultReflected;

    // intersect the ray rayPos + t * rayDir with the finite geometry; done in the coordinate system of the finite geometry
    vec3 rayPos = getWorldPosition() + getWorldOffset() - frameUniforms.iblCenter;
    vec3 rayDir = defaultReflected;

    vec3  r  = vec3(0.0); // resulting direction
    float t0 = -1.0f;     // intersection parameter between ray and finite IBL geometry
    
    if (frameUniforms.iblTechnique == IBL_TECHNIQUE_FINITE_SPHERE) {
        // Normalize sphere-space by scaling down positions by radius. We don't scale down ray direction to preserve 
        // the convenient A = 1 in the quadratic formula. iblHalfExtents.y contains the reciprocal of the IBL sphere radius.
        vec3 rayPosNormalized = rayPos * frameUniforms.iblHalfExtents.y;

        float B = 2.0 * dot(rayPosNormalized, rayDir);
        float C = dot(rayPosNormalized, rayPosNormalized) - 1.0; // 1.0 = r^2, as we are in normalized space

        t0 = 0.5 * (-B + sqrt(B*B - 4.0 * C));
        t0 *= frameUniforms.iblHalfExtents.x;
    }
    else if (frameUniforms.iblTechnique == IBL_TECHNIQUE_FINITE_BOX) {
        vec2 roots = IntersectAABB(rayPos, rayDir, -frameUniforms.iblHalfExtents, frameUniforms.iblHalfExtents);
        t0 = GetSmallestPositive(roots.x, roots.y);
    }

    // translate results back to world space
    vec3 intersection_point = ( t0 >= 0.0 ) ? rayPos + t0 * rayDir : defaultReflected;
    r = normalize(intersection_point);

    return r;
}

/**
 * Returns the reflected vector at the current shading point. The reflected vector
 * return by this function might be different from shading_reflected:
 * - For anisotropic material, we bend the reflection vector to simulate
 *   anisotropic indirect lighting
 * - The reflected vector may be modified to point towards the dominant specular
 *   direction to match reference renderings when the roughness increases
 */

vec3 getReflectedVector(const PixelParams pixel, const vec3 v, const vec3 n) {
#if defined(MATERIAL_HAS_ANISOTROPY)
    vec3  anisotropyDirection = pixel.anisotropy >= 0.0 ? pixel.anisotropicB : pixel.anisotropicT;
    vec3  anisotropicTangent  = cross(anisotropyDirection, v);
    vec3  anisotropicNormal   = cross(anisotropicTangent, anisotropyDirection);
    float bendFactor          = abs(pixel.anisotropy) * saturate(5.0 * pixel.perceptualRoughness);
    vec3  bentNormal          = normalize(mix(n, anisotropicNormal, bendFactor));

    vec3 r = GetAdjustedReflectedDirection(-v, bentNormal);
#else
    vec3 r = GetAdjustedReflectedDirection(-v, n);
#endif
    return r;
}

void getAnisotropyPixelParams(const MaterialInputs material, inout PixelParams pixel) {
#if defined(MATERIAL_HAS_ANISOTROPY)
    vec3 direction = material.anisotropyDirection;
    pixel.anisotropy = material.anisotropy;
    pixel.anisotropicT = normalize(shading_tangentToWorld * direction);
    pixel.anisotropicB = normalize(cross(getWorldGeometricNormalVector(), pixel.anisotropicT));
#endif
}
