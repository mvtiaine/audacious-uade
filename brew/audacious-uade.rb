class AudaciousUade < Formula
    desc "UADE plugin for Audacious media player"
    homepage "https://github.com/mvtiaine/audacious-uade"
    license "GPL-2.0-or-later"

    url "https://github.com/mvtiaine/audacious-uade", :using => :git, :tag => "0.8.1-test"

    depends_on "audacious"
    depends_on "autoconf" => :build
    depends_on "autoconf-archive" => :build
    depends_on "automake" => :build
    depends_on "libtool" => :build
    depends_on "pkg-config" => :build

    def install
        system "autoreconf", "-i"
        system "./configure", "--with-audacious-plugindir=\"#{lib}/audacious\"", *std_configure_args
        system "make"
        system "make", "install"
    end

    test do
        system bin/"audacious", "--help"
    end
end
