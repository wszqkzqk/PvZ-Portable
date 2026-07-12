{ pkgs ? import <nixpkgs> {} }:

pkgs.mkShell {
  nativeBuildInputs = with pkgs; [
    cmake
    ninja
    pkg-config
  ];

  buildInputs = with pkgs; [
    libogg
    libjpeg
    libopenmpt
    libpng
    libvorbis
    libmpg123
    SDL2
  ];

  shellHook = ''
    echo "Welcome to the development environment!"
    echo "All dependencies have been installed temporarily."
  '';
}
