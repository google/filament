/** @public-api */
vec4 getPosition() {
    vec4 pos = position;

// In Vulkan, clip space is Y-down. In OpenGL and Metal, clip space is Y-up.
#if defined(TARGET_VULKAN_ENVIRONMENT)
    pos.y = -pos.y;
#endif

    return pos;
}
