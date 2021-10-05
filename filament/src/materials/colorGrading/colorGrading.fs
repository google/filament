vec3 colorGrade(mediump sampler3D lut, const vec3 x) {
    // Alexa LogC EI 1000
    const float a = 5.555556;
    const float b = 0.047996;
    const float c = 0.244161 / log2(10.0);
    const float d = 0.386036;
    vec3 logc = c * log2(a * x + b) + d;

    // Remap to sample pixel centers
    logc = materialParams.lutSize.x + logc * materialParams.lutSize.y;

    return textureLod(lut, logc, 0.0).rgb;
}
