#version 100

struct Foo
{
    vec4 a;
    vec4 b;
};

struct Bar
{
    vec4 a;
    vec4 b;
};

struct Baz
{
    Foo foo;
    Bar bar;
};

varying vec4 _12_a_a;
varying vec4 _12_a_b;
varying vec4 _12_b_a;
varying vec4 _12_b_b;
varying vec4 baz_foo_a;
varying vec4 baz_foo_b;
varying vec4 baz_bar_a;
varying vec4 baz_bar_b;

void main()
{
    _12_a_a = vec4(10.0);
    _12_a_b = vec4(20.0);
    _12_b_a = vec4(30.0);
    _12_b_b = vec4(40.0);
    _12_a_a = Foo(vec4(50.0), vec4(60.0)).a;
    _12_a_b = Foo(vec4(50.0), vec4(60.0)).b;
    _12_b_a = Bar(vec4(50.0), vec4(60.0)).a;
    _12_b_b = Bar(vec4(50.0), vec4(60.0)).b;
    baz_foo_a = Foo(vec4(100.0), vec4(200.0)).a;
    baz_foo_b = Foo(vec4(100.0), vec4(200.0)).b;
    baz_bar_a = Bar(vec4(300.0), vec4(400.0)).a;
    baz_bar_b = Bar(vec4(300.0), vec4(400.0)).b;
    baz_foo_a = Baz(Foo(vec4(1000.0), vec4(2000.0)), Bar(vec4(3000.0), vec4(4000.0))).foo.a;
    baz_foo_b = Baz(Foo(vec4(1000.0), vec4(2000.0)), Bar(vec4(3000.0), vec4(4000.0))).foo.b;
    baz_bar_a = Baz(Foo(vec4(1000.0), vec4(2000.0)), Bar(vec4(3000.0), vec4(4000.0))).bar.a;
    baz_bar_b = Baz(Foo(vec4(1000.0), vec4(2000.0)), Bar(vec4(3000.0), vec4(4000.0))).bar.b;
}

