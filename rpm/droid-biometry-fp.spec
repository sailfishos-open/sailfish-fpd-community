%define strip /bin/true
%define __requires_exclude  ^.*$
%define __find_requires     %{nil}
%global debug_package       %{nil}
%define __provides_exclude_from ^.*$
%define device_rpm_architecture_string armv7hl
%define _target_cpu %{device_rpm_architecture_string}

Name:     droid-biometry-fp
Summary:  Android Biometry FingerPrint library
Version:  0.0.1
Release:  %(date +'%%Y%%m%%d%%H%%M')
Group:    Kernel/Linux Kernel
License:  GPLv3
Source0:  out/libbiometry_fp_api.so

%description
%{summary}

%build
pwd
ls -lh

%install

mkdir -p $RPM_BUILD_ROOT/usr/libexec/droid-hybris/system/lib
cp out/libbiometry_fp_api.so $RPM_BUILD_ROOT/usr/libexec/droid-hybris/system/lib

%files
%defattr(-,root,root,-)
/usr/libexec/droid-hybris/system/lib/libbiometry_fp_api.so
