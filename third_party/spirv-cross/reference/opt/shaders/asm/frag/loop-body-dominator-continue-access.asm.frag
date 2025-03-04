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

int _239;

layout(binding = 0, std140) uniform Foo
{
    layout(row_major) mat4 lightVP[64];
    uint shadowCascadesNum;
    int test;
} _16;

layout(location = 0) in vec3 fragWorld;
layout(location = 0) out int _entryPointOutput;

mat4 spvWorkaroundRowMajor(mat4 wrap) { return wrap; }

void main()
{
    int _236;
    do
    {
        bool _233;
        int _237;
        uint _230 = 0u;
        SPIRV_CROSS_UNROLL
        for (;;)
        {
            if (_230 < _16.shadowCascadesNum)
            {
                mat4 _231;
                do
                {
                    if (_16.test == 0)
                    {
                        _231 = mat4(vec4(0.5, 0.0, 0.0, 0.0), vec4(0.0, 0.5, 0.0, 0.0), vec4(0.0, 0.0, 0.5, 0.0), vec4(0.0, 0.0, 0.0, 1.0));
                        break;
                    }
                    _231 = mat4(vec4(1.0, 0.0, 0.0, 0.0), vec4(0.0, 1.0, 0.0, 0.0), vec4(0.0, 0.0, 1.0, 0.0), vec4(0.0, 0.0, 0.0, 1.0));
                    break;
                } while(false);
                vec4 _178 = (_231 * spvWorkaroundRowMajor(_16.lightVP[_230])) * vec4(fragWorld, 1.0);
                float _180 = _178.z;
                float _187 = _178.x;
                float _189 = _178.y;
                if ((((_180 >= 0.0) && (_180 <= 1.0)) && (max(_187, _189) <= 1.0)) && (min(_187, _189) >= 0.0))
                {
                    _237 = int(_230);
                    _233 = true;
                    break;
                }
                _230++;
                continue;
            }
            else
            {
                _237 = _239;
                _233 = false;
                break;
            }
        }
        if (_233)
        {
            _236 = _237;
            break;
        }
        _236 = -1;
        break;
    } while(false);
    _entryPointOutput = _236;
}

