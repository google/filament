#version 450

struct Foo
{
    vec4 bar[2];
    vec4 baz[2];
};

out vec4 _14_foo_bar[2];
out vec4 _14_foo_baz[2];
out vec4 _14_foo2_bar[2];
out vec4 _14_foo2_baz[2];
out vec4 foo3_bar[2];
out vec4 foo3_baz[2];

void main()
{
    _14_foo_bar[0] = vec4(1.0);
    _14_foo_baz[1] = vec4(2.0);
    _14_foo2_bar[0] = vec4(3.0);
    _14_foo2_baz[1] = vec4(4.0);
    foo3_bar[0] = vec4(5.0);
    foo3_baz[1] = vec4(6.0);
}

