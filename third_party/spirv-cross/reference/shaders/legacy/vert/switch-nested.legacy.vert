#version 100

struct UBO
{
    int func_arg;
    int inner_func_arg;
};

uniform UBO _34;

vec4 test_inner_func(bool b)
{
    if (b)
    {
        return vec4(1.0);
    }
    else
    {
        return vec4(0.0);
    }
}

vec4 test_func(bool b)
{
    if (b)
    {
        bool param = _34.inner_func_arg != 0;
        return test_inner_func(param);
    }
    else
    {
        return vec4(0.0);
    }
}

void main()
{
    bool param = _34.func_arg != 0;
    gl_Position = test_func(param);
}

