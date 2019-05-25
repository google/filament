/*******************************************************************************
* Copyright 2017-2018 Intel Corporation
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

#ifndef _CONV_HPP
#define _CONV_HPP

#include <stdint.h>
#include <limits.h>
#include <assert.h>

#include "common.hpp"
#include "dnn_types.hpp"
#include "mkldnn_common.hpp"
#include "mkldnn_memory.hpp"

#include "conv/conv_common.hpp"
namespace conv {

int doit(const prb_t *p, res_t *res);
int bench(int argc, char **argv, bool main_bench = true);

}

#endif
