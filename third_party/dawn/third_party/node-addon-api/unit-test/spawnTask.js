const { spawn } = require('child_process');

/**
 *  spawns a child process to run a given node.js script
 */
module.exports.runChildProcess = function (scriptName, options) {
  const childProcess = spawn('node', [scriptName], options);

  childProcess.stdout.on('data', data => {
    console.log(`${data}`);
  });
  childProcess.stderr.on('data', data => {
    console.log(`error: ${data}`);
  });

  return new Promise((resolve, reject) => {
    childProcess.on('error', (error) => {
      console.log(`error: ${error.message}`);
      reject(error);
    });
    childProcess.on('close', code => {
      console.log(`child process exited with code ${code}`);
      resolve(code);
    });
  });
};
