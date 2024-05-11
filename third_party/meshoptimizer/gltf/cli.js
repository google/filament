#!/usr/bin/env node
// This file is part of gltfpack and is distributed under the terms of MIT License.
var gltfpack = require('./library.js');

var fs = require('fs');

var args = process.argv.slice(2);

var interface = {
	read: function (path) {
		return fs.readFileSync(path);
	},
	write: function (path, data) {
		fs.writeFileSync(path, data);
	},
};

gltfpack.pack(args, interface)
	.then(function (log) {
		process.stdout.write(log);
		process.exit(0);
	})
	.catch(function (err) {
		process.stderr.write(err.message);
		process.exit(1);
	});
