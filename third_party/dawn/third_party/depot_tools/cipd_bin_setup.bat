@echo off
:: Copyright 2017 The Chromium Authors. All rights reserved.
:: Use of this source code is governed by a BSD-style license that can be
:: found in the LICENSE file.

"%~dp0\cipd.bat" ensure -log-level warning -ensure-file "%~dp0\cipd_manifest.txt" -root "%~dp0\.cipd_bin"
