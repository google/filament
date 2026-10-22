void postProcessVertex(inout PostProcessVertexInputs postProcess) {
    postProcess.vertex.xy = uvToRenderTargetUV(postProcess.normalizedUV);
}
