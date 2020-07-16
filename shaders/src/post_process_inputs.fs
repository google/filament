LAYOUT_LOCATION(LOCATION_UVS) in highp vec2 vertex_uv;

/** @public-api */
highp vec2 getUV() {
    return vertex_uv;
}

struct PostProcessInputs {
#if defined(FRAG_OUTPUT0)
    FRAG_OUTPUT_TYPE0 FRAG_OUTPUT0;
#endif
#if defined(FRAG_OUTPUT1)
    FRAG_OUTPUT_TYPE1 FRAG_OUTPUT1;
#endif
#if defined(FRAG_OUTPUT2)
    FRAG_OUTPUT_TYPE2 FRAG_OUTPUT2;
#endif
#if defined(FRAG_OUTPUT3)
    FRAG_OUTPUT_TYPE3 FRAG_OUTPUT3;
#endif
#if defined(FRAG_OUTPUT_DEPTH)
    float depth;
#endif
};
