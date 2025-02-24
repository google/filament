'use strict';
const path = require('path');
const { Worker, isMainThread } = require('worker_threads');
const { runTestWithBuildType, whichBuildType } = require('./common');

module.exports = runTestWithBuildType(test);

async function test () {
  if (isMainThread) {
    const buildType = await whichBuildType();
    const worker = new Worker(__filename, { workerData: buildType });
    return new Promise((resolve, reject) => {
      worker.on('exit', () => {
        resolve();
      });
    }, () => {});
  } else {
    await require(path.join(__dirname, 'objectwrap.js'));
  }
}
