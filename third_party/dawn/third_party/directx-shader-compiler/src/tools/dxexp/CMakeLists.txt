# Copyright (C) Microsoft Corporation. All rights reserved.
# This file is distributed under the University of Illinois Open Source License. See LICENSE.TXT for details.
add_llvm_tool(dxexp
  dxexp.cpp
)

find_package(D3D12 REQUIRED)

target_link_libraries(dxexp
  ${D3D12_LIBRARIES}
  dxguid.lib
)
