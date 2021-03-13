%define strip /bin/true
%define __requires_exclude  ^.*$
%define __find_requires     %{nil}
%global debug_package       %{nil}
%define __provides_exclude_from ^.*$

Name:     droid-fake-crypt
Summary:  Android Fake Crypt binary
Version:  1.1.1
Release:  %(date +'%%Y%%m%%d%%H%%M')
License:  GPLv3
Source0:  droid-biometry-fp-0.0.0.tgz

%description
%{summary}

%build
pwd
ls -lh
tar -xvf droid-biometry-fp-0.0.0.tgz

%install
pushd droid-biometry-fp-0.0.0

mkdir -p %{buildroot}%{_libexecdir}/droid-hybris/system/etc/init
mkdir -p %{buildroot}%{_libexecdir}/droid-hybris/system/bin
cp out/target/product/*/system/bin/fake_crypt %{buildroot}%{_libexecdir}/droid-hybris/system/bin/
cp out/target/product/*/system/etc/init/fake_crypt.rc %{buildroot}%{_libexecdir}/droid-hybris/system/etc/init/

popd

%files
%defattr(-,root,root,-)
%{_libexecdir}/droid-hybris/system/etc/init/fake_crypt.rc
%{_libexecdir}/droid-hybris/system/bin/fake_crypt
