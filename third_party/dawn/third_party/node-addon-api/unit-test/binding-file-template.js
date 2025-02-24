const path = require('path');
const fs = require('fs');

/**
 * @param bindingConfigurations
 * This method acts as a template to generate the content of binding.cc file
 */
module.exports.generateFileContent = function (bindingConfigurations) {
  const content = [];
  const inits = [];
  const exports = [];

  for (const config of bindingConfigurations) {
    inits.push(`Object Init${config.objectName}(Env env);`);
    exports.push(`exports.Set("${config.propertyName}", Init${config.objectName}(env));`);
  }

  content.push('#include "napi.h"');
  content.push('using namespace Napi;');

  inits.forEach(init => content.push(init));

  content.push('Object Init(Env env, Object exports) {');

  exports.forEach(exp => content.push(exp));

  content.push('return exports;');
  content.push('}');
  content.push('NODE_API_MODULE(addon, Init);');

  return Promise.resolve(content.join('\r\n'));
};

module.exports.writeToBindingFile = function writeToBindingFile (content) {
  const generatedFilePath = path.join(__dirname, 'generated', 'binding.cc');
  fs.writeFileSync(generatedFilePath, '');
  fs.writeFileSync(generatedFilePath, content, { flag: 'a' });
  console.log('generated binding file ', generatedFilePath, new Date());
};
