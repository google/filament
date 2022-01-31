//------------------------------------------------------------------------------
// Punctual lights evaluation
//------------------------------------------------------------------------------

// Make sure this matches the same constants in Froxel.cpp
#define FROXEL_BUFFER_WIDTH_SHIFT   6u
#define FROXEL_BUFFER_WIDTH         (1u << FROXEL_BUFFER_WIDTH_SHIFT)
#define FROXEL_BUFFER_WIDTH_MASK    (FROXEL_BUFFER_WIDTH - 1u)

#define RECORD_BUFFER_WIDTH_SHIFT   4u
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
uvec3 getFroxelCoords(const highp vec3 fragCoords) {
    uvec3 froxelCoord;

    froxelCoord.xy = uvec2(fragCoords.xy * frameUniforms.resolution.xy *
            vec2(frameUniforms.oneOverFroxelDimension, frameUniforms.oneOverFroxelDimensionY));

    // go from screen-space to reciprocal of normalized view-space Z (i.e. scaled by 1/zLightFar)
    // we get away with the reciprocal because 1/z is handled by the log2() below.
    // see Froxelizer.cpp
    highp float viewSpaceNormalizedZ = frameUniforms.zParams.x * fragCoords.z + frameUniforms.zParams.y;

    // frameUniforms.zParams.w is actually the number of z-slices, make sure it's mediump
    float zSliceCount = frameUniforms.zParams.w;

    // compute the sliceZ mapping in highp, store in mediump
    float sliceZWithoutOffset = log2(viewSpaceNormalizedZ) * frameUniforms.zParams.z;

    // finally discretize the mapping into slices
    // We need to clamp because the far plane (z=1) is out of bounds, any smaller z is not.
    froxelCoord.z = uint(clamp(sliceZWithoutOffset + zSliceCount, 0.0, zSliceCount - 1.0));

    return froxelCoord;
}

/**
 * Computes the froxel index of the fragment at the specified coordinates.
 * The froxel index is computed from the 3D coordinates of the froxel in the
 * froxel grid and later used to fetch from the froxel data texture
 * (light_froxels).
 */
uint getFroxelIndex(const highp vec3 fragCoords) {
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
 * Return the light index from the record index
 * A light record is a single uint index into the lights data buffer (lightsUniforms UBO).
 */
uint getLightIndex(const uint index) {
    uint v = index >> 4u;
    uint c = (index >> 2u) & 0x3u;
    uint s = (index & 0x3u) * 8u;
    // this intermediate is needed to workaround a bug on qualcomm h/w
    highp uvec4 d = froxelRecordUniforms.records[v];
    return (d[c] >> s) & 0xFFu;
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
    // light far attenuation
    highp vec3 v = getWorldPosition() - getWorldCameraPosition();
    float d = dot(v, v);
    attenuation *= saturate(frameUniforms.lightFarAttenuationParams.x - d * frameUniforms.lightFarAttenuationParams.y);
    // Assume a punctual light occupies a volume of 1cm to avoid a division by 0
    return attenuation / max(distanceSquare, 1e-4);
}

float getAngleAttenuation(const highp vec3 lightDir, const highp vec3 l, const highp vec2 scaleOffset) {
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
    uint lightIndex = getLightIndex(index);

    highp mat4 data = lightsUniforms.lights[lightIndex];

    highp vec4 positionFalloff = data[0];
    highp vec3 direction = data[1].xyz;
    vec4 colorIES = vec4(
        unpackHalf2x16(floatBitsToUint(data[2][0])),
        unpackHalf2x16(floatBitsToUint(data[2][1]))
    );
    highp vec2 scaleOffset = data[2].zw;
    highp float intensity = data[3][1];
    highp uint typeShadow = floatBitsToUint(data[3][2]);
    highp uint channels = floatBitsToUint(data[3][3]);

    // poition-to-light vector
    highp vec3 worldPosition = getWorldPosition();
    highp vec3 posToLight = positionFalloff.xyz - worldPosition;

    // and populate the Light structure
    Light light;
    light.colorIntensity.rgb = colorIES.rgb;
    light.colorIntensity.w = computePreExposedIntensity(intensity, frameUniforms.exposure);
    light.l = normalize(posToLight);
    light.attenuation = getDistanceAttenuation(posToLight, positionFalloff.w);
    light.NoL = saturate(dot(shading_normal, light.l));
    light.worldPosition = positionFalloff.xyz;
    light.castsShadows = false;
    light.contactShadows = false;
    light.shadowIndex = 0u;
    light.shadowLayer = 0u;
    light.channels = channels;

    uint type = typeShadow & 0x1u;
    if (type == LIGHT_TYPE_SPOT) {
        light.attenuation *= getAngleAttenuation(-direction, light.l, scaleOffset);
        light.contactShadows = bool(typeShadow & 0x10u);
        light.shadowIndex = (typeShadow >>  8u) & 0xFFu;
        light.shadowLayer = (typeShadow >> 16u) & 0xFFu;
        light.castsShadows   = bool(channels & 0x10000u);
    }

    return light;
}

/**
 * Evaluates all punctual lights that my affect the current fragment.
 * The result of the lighting computations is accumulated in the color
 * parameter, as linear HDR RGB.
 */
void evaluatePunctualLights(const MaterialInputs material,
        const PixelParams pixel, inout vec3 color) {

    // Fetch the light information stored in the froxel that contains the
    // current fragment
    FroxelParams froxel = getFroxelParams(getFroxelIndex(getNormalizedViewportCoord2()));

    // Each froxel contains how many lights can influence
    // the current fragment. A froxel also contains a record offset that
    // tells us where the indices of those lights are in the records
    // texture. The records texture contains the indices of the actual
    // light data in the lightsUniforms uniform buffer

    uint index = froxel.recordOffset;
    uint end = index + froxel.count;
    uint channels = objectUniforms.channels & 0xFFu;

    // Iterate point lights
    for ( ; index < end; index++) {
        Light light = getLight(index);
        if ((light.channels & channels) == 0u) {
            continue;
        }

#if defined(MATERIAL_CAN_SKIP_LIGHTING)
        if (light.NoL <= 0.0 || light.attenuation <= 0.0) {
            continue;
        }
#endif

        float visibility = 1.0;
#if defined(VARIANT_HAS_SHADOWING)
        if (light.NoL > 0.0) {
            if (light.castsShadows) {
                visibility = shadow(false, light_shadowMap, light.shadowLayer, light.shadowIndex, 0u);
            }
            if (light.contactShadows && visibility > 0.0) {
                if ((objectUniforms.flags & FILAMENT_OBJECT_CONTACT_SHADOWS_BIT) != 0u) {
                    visibility *= 1.0 - screenSpaceContactShadow(light.l);
                }
            }
#if defined(MATERIAL_CAN_SKIP_LIGHTING)
            if (visibility <= 0.0) {
                continue;
            }
#endif
        }
#endif

#if defined(MATERIAL_HAS_CUSTOM_SURFACE_SHADING)
        color.rgb += customSurfaceShading(material, pixel, light, visibility);
#else
        color.rgb += surfaceShading(pixel, light, visibility);
#endif
    }
}
