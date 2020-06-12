%define strip /bin/true
%define __requires_exclude  ^.*$
%define __find_requires     %{nil}
%global debug_package       %{nil}
%define __provides_exclude_from ^.*$
%define device_rpm_architecture_string armv7hl
%define _target_cpu %{device_rpm_architecture_string}

Name:     droid-fake-crypt
Summary:  Android Fake Crypt binary
Version:  1.0.0
Release:  %(date +'%%Y%%m%%d%%H%%M')
Group:    Kernel/Linux Kernel
License:  GPLv3
Source0:  out/fake_crypt
Source1:  out/fake_crypt.rc

%description
%{summary}

%build
pwd
ls -lh

%install
mkdir -p $RPM_BUILD_ROOT/usr/libexec/droid-hybris/system/etc/init
mkdir -p $RPM_BUILD_ROOT/usr/libexec/droid-hybris/system/bin
cp out/fake_crypt $RPM_BUILD_ROOT/usr/libexec/droid-hybris/system/bin/
cp out/fake_crypt.rc $RPM_BUILD_ROOT/usr/libexec/droid-hybris/system/etc/init/

%files
%defattr(-,root,root,-)
/usr/libexec/droid-hybris/system/etc/init/fake_crypt.rc
/usr/libexec/droid-hybris/system/bin/fake_crypt
