Name:       sailfish-fpd-community

Summary:    FPD Community
Version:    1.0
Release:    1
Group:      Qt/Qt
License:    LICENSE
URL:        http://example.org/
Source0:    %{name}-%{version}.tar.bz2
Requires:   sailfishsilica-qt5 >= 0.10.9
Requires:   droid-biometry-fp >= 1.0.0
BuildRequires:  pkgconfig(sailfishapp) >= 1.0.2
BuildRequires:  pkgconfig(Qt5Core)
BuildRequires:  pkgconfig(Qt5Qml)
BuildRequires:  pkgconfig(Qt5Quick)
BuildRequires:  pkgconfig(Qt5DBus)
BuildRequires:  desktop-file-utils
BuildRequires:  libhybris-devel
Provides:   sailfish-fpd

%description
SailfishOS Community Fingerprint Daemon

%prep
%setup -q -n %{name}-%{version}

%build
# >> build pre
# << build pre

%qmake5 
make %{?_smp_mflags}

%install
rm -rf %{buildroot}
# >> install pre
# << install pre
%qmake5_install

%files
%defattr(-,root,root,-)
%{_bindir}
%{_sysconfdir}/dbus-1/system.d/org.sailfishos.fingerprint1.conf
/lib/systemd/system/sailfish-fpd-community.service

%post
systemctl daemon-reload || :
systemctl enable sailfish-fpd-community.service || :
systemctl start sailfish-fpd-community.service || :

%pre
# In case of update, stop
if [ "$1" = "2" ]; then
    systemctl stop sailfish-fpd-community.service  || :
fi

%preun
# in case of complete removal, stop and disable
if [ "$1" = "0" ]; then
    systemctl stop sailfish-fpd-community.service || :
    systemctl disable sailfish-fpd-community.service || :
fi
