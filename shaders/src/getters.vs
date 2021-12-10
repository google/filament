//------------------------------------------------------------------------------
// Uniforms access
//------------------------------------------------------------------------------

mat4 getLightFromWorldMatrix() {
    return frameUniforms.lightFromWorldMatrix[0];
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

#if defined(HAS_SKINNING_OR_MORPHING)
vec3 mulBoneNormal(vec3 n, uint i) {
    highp mat3 transform = mat3(
        bonesUniforms.transform[i][0].xyz,
        bonesUniforms.transform[i][1].xyz,
        bonesUniforms.transform[i][2].xyz
    );
    return normalize(cofactor(transform) * n);
}

vec3 mulBoneVertex(vec3 v, uint i) {
    // last row of bonesUniforms.transform[i] is assumed to be [0,0,0,1]
    return mulMat4x3Float3(bonesUniforms.transform[i], v);
}

void skinNormal(inout vec3 n, const uvec4 ids, const vec4 weights) {
    n =   mulBoneNormal(n, ids.x) * weights.x
        + mulBoneNormal(n, ids.y) * weights.y
        + mulBoneNormal(n, ids.z) * weights.z
        + mulBoneNormal(n, ids.w) * weights.w;
}

void skinPosition(inout vec3 p, const uvec4 ids, const vec4 weights) {
    p =   mulBoneVertex(p, ids.x) * weights.x
        + mulBoneVertex(p, ids.y) * weights.y
        + mulBoneVertex(p, ids.z) * weights.z
        + mulBoneVertex(p, ids.w) * weights.w;
}
#endif

/** @public-api */
vec4 getPosition() {
    vec4 pos = mesh_position;

#if defined(HAS_SKINNING_OR_MORPHING)

    if ((objectUniforms.flags & FILAMENT_OBJECT_MORPHING_ENABLED_BIT) != 0u) {
        pos += objectUniforms.morphWeights.x * mesh_custom0;
        pos += objectUniforms.morphWeights.y * mesh_custom1;
        pos += objectUniforms.morphWeights.z * mesh_custom2;
        pos += objectUniforms.morphWeights.w * mesh_custom3;
    }

    if ((objectUniforms.flags & FILAMENT_OBJECT_SKINNING_ENABLED_BIT) != 0u) {
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

/** @public-api */
int getVertexIndex() {
#if defined(TARGET_METAL_ENVIRONMENT) || defined(TARGET_VULKAN_ENVIRONMENT)
    return gl_VertexIndex;
#else
    return gl_VertexID;
#endif
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
    vec3 position = getPosition().xyz;
    return mulMat4x4Float3(transform, position);
#elif defined(VERTEX_DOMAIN_WORLD)
    return vec4(getPosition().xyz, 1.0);
#elif defined(VERTEX_DOMAIN_VIEW)
    mat4 transform = getWorldFromViewMatrix();
    vec3 position = getPosition().xyz;
    return mulMat4x4Float3(transform, position);
#elif defined(VERTEX_DOMAIN_DEVICE)
    mat4 transform = getWorldFromClipMatrix();
    vec4 p = getPosition();
    // GL convention to inverted DX convention
    p.z = p.z * -0.5 + 0.5;
    vec4 position = transform * p;
    if (abs(position.w) < MEDIUMP_FLT_MIN) {
        position.w = position.w < 0.0 ? -MEDIUMP_FLT_MIN : MEDIUMP_FLT_MIN;
    }
    return position * (1.0 / position.w);
#else
#error Unknown Vertex Domain
#endif
}
