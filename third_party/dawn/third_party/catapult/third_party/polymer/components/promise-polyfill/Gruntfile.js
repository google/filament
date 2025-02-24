/**
@license
Copyright (c) 2016 The Polymer Project Authors. All rights reserved.
This code may only be used under the BSD style license found at http://polymer.github.io/LICENSE.txt
The complete set of authors may be found at http://polymer.github.io/AUTHORS.txt
The complete set of contributors may be found at http://polymer.github.io/CONTRIBUTORS.txt
Code distributed by Google as part of the polymer project is also
subject to an additional IP rights grant found at http://polymer.github.io/PATENTS.txt
*/
module.exports = function(grunt) {

	grunt.initConfig({
		pkg: grunt.file.readJSON('package.json'),

		uglify: {
			options: {
				banner: '/*! <%= pkg.name %> <%= pkg.version %> */\n'
			},
			dist: {
				files: {
					'Promise.min.uglify.js': ['Promise.js']
				}
			}
		},

    closurecompiler: {
      options: {
        compilation_level: 'ADVANCED_OPTIMIZATIONS',
      },
      dist: {
        files: {
          'Promise.min.js': ['Promise.js']
        }
      }
    },

    bytesize: {
      dist: {
        src: ['Promise*.js']
      }
    }
	});

	grunt.loadNpmTasks('grunt-contrib-uglify');
	grunt.loadNpmTasks('grunt-closurecompiler');
	grunt.loadNpmTasks('grunt-bytesize');

	grunt.registerTask('build', ['closurecompiler', 'bytesize']);
};
