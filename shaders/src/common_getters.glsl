//------------------------------------------------------------------------------
// Uniforms access
//------------------------------------------------------------------------------

/** @public-api */
highp mat4 getViewFromWorldMatrix() {
    return frameUniforms.viewFromWorldMatrix;
}

/** @public-api */
highp mat4 getWorldFromViewMatrix() {
    return frameUniforms.worldFromViewMatrix;
}

/** @public-api */
highp mat4 getClipFromViewMatrix() {
    return frameUniforms.clipFromViewMatrix;
}

/** @public-api */
highp mat4 getViewFromClipMatrix() {
    return frameUniforms.viewFromClipMatrix;
}

/** @public-api */
highp mat4 getClipFromWorldMatrix() {
#if defined(VARIANT_HAS_STEREO) && defined(FILAMENT_STEREO_INSTANCED)
    int eye = instance_index % CONFIG_STEREO_EYE_COUNT;
    return frameUniforms.clipFromWorldMatrix[eye];
#elif defined(VARIANT_HAS_STEREO) && defined(FILAMENT_STEREO_MULTIVIEW)
    return frameUniforms.clipFromWorldMatrix[gl_ViewID_OVR];
#else
    return frameUniforms.clipFromWorldMatrix[0];
#endif
}

/** @public-api */
highp mat4 getWorldFromClipMatrix() {
    return frameUniforms.worldFromClipMatrix;
}

/** @public-api */
highp mat4 getUserWorldFromWorldMatrix() {
    return frameUniforms.userWorldFromWorldMatrix;
}

/** @public-api */
float getTime() {
    return frameUniforms.time;
}

/** @public-api */
highp vec4 getUserTime() {
    return frameUniforms.userTime;
}

/** @public-api **/
highp float getUserTimeMod(float m) {
    return mod(mod(frameUniforms.userTime.x, m) + mod(frameUniforms.userTime.y, m), m);
}

/**
 * Transforms a texture UV to make it suitable for a render target attachment.
 *
 * In Vulkan and Metal, texture coords are Y-down but in OpenGL they are Y-up. This wrapper function
 * accounts for these differences. When sampling from non-render targets (i.e. uploaded textures)
 * these differences do not matter because OpenGL has a second piece of backwardness, which is that
 * the first row of texels in glTexImage2D is interpreted as the bottom row.
 *
 * To protect users from these differences, we recommend that materials in the SURFACE domain
 * leverage this wrapper function when sampling from offscreen render targets.
 *
 * @public-api
 */
highp vec2 uvToRenderTargetUV(const highp vec2 uv) {
#if defined(TARGET_METAL_ENVIRONMENT) || defined(TARGET_VULKAN_ENVIRONMENT)
    return vec2(uv.x, 1.0 - uv.y);
#else
    return uv;
#endif
}

// TODO: below shouldn't be accessible from post-process materials

/** @public-api */
highp vec4 getResolution() {
    return frameUniforms.resolution;
}

/** @public-api */
highp vec3 getWorldCameraPosition() {
    return frameUniforms.worldFromViewMatrix[3].xyz;
}

/** @public-api, @deprecated use getUserWorldPosition() or getUserWorldFromWorldMatrix() instead  */
highp vec3 getWorldOffset() {
    return getUserWorldFromWorldMatrix()[3].xyz;
}

/** @public-api */
float getExposure() {
    // NOTE: this is a highp uniform only to work around #3602 (qualcomm)
    // We are intentionally casting it to mediump here, as per the Materials doc.
    return frameUniforms.exposure;
}

/** @public-api */
float getEV100() {
    return frameUniforms.ev100;
}

//------------------------------------------------------------------------------
// user defined globals
//------------------------------------------------------------------------------

highp vec4 getMaterialGlobal0() {
    return frameUniforms.custom[0];
}

highp vec4 getMaterialGlobal1() {
    return frameUniforms.custom[1];
}

highp vec4 getMaterialGlobal2() {
    return frameUniforms.custom[2];
}

highp vec4 getMaterialGlobal3() {
    return frameUniforms.custom[3];
}
