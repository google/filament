#version 450

struct Registers
{
    float foo;
};

uniform Registers registers;

layout(location = 0) out float FragColor;

void main()
{
    FragColor = 10.0 + registers.foo;
}

