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

#include "mkldnn_debug.h"

#include "common.hpp"
#include "mkldnn_debug.hpp"

const char *status2str(mkldnn_status_t status) {
    return mkldnn_status2str(status);
}

const char *dt2str(mkldnn_data_type_t dt) {
    return mkldnn_dt2str(dt);
}

mkldnn_data_type_t str2dt(const char *str) {
#define CASE(_dt) \
    if (!strcasecmp(STRINGIFY(_dt), str) \
            || !strcasecmp(STRINGIFY(CONCAT2(mkldnn_, _dt)), str)) \
        return CONCAT2(mkldnn_, _dt);
    CASE(s8);
    CASE(u8);
    CASE(s32);
    CASE(f32);
#undef CASE
    assert(!"unknown data type");
    return mkldnn_f32;
}

const char *tag2str(mkldnn_format_tag_t tag) {
    return mkldnn_fmt_tag2str(tag);
}

mkldnn_format_tag_t str2tag(const char *str) {
#define CASE(_tag) do { \
    if (!strcmp(STRINGIFY(_tag), str) \
            || !strcmp("mkldnn_" STRINGIFY(_tag), str)) \
        return CONCAT2(mkldnn_, _tag); \
} while (0)
    CASE(x);
    CASE(nc);
    CASE(ncw);
    CASE(nwc);
    CASE(nCw8c);
    CASE(nCw16c);
    CASE(nchw);
    CASE(nhwc);
    CASE(chwn);
    CASE(nChw8c);
    CASE(nChw16c);
    CASE(oi);
    CASE(io);
    CASE(oiw);
    CASE(wio);
    CASE(OIw16i16o);
    CASE(OIw16o16i);
    CASE(Oiw16o);
    CASE(Owi16o);
    CASE(OIw8i16o2i);
    CASE(OIw4i16o4i);
    CASE(oihw);
    CASE(ihwo);
    CASE(hwio);
    CASE(iohw);
    CASE(dhwio);
    CASE(OIhw8i8o);
    CASE(OIhw16i16o);
    CASE(OIhw8i16o2i);
    CASE(OIdhw8i16o2i);
    CASE(OIhw4i16o4i);
    CASE(OIhw8o16i2o);
    CASE(OIhw8o8i);
    CASE(OIhw16o16i);
    CASE(IOhw16o16i);
    CASE(Oihw16o);
    CASE(Ohwi8o);
    CASE(Ohwi16o);
    CASE(goiw);
    CASE(goihw);
    CASE(hwigo);
    CASE(giohw);
    CASE(goiw);
    CASE(gOIw16i16o);
    CASE(gOIw16o16i);
    CASE(gOiw16o);
    CASE(gOwi16o);
    CASE(gOIw8i16o2i);
    CASE(gOIw4i16o4i);
    CASE(Goiw16g);
    CASE(gOIhw8i8o);
    CASE(gOIhw16i16o);
    CASE(gOIhw8i16o2i);
    CASE(gOIdhw8i16o2i);
    CASE(gOIhw4i16o4i);
    CASE(gOIhw8o16i2o);
    CASE(gOIhw8o8i);
    CASE(gOIhw16o16i);
    CASE(gIOhw16o16i);
    CASE(gOihw16o);
    CASE(gOhwi8o);
    CASE(gOhwi16o);
    CASE(Goihw8g);
    CASE(Goihw16g);
    CASE(ncdhw);
    CASE(ndhwc);
    CASE(oidhw);
    CASE(goidhw);
    CASE(nCdhw8c);
    CASE(nCdhw16c);
    CASE(OIdhw16i16o);
    CASE(gOIdhw16i16o);
    CASE(OIdhw16o16i);
    CASE(gOIdhw16o16i);
    CASE(Oidhw16o);
    CASE(Odhwi16o);
    CASE(gOidhw16o);
    CASE(gOdhwi16o);
    CASE(ntc);
    CASE(tnc);
    CASE(ldsnc);
    CASE(ldigo);
    CASE(ldgoi);
    CASE(ldgo);
#undef CASE
    assert(!"unknown memory format tag");
    return mkldnn_format_tag_undef;
}
