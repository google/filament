#version 460

uniform UBO {
    int x;
};

out vec4 outColor;

void main() {
    int x = 41;
    {
        int x = 42;

        while (bool test = x < 45) {
            x += 1;
        }

        for (int x = 50; bool test = x < 53; ) {
            x += 1;
        }

        outColor = vec4(x);
    }
}