/**
 * This file points out anomalies/exceptions in test files when generating the binding.cc file
 *
 * nouns: words in file names that are misspelled
 *    *NOTE: a 'constructor' property is explicitly added to override javascript object constructor
 *
 * exportNames: anomalies in init function names
 *
 * propertyNames: anomalies in exported property name of init functions
 *
 * skipBinding: skip including this file in binding.cc
 */
module.exports = {
  nouns: {
    constructor: 'constructor',
    threadsafe: 'threadSafe',
    objectwrap: 'objectWrap'
  },
  exportNames: {
    AsyncWorkerPersistent: 'PersistentAsyncWorker'
  },
  propertyNames: {
    async_worker_persistent: 'persistentasyncworker',
    objectwrap_constructor_exception: 'objectwrapConstructorException'
  },
  skipBinding: [
    'global_object_delete_property',
    'global_object_get_property',
    'global_object_has_own_property',
    'global_object_set_property'
  ]
};
