desc "Generate JavaScript code coverage report in ./covershot"
task :coverage => %w(
  coverage:dependencies
  coverage:clean
  coverage:prepare
  coverage:instrumentation
  coverage:write_manifest
  coverage:execute
  coverage:generate
  coverage:done
)

namespace :coverage do
  task :dependencies do
    unless File.exist?(base_path.join('node_modules'))
      if %x[which npm].strip.length == 0
        raise <<-end_error
          Could not execute `npm`! Please make sure node.js and the Node Package
          Manager (NPM) are available and can be executed without root
          permissions.
        end_error
      else
        cmd = "npm install && bundle exec #{$0} #{ARGV.join ' '}"
        puts "Executing the following command:"
        puts
        puts "  #{cmd}"
        puts
        puts
        Kernel.exec cmd
      end
    end
  end

  task :clean do
    rm_rf base_path.join('covershot')
    rm_rf base_path.join('tmp')
  end

  task :prepare do
    manifest = sprockets['gl-matrix-manifest.js']
    coverage_path = base_path.join('tmp/coverage')

    manifest.dependencies.each do |part|
      path = coverage_path.join('lib').join(part.pathname.basename)
      mkdir_p(path.dirname) unless File.directory?(path.dirname)
      File.open(path, 'w') do |f|
        f.print part.body
      end
    end
  end

  task :instrumentation do
    bin = 'jscoverage'
    opts = [ '--no-highlight' ]
    input = base_path.join('tmp/coverage/lib').to_s
    output = base_path.join('tmp/coverage/lib-cov').to_s

    unless system *[bin, opts, input, output].flatten
      raise "Instrumentation failure. Please make sure `jscoverage` is installed."
    end
  end

  task :write_manifest do
    manifest = sprockets['gl-matrix-manifest.js']
    coverage_path = base_path.join('tmp/coverage')

    File.open(coverage_path.join('manifest.js'), 'w') do |manifest_out|
      manifest_out.puts <<-end_script
        var covershot = require('covershot');
        var csrequire = covershot.require.bind(null, require);

        function pull(str) {
          var exps = csrequire(str);
          for (var i in exps) {
            global[i] = exps[i];
          }
        }

        global.GLMAT_EPSILON = 0.000001;
        global.GLMAT_ARRAY_TYPE = Float32Array;

      end_script
      manifest.dependencies.each do |part|
        path = coverage_path.join('lib-cov').join(part.pathname.basename)
        manifest_out.puts "pull('#{path}');"
      end
      manifest_out.puts <<-end_script
        function CoverageReporter() {
          this.reportRunnerResults = function(suite) {
            covershot.writeCoverage();
          };
        };

        jasmine.getEnv().addReporter(new CoverageReporter());
      end_script
    end
  end

  task :execute do
    jasmine_node = base_path.join('node_modules/jasmine-node/bin/jasmine-node').to_s
    spec = base_path.join('spec').to_s

    unless system jasmine_node, spec
      raise "jasmine-node tests failed. Coverage report not generated."
    end
  end

  task :generate do
    covershot = base_path.join('node_modules/covershot/bin/covershot').to_s
    data_dir = base_path.join('covershot/data').to_s
    format = 'html'

    unless system covershot, data_dir, '-f', format
      raise "Execution of covershot failed. Coverage report not generated."
    end
  end

  task :done do
    rm_rf base_path.join('tmp')
    puts
    puts
    puts "Coverage report generated in: #{base_path.join("covershot/index.html")}"
    puts
  end
end
