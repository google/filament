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

#include "Etc.h"

#include "EtcSourceImage.h"
#include "EtcFile.h"
#include "EtcMath.h"
#include "EtcImage.h"
#include "EtcErrorMetric.h"
#include "EtcBlock4x4EncodingBits.h"
#include "EtcFileHeader.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>

#define MAX_JOBS 1024
using namespace Etc;

int RunMemTest(bool verboseOutput, size_t numTestIterations);





/*int getMem()
{

int tSize = 0, resident = 0, share = 0;
ifstream buffer("/proc/self/statm");
buffer >> tSize >> resident >> share;
buffer.close();

long page_size_kb = sysconf(_SC_PAGE_SIZE) / 1024; // in case x86-64 is configured to use 2MB pages
double rss = resident * page_size_kb;
cout << "RSS - " << rss << " kB\n";

double shared_mem = share * page_size_kb;
cout << "Shared Memory - " << shared_mem << " kB\n";

cout << "Private Memory - " << rss - shared_mem << "kB\n";
return 0;
}*/
