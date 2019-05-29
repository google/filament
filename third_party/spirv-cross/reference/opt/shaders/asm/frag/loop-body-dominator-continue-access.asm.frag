#version 450

layout(binding = 0, std140) uniform Foo
{
    layout(row_major) mat4 lightVP[64];
    uint shadowCascadesNum;
    int test;
} _11;

layout(location = 0) in vec3 fragWorld;
layout(location = 0) out int _entryPointOutput;

int _240;

void main()
{
    uint _227;
    int _236;
    for (;;)
    {
        _227 = 0u;
        bool _231;
        int _237;
        for (;;)
        {
            if (_227 < _11.shadowCascadesNum)
            {
                mat4 _228;
                for (;;)
                {
                    if (_11.test == 0)
                    {
                        _228 = mat4(vec4(0.5, 0.0, 0.0, 0.0), vec4(0.0, 0.5, 0.0, 0.0), vec4(0.0, 0.0, 0.5, 0.0), vec4(0.0, 0.0, 0.0, 1.0));
                        break;
                    }
                    _228 = mat4(vec4(1.0, 0.0, 0.0, 0.0), vec4(0.0, 1.0, 0.0, 0.0), vec4(0.0, 0.0, 1.0, 0.0), vec4(0.0, 0.0, 0.0, 1.0));
                    break;
                }
                vec4 _177 = (_228 * _11.lightVP[_227]) * vec4(fragWorld, 1.0);
                float _179 = _177.z;
                float _186 = _177.x;
                float _188 = _177.y;
                if ((((_179 >= 0.0) && (_179 <= 1.0)) && (max(_186, _188) <= 1.0)) && (min(_186, _188) >= 0.0))
                {
                    _237 = int(_227);
                    _231 = true;
                    break;
                }
                else
                {
                    _227++;
                    continue;
                }
                _227++;
                continue;
            }
            else
            {
                _237 = _240;
                _231 = false;
                break;
            }
        }
        if (_231)
        {
            _236 = _237;
            break;
        }
        _236 = -1;
        break;
    }
    _entryPointOutput = _236;
}

