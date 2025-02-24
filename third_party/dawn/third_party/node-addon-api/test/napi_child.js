// Makes sure that child processes are spawned appropriately.
exports.spawnSync = function (command, args, options) {
  if (require('../index').needsFlag) {
    args.splice(0, 0, '--napi-modules');
  }
  return require('child_process').spawnSync(command, args, options);
};

exports.spawn = function (command, args, options) {
  if (require('../index').needsFlag) {
    args.splice(0, 0, '--napi-modules');
  }
  return require('child_process').spawn(command, args, options);
};
