/*
 * This is the main vertex shader of surface materials. It can be invoked with
 * USE_OPTIMIZED_DEPTH_VERTEX_SHADER defined, and in this case we are guaranteed that the
 * DEPTH variant is active *AND* there is no custom vertex shader (i.e.: materialVertex() is
 * empty).
 * We can use this to remove all code that doesn't participate in the depth computation.
 */

void main() {
    // Initialize the inputs to sensible default values, see material_inputs.vs
#if defined(USE_OPTIMIZED_DEPTH_VERTEX_SHADER)

    // In USE_OPTIMIZED_DEPTH_VERTEX_SHADER mode, we can even skip this if we're already in
    // VERTEX_DOMAIN_DEVICE and we don't have VSM.
#if !defined(VERTEX_DOMAIN_DEVICE) || defined(VARIANT_HAS_VSM)
    // Run initMaterialVertex to compute material.worldPosition.
    MaterialVertexInputs material;
    initMaterialVertex(material);
    // materialVertex() is guaranteed to be empty here, but we keep it to workaround some problem
    // in NVIDA drivers related to depth invariance.
    materialVertex(material);
#endif

#else // defined(USE_OPTIMIZED_DEPTH_VERTEX_SHADER)

    MaterialVertexInputs material;
    initMaterialVertex(material);

#if defined(HAS_ATTRIBUTE_TANGENTS)
    // If the material defines a value for the "normal" property, we need to output
    // the full orthonormal basis to apply normal mapping
    #if defined(MATERIAL_NEEDS_TBN)
        // Extract the normal and tangent in world space from the input quaternion
        // We encode the orthonormal basis as a quaternion to save space in the attributes
        toTangentFrame(mesh_tangents, material.worldNormal, vertex_worldTangent.xyz);

        #if defined(VARIANT_HAS_SKINNING_OR_MORPHING)
        if ((objectUniforms.flags & FILAMENT_OBJECT_MORPHING_ENABLED_BIT) != 0u) {
            #if defined(LEGACY_MORPHING)
            vec3 normal0, normal1, normal2, normal3;
            toTangentFrame(mesh_custom4, normal0);
            toTangentFrame(mesh_custom5, normal1);
            toTangentFrame(mesh_custom6, normal2);
            toTangentFrame(mesh_custom7, normal3);
            material.worldNormal += morphingUniforms.weights[0].xyz * normal0;
            material.worldNormal += morphingUniforms.weights[1].xyz * normal1;
            material.worldNormal += morphingUniforms.weights[2].xyz * normal2;
            material.worldNormal += morphingUniforms.weights[3].xyz * normal3;
            #else
            morphNormal(material.worldNormal);
            material.worldNormal = normalize(material.worldNormal);
            #endif
        }

        if ((objectUniforms.flags & FILAMENT_OBJECT_SKINNING_ENABLED_BIT) != 0u) {
            skinNormal(material.worldNormal, mesh_bone_indices, mesh_bone_weights);
            skinNormal(vertex_worldTangent.xyz, mesh_bone_indices, mesh_bone_weights);
        }
        #endif

        // We don't need to normalize here, even if there's a scale in the matrix
        // because we ensure the worldFromModelNormalMatrix pre-scales the normal such that
        // all its components are < 1.0. This prevents the bitangent to exceed the range of fp16
        // in the fragment shader, where we renormalize after interpolation
        vertex_worldTangent.xyz = objectUniforms.worldFromModelNormalMatrix * vertex_worldTangent.xyz;
        vertex_worldTangent.w = mesh_tangents.w;
        material.worldNormal = objectUniforms.worldFromModelNormalMatrix * material.worldNormal;
    #else // MATERIAL_NEEDS_TBN
        // Without anisotropy or normal mapping we only need the normal vector
        toTangentFrame(mesh_tangents, material.worldNormal);

        #if defined(VARIANT_HAS_SKINNING_OR_MORPHING)
            if ((objectUniforms.flags & FILAMENT_OBJECT_SKINNING_ENABLED_BIT) != 0u) {
                skinNormal(material.worldNormal, mesh_bone_indices, mesh_bone_weights);
            }
        #endif

        material.worldNormal = objectUniforms.worldFromModelNormalMatrix * material.worldNormal;

    #endif // MATERIAL_HAS_ANISOTROPY || MATERIAL_HAS_NORMAL || MATERIAL_HAS_CLEAR_COAT_NORMAL
#endif // HAS_ATTRIBUTE_TANGENTS

    // Invoke user code
    materialVertex(material);

    // Handle built-in interpolated attributes
#if defined(HAS_ATTRIBUTE_COLOR)
    vertex_color = material.color;
#endif
#if defined(HAS_ATTRIBUTE_UV0)
    vertex_uv01.xy = material.uv0;
#endif
#if defined(HAS_ATTRIBUTE_UV1)
    vertex_uv01.zw = material.uv1;
#endif

    // Handle user-defined interpolated attributes
#if defined(VARIABLE_CUSTOM0)
    VARIABLE_CUSTOM_AT0 = material.VARIABLE_CUSTOM0;
#endif
#if defined(VARIABLE_CUSTOM1)
    VARIABLE_CUSTOM_AT1 = material.VARIABLE_CUSTOM1;
#endif
#if defined(VARIABLE_CUSTOM2)
    VARIABLE_CUSTOM_AT2 = material.VARIABLE_CUSTOM2;
#endif
#if defined(VARIABLE_CUSTOM3)
    VARIABLE_CUSTOM_AT3 = material.VARIABLE_CUSTOM3;
#endif

    // The world position can be changed by the user in materialVertex()
    vertex_worldPosition.xyz = material.worldPosition.xyz;

#ifdef HAS_ATTRIBUTE_TANGENTS
    vertex_worldNormal = material.worldNormal;
#endif

#if defined(VARIANT_HAS_SHADOWING) && defined(VARIANT_HAS_DIRECTIONAL_LIGHTING)
    vertex_lightSpacePosition = computeLightSpacePosition(
            vertex_worldPosition.xyz, vertex_worldNormal,
            frameUniforms.lightDirection, frameUniforms.shadowBias, getLightFromWorldMatrix());
#endif

#endif // !defined(USE_OPTIMIZED_DEPTH_VERTEX_SHADER)


#if defined(VERTEX_DOMAIN_DEVICE)
    // The other vertex domains are handled in initMaterialVertex()->computeWorldPosition()
    gl_Position = getPosition();
#else
    gl_Position = getClipFromWorldMatrix() * getWorldPosition(material);
#endif

#if !defined(USE_OPTIMIZED_DEPTH_VERTEX_SHADER)
#if defined(MATERIAL_HAS_CLIP_SPACE_TRANSFORM)
    gl_Position = getClipSpaceTransform(material) * gl_Position;
#endif
#endif // !USE_OPTIMIZED_DEPTH_VERTEX_SHADER

#if defined(VERTEX_DOMAIN_DEVICE)
    // GL convention to inverted DX convention (must happen after clipSpaceTransform)
    gl_Position.z = gl_Position.z * -0.5 + 0.5;
#endif

#if defined(VARIANT_HAS_VSM)
    // For VSM, we use the linear light-space Z coordinate as the depth metric, which works for both
    // directional and spot lights and can be safely interpolated.
    // The value is guaranteed to be between [-znear, -zfar] by construction of viewFromWorldMatrix,
    // (see ShadowMap.cpp).
    // Use vertex_worldPosition.w which is otherwise not used to store the interpolated
    // light-space depth.
    highp float z = (getViewFromWorldMatrix() * getWorldPosition(material)).z;

    // rescale [near, far] to [0, 1]
    highp float depth = -z * frameUniforms.oneOverFarMinusNear - frameUniforms.nearOverFarMinusNear;

    // EVSM pre-mapping
    depth = frameUniforms.vsmExponent * (depth * 2.0 - 1.0);

    vertex_worldPosition.w = depth;
#endif

    // this must happen before we compensate for vulkan below
    vertex_position = gl_Position;

#if defined(TARGET_VULKAN_ENVIRONMENT)
    // In Vulkan, clip space is Y-down. In OpenGL and Metal, clip space is Y-up.
    gl_Position.y = -gl_Position.y;
#endif

#if !defined(TARGET_VULKAN_ENVIRONMENT) && !defined(TARGET_METAL_ENVIRONMENT)
    // This is not needed in Vulkan or Metal because clipControl is always (1, 0)
    gl_Position.z = dot(gl_Position.zw, frameUniforms.clipControl);
#endif
}
