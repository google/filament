
/*
 * DoF Utils
 */

// Below this value we transition to the in-focus image. This value is chosen as a compromise
// between allowing slight out-of-focus and reducing sampling artifacts around blurry objects
// on sharp background. Based on experiments, it should be between 1.0 and 2.0.
#define MAX_IN_FOCUS_COC    2.0

// The maximum circle-of-confusion radius we allow in high-resolution pixels.
// This is limited by our tile size (hard limit) and dilate pass as well as the kernel density
// (soft/quality limit).
#define MAX_COC_RADIUS      32.0

float random(const highp vec2 w) {
    const vec3 m = vec3(0.06711056, 0.00583715, 52.9829189);
    return fract(m.z * fract(dot(w, m.xy)));
}

float min2(const vec2 v) {
    return min(v.x, v.y);
}

float max2(const vec2 v) {
    return max(v.x, v.y);
}

float max4(const vec4 v) {
    return max2(max(v.xy, v.zw));
}

float min4(const vec4 v) {
    return min2(min(v.xy, v.zw));
}

float cocToAlpha(const float coc) {
    // CoC is positive for background field.
    // CoC is negative for the foreground field.
    return saturate(abs(coc) - MAX_IN_FOCUS_COC);
}

// returns circle-of-confusion diameter in pixels
float getCOC(const float depth, const vec2 cocParams) {
    return depth * cocParams.x + cocParams.y;
}

vec4 getCOC(const vec4 depth, const vec2 cocParams) {
    return depth * cocParams.x + cocParams.y;
}

float isForeground(const float coc) {
    return coc < 0.0 ? 1.0 : 0.0;
}

float isBackground(const float coc) {
    return coc > 0.0 ? 1.0 : 0.0;
}

bool isForegroundTile(const vec2 tiles) {
    // A foreground tile is one where the smallest CoC is negative
    return tiles.g < 0.0;
}

bool isBackgroundTile(const vec2 tiles) {
    // A background tile is one where the largest CoC is positive
    return tiles.r > 0.0;
}

bool isFastTile(const vec2 tiles) {
    // We use the distance between the min and max CoC and if the relative error is less than
    // 5% we assume the tile contains a constant CoC.
    // We could cannot use the absolute value of the min/mac CoC -- which would categorize more
    // tiles as "fast" (e.g. when both the foreground and background have similar CoC), because
    // it doesn't tell us anything about objects that could be in between.
    return (tiles.r - tiles.g) <= abs(tiles.r) * 0.05;
}

bool isTrivialTile(const vec2 tiles) {
    float maxCocRadius = max(abs(tiles.r), abs(tiles.g));
    return maxCocRadius < MAX_IN_FOCUS_COC;
}
