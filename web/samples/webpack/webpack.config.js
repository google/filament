/*
 * Copyright (C) 2018 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

const path = require('path');
const CopyWebpackPlugin = require('copy-webpack-plugin');

'use strict';

module.exports = {
    devtool: 'source-map',
    entry: './src/app.ts',

    // We do not actually use the following modules, but emscripten emits JS bindings that
    // conditionally uses them. Therefore we need to tell webpack to skip over their "require"
    // statements.
    externals: {
        fs: 'fs',
        crypto: 'crypto',
        path: 'path'
    },

    output:  {
        path: path.resolve(__dirname, 'public')
    },
    module: {
        rules: [
            {
                test: /\.tsx?$/,
                loader: 'ts-loader'
            }
        ]
    },
    resolve: {
        extensions: [ '.ts', '.tsx', '.js' ]
    },
    plugins: [
        new CopyWebpackPlugin([
            { from: 'node_modules/filament/filament.wasm' },
            { from: 'src/index.html' }
        ])
    ],
    performance: {
        assetFilter: function(assetFilename) {
            return assetFilename.endsWith('.js');
        }
    }
};
