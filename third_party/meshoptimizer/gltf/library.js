// This file is part of gltfpack and is distributed under the terms of MIT License.

/**
 * Initialize the library with the Wasm module (library.wasm)
 *
 * @param wasm Promise with contents of library.wasm
 *
 * Note: this is called automatically in node.js
 */
function init(wasm) {
	if (ready) {
		throw new Error("init must be called once");
	}

	ready = Promise.resolve(wasm)
		.then(function (buffer) {
			return WebAssembly.instantiate(buffer, { wasi_snapshot_preview1: wasi });
		})
		.then(function (result) {
			instance = result.instance;
			instance.exports.__wasm_call_ctors();
		});
}

/**
 * Pack the requested glTF data using the requested command line and access interface.
 *
 * @param args An array of strings with the input arguments; the paths for input and output files are interpreted by the interface
 * @param iface An interface to the system that will be used to service file requests and other system calls
 * @return Promise that indicates completion of the operation
 *
 * iface should contain the following methods:
 * read(path): Given a path, return a Uint8Array with the contents of that path
 * write(path, data): Write the specified Uint8Array to the provided path
 */
function pack(args, iface) {
	if (!ready) {
		throw new Error("init must be called before pack");
	}

	var argv = args.slice();
	argv.unshift("gltfpack");

	return ready.then(function () {
		var buf = uploadArgv(argv);

		output.position = 0;
		output.size = 0;

		interface = iface;
		var result = instance.exports.pack(argv.length, buf);
		interface = undefined;

		instance.exports.free(buf);

		var log = getString(output.data.buffer, 0, output.size);

		if (result != 0) {
			throw new Error(log);
		} else {
			return log;
		}
	});
}

// Library implementation (here be dragons)
var WASI_EBADF = 8;
var WASI_EINVAL = 28;
var WASI_EIO = 29;
var WASI_ENOSYS = 52;

var ready;
var instance;
var interface;

var output = { data: new Uint8Array(), position: 0, size: 0 };
var fds = { 1: output, 2: output, 3: { mount: "/", path: "/" }, 4: { mount: "/gltfpack-$pwd", path: "" } };

var wasi = {
	proc_exit: function(rval) {
	},

	fd_close: function(fd) {
		if (!fds[fd]) {
			return WASI_EBADF;
		}

		try {
			if (fds[fd].close) {
				fds[fd].close();
			}
			fds[fd] = undefined;
			return 0;
		} catch (err) {
			fds[fd] = undefined;
			return WASI_EIO;
		}
	},

	fd_fdstat_get: function(fd, stat) {
		if (!fds[fd]) {
			return WASI_EBADF;
		}

		var heap = getHeap();
		heap.setUint8(stat + 0, fds[fd].path !== undefined ? 3 : 4);
		heap.setUint16(stat + 2, 0, true);
		heap.setUint32(stat + 8, 0, true);
		heap.setUint32(stat + 12, 0, true);
		heap.setUint32(stat + 16, 0, true);
		heap.setUint32(stat + 20, 0, true);
		return 0;
	},

	path_open32: function(parent_fd, dirflags, path, path_len, oflags, fs_rights_base, fs_rights_inheriting, fdflags, opened_fd) {
		if (!fds[parent_fd] || fds[parent_fd].path === undefined) {
			return WASI_EBADF;
		}

		var heap = getHeap();

		var file = {};
		file.name = fds[parent_fd].path + getString(heap.buffer, path, path_len);
		file.position = 0;

		if (oflags & 1) {
			file.data = new Uint8Array(4096);
			file.size = 0;
			file.close = function () {
				interface.write(file.name, new Uint8Array(file.data.buffer, 0, file.size));
			};
		} else {
			try {
				file.data = interface.read(file.name);

				if (!file.data) {
					return WASI_EIO;
				}

				file.size = file.data.length;
			} catch (err) {
				return WASI_EIO;
			}
		}

		var fd = nextFd();
		fds[fd] = file;

		heap.setUint32(opened_fd, fd, true);
		return 0;
	},

	path_filestat_get: function(parent_fd, flags, path, path_len, buf) {
		if (!fds[parent_fd] || fds[parent_fd].path === undefined) {
			return WASI_EBADF;
		}

		var heap = getHeap();
		var name = getString(heap.buffer, path, path_len);

		var heap = getHeap();
		for (var i = 0; i < 64; ++i)
			heap.setUint8(buf + i, 0);

		heap.setUint8(buf + 16, name == "." ? 3 : 4);
		return 0;
	},

	fd_prestat_get: function(fd, buf) {
		if (!fds[fd] || fds[fd].path === undefined) {
			return WASI_EBADF;
		}

		var path_buf = stringBuffer(fds[fd].mount);

		var heap = getHeap();
		heap.setUint8(buf, 0);
		heap.setUint32(buf + 4, path_buf.length, true);
		return 0;
	},

	fd_prestat_dir_name: function(fd, path, path_len) {
		if (!fds[fd] || fds[fd].path === undefined) {
			return WASI_EBADF;
		}

		var path_buf = stringBuffer(fds[fd].mount);

		if (path_len != path_buf.length) {
			return WASI_EINVAL;
		}

		var heap = getHeap();
		new Uint8Array(heap.buffer).set(path_buf, path);
		return 0;
	},

	path_remove_directory: function(parent_fd, path, path_len) {
		return WASI_EINVAL;
	},

	fd_fdstat_set_flags: function(fd, flags) {
		return WASI_ENOSYS;
	},

	fd_seek32: function(fd, offset, whence, newoffset) {
		if (!fds[fd]) {
			return WASI_EBADF;
		}

		var newposition;

		switch (whence) {
		case 0:
			newposition = offset;
			break;

		case 1:
			newposition = fds[fd].position + offset;
			break;

		case 2:
			newposition = fds[fd].size;
			break;

		default:
			return WASI_EINVAL;
		}

		if (newposition > fds[fd].size) {
			return WASI_EINVAL;
		}

		fds[fd].position = newposition;

		var heap = getHeap();
		heap.setUint32(newoffset, newposition, true);
		return 0;
	},

	fd_read: function(fd, iovs, iovs_len, nread) {
		if (!fds[fd]) {
			return WASI_EBADF;
		}

		var heap = getHeap();
		var read = 0;

		for (var i = 0; i < iovs_len; ++i) {
			var buf = heap.getUint32(iovs + 8 * i + 0, true);
			var buf_len = heap.getUint32(iovs + 8 * i + 4, true);

			var readi = Math.min(fds[fd].size - fds[fd].position, buf_len);

			new Uint8Array(heap.buffer).set(fds[fd].data.subarray(fds[fd].position, fds[fd].position + readi), buf);

			fds[fd].position += readi;
			read += readi;
		}

		heap.setUint32(nread, read, true);
		return 0;
	},

	fd_write: function(fd, iovs, iovs_len, nwritten) {
		if (!fds[fd]) {
			return WASI_EBADF;
		}

		var heap = getHeap();
		var written = 0;

		for (var i = 0; i < iovs_len; ++i) {
			var buf = heap.getUint32(iovs + 8 * i + 0, true);
			var buf_len = heap.getUint32(iovs + 8 * i + 4, true);

			if (fds[fd].position + buf_len > fds[fd].data.length) {
				fds[fd].data = growArray(fds[fd].data, fds[fd].position + buf_len);
			}

			fds[fd].data.set(new Uint8Array(heap.buffer, buf, buf_len), fds[fd].position);
			fds[fd].position += buf_len;
			fds[fd].size = Math.max(fds[fd].position, fds[fd].size);

			written += buf_len;
		}

		heap.setUint32(nwritten, written, true);
		return 0;
	},
};

function nextFd() {
	for (var i = 1; ; ++i) {
		if (fds[i] === undefined) {
			return i;
		}
	}
}

function getHeap() {
	return new DataView(instance.exports.memory.buffer);
}

function getString(buffer, offset, length) {
	return new TextDecoder().decode(new Uint8Array(buffer, offset, length));
}

function stringBuffer(string) {
	return new TextEncoder().encode(string);
}

function growArray(data, len) {
	var new_length = Math.max(1, data.length);
	while (new_length < len) {
		new_length *= 2;
	}

	var new_data = new Uint8Array(new_length);
	new_data.set(data);

	return new_data;
}

function uploadArgv(argv) {
	var buf_size = argv.length * 4;
	for (var i = 0; i < argv.length; ++i) {
		buf_size += stringBuffer(argv[i]).length + 1;
	}

	var buf = instance.exports.malloc(buf_size);
	var argp = buf + argv.length * 4;

	var heap = getHeap();

	for (var i = 0; i < argv.length; ++i) {
		var item = stringBuffer(argv[i]);

		heap.setUint32(buf + i * 4, argp, true);
		new Uint8Array(heap.buffer).set(item, argp);
		heap.setUint8(argp + item.length, 0);

		argp += item.length + 1;
	}

	return buf;
}

// Automatic initialization for node.js
if (typeof window === 'undefined' && typeof process !== 'undefined' && process.release.name === 'node') {
	var fs = require('fs');
	var util = require('util');

	// Node versions before v12 don't support TextEncoder/TextDecoder natively, but util. provides compatible replacements
	if (typeof TextEncoder === 'undefined' && typeof TextDecoder === 'undefined') {
		TextEncoder = util.TextEncoder;
		TextDecoder = util.TextDecoder;
	}

	init(fs.readFileSync(__dirname + '/library.wasm'));
}

// UMD
(function (root, factory) {
    if (typeof define === 'function' && define.amd) {
        define([], factory);
    } else if (typeof module === 'object' && module.exports) {
        module.exports = factory();
    } else {
        root.gltfpack = factory();
  }
}(typeof self !== 'undefined' ? self : this, function () {
    return { init, pack };
}));
