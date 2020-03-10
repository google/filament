#version 450

layout(binding = 0, std140) uniform Foo
{
    layout(row_major) mat4 lightVP[64];
    uint shadowCascadesNum;
    int test;
} _11;

layout(location = 0) in vec3 fragWorld;
layout(location = 0) out int _entryPointOutput;

int _228;

void main()
{
    int _225;
    switch (0u)
    {
        default:
        {
            bool _222;
            int _226;
            uint _219 = 0u;
            for (;;)
            {
                if (_219 < _11.shadowCascadesNum)
                {
                    mat4 _220;
                    switch (0u)
                    {
                        default:
                        {
                            if (_11.test == 0)
                            {
                                _220 = mat4(vec4(0.5, 0.0, 0.0, 0.0), vec4(0.0, 0.5, 0.0, 0.0), vec4(0.0, 0.0, 0.5, 0.0), vec4(0.0, 0.0, 0.0, 1.0));
                                break;
                            }
                            _220 = mat4(vec4(1.0, 0.0, 0.0, 0.0), vec4(0.0, 1.0, 0.0, 0.0), vec4(0.0, 0.0, 1.0, 0.0), vec4(0.0, 0.0, 0.0, 1.0));
                            break;
                        }
                    }
                    vec4 _171 = (_220 * _11.lightVP[_219]) * vec4(fragWorld, 1.0);
                    float _173 = _171.z;
                    float _180 = _171.x;
                    float _182 = _171.y;
                    if ((((_173 >= 0.0) && (_173 <= 1.0)) && (max(_180, _182) <= 1.0)) && (min(_180, _182) >= 0.0))
                    {
                        _226 = int(_219);
                        _222 = true;
                        break;
                    }
                    _219++;
                    continue;
                }
                else
                {
                    _226 = _228;
                    _222 = false;
                    break;
                }
            }
            _225 = -1;
            break;
        }
    }
    _entryPointOutput = _225;
}

