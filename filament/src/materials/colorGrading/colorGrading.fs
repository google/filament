vec3 LogC_to_linear(const vec3 x) {
    // Alexa LogC EI 1000
    const float a = 5.555556;
    const float b = 0.047996;
    const float c = 0.244161 / log2(10.0);
    const float d = 0.386036;
    return c * log2(a * x + b) + d;
}

vec3 colorGrade3D(const vec3 v) {
    return textureLod(materialParams_lut, v, 0.0).rgb;
}

vec3 colorGrade1D(const vec3 v) {
    return vec3(
        textureLod(materialParams_lut1d, vec2(v.r, 0.5), 0.0).r,
        textureLod(materialParams_lut1d, vec2(v.g, 0.5), 0.0).r,
        textureLod(materialParams_lut1d, vec2(v.b, 0.5), 0.0).r);
}

vec3 colorGrade(vec3 v) {
    if (!materialConstants_isLDR) {
        v = LogC_to_linear(v);
    }
    // Remap to sample pixel centers.
    v = materialParams.lutSize.x + v * materialParams.lutSize.y;
    return materialConstants_isOneDimensional ? colorGrade1D(v) : colorGrade3D(v);
}
