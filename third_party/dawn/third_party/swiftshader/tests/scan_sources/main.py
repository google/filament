#!/usr/bin/env python3

# Copyright 2020 The SwiftShader Authors. All Rights Reserved.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#    http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#
# Scan all source files for problems that are inconvenient or impossible
# to guard against using static assertions in c++

import glob
import sys
import re

def main(rootdir):
	filelist = []
	for x in [rootdir +"/**/*.cpp", rootdir + "/**/*.hpp"]:
		filelist += glob.glob(x, recursive=True)

	# Regex for Memset< template use as base class (must be the first,
	# thus, there must not be a ',' before it)
	memset_template_check = re.compile(",\s*Memset\s*<")
	
	# Regex for Memset( template use as initializer (must be the first,
	# thus, there must not be a ',' before it)
	memset_call_check = re.compile(",\s*Memset\s*\(")

	retval = 0

	tb = 0
	for fn in filelist:
		with open(fn, encoding="utf-8") as f:
			contents = f.read();
			if memset_template_check.search(contents) != None:
				print("ERROR: " + fn + " has illegal memset<> template use: must always be the first base class.")
				retval = 1
			if memset_call_check.search(contents) != None:
				print("ERROR: " + fn + " has illegal memset<> template use: not called as the first initializer.")
				retval = 1

	sys.exit(retval)

if len(sys.argv) < 2:
	print("Give source directory as parameter.")
	sys.exit(1)
main(sys.argv[1])