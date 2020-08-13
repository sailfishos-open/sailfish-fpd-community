# NOTICE:
#
# Application name defined in TARGET has a corresponding QML filename.
# If name defined in TARGET is changed, the following needs to be done
# to match new name:
#   - corresponding QML filename must be changed
#   - desktop icon filename must be changed
#   - desktop filename must be changed
#   - icon definition filename in desktop file must be changed
#   - translation filenames have to be changed

# The name of your application
TARGET = sailfish-fpd-community

QT += dbus

CONFIG += sailfishapp

LIBS += -lhybris-common

SOURCES += src/sailfish-fpd-community.cpp \
    src/androidfp.cpp \
    src/fpdcommunity.cpp \
    src/hardware/biometry_fp_api.cpp \
    src/util/property_store.cpp

DISTFILES += \
    org.sailfishos.fingerprint1.conf \
    rpm/sailfish-fpd-community.changes.in \
    rpm/sailfish-fpd-community.changes.run.in \
    rpm/sailfish-fpd-community.spec \
    sailfish-fpd-community.service

# to disable building translations every time, comment out the
# following CONFIG line
CONFIG += sailfishapp_i18n

HEADERS += \
    src/androidfp.h \
    src/biometry.h \
    src/fpdcommunity.h \
    src/hardware/android_hw_module.h \
    src/util/property_store.h

target.path = /usr/bin/
systemd_services.path = $$[QT_INSTALL_LIBS]/systemd/system/
systemd_services.files = sailfish-fpd-community.service

dbus.files = org.sailfishos.fingerprint1.conf
dbus.path = /etc/dbus-1/system.d/

INSTALLS += target \
            systemd_services \
            dbus

