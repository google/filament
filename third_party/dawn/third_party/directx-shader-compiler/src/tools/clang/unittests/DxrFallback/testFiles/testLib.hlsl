#include "testLib.h"

#define INLINING [noinline] // Hide cruft to make debugging easier.
//#define INLINING

INLINING
void logAppend(int val)
{
  int slot;
  InterlockedAdd(output[0], 1, slot);
  slot += 1; // to account for the slot counter being at position 0
  output[slot] = val;
}

INLINING
void logAppend2(int key, int val)
{
  int slot;
  InterlockedAdd(output[0], 2, slot);
  slot += 1; // to account for the slot counter being at position 0
  output[slot + 0] = key;
  output[slot + 1] = val;
}

INLINING
int load(int idx)
{
  return one[0] * idx;
}

INLINING
void verify(int val, int expected)
{
  logAppend2(val, expected);
}

INLINING
void append(int val)
{
  logAppend(val);
}

INLINING
int consume()
{
  int slot;
  InterlockedAdd(input[0], 1, slot);
  slot += 1;
  return input[slot];
}

INLINING
int peekInput()
{
  int slot;
  InterlockedAdd(input[0], 0, slot);
  slot += 1;
  return input[slot];
}
