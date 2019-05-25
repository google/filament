/*******************************************************************************
* Copyright 2018 Intel Corporation
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
*     http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*******************************************************************************/

#include <stdio.h>
#include "mkldnn.h"

int main() {
    printf("mkldnn_version: %d.%d.%d\n",
            MKLDNN_VERSION_MAJOR, MKLDNN_VERSION_MINOR, MKLDNN_VERSION_PATCH);
    printf("mkldnn_memory_desc_init_by_tag = %p, "
            "sizeof(mkldnn_memory_desc_t) = %d\n",
            mkldnn_memory_desc_init_by_tag,
            (int)sizeof(mkldnn_memory_desc_t));
    return 0;
}
