LAYOUT_LOCATION(LOCATION_POSITION) in vec4 position;

LAYOUT_LOCATION(0) out vec2 vertex_uv;

void main() {
    vertex_uv = (position.xy * 0.5 + 0.5) * frameUniforms.resolution.xy;

    gl_Position = position;

#if defined(TARGET_METAL_ENVIRONMENT)
    // Metal texture space is vertically flipped that of OpenGL's, so flip the Y coords so we sample
    // the frame correctly. Vulkan doesn't need this fix because its clip space is mirrored
    // (the Y axis points down the screen).
    gl_Position.y = -gl_Position.y;
#endif
}
