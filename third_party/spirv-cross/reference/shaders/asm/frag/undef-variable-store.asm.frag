#version 450

layout(location = 0) out vec4 _entryPointOutput;

vec4 _38;
vec4 _47;

void main()
{
    vec4 _27;
    do
    {
        vec2 _26 = vec2(0.0);
        if (_26.x != 0.0)
        {
            _27 = vec4(1.0, 0.0, 0.0, 1.0);
            break;
        }
        else
        {
            _27 = vec4(1.0, 1.0, 0.0, 1.0);
            break;
        }
        _27 = _38;
        break;
    } while (false);
    _entryPointOutput = _27;
}

