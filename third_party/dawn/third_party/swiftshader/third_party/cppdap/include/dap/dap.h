// Copyright 2020 Google LLC
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     https://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#ifndef dap_dap_h
#define dap_dap_h

namespace dap {

// Explicit library initialization and termination functions.
//
// cppdap automatically initializes and terminates its internal state using lazy
// static initialization, and so will usually work fine without explicit calls
// to these functions.
// However, if you use cppdap types in global state, you may need to call these
// functions to ensure that cppdap is not uninitialized before the last usage.
//
// Each call to initialize() must have a corresponding call to terminate().
// It is undefined behaviour to call initialize() after terminate().
void initialize();
void terminate();

}  // namespace dap

#endif  // dap_dap_h
