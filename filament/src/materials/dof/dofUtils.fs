
/*
 * DoF Utils
 */

// Below this value we transition to the in-focus image. This value is chosen as a compromise
// between allowing slight out-of-focus and reducing sampling artifacts around blurry objects
// on sharp background. Based on experiments, it should be between 1.0 and 2.0.
// This value is in high-resolution pixels.
#define MAX_IN_FOCUS_COC    1.0

// The maximum circle-of-confusion radius we allow in high-resolution pixels.
// This is mostly limited by the kernel density and how many mips we allow -- when the CoC becomes
// too large, the rings start to appear; but worse, the median pass will start erasing pixels
// if the gaps between rings becomes too large. This is also limited by the dilate pass.
// With a minimum ring count of 3 and 4 mip levels, 48 seems to be the maximum allowable.
// Currently our dilate pass is set-up for 32 max.
#define MAX_COC_RADIUS      32.0

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

float rcp(const float x) {
    return 1.0 / x;
}

float rcpOrZero(const float x) {
    return x > MEDIUMP_FLT_MIN ? (1.0 / x) : 0.0;
}

highp float rcpOrZeroHighp(const highp float x) {
    return x > MEDIUMP_FLT_MIN ? (1.0 / x) : 0.0;
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
    return tiles.r < 0.0;
}

bool isBackgroundTile(const vec2 tiles) {
    // A background tile is one where the largest CoC is positive
    return tiles.g > 0.0;
}

bool isFastTile(const vec2 tiles) {
    // We use the distance between the min and max CoC and if the relative error is less than
    // 5% we assume the tile contains a constant CoC.
    // We could cannot use the absolute value of the min/mac CoC -- which would categorize more
    // tiles as "fast" (e.g. when both the foreground and background have similar CoC), because
    // it doesn't tell us anything about objects that could be in between.
    return (tiles.g - tiles.r) <= abs(tiles.g) * 0.05;
}

bool isTrivialTile(const vec2 tiles) {
    float maxCocRadius = max(abs(tiles.r), abs(tiles.g));
    return maxCocRadius < MAX_IN_FOCUS_COC;
}

float downsampleCoC(const vec4 c) {
    // We need to compute a suitable CoC to represent the 4 pixels that are downsampled.
    // We pick the min because this always selects the most foreground sample if there is one,
    // because the foreground can leak on the background, but not the reverse.
    return min4(c);
}

vec4 downsampleCocWeights(const vec4 c, const float outCoc, const float scale) {
    // The bilateral weight is normally computed as saturate(1 - |outCoc - c|) which selects
    // the sample with the outCoc weight (and does a little bit of cross-fade if other samples
    // are close). However, this can also cause some aliasing with dithered objects, so by
    // omitting the abs() we allow samples in the background to "leak" into the foreground, which
    // is not completely different from the "mirror hole filling" of the DoF pass.
    // See "Life of a Bokeh" by Guillaume Abadie, SIGGRAPH 2018.

    // Note: With the implementation of downsampleCoC() above, this always resolves to 1.0;
    // return saturate(1.0 - (outCoc - c) * scale);
    return vec4(1.0);
}
