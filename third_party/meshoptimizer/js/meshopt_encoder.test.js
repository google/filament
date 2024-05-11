var assert = require('assert').strict;
var encoder = require('./meshopt_encoder.js');
var decoder = require('./meshopt_decoder.js');

process.on('unhandledRejection', error => {
	console.log('unhandledRejection', error);
	process.exit(1);
});

function bytes(view) {
	return new Uint8Array(view.buffer, view.byteOffset, view.byteLength);
}

var tests = {
	reorderMesh: function() {
		var indices = new Uint32Array([
			4, 2, 5,
			3, 1, 4,
			0, 1, 3,
			1, 2, 4,
		]);

		var expected = new Uint32Array([
			0, 1, 2,
			3, 1, 0,
			4, 3, 0,
			5, 3, 4,
		]);

		var remap = new Uint32Array([
			5, 3, 1, 4, 0, 2
		]);

		var res = encoder.reorderMesh(indices, /* triangles= */ true, /* optsize= */ true);
		assert.deepEqual(indices, expected);
		assert.deepEqual(res[0], remap);
		assert.equal(res[1], 6); // unique
	},

	roundtripVertexBuffer: function() {
		var data = new Uint8Array(16 * 4);

		// this tests 0/2/4/8 bit groups in one stream
		for (var i = 0; i < 16; ++i)
		{
			data[i * 4 + 0] = 0;
			data[i * 4 + 1] = i * 1;
			data[i * 4 + 2] = i * 2;
			data[i * 4 + 3] = i * 8;
		}

		var encoded = encoder.encodeVertexBuffer(data, 16, 4);
		var decoded = new Uint8Array(16 * 4);
		decoder.decodeVertexBuffer(decoded, 16, 4, encoded);

		assert.deepEqual(decoded, data);
	},

	roundtripIndexBuffer: function() {
		var data = new Uint32Array([0, 1, 2, 2, 1, 3, 4, 6, 5, 7, 8, 9]);

		var encoded = encoder.encodeIndexBuffer(bytes(data), data.length, 4);
		var decoded = new Uint32Array(data.length);
		decoder.decodeIndexBuffer(bytes(decoded), data.length, 4, encoded);

		assert.deepEqual(decoded, data);
	},

	roundtripIndexBuffer16: function() {
		var data = new Uint16Array([0, 1, 2, 2, 1, 3, 4, 6, 5, 7, 8, 9]);

		var encoded = encoder.encodeIndexBuffer(bytes(data), data.length, 2);
		var decoded = new Uint16Array(data.length);
		decoder.decodeIndexBuffer(bytes(decoded), data.length, 2, encoded);

		assert.deepEqual(decoded, data);
	},

	roundtripIndexSequence: function() {
		var data = new Uint32Array([0, 1, 51, 2, 49, 1000]);

		var encoded = encoder.encodeIndexSequence(bytes(data), data.length, 4);
		var decoded = new Uint32Array(data.length);
		decoder.decodeIndexSequence(bytes(decoded), data.length, 4, encoded);

		assert.deepEqual(decoded, data);
	},

	roundtripIndexSequence16: function() {
		var data = new Uint16Array([0, 1, 51, 2, 49, 1000]);

		var encoded = encoder.encodeIndexSequence(bytes(data), data.length, 2);
		var decoded = new Uint16Array(data.length);
		decoder.decodeIndexSequence(bytes(decoded), data.length, 2, encoded);

		assert.deepEqual(decoded, data);
	},

	encodeFilterOct8: function() {
		var data = new Float32Array([
		    1, 0, 0, 0,
		    0, -1, 0, 0,
		    0.7071068, 0, 0.707168, 1,
		    -0.7071068, 0, -0.707168, 1,
		]);

		var expected = new Uint8Array([
		    0x7f, 0, 0x7f, 0,
		    0, 0x81, 0x7f, 0,
		    0x3f, 0, 0x7f, 0x7f,
		    0x81, 0x40, 0x7f, 0x7f,
		]);

		// 4 vectors, encode each vector into 4 bytes with 8 bits of precision/component
		var encoded = encoder.encodeFilterOct(data, 4, 4, 8);
		assert.deepEqual(encoded, expected);
	},

	encodeFilterOct12: function() {
		var data = new Float32Array([
		    1, 0, 0, 0,
		    0, -1, 0, 0,
		    0.7071068, 0, 0.707168, 1,
		    -0.7071068, 0, -0.707168, 1,
		]);

		var expected = new Uint16Array([
		    0x7ff, 0, 0x7ff, 0,
		    0x0, 0xf801, 0x7ff, 0,
		    0x3ff, 0, 0x7ff, 0x7fff,
		    0xf801, 0x400, 0x7ff, 0x7fff,
		]);

		// 4 vectors, encode each vector into 8 bytes with 12 bits of precision/component
		var encoded = encoder.encodeFilterOct(data, 4, 8, 12);
		assert.deepEqual(encoded, bytes(expected));
	},

	encodeFilterQuat12: function() {
		var data = new Float32Array([
		    1, 0, 0, 0,
		    0, -1, 0, 0,
		    0.7071068, 0, 0, 0.707168,
		    -0.7071068, 0, 0, -0.707168,
		]);

		var expected = new Uint16Array([
		    0, 0, 0, 0x7fc,
		    0, 0, 0, 0x7fd,
		    0x7ff, 0, 0, 0x7ff,
		    0x7ff, 0, 0, 0x7ff,
		]);

		// 4 quaternions, encode each quaternion into 8 bytes with 12 bits of precision/component
		var encoded = encoder.encodeFilterQuat(data, 4, 8, 12);
		assert.deepEqual(encoded, bytes(expected));
	},

	encodeFilterExp: function() {
		var data = new Float32Array([
		    1,
		    -23.4,
		    -0.1,
		]);

		var expected = new Uint32Array([
		    0xf7000200,
		    0xf7ffd133,
		    0xf7ffffcd,
		]);

		// 1 vector with 3 components (12 bytes), encode each vector into 12 bytes with 15 bits of precision/component
		var encoded = encoder.encodeFilterExp(data, 1, 12, 15);
		assert.deepEqual(encoded, bytes(expected));
	},

	encodeGltfBuffer: function() {
		var data = new Uint32Array([0, 1, 2, 2, 1, 3, 4, 6, 5, 7, 8, 9]);

		var encoded = encoder.encodeGltfBuffer(bytes(data), data.length, 4, 'TRIANGLES');
		var decoded = new Uint32Array(data.length);
		decoder.decodeGltfBuffer(bytes(decoded), data.length, 4, encoded, 'TRIANGLES');

		assert.deepEqual(decoded, data);
	},
};

Promise.all([encoder.ready, decoder.ready]).then(() => {
	var count = 0;

	for (var key in tests) {
		tests[key]();
		count++;
	}

	console.log(count, 'tests passed');
});
