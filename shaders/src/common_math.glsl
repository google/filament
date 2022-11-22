//------------------------------------------------------------------------------
// Common math
//------------------------------------------------------------------------------

/** @public-api */
#define PI                 3.14159265359
/** @public-api */
#define HALF_PI            1.570796327

#define MEDIUMP_FLT_MAX    65504.0
#define MEDIUMP_FLT_MIN    0.00006103515625

#ifdef TARGET_MOBILE
#define FLT_EPS            MEDIUMP_FLT_MIN
#define saturateMediump(x) min(x, MEDIUMP_FLT_MAX)
#else
#define FLT_EPS            1e-5
#define saturateMediump(x) x
#endif

#define saturate(x)        clamp(x, 0.0, 1.0)
#define atan2(x, y)        atan(y, x)

//------------------------------------------------------------------------------
// Scalar operations
//------------------------------------------------------------------------------

/**
 * Computes x^5 using only multiply operations.
 *
 * @public-api
 */
float pow5(float x) {
    float x2 = x * x;
    return x2 * x2 * x;
}

/**
 * Computes x^2 as a single multiplication.
 *
 * @public-api
 */
float sq(float x) {
    return x * x;
}

//------------------------------------------------------------------------------
// Vector operations
//------------------------------------------------------------------------------

/**
 * Returns the maximum component of the specified vector.
 *
 * @public-api
 */
float max3(const vec3 v) {
    return max(v.x, max(v.y, v.z));
}

float vmax(const vec2 v) {
    return max(v.x, v.y);
}

float vmax(const vec3 v) {
    return max(v.x, max(v.y, v.z));
}

float vmax(const vec4 v) {
    return max(max(v.x, v.y), max(v.y, v.z));
}

/**
 * Returns the minimum component of the specified vector.
 *
 * @public-api
 */
float min3(const vec3 v) {
    return min(v.x, min(v.y, v.z));
}

float vmin(const vec2 v) {
    return min(v.x, v.y);
}

float vmin(const vec3 v) {
    return min(v.x, min(v.y, v.z));
}

float vmin(const vec4 v) {
    return min(min(v.x, v.y), min(v.y, v.z));
}

//------------------------------------------------------------------------------
// Trigonometry
//------------------------------------------------------------------------------

/**
 * Approximates acos(x) with a max absolute error of 9.0x10^-3.
 * Valid in the range -1..1.
 */
float acosFast(float x) {
    // Lagarde 2014, "Inverse trigonometric functions GPU optimization for AMD GCN architecture"
    // This is the approximation of degree 1, with a max absolute error of 9.0x10^-3
    float y = abs(x);
    float p = -0.1565827 * y + 1.570796;
    p *= sqrt(1.0 - y);
    return x >= 0.0 ? p : PI - p;
}

/**
 * Approximates acos(x) with a max absolute error of 9.0x10^-3.
 * Valid only in the range 0..1.
 */
float acosFastPositive(float x) {
    float p = -0.1565827 * x + 1.570796;
    return p * sqrt(1.0 - x);
}

//------------------------------------------------------------------------------
// Matrix and quaternion operations
//------------------------------------------------------------------------------

/**
 * Multiplies the specified 3-component vector by the 4x4 matrix (m * v) in
 * high precision.
 *
 * @public-api
 */
highp vec4 mulMat4x4Float3(const highp mat4 m, const highp vec3 v) {
    return v.x * m[0] + (v.y * m[1] + (v.z * m[2] + m[3]));
}

/**
 * Multiplies the specified 3-component vector by the 3x3 matrix (m * v) in
 * high precision.
 *
 * @public-api
 */
highp vec3 mulMat3x3Float3(const highp mat4 m, const highp vec3 v) {
    return v.x * m[0].xyz + (v.y * m[1].xyz + (v.z * m[2].xyz));
}

/**
 * Extracts the normal vector of the tangent frame encoded in the specified quaternion.
 */
void toTangentFrame(const highp vec4 q, out highp vec3 n) {
    n = vec3( 0.0,  0.0,  1.0) +
        vec3( 2.0, -2.0, -2.0) * q.x * q.zwx +
        vec3( 2.0,  2.0, -2.0) * q.y * q.wzy;
}

/**
 * Extracts the normal and tangent vectors of the tangent frame encoded in the
 * specified quaternion.
 */
void toTangentFrame(const highp vec4 q, out highp vec3 n, out highp vec3 t) {
    toTangentFrame(q, n);
    t = vec3( 1.0,  0.0,  0.0) +
        vec3(-2.0,  2.0, -2.0) * q.y * q.yxw +
        vec3(-2.0,  2.0,  2.0) * q.z * q.zwx;
}

highp mat3 cofactor(const highp mat3 m) {
    highp float a = m[0][0];
    highp float b = m[1][0];
    highp float c = m[2][0];
    highp float d = m[0][1];
    highp float e = m[1][1];
    highp float f = m[2][1];
    highp float g = m[0][2];
    highp float h = m[1][2];
    highp float i = m[2][2];

    highp mat3 cof;
    cof[0][0] = e * i - f * h;
    cof[0][1] = c * h - b * i;
    cof[0][2] = b * f - c * e;
    cof[1][0] = f * g - d * i;
    cof[1][1] = a * i - c * g;
    cof[1][2] = c * d - a * f;
    cof[2][0] = d * h - e * g;
    cof[2][1] = b * g - a * h;
    cof[2][2] = a * e - b * d;
    return cof;
}

//------------------------------------------------------------------------------
// Random
//------------------------------------------------------------------------------

/*
 * Random number between 0 and 1, using interleaved gradient noise.
 * w must not be normalized (e.g. window coordinates)
 */
float interleavedGradientNoise(highp vec2 w) {
    const vec3 m = vec3(0.06711056, 0.00583715, 52.9829189);
    return fract(m.z * fract(dot(w, m.xy)));
}
