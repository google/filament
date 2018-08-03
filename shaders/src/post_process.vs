
LAYOUT_LOCATION(LOCATION_POSITION) in vec4 position;

LAYOUT_LOCATION(0) out vec2 vertex_uv;

void main() {
    vertex_uv = (position.xy * 0.5 + 0.5) * frameUniforms.resolution.xy;

#if defined(TARGET_VULKAN_ENVIRONMENT)
    // In Vulkan, drawing the top row of pixels occurs when position.y = -1.0, but we're
    // sampling from a rectangle the sits in the lower-left corner of the texture. Therefore
    // we need to apply an offset to get the correct texture coordinate.
    //
    // For example, if the sampled area height is 180 and the texture height is 200,
    // we need to sample the texture in the range [20,200) rather than [0,180).
    vertex_uv.y += postProcessUniforms.yOffset;
#endif

#if POST_PROCESS_ANTI_ALIASING
    // Account for the texture actual size
    vertex_uv *= postProcessUniforms.uvScale;
    // Compute texel center
    vertex_uv = (floor(vertex_uv) + vec2(0.5, 0.5)) * frameUniforms.resolution.zw;
#endif
    gl_Position = position;
}
