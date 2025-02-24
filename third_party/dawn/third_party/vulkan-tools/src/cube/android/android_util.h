/*
 * Copyright (c) 2016 Google, Inc.
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
 *
 * Relicensed from the WTFPL (http://www.wtfpl.net/faq/).
 */

#ifndef ANDROID_UTIL_H
#define ANDROID_UTIL_H

#ifdef __cplusplus
extern "C" {
#endif

char **get_args(struct android_app *app, const char *intent_extra_data_key, const char *appTag, int *count);

#ifdef __cplusplus
}
#endif

#endif
