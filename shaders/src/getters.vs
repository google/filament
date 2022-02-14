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

/** @public-api */
int getVertexIndex() {
#if defined(TARGET_METAL_ENVIRONMENT) || defined(TARGET_VULKAN_ENVIRONMENT)
    return gl_VertexIndex;
#else
    return gl_VertexID;
#endif
}

#if defined(VARIANT_HAS_SKINNING_OR_MORPHING)
vec3 mulBoneNormal(vec3 n, uint i) {

    highp mat3 cof;

    // the first 8 elements of the cofactor matrix are stored as fp16
    highp vec2 x0y0 = unpackHalf2x16(bonesUniforms.bones[i].cof[0]);
    highp vec2 z0x1 = unpackHalf2x16(bonesUniforms.bones[i].cof[1]);
    highp vec2 y1z1 = unpackHalf2x16(bonesUniforms.bones[i].cof[2]);
    highp vec2 x2y2 = unpackHalf2x16(bonesUniforms.bones[i].cof[3]);

    // the last element must be computed by hand
    highp float a = bonesUniforms.bones[i].transform[0][0];
    highp float b = bonesUniforms.bones[i].transform[0][1];
    highp float d = bonesUniforms.bones[i].transform[1][0];
    highp float e = bonesUniforms.bones[i].transform[1][1];

    cof[0].xyz = vec3(x0y0, z0x1.x);
    cof[1].xyz = vec3(z0x1.y, y1z1);
    cof[2].xyz = vec3(x2y2, a * e - b * d);

    return normalize(cof * n);
}

vec3 mulBoneVertex(vec3 v, uint i) {
    // last row of bonesUniforms.transform[i] (row major) is assumed to be [0,0,0,1]
    highp mat4x3 m = transpose(bonesUniforms.bones[i].transform);
    return v.x * m[0].xyz + (v.y * m[1].xyz + (v.z * m[2].xyz + m[3].xyz));
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

#define MAX_MORPH_TARGET_BUFFER_WIDTH 2048

void morphPosition(inout vec4 p) {
    ivec3 texcoord = ivec3(getVertexIndex() % MAX_MORPH_TARGET_BUFFER_WIDTH, getVertexIndex() / MAX_MORPH_TARGET_BUFFER_WIDTH, 0);
    for (uint i = 0u; i < objectUniforms.morphTargetCount; ++i) {
        texcoord.z = int(i);
        p += morphingUniforms.weights[i][0] * texelFetch(morphTargetBuffer_positions, texcoord, 0);
    }
}

void morphNormal(inout vec3 n) {
    ivec3 texcoord = ivec3(getVertexIndex() % MAX_MORPH_TARGET_BUFFER_WIDTH, getVertexIndex() / MAX_MORPH_TARGET_BUFFER_WIDTH, 0);
    for (uint i = 0u; i < objectUniforms.morphTargetCount; ++i) {
        texcoord.z = int(i);
        ivec4 tangent = texelFetch(morphTargetBuffer_tangents, texcoord, 0);
        vec3 normal;
        toTangentFrame(float4(tangent) * (1.0 / 32767.0), normal);
        n += morphingUniforms.weights[i][0] * normal;
    }
}
#endif

/** @public-api */
vec4 getPosition() {
    vec4 pos = mesh_position;

#if defined(VARIANT_HAS_SKINNING_OR_MORPHING)

    if ((objectUniforms.flags & FILAMENT_OBJECT_MORPHING_ENABLED_BIT) != 0u) {
        #if defined(LEGACY_MORPHING)
        pos += morphingUniforms.weights[0] * mesh_custom0;
        pos += morphingUniforms.weights[1] * mesh_custom1;
        pos += morphingUniforms.weights[2] * mesh_custom2;
        pos += morphingUniforms.weights[3] * mesh_custom3;
        #else
        morphPosition(pos);
        #endif
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
