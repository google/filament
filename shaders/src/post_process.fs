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
#if defined(FRAG_OUTPUT4)
    inputs.FRAG_OUTPUT4 = clamp(inputs.FRAG_OUTPUT4, -MEDIUMP_FLT_MAX, MEDIUMP_FLT_MAX);
#endif
#if defined(FRAG_OUTPUT5)
    inputs.FRAG_OUTPUT5 = clamp(inputs.FRAG_OUTPUT5, -MEDIUMP_FLT_MAX, MEDIUMP_FLT_MAX);
#endif
#if defined(FRAG_OUTPUT6)
    inputs.FRAG_OUTPUT6 = clamp(inputs.FRAG_OUTPUT6, -MEDIUMP_FLT_MAX, MEDIUMP_FLT_MAX);
#endif
#if defined(FRAG_OUTPUT7)
    inputs.FRAG_OUTPUT7 = clamp(inputs.FRAG_OUTPUT7, -MEDIUMP_FLT_MAX, MEDIUMP_FLT_MAX);
#endif
#endif

#if defined(FRAG_OUTPUT0)
    FRAG_OUTPUT_AT0 FRAG_OUTPUT_SWIZZLE0 = inputs.FRAG_OUTPUT0;
#endif
#if defined(FRAG_OUTPUT1)
    FRAG_OUTPUT_AT1 FRAG_OUTPUT_SWIZZLE1 = inputs.FRAG_OUTPUT1;
#endif
#if defined(FRAG_OUTPUT2)
    FRAG_OUTPUT_AT2 FRAG_OUTPUT_SWIZZLE2 = inputs.FRAG_OUTPUT2;
#endif
#if defined(FRAG_OUTPUT3)
    FRAG_OUTPUT_AT3 FRAG_OUTPUT_SWIZZLE3 = inputs.FRAG_OUTPUT3;
#endif
#if defined(FRAG_OUTPUT4)
    FRAG_OUTPUT_AT4 FRAG_OUTPUT_SWIZZLE4 = inputs.FRAG_OUTPUT4;
#endif
#if defined(FRAG_OUTPUT5)
    FRAG_OUTPUT_AT5 FRAG_OUTPUT_SWIZZLE5 = inputs.FRAG_OUTPUT5;
#endif
#if defined(FRAG_OUTPUT6)
    FRAG_OUTPUT_AT6 FRAG_OUTPUT_SWIZZLE6 = inputs.FRAG_OUTPUT6;
#endif
#if defined(FRAG_OUTPUT7)
    FRAG_OUTPUT_AT7 FRAG_OUTPUT_SWIZZLE7 = inputs.FRAG_OUTPUT7;
#endif
#if defined(FRAG_OUTPUT_DEPTH)
    gl_FragDepth = inputs.depth;
#endif
}
