#version 450

struct Registers
{
    float foo;
};

uniform Registers registers;

layout(location = 0) out float FragColor;

float add_value(float v, float w)
{
    return v + w;
}

void main()
{
    FragColor = add_value(10.0, registers.foo);
}

