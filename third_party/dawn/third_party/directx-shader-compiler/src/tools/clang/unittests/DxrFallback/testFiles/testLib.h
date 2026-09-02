#include "HLSLRayTracingInternalPrototypes.h"

RWStructuredBuffer<int> input : register(u0);
RWStructuredBuffer<int> output : register(u1);
RWStructuredBuffer<int> one : register(u2);

cbuffer TestConstants : register(b0) { int initialStateId; }

// Read one integer from the input buffer
int consume();

int peekInput();

// Write val to the output buffer
void append(int val);

// Returns idx by reading the value of 1 from the one buffer. This is to avoid
// the compiler optimizing stuff away.
int load(int idx);

// Write both val and the expected value to the output
void verify(int val, int expected);
