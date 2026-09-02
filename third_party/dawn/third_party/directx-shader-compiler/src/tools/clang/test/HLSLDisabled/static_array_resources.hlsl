// RUN: %dxc -E main -T vs_6_2 %s | FileCheck %s

// Repro of GitHub #2061

Buffer buf;
static Buffer bufs[1];
void main() { bufs[0] = buf; }