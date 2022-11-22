#!/usr/bin/python3

from z3 import *

def same8(v):
  return Or(v == 0, v == 0xff)

magic = BitVec('magic', 64)

x = BitVec('x', 64)
y = x * magic

s = Solver()
solve_using(s, ForAll([x],
  Or(
    Not(And([same8((x >> (i * 8)) & 0xff) for i in range(8)])), # x has bytes that aren't equal to 0xff or 0x00
    And([(x >> (i * 8 + 7)) & 1 == (y >> (56 + i)) & 1 for i in range(8)]) # every byte of x has bits that are equal to a corresponding top bit of y
)))

print("magic =", hex(s.model().eval(magic).as_long()))
