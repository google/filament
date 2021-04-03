Pod::Spec.new do |spec|
  spec.name = "Filament"
  spec.version = "1.9.18"
  spec.license = { :type => "Apache 2.0", :file => "LICENSE" }
  spec.homepage = "https://google.github.io/filament"
  spec.authors = "Google LLC."
  spec.summary = "Filament is a real-time physically based rendering engine for Android, iOS, Windows, Linux, macOS, and WASM/WebGL."
  spec.platform = :ios, "11.0"
  spec.source = { :http => "https://github.com/google/filament/releases/download/v1.9.18/filament-v1.9.18-ios.tgz" }

  # Fix linking error with Xcode 12; we do not yet support the simulator on Apple silicon.
  spec.pod_target_xcconfig = {
    'EXCLUDED_ARCHS[sdk=iphonesimulator*]' => 'arm64'
  }
  spec.user_target_xcconfig = { 'EXCLUDED_ARCHS[sdk=iphonesimulator*]' => 'arm64' }

  spec.subspec "filament" do |ss|
    ss.source_files =
        "include/filament/*.h",
        "include/backend/*.h",
        "include/filament/MaterialChunkType.h",
        "include/filament/MaterialEnums.h",
        "include/ibl/*.h",
        "include/geometry/*.h"
    ss.header_mappings_dir = "include"
    ss.vendored_libraries =
        "lib/universal/libfilament.a",
        "lib/universal/libbackend.a",
        "lib/universal/libfilabridge.a",
        "lib/universal/libfilaflat.a",
        "lib/universal/libibl.a",
        "lib/universal/libgeometry.a"
    ss.dependency "Filament/utils"
    ss.dependency "Filament/math"
  end

  spec.subspec "filamat" do |ss|
    ss.source_files =
        "include/filamat/*.h",
        "include/filament/MaterialChunkType.h",
        "include/filament/MaterialEnums.h"
    ss.header_mappings_dir = "include"
    ss.vendored_libraries =
      "lib/universal/libfilamat.a",
      "lib/universal/libshaders.a",
      "lib/universal/libsmol-v.a",
      "lib/universal/libfilabridge.a"
    ss.dependency "Filament/utils"
    ss.dependency "Filament/math"
  end

  spec.subspec "gltfio_core" do |ss|
    ss.source_files = "include/gltfio/*.h"
    ss.vendored_libraries =
      "lib/universal/libgltfio_core.a",
      "lib/universal/libdracodec.a",
      "lib/universal/libgltfio_resources.a"
    ss.header_dir = "gltfio"
    ss.dependency "Filament/filament"
  end

  spec.subspec "camutils" do |ss|
    ss.source_files = "include/camutils/*.h"
    ss.vendored_libraries = "lib/universal/libcamutils.a"
    ss.header_dir = "camutils"
    ss.dependency "Filament/math"
  end

  spec.subspec "filameshio" do |ss|
    ss.source_files = "include/filameshio/*.h"
    ss.vendored_libraries =
      "lib/universal/libfilameshio.a",
      "lib/universal/libmeshoptimizer.a"
    ss.header_dir = "filameshio"
    ss.dependency "Filament/filament"
  end

  spec.subspec "image" do |ss|
    ss.source_files = "include/image/*.h"
    ss.vendored_libraries = "lib/universal/libimage.a"
    ss.header_dir = "image"
    ss.dependency "Filament/filament"
  end

  spec.subspec "utils" do |ss|
    ss.source_files = "include/utils/*.h"
    ss.vendored_libraries = "lib/universal/libutils.a"
    ss.header_dir = "utils"
  end

  spec.subspec "math" do |ss|
    ss.source_files = "include/math/*.h"
    ss.header_dir = "math"
  end
end
