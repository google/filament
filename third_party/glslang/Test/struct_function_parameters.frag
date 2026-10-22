#version 400

struct Vertex {
    vec3 position;
    vec3 normal;
};

struct Light {
    vec3 position;
    vec3 color;
    float intensity;
};

struct Camera {
    mat4 viewMatrix;
    mat4 projMatrix;
};

// Test various positions of struct parameters - these should be TYPE_NAME
void render(Vertex v, Light l, Camera c) {
    // Function with struct parameters at start
}

void process(int id, Vertex v, float time) {
    // Function with struct parameter in middle
}

void compute(float delta, int count, Light l, Camera c, bool enabled) {
    // Function with multiple struct parameters mixed with other types
}

void single(Vertex v) {
    // Function with single struct parameter
}

void multiple(Vertex v1, Vertex v2, Light l1, Light l2) {
    // Function with multiple parameters of same struct type
}

void main() {
    Vertex v = Vertex(vec3(0.0), vec3(1.0, 0.0, 0.0));
    Light l = Light(vec3(10.0), vec3(1.0), 1.0);
    Camera c = Camera(mat4(1.0), mat4(1.0));
    
    render(v, l, c);
    process(1, v, 0.5);
    compute(0.016, 100, l, c, true);
} 