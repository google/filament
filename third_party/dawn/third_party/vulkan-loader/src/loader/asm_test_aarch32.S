//
// Copyright (c) 2021 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
// Copyright (c) 2024 Valve Corporation
// Copyright (c) 2024 LunarG, Inc.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//

.text
.global sample
.set PHYS_DEV_OFFSET_INST_DISPATCH, 10
.set PTR_SIZE, 4
sample:
  mov r1, #(PHYS_DEV_OFFSET_INST_DISPATCH + (PTR_SIZE * 4))
  ldr r0, [r0, r1]
