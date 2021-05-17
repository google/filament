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
#if defined(FRAG_OUTPUT4)
    FRAG_OUTPUT_TYPE4 FRAG_OUTPUT4;
#endif
#if defined(FRAG_OUTPUT5)
    FRAG_OUTPUT_TYPE5 FRAG_OUTPUT5;
#endif
#if defined(FRAG_OUTPUT6)
    FRAG_OUTPUT_TYPE6 FRAG_OUTPUT6;
#endif
#if defined(FRAG_OUTPUT7)
    FRAG_OUTPUT_TYPE7 FRAG_OUTPUT7;
#endif
#if defined(FRAG_OUTPUT_DEPTH)
    float depth;
#endif
};
