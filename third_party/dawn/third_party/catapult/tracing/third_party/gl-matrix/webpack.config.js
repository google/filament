/* Copyright (c) 2015, Brandon Jones, Colin MacKenzie IV.

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE. */

var fs = require('fs');
var webpack = require('webpack');

var entryFile = './src/gl-matrix.js';

// Read the comments from the top of the main gl-matrix file and append them to
// the minified version.
var header = '';
var mainFile = fs.readFileSync(entryFile, { encoding: 'utf8' });
if (mainFile) {
  var headerIndex = mainFile.indexOf('\/\/ END HEADER');
  if (headerIndex >= 0) {
    header = mainFile.substr(0, headerIndex);
  }
}

module.exports = {
  entry: entryFile,
  output: {
    path: __dirname + '/dist',
    filename: 'gl-matrix.js',
    libraryTarget: 'umd'
  },
  plugins: [
    new webpack.BannerPlugin(header, { raw: true }),
  ]
};