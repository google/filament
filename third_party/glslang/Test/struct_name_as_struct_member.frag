#version 400

// Original bug report from: https://github.com/KhronosGroup/glslang/issues/3931
// Error was: "syntax error, unexpected TYPE_NAME, expecting IDENTIFIER"
// when defining a struct field with the same name as a previously defined
// struct.
struct B
{
    vec3 t;
};

struct K
{
    float A, B; // This should work. The B field is in a different scope
		// than the B struct.
};

void main(){
  int x, B, y, K;
} 