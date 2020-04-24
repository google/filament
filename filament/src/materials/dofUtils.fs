
/*
 * DoF blur
 *
 * This uses a lot of ideas from
 *   "Bokeh depth of field in a single pass" by Dennis Gustafsson
 *   (http://blog.tuxedolabs.com/2018/05/04/bokeh-depth-of-field-in-single-pass.html)
 *
 * needs:
 *   materialParams_depth : depth texture
 *   materialParams.cocParams : coc scale and bias
 *   materialParams.resolution : screen resolution
 */

// Filter sample count, prefer odd values
// (keep in sync with PostProcessManager.cpp:dof())
const float SAMPLE_COUNT = 25.0;

// A value > 1.0 allows a larger blur w/ dithering
const float BLUR_SCALE = 1.0;

// This is here just for aesthetic reasons
const float BOKEH_ROTATION_ANGLE = PI / 6.0;

#define unitvec(angle) vec2(cos(angle), sin(angle))

// random number between 0 and 1, using interleaved gradient noise
float random(const highp vec2 w) {
    const vec3 m = vec3(0.06711056, 0.00583715, 52.9829189);
    return fract(m.z * fract(dot(w, m.xy)));
}

float getCOC(float depth, vec2 cocParams) {
    float CoC = abs(depth * cocParams.x + cocParams.y);
    return saturate(CoC);
}

void tap(inout vec4 finalColor, inout float blurAmount, float radius,
        highp vec2 uv, sampler2D colorBuffer, float centerDepth, float centerCoc) {
    float depth = textureLod(materialParams_depth,  uv, 0.0).r;
    float coc = getCOC(depth, materialParams.cocParams);
    vec4 color = textureLod(colorBuffer, uv, 0.0);

    // prevent blurry background to bleed onto sharp foreground
    if (depth > centerDepth) {
        coc = clamp(coc, 0.0, centerCoc * 2.0);
    }

    float m = step(radius * BLUR_SCALE, coc * (SAMPLE_COUNT * BLUR_SCALE));
    finalColor += mix(finalColor * (1.0 / blurAmount), color, m);
    blurAmount += 1.0;
}

vec4 blurTexture(sampler2D colorBuffer, highp vec2 uv, highp vec2 direction, float centerDepth, float centerCoc) {
    float blurAmount = 1.0;
    vec4 finalColor = textureLod(colorBuffer, uv, 0.0);

    highp vec2 unit = materialParams.resolution.zw;
    direction *= unit * BLUR_SCALE;

    float noise = 0.0;
    if (BLUR_SCALE != 1.0) {
        // we span 2 samples because the first sample is always fixed (no noise added)
        noise = (random(gl_FragCoord.xy) - 0.5) * 2.0;
        uv += direction * noise;
    }

    vec4 tc = uv.xyxy;
    for (float radius = 1.0 ; radius < (SAMPLE_COUNT * 0.5); radius += 1.0) {
        tc += vec4(direction, -direction);
        tap(finalColor, blurAmount, radius + noise, tc.xy, colorBuffer, centerDepth, centerCoc);
        tap(finalColor, blurAmount, radius - noise, tc.zw, colorBuffer, centerDepth, centerCoc);
    }
    return finalColor * (1.0 / blurAmount);
}
