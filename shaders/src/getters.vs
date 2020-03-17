//------------------------------------------------------------------------------
// Uniforms access
//------------------------------------------------------------------------------

mat4 getLightFromWorldMatrix() {
    return frameUniforms.lightFromWorldMatrix;
}

#if defined(HAS_SHADOWING)
mat4 getSpotLightFromWorldMatrix(uint index) {
    return shadowUniforms.spotLightFromWorldMatrix[index];
}
#endif

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

#if defined(HAS_SKINNING_OR_MORPHING)
vec3 mulBoneNormal(vec3 n, uint i) {
    vec4 q  = bonesUniforms.bones[i + 0u];
    vec3 is = bonesUniforms.bones[i + 3u].xyz;

    // apply the inverse of the non-uniform scales
    n *= is;
    // apply the rigid transform (valid only for unit quaternions)
    n += 2.0 * cross(q.xyz, cross(q.xyz, n) + q.w * n);

    return n;
}

vec3 mulBoneVertex(vec3 v, uint i) {
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
    p =   mulBoneVertex(p, ids.x * 4u) * weights.x
        + mulBoneVertex(p, ids.y * 4u) * weights.y
        + mulBoneVertex(p, ids.z * 4u) * weights.z
        + mulBoneVertex(p, ids.w * 4u) * weights.w;
}
#endif

/** @public-api */
vec4 getPosition() {
    vec4 pos = mesh_position;

#if defined(HAS_SKINNING_OR_MORPHING)

    if (objectUniforms.morphingEnabled == 1) {
        pos += objectUniforms.morphWeights.x * mesh_custom0;
        pos += objectUniforms.morphWeights.y * mesh_custom1;
        pos += objectUniforms.morphWeights.z * mesh_custom2;
        pos += objectUniforms.morphWeights.w * mesh_custom3;
    }

    if (objectUniforms.skinningEnabled == 1) {
        skinPosition(pos.xyz, mesh_bone_indices, mesh_bone_weights);
    }

#endif

    return pos;
}

#if defined(HAS_ATTRIBUTE_CUSTOM0)
vec4 getCustom0() { return mesh_custom0; }
#endif
#if defined(HAS_ATTRIBUTE_CUSTOM1)
vec4 getCustom1() { return mesh_custom1; }
#endif
#if defined(HAS_ATTRIBUTE_CUSTOM2)
vec4 getCustom2() { return mesh_custom2; }
#endif
#if defined(HAS_ATTRIBUTE_CUSTOM3)
vec4 getCustom3() { return mesh_custom3; }
#endif
#if defined(HAS_ATTRIBUTE_CUSTOM4)
vec4 getCustom4() { return mesh_custom4; }
#endif
#if defined(HAS_ATTRIBUTE_CUSTOM5)
vec4 getCustom5() { return mesh_custom5; }
#endif
#if defined(HAS_ATTRIBUTE_CUSTOM6)
vec4 getCustom6() { return mesh_custom6; }
#endif
#if defined(HAS_ATTRIBUTE_CUSTOM7)
vec4 getCustom7() { return mesh_custom7; }
#endif

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
    vec3 position = getPosition().xyz;
    return mulMat4x4Float3(transform, position);
#elif defined(VERTEX_DOMAIN_WORLD)
    return vec4(getPosition().xyz, 1.0);
#elif defined(VERTEX_DOMAIN_VIEW)
    mat4 transform = getWorldFromViewMatrix();
    vec3 position = getPosition().xyz;
    return mulMat4x4Float3(transform, position);
#else
    mat4 transform = getWorldFromClipMatrix();
    vec4 position = transform * getPosition();
    return position * (1.0 / position.w);
#endif
}
