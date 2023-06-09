#version 450

struct AStruct { vec4 foobar; };

void someFunction(out AStruct s) { s.foobar = vec4(1.0); }

highp vec3 global_variable;

void otherFunction() {
    global_variable = vec3(1.0);
}

layout(location = 0) out vec3 FragColor;

void main() {
	AStruct inputs;
	someFunction(inputs);
    otherFunction();
	FragColor = global_variable;
}
