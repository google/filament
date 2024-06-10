//------------------------------------------------------------------------------
// Uniforms access
//------------------------------------------------------------------------------

/** @public-api */
mat4 getWorldFromModelMatrix() {
    return object_uniforms_worldFromModelMatrix;
}

/** @public-api */
mat3 getWorldFromModelNormalMatrix() {
    return object_uniforms_worldFromModelNormalMatrix;
}

/** sort-of public */
float getObjectUserData() {
    return object_uniforms_userData;
}

//------------------------------------------------------------------------------
// Attributes access
//------------------------------------------------------------------------------

#if __VERSION__ >= 300
/** @public-api */
int getVertexIndex() {
#if defined(TARGET_METAL_ENVIRONMENT) || defined(TARGET_VULKAN_ENVIRONMENT)
    return gl_VertexIndex;
#else
    return gl_VertexID;
#endif
}
#endif

#if defined(VARIANT_HAS_SKINNING_OR_MORPHING)
#define MAX_SKINNING_BUFFER_WIDTH 2048u
vec3 mulBoneNormal(vec3 n, uint j) {

    highp mat3 cof;

    // the last element must be computed by hand
    highp float a = bonesUniforms.bones[j].transform[0][0];
    highp float b = bonesUniforms.bones[j].transform[0][1];
    highp float c = bonesUniforms.bones[j].transform[0][2];
    highp float d = bonesUniforms.bones[j].transform[1][0];
    highp float e = bonesUniforms.bones[j].transform[1][1];
    highp float f = bonesUniforms.bones[j].transform[1][2];
    highp float g = bonesUniforms.bones[j].transform[2][0];
    highp float h = bonesUniforms.bones[j].transform[2][1];
    highp float i = bonesUniforms.bones[j].transform[2][2];

    cof[0] = bonesUniforms.bones[j].cof0;

    cof[1].xyz = vec3(
            bonesUniforms.bones[j].cof1x,
            a * i - c * g,
            c * d - a * f);

    cof[2].xyz = vec3(
            d * h - e * g,
            b * g - a * h,
            a * e - b * d);

    return normalize(cof * n);
}

vec3 mulBoneVertex(vec3 v, uint i) {
    // last row of bonesUniforms.transform[i] (row major) is assumed to be [0,0,0,1]
    highp mat4x3 m = transpose(bonesUniforms.bones[i].transform);
    return v.x * m[0].xyz + (v.y * m[1].xyz + (v.z * m[2].xyz + m[3].xyz));
}

void skinPosition(inout vec3 p, const uvec4 ids, const vec4 weights) {
    // standard skinning for 4 weights, some of them could be zero
    if (weights.w >= 0.0) {
        p = weights.x * mulBoneVertex(p, uint(ids.x))
            + weights.y * mulBoneVertex(p, uint(ids.y))
            + weights.z * mulBoneVertex(p, uint(ids.z))
            + weights.w * mulBoneVertex(p, uint(ids.w));
        return;
    }
    // skinning for >4 weights
    vec3 posSum = weights.x * mulBoneVertex(p, uint(ids.x));
    posSum += weights.y * mulBoneVertex(p, uint(ids.y));
    posSum += weights.z * mulBoneVertex(p, uint(ids.z));
    uint pairIndex = uint(-weights.w - 1.);
    uint pairStop = pairIndex + uint(ids.w - 3u);
    for (uint i = pairIndex; i < pairStop; ++i) {
        ivec2 texcoord = ivec2(i % MAX_SKINNING_BUFFER_WIDTH, i / MAX_SKINNING_BUFFER_WIDTH);
        vec2 indexWeight = texelFetch(bonesBuffer_indicesAndWeights, texcoord, 0).rg;
        posSum += mulBoneVertex(p, uint(indexWeight.r)) * indexWeight.g;
    }
    p = posSum;
}

void skinNormal(inout vec3 n, const uvec4 ids, const vec4 weights) {
    // standard skinning for 4 weights, some of them could be zero
    if (weights.w >= 0.0) {
        n = weights.x * mulBoneNormal(n, uint(ids.x))
            + weights.y * mulBoneNormal(n, uint(ids.y))
            + weights.z * mulBoneNormal(n, uint(ids.z))
            + weights.w * mulBoneNormal(n, uint(ids.w));
        return;
    }
    // skinning for >4 weights
    vec3 normSum = weights.x * mulBoneNormal(n, uint(ids.x));
    normSum += weights.y * mulBoneNormal(n, uint(ids.y));
    normSum += weights.z * mulBoneNormal(n, uint(ids.z));
    uint pairIndex = uint(-weights.w - 1.);
    uint pairStop = pairIndex + uint(ids.w - 3u);
    for (uint i = pairIndex; i < pairStop; i = i + 1u) {
        ivec2 texcoord = ivec2(i % MAX_SKINNING_BUFFER_WIDTH, i / MAX_SKINNING_BUFFER_WIDTH);
        vec2 indexWeight = texelFetch(bonesBuffer_indicesAndWeights, texcoord, 0).rg;

        normSum += mulBoneNormal(n, uint(indexWeight.r)) * indexWeight.g;
    }
    n = normSum;
}

void skinNormalTangent(inout vec3 n, inout vec3 t, const uvec4 ids, const vec4 weights) {
    // standard skinning for 4 weights, some of them could be zero
    if (weights.w >= 0.0) {
        n = weights.x * mulBoneNormal(n, uint(ids.x))
            + weights.y * mulBoneNormal(n, uint(ids.y))
            + weights.z * mulBoneNormal(n, uint(ids.z))
            + weights.w * mulBoneNormal(n, uint(ids.w));
        t = weights.x * mulBoneNormal(t, uint(ids.x))
            + weights.y * mulBoneNormal(t, uint(ids.y))
            + weights.z * mulBoneNormal(t, uint(ids.z))
            + weights.w * mulBoneNormal(t, uint(ids.w));
        return;
    }
    // skinning for >4 weights
    vec3 normSum = weights.x * mulBoneNormal(n, uint(ids.x));
    normSum += weights.y * mulBoneNormal(n, uint(ids.y)) ;
    normSum += weights.z * mulBoneNormal(n, uint(ids.z));
    vec3 tangSum = weights.x * mulBoneNormal(t, uint(ids.x));
    tangSum += weights.y * mulBoneNormal(t, uint(ids.y));
    tangSum += weights.z * mulBoneNormal(t, uint(ids.z));
    uint pairIndex = uint(-weights.w - 1.);
    uint pairStop = pairIndex + uint(ids.w - 3u);
    for (uint i = pairIndex; i < pairStop; i = i + 1u) {
        ivec2 texcoord = ivec2(i % MAX_SKINNING_BUFFER_WIDTH, i / MAX_SKINNING_BUFFER_WIDTH);
        vec2 indexWeight = texelFetch(bonesBuffer_indicesAndWeights, texcoord, 0).rg;

        normSum += mulBoneNormal(n, uint(indexWeight.r)) * indexWeight.g;
        tangSum += mulBoneNormal(t, uint(indexWeight.r)) * indexWeight.g;
    }
    n = normSum;
    t = tangSum;
}

#define MAX_MORPH_TARGET_BUFFER_WIDTH 2048

void morphPosition(inout vec4 p) {
    int index = getVertexIndex() + pushConstants.morphingBufferOffset;
    ivec3 texcoord = ivec3(index % MAX_MORPH_TARGET_BUFFER_WIDTH, index / MAX_MORPH_TARGET_BUFFER_WIDTH, 0);
    int c = object_uniforms_morphTargetCount;
    for (int i = 0; i < c; ++i) {
        float w = morphingUniforms.weights[i][0];
        if (w != 0.0) {
            texcoord.z = i;
            p += w * texelFetch(morphTargetBuffer_positions, texcoord, 0);
        }
    }
}

void morphNormal(inout vec3 n) {
    vec3 baseNormal = n;
    int index = getVertexIndex() + pushConstants.morphingBufferOffset;
    ivec3 texcoord = ivec3(index % MAX_MORPH_TARGET_BUFFER_WIDTH, index / MAX_MORPH_TARGET_BUFFER_WIDTH, 0);
    int c = object_uniforms_morphTargetCount;
    for (int i = 0; i < c; ++i) {
        float w = morphingUniforms.weights[i][0];
        if (w != 0.0) {
            texcoord.z = i;
            ivec4 tangent = texelFetch(morphTargetBuffer_tangents, texcoord, 0);
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

    if ((object_uniforms_flagsChannels & FILAMENT_OBJECT_MORPHING_ENABLED_BIT) != 0) {
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

/**
 * Index of the eye being rendered, starting at 0.
 * @public-api
 */
int getEyeIndex() {
#if defined(VARIANT_HAS_STEREO) && defined(FILAMENT_STEREO_INSTANCED)
    return instance_index % CONFIG_STEREO_EYE_COUNT;
#elif defined(VARIANT_HAS_STEREO) && defined(FILAMENT_STEREO_MULTIVIEW)
    // gl_ViewID_OVR is of uint type, which needs an explicit conversion.
    return int(gl_ViewID_OVR);
#endif
    return 0;
}
