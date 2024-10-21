{ pkgs ? import <nixpkgs> {} }:

pkgs.stdenv.mkDerivation {
  pname = "chunky";
  version = "1.0";

  src = ./.;

  nativeBuildInputs = [ pkgs.ncurses ];

  buildInputs = [ pkgs.ncurses ];

  buildPhase = ''
    mkdir -p dist/compiled dist/build
    make
  '';

  installPhase = ''
    mkdir -p $out/bin
    cp dist/build/chunky $out/bin/chunky
  '';

  meta = {
    description = "A chunk-based file viewer with ncurses";
    license = pkgs.lib.licenses.gpl3;
    maintainers = with pkgs.lib.maintainers; [ yourname ];
  };
}

