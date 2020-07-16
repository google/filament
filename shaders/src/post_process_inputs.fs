LAYOUT_LOCATION(LOCATION_UVS) in highp vec2 vertex_uv;

/** @public-api */
highp vec2 getUV() {
    return vertex_uv;
}

struct PostProcessInputs {
#if defined(OUTPUT0)
    OUTPUT_TYPE0 OUTPUT0;
#endif
#if defined(OUTPUT1)
    OUTPUT_TYPE1 OUTPUT1;
#endif
#if defined(OUTPUT2)
    OUTPUT_TYPE2 OUTPUT2;
#endif
#if defined(OUTPUT3)
    OUTPUT_TYPE3 OUTPUT3;
#endif
#if defined(OUTPUT_DEPTH)
    float depth;
#endif
};
