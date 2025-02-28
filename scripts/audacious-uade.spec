%define VERSION_ORIG %(cat ../VERSION)
%define VERSION %(cat ../VERSION|sed s/'\-'/_/g)
%global aud_ver 3.8
%global libopenmpt_ver 0.6.0
%global libxmp_ver 4.5.0
%global deadbeef_ver 0.5.0

Name:           audacious-uade
Version:        %{VERSION}
Release:        1
Requires:       audacious >= %{aud_ver}
Requires:       libopenmpt >= %{libopenmpt_ver}
Requires:       libxmp >= %{libxmp_ver}
Suggests:       deadbeef >= %{deadbeef_ver}
Summary:        UADE plugin for Audacious and DeaDBeeF

License:        GPL-2.0-or-later
URL:            https://github.com/mvtiaine/%{name}
Source:         %{name}-%{VERSION_ORIG}.tar.bz2

BuildRequires:  audacious-devel >= %{aud_ver}
BuildRequires:  libopenmpt-devel >= %{libopenmpt_ver}
BuildRequires:  libxmp-devel >= %{libxmp_ver}
BuildSuggests:  deadbeef-devel >= %{deadbeef_ver}
BuildRequires:  gcc-c++
BuildRequires:  make
BuildRequires:  autoconf
BuildRequires:  automake
BuildRequires:  libtool
BuildRequires:  pkg-config

%description
Audacious (https://audacious-media-player.org/) and DeaDBeeF (https://deadbeef.sourceforge.io/) input plugin for UADE (https://zakalwe.fi/uade/) and other retro music replays

%prep
%autosetup -n %{name}-%{VERSION_ORIG}

%build
%configure --enable-players=all --enable-plugin-audacious=yes --with-static-stdlibs=no
%make_build

%install
%make_install

%check
make -j check

%files
%license COPYING COPYING.LGPL NOTICE
%doc AUTHORS ChangeLog README VERSION
%{_libdir}/audacious/Input/uade.so
%missingok {_libdir}/deadbeef/aaa_uade.so
%{_datadir}/%{name}/doc/*
%{_datadir}/%{name}/ext/*
%{_datadir}/%{name}/lib/uade/uadecore
%{_datadir}/%{name}/share/uade/*
%{_datadir}/%{name}/songdb/*

%changelog
%autochangelog
