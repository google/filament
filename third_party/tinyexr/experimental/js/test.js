var fs = require('fs');
var Module = require('./tinyexr.js')


Module.onRuntimeInitialized = async function(){

  var data = new Uint8Array(fs.readFileSync("../../asakusa.exr"))

  console.log('Module loaded: ', Module);

  var instance = new Module.EXRLoader(data);
  console.log(instance.ok())
  console.log(instance.width())
  console.log(instance.height())

  var image = instance.getBytes()
}
