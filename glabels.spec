Summary: glabels is a GNOME program to create labels and business cards
Name:      glabels
Version:   3.4.1
Release:   1
License: GPL
Group: Applications/Publishing
URL: http://glabels.sourceforge.net/

Source: %{name}-%{version}.tar.gz
BuildRoot: %{_tmppath}/root-%{name}-%{version}
Prefix: %{_prefix}

BuildRequires: gtk2-devel >= 3.14.0
BuildRequires: libxml2-devel >= 2.9.0
BuildRequires: libgnomeui-devel >= @LIBGNOMEUI_REQUIRED@
BuildRequires: libglade2-devel >= @LIBGLADE_REQUIRED@

Requires: gtk2 >= 3.14.0
Requires: libxml2 >= 2.9.0
Requires: libgnomeui >= @LIBGNOMEUI_REQUIRED@
Requires: libglade2 >= @LIBGLADE_REQUIRED@

%description
gLabels is a lightweight program for creating labels and
business cards for the GNOME desktop environment.
It is designed to work with various laser/ink-jet peel-off
label and business card sheets that you'll find at most office
supply stores.

%prep
%setup

%build
%configure
%{__make} %{?_smp_mflags}

%install
%{__rm} -rf %{buildroot}
%makeinstall
%find_lang %{name}

desktop-file-install --vendor gnome --delete-original \
	--add-category X-Red-Hat-Base              \
	--dir %{buildroot}%{_datadir}/applications \
	%{buildroot}%{_datadir}/applications/%{name}.desktop

%{__rm} -rf %{buildroot}/var/scrollkeeper
%{__rm} -f %{buildroot}%{_datadir}/mime/XMLnamespaces
%{__rm} -f %{buildroot}%{_datadir}/mime/globs
%{__rm} -f %{buildroot}%{_datadir}/mime/magic
%{__rm} -f %{buildroot}%{_datadir}/mime/mime.cache
%{__rm} -rf %{buildroot}%{_datadir}/mime/application
%{__rm} -rf %{buildroot}%{_datadir}/mime/aliases
%{__rm} -rf %{buildroot}%{_datadir}/mime/subclasses


%clean
%{__rm} -rf %{buildroot}

%post
scrollkeeper-update
if (update-mime-database -v &> /dev/null); then
  update-mime-database "%{_datadir}/mime" > /dev/null
fi

%postun
scrollkeeper-update
if (update-mime-database -v &> /dev/null); then
  update-mime-database "%{_datadir}/mime" > /dev/null
fi

%files -f %{name}.lang
%defattr(-, root, root)
%doc README COPYING.README_FIRST COPYING COPYING-DOCS COPYING-LIBGLABELS ChangeLog NEWS AUTHORS INSTALL
%doc %{_datadir}/gnome/help/glabels/
%{_bindir}/glabels
%{_bindir}/glabels-batch
%{_libdir}/libglabels.*
%{_libdir}/pkgconfig/libglabels.pc
%{_includedir}/libglabels/*
%{_datadir}/glabels/
%{_datadir}/pixmaps/*
%{_datadir}/omf/
%{_datadir}/applications/*.desktop
%{_datadir}/application-registry/*
%{_datadir}/mime-info/*
%{_datadir}/mime/packages/*
%{_datadir}/man/*
%{_datadir}/gtk-doc/html/libglabels/*

%changelog
* Sun Aug  8 2004 Jim Evins <evins@snaught.com>
- Added support for freedesktop.org mime database registration

* Sat Feb 21 2004 Jim Evins <evins@snaught.com>
- Added libglabels related files

* Tue Dec 23 2003 Jim Evins <evins@snaught.com>
- Added support for scrollkeeper

* Sat Oct 18 2003 Jim Evins <evins@snaught.com>
- Updated, based largely on Dag Wieers <dag@wieers.com> glabels.spec

* Sat May 19 2001 Jim Evins <evins@snaught.com>
- Created

