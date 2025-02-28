%define VERSION_ORIG %(cat ../VERSION)
%define VERSION %(cat ../VERSION|sed s/'\-'/_/g)
%global aud_ver 3.8
%global libopenmpt_ver 0.6.0
%global libxmp_ver 4.5.0
%global deadbeef_ver 0.5.0
%bcond_with deadbeef

Name:           audacious-uade
Version:        %{VERSION}
Release:        1
Requires:       audacious >= %{aud_ver}
%if 0%{?suse_version}
Requires:       libopenmpt0 >= %{libopenmpt_ver}
Requires:       libxmp4 >= %{libxmp_ver}
%else
Requires:       libopenmpt >= %{libopenmpt_ver}
%if %{undefined rhel_version}
Requires:       libxmp >= %{libxmp_ver}
#endif
%endif
%if %{with deadbeef}
Requires:       deadbeef >= %{deadbeef_ver}
%endif
Summary:        UADE plugin for Audacious and DeaDBeeF

License:        GPL-2.0-or-later
URL:            https://github.com/mvtiaine/%{name}
Source:         %{name}-%{VERSION_ORIG}.tar.bz2

BuildRequires:  audacious-devel >= %{aud_ver}
%if %{with deadbeef}
BuildRequires:  deadbeef-devel >= %{deadbeef_ver}
%endif
BuildRequires:  libopenmpt-devel >= %{libopenmpt_ver}
%if %{undefined rhel_version}
BuildRequires:  libxmp-devel >= %{libxmp_ver}
%endif
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
%configure \
    --with-static-stdlibs=no \
%if %{undefined rhel_version}
    --enable-players=all \
%endif
    --enable-plugin-audacious=yes \
    %{?with_deadbeef:--enable-plugin-deadbeef=yes} \
    %{?with_deadbeef:--with-deadbeef-plugindir=%{_libdir}/deadbeef}
%make_build

%install
%make_install

%check
make -j check

%files
%license COPYING COPYING.LGPL NOTICE
%doc AUTHORS ChangeLog README VERSION
%{_libdir}/audacious/Input/uade.so
%if %{with deadbeef}
%{_libdir}/deadbeef/aaa_uade.so
%endif
%{_datadir}/%{name}/doc/*
%{_datadir}/%{name}/ext/*
%{_datadir}/%{name}/lib/uade/uadecore
%{_datadir}/%{name}/share/uade/*
%{_datadir}/%{name}/songdb/*
