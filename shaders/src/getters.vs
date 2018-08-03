//------------------------------------------------------------------------------
// Uniforms access
//------------------------------------------------------------------------------

mat4 getLightFromWorldMatrix() {
    return frameUniforms.lightFromWorldMatrix;
}

/** @public-api */
mat4 getWorldFromModelMatrix() {
    return objectUniforms.worldFromModelMatrix;
}

/** @public-api */
mat3 getWorldFromModelNormalMatrix() {
    return objectUniforms.worldFromModelNormalMatrix;
}

//------------------------------------------------------------------------------
// Attributes access
//------------------------------------------------------------------------------

#if defined(HAS_SKINNING)
void skinNormal(inout vec3 n, const uvec4 ids, const vec4 weights) {
    // this assumes that the sum of the weight is 1.0
    n += (halfPartialTransformVertexUnitQ(n, bonesUniforms.bones[ids.x * 2u]) * weights.x
        + halfPartialTransformVertexUnitQ(n, bonesUniforms.bones[ids.y * 2u]) * weights.y
        + halfPartialTransformVertexUnitQ(n, bonesUniforms.bones[ids.z * 2u]) * weights.z
        + halfPartialTransformVertexUnitQ(n, bonesUniforms.bones[ids.w * 2u]) * weights.w) * 2.0f;
}

void skinPosition(inout vec3 p, const uvec4 ids, const vec4 weights) {
    // this assumes that the sum of the weight is 1.0
    p +=  partialTransformVertexUnitQT(p, bonesUniforms.bones[ids.x * 2u], bonesUniforms.bones[ids.x * 2u + 1u].xyz) * weights.x
        + partialTransformVertexUnitQT(p, bonesUniforms.bones[ids.y * 2u], bonesUniforms.bones[ids.y * 2u + 1u].xyz) * weights.y
        + partialTransformVertexUnitQT(p, bonesUniforms.bones[ids.z * 2u], bonesUniforms.bones[ids.z * 2u + 1u].xyz) * weights.z
        + partialTransformVertexUnitQT(p, bonesUniforms.bones[ids.w * 2u], bonesUniforms.bones[ids.w * 2u + 1u].xyz) * weights.w;
}
#endif

/** @public-api */
vec4 getPosition() {
    return mesh_position;
}

vec4 getSkinnedPosition() {
    vec4 pos = getPosition();
#if defined(HAS_SKINNING)
    skinPosition(pos.xyz, mesh_bone_indices, mesh_bone_weights);
#endif
    return pos;
}

//------------------------------------------------------------------------------
// Helpers
//------------------------------------------------------------------------------

/**
 * Computes and returns the position in world space of the current vertex.
 * The world position computation depends on the current vertex domain. This
 * function optionally applies vertex skinning if needed.
 *
 * NOTE: the "transform" and "position" temporaries are necessary to work around
 * an issue with Adreno drivers (b/110851741).
 */
vec4 computeWorldPosition() {
#if defined(VERTEX_DOMAIN_OBJECT)
    mat4 transform = getWorldFromModelMatrix();
    vec3 position = getSkinnedPosition().xyz;
    return mulMat4x4Float3(transform, position);
#elif defined(VERTEX_DOMAIN_WORLD)
    return vec4(getSkinnedPosition().xyz, 1.0);
#elif defined(VERTEX_DOMAIN_VIEW)
    mat4 transform = getWorldFromViewMatrix();
    vec3 position = getSkinnedPosition().xyz;
    return mulMat4x4Float3(transform, position);
#else
    mat4 transform = getWorldFromViewMatrix() * getViewFromClipMatrix();
    vec3 position = getSkinnedPosition().xyz;
    return mulMat4x4Float3(transform, position);
#endif
}
