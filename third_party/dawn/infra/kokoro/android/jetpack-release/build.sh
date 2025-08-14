#!/bin/bash
# Copyright 2024 The Dawn & Tint Authors
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are met:
#
# 1. Redistributions of source code must retain the above copyright notice, this
#    list of conditions and the following disclaimer.
#
# 2. Redistributions in binary form must reproduce the above copyright notice,
#    this list of conditions and the following disclaimer in the documentation
#    and/or other materials provided with the distribution.
#
# 3. Neither the name of the copyright holder nor the names of its
#    contributors may be used to endorse or promote products derived from
#    this software without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
# AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
# DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
# FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
# DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
# SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
# CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
# OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
# OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.


# Script to build the Android archive package

set -e # Fail on any error

SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}")" >/dev/null 2>&1 && pwd )"
ROOT_DIR="$( cd "${SCRIPT_DIR}/../../../.." >/dev/null 2>&1 && pwd )"

pyenv global 3.9.5
# Enable the script runner to build and upload the .aar if different than the owners of dawn
git config --global --add safe.directory /tmpfs/src/git/dawn
cd $ROOT_DIR
apt-get install pkg-config

echo "**********Fetching Dawn's deps**********"
python3 tools/fetch_dawn_dependencies.py -ns
if [[ $? -ne 0 ]]
then
    echo "FAILURE in fetching deps"
    exit 1
fi

cd tools/android

# Use JDK17. Default is JDK11
echo "**********Updating Java**********"
sudo add-apt-repository ppa:cwchien/gradle
sudo apt-get update
apt-get install -y openjdk-17-jdk
export JAVA_HOME="$(update-java-alternatives -l | grep "1.17" | head -n 1 | tr -s " " | cut -d " " -f 3)"
if [[ $? -ne 0 ]]
then
    echo "FAILURE in updating Java"
    exit 1
fi

# gradle 8.0+ is expected for android library
echo "**********Installing Gradle**********"
sudo apt-get install gradle-8.3
sudo update-alternatives --set gradle /usr/lib/gradle/8.3/bin/gradle
if [[ $? -ne 0 ]]
then
    echo "FAILURE in installing gradle"
    exit 1
fi

# Compile .aar
echo "**********Compiling into AAR**********"
gradle publishToMavenLocal
if [[ $? -ne 0 ]]
then
    echo "FAILURE in publishing to Maven Local"
    exit 1
fi

echo "Successfully built webgpu aar"

# Rename .aar to Chromium branch name
mv webgpu/build/outputs/aar/webgpu-release.aar webgpu/build/outputs/aar/$KOKORO_GOB_BRANCH.aar
