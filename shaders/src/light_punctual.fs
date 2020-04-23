//------------------------------------------------------------------------------
// Punctual lights evaluation
//------------------------------------------------------------------------------

// Make sure this matches the same constants in Froxel.cpp
#define FROXEL_BUFFER_WIDTH_SHIFT   6u
#define FROXEL_BUFFER_WIDTH         (1u << FROXEL_BUFFER_WIDTH_SHIFT)
#define FROXEL_BUFFER_WIDTH_MASK    (FROXEL_BUFFER_WIDTH - 1u)

#define RECORD_BUFFER_WIDTH_SHIFT   5u
#define RECORD_BUFFER_WIDTH         (1u << RECORD_BUFFER_WIDTH_SHIFT)
#define RECORD_BUFFER_WIDTH_MASK    (RECORD_BUFFER_WIDTH - 1u)

#define LIGHT_TYPE_POINT            0u
#define LIGHT_TYPE_SPOT             1u


struct FroxelParams {
    uint recordOffset; // offset at which the list of lights for this froxel starts
    uint count;   // number lights in this froxel
};

/**
 * Returns the coordinates of the froxel at the specified fragment coordinates.
 * The coordinates are a 3D position in the froxel grid.
 */
uvec3 getFroxelCoords(const vec3 fragCoords) {
    uvec3 froxelCoord;

    vec3 adjustedFragCoords = fragCoords;
// In Vulkan and Metal, texture coords are Y-down. In OpenGL, texture coords are Y-up.
#if defined(TARGET_METAL_ENVIRONMENT) || defined(TARGET_VULKAN_ENVIRONMENT)
    adjustedFragCoords.y = frameUniforms.resolution.y - adjustedFragCoords.y;
#endif

    froxelCoord.xy = uvec2((adjustedFragCoords.xy - frameUniforms.origin.xy) *
            vec2(frameUniforms.oneOverFroxelDimension, frameUniforms.oneOverFroxelDimensionY));

    froxelCoord.z = uint(max(0.0,
            log2(frameUniforms.zParams.x * adjustedFragCoords.z + frameUniforms.zParams.y) *
                    frameUniforms.zParams.z + frameUniforms.zParams.w));

    return froxelCoord;
}

/**
 * Computes the froxel index of the fragment at the specified coordinates.
 * The froxel index is computed from the 3D coordinates of the froxel in the
 * froxel grid and later used to fetch from the froxel data texture
 * (light_froxels).
 */
uint getFroxelIndex(const vec3 fragCoords) {
    uvec3 froxelCoord = getFroxelCoords(fragCoords);
    return froxelCoord.x * frameUniforms.fParamsX +
           froxelCoord.y * frameUniforms.fParams.x +
           froxelCoord.z * frameUniforms.fParams.y;
}

/**
 * Computes the texture coordinates of the froxel data given a froxel index.
 */
ivec2 getFroxelTexCoord(uint froxelIndex) {
    return ivec2(froxelIndex & FROXEL_BUFFER_WIDTH_MASK, froxelIndex >> FROXEL_BUFFER_WIDTH_SHIFT);
}

/**
 * Returns the froxel data for the given froxel index. The data is fetched
 * from the light_froxels texture.
 */
FroxelParams getFroxelParams(uint froxelIndex) {
    ivec2 texCoord = getFroxelTexCoord(froxelIndex);
    uvec2 entry = texelFetch(light_froxels, texCoord, 0).rg;

    FroxelParams froxel;
    froxel.recordOffset = entry.r;
    froxel.count = entry.g & 0xFFu;
    return froxel;
}

/**
 * Returns the coordinates of the light record in the light_records texture
 * given the specified index. A light record is a single uint index into the
 * lights data buffer (lightsUniforms UBO).
 */
ivec2 getRecordTexCoord(uint index) {
    return ivec2(index & RECORD_BUFFER_WIDTH_MASK, index >> RECORD_BUFFER_WIDTH_SHIFT);
}

float getSquareFalloffAttenuation(float distanceSquare, float falloff) {
    float factor = distanceSquare * falloff;
    float smoothFactor = saturate(1.0 - factor * factor);
    // We would normally divide by the square distance here
    // but we do it at the call site
    return smoothFactor * smoothFactor;
}

float getDistanceAttenuation(const highp vec3 posToLight, float falloff) {
    float distanceSquare = dot(posToLight, posToLight);
    float attenuation = getSquareFalloffAttenuation(distanceSquare, falloff);
    // Assume a punctual light occupies a volume of 1cm to avoid a division by 0
    return attenuation * 1.0 / max(distanceSquare, 1e-4);
}

float getAngleAttenuation(const vec3 lightDir, const vec3 l, const vec2 scaleOffset) {
    float cd = dot(lightDir, l);
    float attenuation = saturate(cd * scaleOffset.x + scaleOffset.y);
    return attenuation * attenuation;
}

/**
 * Returns a Light structure (see common_lighting.fs) describing a point or spot light.
 * The colorIntensity field will store the *pre-exposed* intensity of the light
 * in the w component.
 *
 * The light parameters used to compute the Light structure are fetched from the
 * lightsUniforms uniform buffer.
 */
Light getLight(const uint index) {

    // retrieve the light data from the UBO
    ivec2 texCoord = getRecordTexCoord(index);
    uint lightIndex = texelFetch(light_records, texCoord, 0).r;
    highp vec4 positionFalloff       = lightsUniforms.lights[lightIndex][0];
    highp vec4 colorIntensity        = lightsUniforms.lights[lightIndex][1];
          vec4 directionIES          = lightsUniforms.lights[lightIndex][2];
    highp vec4 scaleOffsetShadowType = lightsUniforms.lights[lightIndex][3];

    // poition-to-light vector
    highp vec3 worldPosition = vertex_worldPosition;
    highp vec3 posToLight = positionFalloff.xyz - worldPosition;

    // and populate the Light structure
    Light light;
    light.colorIntensity.rgb = colorIntensity.rgb;
    light.colorIntensity.w = computePreExposedIntensity(colorIntensity.w, frameUniforms.exposure);
    light.l = normalize(posToLight);
    light.attenuation = getDistanceAttenuation(posToLight, positionFalloff.w);
    light.NoL = saturate(dot(shading_normal, light.l));

    uint type = floatBitsToUint(scaleOffsetShadowType.w);
    if (type == LIGHT_TYPE_SPOT) {
        light.attenuation *= getAngleAttenuation(-directionIES.xyz, light.l, scaleOffsetShadowType.xy);
        uint shadowBits = floatBitsToUint(scaleOffsetShadowType.z);
        light.castsShadows = bool(shadowBits & 0x1u);
        light.contactShadows = bool((shadowBits >> 1u) & 0x1u);
        light.shadowIndex = (shadowBits >> 2u) & 0xFu;
        light.shadowLayer = (shadowBits >> 6u) & 0xFu;
    }

    return light;
}

/**
 * Evaluates all punctual lights that my affect the current fragment.
 * The result of the lighting computations is accumulated in the color
 * parameter, as linear HDR RGB.
 */
void evaluatePunctualLights(const PixelParams pixel, inout vec3 color) {
    // Fetch the light information stored in the froxel that contains the
    // current fragment
    FroxelParams froxel = getFroxelParams(getFroxelIndex(gl_FragCoord.xyz));

    // Each froxel contains how many lights can influence
    // the current fragment. A froxel also contains a record offset that
    // tells us where the indices of those lights are in the records
    // texture. The records texture contains the indices of the actual
    // light data in the lightsUniforms uniform buffer

    uint index = froxel.recordOffset;
    uint end = index + froxel.count;

    // Iterate point lights
    for ( ; index < end; index++) {
        Light light = getLight(index);
        float visibility = 1.0;
#if defined(HAS_SHADOWING)
        if (light.NoL > 0.0){
            if (light.castsShadows) {
                visibility = shadow(light_shadowMap, light.shadowLayer,
                getSpotLightSpacePosition(light.shadowIndex));
            }
            if (light.contactShadows && visibility > 0.0) {
                if (objectUniforms.screenSpaceContactShadows != 0u) {
                    visibility *= 1.0 - screenSpaceContactShadow(light.l);
                }
            }
        }
#endif
#if defined(MATERIAL_CAN_SKIP_LIGHTING)
        if (light.NoL <= 0.0) {
            continue;
        }
#endif
        color.rgb += surfaceShading(pixel, light, visibility);
    }
}
