// The sole purpose of this no-op function is to improve parity between the depth vertex shader
// and color vertex shader, thus working around a variance issue seen with NVIDIA drivers.
void materialVertex(inout MaterialVertexInputs m) { }

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
#else
    gl_Position = getClipFromWorldMatrix() * getWorldPosition(material);
#endif

#ifdef MATERIAL_HAS_CLIP_SPACE_TRANSFORM
    gl_Position = getClipSpaceTransform(material) * gl_Position;
#endif

#if defined(HAS_VSM)
    vertex_worldPosition = material.worldPosition.xyz;
#endif

#if defined(TARGET_VULKAN_ENVIRONMENT)
    // In Vulkan, clip space is Y-down. In OpenGL and Metal, clip space is Y-up.
    gl_Position.y = -gl_Position.y;
#endif

    gl_Position.z = dot(gl_Position.zw, frameUniforms.clipControl.xy);
}
