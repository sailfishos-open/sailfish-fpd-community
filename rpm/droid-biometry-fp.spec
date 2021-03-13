%define strip /bin/true
%define __requires_exclude  ^.*$
%define __find_requires     %{nil}
%global debug_package       %{nil}
%define __provides_exclude_from ^.*$

Name:     droid-biometry-fp
Summary:  Android Biometry FingerPrint library
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

if [ -f out/target/product/*/system/lib64/libbiometry_fp_api.so ]; then
DROIDLIB=lib64
else
DROIDLIB=lib
fi

mkdir -p %{buildroot}%{_libexecdir}/droid-hybris/system/$DROIDLIB/
cp out/target/product/*/system/$DROIDLIB/libbiometry_fp_api.so %{buildroot}%{_libexecdir}/droid-hybris/system/$DROIDLIB/

popd

LIBBIOMETRYFPSOLOC=file.list
echo %{_libexecdir}/droid-hybris/system/$DROIDLIB/libbiometry_fp_api.so > ${LIBBIOMETRYFPSOLOC}

%files -f file.list
%defattr(-,root,root,-)
