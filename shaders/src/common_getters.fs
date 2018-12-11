//------------------------------------------------------------------------------
// Uniforms access
//------------------------------------------------------------------------------

/** @public-api */
mat4 getViewFromWorldMatrix() {
    return frameUniforms.viewFromWorldMatrix;
}

/** @public-api */
mat4 getWorldFromViewMatrix() {
    return frameUniforms.worldFromViewMatrix;
}

/** @public-api */
mat4 getClipFromViewMatrix() {
    return frameUniforms.clipFromViewMatrix;
}

/** @public-api */
mat4 getViewFromClipMatrix() {
    return frameUniforms.viewFromClipMatrix;
}

/** @public-api */
mat4 getClipFromWorldMatrix() {
    return frameUniforms.clipFromWorldMatrix;
}

/** @public-api */
vec4 getResolution() {
    return frameUniforms.resolution;
}

/** @public-api */
vec3 getWorldCameraPosition() {
    return frameUniforms.cameraPosition;
}

/** @public-api */
float getTime() {
    return frameUniforms.time;
}

/** @public-api */
HIGHP vec4 getUserTime() {
    return frameUniforms.userTime;
}

/** @public-api **/
HIGHP float getUserTimeMod(float m) {
    return mod(mod(frameUniforms.userTime.x, m) + mod(frameUniforms.userTime.y, m), m);
}

/** @public-api */
float getExposure() {
    return frameUniforms.exposure;
}

/** @public-api */
float getEV100() {
    return frameUniforms.ev100;
}
