namespace :test do
  desc "run test suite with node.js"
  task :node => :build do
    # make sure jasmine-node exists, and barf if it doesn't
    if %x['jasmine-node'] =~ /USAGE/
      unless system 'jasmine-node', base_path.join('spec').to_s
        raise "node.js tests failed"
      end
    else
      puts "jasmine-node is not available"
      puts
      puts "Please run:"
      puts "   npm install -g jasmine-node"
      puts
      puts "...and then try again."
      puts
      exit
    end
  end
end
