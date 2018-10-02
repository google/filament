/*
 * Copyright 2015 The Etc2Comp Authors.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *  http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#pragma once

#include "EtcConfig.h"

#if ETC_WINDOWS
	const char ETC_PATH_SLASH = '\\';
	const char ETC_BAD_PATH_SLASH = '/';

	extern const char *ETC_MKDIR_COMMAND;
	extern const char *ETC_IF_DIR_NOT_EXIST_COMMAND;

	int strcasecmp(const char *s1, const char *s2);
#else
	const char ETC_PATH_SLASH = '/';
	const char ETC_BAD_PATH_SLASH = '\\';

	extern const char *ETC_MKDIR_COMMAND;
#endif

	void CreateNewDir(const char *path);
