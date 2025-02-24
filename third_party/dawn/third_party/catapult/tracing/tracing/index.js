// Copyright (c) 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
'use strict';

const fs = require('fs');
const path = require('path');

const catapultPath = fs.realpathSync(path.join(__dirname, '..', '..'));
const catapultBuildPath = path.join(catapultPath, 'catapult_build');

const nodeBootstrap = require(path.join(
    catapultBuildPath, 'node_bootstrap.js'));

HTMLImportsLoader.addArrayToSourcePath(
    nodeBootstrap.getSourcePathsForProject('tracing'));

// Go!
HTMLImportsLoader.loadHTML('/tracing/importer/import.html');
HTMLImportsLoader.loadHTML('/tracing/model/model.html');
HTMLImportsLoader.loadHTML('/tracing/extras/full_config.html');

// Make the tracing namespace the main tracing export.
module.exports = global.tr;
