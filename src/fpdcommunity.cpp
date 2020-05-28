#include "fpdcommunity.h"
#include <QDebug>
#include <QDBusConnection>
#include <QDBusError>
#include <QDBusInterface>
#include <QCoreApplication>

FPDCommunity::FPDCommunity()
{
    qDebug() << Q_FUNC_INFO;
    connect(&m_androidFP, &AndroidFP::enrollProgress, this, &FPDCommunity::slot_enrollProgress);
    connect(&m_androidFP, &AndroidFP::succeeded, this, &FPDCommunity::slot_succeeded);
    connect(&m_androidFP, &AndroidFP::removed, this, &FPDCommunity::slot_removed);
    connect(&m_androidFP, &AndroidFP::acquired, this, &FPDCommunity::slot_acquired);

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
            qDebug() << "Didnt register service";
            QCoreApplication::quit();
            return;
        }

        if (!connection.registerObject("/", this, QDBusConnection::ExportAllInvokables | QDBusConnection::ExportAllSignals | QDBusConnection::ExportAllProperties))
        {
            qDebug() << "Didnt register object"
                        "";
            QCoreApplication::quit();
            return;
        }
        m_dbusRegistered = true;

        qDebug() << "Sucessfully registered to dbus systemBus";
    }
}

void FPDCommunity::Enroll(const QString &finger)
{
    qDebug() << Q_FUNC_INFO << finger;

    if (m_state == FPSTATE_IDLE) {
        setState(FPSTATE_ENROLLING);
        m_androidFP.enroll(100000); //nemo userID
    }
}

void FPDCommunity::Identify()
{
    qDebug() << Q_FUNC_INFO;
    m_androidFP.authenticate();
}

void FPDCommunity::Clear()
{
    qDebug() << Q_FUNC_INFO;

    if (m_state == FPSTATE_IDLE) {
        setState(FPSTATE_REMOVING);
        m_androidFP.clear();
    }
}

void FPDCommunity::slot_enrollProgress(float pc)
{
    qDebug() << Q_FUNC_INFO << pc;
    emit EnrollProgressChanged(pc * 100);
}

void FPDCommunity::slot_succeeded(int finger)
{
    qDebug() << Q_FUNC_INFO << finger;
    setState(FPSTATE_IDLE);
    emit Added(QString::number(finger));
}

void FPDCommunity::slot_acquired(UHardwareBiometryFingerprintAcquiredInfo info)
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

void FPDCommunity::slot_removed(int finger)
{
    qDebug() << Q_FUNC_INFO << finger;
    setState(FPSTATE_IDLE);
}

void FPDCommunity::setState(FPDCommunity::State newState)
{
    qDebug() << Q_FUNC_INFO << newState;
    if (newState != m_state) {
        m_state = newState;
        emit StateChanged(QtEnumToString(m_state));
    }
}

