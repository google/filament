// This is a minimal example showing how to create the Draco decoder module.
// The decoder module is created asynchronously, so you need to set a
// callback to make sure it is initialized before you try and call the module.

'use_strict';

const draco3d = require('./draco3d');

let decoderModule = null;

// The code to create the encoder and decoder modules is asynchronous.
// draco3d.createDecoderModule will return a promise to a funciton with a
// module as a parameter when the module has been fully initialized.
// Create and set the decoder module.
draco3d.createDecoderModule({}).then(function(module) {
  // This is reached when everything is ready, and you can call methods on
  // Module.
  decoderModule = module;
  console.log('Decoder Module Initialized!');
  moduleInitialized();
});

function moduleInitialized() {
  let decoder = new decoderModule.Decoder();
  // Do the actual decoding here. See 'draco_nodejs_example.js' for a more
  // comprehensive example.
  cleanup(decoder);
}

function cleanup(decoder) {
  decoderModule.destroy(decoder);
}
