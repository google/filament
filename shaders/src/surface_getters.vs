//------------------------------------------------------------------------------
// Uniforms access
//------------------------------------------------------------------------------

#if CLIENT_MATERIAL_API_LEVEL < UNSTABLE_MATERIAL_API_LEVEL
/** @public-api */
// NOTE: getWorldFromModelMatrix is supported under API level 1 in the vertex shader,
// but requires API level 2 in the fragment shader. Therefore, it is defined here
// for vertex shaders, but redirected to an error macro in surface_instancing.glsl for fragment shaders.
mat4 getWorldFromModelMatrix() {
    return object_uniforms_worldFromModelMatrix;
}

/** @public-api */
mat3 getWorldFromModelNormalMatrix() {
    return object_uniforms_worldFromModelNormalMatrix;
}
#endif

/** sort-of public */
float getObjectUserData() {
    return object_uniforms_userData;
}

//------------------------------------------------------------------------------
// Attributes access
//------------------------------------------------------------------------------

#if MATERIAL_FEATURE_LEVEL > 0
/** @public-api */
int getVertexIndex() {
#if defined(TARGET_METAL_ENVIRONMENT) || defined(TARGET_VULKAN_ENVIRONMENT) || defined(TARGET_WEBGPU_ENVIRONMENT)
    return gl_VertexIndex;
#else
    return gl_VertexID;
#endif
}
#endif

#if defined(VARIANT_HAS_SKINNING_OR_MORPHING)
#define MAX_SKINNING_BUFFER_WIDTH_LOG2 11u
#define MAX_SKINNING_BUFFER_WIDTH (1u << MAX_SKINNING_BUFFER_WIDTH_LOG2)
#define MAX_SKINNING_BUFFER_WIDTH_MASK (MAX_SKINNING_BUFFER_WIDTH - 1u)

highp mat3 computeBoneCofactor(uint j) {
    highp vec3 r0 = bonesUniforms.bones[j].transform[0].xyz;
    highp vec3 r1 = bonesUniforms.bones[j].transform[1].xyz;
    highp vec3 r2 = bonesUniforms.bones[j].transform[2].xyz;

    highp mat3 cof;
    cof[0] = bonesUniforms.bones[j].cof0;
    cof[1] = vec3(
            bonesUniforms.bones[j].cof1x,
            r0.x * r2.z - r0.z * r2.x,
            r0.z * r1.x - r0.x * r1.z);
    cof[2] = vec3(
            r1.x * r2.y - r1.y * r2.x,
            r0.y * r2.x - r0.x * r2.y,
            r0.x * r1.y - r0.y * r1.x);
    return cof;
}

vec3 mulBoneNormal(vec3 n, uint j) {
    return normalize(computeBoneCofactor(j) * n);
}

void accumulateBoneNormalTangent(inout vec3 normSum, inout vec3 tangSum,
        const vec3 n, const vec3 t, uint j, float weight) {
    highp mat3 cof = computeBoneCofactor(j);
    normSum += weight * normalize(cof * n);
    tangSum += weight * normalize(cof * t);
}

vec3 mulBoneVertex(vec3 v, uint i) {
    // last row of bonesUniforms.bones[i].transform (row major) is assumed to be [0,0,0,1]
    return vec4(v, 1.0) * bonesUniforms.bones[i].transform;
}

void skinPosition(inout vec3 p, const uvec4 ids, const vec4 weights) {
    vec3 posSum = weights.x * mulBoneVertex(p, uint(ids.x))
            + weights.y * mulBoneVertex(p, uint(ids.y))
            + weights.z * mulBoneVertex(p, uint(ids.z));
    // standard skinning for 4 weights, some of them could be zero
    if (weights.w >= 0.0) {
        p = posSum + weights.w * mulBoneVertex(p, uint(ids.w));
        return;
    }
    // skinning for >4 weights
    uint pairIndex = uint(-weights.w - 1.0);
    uint pairStop = pairIndex + uint(ids.w - 3u);
    for (uint i = pairIndex; i < pairStop; ++i) {
        ivec2 texcoord = ivec2(
                i & MAX_SKINNING_BUFFER_WIDTH_MASK, i >> MAX_SKINNING_BUFFER_WIDTH_LOG2);
        vec2 indexWeight = texelFetch(sampler1_indicesAndWeights, texcoord, 0).rg;
        posSum += mulBoneVertex(p, uint(indexWeight.r)) * indexWeight.g;
    }
    p = posSum;
}

void skinNormal(inout vec3 n, const uvec4 ids, const vec4 weights) {
    vec3 normSum = weights.x * mulBoneNormal(n, uint(ids.x))
            + weights.y * mulBoneNormal(n, uint(ids.y))
            + weights.z * mulBoneNormal(n, uint(ids.z));
    // standard skinning for 4 weights, some of them could be zero
    if (weights.w >= 0.0) {
        n = normSum + weights.w * mulBoneNormal(n, uint(ids.w));
        return;
    }
    // skinning for >4 weights
    uint pairIndex = uint(-weights.w - 1.0);
    uint pairStop = pairIndex + uint(ids.w - 3u);
    for (uint i = pairIndex; i < pairStop; ++i) {
        ivec2 texcoord = ivec2(
                i & MAX_SKINNING_BUFFER_WIDTH_MASK, i >> MAX_SKINNING_BUFFER_WIDTH_LOG2);
        vec2 indexWeight = texelFetch(sampler1_indicesAndWeights, texcoord, 0).rg;
        normSum += mulBoneNormal(n, uint(indexWeight.r)) * indexWeight.g;
    }
    n = normSum;
}

void skinNormalTangent(inout vec3 n, inout vec3 t, const uvec4 ids, const vec4 weights) {
    highp mat3 cof = computeBoneCofactor(uint(ids.x));
    vec3 normSum = weights.x * normalize(cof * n);
    vec3 tangSum = weights.x * normalize(cof * t);
    accumulateBoneNormalTangent(normSum, tangSum, n, t, uint(ids.y), weights.y);
    accumulateBoneNormalTangent(normSum, tangSum, n, t, uint(ids.z), weights.z);
    // standard skinning for 4 weights, some of them could be zero
    if (weights.w >= 0.0) {
        accumulateBoneNormalTangent(normSum, tangSum, n, t, uint(ids.w), weights.w);
        n = normSum;
        t = tangSum;
        return;
    }
    // skinning for >4 weights
    uint pairIndex = uint(-weights.w - 1.0);
    uint pairStop = pairIndex + uint(ids.w - 3u);
    for (uint i = pairIndex; i < pairStop; ++i) {
        ivec2 texcoord = ivec2(
                i & MAX_SKINNING_BUFFER_WIDTH_MASK, i >> MAX_SKINNING_BUFFER_WIDTH_LOG2);
        vec2 indexWeight = texelFetch(sampler1_indicesAndWeights, texcoord, 0).rg;
        accumulateBoneNormalTangent(normSum, tangSum, n, t, uint(indexWeight.r), indexWeight.g);
    }
    n = normSum;
    t = tangSum;
}

#define MAX_MORPH_TARGET_BUFFER_WIDTH_LOG2 11
#define MAX_MORPH_TARGET_BUFFER_WIDTH (1 << MAX_MORPH_TARGET_BUFFER_WIDTH_LOG2)
#define MAX_MORPH_TARGET_BUFFER_WIDTH_MASK (MAX_MORPH_TARGET_BUFFER_WIDTH - 1)

#if CLIENT_MATERIAL_API_LEVEL >= UNSTABLE_MATERIAL_API_LEVEL
/**
 * @api-level 2
 */
void morphData2(inout vec2 v, highp sampler2DArray data) {
    int index = getVertexIndex() + pushConstants.morphingBufferOffset;
    ivec3 texcoord = ivec3(
            index & MAX_MORPH_TARGET_BUFFER_WIDTH_MASK,
            index >> MAX_MORPH_TARGET_BUFFER_WIDTH_LOG2, 0);
    int c = object_uniforms_morphTargetCount;
    for (int i = 0; i < c; ++i) {
        float w = morphingUniforms.weights[i][0];
        if (w != 0.0) {
            texcoord.z = i;
            v += w * texelFetch(data, texcoord, 0).xy;
        }
    }
}

/**
 * @api-level 2
 */
void morphData3(inout vec3 v, highp sampler2DArray data) {
    int index = getVertexIndex() + pushConstants.morphingBufferOffset;
    ivec3 texcoord = ivec3(
            index & MAX_MORPH_TARGET_BUFFER_WIDTH_MASK,
            index >> MAX_MORPH_TARGET_BUFFER_WIDTH_LOG2, 0);
    int c = object_uniforms_morphTargetCount;
    for (int i = 0; i < c; ++i) {
        float w = morphingUniforms.weights[i][0];
        if (w != 0.0) {
            texcoord.z = i;
            v += w * texelFetch(data, texcoord, 0).xyz;
        }
    }
}

/**
 * @api-level 2
 */
void morphData4(inout vec4 v, highp sampler2DArray data) {
    int index = getVertexIndex() + pushConstants.morphingBufferOffset;
    ivec3 texcoord = ivec3(
            index & MAX_MORPH_TARGET_BUFFER_WIDTH_MASK,
            index >> MAX_MORPH_TARGET_BUFFER_WIDTH_LOG2, 0);
    int c = object_uniforms_morphTargetCount;
    for (int i = 0; i < c; ++i) {
        float w = morphingUniforms.weights[i][0];
        if (w != 0.0) {
            texcoord.z = i;
            v += w * texelFetch(data, texcoord, 0);
        }
    }
}

void morphPosition(inout vec4 p) {
    morphData4(p, sampler1_positions);
}
#else
#define morphData2 ERROR_morphData2_api_level_2_END
#define morphData3 ERROR_morphData3_api_level_2_END
#define morphData4 ERROR_morphData4_api_level_2_END

void morphPosition(inout vec4 p) {
    int index = getVertexIndex() + pushConstants.morphingBufferOffset;
    ivec3 texcoord = ivec3(
            index & MAX_MORPH_TARGET_BUFFER_WIDTH_MASK,
            index >> MAX_MORPH_TARGET_BUFFER_WIDTH_LOG2, 0);
    int c = object_uniforms_morphTargetCount;
    for (int i = 0; i < c; ++i) {
        float w = morphingUniforms.weights[i][0];
        if (w != 0.0) {
            texcoord.z = i;
            p += w * texelFetch(sampler1_positions, texcoord, 0);
        }
    }
}
#endif

void morphNormal(inout vec3 n) {
    vec3 baseNormal = n;
    int index = getVertexIndex() + pushConstants.morphingBufferOffset;
    ivec3 texcoord = ivec3(
            index & MAX_MORPH_TARGET_BUFFER_WIDTH_MASK,
            index >> MAX_MORPH_TARGET_BUFFER_WIDTH_LOG2, 0);
    int c = object_uniforms_morphTargetCount;
    for (int i = 0; i < c; ++i) {
        float w = morphingUniforms.weights[i][0];
        if (w != 0.0) {
            texcoord.z = i;
            ivec4 tangent = texelFetch(sampler1_tangents, texcoord, 0);
            vec3 normal;
            toTangentFrame(float4(tangent) * (1.0 / 32767.0), normal);
            n += w * (normal - baseNormal);
        }
    }
}
#endif

/** @public-api */
vec4 getPosition() {
    vec4 pos = mesh_position;

#if defined(VARIANT_HAS_SKINNING_OR_MORPHING)
        if ((object_uniforms_flagsChannels & FILAMENT_OBJECT_MORPHING_POSITION_BIT) != 0) {
    #if defined(LEGACY_MORPHING)
            pos += morphingUniforms.weights[0] * mesh_custom0;
            pos += morphingUniforms.weights[1] * mesh_custom1;
            pos += morphingUniforms.weights[2] * mesh_custom2;
            pos += morphingUniforms.weights[3] * mesh_custom3;
    #else
            morphPosition(pos);
    #endif
        }

    if ((object_uniforms_flagsChannels & FILAMENT_OBJECT_SKINNING_ENABLED_BIT) != 0) {
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
// Surface Helpers
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
    // w could be zero (e.g.: with the skybox) which corresponds to an infinite distance in
    // world-space. However, we want to avoid infinites and divides-by-zero, so we use a very
    // small number instead in that case (2^-63 seem to work well).
    const highp float ALMOST_ZERO_FLT = 1.08420217249e-19;
    if (abs(position.w) < ALMOST_ZERO_FLT) {
        position.w = position.w < 0.0 ? -ALMOST_ZERO_FLT : ALMOST_ZERO_FLT;
    }
    return position * (1.0 / position.w);
#else
#error Unknown Vertex Domain
#endif
}
