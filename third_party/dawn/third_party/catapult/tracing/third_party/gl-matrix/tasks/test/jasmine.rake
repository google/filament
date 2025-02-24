namespace :test do
  desc "Run specs via server"
  task :jasmine do
    port = ENV['JASMINE_PORT'] || 8888
    puts "your tests are here:"
    puts "  http://localhost:#{port}/"
    Jasmine::Server.new(port).start
  end
end
