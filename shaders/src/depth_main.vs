void main() {
#if defined(VERTEX_DOMAIN_DEVICE)
    gl_Position = getSkinnedPosition();
#else
    MaterialVertexInputs material;
    initMaterialVertex(material);
    gl_Position = getClipFromWorldMatrix() * material.worldPosition;
#endif

#if defined(TARGET_VULKAN_ENVIRONMENT)
    // In Vulkan, clip-space Z is [0,w] rather than [-w,+w] and Y is flipped.
    gl_Position.y = -gl_Position.y;
    gl_Position.z = (gl_Position.z + gl_Position.w) * 0.5;
#endif
}
