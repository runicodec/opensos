{ pkgs, lib, config, inputs, ... }:

let
  rmlui = pkgs.stdenv.mkDerivation rec {
    pname = "RmlUi";
    version = "6.0";

    src = pkgs.fetchzip {
      url = "https://github.com/mikke89/RmlUi/archive/refs/tags/${version}.tar.gz";
      sha256 = "sha256-aBu97ZugJIfJTYFpgDYC/OU+pX/fe9ne4PsTG01mWIM=";
    };

    nativeBuildInputs = [ pkgs.cmake pkgs.ninja ];
    buildInputs = [ pkgs.freetype ];

    cmakeFlags = [
      "-DBUILD_SHARED_LIBS=ON"
      "-DRMLUI_SAMPLES=OFF"
      "-DRMLUI_TESTS=OFF"
    ];

    meta = {
      description = "RmlUi - The HTML/CSS User Interface Library";
      homepage = "https://github.com/mikke89/RmlUi";
    };
  };
in
{
  packages = [
    pkgs.cmake
    pkgs.ninja
    pkgs.pkg-config
    pkgs.SDL2
    pkgs.SDL2_image
    pkgs.SDL2_mixer
    pkgs.SDL2_ttf
    pkgs.freetype
    pkgs.libwebp
    pkgs.libtiff
    pkgs.libjpeg
    pkgs.libpng
    pkgs.libjxl
    pkgs.libavif
    rmlui
  ];

  languages.cplusplus.enable = true;

  env = {
    # Help CMake find packages installed by Nix
    CMAKE_PREFIX_PATH = lib.concatStringsSep ":" [
      "${pkgs.SDL2.dev}"
      "${pkgs.SDL2_image}"
      "${pkgs.SDL2_mixer.dev}"
      "${pkgs.SDL2_ttf}"
      "${pkgs.freetype.dev}"
      "${pkgs.libwebp}"
      "${pkgs.libtiff.dev}"
      "${pkgs.libjpeg.dev}"
      "${pkgs.libpng.dev}"
      "${pkgs.libjxl.dev}"
      "${pkgs.libavif.dev}"
      "${rmlui}"
    ];
  };

  enterShell = ''
    echo "SOSandCE C++ dev environment"
    echo "  cmake  $(cmake --version | head -1)"
    echo "  clang  $(clang --version | head -1)"
    echo "  ninja  $(ninja --version)"
    echo ""
    echo "Build with:"
    echo "  cmake -B build -G Ninja -DCMAKE_BUILD_TYPE=Debug"
    echo "  cmake --build build"
  '';

  tasks."sosandce:build" = {
    exec = ''
      cmake -B build -G Ninja -DCMAKE_BUILD_TYPE=Debug
      cmake --build build
    '';
  };

  tasks."sosandce:release" = {
    exec = ''
      cmake -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
      cmake --build build
    '';
  };

}
