#!/bin/sh

set -e

OUT_DEVICE=${HABUILD_DEVICE:-$DEVICE}

if [ -f out/target/product/${OUT_DEVICE}/system/lib64/libbiometry_fp_api.so ]; then
    DROIDLIB=lib64
else
    DROIDLIB=lib
fi

if [ ! -f ./out/target/product/${OUT_DEVICE}/system/${DROIDLIB}/libbiometry_fp_api.so ]; then
    echo "Please build Fingerprint support as per HADK instructions"
    exit 1
fi

pkg=droid-biometry-fp-0.0.0
fold=$(dirname "$0")/../$pkg
rm -rf $fold
mkdir $fold
mkdir -p $fold/out/target/product/${OUT_DEVICE}/system/${DROIDLIB}
mv ./out/target/product/${OUT_DEVICE}/system/${DROIDLIB}/libbiometry_fp_api.so $fold/out/target/product/${OUT_DEVICE}/system/${DROIDLIB}/

if [ -f ./out/target/product/${OUT_DEVICE}/system/bin/fake_crypt ]; then
mkdir -p $fold/out/target/product/${OUT_DEVICE}/system/bin
mkdir -p $fold/out/target/product/${OUT_DEVICE}/system/etc/init
mv ./out/target/product/${OUT_DEVICE}/system/bin/fake_crypt $fold/out/target/product/${OUT_DEVICE}/system/bin/
mv ./out/target/product/${OUT_DEVICE}/system/etc/init/fake_crypt.rc $fold/out/target/product/${OUT_DEVICE}/system/etc/init/
sed -i 's+/system/bin/fake_crypt+/usr/libexec/droid-hybris/system/bin/fake_crypt+g' $fold/out/target/product/${OUT_DEVICE}/system/etc/init/fake_crypt.rc
fi

ls -lh $fold

tar -cjvf $fold.tgz -C $(dirname $fold) $pkg
