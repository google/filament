LAYOUT_LOCATION(0) in highp vec2 vertex_uv;

LAYOUT_LOCATION(0) out vec4 fragColor;

/** @public-api */
vec2 getUV() {
    return vertex_uv;
}

struct PostProcessInputs {
    vec4 color;
};
