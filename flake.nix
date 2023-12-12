{
  description = "Filament";

  inputs = {
    nixpkgs.url = "nixpkgs";
    systems.url = "github:nix-systems/x86_64-linux";
    flake-utils = {
      url = "github:numtide/flake-utils";
      inputs.systems.follows = "systems";
    };
    flake-compat.url = "https://flakehub.com/f/edolstra/flake-compat/1.tar.gz";
  };

  outputs =
    { nixpkgs
    , flake-utils
    , ...
    }:
    flake-utils.lib.eachDefaultSystem (system:
    let
      pkgs = nixpkgs.legacyPackages.${system};
      pname = "filament";
      version = "0.0.1";
      src = ./.;
      buildInputs = with pkgs; [
        # Tip: you can use `nix-locate foo.h` to find the package that provides
        # a header file, see https://github.com/nix-community/nix-index
        libGL
        xorg.libX11
        xorg.libXi
        xorg.libXxf86vm
        xorg.libxcb
      ];
      nativeBuildInputs = with pkgs; [
        # C++
        clang-tools
        cmake
        ninja
        # Documentation
        doxygen
        graphviz
        mdbook
        nodejs
      ];
      stdenv = pkgs.llvmPackages_14.libcxxStdenv;
    in
    {
      devShells.default = pkgs.mkShell.override { inherit stdenv; } {
        inherit buildInputs nativeBuildInputs;

        # You can use NIX_CFLAGS_COMPILE to set the default CFLAGS for the shell
        #NIX_CFLAGS_COMPILE = "-g";
        # You can use NIX_LDFLAGS to set the default linker flags for the shell
        #NIX_LDFLAGS = "-L${lib.getLib zstd}/lib -lzstd";
      };

      packages.default = stdenv.mkDerivation {
        inherit buildInputs nativeBuildInputs pname version src;
      };
    });
}
