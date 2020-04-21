/**
 * @fileoverview Main file for draco-animation package.
 */

var createEncoderModule = require('./draco_animation_encoder_nodejs');
var createDecoderModule = require('./draco_animation_decoder_nodejs');

module.exports = {
  createEncoderModule,
  createDecoderModule
}
