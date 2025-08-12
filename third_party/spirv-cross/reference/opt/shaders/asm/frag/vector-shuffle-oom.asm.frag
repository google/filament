#version 450
#if defined(GL_EXT_control_flow_attributes)
#extension GL_EXT_control_flow_attributes : require
#define SPIRV_CROSS_FLATTEN [[flatten]]
#define SPIRV_CROSS_BRANCH [[dont_flatten]]
#define SPIRV_CROSS_UNROLL [[unroll]]
#define SPIRV_CROSS_LOOP [[dont_unroll]]
#else
#define SPIRV_CROSS_FLATTEN
#define SPIRV_CROSS_BRANCH
#define SPIRV_CROSS_UNROLL
#define SPIRV_CROSS_LOOP
#endif

struct _15
{
    vec4 _m0;
};

layout(binding = 0, std140) uniform _3_22044
{
    vec4 _m0;
    float _m1;
    vec4 _m2;
} _22044;

layout(binding = 2, std140) uniform _4_12348
{
    vec3 _m0;
    vec3 _m1;
    float _m2;
    vec3 _m3;
    float _m4;
    vec3 _m5;
    float _m6;
    vec3 _m7;
    float _m8;
    vec3 _m9;
    float _m10;
    vec3 _m11;
    float _m12;
    vec2 _m13;
    vec2 _m14;
    vec3 _m15;
    float _m16;
    float _m17;
    float _m18;
    float _m19;
    float _m20;
    vec4 _m21;
    vec4 _m22;
    layout(row_major) mat4 _m23;
    vec4 _m24;
} _12348;

layout(binding = 1, std140) uniform _7_15259
{
    layout(row_major) mat4 _m0;
    layout(row_major) mat4 _m1;
    layout(row_major) mat4 _m2;
    layout(row_major) mat4 _m3;
    vec4 _m4;
    vec4 _m5;
    float _m6;
    float _m7;
    float _m8;
    float _m9;
    vec3 _m10;
    float _m11;
    vec3 _m12;
    float _m13;
    vec3 _m14;
    float _m15;
    vec3 _m16;
    float _m17;
    float _m18;
    float _m19;
    vec2 _m20;
    vec2 _m21;
    vec2 _m22;
    vec4 _m23;
    vec2 _m24;
    vec2 _m25;
    vec2 _m26;
    vec3 _m27;
    float _m28;
    float _m29;
    float _m30;
    float _m31;
    float _m32;
    vec2 _m33;
    float _m34;
    float _m35;
    vec3 _m36;
    layout(row_major) mat4 _m37[2];
    vec4 _m38[2];
} _15259;

uniform sampler2D SPIRV_Cross_Combined;
uniform sampler2D SPIRV_Cross_Combined_1;
uniform sampler2D SPIRV_Cross_Combined_2;

layout(location = 0) out vec4 _4317;

void main()
{
    vec4 _17581 = _22044._m2 * _22044._m0.xyxy;
    vec2 _7011 = _17581.xy;
    vec2 _21058 = _17581.zw;
    vec2 _13149 = clamp(fma(gl_FragCoord.xy, _15259._m23.xy, vec2(0.0, -2.0) * _22044._m0.xy), _7011, _21058);
    vec3 _12103 = _12348._m5 * clamp(textureLod(SPIRV_Cross_Combined, _13149, 0.0).w * _22044._m1, 0.0, 1.0);
    vec4 _17670 = textureLod(SPIRV_Cross_Combined_1, _13149, 0.0);
    float _16938 = _17670.y;
    vec3 _7719;
    SPIRV_CROSS_BRANCH
    if (_16938 > 0.0)
    {
        _7719 = _12103 + (textureLod(SPIRV_Cross_Combined_2, _13149, 0.0).xyz * clamp(_16938 * _17670.z, 0.0, 1.0));
    }
    else
    {
        _7719 = _12103;
    }
    vec2 _13150 = clamp(fma(gl_FragCoord.xy, _15259._m23.xy, vec2(-1.0) * _22044._m0.xy), _7011, _21058);
    vec3 _12104 = _12348._m5 * clamp(textureLod(SPIRV_Cross_Combined, _13150, 0.0).w * _22044._m1, 0.0, 1.0);
    vec4 _17671 = textureLod(SPIRV_Cross_Combined_1, _13150, 0.0);
    float _16939 = _17671.y;
    vec3 _7720;
    SPIRV_CROSS_BRANCH
    if (_16939 > 0.0)
    {
        _7720 = _12104 + (textureLod(SPIRV_Cross_Combined_2, _13150, 0.0).xyz * clamp(_16939 * _17671.z, 0.0, 1.0));
    }
    else
    {
        _7720 = _12104;
    }
    vec2 _13151 = clamp(fma(gl_FragCoord.xy, _15259._m23.xy, vec2(0.0, -1.0) * _22044._m0.xy), _7011, _21058);
    vec3 _12105 = _12348._m5 * clamp(textureLod(SPIRV_Cross_Combined, _13151, 0.0).w * _22044._m1, 0.0, 1.0);
    vec4 _17672 = textureLod(SPIRV_Cross_Combined_1, _13151, 0.0);
    float _16940 = _17672.y;
    vec3 _7721;
    SPIRV_CROSS_BRANCH
    if (_16940 > 0.0)
    {
        _7721 = _12105 + (textureLod(SPIRV_Cross_Combined_2, _13151, 0.0).xyz * clamp(_16940 * _17672.z, 0.0, 1.0));
    }
    else
    {
        _7721 = _12105;
    }
    vec2 _13152 = clamp(fma(gl_FragCoord.xy, _15259._m23.xy, vec2(1.0, -1.0) * _22044._m0.xy), _7011, _21058);
    vec3 _12106 = _12348._m5 * clamp(textureLod(SPIRV_Cross_Combined, _13152, 0.0).w * _22044._m1, 0.0, 1.0);
    vec4 _17673 = textureLod(SPIRV_Cross_Combined_1, _13152, 0.0);
    float _16941 = _17673.y;
    vec3 _7722;
    SPIRV_CROSS_BRANCH
    if (_16941 > 0.0)
    {
        _7722 = _12106 + (textureLod(SPIRV_Cross_Combined_2, _13152, 0.0).xyz * clamp(_16941 * _17673.z, 0.0, 1.0));
    }
    else
    {
        _7722 = _12106;
    }
    vec2 _13153 = clamp(fma(gl_FragCoord.xy, _15259._m23.xy, vec2(-2.0, 0.0) * _22044._m0.xy), _7011, _21058);
    vec3 _12107 = _12348._m5 * clamp(textureLod(SPIRV_Cross_Combined, _13153, 0.0).w * _22044._m1, 0.0, 1.0);
    vec4 _17674 = textureLod(SPIRV_Cross_Combined_1, _13153, 0.0);
    float _16942 = _17674.y;
    vec3 _7723;
    SPIRV_CROSS_BRANCH
    if (_16942 > 0.0)
    {
        _7723 = _12107 + (textureLod(SPIRV_Cross_Combined_2, _13153, 0.0).xyz * clamp(_16942 * _17674.z, 0.0, 1.0));
    }
    else
    {
        _7723 = _12107;
    }
    vec2 _13154 = clamp(fma(gl_FragCoord.xy, _15259._m23.xy, vec2(-1.0, 0.0) * _22044._m0.xy), _7011, _21058);
    vec3 _12108 = _12348._m5 * clamp(textureLod(SPIRV_Cross_Combined, _13154, 0.0).w * _22044._m1, 0.0, 1.0);
    vec4 _17675 = textureLod(SPIRV_Cross_Combined_1, _13154, 0.0);
    float _16943 = _17675.y;
    vec3 _7724;
    SPIRV_CROSS_BRANCH
    if (_16943 > 0.0)
    {
        _7724 = _12108 + (textureLod(SPIRV_Cross_Combined_2, _13154, 0.0).xyz * clamp(_16943 * _17675.z, 0.0, 1.0));
    }
    else
    {
        _7724 = _12108;
    }
    vec2 _13155 = clamp(gl_FragCoord.xy * _15259._m23.xy, _7011, _21058);
    vec3 _12109 = _12348._m5 * clamp(textureLod(SPIRV_Cross_Combined, _13155, 0.0).w * _22044._m1, 0.0, 1.0);
    vec4 _17676 = textureLod(SPIRV_Cross_Combined_1, _13155, 0.0);
    float _16944 = _17676.y;
    vec3 _7725;
    SPIRV_CROSS_BRANCH
    if (_16944 > 0.0)
    {
        _7725 = _12109 + (textureLod(SPIRV_Cross_Combined_2, _13155, 0.0).xyz * clamp(_16944 * _17676.z, 0.0, 1.0));
    }
    else
    {
        _7725 = _12109;
    }
    vec2 _13156 = clamp(fma(gl_FragCoord.xy, _15259._m23.xy, vec2(1.0, 0.0) * _22044._m0.xy), _7011, _21058);
    vec3 _12110 = _12348._m5 * clamp(textureLod(SPIRV_Cross_Combined, _13156, 0.0).w * _22044._m1, 0.0, 1.0);
    vec4 _17677 = textureLod(SPIRV_Cross_Combined_1, _13156, 0.0);
    float _16945 = _17677.y;
    vec3 _7726;
    SPIRV_CROSS_BRANCH
    if (_16945 > 0.0)
    {
        _7726 = _12110 + (textureLod(SPIRV_Cross_Combined_2, _13156, 0.0).xyz * clamp(_16945 * _17677.z, 0.0, 1.0));
    }
    else
    {
        _7726 = _12110;
    }
    vec2 _13157 = clamp(fma(gl_FragCoord.xy, _15259._m23.xy, vec2(2.0, 0.0) * _22044._m0.xy), _7011, _21058);
    vec3 _12111 = _12348._m5 * clamp(textureLod(SPIRV_Cross_Combined, _13157, 0.0).w * _22044._m1, 0.0, 1.0);
    vec4 _17678 = textureLod(SPIRV_Cross_Combined_1, _13157, 0.0);
    float _16946 = _17678.y;
    vec3 _7727;
    SPIRV_CROSS_BRANCH
    if (_16946 > 0.0)
    {
        _7727 = _12111 + (textureLod(SPIRV_Cross_Combined_2, _13157, 0.0).xyz * clamp(_16946 * _17678.z, 0.0, 1.0));
    }
    else
    {
        _7727 = _12111;
    }
    vec2 _13158 = clamp(fma(gl_FragCoord.xy, _15259._m23.xy, vec2(-1.0, 1.0) * _22044._m0.xy), _7011, _21058);
    vec3 _12112 = _12348._m5 * clamp(textureLod(SPIRV_Cross_Combined, _13158, 0.0).w * _22044._m1, 0.0, 1.0);
    vec4 _17679 = textureLod(SPIRV_Cross_Combined_1, _13158, 0.0);
    float _16947 = _17679.y;
    vec3 _7728;
    SPIRV_CROSS_BRANCH
    if (_16947 > 0.0)
    {
        _7728 = _12112 + (textureLod(SPIRV_Cross_Combined_2, _13158, 0.0).xyz * clamp(_16947 * _17679.z, 0.0, 1.0));
    }
    else
    {
        _7728 = _12112;
    }
    vec2 _13159 = clamp(fma(gl_FragCoord.xy, _15259._m23.xy, vec2(0.0, 1.0) * _22044._m0.xy), _7011, _21058);
    vec3 _12113 = _12348._m5 * clamp(textureLod(SPIRV_Cross_Combined, _13159, 0.0).w * _22044._m1, 0.0, 1.0);
    vec4 _17680 = textureLod(SPIRV_Cross_Combined_1, _13159, 0.0);
    float _16948 = _17680.y;
    vec3 _7729;
    SPIRV_CROSS_BRANCH
    if (_16948 > 0.0)
    {
        _7729 = _12113 + (textureLod(SPIRV_Cross_Combined_2, _13159, 0.0).xyz * clamp(_16948 * _17680.z, 0.0, 1.0));
    }
    else
    {
        _7729 = _12113;
    }
    vec2 _13160 = clamp(fma(gl_FragCoord.xy, _15259._m23.xy, _22044._m0.xy), _7011, _21058);
    vec3 _12114 = _12348._m5 * clamp(textureLod(SPIRV_Cross_Combined, _13160, 0.0).w * _22044._m1, 0.0, 1.0);
    vec4 _17681 = textureLod(SPIRV_Cross_Combined_1, _13160, 0.0);
    float _16949 = _17681.y;
    vec3 _7730;
    SPIRV_CROSS_BRANCH
    if (_16949 > 0.0)
    {
        _7730 = _12114 + (textureLod(SPIRV_Cross_Combined_2, _13160, 0.0).xyz * clamp(_16949 * _17681.z, 0.0, 1.0));
    }
    else
    {
        _7730 = _12114;
    }
    vec2 _13161 = clamp(fma(gl_FragCoord.xy, _15259._m23.xy, vec2(0.0, 2.0) * _22044._m0.xy), _7011, _21058);
    vec3 _12115 = _12348._m5 * clamp(textureLod(SPIRV_Cross_Combined, _13161, 0.0).w * _22044._m1, 0.0, 1.0);
    vec4 _17682 = textureLod(SPIRV_Cross_Combined_1, _13161, 0.0);
    float _16950 = _17682.y;
    vec3 _7731;
    SPIRV_CROSS_BRANCH
    if (_16950 > 0.0)
    {
        _7731 = _12115 + (textureLod(SPIRV_Cross_Combined_2, _13161, 0.0).xyz * clamp(_16950 * _17682.z, 0.0, 1.0));
    }
    else
    {
        _7731 = _12115;
    }
    vec3 _13750 = (((((((((((((_7719 * 0.5).xyz + (_7720 * 0.5)).xyz + (_7721 * 0.75)).xyz + (_7722 * 0.5)).xyz + (_7723 * 0.5)).xyz + (_7724 * 0.75)).xyz + (_7725 * 1.0)).xyz + (_7726 * 0.75)).xyz + (_7727 * 0.5)).xyz + (_7728 * 0.5)).xyz + (_7729 * 0.75)).xyz + (_7730 * 0.5)).xyz + (_7731 * 0.5)).xyz * vec3(0.125);
    _15 _25050 = _15(vec4(_13750.x, _13750.y, _13750.z, vec4(0.0).w));
    _25050._m0.w = 1.0;
    _4317 = _25050._m0;
}

