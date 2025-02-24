/* eslint-disable node/no-unpublished-require */
/* eslint-disable no-console */

const timer = require('grunt-timer');
const { spawnSync } = require('child_process');
const path = require('path');

const kAllSuites = ['webgpu', 'stress', 'manual', 'unittests', 'demo'];

const kFilesForEslint = [
  // TS
  'src/**/*.ts',
  // JS
  '*.js',
  '.*.js',
  'tools/**/*.js',
];

module.exports = function (grunt) {
  timer.init(grunt);

  // Project configuration.
  grunt.initConfig({
    pkg: grunt.file.readJSON('package.json'),

    clean: {
      gen: ['gen/'],
      out: ['out/'],
      'out-wpt': ['out-wpt/'],
      'out-node': ['out-node/'],
    },

    run: {
      'generate-version': {
        cmd: 'node',
        args: ['tools/gen_version'],
      },
      'generate-listings-and-webworkers': {
        cmd: 'node',
        args: ['tools/gen_listings_and_webworkers', 'gen/', ...kAllSuites.map(s => 'src/' + s)],
      },
      validate: {
        cmd: 'node',
        args: ['tools/validate', ...kAllSuites.map(s => 'src/' + s)],
      },
      'generate-cache': {
        // Note this generates files into the src/ directory (not the gen/ directory).
        cmd: 'node',
        args: ['tools/gen_cache', 'src/webgpu'],
      },
      'validate-cache': {
        cmd: 'node',
        args: ['tools/gen_cache', 'src/webgpu', '--validate'],
      },
      // Note these generate `cts*.https.html` directly into the out-wpt/ directory rather than
      // the gen/ directory (as well as generating a `webgpu_variant_list*.json` file in gen/).
      'write-out-wpt-cts-html': {
        cmd: 'node',
        args: ['tools/gen_wpt_cts_html', 'tools/gen_wpt_cfg_unchunked.json'],
      },
      'write-out-wpt-cts-html-chunked2sec': {
        cmd: 'node',
        args: ['tools/gen_wpt_cts_html', 'tools/gen_wpt_cfg_chunked2sec.json'],
      },
      'write-out-wpt-cts-html-withsomeworkers': {
        cmd: 'node',
        args: ['tools/gen_wpt_cts_html', 'tools/gen_wpt_cfg_withsomeworkers.json'],
      },
      unittest: {
        cmd: 'node',
        args: ['tools/run_node', 'unittests:*'],
      },
      'build-out': {
        cmd: 'node',
        args: [
          'node_modules/@babel/cli/bin/babel',
          '--extensions=.ts,.js',
          '--source-maps=true',
          '--out-dir=out/',
          'src/',
          // These files will be generated, instead of compiled from TypeScript.
          '--ignore=src/common/internal/version.ts',
          '--ignore=src/*/listing.ts',
        ],
      },
      'build-out-wpt': {
        cmd: 'node',
        args: [
          'node_modules/@babel/cli/bin/babel',
          '--extensions=.ts,.js',
          '--source-maps=false',
          '--delete-dir-on-start',
          '--out-dir=out-wpt/',
          'src/',
          '--only=src/common/',
          '--only=src/external/',
          '--only=src/webgpu/',
          // These files will be generated, instead of compiled from TypeScript.
          '--ignore=src/common/internal/version.ts',
          '--ignore=src/*/listing.ts',
          // These files are only used by non-WPT builds.
          '--ignore=src/common/runtime/cmdline.ts',
          '--ignore=src/common/runtime/server.ts',
          '--ignore=src/common/runtime/standalone.ts',
          '--ignore=src/common/runtime/helper/sys.ts',
          '--ignore=src/common/tools',
        ],
      },
      'build-out-node': {
        cmd: 'node',
        args: [
          'node_modules/typescript/lib/tsc.js',
          '--project',
          'node.tsconfig.json',
          '--outDir',
          'out-node/',
        ],
      },
      'copy-assets': {
        cmd: 'node',
        args: [
          'node_modules/@babel/cli/bin/babel',
          'src/resources/',
          '--out-dir=out/resources/',
          '--copy-files',
        ],
      },
      'copy-assets-wpt': {
        cmd: 'node',
        args: [
          'node_modules/@babel/cli/bin/babel',
          'src/resources/',
          '--out-dir=out-wpt/resources/',
          '--copy-files',
        ],
      },
      'copy-assets-node': {
        cmd: 'node',
        args: [
          'node_modules/@babel/cli/bin/babel',
          'src/resources/',
          '--out-dir=out-node/resources/',
          '--copy-files',
        ],
      },
      lint: {
        cmd: 'node',
        args: ['node_modules/eslint/bin/eslint', ...kFilesForEslint, '--max-warnings=0'],
      },
      fix: {
        cmd: 'node',
        args: ['node_modules/eslint/bin/eslint', ...kFilesForEslint, '--fix'],
      },
      'autoformat-out-wpt': {
        cmd: 'node',
        // MAINTENANCE_TODO(gpuweb/cts#3128): This autoformat step is broken after a dependencies upgrade.
        args: [
          'node_modules/prettier/bin/prettier.cjs',
          '--log-level=warn',
          '--write',
          'out-wpt/**/*.js',
        ],
      },
      tsdoc: {
        cmd: 'node',
        args: ['node_modules/typedoc/bin/typedoc'],
      },
      'tsdoc-treatWarningsAsErrors': {
        cmd: 'node',
        args: ['node_modules/typedoc/bin/typedoc', '--treatWarningsAsErrors'],
      },

      serve: {
        cmd: 'node',
        args: ['node_modules/http-server/bin/http-server', '-p8080', '-a127.0.0.1', '-c-1'],
      },
    },

    copy: {
      'gen-to-out': {
        // Must run after generate-common and run:build-out.
        files: [
          { expand: true, dest: 'out/', cwd: 'gen', src: 'common/internal/version.js' },
          { expand: true, dest: 'out/', cwd: 'gen', src: '*/**/*.js' },
        ],
      },
      'gen-to-out-wpt': {
        // Must run after generate-common and run:build-out-wpt.
        files: [
          { expand: true, dest: 'out-wpt/', cwd: 'gen', src: 'common/internal/version.js' },
          { expand: true, dest: 'out-wpt/', cwd: 'gen', src: 'webgpu/**/*.js' },
        ],
      },
      'htmlfiles-to-out': {
        // Must run after run:build-out.
        files: [{ expand: true, dest: 'out/', cwd: 'src', src: 'webgpu/**/*.html' }],
      },
      'htmlfiles-to-out-wpt': {
        // Must run after run:build-out-wpt.
        files: [{ expand: true, dest: 'out-wpt/', cwd: 'src', src: 'webgpu/**/*.html' }],
      },
    },

    concurrent: {
      'write-out-wpt-cts-html-all': {
        tasks: [
          'run:write-out-wpt-cts-html',
          'run:write-out-wpt-cts-html-chunked2sec',
          'run:write-out-wpt-cts-html-withsomeworkers',
        ],
      },
      'all-builds': {
        tasks: ['build-standalone', 'build-wpt', 'run:build-out-node'],
      },
      'all-checks': {
        tasks: [
          'ts-check',
          'run:validate',
          'run:validate-cache',
          'run:unittest',
          'run:lint',
          'run:tsdoc-treatWarningsAsErrors',
        ],
      },
      'all-builds-and-checks': {
        tasks: [
          'build-all', // Internally concurrent
          'concurrent:all-checks',
        ],
      },
    },
  });

  grunt.loadNpmTasks('grunt-contrib-clean');
  grunt.loadNpmTasks('grunt-contrib-copy');
  grunt.loadNpmTasks('grunt-concurrent');
  grunt.loadNpmTasks('grunt-run');

  const helpMessageTasks = [];
  function registerTaskAndAddToHelp(name, desc, deps) {
    grunt.registerTask(name, deps);
    addExistingTaskToHelp(name, desc);
  }
  function addExistingTaskToHelp(name, desc) {
    helpMessageTasks.push({ name, desc });
  }

  grunt.registerTask('ts-check', () => {
    spawnSync(
      path.join('node_modules', '.bin', 'tsc'),
      ['--project', 'tsconfig.json', '--noEmit'],
      {
        shell: true,
        stdio: 'inherit',
      }
    );
  });

  grunt.registerTask('generate-common', 'Generate files into gen/ and src/', [
    'clean:gen',
    'run:generate-version',
    'run:generate-listings-and-webworkers',
    'run:generate-cache',
  ]);
  grunt.registerTask('build-standalone', 'Build out/ (no checks; run after generate-common)', [
    'clean:out',
    'run:build-out',
    'run:copy-assets',
    'copy:gen-to-out',
    'copy:htmlfiles-to-out',
  ]);
  grunt.registerTask('build-wpt', 'Build out-wpt/ (no checks; run after generate-common)', [
    'clean:out-wpt',
    'run:build-out-wpt',
    'run:copy-assets-wpt',
    'copy:gen-to-out-wpt',
    'copy:htmlfiles-to-out-wpt',
    'concurrent:write-out-wpt-cts-html-all',
    'run:autoformat-out-wpt',
  ]);
  grunt.registerTask('build-node', 'Build out-node/ (no checks; run after generate-common)', [
    'clean:out-node',
    'run:build-out-node',
    'run:copy-assets-node',
  ]);
  grunt.registerTask('build-all', 'Build out*/ (no checks; run after generate-common)', [
    'concurrent:all-builds',
    'build-done-message',
  ]);
  grunt.registerTask('build-done-message', () => {
    grunt.log.writeln(`\
=====================================================
==== Build completed! Continuing checks/tests... ====
=====================================================`);
  });

  grunt.registerTask('pre', ['all']);

  registerTaskAndAddToHelp('all', 'Run all builds and checks', [
    'generate-common',
    'concurrent:all-builds-and-checks',
  ]);
  registerTaskAndAddToHelp('standalone', 'Build standalone (out/) (no checks)', [
    'generate-common',
    'build-standalone',
    'build-done-message',
  ]);
  registerTaskAndAddToHelp('wpt', 'Build for WPT (out-wpt/) (no checks)', [
    'generate-common',
    'build-wpt',
    'build-done-message',
  ]);
  registerTaskAndAddToHelp('node', 'Build node (out-node/) (no checks)', [
    'generate-common',
    'build-node',
    'build-done-message',
  ]);
  registerTaskAndAddToHelp('checks', 'Run all checks (and build tsdoc)', ['concurrent:all-checks']);
  registerTaskAndAddToHelp('unittest', 'Just run unittests', ['run:unittest']);
  registerTaskAndAddToHelp('typecheck', 'Just typecheck', ['ts-check']);
  registerTaskAndAddToHelp('tsdoc', 'Just build tsdoc', ['run:tsdoc']);

  registerTaskAndAddToHelp('serve', 'Serve out/ (without building anything)', ['run:serve']);
  registerTaskAndAddToHelp('lint', 'Check lint and formatting', ['run:lint']);
  registerTaskAndAddToHelp('fix', 'Fix lint and formatting', ['run:fix']);

  addExistingTaskToHelp('clean', 'Delete built and generated files');

  grunt.registerTask('default', '', () => {
    console.error('\nRecommended tasks:');
    const nameColumnSize = Math.max(...helpMessageTasks.map(({ name }) => name.length));
    for (const { name, desc } of helpMessageTasks) {
      console.error(`$ grunt ${name.padEnd(nameColumnSize)}  # ${desc}`);
    }
  });
};
