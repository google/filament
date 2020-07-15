#version 100
precision mediump float;
precision highp int;

struct Foo
{
    highp vec4 a;
    highp vec4 b;
};

struct Bar
{
    highp vec4 a;
    highp vec4 b;
};

struct Baz
{
    Foo foo;
    Bar bar;
};

varying highp vec4 baz_foo_a;
varying highp vec4 baz_foo_b;
varying highp vec4 baz_bar_a;
varying highp vec4 baz_bar_b;
varying highp vec4 _33_a_a;
varying highp vec4 _33_a_b;
varying highp vec4 _33_b_a;
varying highp vec4 _33_b_b;

void main()
{
    gl_FragData[0] = (((_33_a_a + _33_b_b) + baz_foo_b) + baz_foo_a) + baz_bar_b;
}

