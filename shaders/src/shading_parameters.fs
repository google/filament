//------------------------------------------------------------------------------
// Material evaluation
//------------------------------------------------------------------------------

/**
 * Computes global shading parameters used to apply lighting, such as the view
 * vector in world space, the tangent frame at the shading point, etc.
 */
void computeShadingParams() {
#if defined(HAS_ATTRIBUTE_TANGENTS)
#if defined(GEOMETRIC_SPECULAR_AA_NORMAL)
    // Pick either the regular interpolated normal or the centroid interpolated
    // normal to reduce shading aliasing
    HIGHP vec3 n = dot(vertex_worldNormal, vertex_worldNormal) >= 1.01 ?
            vertex_worldNormalCentroid : vertex_worldNormal;
#else
    HIGHP vec3 n = vertex_worldNormal;
#endif

#if defined(MATERIAL_HAS_DOUBLE_SIDED_CAPABILITY)
    if (isDoubleSided()) {
        n = gl_FrontFacing ? n : -n;
    }
#endif

#if defined(MATERIAL_HAS_ANISOTROPY) || defined(MATERIAL_HAS_NORMAL) || defined(MATERIAL_HAS_CLEAR_COAT_NORMAL)
    // Re-normalize post-interpolation values
    shading_tangentToWorld = mat3(
            normalize(vertex_worldTangent), normalize(vertex_worldBitangent), normalize(n));
#endif
    // Leave the tangent and bitangent uninitialized, we won't use them
    shading_tangentToWorld[2] = normalize(n);
#endif

    shading_position = vertex_worldPosition;
    shading_view = normalize(frameUniforms.cameraPosition - shading_position);
}

/**
 * Computes global shading parameters that the material might need to access
 * before lighting: N dot V, the reflected vector and the shading normal (before
 * applying the normal map). These parameters can be useful to material authors
 * to compute other material properties.
 *
 * This function must be invoked by the user's material code (guaranteed by
 * the material compiler) after setting a value for MaterialInputs.normal.
 */
void prepareMaterial(const MaterialInputs material) {
#if defined(HAS_ATTRIBUTE_TANGENTS)
#if defined(MATERIAL_HAS_NORMAL)
    shading_normal = normalize(shading_tangentToWorld * material.normal);
#else
    shading_normal = shading_tangentToWorld[2];
#endif
    shading_NoV = clampNoV(dot(shading_normal, shading_view));
    shading_reflected = reflect(-shading_view, shading_normal);

#if defined(MATERIAL_HAS_CLEAR_COAT)
#if defined(MATERIAL_HAS_CLEAR_COAT_NORMAL)
    shading_clearCoatNormal = normalize(shading_tangentToWorld * material.clearCoatNormal);
#else
    shading_clearCoatNormal = shading_tangentToWorld[2];
#endif
#endif
#endif
}
