#include "fpdcommunity.h"
#include <QDebug>
#include <QDBusConnection>
#include <QDBusError>
#include <QDBusInterface>
#include <QCoreApplication>
#include <QTimer>
#include <QDir>
#include <QDataStream>
#include <QFile>
#define FINGERPRINT_PATH "/var/cache/sailfish-fpd-community/"
#define FINGERPRINT_FILE "fingerprints.db"

FPDCommunity::FPDCommunity()
{
    qDebug() << Q_FUNC_INFO;

    connect(&m_androidFP, &AndroidFP::enrollProgress, this, &FPDCommunity::slot_enrollProgress);
    connect(&m_androidFP, &AndroidFP::succeeded, this, &FPDCommunity::slot_succeeded);
    connect(&m_androidFP, &AndroidFP::failed, this, &FPDCommunity::slot_failed);
    connect(&m_androidFP, &AndroidFP::removed, this, &FPDCommunity::slot_removed);
    connect(&m_androidFP, &AndroidFP::acquired, this, &FPDCommunity::slot_acquired);
    connect(&m_androidFP, &AndroidFP::authenticated, this, &FPDCommunity::slot_authenticated);
    connect(&m_androidFP, &AndroidFP::enumerated, this, &FPDCommunity::slot_enumerated);

    //Create folder to store fingerprint names
    if (!(QDir().mkpath(FINGERPRINT_PATH))) {
        qWarning() << "Unable to create " << FINGERPRINT_PATH;
    }

    // fingers are enumerated and loaded after that
    enumerate();
    registerDBus();
}

void FPDCommunity::registerDBus()
{
    qDebug() << Q_FUNC_INFO;
    if (!m_dbusRegistered)
    {
        // DBus
        QDBusConnection connection = QDBusConnection::systemBus();
        qDebug() << "Registering service on dbus" << SERVICE_NAME;
        if (!connection.registerService(SERVICE_NAME))
        {
            qCritical() << "Didnt register service";
            QCoreApplication::quit();
            return;
        }

        if (!connection.registerObject("/org/sailfishos/fingerprint1", this, QDBusConnection::ExportAllInvokables | QDBusConnection::ExportAllSignals | QDBusConnection::ExportAllProperties))
        {
            qCritical() << "Didnt register object"
                        "";
            QCoreApplication::quit();
            return;
        }
        m_dbusRegistered = true;

        qInfo() << "Sucessfully registered to dbus systemBus";
    }
}

void FPDCommunity::saveFingers()
{
    qDebug() << Q_FUNC_INFO;

    QString filename = FINGERPRINT_PATH FINGERPRINT_FILE;
    QFile fingerprintFile(filename);

    if (!fingerprintFile.open(QIODevice::WriteOnly)){
        qWarning() << "Could not write " << filename;
        return;
    }

    QDataStream out(&fingerprintFile);
    out.setVersion(QDataStream::Qt_5_6);
    out << m_fingerMap;
}

void FPDCommunity::loadFingers()
{
    qDebug() << Q_FUNC_INFO;

    QString filename = FINGERPRINT_PATH FINGERPRINT_FILE;
    QFile fingerprintFile(filename);
    QDataStream in(&fingerprintFile);
    in.setVersion(QDataStream::Qt_5_6);

    if (!fingerprintFile.open(QIODevice::ReadOnly)) {
        qInfo() << "Could not read the file:" << filename << "Error string:" << fingerprintFile.errorString();
        qInfo() << "Assuming empty fingerprint map";
        m_fingerMap.clear();
    } else {
        in >> m_fingerMap;
    }

    QList<uint32_t> enumeratedFingers = m_androidFP.fingerprints();

    //Check each of the loaded prints to ensure it was loaded from the store
    QList<uint32_t> keys = m_fingerMap.keys();
    for (int i = 0; i < keys.size(); ++i) {
        if (!enumeratedFingers.contains(keys[i])) {
            qWarning() << "Loaded finger " << keys[i] << m_fingerMap.value(keys[i]) << "not found in store, removing";
            m_fingerMap.remove(keys[i]);
        }
    }

    // Check whether all enumerated fingerprints are in the map
    QList<uint32_t> mapped = m_fingerMap.keys();
    for (uint32_t k: enumeratedFingers) {
        if (!mapped.contains(k)) {
            qWarning() << "Unknown fingerprint found, adding to the list:" << k;
            m_fingerMap[k] = QStringLiteral("Unknown %1").arg(k);
        }
    }

    //Save after load incase any were removed or added
    saveFingers();

    qDebug() << "Loaded finger map:" << m_fingerMap;
}

int FPDCommunity::Enroll(const QString &finger)
{
    qDebug() << Q_FUNC_INFO << finger;

    // check if we have this ID already
    if (m_fingerMap.values().contains(finger)) {
        qWarning() << "Finger" << finger << "is in the database already";
        return FPREPLY_KEY_ALREADY_EXISTS;
    }

    if (m_state == FPSTATE_IDLE) {
        setState(FPSTATE_ENROLLING);
        m_addingFinger = finger;
        m_androidFP.enroll(100000); //nemo userID
        emit EnrollProgressChanged(0);
        return FPREPLY_STARTED;
    }
    return FPREPLY_ALREADY_BUSY;
}

int FPDCommunity::Identify()
{
    qDebug() << Q_FUNC_INFO;
    if (m_state != FPSTATE_IDLE) {
        return FPREPLY_ALREADY_BUSY;
    }

    if (m_fingerMap.size() == 0) {
        return FPREPLY_NO_KEYS_AVAILABLE;
    }

    setState(FPSTATE_IDENTIFYING);
    QTimer::singleShot(30000, this, &FPDCommunity::slot_cancelIdentify);
    m_androidFP.authenticate();
    return FPREPLY_STARTED;
}

void FPDCommunity::Clear()
{
    qDebug() << Q_FUNC_INFO;

    if (m_state == FPSTATE_IDLE) {
        setState(FPSTATE_REMOVING);
        m_androidFP.clear();
    }
}

QString FPDCommunity::GetState()
{
    qDebug() << Q_FUNC_INFO;
    return QtEnumToString(m_state);
}

QStringList FPDCommunity::GetAll()
{
    qDebug() << Q_FUNC_INFO;
    return m_fingerMap.values();
}

int FPDCommunity::Abort()
{
    qDebug() << Q_FUNC_INFO;
    if (m_state == FPSTATE_IDLE) {
        return FPREPLY_ALREADY_IDLE;
    }
    m_androidFP.cancel();
    setState(FPSTATE_IDLE);
    return FPREPLY_STARTED;
}

void FPDCommunity::Verify()
{

}

int FPDCommunity::Remove(const QString &finger)
{
    qDebug() << Q_FUNC_INFO << finger;

    if (m_state != FPSTATE_IDLE) {
        return FPREPLY_ALREADY_BUSY;
    }

    uint32_t key = 0;
    for (auto i = m_fingerMap.constBegin(); i != m_fingerMap.constEnd(); ++i) {
        if (i.value() == finger) {
            key = i.key();
            break;
        }
    }

    if (!key) {
        return FPREPLY_KEY_DOES_NOT_EXIST;
    }

    setState(FPSTATE_REMOVING);
    m_androidFP.remove(key);
    return FPREPLY_STARTED;
}

void FPDCommunity::enumerate()
{
    qDebug() << Q_FUNC_INFO;
    if (m_state == FPSTATE_IDLE) {
        setState(FPSTATE_ENUMERATING);
        m_androidFP.enumerate();
    }
}

void FPDCommunity::slot_enrollProgress(float pc)
{
    qDebug() << Q_FUNC_INFO << pc;
    emit EnrollProgressChanged((int)(pc * 100));
}

void FPDCommunity::slot_succeeded(uint32_t finger)
{
    qDebug() << Q_FUNC_INFO << finger;
    m_fingerMap[finger] = m_addingFinger;
    emit Added(m_addingFinger);
    m_addingFinger = "";
    saveFingers();
    setState(FPSTATE_IDLE);
    enumerate();
}

void FPDCommunity::slot_failed(const QString &message)
{
    qDebug() << Q_FUNC_INFO << message;

    if (!(m_state == FPSTATE_IDENTIFYING && message == "FINGER_NOT_RECOGNIZED")) {
        setState(FPSTATE_IDLE);
        enumerate();
    }
}

void FPDCommunity::slot_acquired(int info)
{
    qDebug() << Q_FUNC_INFO << info;
    AcquiredState newState;

    switch(info) {
    case ACQUIRED_GOOD:
        newState = FPACQUIRED_GOOD;
        break;
    case ACQUIRED_PARTIAL:
        newState = FPACQUIRED_PARTIAL;
        break;
    case ACQUIRED_INSUFFICIENT:
        newState = FPACQUIRED_INSUFFICIENT;
        break;
    case ACQUIRED_IMAGER_DIRTY:
        newState = FPACQUIRED_IMAGER_DIRTY;
        break;
    case ACQUIRED_TOO_SLOW:
        newState = FPACQUIRED_TOO_SLOW;
        break;
    case ACQUIRED_TOO_FAST:
        newState = FPACQUIRED_TOO_FAST;
        break;
    case ACQUIRED_VENDOR:
        newState = FPACQUIRED_UNRECOGNIZED;
        break;
    default:
        newState = FPACQUIRED_GOOD; //Should we default to good?
        break;
    }

    if (newState != m_acquired) {
        m_acquired = newState;
        emit AcquisitionInfo(QtEnumToString(m_acquired));
    }
}

void FPDCommunity::slot_removed(uint32_t finger)
{
    qDebug() << Q_FUNC_INFO << finger;
    QString f = m_fingerMap[finger];
    emit Removed(f);
    m_fingerMap.remove(finger);
    setState(FPSTATE_IDLE);
    enumerate();
}

void FPDCommunity::slot_authenticated(uint32_t finger)
{
    qDebug() << Q_FUNC_INFO << finger;
    if (m_fingerMap.contains(finger)){
        emit Identified(m_fingerMap[finger]);
    } else {
        qWarning() << "Authenticated finger was not found in the map.  Assuming name";
        emit Identified(QStringLiteral("Unknown %1").arg(finger));
    }
    setState(FPSTATE_IDLE);
}

void FPDCommunity::slot_cancelIdentify()
{
    qDebug() << Q_FUNC_INFO;
    m_androidFP.cancel();
    setState(FPSTATE_IDLE);
}

void FPDCommunity::slot_enumerated()
{
    qDebug() << Q_FUNC_INFO;
    setState(FPSTATE_IDLE);
    loadFingers();
    emit ListChanged();
}

void FPDCommunity::setState(FPDCommunity::State newState)
{
    qDebug() << Q_FUNC_INFO << newState;
    if (newState != m_state) {
        m_state = newState;
        emit StateChanged(QtEnumToString(m_state));
    }
}

