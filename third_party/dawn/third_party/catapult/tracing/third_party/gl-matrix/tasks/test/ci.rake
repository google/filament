namespace :test do
  desc "Run continuous integration tests"
  RSpec::Core::RakeTask.new('ci' => :build) do |t|
    t.rspec_opts = ["--colour", "--format", ENV['JASMINE_SPEC_FORMAT'] || "progress"]
    t.verbose = true
    t.rspec_opts += ["-r #{base_path.join('tasks/support/gl-matrix')}"]
    t.pattern = [Jasmine.runner_filepath]
  end
end
