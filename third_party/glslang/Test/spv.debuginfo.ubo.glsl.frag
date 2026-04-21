#version 460

uniform UBO1 {
    vec4 memberA;
};

uniform UBO2 {
    vec4 memberB;
} uboInstance;

out vec4 outColor;

void main() {
    outColor = memberA + uboInstance.memberB;
}