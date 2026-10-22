var assert = require('assert').strict;
var simplifier = require('./meshopt_simplifier.js');

process.on('unhandledRejection', error => {
	console.log('unhandledRejection', error);
	process.exit(1);
});

var tests = {
	compactMesh: function() {
		var indices = new Uint32Array([
			0, 1, 3,
			3, 1, 5,
		]);

		var expected = new Uint32Array([
			0, 1, 2,
			2, 1, 3,
		]);

		var missing = 2**32-1;

		var remap = new Uint32Array([
			0, 1, missing, 2, missing, 3
		]);

		var res = simplifier.compactMesh(indices);
		assert.deepEqual(indices, expected);
		assert.deepEqual(res[0], remap);
		assert.equal(res[1], 4); // unique
	},

	simplify: function() {
		// 0
		// 1 2
		// 3 4 5
		var indices = new Uint32Array([
			0, 2, 1,
			1, 2, 3,
			3, 2, 4,
			2, 5, 4,
		]);

		var positions = new Float32Array([
			0, 2, 0,
			0, 1, 0,
			1, 1, 0,
			0, 0, 0,
			1, 0, 0,
			2, 0, 0,
		]);

		var res = simplifier.simplify(indices, positions, 3, /* target indices */ 3, /* target error */ 0.01);

		var expected = new Uint32Array([
			3, 0, 5,
		]);

		assert.deepEqual(res[0], expected);
		assert.equal(res[1], 0); // error
	},

	simplify16: function() {
		// 0
		// 1 2
		// 3 4 5
		var indices = new Uint16Array([
			0, 2, 1,
			1, 2, 3,
			3, 2, 4,
			2, 5, 4,
		]);

		var positions = new Float32Array([
			0, 2, 0,
			0, 1, 0,
			1, 1, 0,
			0, 0, 0,
			1, 0, 0,
			2, 0, 0,
		]);

		var res = simplifier.simplify(indices, positions, 3, /* target indices */ 3, /* target error */ 0.01);

		var expected = new Uint16Array([
			3, 0, 5,
		]);

		assert.deepEqual(res[0], expected);
		assert.equal(res[1], 0); // error
	},

	simplifyLockBorder: function() {
		// 0
		// 1 2
		// 3 4 5
		var indices = new Uint32Array([
			0, 2, 1,
			1, 2, 3,
			3, 2, 4,
			2, 5, 4,
		]);

		var positions = new Float32Array([
			0, 2, 0,
			0, 1, 0,
			1, 1, 0,
			0, 0, 0,
			1, 0, 0,
			2, 0, 0,
		]);

		var res = simplifier.simplify(indices, positions, 3, /* target indices */ 3, /* target error */ 0.01, ["LockBorder"]);

		var expected = new Uint32Array([
			0, 2, 1,
			1, 2, 3,
			3, 2, 4,
			2, 5, 4,
		]);

		assert.deepEqual(res[0], expected);
		assert.equal(res[1], 0); // error
	},

	getScale: function() {
		var positions = new Float32Array([
			0, 0, 0, 1, 0, 0, 0, 2, 0, 0, 0, 3
		]);

		assert(simplifier.getScale(positions, 3) == 3.0);
	},
};

Promise.all([simplifier.ready]).then(() => {
	var count = 0;

	for (var key in tests) {
		tests[key]();
		count++;
	}

	console.log(count, 'tests passed');
});
