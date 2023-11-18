{ lib
, stdenv
, autoreconfHook
, autoconf-archive
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
    autoconf-archive
    libtool
    pkg-config
    which
  ];

  buildInputs = [
    audacious
    libbsd
  ];

  env.NIX_CFLAGS_COMPILE = "-Wno-format-security";
  
  configureFlags = [ "--with-audacious-plugindir=${placeholder "out"}/lib/audacious" ];

  meta = with lib; {
    description = "UADE plugin for Audacious music player";
    homepage = "https://github.com/mvtiaine/audacious-uade";
    license = licenses.gpl2Plus;
  };
}
