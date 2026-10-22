// This is a minimal example showing how to create the Draco encoder module.
// The encoder module is created asynchronously, so you need to set a
// callback to make sure it is initialized before you try and call the module.

'use_strict';

const draco3d = require('./draco3d');

let encoderModule = null;

// The code to create the encoder module is asynchronous.
// draco3d.createEncoderModule will return a promise to a funciton with a
// module as a parameter when the module has been fully initialized.
draco3d.createEncoderModule({}).then(function(module) {
  // This is reached when everything is ready, and you can call methods on
  // Module.
  encoderModule = module;
  console.log('Encoder Module Initialized!');
  moduleInitialized();
});

function moduleInitialized() {
  let encoder = new encoderModule.Encoder();
  // Do the actual encoding here. See 'draco_nodejs_example.js' for a more
  // comprehensive example.
  cleanup(encoder);
}

function cleanup(encoder) {
  encoderModule.destroy(encoder);
}
