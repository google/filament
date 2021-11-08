// The sole purpose of this no-op function is to improve parity between the depth vertex shader
// and color vertex shader, thus working around a variance issue seen with NVIDIA drivers.
void materialVertex(inout MaterialVertexInputs m) { }

// NOTE: This shader is only used when the user's material does not have custom vertex code.
//       There is no need to check anything related to material inputs in this file.
void main() {

// World position is used to compute gl_Position, except for vertices already in the device domain.
// Regardless of vertex domain, if VSM is turned on, then we need to compute world position to pass
// to the fragment shader.
#if !defined(VERTEX_DOMAIN_DEVICE) || defined(HAS_VSM)
    // Run initMaterialVertex to compute material.worldPosition.
    MaterialVertexInputs material;
    initMaterialVertex(material);
    materialVertex(material);
#endif

#if defined(VERTEX_DOMAIN_DEVICE)
    gl_Position = getPosition();
    // GL convention to inverted DX convention
    gl_Position.z = gl_Position.z * -0.5 + 0.5;
#else
    gl_Position = getClipFromWorldMatrix() * getWorldPosition(material);
#endif

#if defined(HAS_VSM)
    // For VSM, we use the linear light-space Z coordinate as the depth metric, which works for both
    // directional and spot lights and can be safely interpolated.
    // The value is guaranteed to be between [-znear, -zfar] by construction of viewFromWorldMatrix,
    // (see ShadowMap.cpp).
    // Use vertex_worldPosition.w which is otherwise not used to store the interpolated
    // light-space depth.
    highp float z = (frameUniforms.viewFromWorldMatrix * vec4(material.worldPosition.xyz, 1.0)).z;

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
