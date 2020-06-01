# Sailfish Fingerprint Daemon - Community Edition

This daemon implements the DBUS API of the Jolla sailfish-fpd packge, and can be used with sailfish-devicelock-fpd
to add fingerprint support to community ports.

While sailfish-fpd is available in the repositories, it requires a closed, device specific sailfish-fpd-slave
package which talks to the hardware.  In this implementation, the daemon itself talks to the hardware using
an android library, inspired from biometryd from Ubuntu Touch, by Erfan Abdi (https://github.com/erfanoabdi/biometryd)

Some of the code from biometryd has been copied into this project.  That code is licensed under the LGPLv3 and 
retains its header information.

Reverse engineering was performed using:
 * dbus-monitor --system "interface=org.sailfishos.fingerprint1"
  ** This show the correct ordering of method calls from the various clients
 * running "strings" on the sailfilsh-fpd binary
  ** This gave a good hint to some of the return variables and states
 * The soruces for mce, which talks to the daemon and whose code is open and available
  ** This gave solid function definitions and some of the enums used
  
## How to build for a device

### Android Library

In HADK:

    git clone https://github.com/piggz/sailfish-fpd-community.git hybris/mw/sailfish-fpd-community
    source build/envsetup.sh
    export USE_CCACHE=1
    lunch aosp_$DEVICE-user (or appropriate name)
    make libbiometry_fp_api_32
    hybris/mw/sailfish-fpd-community/rpm/copy-hal.sh

In SDK:

    rpm/dhd/helpers/build_packages.sh --build=hybris/mw/sailfish-fpd-community --spec=rpm/droid-biometry-fp.spec --do-not-install

### Daemon

in SDK:

    rpm/dhd/helpers/build_packages.sh --build=hybris/mw/sailfish-fpd-community

## Testing

While the daemon is known to work on several devices, it may sometimes be possible to test the daemon without
using the sailfish settings page.  For this, a seperate GUI application has been developed to exercise
the DBUS API.  This application can be built from the following sources:
https://github.com/piggz/sailfish-fpd-community-test
