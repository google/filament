module.exports = function(config) {
   config.set({

      frameworks:['jasmine'],

      // base path, that will be used to resolve files and exclude
      basePath : '..',

      // list of files / patterns to load in the browser
      files : [
         'test/libs/es5-shim.js'
      ,  'test/libs/es5-sham.js'
      ,  'test/libs/sinon.js'
      ,  'test/libs/sinon-ie.js'
      ,  'test/libs/*.js'

      ,  'src/events.js'
      ,  'src/functional.js'
      ,  'src/util.js'
      ,  'src/parseResponseHeaders.browser.js'
      ,  'src/detectCrossOrigin.browser.js'
      ,  'src/streamingHttp.browser.js'


      ,  'test/specs/streamingHttp.integration.spec.js'
      ],

      // level of logging
      // possible values: LOG_DISABLE || LOG_ERROR || LOG_WARN || LOG_INFO || LOG_DEBUG
      logLevel : config.LOG_INFO,

      // enable / disable watching file and executing tests whenever any file changes
      autoWatch : false,

      // Continuous Integration mode
      // if true, it capture browsers, run tests and exit
      singleRun : true

   });
};
