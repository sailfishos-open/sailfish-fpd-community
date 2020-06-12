#!/bin/sh

set -e

OUT_DEVICE=${HABUILD_DEVICE:-$DEVICE}

if [ ! -f ./out/target/product/${OUT_DEVICE}/system/lib/libbiometry_fp_api.so ]; then
    echo "Please build Fingerprint support as per HADK instructions"
    exit 1
fi



fold=$(dirname "$0")/../out
rm -rf $fold
mkdir $fold
mv ./out/target/product/${OUT_DEVICE}/system/lib/libbiometry_fp_api.so $fold

if [ -f ./out/target/product/${OUT_DEVICE}/system/bin/fake_crypt ]; then
mv ./out/target/product/${OUT_DEVICE}/system/bin/fake_crypt $fold
mv ./out/target/product/${OUT_DEVICE}/system/etc/init/fake_crypt.rc $fold
fi

ls -lh $fold

