//
// Copyright 2021 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// proc_table:
//   Mapping from a string entry point name to function address.
//

#ifndef LIBGLESV2_PROC_TABLE_CL_H_
#define LIBGLESV2_PROC_TABLE_CL_H_

#include <string>
#include <unordered_map>

namespace cl
{

using ProcTable = std::unordered_map<std::string, void *>;

const ProcTable &GetProcTable();

}  // namespace cl

#endif  // LIBGLESV2_PROC_TABLE_CL_H_
