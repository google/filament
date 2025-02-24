# Copyright (c) 2013 Brandon Jones, Colin MacKenzie IV
# 
# This software is provided 'as-is', without any express or implied
# warranty. In no event will the authors be held liable for any damages
# arising from the use of this software.
# 
# Permission is granted to anyone to use this software for any purpose,
# including commercial applications, and to alter it and redistribute it
# freely, subject to the following restrictions:
# 
#  1. The origin of this software must not be misrepresented; you must not
#     claim that you wrote the original software. If you use this software
#     in a product, an acknowledgment in the product documentation would be
#     appreciated but is not required.
# 
#  2. Altered source versions must be plainly marked as such, and must not
#     be misrepresented as being the original software.
# 
#  3. This notice may not be removed or altered from any source distribution.
# 
$:.unshift File.expand_path('.', File.dirname(__FILE__))
require 'sprockets'
require 'jasmine'

class Jasmine::Config
  def simple_config_file
    File.expand_path GLMatrix.base_path.join('spec/jasmine.yml')
  end
end

class Rack::Jasmine::Runner
  alias_method :jasmine_call, :call
  def call(env)
    GLMatrix.compile
    jasmine_call env
  end
end

module GLMatrix
  autoload :ReleaseHelper, 'gl-matrix/release_helper'
  autoload :Version,       'gl-matrix/version'
  autoload :VERSION,       'gl-matrix/version'

  module_function

  def release(&block)
    GLMatrix::ReleaseHelper.release &block
  end

  def sprockets
    env = Sprockets::Environment.new base_path
    env.append_path base_path.join('src')
    env
  end

  def base_path
    Pathname.new File.expand_path('../..', File.dirname(__FILE__))
  end

  # Compiles the source file to the dest file. If a block
  # is given, the source file is yielded and replaced with
  # the result. Returns the destination as a Pathname.
  def compile(source = 'gl-matrix.js', dest = 'dist/gl-matrix.js')
    dest = base_path.join dest
    js = sprockets[source]
    js = yield js if block_given?

    File.open dest, "w" do |f|
      f.puts js
    end

    puts "compiled #{source} to #{dest.relative_path_from base_path}"
    dest
  end

  def minify(source = 'gl-matrix.js', dest = 'dist/gl-matrix-min.js')
    dest = compile source, dest do |js|
      Uglifier.compile js
    end

    puts "minified #{source} to #{dest.relative_path_from base_path}"
  end

end
