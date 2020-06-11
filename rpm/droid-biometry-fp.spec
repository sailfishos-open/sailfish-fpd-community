%define strip /bin/true
%define __requires_exclude  ^.*$
%define __find_requires     %{nil}
%global debug_package       %{nil}
%define __provides_exclude_from ^.*$
%define device_rpm_architecture_string armv7hl
%define _target_cpu %{device_rpm_architecture_string}
#Define the following macro if packaging fake_crpyt
#define with_fake_crypt 1

Name:     droid-biometry-fp
Summary:  Android Biometry FingerPrint library
Version:  1.0.0
Release:  %(date +'%%Y%%m%%d%%H%%M')
Group:    Kernel/Linux Kernel
License:  GPLv3
Source0:  out/libbiometry_fp_api.so
%if 0%{?with_fake_crypt:1}
Source1:  out/fake_crypt
Source2:  out/fake_crypt.rc
%endif

%description
%{summary}

%build
pwd
ls -lh

%install

mkdir -p $RPM_BUILD_ROOT/usr/libexec/droid-hybris/system/lib
cp out/libbiometry_fp_api.so $RPM_BUILD_ROOT/usr/libexec/droid-hybris/system/lib
%if 0%{?with_fake_crypt:1}
mkdir -p $RPM_BUILD_ROOT/usr/libexec/droid-hybris/system/etc/init
mkdir -p $RPM_BUILD_ROOT/usr/libexec/droid-hybris/system/bin
cp out/fake_crypt $RPM_BUILD_ROOT/usr/libexec/droid-hybris/system/bin/
cp out/fake_crypt.rc $RPM_BUILD_ROOT/usr/libexec/droid-hybris/system/etc/init/
%endif

%files
%defattr(-,root,root,-)
/usr/libexec/droid-hybris/system/lib/libbiometry_fp_api.so
%if 0%{?with_fake_crypt:1}
/usr/libexec/droid-hybris/system/etc/init/fake_crypt.rc
/usr/libexec/droid-hybris/system/bin/fake_crypt
%endif
