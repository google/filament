#version 450

layout(binding = 0, std140) uniform Foo
{
    layout(row_major) mat4 lightVP[64];
    uint shadowCascadesNum;
    int test;
} _11;

layout(location = 0) in vec3 fragWorld;
layout(location = 0) out int _entryPointOutput;

mat4 _235;
int _245;

void main()
{
    uint _229;
    bool _231;
    mat4 _234;
    _234 = _235;
    _231 = false;
    _229 = 0u;
    bool _251;
    mat4 _232;
    int _243;
    bool _158;
    for (;;)
    {
        _158 = _229 < _11.shadowCascadesNum;
        if (_158)
        {
            bool _209 = _11.test == 0;
            mat4 _233;
            if (_209)
            {
                _233 = mat4(vec4(0.5, 0.0, 0.0, 0.0), vec4(0.0, 0.5, 0.0, 0.0), vec4(0.0, 0.0, 0.5, 0.0), vec4(0.0, 0.0, 0.0, 1.0));
            }
            else
            {
                _233 = _234;
            }
            bool _250 = _209 ? true : _231;
            if (!_250)
            {
                _232 = mat4(vec4(1.0, 0.0, 0.0, 0.0), vec4(0.0, 1.0, 0.0, 0.0), vec4(0.0, 0.0, 1.0, 0.0), vec4(0.0, 0.0, 0.0, 1.0));
            }
            else
            {
                _232 = _233;
            }
            _251 = _250 ? _250 : true;
            vec4 _171 = (_232 * _11.lightVP[_229]) * vec4(fragWorld, 1.0);
            float _218 = _171.z;
            float _222 = _171.x;
            float _224 = _171.y;
            if ((((_218 >= 0.0) && (_218 <= 1.0)) && (max(_222, _224) <= 1.0)) && (min(_222, _224) >= 0.0))
            {
                _243 = int(_229);
                break;
            }
            else
            {
                _234 = _232;
                _231 = _251;
                _229++;
                continue;
            }
            _234 = _232;
            _231 = _251;
            _229++;
            continue;
        }
        else
        {
            _243 = _245;
            break;
        }
    }
    _entryPointOutput = (_158 ? true : false) ? _243 : (-1);
}

