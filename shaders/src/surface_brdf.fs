//------------------------------------------------------------------------------
// BRDF configuration
//------------------------------------------------------------------------------

// Diffuse BRDFs
#define DIFFUSE_LAMBERT             0
#define DIFFUSE_BURLEY              1

// Specular BRDF
// Normal distribution functions
#define SPECULAR_D_GGX              0

// Anisotropic NDFs
#define SPECULAR_D_GGX_ANISOTROPIC  0

// Cloth NDFs
#define SPECULAR_D_CHARLIE          0

// Visibility functions
#define SPECULAR_V_SMITH_GGX        0
#define SPECULAR_V_SMITH_GGX_FAST   1
#define SPECULAR_V_GGX_ANISOTROPIC  2
#define SPECULAR_V_KELEMEN          3
#define SPECULAR_V_NEUBELT          4

// Fresnel functions
#define SPECULAR_F_SCHLICK          0

#define BRDF_DIFFUSE                DIFFUSE_LAMBERT

#if FILAMENT_QUALITY < FILAMENT_QUALITY_HIGH
#define BRDF_SPECULAR_D             SPECULAR_D_GGX
#define BRDF_SPECULAR_V             SPECULAR_V_SMITH_GGX_FAST
#define BRDF_SPECULAR_F             SPECULAR_F_SCHLICK
#else
#define BRDF_SPECULAR_D             SPECULAR_D_GGX
#define BRDF_SPECULAR_V             SPECULAR_V_SMITH_GGX
#define BRDF_SPECULAR_F             SPECULAR_F_SCHLICK
#endif

#define BRDF_CLEAR_COAT_D           SPECULAR_D_GGX
#define BRDF_CLEAR_COAT_V           SPECULAR_V_KELEMEN

#define BRDF_ANISOTROPIC_D          SPECULAR_D_GGX_ANISOTROPIC
#define BRDF_ANISOTROPIC_V          SPECULAR_V_GGX_ANISOTROPIC

#define BRDF_CLOTH_D                SPECULAR_D_CHARLIE
#define BRDF_CLOTH_V                SPECULAR_V_NEUBELT

//------------------------------------------------------------------------------
// Specular BRDF implementations
//------------------------------------------------------------------------------

float D_GGX(float roughness, float NoH, const vec3 h) {
    // Walter et al. 2007, "Microfacet Models for Refraction through Rough Surfaces"

    // In mediump, there are two problems computing 1.0 - NoH^2
    // 1) 1.0 - NoH^2 suffers floating point cancellation when NoH^2 is close to 1 (highlights)
    // 2) NoH doesn't have enough precision around 1.0
    // Both problem can be fixed by computing 1-NoH^2 in highp and providing NoH in highp as well

    // However, we can do better using Lagrange's identity:
    //      ||a x b||^2 = ||a||^2 ||b||^2 - (a . b)^2
    // since N and H are unit vectors: ||N x H||^2 = 1.0 - NoH^2
    // This computes 1.0 - NoH^2 directly (which is close to zero in the highlights and has
    // enough precision).
    // Overall this yields better performance, keeping all computations in mediump
#if defined(TARGET_MOBILE)
    vec3 NxH = cross(shading_normal, h);
    float oneMinusNoHSquared = dot(NxH, NxH);
#else
    float oneMinusNoHSquared = 1.0 - NoH * NoH;
#endif

    float a = NoH * roughness;
    float k = min(roughness / (oneMinusNoHSquared + a * a), 453.5); // 453.5 prevents fp16 overflow
    float d = k * (k * (1.0 / PI));
    return d;
}

float D_GGX_Anisotropic(float at, float ab, float ToH, float BoH, float NoH) {
    // Burley 2012, "Physically-Based Shading at Disney"

    // The values at and ab are perceptualRoughness^2, a2 is therefore perceptualRoughness^4
    // The dot product below computes perceptualRoughness^8. We cannot fit in fp16 without clamping
    // the roughness to too high values so we perform the dot product and the division in fp32
    float a2 = at * ab;
    highp vec3 d = vec3(ab * ToH, at * BoH, a2 * NoH);
    highp float d2 = dot(d, d);
    float b2 = a2 / d2;
    return a2 * b2 * b2 * (1.0 / PI);
}

float D_Charlie(float roughness, float NoH) {
    // Estevez and Kulla 2017, "Production Friendly Microfacet Sheen BRDF"
    float invAlpha  = 1.0 / roughness;
    float cos2h = NoH * NoH;
    float sin2h = max(1.0 - cos2h, 0.0078125); // 2^(-14/2), so sin2h^2 > 0 in fp16
    return (2.0 + invAlpha) * pow(sin2h, invAlpha * 0.5) / (2.0 * PI);
}

float V_SmithGGXCorrelated(float roughness, float NoV, float NoL) {
    // Heitz 2014, "Understanding the Masking-Shadowing Function in Microfacet-Based BRDFs"
    float a2 = roughness * roughness;
    // TODO: lambdaV can be pre-computed for all the lights, it should be moved out of this function
    float lambdaV = NoL * sqrt((NoV - a2 * NoV) * NoV + a2);
    float lambdaL = NoV * sqrt((NoL - a2 * NoL) * NoL + a2);
    // 0.0000077 = nextafter(0.5 / MEDIUMP_FLT_MAX, 1.0) in fp16, so we don't overflow
    float v = PREVENT_DIV0(0.5, lambdaV + lambdaL, 0.0000077);
    // a2=0 => v = 1 / 4*NoL*NoV   => min=1/4, max=+inf
    // a2=1 => v = 1 / 2*(NoL+NoV) => min=1/4, max=+inf
    return v;
}

float V_SmithGGXCorrelated_Fast(float roughness, float NoV, float NoL) {
    // Hammon 2017, "PBR Diffuse Lighting for GGX+Smith Microsurfaces"
    // 0.0000077 = nextafter(0.5 / MEDIUMP_FLT_MAX, 1.0) in fp16, so we don't overflow
    float v = PREVENT_DIV0(0.5, mix(2.0 * NoL * NoV, NoL + NoV, roughness), 0.0000077);
    return v;
}

float V_SmithGGXCorrelated_Anisotropic(float at, float ab, float ToV, float BoV,
        float ToL, float BoL, float NoV, float NoL) {
    // Heitz 2014, "Understanding the Masking-Shadowing Function in Microfacet-Based BRDFs"
    // TODO: lambdaV can be pre-computed for all the lights, it should be moved out of this function
    float lambdaV = NoL * length(vec3(at * ToV, ab * BoV, NoV));
    float lambdaL = NoV * length(vec3(at * ToL, ab * BoL, NoL));
    // 0.0000077 = nextafter(0.5 / MEDIUMP_FLT_MAX, 1.0) in fp16, so we don't overflow
    float v = PREVENT_DIV0(0.5, lambdaV + lambdaL, 0.0000077);
    return v;
}

float V_Kelemen(float LoH) {
    // Kelemen 2001, "A Microfacet Based Coupled Specular-Matte BRDF Model with Importance Sampling"
    // 0.0000039 = nextafter(0.25 / MEDIUMP_FLT_MAX, 1.0) in fp16, so we don't overflow
    return PREVENT_DIV0(0.25, LoH * LoH, 0.0000039);
}

float V_Neubelt(float NoV, float NoL) {
    // Neubelt and Pettineo 2013, "Crafting a Next-gen Material Pipeline for The Order: 1886"
    // 0.00001532 = nextafter(1.0 / MEDIUMP_FLT_MAX, 1.0) in fp16, so we don't overflow
    return PREVENT_DIV0(1.0, 4.0 * (NoL + NoV - NoL * NoV), 0.00001532);
}

vec3 F_Schlick(const vec3 f0, float f90, float VoH) {
    // Schlick 1994, "An Inexpensive BRDF Model for Physically-Based Rendering"
    return f0 + (f90 - f0) * pow5(1.0 - VoH);
}

vec3 F_Schlick(const vec3 f0, float VoH) {
    float f = pow(1.0 - VoH, 5.0);
    return f + f0 * (1.0 - f);
}

float F_Schlick(float f0, float f90, float VoH) {
    return f0 + (f90 - f0) * pow5(1.0 - VoH);
}

//------------------------------------------------------------------------------
// Iridescence
//------------------------------------------------------------------------------

#if defined(MATERIAL_HAS_IRIDESCENCE)

// Belcour and Barla 2017, "A Practical Extension to Microfacet Theory for the Modeling of Varying
// Iridescence", in the approximate form KHR_materials_iridescence specifies: Schlick replaces the
// polarized Fresnel equations, and the spectral integral is evaluated in Fourier space against
// Gaussians fitted to the XYZ color matching functions.
//
// The interference math is highp: the Gaussian fits are stated in inverse meters and their
// amplitudes are around 1e-13, neither of which mediump can represent.

// Schlick with a reflectance of one at grazing incidence, which is what an interface between two
// dielectrics has; fresnel() below substitutes a shadowing term for that.
float F_SchlickIridescence(float f0, float cosTheta) {
    return f0 + (1.0 - f0) * pow5(saturate(1.0 - cosTheta));
}

vec3 F_SchlickIridescence(const vec3 f0, float cosTheta) {
    return f0 + (1.0 - f0) * pow5(saturate(1.0 - cosTheta));
}

// Spectral counterparts of iorToF0() and f0ToIor(). The latter is exact for a dielectric and an
// approximation for a metal, whose complex index it cannot recover.
vec3 iorToF0(const vec3 transmittedIor, float incidentIor) {
    vec3 t = (transmittedIor - incidentIor) / (transmittedIor + incidentIor);
    return t * t;
}

vec3 f0ToIor(const vec3 f0) {
    vec3 r = sqrt(f0);
    return (1.0 + r) / (1.0 - r);
}

// The XYZ sensitivity curves in Fourier space at one optical path difference and phase: four
// Gaussians, one per curve plus the second lobe of x.
highp vec3 evaluateIridescenceSensitivity(highp float opd, const highp vec3 shift) {
    // the path difference arrives in nanometers and the fits are in inverse meters
    highp float phase = 2.0 * PI * opd * 1.0e-9;

    highp vec3 value    = vec3(5.4856e-13, 4.4201e-13, 5.2481e-13);
    highp vec3 position = vec3(1.6810e+06, 1.7953e+06, 2.2084e+06);
    highp vec3 variance = vec3(4.3278e+09, 9.3046e+09, 6.6121e+09);

    highp vec3 xyz = value * sqrt(2.0 * PI * variance) * cos(position * phase + shift) *
            exp(-variance * phase * phase);
    xyz.x += 9.7470e-14 * sqrt(2.0 * PI * 4.5282e+09) *
            cos(2.2399e+06 * phase + shift.x) * exp(-4.5282e+09 * phase * phase);
    xyz /= 1.0685e-7;

    highp mat3 XYZ_TO_REC709 = mat3(
         3.2404542, -0.9692660,  0.0556434,
        -1.5371385,  1.8760108, -0.2040259,
        -0.4985314,  0.0415560,  1.0572252);

    return XYZ_TO_REC709 * xyz;
}

// Fresnel of a base surface under a film of the given IOR and thickness in nanometers. The stack is
// air, the film and the base; the film absorbs nothing, so what leaves is the geometric series of
// what bounces between its two interfaces, expanded to the second order as the specification does.
vec3 F_Iridescence(float outsideIor, float filmIor, const vec3 baseF0,
        highp float thicknessNm, float cosTheta1) {
    // A film thin enough not to be there must behave as though it is not, or a thickness map whose
    // minimum is zero shows an edge where the first interface appears.
    float iridescenceIor = mix(outsideIor, filmIor, smoothstep(0.0, 0.03, thicknessNm));

    // Snell's law through the film. Past the critical angle nothing enters it at all.
    float cosTheta2Sq = 1.0 - sq(outsideIor / iridescenceIor) * (1.0 - sq(cosTheta1));
    if (cosTheta2Sq < 0.0) {
        return vec3(1.0);
    }
    float cosTheta2 = sqrt(cosTheta2Sq);

    float R12 = F_SchlickIridescence(iorToF0(iridescenceIor, outsideIor), cosTheta1);
    float T121 = 1.0 - R12;

    // the clamp short of one is what keeps a white conductor's f0 from taking f0ToIor() to infinity
    vec3 baseIor = f0ToIor(clamp(baseF0, 0.0, 0.9999));
    vec3 R23 = F_SchlickIridescence(iorToF0(baseIor, iridescenceIor), cosTheta2);

    highp float opd = 2.0 * iridescenceIor * thicknessNm * cosTheta2;

    // the phase each reflection picks up, approximated as nothing entering a denser medium and half
    // a turn entering a thinner one
    float phi21 = PI - (iridescenceIor < outsideIor ? PI : 0.0);
    highp vec3 phi = phi21 + vec3(
            baseIor.x < iridescenceIor ? PI : 0.0,
            baseIor.y < iridescenceIor ? PI : 0.0,
            baseIor.z < iridescenceIor ? PI : 0.0);

    vec3 R123 = clamp(R12 * R23, 1e-5, 0.9999);
    vec3 r123 = sqrt(R123);
    vec3 Rs = (T121 * T121) * R23 / (1.0 - R123);

    vec3 I = R12 + Rs;
    vec3 Cm = Rs - T121;

    for (int m = 1; m <= 2; m++) {
        Cm *= r123;
        I += Cm * 2.0 * evaluateIridescenceSensitivity(float(m) * opd, float(m) * phi);
    }

    return max(I, vec3(0.0));
}

// Inverse of F_SchlickIridescence: the f0 whose Schlick curve passes through F at this angle.
vec3 F_IridescenceToF0(const vec3 F, float cosTheta) {
    // Schlick reaches one at grazing incidence whatever f0 was, so no f0 reproduces anything else
    // there; the clamp keeps the division finite as that is approached
    float weight = min(pow5(saturate(1.0 - cosTheta)), 0.9999);

    return saturate((F - weight) / (1.0 - weight));
}

// Evaluate the film at the viewing angle and refit the result to a Schlick curve, so that what the
// lobes below it read is an ordinary f0.
//
// The specification's rgb_mix() is deliberately absent: it weights the diffuse lobe by the inverse
// of the film's strongest channel, and Filament does not take a Fresnel share out of the diffuse
// lobe for an ordinary surface either.
vec3 iridescentF0(const vec3 baseF0, float NoV, float iridescence, float iridescenceIor,
        highp float thicknessNm) {
    vec3 F = F_Iridescence(1.0, iridescenceIor, baseF0, thicknessNm, NoV);

    return mix(baseF0, F_IridescenceToF0(F, NoV), iridescence);
}

#endif // MATERIAL_HAS_IRIDESCENCE

//------------------------------------------------------------------------------
// Specular BRDF dispatch
//------------------------------------------------------------------------------

float distribution(float roughness, float NoH, const vec3 h) {
#if BRDF_SPECULAR_D == SPECULAR_D_GGX
    return D_GGX(roughness, NoH, h);
#endif
}

float visibility(float roughness, float NoV, float NoL) {
#if BRDF_SPECULAR_V == SPECULAR_V_SMITH_GGX
    return V_SmithGGXCorrelated(roughness, NoV, NoL);
#elif BRDF_SPECULAR_V == SPECULAR_V_SMITH_GGX_FAST
    return V_SmithGGXCorrelated_Fast(roughness, NoV, NoL);
#endif
}

vec3 fresnel(const vec3 f0, float LoH) {
#if BRDF_SPECULAR_F == SPECULAR_F_SCHLICK
#if FILAMENT_QUALITY == FILAMENT_QUALITY_LOW
    return F_Schlick(f0, LoH); // f90 = 1.0
#else
    float f90 = saturate(dot(f0, vec3(50.0 * 0.33)));
    return F_Schlick(f0, f90, LoH);
#endif
#endif
}

vec3 fresnel(const vec3 f0, const float f90, float LoH) {
#if BRDF_SPECULAR_F == SPECULAR_F_SCHLICK
    return F_Schlick(f0, f90, LoH);
#endif
}

float distributionAnisotropic(float at, float ab, float ToH, float BoH, float NoH) {
#if BRDF_ANISOTROPIC_D == SPECULAR_D_GGX_ANISOTROPIC
    return D_GGX_Anisotropic(at, ab, ToH, BoH, NoH);
#endif
}

float visibilityAnisotropic(float roughness, float at, float ab,
        float ToV, float BoV, float ToL, float BoL, float NoV, float NoL) {
#if BRDF_ANISOTROPIC_V == SPECULAR_V_SMITH_GGX
    return V_SmithGGXCorrelated(roughness, NoV, NoL);
#elif BRDF_ANISOTROPIC_V == SPECULAR_V_GGX_ANISOTROPIC
    return V_SmithGGXCorrelated_Anisotropic(at, ab, ToV, BoV, ToL, BoL, NoV, NoL);
#endif
}

float distributionClearCoat(float roughness, float NoH, const vec3 h) {
#if BRDF_CLEAR_COAT_D == SPECULAR_D_GGX
    return D_GGX(roughness, NoH, h);
#endif
}

float visibilityClearCoat(float LoH) {
#if BRDF_CLEAR_COAT_V == SPECULAR_V_KELEMEN
    return V_Kelemen(LoH);
#endif
}

float distributionCloth(float roughness, float NoH) {
#if BRDF_CLOTH_D == SPECULAR_D_CHARLIE
    return D_Charlie(roughness, NoH);
#endif
}

float visibilityCloth(float NoV, float NoL) {
#if BRDF_CLOTH_V == SPECULAR_V_NEUBELT
    return V_Neubelt(NoV, NoL);
#endif
}

//------------------------------------------------------------------------------
// Diffuse BRDF implementations
//------------------------------------------------------------------------------

float Fd_Lambert() {
    return 1.0 / PI;
}

float Fd_Burley(float roughness, float NoV, float NoL, float LoH) {
    // Burley 2012, "Physically-Based Shading at Disney"
    float f90 = 0.5 + 2.0 * roughness * LoH * LoH;
    float lightScatter = F_Schlick(1.0, f90, NoL);
    float viewScatter  = F_Schlick(1.0, f90, NoV);
    return lightScatter * viewScatter * (1.0 / PI);
}

// Energy conserving wrap diffuse term, does *not* include the divide by pi
float Fd_Wrap(float NoL, float w) {
    return saturate((NoL + w) / sq(1.0 + w));
}

//------------------------------------------------------------------------------
// Diffuse BRDF dispatch
//------------------------------------------------------------------------------

float diffuse(float roughness, float NoV, float NoL, float LoH) {
#if BRDF_DIFFUSE == DIFFUSE_LAMBERT
    return Fd_Lambert();
#elif BRDF_DIFFUSE == DIFFUSE_BURLEY
    return Fd_Burley(roughness, NoV, NoL, LoH);
#endif
}
