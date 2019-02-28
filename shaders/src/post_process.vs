
LAYOUT_LOCATION(LOCATION_POSITION) in vec4 position;

LAYOUT_LOCATION(0) out vec2 vertex_uv;

void main() {
    vertex_uv = (position.xy * 0.5 + 0.5) * frameUniforms.resolution.xy;

#if defined(TARGET_VULKAN_ENVIRONMENT) || defined(TARGET_METAL_ENVIRONMENT)
    // In Vulkan and Metal, drawing the top row of pixels occurs when position.y = -1.0 (1.0 for
    // Metal), but we're sampling from a rectangle the sits in the lower-left corner of the texture.
    // Therefore we need to apply an offset to get the correct texture coordinate.
    //
    // For example, if the sampled area height is 180 and the texture height is 200,
    // we need to sample the texture in the range [20,200) rather than [0,180).
    vertex_uv.y += postProcessUniforms.yOffset;
#endif

#if POST_PROCESS_ANTI_ALIASING
    // texel to uv, accounting for the texture actual size
    vertex_uv *= frameUniforms.resolution.zw * postProcessUniforms.uvScale;
#endif

    gl_Position = position;

#if defined(TARGET_METAL_ENVIRONMENT)
    // Metal texture space is vertically flipped that of OpenGL's, so flip the Y coords so we sample
    // the frame correctly. Vulkan doesn't need this fix because its clip space is mirrored
    // (the Y axis points down the screen).
    gl_Position.y = -gl_Position.y;
#endif
}
