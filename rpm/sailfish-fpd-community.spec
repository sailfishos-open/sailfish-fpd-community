Name:       sailfish-fpd-community

Summary:    FPD Community
Version:    1.1.1
Release:    1
License:    LICENSE
URL:        http://example.org/
Source0:    %{name}-%{version}.tar.bz2
Requires:   droid-biometry-fp >= 1.0.0
BuildRequires:  pkgconfig(Qt5Core)
BuildRequires:  pkgconfig(Qt5DBus)
BuildRequires:  desktop-file-utils
BuildRequires:  libhybris-devel
BuildRequires:  systemd
Provides:   sailfish-fpd

%description
SailfishOS Community Fingerprint Daemon

%prep
%autosetup -n %{name}-%{version}

%build

%qmake5
%make_build

%install
rm -rf %{buildroot}
%qmake5_install

mkdir -p %{buildroot}/%{_unitdir}/multi-user.target.wants
cp sailfish-fpd-community.service %{buildroot}/%{_unitdir}/
ln -s ../sailfish-fpd-community.service %{buildroot}/%{_unitdir}/multi-user.target.wants/sailfish-fpd-community.service

%files
%defattr(-,root,root,-)
%{_bindir}
%{_sysconfdir}/dbus-1/system.d/org.sailfishos.fingerprint1.conf
%{_unitdir}/sailfish-fpd-community.service
%{_unitdir}/multi-user.target.wants/sailfish-fpd-community.service

%post
systemctl daemon-reload || :
systemctl reload-or-restart sailfish-fpd-community.service || :

%preun
# in case of complete removal, stop
if [ "$1" = "0" ]; then
    systemctl stop sailfish-fpd-community.service || :
fi

%postun
systemctl daemon-reload || :
