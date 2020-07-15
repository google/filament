#version 450

layout(binding = 0, std140) uniform Foo
{
    layout(row_major) mat4 lightVP[64];
    uint shadowCascadesNum;
    int test;
} _11;

layout(location = 0) in vec3 fragWorld;
layout(location = 0) out int _entryPointOutput;

int _231;

void main()
{
    int _228;
    do
    {
        bool _225;
        int _229;
        uint _222 = 0u;
        for (;;)
        {
            if (_222 < _11.shadowCascadesNum)
            {
                mat4 _223;
                do
                {
                    if (_11.test == 0)
                    {
                        _223 = mat4(vec4(0.5, 0.0, 0.0, 0.0), vec4(0.0, 0.5, 0.0, 0.0), vec4(0.0, 0.0, 0.5, 0.0), vec4(0.0, 0.0, 0.0, 1.0));
                        break;
                    }
                    _223 = mat4(vec4(1.0, 0.0, 0.0, 0.0), vec4(0.0, 1.0, 0.0, 0.0), vec4(0.0, 0.0, 1.0, 0.0), vec4(0.0, 0.0, 0.0, 1.0));
                    break;
                } while(false);
                vec4 _170 = (_223 * _11.lightVP[_222]) * vec4(fragWorld, 1.0);
                float _172 = _170.z;
                float _179 = _170.x;
                float _181 = _170.y;
                if ((((_172 >= 0.0) && (_172 <= 1.0)) && (max(_179, _181) <= 1.0)) && (min(_179, _181) >= 0.0))
                {
                    _229 = int(_222);
                    _225 = true;
                    break;
                }
                _222++;
                continue;
            }
            else
            {
                _229 = _231;
                _225 = false;
                break;
            }
        }
        _228 = -1;
        break;
    } while(false);
    _entryPointOutput = _228;
}

