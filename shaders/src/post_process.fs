void main() {
    PostProcessInputs inputs;
    inputs.color = vec4(1.0);

    // Invoke user code
    postProcess(inputs);

#if defined(TARGET_MOBILE)
    inputs.color = clamp(inputs.color, -MEDIUMP_FLT_MAX, MEDIUMP_FLT_MAX);
#endif

    fragColor = inputs.color;
}
