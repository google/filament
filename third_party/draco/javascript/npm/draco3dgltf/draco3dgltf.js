/**
 * @fileoverview Main file for draco3d package.
 */

var createEncoderModule = require('./draco_encoder_gltf_nodejs');
var createDecoderModule = require('./draco_decoder_gltf_nodejs');

module.exports = {
  createEncoderModule,
  createDecoderModule
}
