// The sole purpose of this no-op function is to improve parity between the depth vertex shader
// and color vertex shader, thus working around a variance issue seen with NVIDIA drivers.
void materialVertex(inout MaterialVertexInputs m) { }

void main() {
#if defined(VERTEX_DOMAIN_DEVICE)
    gl_Position = getPosition();
#else
    MaterialVertexInputs material;
    initMaterialVertex(material);
    materialVertex(material);
    gl_Position = getClipFromWorldMatrix() * getWorldPosition(material);
#endif

#if defined(TARGET_VULKAN_ENVIRONMENT)
    // In Vulkan, clip space is Y-down. In OpenGL and Metal, clip space is Y-up.
    gl_Position.y = -gl_Position.y;
#endif

#if defined(TARGET_VULKAN_ENVIRONMENT) || defined(TARGET_METAL_ENVIRONMENT)
    // In Vulkan and Metal, clip-space Z is [0,w] rather than [-w,+w].
    gl_Position.z = (gl_Position.z + gl_Position.w) * 0.5;
#endif
}
