const listOfTestModules = require('./listOfTestModules');
const exceptions = require('./exceptions');
const { generateFileContent, writeToBindingFile } = require('./binding-file-template');

const buildDirs = listOfTestModules.dirs;
const buildFiles = listOfTestModules.files;

/**
 * @param none
 * @requires list of files to bind as command-line argument
 * @returns list of binding configurations
 */
function generateBindingConfigurations () {
  const testFilesToBind = process.argv.slice(2);
  console.log('test modules to bind: ', testFilesToBind);

  const configs = [];

  testFilesToBind.forEach((file) => {
    const configName = file.split('.cc')[0];

    if (buildDirs[configName]) {
      for (const file of buildDirs[configName]) {
        if (exceptions.skipBinding.includes(file)) continue;
        configs.push(buildFiles[file]);
      }
    } else if (buildFiles[configName]) {
      configs.push(buildFiles[configName]);
    } else {
      console.log('not found', file, configName);
    }
  });

  return Promise.resolve(configs);
}

generateBindingConfigurations().then(generateFileContent).then(writeToBindingFile);

/**
 * Test cases
 * @fires only when run directly from terminal with TEST=true
 * eg: TEST=true node generate-binding-cc
 */
if (require.main === module && process.env.TEST === 'true') {
  const assert = require('assert');

  const setArgsAndCall = (fn, filterCondition) => { process.argv = [null, null, ...filterCondition.split(' ')]; return fn(); };
  const assertPromise = (promise, expectedVal) => promise.then((val) => assert.deepEqual(val, expectedVal)).catch(console.log);

  const expectedVal = [{
    dir: '',
    objectName: 'AsyncProgressWorker',
    propertyName: 'async_progress_worker'
  },
  {
    dir: '',
    objectName: 'PersistentAsyncWorker',
    propertyName: 'persistentasyncworker'
  }];
  assertPromise(setArgsAndCall(generateBindingConfigurations, 'async_progress_worker async_worker_persistent'), expectedVal);
}
