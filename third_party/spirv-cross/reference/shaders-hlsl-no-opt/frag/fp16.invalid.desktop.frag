static min16float4 v4;
static min16float3 v3;
static min16float v1;
static min16float2 v2;
static float o1;
static float2 o2;
static float3 o3;
static float4 o4;

struct SPIRV_Cross_Input
{
    min16float v1 : TEXCOORD0;
    min16float2 v2 : TEXCOORD1;
    min16float3 v3 : TEXCOORD2;
    min16float4 v4 : TEXCOORD3;
};

struct SPIRV_Cross_Output
{
    float o1 : SV_Target0;
    float2 o2 : SV_Target1;
    float3 o3 : SV_Target2;
    float4 o4 : SV_Target3;
};

float mod(float x, float y)
{
    return x - y * floor(x / y);
}

float2 mod(float2 x, float2 y)
{
    return x - y * floor(x / y);
}

float3 mod(float3 x, float3 y)
{
    return x - y * floor(x / y);
}

float4 mod(float4 x, float4 y)
{
    return x - y * floor(x / y);
}

uint SPIRV_Cross_packFloat2x16(min16float2 value)
{
    uint2 Packed = f32tof16(value);
    return Packed.x | (Packed.y << 16);
}

min16float2 SPIRV_Cross_unpackFloat2x16(uint value)
{
    return min16float2(f16tof32(uint2(value & 0xffff, value >> 16)));
}

void test_constants()
{
    min16float a = min16float(1.0);
    min16float b = min16float(1.5);
    min16float c = min16float(-1.5);
    min16float d = min16float(0.0 / 0.0);
    min16float e = min16float(1.0 / 0.0);
    min16float f = min16float(-1.0 / 0.0);
    min16float g = min16float(1014.0);
    min16float h = min16float(9.5367431640625e-07);
}

min16float test_result()
{
    return min16float(1.0);
}

void test_conversions()
{
    min16float one = test_result();
    int a = int(one);
    uint b = uint(one);
    bool c = one != min16float(0.0);
    float d = float(one);
    double e = double(one);
    min16float a2 = min16float(a);
    min16float b2 = min16float(b);
    min16float c2 = min16float(c);
    min16float d2 = min16float(d);
    min16float e2 = min16float(e);
}

void test_builtins()
{
    min16float4 res = radians(v4);
    res = degrees(v4);
    res = sin(v4);
    res = cos(v4);
    res = tan(v4);
    res = asin(v4);
    res = atan2(v4, v3.xyzz);
    res = atan(v4);
    res = sinh(v4);
    res = cosh(v4);
    res = tanh(v4);
    res = pow(v4, v4);
    res = exp(v4);
    res = log(v4);
    res = exp2(v4);
    res = log2(v4);
    res = sqrt(v4);
    res = rsqrt(v4);
    res = abs(v4);
    res = sign(v4);
    res = floor(v4);
    res = trunc(v4);
    res = round(v4);
    res = ceil(v4);
    res = frac(v4);
    res = mod(v4, v4);
    min16float4 tmp;
    min16float4 _144 = modf(v4, tmp);
    res = _144;
    res = min(v4, v4);
    res = max(v4, v4);
    res = clamp(v4, v4, v4);
    res = lerp(v4, v4, v4);
    bool4 _164 = bool4(v4.x < v4.x, v4.y < v4.y, v4.z < v4.z, v4.w < v4.w);
    res = min16float4(_164.x ? v4.x : v4.x, _164.y ? v4.y : v4.y, _164.z ? v4.z : v4.z, _164.w ? v4.w : v4.w);
    res = step(v4, v4);
    res = smoothstep(v4, v4, v4);
    bool4 btmp = isnan(v4);
    btmp = isinf(v4);
    res = mad(v4, v4, v4);
    uint pack0 = SPIRV_Cross_packFloat2x16(v4.xy);
    uint pack1 = SPIRV_Cross_packFloat2x16(v4.zw);
    res = min16float4(SPIRV_Cross_unpackFloat2x16(pack0), SPIRV_Cross_unpackFloat2x16(pack1));
    min16float t0 = length(v4);
    t0 = distance(v4, v4);
    t0 = dot(v4, v4);
    min16float3 res3 = cross(v3, v3);
    res = normalize(v4);
    res = faceforward(v4, v4, v4);
    res = reflect(v4, v4);
    res = refract(v4, v4, v1);
    btmp = bool4(v4.x < v4.x, v4.y < v4.y, v4.z < v4.z, v4.w < v4.w);
    btmp = bool4(v4.x <= v4.x, v4.y <= v4.y, v4.z <= v4.z, v4.w <= v4.w);
    btmp = bool4(v4.x > v4.x, v4.y > v4.y, v4.z > v4.z, v4.w > v4.w);
    btmp = bool4(v4.x >= v4.x, v4.y >= v4.y, v4.z >= v4.z, v4.w >= v4.w);
    btmp = bool4(v4.x == v4.x, v4.y == v4.y, v4.z == v4.z, v4.w == v4.w);
    btmp = bool4(v4.x != v4.x, v4.y != v4.y, v4.z != v4.z, v4.w != v4.w);
    res = ddx(v4);
    res = ddy(v4);
    res = ddx_fine(v4);
    res = ddy_fine(v4);
    res = ddx_coarse(v4);
    res = ddy_coarse(v4);
    res = fwidth(v4);
    res = fwidth(v4);
    res = fwidth(v4);
}

void frag_main()
{
    test_constants();
    test_conversions();
    test_builtins();
}

SPIRV_Cross_Output main(SPIRV_Cross_Input stage_input)
{
    v4 = stage_input.v4;
    v3 = stage_input.v3;
    v1 = stage_input.v1;
    v2 = stage_input.v2;
    frag_main();
    SPIRV_Cross_Output stage_output;
    stage_output.o1 = o1;
    stage_output.o2 = o2;
    stage_output.o3 = o3;
    stage_output.o4 = o4;
    return stage_output;
}
