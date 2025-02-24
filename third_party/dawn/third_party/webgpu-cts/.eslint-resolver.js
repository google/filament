const path = require('path');
const resolve = require('resolve'); // eslint-disable-line node/no-extraneous-require

// Implements the following resolver spec:
// https://github.com/benmosher/eslint-plugin-import/blob/master/resolvers/README.md
exports.interfaceVersion = 2;

exports.resolve = function (source, file, config) {
  if (resolve.isCore(source)) return { found: true, path: null };

  source = source.replace(/\.js$/, '.ts');
  try {
    return {
      found: true,
      path: resolve.sync(source, {
        extensions: [],
        basedir: path.dirname(path.resolve(file)),
        ...config,
      }),
    };
  } catch (err) {
    return { found: false };
  }
};
