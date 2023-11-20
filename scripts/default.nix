{ lib
, stdenv
, autoreconfHook
, libtool
, pkg-config
, which
, audacious
, libbsd
}:

stdenv.mkDerivation rec {
  pname = "audacious-uade";
  version = lib.removeSuffix "\n" (builtins.readFile ./VERSION);
  src = ./.;

  nativeBuildInputs = [
    autoreconfHook
    libtool
    pkg-config
    which
  ];

  buildInputs = [
    audacious
    libbsd
  ];

  configureFlags = [ "--with-audacious-plugindir=${placeholder "out"}/lib/audacious" ];

  meta = with lib; {
    description = "UADE plugin for Audacious music player";
    homepage = "https://github.com/mvtiaine/audacious-uade";
    license = licenses.gpl2Plus;
  };
}
