# Copyright 2019 The SwiftShader Authors. All Rights Reserved.
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

import argparse, json, sys

def run():
    parser = argparse.ArgumentParser(
        description = "Generates the correct ICD JSON file for swiftshader in GN builds"
    )
    parser.add_argument('--input', type=str, help='The template ICD JSON')
    parser.add_argument('--output', type=str, help='The output ICD JSON in the GN build dir')
    parser.add_argument('--library_path', type=str, help='The file containing a list of directories to check for stale files')
    args = parser.parse_args()

    with open(args.input) as infile:
        with open(args.output, 'w') as outfile:
            data = json.load(infile)
            data['ICD']['library_path'] = args.library_path
            json.dump(data, outfile)

    return 0

if __name__ == "__main__":
    sys.exit(run())
