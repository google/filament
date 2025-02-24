const path = require('path');

// Automatically transpile .ts imports
require('ts-node').register({
  // Specify the project file so ts-node doesn't try to find it itself based on the CWD.
  project: path.resolve(__dirname, '../../../tsconfig.json'),
  compilerOptions: {
    module: 'commonjs',
  },
  transpileOnly: true,
});
const Module = require('module');

// Redirect imports of .js files to .ts files
const resolveFilename = Module._resolveFilename;
Module._resolveFilename = (request, parentModule, isMain) => {
  do {
    if (request.startsWith('.') && parentModule.filename.endsWith('.ts')) {
      // Required for browser (because it needs the actual correct file path and
      // can't do any kind of file resolution).
      if (request.endsWith('/index.js')) {
        throw new Error(
          "Avoid the name `index.js`; we don't have Node-style path resolution: " + request
        );
      }

      // Import of Node addon modules are valid and should pass through.
      if (request.endsWith('.node')) {
        break;
      }

      if (!request.endsWith('.js')) {
        throw new Error('All relative imports must end in .js: ' + request);
      }

      try {
        const tsRequest = request.substring(0, request.length - '.js'.length) + '.ts';
        return resolveFilename.call(this, tsRequest, parentModule, isMain);
      } catch (ex) {
        // If the .ts file doesn't exist, try .js instead.
        break;
      }
    }
  } while (0);

  return resolveFilename.call(this, request, parentModule, isMain);
};

process.on('unhandledRejection', ex => {
  throw ex;
});
