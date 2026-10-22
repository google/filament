#version 460

layout(location = 0) in vec4 a0; // accessed
layout(location = 1) in vec4 a1; // accessed
layout(location = 2) in vec4 a2; // accessed
layout(location = 3) in vec4 a3; // accessed
layout(location = 4) in vec4 a4; // accessed
layout(location = 5) in vec4 a5; // accessed
layout(location = 6) in vec4 a6; // accessed
layout(location = 7) in vec4 a7; // accessed
layout(location = 8) in vec4 a8; // accessed
layout(location = 9) in vec4 a9; // accessed
layout(location = 10) in vec4 a10; // accessed
layout(location = 11) in vec4 n0; // not accessed
layout(location = 12) in vec4 n1; // not accessed
layout(location = 13) in vec4 n2; // not accessed
layout(location = 14) in vec4 n3; // not accessed
layout(location = 15) in vec4 n4; // not accessed
layout(location = 16) in vec4 n5; // not accessed
layout(location = 17) in vec4 n6; // not accessed
layout(location = 18) in vec4 n7; // not accessed
layout(location = 19) in vec4 n8; // not accessed
layout(location = 20) in vec4 n9; // not accessed

void main() {
    // empty case is live
    switch (2) {
    case 2: break;
    case 1:
        gl_Position = n0;
    }

    // empty default case is live
    switch (3) {
    default: break;
    case 1:
        gl_Position = n1;
    }

    // no live case
    switch (3) {
    case 2: // fallthrough
    case 1:
        gl_Position = n2;
    }

    // ensure break is handled correctly
    switch (1) {
    case -1:
        gl_Position = n3;
        break;
    case 1:
        gl_Position = a0;
        break;
        gl_Position = n4;
    case 0:
        gl_Position = n5;
    }

    const int cx = 1;

    // signed/unsigned mismatch
    switch (cx) {
    case uint(1):
        gl_Position = a1;
    }

    // signed/unsigned conversion
    switch (-1) {
    case ~uint(0):
        gl_Position = a2;
    }

    // const variable case
    switch (1) {
    case cx:
        gl_Position = a3;
        break;
    case -1:
        gl_Position = n6;
    }

    // fallthrough with const variable
    switch (cx) {
    default: // fallthrough
    case 2:
        gl_Position = a4;
    }

    // non-trivial constant expression
    switch (((cx + 1) * 2) - 3) {
    case 1:
        gl_Position = a5;
        break;
    case 2:
        gl_Position = n7;
    }

    // expression as case
    switch (5 + 3) {
    case 6 + 2:
        gl_Position = a6;
        break;
    case 5 + 2:
        gl_Position = n8;
        break;
    }

    int x = 2;

    // liveness of non-const variables cannot be deduced
    switch (x) {
    case 1:
        gl_Position = a7;
        break;
    case 2:
        gl_Position = a8;
        break;
    }

    // const and non-const expression
    switch (cx + x) {
    case 1:
        gl_Position = a9;
        break;
    case 3:
        gl_Position = a10;
        break;
    }
}