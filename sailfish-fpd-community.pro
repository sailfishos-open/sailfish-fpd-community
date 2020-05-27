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
    rpm/sailfish-fpd-community.changes.in \
    rpm/sailfish-fpd-community.changes.run.in \
    rpm/sailfish-fpd-community.spec \
    rpm/sailfish-fpd-community.yaml \
    translations/*.ts \
    sailfish-fpd-community.desktop

SAILFISHAPP_ICONS = 86x86 108x108 128x128 172x172

# to disable building translations every time, comment out the
# following CONFIG line
CONFIG += sailfishapp_i18n

# German translation is enabled as an example. If you aren't
# planning to localize your app, remember to comment out the
# following TRANSLATIONS line. And also do not forget to
# modify the localized app name in the the .desktop file.
TRANSLATIONS += translations/sailfish-fpd-community-de.ts

HEADERS += \
    src/androidfp.h \
    src/biometry.h \
    src/fpdcommunity.h \
    src/hardware/android_hw_module.h \
    src/util/property_store.h
