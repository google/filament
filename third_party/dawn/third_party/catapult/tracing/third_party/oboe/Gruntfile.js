module.exports = function (grunt) {

  function runNpmScript(command, cb) {
    var opts = {
      cmd: 'npm',
      args: ['run', command],
      opts: {
        stdio: 'inherit'
      }
    };

    grunt.util.spawn(opts, function(error, result, code) {
      if(error) {
        grunt.fail.warn(command + " failed.");
      }
      cb();
    });
  }

  var autoStartBrowsers = ['Chrome', 'Firefox', 'Safari'];

  var STREAM_SOURCE_PORT_HTTP = 4567;

  // NB: source files are order sensitive
  var OBOE_BROWSER_SOURCE_FILES = [
    'build/version.js'
    ,  'src/LICENCE.js'
    ,  'src/functional.js'
    ,  'src/util.js'
    ,  'src/lists.js'
    ,  'src/libs/clarinet.js'
    ,  'src/ascentManager.js'
    ,  'src/parseResponseHeaders.browser.js'
    ,  'src/detectCrossOrigin.browser.js'
    ,  'src/streamingHttp.browser.js'
    ,  'src/jsonPathSyntax.js'
    ,  'src/ascent.js'
    ,  'src/incrementalContentBuilder.js'
    ,  'src/jsonPath.js'
    ,  'src/singleEventPubSub.js'
    ,  'src/pubSub.js'
    ,  'src/events.js'
    ,  'src/patternAdapter.js'
    ,  'src/instanceApi.js'
    ,  'src/wire.js'
    ,  'src/defaults.js'
    ,  'src/publicApi.js'
  ];

  var OBOE_NODE_SOURCE_FILES = [
    'build/version.js'
    ,  'src/LICENCE.js'
    ,  'src/functional.js'
    ,  'src/util.js'
    ,  'src/lists.js'
    ,  'src/libs/clarinet.js'
    ,  'src/ascentManager.js'
    ,  'src/streamingHttp.node.js'
    ,  'src/jsonPathSyntax.js'
    ,  'src/ascent.js'
    ,  'src/incrementalContentBuilder.js'
    ,  'src/jsonPath.js'
    ,  'src/singleEventPubSub.js'
    ,  'src/pubSub.js'
    ,  'src/events.js'
    ,  'src/patternAdapter.js'
    ,  'src/instanceApi.js'
    ,  'src/wire.js'
    ,  'src/defaults.js'
    ,  'src/publicApi.js'
  ];

  var FILES_TRIGGERING_KARMA = [
    'src/**/*.js',
    'test/specs/*.spec.js',
    'test/libs/*.js'
  ];

  // load the wrapper file for packaging source targeted at either
  // browser or node
  function wrapper(target){
    return require('fs')
      .readFileSync('src/wrapper.' + target + '.js', 'utf8')
      .split('// ---contents--- //');
  }

  grunt.initConfig({

    pkg:grunt.file.readJSON("package.json")

    ,  clean: ['dist/*.js', 'build/*.js']

    ,  concat: {
      browser:{
        src: OBOE_BROWSER_SOURCE_FILES,
        dest: 'build/oboe-browser.concat.js'
      },
      node:{
        src: OBOE_NODE_SOURCE_FILES,
        dest: 'build/oboe-node.concat.js'
      }
    }

    ,  wrap: {
      browserPackage: {
        src: 'build/oboe-browser.concat.js',
        dest: '.',
        wrapper: wrapper('browser')
      },

      nodePackage: {
        src: 'build/oboe-node.concat.js',
        dest: '.',
        wrapper: wrapper('node')
      }
    }


    ,  uglify: {
      build:{
        files:{
          'build/oboe-browser.min.js': 'build/oboe-browser.concat.js'
        }
      }
    }

    ,  karma: {
      options:{
        singleRun: true,
        proxies: {
          '/testServer'   : 'http://localhost:' + STREAM_SOURCE_PORT_HTTP
        },
        // test results reporter to use
        // possible values: 'dots', 'progress', 'junit'
        reporters : ['progress'],

        // enable / disable colors in the output (reporters and logs)
        colors : true
      }
      ,
      'coverage':{
        reporters : ['coverage'],
        preprocessors: {
          // source files to generate coverage for
          // (these files will be instrumented by Istanbul)
          'src/**/*.js': ['coverage']
        },
        'browsers': ['PhantomJS'],
        configFile: 'test/unit.conf.js'
      }

      ,
      'precaptured-dev': {
        // for doing a single test run with already captured browsers during development.
        // this is good for running tests in browsers karma can't easily start such as
        // IE running inside a Windows VM on a unix dev environment
        browsers: [],
        configFile: 'test/unit.conf.js',
        singleRun: 'true'
      }
      ,
      'single-dev': {
        browsers: autoStartBrowsers,
        configFile: 'test/unit.conf.js'
      }
      ,
      'single-concat': {
        browsers: autoStartBrowsers,
        configFile: 'test/concat.conf.js'
      }
      ,
      'single-minified': {
        browsers: autoStartBrowsers,
        configFile: 'test/min.conf.js'
      }

      ,
      'single-amd': {
        browsers: autoStartBrowsers,
        configFile: 'test/amd.conf.js'
      }

      ,
      'single-browser-http': {
        browsers: autoStartBrowsers,
        configFile: 'test/http.conf.js'
      }

      ,
      'persist': {
        // for setting up a persistent karma server.
        // To start the server, the task is:
        //    karma:persist
        // To run these, the task is:
        //    karma:persist:run
        configFile: 'test/unit.conf.js',
        browsers: [],
        singleRun:false,
        background:true
      }
    }

    ,  copy: {
      browserDist: {
        files: [
          {src: ['build/oboe-browser.min.js'],    dest: 'dist/oboe-browser.min.js'}
          ,  {src: ['build/oboe-browser.concat.js'], dest: 'dist/oboe-browser.js'    }
        ]
      },
      nodeDist: {
        files: [
          {src: ['build/oboe-node.concat.js'],    dest: 'dist/oboe-node.js'}
        ]
      }
    }

    ,  exec:{
      // these might not go too well on Windows :-) - get Cygwin.
      reportMinifiedSize:{
        command: "echo minified size is `wc -c < dist/oboe-browser.min.js` bytes"
      },
      reportMinifiedAndGzippedSize:{
        command: "echo Size after gzip is `gzip --best --stdout dist/oboe-browser.min.js | wc -c` bytes - max 5120"
      },
      createGitVersionJs:{
        command: "echo \"// `git describe`\" > build/version.js"
      }
    }

    ,  watch:{
      karma:{
        files:FILES_TRIGGERING_KARMA,
        tasks:['karma:persist:run']
      },

      // like above but reports the file size. This is good for
      // watching while developing to make sure it doesn't get
      // too big. Doesn't run tests against minified.
      karmaAndSize:{
        files: FILES_TRIGGERING_KARMA,
        tasks:[
          'karma:persist:run',
          'browser-build',
          'dist-sizes']
      },

      // like above but reports the file size. This is good for
      // watching while developing to make sure it doesn't get
      // too big. Doesn't run tests against minified.
      testNode:{
        files: FILES_TRIGGERING_KARMA,
        tasks:[
          'node-build']
      },

      restartStreamSourceAndRunTests:{
        // this fails at the moment because start-stream-source
        // fails if run more than once - the port is taken.
        files: ['test/streamsource.js'],
        tasks: ['start-stream-source', 'karma:persist:run']
      }
    }

    ,  concurrent:{
      watchDev: {
        tasks:[ 'watch:karmaAndSize', 'watch:restartStreamSourceAndRunTests' ],
        options:{
          logConcurrentOutput: true
        }
      }
    }

  });

  require('matchdep').filterDev('grunt-*').forEach(grunt.loadNpmTasks);

  var streamSource;

  grunt.registerTask('start-stream-source', function () {
    grunt.log.ok('do we have a streaming source already?', !!streamSource);

    // if we previously loaded the streamsource, stop it to let the new one in:
    if( streamSource ) {
      grunt.log.ok('there seems to be a streaming server already, let\'s stop it');
      streamSource.stop();
    }

    streamSource = require('./test/streamsource.js');
    streamSource.start(STREAM_SOURCE_PORT_HTTP, grunt);
  });

  grunt.registerTask("jasmine_node_oboe", "Runs jasmine-node.", function() {
    runNpmScript('test-node', this.async());
  });

  // change the auto-starting browsers so that future tests will use
  // phantomjs instead of actual browsers. Can do:
  //    grunt headless-mode default
  // to run without any actual browsers
  grunt.registerTask('headless-mode', function(){
    autoStartBrowsers.length = 0;
    autoStartBrowsers.push('PhantomJS');
  });

  grunt.registerTask('test-start-server',   [
    'karma:persist'
  ]);

  grunt.registerTask('test-run',   [
    'karma:persist:run'
  ]);

  grunt.registerTask('dist-sizes',   [
    'exec:reportMinifiedAndGzippedSize'
  ]);

  grunt.registerTask('node-build',      [
    'exec:createGitVersionJs',
    'concat:node',
    'wrap:nodePackage',
    'copy:nodeDist'
  ]);

  grunt.registerTask('node-build-test',      [
    'node-build',
    'jasmine_node_oboe'
  ]);

  grunt.registerTask('node',      [
    'start-stream-source',
    'node-build-test'
  ]);

  grunt.registerTask('browser-build',      [
    'exec:createGitVersionJs',
    'concat:browser',
    'concat:node',
    'wrap:browserPackage',
    'uglify',
    'copy:browserDist'
  ]);

  grunt.registerTask('browser-build-test',      [
    'karma:single-dev',
    'karma:single-browser-http',
    'browser-build',
    'karma:single-concat',
    'karma:single-minified',
    'karma:single-amd'
  ]);

  grunt.registerTask('build',      [
    'browser-build',
    'node-build'
  ]);

  // build and run just the integration tests.
  grunt.registerTask('build-integration-test',      [
    'build',
    'start-stream-source',
    'karma:single-concat',
    'jasmine_node_oboe',
    'dist-sizes'
  ]);

  grunt.registerTask('default',      [

    'clear',
    'clean',
    'start-stream-source',

    'browser-build-test',

    'node-build-test',

    'dist-sizes'
  ]);



  // browser-test-auto-run or node-test-auto-run
  //
  // The most useful for developing. Start this task, capture some browsers
  // (unless node) then edit the code. Tests will be run as the code is
  // saved.
  grunt.registerTask('browser-test-auto-run',   [
    'start-stream-source',
    'karma:persist',
    'concurrent:watchDev'
  ]);
  grunt.registerTask('node-test-auto-run',   [
    'start-stream-source',
    'watch:testNode'
  ]);
  grunt.registerTask('coverage',   [
    'karma:coverage'
  ]);
};
