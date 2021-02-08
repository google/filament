// This is a minimal example showing how to create the Draco decoder and
// encoder module. The modules are created asynchronously, so you need to set
// callbacks to make sure they are initialized before you try and call the
// modules.

'use_strict';

const draco3d = require('./draco3d');

// Global decoder and encoder module variables.
let decoderModule = null;
let encoderModule = null;

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

// Create and set the encoder module.
draco3d.createEncoderModule({}).then(function(module) {
  // This is reached when everything is ready, and you can call methods on
  // Module.
  encoderModule = module;
  console.log('Encoder Module Initialized!');
  moduleInitialized();
});

function moduleInitialized() {
  if (encoderModule && decoderModule) {
    console.log('Both Modules Initialized!');
    let encoder = new encoderModule.Encoder();
    let decoder = new decoderModule.Decoder();
    // Do the actual encoding and decoding here. See 'draco_nodejs_example.js'
    // for a more comprehensive example.
    cleanup(encoder, decoder);
  }
}

function cleanup(encoder, decoder) {
  encoderModule.destroy(encoder);
  decoderModule.destroy(decoder);
}
