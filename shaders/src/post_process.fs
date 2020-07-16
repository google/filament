void main() {
    PostProcessInputs inputs;

    // Invoke user code
    postProcess(inputs);

#if defined(TARGET_MOBILE)
#if defined(FRAG_OUTPUT0)
    inputs.FRAG_OUTPUT0 = clamp(inputs.FRAG_OUTPUT0, -MEDIUMP_FLT_MAX, MEDIUMP_FLT_MAX);
#endif
#if defined(FRAG_OUTPUT1)
    inputs.FRAG_OUTPUT1 = clamp(inputs.FRAG_OUTPUT1, -MEDIUMP_FLT_MAX, MEDIUMP_FLT_MAX);
#endif
#if defined(FRAG_OUTPUT2)
    inputs.FRAG_OUTPUT2 = clamp(inputs.FRAG_OUTPUT2, -MEDIUMP_FLT_MAX, MEDIUMP_FLT_MAX);
#endif
#if defined(FRAG_OUTPUT3)
    inputs.FRAG_OUTPUT3 = clamp(inputs.FRAG_OUTPUT3, -MEDIUMP_FLT_MAX, MEDIUMP_FLT_MAX);
#endif
#endif

#if defined(FRAG_OUTPUT0)
    FRAG_OUTPUT_AT0 = inputs.FRAG_OUTPUT0;
#endif
#if defined(FRAG_OUTPUT1)
    FRAG_OUTPUT_AT1 = inputs.FRAG_OUTPUT1;
#endif
#if defined(FRAG_OUTPUT2)
    FRAG_OUTPUT_AT2 = inputs.FRAG_OUTPUT2;
#endif
#if defined(FRAG_OUTPUT3)
    FRAG_OUTPUT_AT3 = inputs.FRAG_OUTPUT3;
#endif
#if defined(FRAG_OUTPUT_DEPTH)
    gl_FragDepth = inputs.depth;
#endif
}
