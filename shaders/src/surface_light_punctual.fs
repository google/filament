//------------------------------------------------------------------------------
// Punctual lights evaluation
//------------------------------------------------------------------------------

// Make sure this matches the same constants in Froxel.cpp
#define FROXEL_BUFFER_WIDTH_SHIFT   6u
#define FROXEL_BUFFER_WIDTH         (1u << FROXEL_BUFFER_WIDTH_SHIFT)
#define FROXEL_BUFFER_WIDTH_MASK    (FROXEL_BUFFER_WIDTH - 1u)

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

    froxelCoord.xy = uvec2(fragCoords.xy * frameUniforms.froxelCountXY);

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
 * froxel grid and later used to fetch from the froxel buffer.
 */
uint getFroxelIndex(const highp vec3 fragCoords) {
    uvec3 froxelCoord = getFroxelCoords(fragCoords);
    return froxelCoord.x * frameUniforms.fParams.x +
           froxelCoord.y * frameUniforms.fParams.y +
           froxelCoord.z * frameUniforms.fParams.z;
}

/**
 * Computes the texture coordinates of the froxel data given a froxel index.
 */
ivec2 getFroxelTexCoord(uint froxelIndex) {
    return ivec2(froxelIndex & FROXEL_BUFFER_WIDTH_MASK, froxelIndex >> FROXEL_BUFFER_WIDTH_SHIFT);
}

/**
 * Returns the froxel data for the given froxel index. The data is fetched
 * from FroxelsUniforms UBO.
 */
FroxelParams getFroxelParams(const uint froxelIndex) {
    uint w = froxelIndex >> 2u;
    uint c = froxelIndex & 0x3u;
    highp uvec4 d = froxelsUniforms.records[w];
    highp uint f = d[c];
    FroxelParams froxel;
    froxel.recordOffset = f >> 16u;
    froxel.count = f & 0xFFu;
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

float getAngleAttenuation(const highp vec3 lightDir, const highp vec3 l, const highp vec2 scaleOffset) {
    float cd = dot(lightDir, l);
    float attenuation = saturate(cd * scaleOffset.x + scaleOffset.y);
    return attenuation * attenuation;
}

/**
 * Evaluates all punctual lights that may affect the current fragment.
 * The result of the lighting computations is accumulated in the color
 * parameter, as linear HDR RGB.
 */
void evaluatePunctualLights(const MaterialInputs material,
        const PixelParams pixel, inout vec3 color) {

#if !defined(MATERIAL_HAS_CUSTOM_SURFACE_SHADING)
    if (!CONFIG_DEBUG_FROXEL_VISUALIZATION) {
        highp vec3 v = getWorldPosition() - getWorldCameraPosition();
        if (dot(v, v) * frameUniforms.lightFarAttenuationParams.y >=
                frameUniforms.lightFarAttenuationParams.x) {
            return;
        }
    }
#endif

    // Fetch the light information stored in the froxel that contains the
    // current fragment
    FroxelParams froxel = getFroxelParams(getFroxelIndex(getNormalizedPhysicalViewportCoord()));

    // Each froxel contains how many lights can influence
    // the current fragment. A froxel also contains a record offset that
    // tells us where the indices of those lights are in the records
    // buffer. The records buffer contains the indices of the actual
    // light data in the lightsUniforms UBO.

    uint index = froxel.recordOffset;
    uint end = index + froxel.count;
    int channels = object_uniforms_flagsChannels & 0xFF;

    // Iterate point lights
    for ( ; index < end; index++) {
        uint lightIndex = getLightIndex(index);

        highp vec4 data3 = lightsUniforms.lights[lightIndex][3];
        highp uint lightChannels = floatBitsToUint(data3[3]);
        if ((int(lightChannels) & channels) == 0) {
            continue;
        }

        highp vec4 positionFalloff = lightsUniforms.lights[lightIndex][0];
        highp vec3 worldPosition = getWorldPosition();
        highp vec3 posToLight = positionFalloff.xyz - worldPosition;
        highp float distanceSquare = dot(posToLight, posToLight);
        float factor = distanceSquare * positionFalloff.w;
#if !defined(MATERIAL_HAS_CUSTOM_SURFACE_SHADING)
        if (factor >= 1.0) {
            continue;
        }
#endif

        highp float rawNoL = dot(shading_normal, posToLight);
#if defined(MATERIAL_CAN_SKIP_LIGHTING)
        if (rawNoL <= 0.0) {
            continue;
        }
#endif

        highp float invDist = inversesqrt(max(distanceSquare, 1e-12));
        vec3 l = posToLight * invDist;

        float smoothFactor = saturate(1.0 - factor * factor);
        float attenuation = smoothFactor * smoothFactor;
        highp vec3 v = worldPosition - getWorldCameraPosition();
        attenuation *= saturate(frameUniforms.lightFarAttenuationParams.x -
                dot(v, v) * frameUniforms.lightFarAttenuationParams.y);
        // Assume a punctual light occupies a volume of 1cm to avoid a division by 0
        attenuation *= min(invDist * invDist, 1e4);

        highp uint typeShadow = floatBitsToUint(data3[2]);
        highp vec3 direction = lightsUniforms.lights[lightIndex][1].xyz;
        highp vec4 data2 = lightsUniforms.lights[lightIndex][2];

        Light light;
        light.attenuation = attenuation;
        light.direction = direction;
        light.l = l;
        light.NoL = saturate(rawNoL * invDist);
        light.worldPosition = positionFalloff.xyz;
        light.channels = int(lightChannels);
        light.contactShadows = bool(typeShadow & 0x10u);
#if defined(MATERIAL_HAS_LIGHTING)
        if (RUNTIME_CONFIG_HAS_DYNAMIC_LIGHTING) {
            light.lightType = (typeShadow & 0x1u);
#if defined(VARIANT_HAS_SHADOWING)
            light.shadowIndex = int((typeShadow >> 8u) & 0xFFu);
            light.castsShadows = bool(lightChannels & 0x10000u);
#endif
            if (light.lightType == LIGHT_TYPE_SPOT) {
                light.attenuation *= getAngleAttenuation(-direction, light.l, data2.zw);
            }
        }
#endif

#if !defined(MATERIAL_HAS_CUSTOM_SURFACE_SHADING)
        if (light.attenuation <= 0.0) {
            continue;
        }
#endif

        light.colorIntensity.rg = unpackHalf2x16(floatBitsToUint(data2[0]));
        light.colorIntensity.b = unpackHalf2x16(floatBitsToUint(data2[1])).x;
        light.colorIntensity.w = computePreExposedIntensity(data3[1], frameUniforms.exposure);

        float visibility = 1.0;
#if defined(VARIANT_HAS_SHADOWING)
        if (light.NoL > 0.0) {
            if (light.castsShadows) {
                int shadowIndex = light.shadowIndex;
                if (light.lightType == LIGHT_TYPE_POINT) {
                    // point-light shadows are sampled from a direction
                    int face = getPointLightFace(-posToLight);
                    shadowIndex += face;
                }
                highp float zLight = dot(shadowUniforms.shadows[shadowIndex].lightFromWorldZ,
                        vec4(worldPosition, 1.0));
                light.zLight = zLight;
                highp vec4 shadowPosition = getShadowPosition(shadowIndex, light.direction, zLight);
                visibility = shadow(false, sampler0_shadowMap, shadowIndex,
                        shadowPosition, zLight);
#if defined(MATERIAL_HAS_SHADOW_STRENGTH)
                applyShadowStrength(visibility, material.shadowStrength);
#endif
            }
            if (light.contactShadows && visibility > 0.0) {
                if ((object_uniforms_flagsChannels & FILAMENT_OBJECT_CONTACT_SHADOWS_BIT) != 0) {
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

    if (CONFIG_DEBUG_FROXEL_VISUALIZATION) {
        if (froxel.count > 0u && frameUniforms.enableFroxelViz != 0) {
            const vec3 debugColors[17] = vec3[](
                vec3(0.0,     0.0,     0.0),         // black
                vec3(0.0,     0.0,     0.1647),      // darkest blue
                vec3(0.0,     0.0,     0.3647),      // darker blue
                vec3(0.0,     0.0,     0.6647),      // dark blue
                vec3(0.0,     0.0,     0.9647),      // blue
                vec3(0.0,     0.9255,  0.9255),      // cyan
                vec3(0.0,     0.5647,  0.0),         // dark green
                vec3(0.0,     0.7843,  0.0),         // green
                vec3(1.0,     1.0,     0.0),         // yellow
                vec3(0.90588, 0.75294, 0.0),         // yellow-orange
                vec3(1.0,     0.5647,  0.0),         // orange
                vec3(1.0,     0.0,     0.0),         // bright red
                vec3(0.8392,  0.0,     0.0),         // red
                vec3(1.0,     0.0,     1.0),         // magenta
                vec3(0.6,     0.3333,  0.7882),      // purple
                vec3(1.0,     1.0,     1.0),         // white
                vec3(1.0,     1.0,     1.0)          // white
            );
            color = mix(color, debugColors[clamp(froxel.count, 0u, 16u)], 0.8);
        }
    }
}
