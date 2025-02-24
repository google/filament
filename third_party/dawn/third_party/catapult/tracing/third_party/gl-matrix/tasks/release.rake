desc "tag and release gl-matrix v#{GLMatrix::VERSION}"
task :release do
  require 'thor'
  Bundler.ui = Bundler::UI::Shell.new(Thor::Shell::Basic.new)
  Bundler.ui.debug! if ENV['DEBUG']

  # Sanity check: rebuild files just in case dev forgot to.
  # If so, files will change and release will abort since changes
  # were not checked in.
  Rake::Task['build'].invoke

  release do
    # Put other release-related stuff here, such as publishing docs;
    # if anything fails, gl-matrix will be untagged and not pushed.
    #
    # Example:
    #
    #   Rake::Task['doc:publish'].invoke
    #
  end
end
