#version 100

struct UBO
{
    int func_arg;
    int inner_func_arg;
};

uniform UBO _34;

void main()
{
    vec4 _102;
    for (int SPIRV_Cross_Dummy30 = 0; SPIRV_Cross_Dummy30 < 1; SPIRV_Cross_Dummy30++)
    {
        if (_34.func_arg != 0)
        {
            vec4 _101;
            for (int SPIRV_Cross_Dummy45 = 0; SPIRV_Cross_Dummy45 < 1; SPIRV_Cross_Dummy45++)
            {
                if (_34.inner_func_arg != 0)
                {
                    _101 = vec4(1.0);
                    break;
                }
                else
                {
                    _101 = vec4(0.0);
                    break;
                }
            }
            _102 = _101;
            break;
        }
        else
        {
            _102 = vec4(0.0);
            break;
        }
    }
    gl_Position = _102;
}

