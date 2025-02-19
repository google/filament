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

_15 _10264;

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
    _15 _13863;
    _13863._m0 = vec4(0.0);
    vec2 _19927 = gl_FragCoord.xy * _15259._m23.xy;
    vec4 _17581 = _22044._m2 * _22044._m0.xyxy;
    vec2 _13149 = clamp(_19927 + (vec3(0.0, -2.0, 0.5).xy * _22044._m0.xy), _17581.xy, _17581.zw);
    vec3 _12103 = _12348._m5 * clamp(textureLod(SPIRV_Cross_Combined, _13149, 0.0).w * _22044._m1, 0.0, 1.0);
    vec4 _17670 = textureLod(SPIRV_Cross_Combined_1, _13149, 0.0);
    vec3 _7719;
    SPIRV_CROSS_BRANCH
    if (_17670.y > 0.0)
    {
        _7719 = _12103 + (textureLod(SPIRV_Cross_Combined_2, _13149, 0.0).xyz * clamp(_17670.y * _17670.z, 0.0, 1.0));
    }
    else
    {
        _7719 = _12103;
    }
    vec3 _22177 = vec4(0.0).xyz + (_7719 * 0.5);
    vec4 _15527 = vec4(_22177.x, _22177.y, _22177.z, vec4(0.0).w);
    _13863._m0 = _15527;
    vec2 _13150 = clamp(_19927 + (vec3(-1.0, -1.0, 0.5).xy * _22044._m0.xy), _17581.xy, _17581.zw);
    vec3 _12104 = _12348._m5 * clamp(textureLod(SPIRV_Cross_Combined, _13150, 0.0).w * _22044._m1, 0.0, 1.0);
    vec4 _17671 = textureLod(SPIRV_Cross_Combined_1, _13150, 0.0);
    vec3 _7720;
    SPIRV_CROSS_BRANCH
    if (_17671.y > 0.0)
    {
        _7720 = _12104 + (textureLod(SPIRV_Cross_Combined_2, _13150, 0.0).xyz * clamp(_17671.y * _17671.z, 0.0, 1.0));
    }
    else
    {
        _7720 = _12104;
    }
    vec3 _22178 = _15527.xyz + (_7720 * 0.5);
    vec4 _15528 = vec4(_22178.x, _22178.y, _22178.z, _15527.w);
    _13863._m0 = _15528;
    vec2 _13151 = clamp(_19927 + (vec3(0.0, -1.0, 0.75).xy * _22044._m0.xy), _17581.xy, _17581.zw);
    vec3 _12105 = _12348._m5 * clamp(textureLod(SPIRV_Cross_Combined, _13151, 0.0).w * _22044._m1, 0.0, 1.0);
    vec4 _17672 = textureLod(SPIRV_Cross_Combined_1, _13151, 0.0);
    vec3 _7721;
    SPIRV_CROSS_BRANCH
    if (_17672.y > 0.0)
    {
        _7721 = _12105 + (textureLod(SPIRV_Cross_Combined_2, _13151, 0.0).xyz * clamp(_17672.y * _17672.z, 0.0, 1.0));
    }
    else
    {
        _7721 = _12105;
    }
    vec3 _22179 = _15528.xyz + (_7721 * 0.75);
    vec4 _15529 = vec4(_22179.x, _22179.y, _22179.z, _15528.w);
    _13863._m0 = _15529;
    vec2 _13152 = clamp(_19927 + (vec3(1.0, -1.0, 0.5).xy * _22044._m0.xy), _17581.xy, _17581.zw);
    vec3 _12106 = _12348._m5 * clamp(textureLod(SPIRV_Cross_Combined, _13152, 0.0).w * _22044._m1, 0.0, 1.0);
    vec4 _17673 = textureLod(SPIRV_Cross_Combined_1, _13152, 0.0);
    vec3 _7722;
    SPIRV_CROSS_BRANCH
    if (_17673.y > 0.0)
    {
        _7722 = _12106 + (textureLod(SPIRV_Cross_Combined_2, _13152, 0.0).xyz * clamp(_17673.y * _17673.z, 0.0, 1.0));
    }
    else
    {
        _7722 = _12106;
    }
    vec3 _22180 = _15529.xyz + (_7722 * 0.5);
    vec4 _15530 = vec4(_22180.x, _22180.y, _22180.z, _15529.w);
    _13863._m0 = _15530;
    vec2 _13153 = clamp(_19927 + (vec3(-2.0, 0.0, 0.5).xy * _22044._m0.xy), _17581.xy, _17581.zw);
    vec3 _12107 = _12348._m5 * clamp(textureLod(SPIRV_Cross_Combined, _13153, 0.0).w * _22044._m1, 0.0, 1.0);
    vec4 _17674 = textureLod(SPIRV_Cross_Combined_1, _13153, 0.0);
    vec3 _7723;
    SPIRV_CROSS_BRANCH
    if (_17674.y > 0.0)
    {
        _7723 = _12107 + (textureLod(SPIRV_Cross_Combined_2, _13153, 0.0).xyz * clamp(_17674.y * _17674.z, 0.0, 1.0));
    }
    else
    {
        _7723 = _12107;
    }
    vec3 _22181 = _15530.xyz + (_7723 * 0.5);
    vec4 _15531 = vec4(_22181.x, _22181.y, _22181.z, _15530.w);
    _13863._m0 = _15531;
    vec2 _13154 = clamp(_19927 + (vec3(-1.0, 0.0, 0.75).xy * _22044._m0.xy), _17581.xy, _17581.zw);
    vec3 _12108 = _12348._m5 * clamp(textureLod(SPIRV_Cross_Combined, _13154, 0.0).w * _22044._m1, 0.0, 1.0);
    vec4 _17675 = textureLod(SPIRV_Cross_Combined_1, _13154, 0.0);
    vec3 _7724;
    SPIRV_CROSS_BRANCH
    if (_17675.y > 0.0)
    {
        _7724 = _12108 + (textureLod(SPIRV_Cross_Combined_2, _13154, 0.0).xyz * clamp(_17675.y * _17675.z, 0.0, 1.0));
    }
    else
    {
        _7724 = _12108;
    }
    vec3 _22182 = _15531.xyz + (_7724 * 0.75);
    vec4 _15532 = vec4(_22182.x, _22182.y, _22182.z, _15531.w);
    _13863._m0 = _15532;
    vec2 _13155 = clamp(_19927 + (vec3(0.0, 0.0, 1.0).xy * _22044._m0.xy), _17581.xy, _17581.zw);
    vec3 _12109 = _12348._m5 * clamp(textureLod(SPIRV_Cross_Combined, _13155, 0.0).w * _22044._m1, 0.0, 1.0);
    vec4 _17676 = textureLod(SPIRV_Cross_Combined_1, _13155, 0.0);
    vec3 _7725;
    SPIRV_CROSS_BRANCH
    if (_17676.y > 0.0)
    {
        _7725 = _12109 + (textureLod(SPIRV_Cross_Combined_2, _13155, 0.0).xyz * clamp(_17676.y * _17676.z, 0.0, 1.0));
    }
    else
    {
        _7725 = _12109;
    }
    vec3 _22183 = _15532.xyz + (_7725 * 1.0);
    vec4 _15533 = vec4(_22183.x, _22183.y, _22183.z, _15532.w);
    _13863._m0 = _15533;
    vec2 _13156 = clamp(_19927 + (vec3(1.0, 0.0, 0.75).xy * _22044._m0.xy), _17581.xy, _17581.zw);
    vec3 _12110 = _12348._m5 * clamp(textureLod(SPIRV_Cross_Combined, _13156, 0.0).w * _22044._m1, 0.0, 1.0);
    vec4 _17677 = textureLod(SPIRV_Cross_Combined_1, _13156, 0.0);
    vec3 _7726;
    SPIRV_CROSS_BRANCH
    if (_17677.y > 0.0)
    {
        _7726 = _12110 + (textureLod(SPIRV_Cross_Combined_2, _13156, 0.0).xyz * clamp(_17677.y * _17677.z, 0.0, 1.0));
    }
    else
    {
        _7726 = _12110;
    }
    vec3 _22184 = _15533.xyz + (_7726 * 0.75);
    vec4 _15534 = vec4(_22184.x, _22184.y, _22184.z, _15533.w);
    _13863._m0 = _15534;
    vec2 _13157 = clamp(_19927 + (vec3(2.0, 0.0, 0.5).xy * _22044._m0.xy), _17581.xy, _17581.zw);
    vec3 _12111 = _12348._m5 * clamp(textureLod(SPIRV_Cross_Combined, _13157, 0.0).w * _22044._m1, 0.0, 1.0);
    vec4 _17678 = textureLod(SPIRV_Cross_Combined_1, _13157, 0.0);
    vec3 _7727;
    SPIRV_CROSS_BRANCH
    if (_17678.y > 0.0)
    {
        _7727 = _12111 + (textureLod(SPIRV_Cross_Combined_2, _13157, 0.0).xyz * clamp(_17678.y * _17678.z, 0.0, 1.0));
    }
    else
    {
        _7727 = _12111;
    }
    vec3 _22185 = _15534.xyz + (_7727 * 0.5);
    vec4 _15535 = vec4(_22185.x, _22185.y, _22185.z, _15534.w);
    _13863._m0 = _15535;
    vec2 _13158 = clamp(_19927 + (vec3(-1.0, 1.0, 0.5).xy * _22044._m0.xy), _17581.xy, _17581.zw);
    vec3 _12112 = _12348._m5 * clamp(textureLod(SPIRV_Cross_Combined, _13158, 0.0).w * _22044._m1, 0.0, 1.0);
    vec4 _17679 = textureLod(SPIRV_Cross_Combined_1, _13158, 0.0);
    vec3 _7728;
    SPIRV_CROSS_BRANCH
    if (_17679.y > 0.0)
    {
        _7728 = _12112 + (textureLod(SPIRV_Cross_Combined_2, _13158, 0.0).xyz * clamp(_17679.y * _17679.z, 0.0, 1.0));
    }
    else
    {
        _7728 = _12112;
    }
    vec3 _22186 = _15535.xyz + (_7728 * 0.5);
    vec4 _15536 = vec4(_22186.x, _22186.y, _22186.z, _15535.w);
    _13863._m0 = _15536;
    vec2 _13159 = clamp(_19927 + (vec3(0.0, 1.0, 0.75).xy * _22044._m0.xy), _17581.xy, _17581.zw);
    vec3 _12113 = _12348._m5 * clamp(textureLod(SPIRV_Cross_Combined, _13159, 0.0).w * _22044._m1, 0.0, 1.0);
    vec4 _17680 = textureLod(SPIRV_Cross_Combined_1, _13159, 0.0);
    vec3 _7729;
    SPIRV_CROSS_BRANCH
    if (_17680.y > 0.0)
    {
        _7729 = _12113 + (textureLod(SPIRV_Cross_Combined_2, _13159, 0.0).xyz * clamp(_17680.y * _17680.z, 0.0, 1.0));
    }
    else
    {
        _7729 = _12113;
    }
    vec3 _22187 = _15536.xyz + (_7729 * 0.75);
    vec4 _15537 = vec4(_22187.x, _22187.y, _22187.z, _15536.w);
    _13863._m0 = _15537;
    vec2 _13160 = clamp(_19927 + (vec3(1.0, 1.0, 0.5).xy * _22044._m0.xy), _17581.xy, _17581.zw);
    vec3 _12114 = _12348._m5 * clamp(textureLod(SPIRV_Cross_Combined, _13160, 0.0).w * _22044._m1, 0.0, 1.0);
    vec4 _17681 = textureLod(SPIRV_Cross_Combined_1, _13160, 0.0);
    vec3 _7730;
    SPIRV_CROSS_BRANCH
    if (_17681.y > 0.0)
    {
        _7730 = _12114 + (textureLod(SPIRV_Cross_Combined_2, _13160, 0.0).xyz * clamp(_17681.y * _17681.z, 0.0, 1.0));
    }
    else
    {
        _7730 = _12114;
    }
    vec3 _22188 = _15537.xyz + (_7730 * 0.5);
    vec4 _15539 = vec4(_22188.x, _22188.y, _22188.z, _15537.w);
    _13863._m0 = _15539;
    vec2 _13161 = clamp(_19927 + (vec3(0.0, 2.0, 0.5).xy * _22044._m0.xy), _17581.xy, _17581.zw);
    vec3 _12115 = _12348._m5 * clamp(textureLod(SPIRV_Cross_Combined, _13161, 0.0).w * _22044._m1, 0.0, 1.0);
    vec4 _17682 = textureLod(SPIRV_Cross_Combined_1, _13161, 0.0);
    vec3 _7731;
    SPIRV_CROSS_BRANCH
    if (_17682.y > 0.0)
    {
        _7731 = _12115 + (textureLod(SPIRV_Cross_Combined_2, _13161, 0.0).xyz * clamp(_17682.y * _17682.z, 0.0, 1.0));
    }
    else
    {
        _7731 = _12115;
    }
    vec3 _22189 = _15539.xyz + (_7731 * 0.5);
    vec4 _15541 = vec4(_22189.x, _22189.y, _22189.z, _15539.w);
    _13863._m0 = _15541;
    vec3 _13750 = _15541.xyz / vec3(((((((((((((0.0 + 0.5) + 0.5) + 0.75) + 0.5) + 0.5) + 0.75) + 1.0) + 0.75) + 0.5) + 0.5) + 0.75) + 0.5) + 0.5);
    _13863._m0 = vec4(_13750.x, _13750.y, _13750.z, _15541.w);
    _13863._m0.w = 1.0;
    _4317 = _13863._m0;
}

