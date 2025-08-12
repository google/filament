#version 440

void main() {
    uint a = 5;
    atomicAdd(a * 2, 0);
    atomicAdd(a, 0);
}
