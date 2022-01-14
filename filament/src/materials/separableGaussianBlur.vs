void postProcessVertex(inout PostProcessVertexInputs postProcess) {
    // in the fragment shader, this is interpolated to pixel centers, but since we use
    // texel-fetch, it's not what we want. Convert from screen uv to texture uv.
    vec2 size = vec2(textureSize(materialParams_source, int(materialParams.level)));
    postProcess.vertex.xy = (postProcess.normalizedUV - 0.5 * materialParams.resolution.zw) + 0.5 / size;
}
