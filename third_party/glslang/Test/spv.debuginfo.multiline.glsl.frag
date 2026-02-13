#version 460

in float inx;
out float outx;

float add(float x, float y, float z) {
    return 
        x
        +
        y
        +
        z
    ;
}

void main() {
    outx
        =
        add(
            inx+1,
            inx+2,
            inx+3
        )
    ;
}