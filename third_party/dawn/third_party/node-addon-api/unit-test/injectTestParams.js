const fs = require('fs');
const path = require('path');

const listOfTestModules = require('./listOfTestModules');

const buildDirs = listOfTestModules.dirs;
const buildFiles = listOfTestModules.files;

if (!fs.existsSync('./generated')) {
  fs.mkdirSync('./generated');
}

/**
 *  @returns : list of files to compile by node-gyp
 *  @param : none
 *  @requires : picks `filter` parameter from process.env
 *  This function is used as an utility method to inject a list of files to compile into binding.gyp
 */
module.exports.filesToCompile = function () {
  // match filter argument with available test modules
  const matchedModules = require('./matchModules').matchWildCards(process.env.npm_config_filter || '');

  // standard list of files to compile
  const addedFiles = './generated/binding.cc test_helper.h';

  const filterConditions = matchedModules.split(' ').length ? matchedModules.split(' ') : [matchedModules];
  const files = [];

  // generate a list of all files to compile
  for (const matchCondition of filterConditions) {
    if (buildDirs[matchCondition.toLowerCase()]) {
      for (const file of buildDirs[matchCondition.toLowerCase()]) {
        const config = buildFiles[file];
        const separator = config.dir.length ? '/' : '';
        files.push(config.dir + separator + file);
      }
    } else if (buildFiles[matchCondition.toLowerCase()]) {
      const config = buildFiles[matchCondition.toLowerCase()];
      const separator = config.dir.length ? '/' : '';
      files.push(config.dir + separator + matchCondition.toLowerCase());
    }
  }

  // generate a string of files to feed to the compiler
  let filesToCompile = '';
  files.forEach((file) => {
    filesToCompile = `${filesToCompile} ../test/${file}.cc`;
  });

  // log list of compiled files
  fs.writeFileSync(path.join(__dirname, '/generated/compilelist'), `${addedFiles} ${filesToCompile}`.split(' ').join('\r\n'));

  // return file list
  return `${addedFiles} ${filesToCompile}`;
};

/**
 * @returns list of test files to bind exported init functions
 * @param : none
 * @requires : picks `filter` parameter from process.env
 * This function is used as an utility method by the generateBindingCC step in binding.gyp
 */
module.exports.filesForBinding = function () {
  const filterCondition = require('./matchModules').matchWildCards(process.env.npm_config_filter || '');
  fs.writeFileSync(path.join(__dirname, '/generated/bindingList'), filterCondition.split(' ').join('\r\n'));
  return filterCondition;
};

/**
 * Test cases
 * @fires only when run directly from terminal
 * eg: node injectTestParams
 */
if (require.main === module) {
  const assert = require('assert');

  const setEnvAndCall = (fn, filterCondition) => { process.env.npm_config_filter = filterCondition; return fn(); };

  assert.strictEqual(setEnvAndCall(exports.filesToCompile, 'typed*ex*'), './generated/binding.cc test_helper.h  ../test/typed_threadsafe_function/typed_threadsafe_function_existing_tsfn.cc');

  const expectedFilesToMatch = [
    './generated/binding.cc test_helper.h ',
    '../test/threadsafe_function/threadsafe_function.cc',
    '../test/threadsafe_function/threadsafe_function_ctx.cc',
    '../test/threadsafe_function/threadsafe_function_existing_tsfn.cc',
    '../test/threadsafe_function/threadsafe_function_ptr.cc',
    '../test/threadsafe_function/threadsafe_function_sum.cc',
    '../test/threadsafe_function/threadsafe_function_unref.cc',
    '../test/typed_threadsafe_function/typed_threadsafe_function.cc',
    '../test/typed_threadsafe_function/typed_threadsafe_function_ctx.cc',
    '../test/typed_threadsafe_function/typed_threadsafe_function_existing_tsfn.cc',
    '../test/typed_threadsafe_function/typed_threadsafe_function_ptr.cc',
    '../test/typed_threadsafe_function/typed_threadsafe_function_sum.cc',
    '../test/typed_threadsafe_function/typed_threadsafe_function_unref.cc'
  ];
  assert.strictEqual(setEnvAndCall(exports.filesToCompile, 'threadsafe_function typed_threadsafe_function'), expectedFilesToMatch.join(' '));

  assert.strictEqual(setEnvAndCall(exports.filesToCompile, 'objectwrap'), './generated/binding.cc test_helper.h  ../test/objectwrap.cc');

  console.log('ALL tests passed');
}
