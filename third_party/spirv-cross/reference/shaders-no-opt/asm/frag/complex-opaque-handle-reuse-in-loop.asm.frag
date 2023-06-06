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

struct MyConsts
{
    uint opt;
};

uvec4 _37;

layout(binding = 3, std140) uniform type_scene
{
    MyConsts myConsts;
} scene;

uniform sampler2D SPIRV_Cross_CombinedtexTablemySampler[1];

layout(location = 1) out uint out_var_SV_TARGET1;

void main()
{
    uint _42;
    bool _47;
    float _55;
    do
    {
        _42 = _37.y & 16777215u;
        _47 = scene.myConsts.opt != 0u;
        SPIRV_CROSS_BRANCH
        if (_47)
        {
            _55 = 1.0;
            break;
        }
        else
        {
            _55 = textureLod(SPIRV_Cross_CombinedtexTablemySampler[_42], vec2(0.0), 0.0).x;
            break;
        }
        break; // unreachable workaround
    } while(false);
    float _66;
    do
    {
        SPIRV_CROSS_BRANCH
        if (_47)
        {
            _66 = 1.0;
            break;
        }
        else
        {
            _66 = textureLod(SPIRV_Cross_CombinedtexTablemySampler[_42], vec2(0.0), 0.0).x;
            break;
        }
        break; // unreachable workaround
    } while(false);
    out_var_SV_TARGET1 = uint(cross(vec3(-1.0, -1.0, _55), vec3(1.0, 1.0, _66)).x);
}

