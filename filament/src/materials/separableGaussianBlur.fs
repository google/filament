float vmax(const float v) {
    return v;
}

highp vec4 sourceTexLod(const highp vec2 p, float m, float l) {
    // This condition is optimized away at compile-time.
    if (materialConstants_arraySampler) {
        return textureLod(materialParams_sourceArray, vec3(p, l), m);
    } else {
        return textureLod(materialParams_source, p, m);
    }
}

void tap(inout highp vec4 sum, const float weight, const highp vec2 position) {
    highp vec4 s = sourceTexLod(position, materialParams.level, materialParams.layer);
    sum += s * weight;
}

void tapReinhard(inout highp vec4 sum, inout float totalWeight, const float weight, const highp vec2 position) {
    highp vec4 s = sourceTexLod(position, materialParams.level, materialParams.layer);
    float w = weight / (1.0 + vmax(s));
    totalWeight += w;
    sum += s * w;
}

void postProcess(inout PostProcessInputs postProcess) {
    highp vec2 uv = variable_vertex.xy;

    // we handle the center pixel separately
    highp vec4 sum = vec4(0.0);

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

    // These conditions are evaluated at compile time and help the driver optimize out unnecessary
    // computations.
    postProcess.color = vec4(0.0);
    if (materialConstants_componentCount == 1) {
        postProcess.color.r = sum.r;
    } else if (materialConstants_componentCount == 2) {
        postProcess.color.rg = sum.rg;
    } else if (materialConstants_componentCount == 3) {
        postProcess.color.rgb = sum.rgb;
    } else {
        postProcess.color.rgba = sum.rgba;
   }
}
