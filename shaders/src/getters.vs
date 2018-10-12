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
vec3 mulBoneNormal(vec3 n, uint i) {
    vec4 q  = bonesUniforms.bones[i + 0u];
    vec3 is = bonesUniforms.bones[i + 3u].xyz;

    // apply the inverse of the non-uniform scales
    n *= is;
    // apply the rigid transform (valid only for unit quaternions)
    n += 2.0 * cross(q.xyz, cross(q.xyz, n) + q.w * n);

    return n;
}

vec3 mulBoneVertice(vec3 v, uint i) {
    vec4 q = bonesUniforms.bones[i + 0u];
    vec3 t = bonesUniforms.bones[i + 1u].xyz;
    vec3 s = bonesUniforms.bones[i + 2u].xyz;

    // apply the non-uniform scales
    v *= s;
    // apply the rigid transform (valid only for unit quaternions)
    v += 2.0 * cross(q.xyz, cross(q.xyz, v) + q.w * v);
    // apply the translation
    v += t;

    return v;
}

void skinNormal(inout vec3 n, const uvec4 ids, const vec4 weights) {
    n =   mulBoneNormal(n, ids.x * 4u) * weights.x
        + mulBoneNormal(n, ids.y * 4u) * weights.y
        + mulBoneNormal(n, ids.z * 4u) * weights.z
        + mulBoneNormal(n, ids.w * 4u) * weights.w;
}

void skinPosition(inout vec3 p, const uvec4 ids, const vec4 weights) {
    p =   mulBoneVertice(p, ids.x * 4u) * weights.x
        + mulBoneVertice(p, ids.y * 4u) * weights.y
        + mulBoneVertice(p, ids.z * 4u) * weights.z
        + mulBoneVertice(p, ids.w * 4u) * weights.w;
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
