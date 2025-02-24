// Copyright (c) 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.


/**
 * Set global throw_error to true. This will be used to check whether to throw
 * Error in error.js file.
 */
this.throw_error = true;


HTMLImportsLoader.loadHTML('/load_simple_html.html');
maybeRaiseExceptionInFoo();
