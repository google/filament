#version 400

struct Inner {
    int id;
    float value;
};

struct Outer {
    int before;
    uint padding;
    mat2 matrix;
    int after;
};

struct Complex {
    Inner nested;
    Inner array[2];
};

struct Point {
    float x;
    float y;
};

struct Color {
    vec3 rgb;
    float alpha;
};

void store_data(Inner data[4]) {
    // Function that takes array of structs
}

void store_outer(Outer value) {
    // Function that takes single struct
}

void process_point(Point p) {
    // Function that takes single struct
}

void main() {
    // Pattern 1: Complex array constructors (most common tint failure pattern)
    Outer data[4] = Outer[4](
        Outer(0, 0u, mat2(vec2(0.0), vec2(0.0)), 0),
        Outer(1, 1u, mat2(vec2(1.0), vec2(1.0)), 1),
        Outer(2, 2u, mat2(vec2(2.0), vec2(2.0)), 2),
        Outer(3, 3u, mat2(vec2(3.0), vec2(3.0)), 3)
    );
    
    // Pattern 2: Nested struct array constructors
    Inner inner_array[4] = Inner[4](
        Inner(0, 0.0),
        Inner(1, 1.0),
        Inner(2, 2.0),
        Inner(3, 3.0)
    );
    
    // Pattern 3: Struct with nested struct constructors
    Complex complex_data = Complex(
        Inner(42, 3.14),
        Inner[2](Inner(1, 1.0), Inner(2, 2.0))
    );
    
    // Pattern 4: Function calls with array constructors
    store_data(Inner[4](Inner(1, 1.0), Inner(2, 2.0), Inner(3, 3.0), Inner(4, 4.0)));
    store_outer(Outer(10, 10u, mat2(vec2(5.0), vec2(6.0)), 20));
    
    // Pattern 5: Basic struct constructors (from deleted basic_struct_constructor.frag)
    Point p1 = Point(1.0, 2.0);
    Color c1 = Color(vec3(1.0, 0.0, 0.0), 1.0);
    Point p2;
    p2 = Point(3.0, 4.0);
    
    // Pattern 6: Struct constructors in function calls (from deleted struct_constructor_in_function_call.frag)
    process_point(Point(5.0, 6.0));
} 