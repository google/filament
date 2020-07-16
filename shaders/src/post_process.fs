void main() {
    PostProcessInputs inputs;

    // Invoke user code
    postProcess(inputs);

#if defined(TARGET_MOBILE)
    inputs.color = clamp(inputs.color, -MEDIUMP_FLT_MAX, MEDIUMP_FLT_MAX);
#endif

#if defined(OUTPUT0)
    OUTPUT_AT0 = inputs.OUTPUT0;
#endif
#if defined(OUTPUT1)
    OUTPUT_AT1 = inputs.OUTPUT1;
#endif
#if defined(OUTPUT2)
    OUTPUT_AT2 = inputs.OUTPUT2;
#endif
#if defined(OUTPUT3)
    OUTPUT_AT3 = inputs.OUTPUT3;
#endif
#if defined(OUTPUT_DEPTH)
    gl_FragDepth = inputs.depth;
#endif
}
