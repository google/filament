desc "compile & minify sources into a single file"
task :build => ['build:compile', 'build:minify']
