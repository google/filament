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
mat4 getWorldFromClipMatrix() {
    return frameUniforms.worldFromClipMatrix;
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
highp vec3 getWorldOffset() {
    return frameUniforms.worldOffset;
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

/** @public-api */
float getExposure() {
    return frameUniforms.exposure;
}

/** @public-api */
float getEV100() {
    return frameUniforms.ev100;
}
