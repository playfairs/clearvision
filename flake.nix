{
  description = "ClearVision - Fast recursive search utility written in C";

  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/nixos-unstable";
    flake-utils.url = "github:numtide/flake-utils";
  };

  outputs = { self, nixpkgs, flake-utils }:
    flake-utils.lib.eachDefaultSystem (system:
      let
        pkgs = nixpkgs.legacyPackages.${system};
      in
      {
        devShells.default = pkgs.mkShell {
          buildInputs = with pkgs; [
            just
            clang
            gcc
            pkg-config
            sqlite
            pcre2.dev
            ncurses.dev
            gdb
          ];
        };

        packages.default = pkgs.stdenv.mkDerivation {
          pname = "clearvision";
          version = "0.1.0";

          src = ./.;

          nativeBuildInputs = with pkgs; [
            just
            clang
            pkg-config
          ];

          buildInputs = with pkgs; [
            sqlite
            pcre2
            ncurses
          ];

          buildPhase = ''
            just release
          '';

          installPhase = ''
            install -Dm755 cv $out/bin/cv
          '';
        };
      }
    );
}
