void main() {
    PostProcessInputs inputs;

    // Invoke user code
    postProcess(inputs);

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
