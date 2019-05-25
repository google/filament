// ======================================================================== //
// Copyright 2009-2019 Intel Corporation                                    //
//                                                                          //
// Licensed under the Apache License, Version 2.0 (the "License");          //
// you may not use this file except in compliance with the License.         //
// You may obtain a copy of the License at                                  //
//                                                                          //
//     http://www.apache.org/licenses/LICENSE-2.0                           //
//                                                                          //
// Unless required by applicable law or agreed to in writing, software      //
// distributed under the License is distributed on an "AS IS" BASIS,        //
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. //
// See the License for the specific language governing permissions and      //
// limitations under the License.                                           //
// ======================================================================== //

#include "platform.h"

namespace oidn {

  void* alignedMalloc(size_t size, size_t alignment)
  {
    if (size == 0)
      return nullptr;

    assert((alignment & (alignment-1)) == 0);
    void* ptr = _mm_malloc(size, alignment);

    if (ptr == nullptr)
      throw std::bad_alloc();

    return ptr;
  }

  void alignedFree(void* ptr)
  {
    if (ptr)
      _mm_free(ptr);
  }

} // namespace oidn
