# Copyright 2017 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

NDK_HOME ?= $(HOME)/tools/android-ndk-r13b

all: build

build:
	$(NDK_HOME)/ndk-build

install: build
	adb push libs/armeabi-v7a/atrace_helper /data/local/tmp/atrace_helper

test: install
	adb shell /data/local/tmp/atrace_helper -c 1

testf: install
	adb shell /data/local/tmp/atrace_helper -c 1 -m system_server -g -s -o /data/local/tmp/dump.json
	adb pull /data/local/tmp/dump.json /tmp/dump.json
	python -c 'import json; json.load(open("/tmp/dump.json"))'

benchmark: install
	adb shell "time /data/local/tmp/atrace_helper -c 1 -m -o /dev/null"
