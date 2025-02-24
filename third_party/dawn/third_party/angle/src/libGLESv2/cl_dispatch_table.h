//
// Copyright 2021 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// cl_dispatch_table.h: Declares dispatch table for CL ICD Loader.

#ifndef LIBGLESV2_CL_DISPATCH_TABLE_H_
#define LIBGLESV2_CL_DISPATCH_TABLE_H_

#include "angle_cl.h"
#include "export.h"

extern "C" {

ANGLE_EXPORT extern const cl_icd_dispatch gCLIcdDispatchTable;

}  // extern "C"

#endif  // LIBGLESV2_CL_DISPATCH_TABLE_H_
