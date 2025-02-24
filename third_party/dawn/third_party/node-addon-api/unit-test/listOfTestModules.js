const fs = require('fs');
const path = require('path');
const exceptions = require('./exceptions');

const buildFiles = {};
const buidDirs = {};

/**
 * @param fileName - expect to be in snake case , eg: this_is_a_test_file.cc
 * @returns init function name in the file
 *
 * general format of init function name is camelCase version of the snake_case file name
 */
function getExportObjectName (fileName) {
  fileName = fileName.split('_').map(token => exceptions.nouns[token] ? exceptions.nouns[token] : token).join('_');
  const str = fileName.replace(/(_\w)/g, (k) => k[1].toUpperCase());
  const exportObjectName = str.charAt(0).toUpperCase() + str.substring(1);
  if (exceptions.exportNames[exportObjectName]) {
    return exceptions.exportNames[exportObjectName];
  }
  return exportObjectName;
}

/**
 * @param fileName - expect to be in snake case , eg: this_is_a_test_file.cc
 * @returns property name of exported init function
 */
function getExportPropertyName (fileName) {
  if (exceptions.propertyNames[fileName.toLowerCase()]) {
    return exceptions.propertyNames[fileName.toLowerCase()];
  }
  return fileName;
}

/**
 * creates a configuration list for all available test modules
 * The configuration object contains the expected init function names and corresponding export property names
 */
function listOfTestModules (currentDirectory = path.join(__dirname, '/../test'), pre = '') {
  fs.readdirSync(currentDirectory).forEach((file) => {
    if (file === 'binding.cc' ||
      file === 'binding.gyp' ||
      file === 'build' ||
      file === 'common' ||
      file === 'thunking_manual.cc' ||
      file === 'addon_build' ||
      file[0] === '.') {
      return;
    }
    const absoluteFilepath = path.join(currentDirectory, file);
    const fileName = file.toLowerCase().replace('.cc', '');
    if (fs.statSync(absoluteFilepath).isDirectory()) {
      buidDirs[fileName] = [];
      listOfTestModules(absoluteFilepath, pre + file + '/');
    } else {
      if (!file.toLowerCase().endsWith('.cc')) return;
      if (currentDirectory.trim().split('/test/').length > 1) {
        buidDirs[currentDirectory.split('/test/')[1].toLowerCase()].push(fileName);
      }
      const relativePath = (currentDirectory.split(`${fileName}.cc`)[0]).split('/test/')[1] || '';
      buildFiles[fileName] = { dir: relativePath, propertyName: getExportPropertyName(fileName), objectName: getExportObjectName(fileName) };
    }
  });
}
listOfTestModules();

module.exports = {
  dirs: buidDirs,
  files: buildFiles
};

/**
 * Test cases
 * @fires only when run directly from terminal
 * eg: node listOfTestModules
 */
if (require.main === module) {
  const assert = require('assert');
  assert.strictEqual(getExportObjectName('objectwrap_constructor_exception'), 'ObjectWrapConstructorException');
  assert.strictEqual(getExportObjectName('typed_threadsafe_function'), 'TypedThreadSafeFunction');
  assert.strictEqual(getExportObjectName('objectwrap_removewrap'), 'ObjectWrapRemovewrap');
  assert.strictEqual(getExportObjectName('function_reference'), 'FunctionReference');
  assert.strictEqual(getExportObjectName('async_worker'), 'AsyncWorker');
  assert.strictEqual(getExportObjectName('async_progress_worker'), 'AsyncProgressWorker');
  assert.strictEqual(getExportObjectName('async_worker_persistent'), 'PersistentAsyncWorker');

  console.log('ALL tests passed');
}
