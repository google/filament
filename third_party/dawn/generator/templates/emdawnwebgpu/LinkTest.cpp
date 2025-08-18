#include <cstring>
#include <webgpu/webgpu.h>

namespace {

// Make a runtime-constructed value of T in a way that can't be optimized away.
template<typename T>
T val() {
    T value{};
    static volatile bool x;
    if (x) {
        char nonzero = 123;
        memcpy(&value, &nonzero, 1);
    }
    return value;
}

}

{% macro render_dummy_args(this, args) %}
    {%- if this %}
        val<{{ as_cType(this.name) }}>()
        {{- ", " if args }}
    {%- endif %}
    {%- for arg in args -%}
        val<{{ decorate(as_cType(arg.type.name), arg) }}>()
        {{- ", " if not loop.last }}
    {%- endfor %}
{%- endmacro -%}

int main() {
    {% for function in by_category["function"] %}
        {{as_cMethod(None, function.name)}}(
            {{- render_dummy_args(None, function.arguments) -}}
        );
    {% endfor %}
    {% for type in by_category["object"] if len(c_methods(type)) > 0 %}

        {% for method in c_methods(type) %}
            {{as_cMethod(type.name, method.name)}}(
                {{- render_dummy_args(type, method.arguments) -}}
            );
        {% endfor %}
    {% endfor %}
}
