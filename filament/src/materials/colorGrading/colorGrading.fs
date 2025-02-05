vec3 oetfSRGB(vec3 x) {
    float a  = 0.055f;
    float a1 = 1.055f;
    float b  = 12.92f;
    float p  = 1.0f / 2.4f;
    for (int i = 0; i < 3; i++) {
        x[i] = x[i] <= 0.0031308f ? x[i] * b : a1 * pow(x[i], p) - a;
    }
    return x;
}

vec3 colorGrade(mediump sampler3D lut, const vec3 x) {
    // Alexa LogC EI 1000
    // const float a = 5.555556;
    // const float b = 0.047996;
    // const float c = 0.244161 / log2(10.0);
    // const float d = 0.386036;
    // vec3 logc = c * log2(a * x + b) + d;

    // Remap to sample pixel centers
    // logc = materialParams.lutSize.x + logc * materialParams.lutSize.y;

    // return textureLod(lut, logc, 0.0).rgb;

    // Testing: apply the sRGB OETF in the shader.
    return oetfSRGB(x);
}
