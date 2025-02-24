@echo off
:: Copyright (c) 2018 The Chromium Authors. All rights reserved.
:: Use of this source code is governed by a BSD-style license that can be
:: found in the LICENSE file.
setlocal

:: Defer control.
python3 "%~dp0fake_cipd.py" %*
