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

#ifndef _MKLDNN_DEBUG_HPP
#define _MKLDNN_DEBUG_HPP

#include "mkldnn.h"

/* status */
const char *status2str(mkldnn_status_t status);

/* data type */
const char *dt2str(mkldnn_data_type_t dt);
mkldnn_data_type_t str2dt(const char *str);

/* format */
const char *tag2str(mkldnn_format_tag_t tag);
mkldnn_format_tag_t str2tag(const char *str);

#endif
