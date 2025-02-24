const fs = require('fs');
const { generateFileContent, writeToBindingFile } = require('./binding-file-template');

/**
 * @summary setup script to execute before node-gyp begins target actions
 */
if (!fs.existsSync('./generated')) {
  // create generated folder
  fs.mkdirSync('./generated');
  // create empty binding.cc file
  generateFileContent([]).then(writeToBindingFile);
  // FIX: Its necessary to have an empty bindng.cc file, otherwise build fails first time
}
