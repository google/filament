//===- subzero/src/IceRegList.h - Register list macro defs  -----*- C++ -*-===//
//
//                        The Subzero Code Generator
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
/// \file
/// \brief Defines the REGLIST*() macros used in the IceInst*.def files.
///
//===----------------------------------------------------------------------===//

#ifndef SUBZERO_SRC_ICEINSTREGLIST_H
#define SUBZERO_SRC_ICEINSTREGLIST_H

// REGLISTn is a family of macros that we use to define register aliasing.  "n"
// indicates how many register aliases are being provided to the macro.  It
// assumes the parameters are register names declared in the "ns"
// namespace/class, but with the common "Reg_" prefix removed for brevity.
#define NO_ALIASES()                                                           \
  {}
#define REGLIST1(ns, r0)                                                       \
  { ns::Reg_##r0 }
#define REGLIST2(ns, r0, r1)                                                   \
  { ns::Reg_##r0, ns::Reg_##r1 }
#define REGLIST3(ns, r0, r1, r2)                                               \
  { ns::Reg_##r0, ns::Reg_##r1, ns::Reg_##r2 }
#define REGLIST4(ns, r0, r1, r2, r3)                                           \
  { ns::Reg_##r0, ns::Reg_##r1, ns::Reg_##r2, ns::Reg_##r3 }
#define REGLIST7(ns, r0, r1, r2, r3, r4, r5, r6)                               \
  {                                                                            \
    ns::Reg_##r0, ns::Reg_##r1, ns::Reg_##r2, ns::Reg_##r3, ns::Reg_##r4,      \
        ns::Reg_##r5, ns::Reg_##r6                                             \
  }

#endif // SUBZERO_SRC_ICEINSTREGLIST_H
