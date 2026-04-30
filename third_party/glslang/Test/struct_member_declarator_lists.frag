#version 400

struct Vector {
    float x;
    float y;
    float z;
};

struct Color {
    float r;
    float g;
    float b;
};

struct Material {
    float shininess;
};

struct B {
    vec3 t;
};

struct K {
    // Original #3931 issue: struct names used as member names in declarator lists
    // These should be IDENTIFIER tokens, not TYPE_NAME
    float Vector, Color;           // Two struct names as member names
    int Material, shininess;       // Mix of struct name and regular name
    vec2 a, position, b;          // Regular names in declarator list
    mat3 ambient, diffuse, spec;  // Regular names in declarator list
    
    // Original bug report pattern
    float A, B;                   // B is a struct name used as member name
    
    // Array member pattern
    float A_array[2], B_array[3]; // B is a struct name used as array member name
};

void main() {
    K k;
    // Access the members that have struct names
    k.Vector = 1.0;
    k.Color = 2.0;
    k.Material = 3;
    k.B = 4.0;
    k.B_array[0] = 5.0;
    
    // Test variable declarations with struct names
    int x, B, y, K;
} 