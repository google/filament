// BLUR_TYPE and BLUR_SWIZZLE must be defined
// BLUR_TYPE        vec2, vec3, vec4
// BLUR_SWIZZLE     r, rg, rgb, rgba

float vmax(const float v) {
    return v;
}

void tap(inout highp BLUR_TYPE sum, const float weight, const highp vec2 position) {
    vec4 s = textureLod(materialParams_source, position, materialParams.level);
    sum.BLUR_SWIZZLE += s.BLUR_SWIZZLE * weight;
}

void tapReinhard(inout highp BLUR_TYPE sum, inout float totalWeight, const float weight, const highp vec2 position) {
    vec4 s = textureLod(materialParams_source, position, materialParams.level);
    float w = weight / (1.0 + vmax(s.BLUR_SWIZZLE));
    totalWeight += w;
    sum.BLUR_SWIZZLE += s.BLUR_SWIZZLE * w;
}

void postProcess(inout PostProcessInputs postProcess) {
    highp vec2 uv = variable_vertex.xy;

    // we handle the center pixel separately
    highp BLUR_TYPE sum = BLUR_TYPE(0.0);

    if (materialParams.reinhard != 0) {
        float totalWeight = 0.0;
        tapReinhard(sum, totalWeight, materialParams.kernel[0].x, uv);
        vec2 offset = materialParams.axis;
        for (int i = 1; i < materialParams.count; i++, offset += materialParams.axis * 2.0) {
            float k = materialParams.kernel[i].x;
            vec2 o = offset + materialParams.axis * materialParams.kernel[i].y;
            tapReinhard(sum, totalWeight, k, uv + o);
            tapReinhard(sum, totalWeight, k, uv - o);
        }
        sum *= 1.0 / totalWeight;
    } else {
        tap(sum, materialParams.kernel[0].x, uv);
        vec2 offset = materialParams.axis;
        for (int i = 1; i < materialParams.count; i++, offset += materialParams.axis * 2.0) {
            float k = materialParams.kernel[i].x;
            vec2 o = offset + materialParams.axis * materialParams.kernel[i].y;
            tap(sum, k, uv + o);
            tap(sum, k, uv - o);
        }
    }
    postProcess.color.BLUR_SWIZZLE = sum.BLUR_SWIZZLE;
}
